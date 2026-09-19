`default_nettype none
`timescale 1 ns / 1 ps

module aqp_ovl_text_ram(
    input  wire        wrclk,
    input  wire  [9:0] wraddr,
    input  wire [15:0] wrdata,
    input  wire        wren,

    input  wire        rdclk,
    input  wire  [9:0] rdaddr,
    output reg  [15:0] rddata);

    reg [15:0] mem [0:1023];

    always @(posedge wrclk) if (wren) mem[wraddr] <= wrdata;
    always @(posedge rdclk) rddata <= mem[rdaddr];

endmodule
