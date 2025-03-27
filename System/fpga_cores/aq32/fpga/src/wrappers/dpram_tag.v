`default_nettype none

module dpram_tag(
    input  wire        a_clk,
    input  wire [10:0] a_addr,
    input  wire  [8:0] a_wrdata,
    input  wire        a_wren,
    output wire  [8:0] a_rddata,

    input  wire        b_clk,
    input  wire [10:0] b_addr,
    input  wire  [8:0] b_wrdata,
    input  wire        b_wren,
    output wire  [8:0] b_rddata
);

    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram(
        .CLKA(a_clk), .SSRA(1'b0), .ADDRA(a_addr), .DOPA(a_rddata[8]), .DOA(a_rddata[7:0]), .DIPA(a_wrdata[8]), .DIA(a_wrdata[ 7: 0]), .ENA(1'b1), .WEA(a_wren),
        .CLKB(b_clk), .SSRB(1'b0), .ADDRB(b_addr), .DOPB(b_rddata[8]), .DOB(b_rddata[7:0]), .DIPB(a_wrdata[8]), .DIB(b_wrdata[ 7: 0]), .ENB(1'b1), .WEB(b_wren));

endmodule
