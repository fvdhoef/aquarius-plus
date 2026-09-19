// IS61LV5128AL Asynchronous SRAM, 512K x 8; speed: 10ns.

`default_nettype none
`timescale 1 ns / 1 ps

`define OEb

module is61lv5128al(
    input wire [18:0] A,
    inout wire  [7:0] IO,
    input wire        CE_n,
    input wire        OE_n,
    input wire        WE_n
);

    parameter Toha  = 2;
    parameter Tsa   = 0;
    parameter Taa   = 10;
    parameter Thzce = 4;
    parameter Thzwe = 5;

    reg  [7:0] mem [0:524287];

    wire [7:0] dout = mem[A];
    wire       r_en =  WE_n && !CE_n && !OE_n;     // WE=1, CE=0, OE=0  Read
    wire       w_en = !WE_n && !CE_n;             // WE=0, CE=0  OE=x  Write

    assign #(r_en ? Taa : Thzce) IO = r_en ? dout : 8'bZ;

    initial $timeformat(-9, 0.1, " ns", 10); // show current simulation time

    always @(A or w_en) begin
        #Tsa    // address setup time
        if (w_en) #Thzwe mem[A] = IO;
    end

    // Timing Check
    specify specparam   // specify delay
        tSA   = 0,
        tAW   = 8,
        tSCE  = 8,
        tSD   = 6,
        tPWE2 = 10,
        tPWE1 = 8;

    $setup(A,  negedge CE_n, tSA);
    $setup(A,  posedge CE_n, tAW);
    $setup(IO, posedge CE_n, tSD);
    $setup(A,  negedge WE_n, tSA);
    $setup(IO, posedge WE_n, tSD);

    $width(negedge CE_n, tSCE);

`ifdef OEb
    $width(negedge WE_n, tPWE1);
`else
    $width(negedge WE_n, tPWE2);
`endif

    endspecify

endmodule
