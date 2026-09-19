`default_nettype none
`timescale 1 ns / 1 ps

module linebuf(
    input  wire        clk,

    input  wire        linesel,

    input  wire  [8:0] idx1,
    input  wire  [7:0] wrdata1,
    input  wire        wren1,

    input  wire  [8:0] idx2,
    output wire  [7:0] rddata2);

    wire [10:0] addr1 = {1'b0,  linesel, idx1};
    wire [10:0] addr2 = {1'b0, !linesel, idx2};
    wire  [7:0] rddata1;
    wire  [0:0] dopa, dopb; // unused

    RAMB16_S9_S9 RAMB16_S9_S9_inst(
        .CLKA(clk),
        .SSRA(1'b0),
        .ADDRA(addr1),
        .DOA(rddata1),
        .DOPA(dopa),
        .DIA(wrdata1),
        .DIPA(1'b0),
        .ENA(1'b1),
        .WEA(wren1),

        .CLKB(clk),
        .SSRB(1'b0),
        .ADDRB(addr2),
        .DOB(rddata2),
        .DOPB(dopb),
        .DIB(8'b0),
        .DIPB(1'b0),
        .ENB(1'b1),
        .WEB(1'b0)
    );

endmodule
