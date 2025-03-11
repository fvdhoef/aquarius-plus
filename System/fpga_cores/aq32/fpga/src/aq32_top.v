`default_nettype none
`timescale 1 ns / 1 ps

module aq32_top(
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

    assign exp            = 9'b0;
    assign hc1[7:0]       = 8'bZ;
    assign hc2[7:0]       = 8'bZ;
    assign hc1[8]         = 1'b0;
    assign hc2[8]         = 1'b0;
    assign cassette_out   = 1'b0;
    assign printer_out    = 1'b0;
    assign ebus_cart_ce_n = 1'b1;
    assign ebus_reset_n   = 1'bZ;
    assign ebus_wr_n      = 1'b1;
    assign ebus_a[15:14]  = 2'bZ;
    assign ebus_mreq_n    = 1'b1;
    assign ebus_iorq_n    = 1'b1;
    assign ebus_int_n     = 1'bZ;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk, video_clk;

    aqp_clkctrl clkctrl(
        .clk_in(sysclk),        // 14.31818MHz
        .clk_out(clk),          // 28.63636MHz
        .video_clk(video_clk)   // 25.175MHz
    );

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset_req = 1'b0;
    wire ebus_phi_clken;
    wire reset;

    aqp_sysctrl sysctrl(
        .sysclk(clk),
        .reset_req(reset_req),

        .ebus_phi(ebus_phi),
        .ebus_phi_clken(ebus_phi_clken),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // CPU
    //////////////////////////////////////////////////////////////////////////
    wire [31:0] cpu_addr;
    wire [31:0] cpu_wrdata;
    wire  [3:0] cpu_bytesel;
    wire        cpu_wren;
    wire  [1:0] cpu_cache_op;
    wire        cpu_strobe;
    reg         cpu_wait;
    reg  [31:0] cpu_rddata;
    wire        cpu_error;
    wire        cpu_m_mode;
    wire        cpu_tlb_miss;
    wire  [5:0] cpu_asid;

    wire [15:0] cpu_irq = {16'b0};

    cpu #(.VEC_RESET(32'hFFFFF800)) cpu(
        .clk(clk),
        .reset(reset),

        // Bus interface
        .bus_addr(cpu_addr),
        .bus_wrdata(cpu_wrdata),
        .bus_bytesel(cpu_bytesel),
        .bus_wren(cpu_wren),
        .bus_cache_op(cpu_cache_op),
        .bus_strobe(cpu_strobe),
        .bus_wait(cpu_wait),
        .bus_rddata(cpu_rddata),
        .bus_error(cpu_error),
        .cpu_tlb_miss(cpu_tlb_miss),
        .cpu_m_mode(cpu_m_mode),
        .cpu_asid(cpu_asid),

        // Interrupt input
        .irq(cpu_irq));

    //////////////////////////////////////////////////////////////////////////
    // Boot ROM
    //////////////////////////////////////////////////////////////////////////
    wire [31:0] bootrom_rddata;

    bootrom bootrom(
        .clk(clk),
        .addr(cpu_addr[10:2]),
        .rddata(bootrom_rddata));

    //////////////////////////////////////////////////////////////////////////
    // SRAM controller
    //////////////////////////////////////////////////////////////////////////
    wire [18:0] sram_a;
    wire        sram_ctrl_strobe;
    wire        sram_ctrl_wait;
    wire [31:0] sram_ctrl_rddata;

    assign ebus_a[13:0]  = sram_a[13:0];
    assign ebus_ba       = sram_a[18:14];

    sram_ctrl sram_ctrl(
        .clk(clk),
        .reset(reset),

        // Command interface
        .bus_addr(cpu_addr[18:2]),
        .bus_wrdata(cpu_wrdata),
        .bus_bytesel(cpu_bytesel),
        .bus_wren(cpu_wren),
        .bus_strobe(sram_ctrl_strobe),
        .bus_wait(sram_ctrl_wait),
        .bus_rddata(sram_ctrl_rddata),

        // SRAM interface
        .sram_a(sram_a),
        .sram_ce_n(ebus_ram_ce_n),
        .sram_oe_n(ebus_rd_n),
        .sram_we_n(ebus_ram_we_n),
        .sram_dq(ebus_d));

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

    assign esp_tx_data = 9'b0;
    assign esp_tx_wr   = 1'b0;
    assign esp_rx_rd   = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // ESP SPI slave interface
    //////////////////////////////////////////////////////////////////////////
    assign ebus_busreq_n = 1'b0;

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

    assign spi_txdata       = 64'b0;
    assign spi_txdata_valid = 1'b0;

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
            1'b1,       // Core type 01 specific: show mouse support
            1'b0        // Z80 present
        }),
        .sysinfo_version_major(8'h00),
        .sysinfo_version_minor(8'h01),

        .core_name("Aquarius32      "),

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

    assign common_audio_l = 16'd0;
    assign common_audio_r = 16'd0;

    //////////////////////////////////////////////////////////////////////////
    // Video
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] video_r;
    wire [3:0] video_g;
    wire [3:0] video_b;
    wire       video_de;
    wire       video_hsync;
    wire       video_vsync;
    wire       video_newframe;
    wire       video_oddline;

    wire       video_irq;
    wire       reg_fd_val;

    wire       io_video_strobe;
    wire       tram_strobe;
    wire       chram_strobe;
    wire       vram_strobe;

    wire       io_video_wren = cpu_wren && io_video_strobe;
    wire       tram_wren     = cpu_wren && tram_strobe;
    wire       chram_wren    = cpu_wren && chram_strobe;
    wire       vram_wren     = cpu_wren && vram_strobe;

    wire [7:0] rddata_tram;
    wire [7:0] rddata_chram;
    wire [7:0] rddata_vram;
    wire [7:0] rddata_io_video;

    video video(
        .clk(clk),
        .reset(reset),

        .vclk(video_clk),

        .io_addr(cpu_addr[3:0]),
        .io_rddata(rddata_io_video),
        .io_wrdata(cpu_wrdata[7:0]),
        .io_wren(io_video_wren),
        .irq(video_irq),

        .tram_addr(cpu_addr[10:0]),
        .tram_rddata(rddata_tram),
        .tram_wrdata(cpu_wrdata[7:0]),
        .tram_wren(tram_wren),

        .chram_addr(cpu_addr[10:0]),
        .chram_rddata(rddata_chram),
        .chram_wrdata(cpu_wrdata[7:0]),
        .chram_wren(chram_wren),

        .vram_addr(cpu_addr[13:0]),
        .vram_rddata(rddata_vram),
        .vram_wrdata(cpu_wrdata[7:0]),
        .vram_wren(vram_wren),

        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),
        .video_de(video_de),
        .video_hsync(video_hsync),
        .video_vsync(video_vsync),
        .video_newframe(video_newframe),
        .video_oddline(video_oddline),

        .reg_fd_val(reg_fd_val));

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
        .video_mode(1'b1),

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
    // CPU bus interconnect
    //////////////////////////////////////////////////////////////////////////
    assign sram_ctrl_strobe = cpu_strobe && cpu_addr[31:19] == {12'hFFF, 1'b0};
    assign tram_strobe      = cpu_strobe && cpu_addr[31:12] == {20'hFF000};
    assign chram_strobe     = cpu_strobe && cpu_addr[31:11] == {20'hFF100, 1'b0};
    assign vram_strobe      = cpu_strobe && cpu_addr[31:14] == {16'hFF20, 2'b00};
    assign io_video_strobe  = cpu_strobe && cpu_addr[31:14] == {16'hFF30, 2'b00};
    wire   bootrom_strobe   = cpu_strobe && cpu_addr[31:11] == {20'hFFFFF, 1'b1};

    reg [31:0] q_cpu_addr;
    always @(posedge clk) q_cpu_addr <= cpu_addr;

    always @* begin
        cpu_wait = 0;
        if (sram_ctrl_strobe) cpu_wait = sram_ctrl_wait;
        if (tram_strobe)      cpu_wait = !cpu_wren && q_cpu_addr[11:0] != cpu_addr[11:0];
        if (chram_strobe)     cpu_wait = !cpu_wren && q_cpu_addr[10:0] != cpu_addr[10:0];
        if (vram_strobe)      cpu_wait = !cpu_wren && q_cpu_addr[13:0] != cpu_addr[13:0];
        if (io_video_strobe)  cpu_wait = !cpu_wren && q_cpu_addr[13:0] != cpu_addr[13:0];
        if (bootrom_strobe)   cpu_wait = !cpu_wren && q_cpu_addr[10:2] != cpu_addr[10:2];
    end

    always @* begin
        cpu_rddata = 0;
        if (sram_ctrl_strobe) cpu_rddata = sram_ctrl_rddata;
        if (tram_strobe)      cpu_rddata = {rddata_tram,     rddata_tram,     rddata_tram,     rddata_tram};
        if (chram_strobe)     cpu_rddata = {rddata_chram,    rddata_chram,    rddata_chram,    rddata_chram};
        if (vram_strobe)      cpu_rddata = {rddata_vram,     rddata_vram,     rddata_vram,     rddata_vram};
        if (io_video_strobe)  cpu_rddata = {rddata_io_video, rddata_io_video, rddata_io_video, rddata_io_video};
        if (bootrom_strobe)   cpu_rddata = bootrom_rddata;
    end

    assign cpu_error    = 0;
    assign cpu_tlb_miss = 0;

endmodule
