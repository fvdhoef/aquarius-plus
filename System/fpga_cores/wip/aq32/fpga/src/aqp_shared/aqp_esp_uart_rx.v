`default_nettype none
`timescale 1 ns / 1 ps

module aqp_esp_uart_rx(
    input  wire        clk,
    input  wire        reset,

    input  wire        uart_rxd,

    output reg  [7:0]  rx_data,
    output reg         rx_valid);

    reg q_started;
    reg q_framing_error;

    // Synchronize input signal
    reg [3:0] q_rxd;
    always @(posedge clk) q_rxd <= {q_rxd[2:0], uart_rxd};
    wire rx_in           = q_rxd[2];
    wire start_condition = (q_rxd[3:2] == 'b10);

    // Bit-timing
    reg [2:0] q_clk_cnt;
    always @(posedge clk)
        if (!q_started || q_clk_cnt == 3'd5)
            q_clk_cnt <= 0;
        else
            q_clk_cnt <= q_clk_cnt + 3'd1;

    // Receive logic
    reg [3:0] q_bit_cnt;
    reg [7:0] q_shift;
    always @(posedge clk or posedge reset)
        if (reset) begin
            q_started       <= 1'b0;
            rx_valid        <= 1'b0;
            q_shift         <= 8'b0;
            rx_data         <= 8'b0;
            q_bit_cnt       <= 4'b0;
            q_framing_error <= 1'b0;

        end else begin
            rx_valid <= 0;

            if (!q_started) begin
                q_bit_cnt       <= 4'd0;
                q_framing_error <= 1'b0;

                if (start_condition)
                    q_started <= 1;

            end else if (q_clk_cnt == 3'd2) begin
                if (q_bit_cnt == 4'd9) begin
                    if (rx_in) begin
                        q_started <= 0;
                        if (!(q_framing_error)) begin
                            rx_data  <= q_shift;
                            rx_valid <= 1;
                        end
                    end else begin
                        q_framing_error <= 1'b1;
                    end
                end else begin
                    q_shift   <= {rx_in, q_shift[7:1]};
                    q_bit_cnt <= q_bit_cnt + 4'd1;
                end
            end
        end

endmodule
