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

    dpram8k ram0(
        .a_clk(p1_clk), .a_addr(p1_addr[10:0]), .a_wrdata(p1_wrdata), .a_wrsel(p1_bytesel), .a_wren(p1_wren && !p1_addr[11]), .a_rddata(p1_ram0_rddata),
        .b_clk(p2_clk), .b_addr(p2_addr[11:1]), .b_wrdata(32'b0),     .b_wrsel(4'b0),       .b_wren(1'b0),                    .b_rddata(p2_ram0_rddata));

    dpram8k ram1(
        .a_clk(p1_clk), .a_addr(p1_addr[10:0]), .a_wrdata(p1_wrdata), .a_wrsel(p1_bytesel), .a_wren(p1_wren &&  p1_addr[11]), .a_rddata(p1_ram1_rddata),
        .b_clk(p2_clk), .b_addr(p2_addr[11:1]), .b_wrdata(32'b0),     .b_wrsel(4'b0),       .b_wren(1'b0),                    .b_rddata(p2_ram1_rddata));

    reg       q_p1_sel;
    reg [1:0] q_p2_sel;

    always @(p1_clk) q_p1_sel <= p1_addr[11];
    always @(p2_clk) q_p2_sel <= {p2_addr[12], p2_addr[0]};

    assign p1_rddata = !q_p1_sel ? p1_ram0_rddata : p1_ram1_rddata;

    always @* case (q_p2_sel)
        2'b00: p2_rddata = p2_ram0_rddata[15:0];
        2'b01: p2_rddata = p2_ram0_rddata[31:16];
        2'b10: p2_rddata = p2_ram1_rddata[15:0];
        2'b11: p2_rddata = p2_ram1_rddata[31:16];
    endcase

endmodule
