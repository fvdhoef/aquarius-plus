`default_nettype none
`timescale 1 ns / 1 ps

module aqp_ovl_font_ram(
    input  wire        wrclk,
    input  wire [10:0] wraddr,
    input  wire  [7:0] wrdata,
    input  wire        wren,

    input  wire        rdclk,
    input  wire [10:0] rdaddr,
    output reg   [7:0] rddata);

    reg [7:0] mem [0:2047];

    always @(posedge wrclk) if (wren) mem[wraddr] <= wrdata;
    always @(posedge rdclk) rddata <= mem[rdaddr];

endmodule
