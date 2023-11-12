module esp_uart_tx(
    input  wire        clk,
    input  wire        rst,

    output reg         uart_txd,

    input  wire  [7:0] tx_data,
    input  wire        tx_valid,
    input  wire        tx_break,
    output wire        tx_busy);

    // Bit-timing
    reg [2:0] clk_cnt_r = 3'd0;
    always @(posedge clk) clk_cnt_r <= clk_cnt_r + 3'd1;

    wire next_bit = clk_cnt_r == 3'd0;

    // Shift out serial data
    reg [8:0] tx_shift_r;
    reg [3:0] bit_cnt_r;
    reg busy;
    reg uart_txd_r;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            uart_txd_r <= 1'b1;
            busy       <= 1'b0;
            tx_shift_r <= 9'b0;
            bit_cnt_r  <= 4'b0;

        end else begin
            if (!busy) begin
                uart_txd_r <= 1'b1;
                if (tx_valid || tx_break) begin
                    tx_shift_r <= tx_break ? 9'b0 : { tx_data, 1'b0 };
                    busy       <= 1'b1;
                    bit_cnt_r  <= tx_break ? 4'd15 : 4'd9;
                end

            end else if (next_bit) begin
                uart_txd_r <= tx_shift_r[0];

                if (bit_cnt_r == 4'd0) begin
                    busy <= 1'b0;
                    uart_txd_r <= 1'b1;
                end else begin
                    bit_cnt_r <= bit_cnt_r - 4'd1;
                end

                tx_shift_r <= { 1'b0, tx_shift_r[8:1] };
            end
        end
    end

    always @(posedge clk) uart_txd <= uart_txd_r;

    assign tx_busy = busy;

endmodule
