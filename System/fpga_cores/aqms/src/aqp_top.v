`default_nettype none
`timescale 1 ns / 1 ps

module aqp_top(
    input  wire        sysclk,          // 14.31818MHz

    // Z80 bus interface
    inout  wire        ebus_reset_n,
    output wire        ebus_phi,        // 3.579545MHz
    inout  wire [15:0] ebus_a,
    inout  wire  [7:0] ebus_d,
    inout  wire        ebus_rd_n,
    inout  wire        ebus_wr_n,
    inout  wire        ebus_mreq_n,
    inout  wire        ebus_iorq_n,
    output wire        ebus_int_n,      // Open-drain output
    output wire        ebus_busreq_n,   // Open-drain output
    input  wire        ebus_busack_n,
    output wire  [4:0] ebus_ba,
    output wire        ebus_ram_ce_n,   // 512KB RAM
    output wire        ebus_cart_ce_n,  // Cartridge
    output wire        ebus_ram_we_n,

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output wire        cassette_out,
    input  wire        cassette_in,
    output wire        printer_out,
    input  wire        printer_in,

    // Misc
    output wire  [8:0] exp,
    input  wire        has_z80,

    // Hand controller interface
    inout  wire  [8:0] hc1,
    inout  wire  [8:0] hc2,

    // VGA output
    output wire  [3:0] vga_r,
    output wire  [3:0] vga_g,
    output wire  [3:0] vga_b,
    output wire        vga_hsync,
    output wire        vga_vsync,

    // ESP32 serial interface
    output wire        esp_tx,
    input  wire        esp_rx,
    output wire        esp_rts,
    input  wire        esp_cts,

    // ESP32 SPI interface (also used for loading FPGA image)
    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    assign exp = 9'b0;

    wire [15:0] spibm_a;
    wire  [7:0] spibm_wrdata;
    wire        spibm_wrdata_en;
    wire        spibm_en;
    wire        spibm_rd_n, spibm_wr_n, spibm_mreq_n, spibm_iorq_n;
    wire        spibm_busreq_n;

    wire  [7:0] ebus_d_out;
    wire        ebus_d_oe;

    wire        use_t80;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk, video_clk;
    wire video_mode;

    aqp_clkctrl clkctrl(
        .clk_in(sysclk),    // 14.31818MHz
        .clk_out(clk),      // 28.63636MHz

        .video_clk(video_clk),
        .video_mode(video_mode)
    );

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset_req;
    wire turbo;
    wire turbo_unlimited;
    wire ebus_phi_clken;
    wire reset;

    aqp_sysctrl sysctrl(
        .sysclk(clk),
        .ebus_reset_n(ebus_reset_n),
        .reset_req(reset_req),

        .turbo_mode(turbo),
        .turbo_unlimited(turbo_unlimited && use_t80),

        .ebus_phi(ebus_phi),
        .ebus_phi_clken(ebus_phi_clken),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    wire ebus_int_n_pushpull;

    assign ebus_int_n = !ebus_int_n_pushpull ? 1'b0 : 1'bZ;

    // Register data from external bus
    reg [7:0] ebus_d_in;
    always @(posedge clk) if (!ebus_wr_n) ebus_d_in <= ebus_d;

    // Synchronize RD#/WR# signals for when using external Z80
    reg [2:0] q_ebus_wr_n;
    reg [2:0] q_ebus_rd_n;
    always @(posedge clk) q_ebus_wr_n <= {q_ebus_wr_n[1:0], ebus_wr_n};
    always @(posedge clk) q_ebus_rd_n <= {q_ebus_rd_n[1:0], ebus_rd_n};

    wire bus_read  = (use_t80 ? q_ebus_rd_n[1:0] : q_ebus_rd_n[2:1]) == 2'b10;
    wire bus_write = (use_t80 ? q_ebus_wr_n[1:0] : q_ebus_wr_n[2:1]) == 2'b10;
    wire common_ebus_stb = (bus_read || bus_write);

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] esp_tx_data;
    wire       esp_tx_wr;
    wire       esp_tx_fifo_full;
    wire [8:0] esp_rx_data;
    wire       esp_rx_rd;
    wire       esp_rx_empty;
    wire       esp_rx_fifo_overflow;
    wire       esp_rx_framing_error;

    aqp_esp_uart esp_uart(
        .clk(clk),
        .reset(reset),

        .txfifo_data(esp_tx_data),
        .txfifo_wr(esp_tx_wr),
        .txfifo_full(esp_tx_fifo_full),

        .rxfifo_data(esp_rx_data),
        .rxfifo_rd(esp_rx_rd),
        .rxfifo_empty(esp_rx_empty),
        .rxfifo_overflow(esp_rx_fifo_overflow),
        .rx_framing_error(esp_rx_framing_error),

        .esp_rx(esp_rx),
        .esp_tx(esp_tx),
        .esp_cts(esp_cts),
        .esp_rts(esp_rts));

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    wire  [7:0] hc1_in = hc1[7:0];
    wire  [7:0] hc1_out;
    wire        hc1_oe;

    wire  [7:0] hc2_in = hc2[7:0];
    wire  [7:0] hc2_out;
    wire        hc2_oe;

    assign hc1[7:0] = hc1_oe ? hc1_out : 8'bZ;
    assign hc2[7:0] = hc2_oe ? hc2_out : 8'bZ;
    assign hc1[8]   = 1'b0;
    assign hc2[8]   = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // ESP SPI slave interface
    //////////////////////////////////////////////////////////////////////////
    assign spibm_en      = !spibm_busreq_n && (!ebus_busack_n || !has_z80);
    assign ebus_busreq_n = !(use_t80 || !spibm_busreq_n);

    wire        spi_msg_end;
    wire  [7:0] spi_cmd;
    wire [63:0] spi_rxdata;
    wire [63:0] spi_txdata;
    wire        spi_txdata_valid;

    wire  [9:0] ovl_text_addr;
    wire [15:0] ovl_text_wrdata;
    wire        ovl_text_wr;

    wire [10:0] ovl_font_addr;
    wire  [7:0] ovl_font_wrdata;
    wire        ovl_font_wr;

    wire  [3:0] ovl_palette_addr;
    wire [15:0] ovl_palette_wrdata;
    wire        ovl_palette_wr;

    aqp_esp_spi esp_spi(
        .clk(clk),
        .reset(reset),

        // System information
        .sysinfo_core_type(8'h01),
        .sysinfo_flags({
            1'b0,       // Core type 01 specific: unused
            1'b0,       // Core type 01 specific: unused
            1'b0,       // Core type 01 specific: unused
            1'b0,       // Core type 01 specific: show force turbo mode
            1'b0,       // Core type 01 specific: show Aquarius+ options
            1'b0,       // Core type 01 specific: show video timing switch
            1'b0,       // Core type 01 specific: show mouse support
            has_z80     // Z80 present
        }),
        .sysinfo_version_major(8'h01),
        .sysinfo_version_minor(8'h00),

        .core_name("Master System   "),

        // Bus master interface
        .ebus_phi(ebus_phi),

        .spibm_a(spibm_a),
        .spibm_rddata(ebus_d),
        .spibm_wrdata(spibm_wrdata),
        .spibm_wrdata_en(spibm_wrdata_en),
        .spibm_rd_n(spibm_rd_n),
        .spibm_wr_n(spibm_wr_n),
        .spibm_mreq_n(spibm_mreq_n),
        .spibm_iorq_n(spibm_iorq_n),
        .spibm_busreq_n(spibm_busreq_n),

        // Interface for core specific messages
        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        .spi_txdata(spi_txdata),
        .spi_txdata_valid(spi_txdata_valid),

        // Display overlay interface
        .ovl_text_addr(ovl_text_addr),
        .ovl_text_wrdata(ovl_text_wrdata),
        .ovl_text_wr(ovl_text_wr),

        .ovl_font_addr(ovl_font_addr),
        .ovl_font_wrdata(ovl_font_wrdata),
        .ovl_font_wr(ovl_font_wr),

        .ovl_palette_addr(ovl_palette_addr),
        .ovl_palette_wrdata(ovl_palette_wrdata),
        .ovl_palette_wr(ovl_palette_wr),

        // ESP SPI slave interface
        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),
        .esp_notify(esp_notify));

    //////////////////////////////////////////////////////////////////////////
    // PWM DAC
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] common_audio_l;
    wire [15:0] common_audio_r;

    aqp_pwm_dac pwm_dac(
        .clk(clk),
        .reset(reset),

        // Sample input
        .next_sample(1'b1),
        .left_data(common_audio_l),
        .right_data(common_audio_r),

        // PWM audio output
        .audio_l(audio_l),
        .audio_r(audio_r));

    //////////////////////////////////////////////////////////////////////////
    // Core common
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] video_r;
    wire [3:0] video_g;
    wire [3:0] video_b;
    wire       video_de;
    wire       video_hsync;
    wire       video_vsync;
    wire       video_newframe;
    wire       video_oddline;

    aqms_common common(
        .clk(clk),
        .reset(reset),

        .reset_req(reset_req),
        .use_t80(use_t80),
        .has_z80(has_z80),

        // Bus interface
        .ebus_a(ebus_a),
        .ebus_d_in(ebus_d_in),
        .ebus_d_out(ebus_d_out),
        .ebus_d_oe(ebus_d_oe),
        .ebus_rd_n(ebus_rd_n),
        .ebus_wr_n(ebus_wr_n),
        .ebus_mreq_n(ebus_mreq_n),
        .ebus_iorq_n(ebus_iorq_n),
        .ebus_int_n(ebus_int_n_pushpull),
        .ebus_ba(ebus_ba),
        .ebus_ram_ce_n(ebus_ram_ce_n),
        .ebus_ram_we_n(ebus_ram_we_n),

        .ebus_stb(common_ebus_stb),

        // Video output
        .video_clk(video_clk),
        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),
        .video_de(video_de),
        .video_hsync(video_hsync),
        .video_vsync(video_vsync),
        .video_newframe(video_newframe),
        .video_oddline(video_oddline),
        .video_mode(video_mode),

        // Audio output
        .audio_l(common_audio_l),
        .audio_r(common_audio_r),

        // ESP SPI interface
        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        .spi_txdata(spi_txdata),
        .spi_txdata_valid(spi_txdata_valid),

        // ESP UART interface
        .esp_tx_data(esp_tx_data),
        .esp_tx_wr(esp_tx_wr),
        .esp_tx_fifo_full(esp_tx_fifo_full),
        .esp_rx_data(esp_rx_data),
        .esp_rx_rd(esp_rx_rd),
        .esp_rx_empty(esp_rx_empty),
        .esp_rx_fifo_overflow(esp_rx_fifo_overflow),
        .esp_rx_framing_error(esp_rx_framing_error),

        // Other
        .turbo(turbo),
        .turbo_unlimited(turbo_unlimited),

        // Hand controller interface
        .hc1_in(hc1_in),
        .hc2_in(hc2_in)
    );

    assign ebus_cart_ce_n = 1'b1;
    assign cassette_out   = 1'b0;
    assign printer_out    = 1'b1;

    assign hc1_out = 8'b0;
    assign hc1_oe  = 1'b0;
    assign hc2_out = 8'b0;
    assign hc2_oe  = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Display overlay
    //////////////////////////////////////////////////////////////////////////
    aqp_overlay overlay(
        // Core video interface
        .video_clk(video_clk),
        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),
        .video_de(video_de),
        .video_hsync(video_hsync),
        .video_vsync(video_vsync),
        .video_newframe(video_newframe),
        .video_oddline(video_oddline),
        .video_mode(video_mode),

        // Overlay interface
        .ovl_clk(clk),

        .ovl_text_addr(ovl_text_addr),
        .ovl_text_wrdata(ovl_text_wrdata),
        .ovl_text_wr(ovl_text_wr),

        .ovl_font_addr(ovl_font_addr),
        .ovl_font_wrdata(ovl_font_wrdata),
        .ovl_font_wr(ovl_font_wr),

        .ovl_palette_addr(ovl_palette_addr),
        .ovl_palette_wrdata(ovl_palette_wrdata),
        .ovl_palette_wr(ovl_palette_wr),

        // VGA signals
        .vga_r(vga_r),
        .vga_g(vga_g),
        .vga_b(vga_b),
        .vga_hsync(vga_hsync),
        .vga_vsync(vga_vsync)
    );

    //////////////////////////////////////////////////////////////////////////
    // T80 core
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] t80_addr;        // should tristate when busak_n == 0
    wire  [7:0] t80_dq_out;
    wire  [7:0] t80_dq_in = ebus_d;
    wire        t80_dq_oe;

    wire        t80_mreq_n;      // should tristate when busak_n == 0
    wire        t80_iorq_n;      // should tristate when busak_n == 0
    wire        t80_rd_n;        // should tristate when busak_n == 0
    wire        t80_wr_n;        // should tristate when busak_n == 0
    wire        t80_wait_n = 1'b1;

    wire        t80_busrq_n = spibm_busreq_n;
    wire        t80_busak_n;

    wire        t80_int_n = ebus_int_n_pushpull;
    wire        t80_nmi_n = 1'b1;

    aqp_t80 aqp_t80(
        .clk(clk),
        .reset(reset || !use_t80),
        .clken(ebus_phi_clken),
        .phi(ebus_phi),

        .addr(t80_addr),        // should tristate when busak_n == 0
        .dq_out(t80_dq_out),
        .dq_in(t80_dq_in),
        .dq_oe(t80_dq_oe),

        .mreq_n(t80_mreq_n),    // should tristate when busak_n == 0
        .iorq_n(t80_iorq_n),    // should tristate when busak_n == 0
        .rd_n(t80_rd_n),        // should tristate when busak_n == 0
        .wr_n(t80_wr_n),        // should tristate when busak_n == 0
        .wait_n(t80_wait_n),

        .busrq_n(t80_busrq_n),
        .busak_n(t80_busak_n),

        .int_n(t80_int_n),
        .nmi_n(t80_nmi_n)
    );

    //////////////////////////////////////////////////////////////////////////
    // Bus logic
    //////////////////////////////////////////////////////////////////////////
    assign ebus_a      = spibm_en ? spibm_a      : (use_t80 ? t80_addr   : 16'bZ);
    assign ebus_rd_n   = spibm_en ? spibm_rd_n   : (use_t80 ? t80_rd_n   : 1'bZ);
    assign ebus_wr_n   = spibm_en ? spibm_wr_n   : (use_t80 ? t80_wr_n   : 1'bZ);
    assign ebus_mreq_n = spibm_en ? spibm_mreq_n : (use_t80 ? t80_mreq_n : 1'bZ);
    assign ebus_iorq_n = spibm_en ? spibm_iorq_n : (use_t80 ? t80_iorq_n : 1'bZ);

    assign ebus_d =
        (spibm_en && spibm_wrdata_en) ? spibm_wrdata :
        (use_t80 && t80_dq_oe         ? t80_dq_out   :
        (ebus_d_oe                    ? ebus_d_out   :
                                        8'bZ));

endmodule
