`default_nettype none
`timescale 1 ns / 1 ps

module dpram32k(
    input  wire        a_clk,
    input  wire [12:0] a_addr,
    input  wire [31:0] a_wrdata,
    input  wire  [7:0] a_wrsel,
    input  wire        a_wren,
    output wire [31:0] a_rddata,

    input  wire        b_clk,
    input  wire [12:0] b_addr,
    input  wire [31:0] b_wrdata,
    input  wire  [7:0] b_wrsel,
    input  wire        b_wren,
    output wire [31:0] b_rddata
);

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram0(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[ 1: 0]), .DIA(a_wrdata[ 1: 0]), .ENA(1'b1), .WEA(a_wrsel[0] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 1: 0]), .DIB(b_wrdata[ 1: 0]), .ENB(1'b1), .WEB(b_wrsel[0] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram1(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[ 3: 2]), .DIA(a_wrdata[ 3: 2]), .ENA(1'b1), .WEA(a_wrsel[0] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 3: 2]), .DIB(b_wrdata[ 3: 2]), .ENB(1'b1), .WEB(b_wrsel[0] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram2(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[ 5: 4]), .DIA(a_wrdata[ 5: 4]), .ENA(1'b1), .WEA(a_wrsel[1] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 5: 4]), .DIB(b_wrdata[ 5: 4]), .ENB(1'b1), .WEB(b_wrsel[1] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram3(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[ 7: 6]), .DIA(a_wrdata[ 7: 6]), .ENA(1'b1), .WEA(a_wrsel[1] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 7: 6]), .DIB(b_wrdata[ 7: 6]), .ENB(1'b1), .WEB(b_wrsel[1] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram4(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[ 9: 8]), .DIA(a_wrdata[ 9: 8]), .ENA(1'b1), .WEA(a_wrsel[2] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 9: 8]), .DIB(b_wrdata[ 9: 8]), .ENB(1'b1), .WEB(b_wrsel[2] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram5(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[11:10]), .DIA(a_wrdata[11:10]), .ENA(1'b1), .WEA(a_wrsel[2] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[11:10]), .DIB(b_wrdata[11:10]), .ENB(1'b1), .WEB(b_wrsel[2] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram6(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[13:12]), .DIA(a_wrdata[13:12]), .ENA(1'b1), .WEA(a_wrsel[3] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[13:12]), .DIB(b_wrdata[13:12]), .ENB(1'b1), .WEB(b_wrsel[3] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram7(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[15:14]), .DIA(a_wrdata[15:14]), .ENA(1'b1), .WEA(a_wrsel[3] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[15:14]), .DIB(b_wrdata[15:14]), .ENB(1'b1), .WEB(b_wrsel[3] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram8(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[17:16]), .DIA(a_wrdata[17:16]), .ENA(1'b1), .WEA(a_wrsel[4] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[17:16]), .DIB(b_wrdata[17:16]), .ENB(1'b1), .WEB(b_wrsel[4] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram9(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[19:18]), .DIA(a_wrdata[19:18]), .ENA(1'b1), .WEA(a_wrsel[4] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[19:18]), .DIB(b_wrdata[19:18]), .ENB(1'b1), .WEB(b_wrsel[4] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram10(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[21:20]), .DIA(a_wrdata[21:20]), .ENA(1'b1), .WEA(a_wrsel[5] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[21:20]), .DIB(b_wrdata[21:20]), .ENB(1'b1), .WEB(b_wrsel[5] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram11(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[23:22]), .DIA(a_wrdata[23:22]), .ENA(1'b1), .WEA(a_wrsel[5] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[23:22]), .DIB(b_wrdata[23:22]), .ENB(1'b1), .WEB(b_wrsel[5] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram12(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[25:24]), .DIA(a_wrdata[25:24]), .ENA(1'b1), .WEA(a_wrsel[6] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[25:24]), .DIB(b_wrdata[25:24]), .ENB(1'b1), .WEB(b_wrsel[6] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram13(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[27:26]), .DIA(a_wrdata[27:26]), .ENA(1'b1), .WEA(a_wrsel[6] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[27:26]), .DIB(b_wrdata[27:26]), .ENB(1'b1), .WEB(b_wrsel[6] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram14(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[29:28]), .DIA(a_wrdata[29:28]), .ENA(1'b1), .WEA(a_wrsel[7] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[29:28]), .DIB(b_wrdata[29:28]), .ENB(1'b1), .WEB(b_wrsel[7] && b_wren));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram15(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[31:30]), .DIA(a_wrdata[31:30]), .ENA(1'b1), .WEA(a_wrsel[7] && a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[31:30]), .DIB(b_wrdata[31:30]), .ENB(1'b1), .WEB(b_wrsel[7] && b_wren));

endmodule
