`default_nettype none
`timescale 1 ns / 1 ps

module aqp_handctrl(
    input  wire       clk,
    input  wire       reset,

    output wire       hctrl_clk,
    output wire       hctrl_load_n,
    input  wire       hctrl_data,

    output reg  [7:0] hctrl1_data,
    output reg  [7:0] hctrl2_data);

    // Clock divider for hctrl_clk
    reg [7:0] q_clkdiv = 8'd0;
    always @(posedge clk)
        q_clkdiv <= q_clkdiv + 8'd1;

    assign hctrl_clk = q_clkdiv[7];

    // Bit counter
    reg [4:0] q_bitcnt = 5'd0;

    wire do_shift   = (q_clkdiv == 8'd0);
    wire shift_done = (q_bitcnt == 5'd16);

    reg [15:0] q_data;
    always @(posedge clk)
        if (do_shift) begin
            // Shift in data
            q_data <= {q_data[14:0], hctrl_data};

            if (q_bitcnt == 5'd16)
                q_bitcnt <= 5'd0;
            else
                q_bitcnt <= q_bitcnt + 4'd1;
        end

    // Generate shift register LOAD# signal
    assign hctrl_load_n = (q_bitcnt == 5'd0) ? 1'b0 : 1'b1;

    always @(posedge clk)
        if (reset) begin
            hctrl1_data <= 8'hFF;
            hctrl2_data <= 8'hFF;

        end else if (shift_done) begin
            hctrl1_data <= q_data[7:0];
            hctrl2_data <= q_data[15:8];
        end

endmodule
