`default_nettype none
`timescale 1 ns / 1 ps

module vram(
    input  wire        clk,

    input  wire [2:0]  a_offset,

    input  wire [12:0] a_addr,
    input  wire [31:0] a_wrdata,
    input  wire  [7:0] a_wrsel,
    input  wire        a_wren,
    output reg  [31:0] a_rddata,

    input  wire [12:0] b_addr,
    output wire [31:0] b_rddata
);

    reg  [31:0] wrdata;
    reg   [7:0] wrsel;
    wire [31:0] rddata;
    wire [12:0] addr_plus1 = a_addr + 13'd1;
    reg   [7:0] addr_sel;

    always @* case (a_offset)
        3'd0: begin wrdata = {a_wrdata                       }; wrsel = {a_wrsel                   }; a_rddata = {rddata                     }; addr_sel = 8'b00000000; end
        3'd1: begin wrdata = {a_wrdata[27:0], a_wrdata[31:28]}; wrsel = {a_wrsel[6:0], a_wrsel[7]  }; a_rddata = {rddata[27:0], rddata[31:28]}; addr_sel = 8'b00000001; end
        3'd2: begin wrdata = {a_wrdata[23:0], a_wrdata[31:24]}; wrsel = {a_wrsel[5:0], a_wrsel[7:6]}; a_rddata = {rddata[23:0], rddata[31:24]}; addr_sel = 8'b00000011; end
        3'd3: begin wrdata = {a_wrdata[19:0], a_wrdata[31:20]}; wrsel = {a_wrsel[4:0], a_wrsel[7:5]}; a_rddata = {rddata[19:0], rddata[31:20]}; addr_sel = 8'b00000111; end
        3'd4: begin wrdata = {a_wrdata[15:0], a_wrdata[31:16]}; wrsel = {a_wrsel[3:0], a_wrsel[7:4]}; a_rddata = {rddata[15:0], rddata[31:16]}; addr_sel = 8'b00001111; end
        3'd5: begin wrdata = {a_wrdata[11:0], a_wrdata[31:12]}; wrsel = {a_wrsel[2:0], a_wrsel[7:3]}; a_rddata = {rddata[11:0], rddata[31:12]}; addr_sel = 8'b00011111; end
        3'd6: begin wrdata = {a_wrdata[ 7:0], a_wrdata[31: 8]}; wrsel = {a_wrsel[1:0], a_wrsel[7:2]}; a_rddata = {rddata[ 7:0], rddata[31: 8]}; addr_sel = 8'b00111111; end
        3'd7: begin wrdata = {a_wrdata[ 3:0], a_wrdata[31: 4]}; wrsel = {a_wrsel[0],   a_wrsel[7:1]}; a_rddata = {rddata[ 3:0], rddata[31: 4]}; addr_sel = 8'b01111111; end
    endcase

    wire [12:0] addr0 = addr_sel[0] ? addr_plus1 : a_addr;
    wire [12:0] addr1 = addr_sel[1] ? addr_plus1 : a_addr;
    wire [12:0] addr2 = addr_sel[2] ? addr_plus1 : a_addr;
    wire [12:0] addr3 = addr_sel[3] ? addr_plus1 : a_addr;
    wire [12:0] addr4 = addr_sel[4] ? addr_plus1 : a_addr;
    wire [12:0] addr5 = addr_sel[5] ? addr_plus1 : a_addr;
    wire [12:0] addr6 = addr_sel[6] ? addr_plus1 : a_addr;
    wire [12:0] addr7 = addr_sel[7] ? addr_plus1 : a_addr;

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram0a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr0),  .DOA(rddata[ 1: 0]),   .DIA(wrdata[ 1: 0]), .ENA(1'b1), .WEA(wrsel[0] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 1: 0]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram0b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr0),  .DOA(rddata[ 3: 2]),   .DIA(wrdata[ 3: 2]), .ENA(1'b1), .WEA(wrsel[0] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 3: 2]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram1a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr1),  .DOA(rddata[ 5: 4]),   .DIA(wrdata[ 5: 4]), .ENA(1'b1), .WEA(wrsel[1] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 5: 4]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram1b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr1),  .DOA(rddata[ 7: 6]),   .DIA(wrdata[ 7: 6]), .ENA(1'b1), .WEA(wrsel[1] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 7: 6]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram2a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr2),  .DOA(rddata[ 9: 8]),   .DIA(wrdata[ 9: 8]), .ENA(1'b1), .WEA(wrsel[2] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[ 9: 8]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram2b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr2),  .DOA(rddata[11:10]),   .DIA(wrdata[11:10]), .ENA(1'b1), .WEA(wrsel[2] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[11:10]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram3a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr3),  .DOA(rddata[13:12]),   .DIA(wrdata[13:12]), .ENA(1'b1), .WEA(wrsel[3] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[13:12]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram3b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr3),  .DOA(rddata[15:14]),   .DIA(wrdata[15:14]), .ENA(1'b1), .WEA(wrsel[3] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[15:14]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram4a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr4),  .DOA(rddata[17:16]),   .DIA(wrdata[17:16]), .ENA(1'b1), .WEA(wrsel[4] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[17:16]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram4b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr4),  .DOA(rddata[19:18]),   .DIA(wrdata[19:18]), .ENA(1'b1), .WEA(wrsel[4] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[19:18]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram5a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr5),  .DOA(rddata[21:20]),   .DIA(wrdata[21:20]), .ENA(1'b1), .WEA(wrsel[5] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[21:20]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram5b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr5),  .DOA(rddata[23:22]),   .DIA(wrdata[23:22]), .ENA(1'b1), .WEA(wrsel[5] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[23:22]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram6a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr6),  .DOA(rddata[25:24]),   .DIA(wrdata[25:24]), .ENA(1'b1), .WEA(wrsel[6] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[25:24]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram6b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr6),  .DOA(rddata[27:26]),   .DIA(wrdata[27:26]), .ENA(1'b1), .WEA(wrsel[6] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[27:26]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram7a(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr7),  .DOA(rddata[29:28]),   .DIA(wrdata[29:28]), .ENA(1'b1), .WEA(wrsel[7] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[29:28]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));
    RAMB16_S2_S2 #(.SIM_COLLISION_CHECK("NONE")) ram7b(
        .CLKA(clk), .SSRA(1'b0), .ADDRA(addr7),  .DOA(rddata[31:30]),   .DIA(wrdata[31:30]), .ENA(1'b1), .WEA(wrsel[7] && a_wren),
        .CLKB(clk), .SSRB(1'b0), .ADDRB(b_addr), .DOB(b_rddata[31:30]), .DIB(2'b0),          .ENB(1'b1), .WEB(1'b0));

endmodule
