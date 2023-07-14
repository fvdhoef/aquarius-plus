module handctrl(
    input  wire       clk,
    input  wire       reset,

    output wire       hctrl_clk,
    output wire       hctrl_load_n,
    input  wire       hctrl_data,

    output reg  [7:0] hctrl1_data,
    output reg  [7:0] hctrl2_data);

    // Clock divider for hctrl_clk
    reg [7:0] clkdiv_r = 8'd0;
    always @(posedge clk)
        clkdiv_r <= clkdiv_r + 8'd1;

    assign hctrl_clk = clkdiv_r[7];

    // Bit counter
    reg [4:0] bitcnt_r = 5'd0;

    wire do_shift = (clkdiv_r == 4'd0);
    wire shift_done = (bitcnt_r == 5'd16);

    reg [15:0] data_r;
    always @(posedge clk)
        if (do_shift) begin
            // Shift in data
            data_r <= {data_r[14:0], hctrl_data};

            if (bitcnt_r == 5'd16)
                bitcnt_r <= 4'd0;
            else
                bitcnt_r <= bitcnt_r + 4'd1;
        end

    // Generate shift register LOAD# signal
    assign hctrl_load_n = (bitcnt_r == 5'd0) ? 1'b0 : 1'b1;

    always @(posedge clk)
        if (reset) begin
            hctrl1_data <= 8'hFF;
            hctrl2_data <= 8'hFF;

        end else if (shift_done) begin
            hctrl1_data <= data_r[7:0];
            hctrl2_data <= data_r[15:8];
        end

endmodule
