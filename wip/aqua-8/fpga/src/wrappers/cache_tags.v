`default_nettype none
`timescale 1 ns / 1 ps

module cache_tags(
    input  wire        clk,
    input  wire [10:0] addr,
    input  wire  [8:0] wrdata,
    input  wire        wren,
    output wire  [8:0] rddata
);

    RAMB16_S9 ram(.CLK(clk), .SSR(1'b0), .ADDR(addr), .DOP(rddata[8]), .DO(rddata[7:0]), .DIP(wrdata[8]), .DI(wrdata[ 7: 0]), .EN(1'b1), .WE(wren));

endmodule
