module ram64x1d(
    input  wire       a_clk,
    input  wire [5:0] a_addr,
    output reg        a_rddata,
    input  wire       a_wrdata,
    input  wire       a_wren,

    input  wire [5:0] b_addr,
    output reg        b_rddata);

    parameter INIT = 64'b0000;

    wire a_wren0 = a_wren && (a_addr[5:4] == 2'b00);
    wire a_wren1 = a_wren && (a_addr[5:4] == 2'b01);
    wire a_wren2 = a_wren && (a_addr[5:4] == 2'b10);
    wire a_wren3 = a_wren && (a_addr[5:4] == 2'b11);
    wire a_rddata0, a_rddata1, a_rddata2, a_rddata3;
    wire b_rddata0, b_rddata1, b_rddata2, b_rddata3;

    always @* case (a_addr[5:4])
        2'b00: a_rddata <= a_rddata0;
        2'b01: a_rddata <= a_rddata1;
        2'b10: a_rddata <= a_rddata2;
        2'b11: a_rddata <= a_rddata3;
    endcase

    always @* case (b_addr[5:4])
        2'b00: b_rddata <= b_rddata0;
        2'b01: b_rddata <= b_rddata1;
        2'b10: b_rddata <= b_rddata2;
        2'b11: b_rddata <= b_rddata3;
    endcase

    RAM16X1D #(.INIT(INIT[15:0])) ram0(
        .A3(a_addr[3]), .A2(a_addr[2]), .A1(a_addr[1]), .A0(a_addr[0]), .SPO(a_rddata0),
        .WCLK(a_clk), .D(a_wrdata), .WE(a_wren0),
        .DPRA3(b_addr[3]), .DPRA2(b_addr[2]), .DPRA1(b_addr[1]), .DPRA0(b_addr[0]), .DPO(b_rddata0));

    RAM16X1D #(.INIT(INIT[31:16])) ram1(
        .A3(a_addr[3]), .A2(a_addr[2]), .A1(a_addr[1]), .A0(a_addr[0]), .SPO(a_rddata1),
        .WCLK(a_clk), .D(a_wrdata), .WE(a_wren1),
        .DPRA3(b_addr[3]), .DPRA2(b_addr[2]), .DPRA1(b_addr[1]), .DPRA0(b_addr[0]), .DPO(b_rddata1));

    RAM16X1D #(.INIT(INIT[47:32])) ram2(
        .A3(a_addr[3]), .A2(a_addr[2]), .A1(a_addr[1]), .A0(a_addr[0]), .SPO(a_rddata2),
        .WCLK(a_clk), .D(a_wrdata), .WE(a_wren2),
        .DPRA3(b_addr[3]), .DPRA2(b_addr[2]), .DPRA1(b_addr[1]), .DPRA0(b_addr[0]), .DPO(b_rddata2));

    RAM16X1D #(.INIT(INIT[63:48])) ram3(
        .A3(a_addr[3]), .A2(a_addr[2]), .A1(a_addr[1]), .A0(a_addr[0]), .SPO(a_rddata3),
        .WCLK(a_clk), .D(a_wrdata), .WE(a_wren3),
        .DPRA3(b_addr[3]), .DPRA2(b_addr[2]), .DPRA1(b_addr[1]), .DPRA0(b_addr[0]), .DPO(b_rddata3));

endmodule
