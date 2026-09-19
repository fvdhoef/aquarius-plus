`default_nettype none
`timescale 1 ns / 1 ps

module cache_ram(
    input  wire        clk,
    input  wire [10:0] addr,
    input  wire [31:0] wrdata,
    input  wire  [3:0] wrsel, 
    input  wire        wren,
    output wire [31:0] rddata
);

    wire [3:0] dop;
    RAMB16_S9 ram_7_0  (.CLK(clk), .SSR(1'b0), .ADDR(addr), .DO(rddata[ 7: 0]), .DOP(dop[0]), .DI(wrdata[ 7: 0]), .DIP(1'b0), .EN(1'b1), .WE(wren && wrsel[0]));
    RAMB16_S9 ram_15_8 (.CLK(clk), .SSR(1'b0), .ADDR(addr), .DO(rddata[15: 8]), .DOP(dop[1]), .DI(wrdata[15: 8]), .DIP(1'b0), .EN(1'b1), .WE(wren && wrsel[1]));
    RAMB16_S9 ram_23_16(.CLK(clk), .SSR(1'b0), .ADDR(addr), .DO(rddata[23:16]), .DOP(dop[2]), .DI(wrdata[23:16]), .DIP(1'b0), .EN(1'b1), .WE(wren && wrsel[2]));
    RAMB16_S9 ram_31_24(.CLK(clk), .SSR(1'b0), .ADDR(addr), .DO(rddata[31:24]), .DOP(dop[3]), .DI(wrdata[31:24]), .DIP(1'b0), .EN(1'b1), .WE(wren && wrsel[3]));

endmodule
