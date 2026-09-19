`default_nettype none
`timescale 1 ns / 1 ps

module spiregs(
    input  wire        clk,
    input  wire        reset,

    input  wire        spi_msg_end,
    input  wire  [7:0] spi_cmd,
    input  wire [63:0] spi_rxdata,
    output wire [63:0] spi_txdata,
    output wire        spi_txdata_valid,

    output reg         reset_req,
    output reg  [63:0] keys,
    output reg   [7:0] hctrl1,
    output reg   [7:0] hctrl2,

    input  wire        has_z80,
    output wire        video_mode);

    assign spi_txdata       = 64'b0;
    assign spi_txdata_valid = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Commands
    //////////////////////////////////////////////////////////////////////////
    localparam
        CMD_RESET           = 8'h01,
        CMD_SET_KEYB_MATRIX = 8'h10,
        CMD_SET_HCTRL       = 8'h11,
        CMD_SET_VIDMODE     = 8'h40;

    // 01h: Reset command
    always @(posedge clk) begin
        reset_req <= 0;
        if (spi_cmd == CMD_RESET && spi_msg_end) begin
            reset_req <= 1;
        end
    end

    // 10h: Set keyboard matrix
    always @(posedge clk or posedge reset)
        if (reset)
            keys <= 64'hFFFFFFFFFFFFFFFF;
        else if (spi_cmd == CMD_SET_KEYB_MATRIX && spi_msg_end)
            keys <= spi_rxdata;

    // 11h: Set handcontrollers
    always @(posedge clk or posedge reset)
        if (reset)
            {hctrl2, hctrl1} <= 16'hFFFF;
        else if (spi_cmd == CMD_SET_HCTRL && spi_msg_end)
            {hctrl2, hctrl1} <= spi_rxdata[63:48];

    // 40h: Set video mode
    reg q_video_mode = 1'b0;
    always @(posedge clk) begin
        if (spi_cmd == CMD_SET_VIDMODE && spi_msg_end) begin
            q_video_mode <= spi_rxdata[56];
        end
    end
    assign video_mode = q_video_mode;

endmodule
