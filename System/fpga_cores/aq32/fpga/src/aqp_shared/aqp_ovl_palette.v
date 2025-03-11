`default_nettype none
`timescale 1 ns / 1 ps

module aqp_ovl_palette(
    input  wire        wrclk,
    input  wire  [3:0] wraddr,
    input  wire [15:0] wrdata,
    input  wire        wren,

    input  wire  [3:0] rdaddr,
    output wire [15:0] rddata);

    reg [15:0] mem [0:15];

    always @(posedge wrclk)
        if (wren) mem[wraddr] <= wrdata;

    assign rddata = mem[rdaddr];

endmodule
