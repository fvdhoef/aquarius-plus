`default_nettype none
`timescale 1 ns / 1 ps

module reset_sync(
    input  wire async_rst_in,
    input  wire clk,
    output reg  reset_out);

    reg [1:0] dff;
    always @(posedge clk or posedge async_rst_in)
        if (async_rst_in)
            {reset_out, dff} <= 3'b111;
        else
            {reset_out, dff} <= {dff, 1'b0};

endmodule
