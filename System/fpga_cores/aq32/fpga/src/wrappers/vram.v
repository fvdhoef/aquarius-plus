`default_nettype none
`timescale 1 ns / 1 ps

module vram(
    // First port - CPU access
    input  wire        p1_clk,
    input  wire [11:0] p1_addr,
    output wire [31:0] p1_rddata,
    input  wire [31:0] p1_wrdata,
    input  wire  [3:0] p1_bytesel,
    input  wire        p1_wren,

    // Second port - Video access
    input  wire        p2_clk,
    input  wire [12:0] p2_addr,
    output reg  [15:0] p2_rddata);

    wire [31:0] p1_ram0_rddata, p1_ram1_rddata;
    wire [31:0] p2_ram0_rddata, p2_ram1_rddata;

    reg q_p1_sel;
    always @(p1_clk) q_p1_sel <= p1_addr[11];

    reg [1:0] q_p2_sel;
    always @(p2_clk) q_p2_sel <= {p2_addr[12], p2_addr[0]};

    assign p1_rddata = !q_p1_sel ? p1_ram0_rddata : p1_ram1_rddata;

    always @*
        case (q_p2_sel)
            2'b00: p2_rddata = p2_ram0_rddata[15:0];
            2'b01: p2_rddata = p2_ram0_rddata[31:16];
            2'b10: p2_rddata = p2_ram1_rddata[15:0];
            2'b11: p2_rddata = p2_ram1_rddata[31:16];
        endcase

    wire [7:0] dopa, dopb;

    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram0_0(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram0_rddata[ 7: 0]), .DOPA(dopa[0]), .DIA(p1_wrdata[ 7: 0]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] && !p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram0_rddata[ 7: 0]), .DOPB(dopb[0]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram0_1(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram0_rddata[15: 8]), .DOPA(dopa[1]), .DIA(p1_wrdata[15: 8]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] && !p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram0_rddata[15: 8]), .DOPB(dopb[1]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram0_2(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram0_rddata[23:16]), .DOPA(dopa[2]), .DIA(p1_wrdata[23:16]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] && !p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram0_rddata[23:16]), .DOPB(dopb[2]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram0_3(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram0_rddata[31:24]), .DOPA(dopa[3]), .DIA(p1_wrdata[31:24]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] && !p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram0_rddata[31:24]), .DOPB(dopb[3]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));

    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram1_0(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram1_rddata[ 7: 0]), .DOPA(dopa[4]), .DIA(p1_wrdata[ 7: 0]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] &&  p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram1_rddata[ 7: 0]), .DOPB(dopb[4]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram1_1(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram1_rddata[15: 8]), .DOPA(dopa[5]), .DIA(p1_wrdata[15: 8]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] &&  p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram1_rddata[15: 8]), .DOPB(dopb[5]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram1_2(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram1_rddata[23:16]), .DOPA(dopa[6]), .DIA(p1_wrdata[23:16]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] &&  p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram1_rddata[23:16]), .DOPB(dopb[6]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));
    RAMB16_S9_S9 #(.SIM_COLLISION_CHECK("NONE")) ram1_3(
        .CLKA(p1_clk), .SSRA(1'b0), .ADDRA(p1_addr[10:0]), .DOA(p1_ram1_rddata[31:24]), .DOPA(dopa[7]), .DIA(p1_wrdata[31:24]), .DIPA(1'b0), .ENA(1'b1), .WEA(p1_wren && p1_bytesel[0] &&  p1_addr[11]),
        .CLKB(p2_clk), .SSRB(1'b0), .ADDRB(p2_addr[11:1]), .DOB(p2_ram1_rddata[31:24]), .DOPB(dopb[7]), .DIB(8'b0),             .DIPB(1'b0), .ENB(1'b1), .WEB(1'b0));

endmodule
