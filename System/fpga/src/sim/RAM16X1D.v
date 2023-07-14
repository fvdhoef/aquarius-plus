// $Header: /devl/xcs/repo/env/Databases/CAEInterfaces/verunilibs/data/unisims/RAM16X1D.v,v 1.8 2005/03/14 22:32:58 yanx Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2004 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 10.1
//  \   \         Description : Xilinx Functional Simulation Library Component
//  /   /                  Static Dual Port Synchronous RAM 16-Deep by 1-Wide
// /___/   /\     Filename : RAM16X1D.v
// \   \  /  \    Timestamp : Thu Mar 25 16:43:33 PST 2004
//  \___\/\___\
//
// Revision:
//    03/23/04 - Initial version.
//    02/04/05 - Rev 0.0.1 Remove input/output bufs; Remove for-loop in initial block;
// End Revision

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

    reg  [15:0] mem;
    wire [3:0] adr;

    assign adr = {A3, A2, A1, A0};
    assign SPO = mem[adr];
    assign DPO = mem[{DPRA3, DPRA2, DPRA1, DPRA0}];

    initial 
        mem = INIT;

    always @(posedge WCLK) 
        if (WE == 1'b1)
            mem[adr] <= #100 D;

endmodule

