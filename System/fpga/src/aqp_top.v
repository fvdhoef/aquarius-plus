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
    output wire  [9:0] exp,

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
    input  wire        esp_sclk,        // Connected to EXP7
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    assign exp = 10'b0;

    wire [15:0] spibm_a;
    wire  [7:0] spibm_wrdata;
    wire        spibm_wrdata_en;
    wire        spibm_en;
    wire        spibm_rd_n, spibm_wr_n, spibm_mreq_n, spibm_iorq_n;
    wire        spibm_busreq;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk, vclk;
    wire video_mode;

    aqp_clkctrl clkctrl(
        .clk_in(sysclk),    // 14.31818MHz
        .clk_out(clk),      // 28.63636MHz

        .vclk(vclk),
        .video_mode(video_mode)
    );

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset_req;
    wire turbo;
    wire reset;

    aqp_sysctrl sysctrl(
        .sysclk(clk),
        .ebus_reset_n(ebus_reset_n),
        .reset_req(reset_req),

        .turbo_mode(turbo),

        .ebus_phi(ebus_phi),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    wire       ebus_int_n_pushpull;
    wire [7:0] ebus_d_out;
    wire       ebus_d_oe;

    assign ebus_int_n = !ebus_int_n_pushpull ? 1'b0 : 1'bZ;

    // Register data from external bus
    reg [7:0] ebus_d_in;
    always @(posedge clk) if (!ebus_wr_n) ebus_d_in <= ebus_d;

    assign ebus_d = (spibm_en && spibm_wrdata_en) ? spibm_wrdata : (ebus_d_oe ? ebus_d_out : 8'bZ);

    reg [2:0] q_ebus_wr_n;
    reg [2:0] q_ebus_rd_n;
    always @(posedge clk) q_ebus_wr_n <= {q_ebus_wr_n[1:0], ebus_wr_n};
    always @(posedge clk) q_ebus_rd_n <= {q_ebus_rd_n[1:0], ebus_rd_n};

    wire bus_read  = q_ebus_rd_n[2:1] == 2'b10;
    wire bus_write = q_ebus_wr_n[2:1] == 2'b10;
    wire common_ebus_stb = bus_read || bus_write;

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    wire  [8:0] esp_tx_data;   // if bit8 set: transmit break, ignore data
    wire  [7:0] esp_rx_data;

    wire esp_txvalid;
    wire esp_txbusy;

    wire esp_rxfifo_not_empty;
    wire esp_rxfifo_read;
    wire esp_rxfifo_overflow, esp_rx_framing_error, esp_rx_break;

    aqp_esp_uart esp_uart(
        .clk(clk),
        .reset(reset),

        .tx_data(ebus_d_in),
        .tx_valid(esp_txvalid),
        .tx_break(esp_txvalid && esp_tx_data[8]),
        .tx_busy(esp_txbusy),

        .rxfifo_data(esp_rx_data),
        .rxfifo_not_empty(esp_rxfifo_not_empty),
        .rxfifo_read(esp_rxfifo_read),
        .rxfifo_overflow(esp_rxfifo_overflow),
        .rx_framing_error(esp_rx_framing_error),
        .rx_break(esp_rx_break),

        .uart_rxd(esp_rx),
        .uart_txd(esp_tx),
        .uart_cts(esp_cts),
        .uart_rts(esp_rts));

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
    assign hc1[8] = 1'b0;
    assign hc2[8] = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // ESP SPI slave interface
    //////////////////////////////////////////////////////////////////////////
    assign spibm_en      = spibm_busreq && !ebus_busack_n;
    assign ebus_a        = spibm_en ? spibm_a      : 16'bZ;
    assign ebus_rd_n     = spibm_en ? spibm_rd_n   : 1'bZ;
    assign ebus_wr_n     = spibm_en ? spibm_wr_n   : 1'bZ;
    assign ebus_mreq_n   = spibm_en ? spibm_mreq_n : 1'bZ;
    assign ebus_iorq_n   = spibm_en ? spibm_iorq_n : 1'bZ;
    assign ebus_busreq_n = spibm_busreq ? 1'b0 : 1'bZ;

    wire        spi_msg_end;
    wire  [7:0] spi_cmd;
    wire [63:0] spi_rxdata;

    aqp_esp_spi esp_spi(
        .clk(clk),
        .reset(reset),

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
        .spibm_busreq(spibm_busreq),

        // Interface for core specific messages
        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        
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
    // Aq+ common
    //////////////////////////////////////////////////////////////////////////
    wire        video_de;
    wire        video_newframe;
    wire        video_oddline;
    wire        turbo_unlimited; // unused

    aqplus_common common(
        .clk(clk),
        .reset(reset),

        .reset_req(reset_req),

        .vclk(vclk),

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
        .ebus_cart_ce_n(ebus_cart_ce_n),
        .ebus_ram_we_n(ebus_ram_we_n),

        .ebus_stb(common_ebus_stb),

        // Video output
        .video_r(vga_r),
        .video_g(vga_g),
        .video_b(vga_b),
        .video_de(video_de),
        .video_hsync(vga_hsync),
        .video_vsync(vga_vsync),
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

        // ESP UART interface
        .esp_tx_data(esp_tx_data),
        .esp_tx_wr(esp_txvalid),
        .esp_tx_fifo_full(esp_txbusy),
        .esp_rx_data({esp_rx_break, esp_rx_data}),
        .esp_rx_rd(esp_rxfifo_read),
        .esp_rx_empty(!esp_rxfifo_not_empty),
        .esp_rx_fifo_overflow(esp_rxfifo_overflow),
        .esp_rx_framing_error(esp_rx_framing_error),

        // Other
        .cassette_out(cassette_out),
        .cassette_in(cassette_in),
        .printer_out(printer_out),
        .printer_in(printer_in),
        .turbo(turbo),
        .turbo_unlimited(turbo_unlimited),

        // Hand controller interface
        .hc1_in(hc1_in),
        .hc1_out(hc1_out),
        .hc1_oe(hc1_oe),

        .hc2_in(hc2_in),
        .hc2_out(hc2_out),
        .hc2_oe(hc2_oe)
    );

endmodule
