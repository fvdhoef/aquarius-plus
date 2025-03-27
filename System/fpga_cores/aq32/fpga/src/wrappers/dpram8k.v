`default_nettype none

module dpram8k(
    input  wire        a_clk,
    input  wire [10:0] a_addr,
    input  wire [31:0] a_wrdata,
    input  wire  [3:0] a_wrsel, 
    input  wire        a_wren,
    output wire [31:0] a_rddata,

    input  wire        b_clk,
    input  wire [10:0] b_addr,
    input  wire [31:0] b_wrdata,
    input  wire  [3:0] b_wrsel, 
    input  wire        b_wren,
    output wire [31:0] b_rddata
);

    wire [3:0] dopa, dopb;

    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram_7_0(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[ 7: 0]), .DOPA(dopa[0]), .DIA(a_wrdata[ 7: 0]), .DIPA(1'b0), .ENA(1'b1), .WEA(a_wren && a_wrsel[0]),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 7: 0]), .DOPB(dopb[0]), .DIB(b_wrdata[ 7: 0]), .DIPB(1'b0), .ENB(1'b1), .WEB(b_wren && b_wrsel[0]));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram_15_8(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[15: 8]), .DOPA(dopa[1]), .DIA(a_wrdata[15: 8]), .DIPA(1'b0), .ENA(1'b1), .WEA(a_wren && a_wrsel[1]),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[15: 8]), .DOPB(dopb[1]), .DIB(b_wrdata[15: 8]), .DIPB(1'b0), .ENB(1'b1), .WEB(b_wren && b_wrsel[1]));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram_23_16(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[23:16]), .DOPA(dopa[2]), .DIA(a_wrdata[23:16]), .DIPA(1'b0), .ENA(1'b1), .WEA(a_wren && a_wrsel[2]),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[23:16]), .DOPB(dopb[2]), .DIB(b_wrdata[23:16]), .DIPB(1'b0), .ENB(1'b1), .WEB(b_wren && b_wrsel[2]));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram_31_24(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOA(a_rddata[31:24]), .DOPA(dopa[3]), .DIA(a_wrdata[31:24]), .DIPA(1'b0), .ENA(1'b1), .WEA(a_wren && a_wrsel[3]),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[31:24]), .DOPB(dopb[3]), .DIB(b_wrdata[31:24]), .DIPB(1'b0), .ENB(1'b1), .WEB(b_wren && b_wrsel[3]));

endmodule
