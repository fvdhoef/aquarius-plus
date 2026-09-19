`default_nettype none
`timescale 1 ns / 1 ps

module fpga_core(
    input  wire         clk_25_175,
    input  wire         reset_25_175,

    input  wire         clk_28_63636,

    // Core information
    output wire   [7:0] core_type,
    output wire   [7:0] core_flags,
    output wire  [15:0] core_version,
    output wire [127:0] core_name,

    // Interface for core specific messages
    input  wire         spi_msg_end,
    input  wire   [7:0] spi_cmd,
    input  wire  [63:0] spi_rxdata,
    output wire  [63:0] spi_txdata,
    output wire         spi_txdata_valid,

    // Memory interface
    output wire  [18:0] sram_a,
    output wire         sram_ce_n,
    output wire         sram_oe_n,
    output wire         sram_we_n,
    inout  wire   [7:0] sram_dq,

    // Video output
    input  wire   [9:0] video_hpos,
    input  wire         video_hlast,
    input  wire   [9:0] video_vpos,
    input  wire         video_vlast,
    output wire   [3:0] video_r,
    output wire   [3:0] video_g,
    output wire   [3:0] video_b,

    // Audio outputs (signed 16-bits)s
    output wire  [15:0] audio_l,
    output wire  [15:0] audio_r,

    // Input peripherals
    input  wire   [7:0] hctrl1,
    input  wire   [7:0] hctrl2,
    input  wire  [63:0] keys,
    input  wire  [63:0] gamepad1,
    input  wire  [63:0] gamepad2,
    input  wire  [15:0] kbbuf16_wrdata,
    input  wire         kbbuf16_wren,

    // ESP32 UART
    output wire   [8:0] uart_txfifo_data,
    output wire         uart_txfifo_wren,
    input  wire         uart_txfifo_full,
    input  wire   [8:0] uart_rxfifo_data,
    output wire         uart_rxfifo_rden,
    input  wire         uart_rxfifo_empty
);

    wire clk   = clk_25_175;
    wire reset = reset_25_175;

    assign spi_txdata       = 0;
    assign spi_txdata_valid = 0;

    assign uart_txfifo_data = 0;
    assign uart_txfifo_wren = 0;
    assign uart_rxfifo_rden = 0;

    //////////////////////////////////////////////////////////////////////////
    // Core information
    //////////////////////////////////////////////////////////////////////////
    assign core_type    = 8'h02;
    assign core_flags   = 8'h00;
    assign core_version = {8'd0, 8'd01};
    assign core_name    = "My32            ";

    //////////////////////////////////////////////////////////////////////////
    // Memory
    //////////////////////////////////////////////////////////////////////////
    wire [18:0] mem_addr;
    wire  [7:0] mem_wrdata = 0;
    wire        mem_wren = 0;
    wire        mem_strobe;
    wire        mem_wait;
    wire  [7:0] mem_rddata;
    wire        mem_rddata_valid;

    sram_ctrl sram_ctrl(
        .clk(clk_25_175),
        .reset(reset),

        // Memory interface (pipelined)
        .mem_addr(mem_addr),
        .mem_wrdata(mem_wrdata),
        .mem_wren(mem_wren),
        .mem_strobe(mem_strobe),
        .mem_wait(mem_wait),
        .mem_rddata(mem_rddata),
        .mem_rddata_valid(mem_rddata_valid),

        // SRAM memory interface
        .sram_a(sram_a),
        .sram_oe_n(sram_oe_n),
        .sram_ce_n(sram_ce_n),
        .sram_we_n(sram_we_n),
        .sram_dq(sram_dq)
    );

    //////////////////////////////////////////////////////////////////////////
    // Video
    //////////////////////////////////////////////////////////////////////////

    // Palette interface
    wire   [7:0] palette_idx    = 0;
    wire  [11:0] palette_wrdata = 0;
    wire         palette_wren   = 0;
    wire  [11:0] palette_rddata;

    // Register interface
    wire         video_mode = 0;
    wire  [18:0] base_addr = 0;

    wire  [18:0] next_line_addr_wrdata = 0;
    wire         next_line_addr_wren = 0;
    wire  [18:0] next_line_addr_rddata;

    video video(
        .clk(clk_25_175),
        .reset(reset),

        // Palette interface
        .palette_idx(palette_idx),
        .palette_wrdata(palette_wrdata),
        .palette_wren(palette_wren),
        .palette_rddata(palette_rddata),

        // Register interface
        .video_mode(video_mode),
        .base_addr(base_addr),

        .next_line_addr_wrdata(next_line_addr_wrdata),
        .next_line_addr_wren(next_line_addr_wren),
        .next_line_addr_rddata(next_line_addr_rddata),

        // Memory interface (pipelined)
        .mem_addr(mem_addr),
        .mem_strobe(mem_strobe),
        .mem_wait(mem_wait),
        .mem_rddata(mem_rddata),
        .mem_rddata_valid(mem_rddata_valid),

        // Video output
        .video_hpos(video_hpos),
        .video_hlast(video_hlast),
        .video_vpos(video_vpos),
        .video_vlast(video_vlast),
        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b)
    );

    //////////////////////////////////////////////////////////////////////////
    // Audio
    //////////////////////////////////////////////////////////////////////////
    assign audio_l = 0;
    assign audio_r = 0;

endmodule
