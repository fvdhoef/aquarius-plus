`default_nettype none
`timescale 1 ns / 1 ps

module distram256d#(
    parameter WIDTH = 32
) (
    input  wire               clk,
    input  wire         [7:0] a_addr,
    output wire [(WIDTH-1):0] a_rddata,
    input  wire [(WIDTH-1):0] a_wrdata,
    input  wire [(WIDTH-1):0] a_wren,
    input  wire         [7:0] b_addr,
    output wire [(WIDTH-1):0] b_rddata
);

    wire [(WIDTH-1):0] a_rddata0, b_rddata0, a_rddata1, b_rddata1;

    distram128d #(.WIDTH(WIDTH)) ram0(
        .clk(clk),
        .a_addr(a_addr[6:0]),
        .a_rddata(a_rddata0),
        .a_wrdata(a_wrdata),
        .a_wren({WIDTH{!a_addr[7]}} & a_wren),
        .b_addr(b_addr[6:0]),
        .b_rddata(b_rddata0));

    distram128d #(.WIDTH(WIDTH)) ram1(
        .clk(clk),
        .a_addr(a_addr[6:0]),
        .a_rddata(a_rddata1),
        .a_wrdata(a_wrdata),
        .a_wren({WIDTH{a_addr[7]}} & a_wren),
        .b_addr(b_addr[6:0]),
        .b_rddata(b_rddata1));

    assign a_rddata = a_addr[7] ? a_rddata1 : a_rddata0;
    assign b_rddata = b_addr[7] ? b_rddata1 : b_rddata0;

endmodule
