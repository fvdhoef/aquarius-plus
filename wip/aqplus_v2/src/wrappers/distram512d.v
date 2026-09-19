`default_nettype none
`timescale 1 ns / 1 ps

module distram512d#(
    parameter WIDTH = 32
) (
    input  wire               clk,
    input  wire         [8:0] a_addr,
    output reg  [(WIDTH-1):0] a_rddata,
    input  wire [(WIDTH-1):0] a_wrdata,
    input  wire [(WIDTH-1):0] a_wren,
    input  wire         [8:0] b_addr,
    output reg  [(WIDTH-1):0] b_rddata
);

    wire [(WIDTH-1):0] a_rddata0, b_rddata0;
    wire [(WIDTH-1):0] a_rddata1, b_rddata1;
    wire [(WIDTH-1):0] a_rddata2, b_rddata2;
    wire [(WIDTH-1):0] a_rddata3, b_rddata3;

    distram128d #(.WIDTH(WIDTH)) ram0(
        .clk(clk),
        .a_addr(a_addr[6:0]),
        .a_rddata(a_rddata0),
        .a_wrdata(a_wrdata),
        .a_wren({WIDTH{a_addr[8:7] == 2'd0}} & a_wren),
        .b_addr(b_addr[6:0]),
        .b_rddata(b_rddata0));

    distram128d #(.WIDTH(WIDTH)) ram1(
        .clk(clk),
        .a_addr(a_addr[6:0]),
        .a_rddata(a_rddata1),
        .a_wrdata(a_wrdata),
        .a_wren({WIDTH{a_addr[8:7] == 2'd1}} & a_wren),
        .b_addr(b_addr[6:0]),
        .b_rddata(b_rddata1));

    distram128d #(.WIDTH(WIDTH)) ram2(
        .clk(clk),
        .a_addr(a_addr[6:0]),
        .a_rddata(a_rddata2),
        .a_wrdata(a_wrdata),
        .a_wren({WIDTH{a_addr[8:7] == 2'd2}} & a_wren),
        .b_addr(b_addr[6:0]),
        .b_rddata(b_rddata2));

    distram128d #(.WIDTH(WIDTH)) ram3(
        .clk(clk),
        .a_addr(a_addr[6:0]),
        .a_rddata(a_rddata3),
        .a_wrdata(a_wrdata),
        .a_wren({WIDTH{a_addr[8:7] == 2'd3}} & a_wren),
        .b_addr(b_addr[6:0]),
        .b_rddata(b_rddata3));

    always @* case (a_addr[8:7])
        2'd0: a_rddata = a_rddata0;
        2'd1: a_rddata = a_rddata1;
        2'd2: a_rddata = a_rddata2;
        2'd3: a_rddata = a_rddata3;
    endcase

    always @* case (b_addr[8:7])
        2'd0: b_rddata = b_rddata0;
        2'd1: b_rddata = b_rddata1;
        2'd2: b_rddata = b_rddata2;
        2'd3: b_rddata = b_rddata3;
    endcase

endmodule
