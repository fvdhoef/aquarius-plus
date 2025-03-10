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
    output reg         reset_req_cold,
    output reg  [63:0] keys,

    output reg   [7:0] kbbuf_data,
    output reg         kbbuf_wren);

    assign spi_txdata       = 64'b0;
    assign spi_txdata_valid = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Commands
    //////////////////////////////////////////////////////////////////////////
    localparam
        CMD_RESET           = 8'h01,
        CMD_SET_KEYB_MATRIX = 8'h10,
        CMD_WRITE_KBBUF     = 8'h12;

    // 01h: Reset command
    always @(posedge clk) begin
        reset_req      <= 1'b0;
        reset_req_cold <= 1'b0;
        if (spi_cmd == CMD_RESET && spi_msg_end) begin
            reset_req      <= 1'b1;
            reset_req_cold <= spi_rxdata[57];
            // q_use_t80      <= spi_rxdata[56];
        end
    end

    // 10h: Set keyboard matrix
    always @(posedge clk or posedge reset)
        if (reset)
            keys <= 64'hFFFFFFFFFFFFFFFF;
        else if (spi_cmd == CMD_SET_KEYB_MATRIX && spi_msg_end)
            keys <= spi_rxdata;

    // 12h: Write keyboard buffer
    always @(posedge clk or posedge reset)
        if (reset) begin
            kbbuf_data <= 8'h00;
            kbbuf_wren <= 1'b0;
        end else begin
            kbbuf_wren <= 1'b0;
            if (spi_cmd == CMD_WRITE_KBBUF && spi_msg_end) begin
                kbbuf_data <= spi_rxdata[63:56];
                kbbuf_wren <= 1'b1;
            end
        end

endmodule
