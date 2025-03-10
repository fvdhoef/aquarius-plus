`timescale  1 ps / 1 ps

module RAM16X1D(
    // Read-only port
    input  wire DPRA0,
    input  wire DPRA1,
    input  wire DPRA2,
    input  wire DPRA3,
    output wire DPO,

    // Read-write port
    input  wire A0,
    input  wire A1,
    input  wire A2,
    input  wire A3,
    output wire SPO,
    input  wire WCLK,
    input  wire D,
    input  wire WE);

    parameter INIT = 16'h0000;

endmodule
