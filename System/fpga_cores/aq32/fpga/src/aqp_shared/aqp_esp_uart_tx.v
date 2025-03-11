`default_nettype none
`timescale 1 ns / 1 ps

module aqp_esp_uart_tx(
    input  wire        clk,
    input  wire        reset,

    output reg         uart_txd,

    input  wire  [7:0] tx_data,
    input  wire        tx_valid,
    output wire        tx_busy);

    // Bit-timing
    reg [2:0] q_clk_cnt = 3'd0;
    always @(posedge clk) q_clk_cnt <= q_clk_cnt + 3'd1;

    wire next_bit = q_clk_cnt == 3'd0;

    // Shift out serial data
    reg [8:0] q_tx_shift;
    reg [3:0] q_bit_cnt;
    reg       q_busy;
    reg       q_uart_txd;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_uart_txd <= 1'b1;
            q_busy     <= 1'b0;
            q_tx_shift <= 9'b0;
            q_bit_cnt  <= 4'b0;

        end else begin
            if (!q_busy) begin
                q_uart_txd <= 1'b1;
                if (tx_valid) begin
                    q_tx_shift <= { tx_data, 1'b0 };
                    q_busy     <= 1'b1;
                    q_bit_cnt  <= 4'd9;
                end

            end else if (next_bit) begin
                q_uart_txd <= q_tx_shift[0];

                if (q_bit_cnt == 4'd0) begin
                    q_busy     <= 1'b0;
                    q_uart_txd <= 1'b1;
                end else begin
                    q_bit_cnt  <= q_bit_cnt - 4'd1;
                end

                q_tx_shift <= { 1'b0, q_tx_shift[8:1] };
            end
        end
    end

    always @(posedge clk) uart_txd <= q_uart_txd;

    assign tx_busy = q_busy;

endmodule
