`default_nettype none
`timescale 1 ns / 1 ps

module palette(
    input  wire        clk,
    input  wire  [4:0] addr,
    input  wire  [7:0] wrdata,
    input  wire        wren,

    input  wire  [4:0] palidx,
    output wire  [1:0] pal_r,
    output wire  [1:0] pal_g,
    output wire  [1:0] pal_b);

    wire [5:0] pal_color;
    assign pal_r  = pal_color[1:0];
    assign pal_g  = pal_color[3:2];
    assign pal_b  = pal_color[5:4];

    wire [6:0] rddata;  // unused

    generate
        genvar i;
        for (i=0; i<6; i=i+1) begin: palram_gen
            ram64x1d palram(
                .a_clk(clk),
                .a_addr({1'b0, addr}),
                .a_rddata(rddata[i]),
                .a_wrdata(wrdata[i]),
                .a_wren(wren),

                .b_addr({1'b0, palidx}),
                .b_rddata(pal_color[i]));
        end
    endgenerate

endmodule
