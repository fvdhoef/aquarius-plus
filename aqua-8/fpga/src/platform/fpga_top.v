`default_nettype none
`timescale 1 ns / 1 ps

module fpga_top(
    input  wire        sysclk,

    // Z80 bus interface
    inout  wire        ebus_reset_n,
    output wire        ebus_phi,
    output wire [15:0] ebus_a,
    inout  wire  [7:0] ebus_d,
    output wire        ebus_rd_n,
    output wire        ebus_wr_n,
    output wire        ebus_mreq_n,
    output wire        ebus_iorq_n,
    output wire        ebus_int_n,
    output wire        ebus_busreq_n,
    input  wire        ebus_busack_n,
    output wire  [4:0] ebus_ba,
    output wire        ebus_ram_ce_n,
    output wire        ebus_cart_ce_n,
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

    //////////////////////////////////////////////////////////////////////////
    // Clock generator
    //////////////////////////////////////////////////////////////////////////
    wire clk_25_175;
    wire clk_28_63636;

    fpga_clkgen fpga_clkgen(
        .sysclk(sysclk),
        .clk_25_175(clk_25_175),
        .clk_28_63636(clk_28_63636));

    //////////////////////////////////////////////////////////////////////////
    // Generate reset signal
    //////////////////////////////////////////////////////////////////////////
    wire spi_reset_req;

`ifdef MODEL_TECH
    reg [4:0] q_reset_cnt = 0;
    always @(posedge clk_25_175)
        if      (spi_reset_req)   q_reset_cnt <= 0;
        else if (!q_reset_cnt[4]) q_reset_cnt <= q_reset_cnt + 20'd1;

    wire reset_25_175 = !q_reset_cnt[4];
`else
    reg [19:0] q_reset_cnt = 0;
    always @(posedge clk_25_175)
        if      (spi_reset_req)    q_reset_cnt <= 0;
        else if (!q_reset_cnt[19]) q_reset_cnt <= q_reset_cnt + 20'd1;

    wire reset_25_175 = !q_reset_cnt[19];
`endif

    //////////////////////////////////////////////////////////////////////////
    // Handle unused external Z80 and peripherals
    //////////////////////////////////////////////////////////////////////////
    reg [2:0] q_clk_div = 0;
    always @(posedge(clk_25_175)) q_clk_div <= q_clk_div + 1;

    assign ebus_phi       = q_clk_div[2];
    assign ebus_reset_n   = 0;
    assign ebus_busreq_n  = 0;
    assign ebus_mreq_n    = 1;
    assign ebus_iorq_n    = 1;
    assign ebus_wr_n      = 1;
    assign ebus_a[15:14]  = 2'bZ;
    assign ebus_int_n     = 1'bZ;
    assign ebus_cart_ce_n = 1;

    assign cassette_out   = 0;
    assign printer_out    = 0;
    assign exp            = 0;

    //////////////////////////////////////////////////////////////////////////
    // Video overlay
    //////////////////////////////////////////////////////////////////////////
    wire  [9:0] video_hpos;
    wire        video_hlast;
    wire  [9:0] video_vpos;
    wire        video_vlast;
    wire  [3:0] video_r;
    wire  [3:0] video_g;
    wire  [3:0] video_b;

    wire  [9:0] ovl_text_addr;
    wire [15:0] ovl_text_wrdata;
    wire        ovl_text_wren;
    wire [10:0] ovl_font_addr;
    wire  [7:0] ovl_font_wrdata;
    wire        ovl_font_wren;
    wire  [3:0] ovl_palette_addr;
    wire [15:0] ovl_palette_wrdata;
    wire        ovl_palette_wren;

    fpga_overlay fpga_overlay(
        .clk(clk_25_175),

        // Core video interface
        .video_hpos(video_hpos),
        .video_hlast(video_hlast),
        .video_vpos(video_vpos),
        .video_vlast(video_vlast),
        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),

        // Overlay interface
        .ovl_text_addr(ovl_text_addr),
        .ovl_text_wrdata(ovl_text_wrdata),
        .ovl_text_wren(ovl_text_wren),

        .ovl_font_addr(ovl_font_addr),
        .ovl_font_wrdata(ovl_font_wrdata),
        .ovl_font_wren(ovl_font_wren),

        .ovl_palette_addr(ovl_palette_addr),
        .ovl_palette_wrdata(ovl_palette_wrdata),
        .ovl_palette_wren(ovl_palette_wren),

        // VGA signals
        .vga_r(vga_r),
        .vga_g(vga_g),
        .vga_b(vga_b),
        .vga_hsync(vga_hsync),
        .vga_vsync(vga_vsync)
    );

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    assign hc1[7:0] = 8'bZ;
    assign hc2[7:0] = 8'bZ;
    assign hc1[8]   = 1'b0;
    assign hc2[8]   = 1'b0;

    wire [7:0] spi_hctrl1, spi_hctrl2;

    // Synchronize inputs
    reg [7:0] q_hctrl1, q2_hctrl1;
    reg [7:0] q_hctrl2, q2_hctrl2;
    always @(posedge clk_25_175) q_hctrl1  <= hc1[7:0];
    always @(posedge clk_25_175) q2_hctrl1 <= q_hctrl1;
    always @(posedge clk_25_175) q_hctrl2  <= hc2[7:0];
    always @(posedge clk_25_175) q2_hctrl2 <= q_hctrl2;

    // Combine data from ESP with data from handcontroller input
    wire [7:0] hctrl1 = q2_hctrl1 & spi_hctrl1;
    wire [7:0] hctrl2 = q2_hctrl2 & spi_hctrl2;

    //////////////////////////////////////////////////////////////////////////
    // Audio DAC
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] audio_sample_l;
    wire [15:0] audio_sample_r;

    fpga_dac fpga_dac(
        .clk(clk_25_175),

        // Sample input
        .left_data(audio_sample_l),
        .right_data(audio_sample_r),

        // PWM audio output
        .audio_l(audio_l),
        .audio_r(audio_r));

    //////////////////////////////////////////////////////////////////////////
    // ESP32 SPI interface (also used for loading FPGA image)
    //////////////////////////////////////////////////////////////////////////
    wire   [7:0] core_type;
    wire   [7:0] core_flags;
    wire  [15:0] core_version;
    wire [127:0] core_name;

    wire         spi_msg_end;
    wire   [7:0] spi_cmd;
    wire  [63:0] spi_rxdata;
    wire  [63:0] spi_txdata;
    wire         spi_txdata_valid;

    wire  [63:0] keys;
    wire  [63:0] gamepad1;
    wire  [63:0] gamepad2;
    wire  [15:0] kbbuf16_wrdata;
    wire         kbbuf16_wren;

    assign spi_txdata       = 64'b0;
    assign spi_txdata_valid = 1'b0;

    fpga_spislave fpga_spislave(
        .clk(clk_25_175),
        .reset(reset_25_175),

        // System information
        .core_type(core_type),
        .core_flags(core_flags),
        .core_version(core_version),
        .core_name(core_name),

        // Interface for core specific messages
        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        .spi_txdata(spi_txdata),
        .spi_txdata_valid(spi_txdata_valid),

        // Display overlay interface
        .ovl_text_addr(ovl_text_addr),
        .ovl_text_wrdata(ovl_text_wrdata),
        .ovl_text_wren(ovl_text_wren),

        .ovl_font_addr(ovl_font_addr),
        .ovl_font_wrdata(ovl_font_wrdata),
        .ovl_font_wren(ovl_font_wren),

        .ovl_palette_addr(ovl_palette_addr),
        .ovl_palette_wrdata(ovl_palette_wrdata),
        .ovl_palette_wren(ovl_palette_wren),

        // Command signals
        .reset_req(spi_reset_req),
        .hctrl1(spi_hctrl1),
        .hctrl2(spi_hctrl2),
        .keys(keys),
        .gamepad1(gamepad1),
        .gamepad2(gamepad2),
        .kbbuf16_wrdata(kbbuf16_wrdata),
        .kbbuf16_wren(kbbuf16_wren),

        // ESP SPI slave interface
        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),
        .esp_notify(esp_notify));

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] uart_txfifo_data;
    wire       uart_txfifo_wren;
    wire       uart_txfifo_full;
    wire [8:0] uart_rxfifo_data;
    wire       uart_rxfifo_rden;
    wire       uart_rxfifo_empty;

    fpga_uart fpga_uart(
        .clk(clk_25_175),
        .reset(reset_25_175),

        .txfifo_data(uart_txfifo_data),
        .txfifo_wren(uart_txfifo_wren),
        .txfifo_full(uart_txfifo_full),

        .rxfifo_data(uart_rxfifo_data),
        .rxfifo_rden(uart_rxfifo_rden),
        .rxfifo_empty(uart_rxfifo_empty),

        .esp_rx(esp_rx),
        .esp_tx(esp_tx),
        .esp_cts(esp_cts),
        .esp_rts(esp_rts));

    //////////////////////////////////////////////////////////////////////////
    // FPGA core
    //////////////////////////////////////////////////////////////////////////
    fpga_core fpga_core(
        .clk_25_175(clk_25_175),
        .reset_25_175(reset_25_175),

        .clk_28_63636(clk_28_63636),

        // Core information
        .core_type(core_type),
        .core_flags(core_flags),
        .core_version(core_version),
        .core_name(core_name),

        // Interface for core specific messages
        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        .spi_txdata(spi_txdata),
        .spi_txdata_valid(spi_txdata_valid),

        // Memory interface
        .sram_a({ebus_ba, ebus_a[13:0]}),
        .sram_ce_n(ebus_ram_ce_n),
        .sram_oe_n(ebus_rd_n),
        .sram_we_n(ebus_ram_we_n),
        .sram_dq(ebus_d),

        // Video output
        .video_hpos(video_hpos),
        .video_hlast(video_hlast),
        .video_vpos(video_vpos),
        .video_vlast(video_vlast),
        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),

        // Audio outputs
        .audio_l(audio_sample_l),
        .audio_r(audio_sample_r),

        // Input peripherals
        .hctrl1(hctrl1),
        .hctrl2(hctrl2),
        .keys(keys),
        .gamepad1(gamepad1),
        .gamepad2(gamepad2),
        .kbbuf16_wrdata(kbbuf16_wrdata),
        .kbbuf16_wren(kbbuf16_wren),

        // ESP32 serial interface
        .uart_txfifo_data(uart_txfifo_data),
        .uart_txfifo_wren(uart_txfifo_wren),
        .uart_txfifo_full(uart_txfifo_full),
        .uart_rxfifo_data(uart_rxfifo_data),
        .uart_rxfifo_rden(uart_rxfifo_rden),
        .uart_rxfifo_empty(uart_rxfifo_empty)
    );

endmodule
