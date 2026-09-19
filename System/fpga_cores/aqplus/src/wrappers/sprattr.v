`default_nettype none
`timescale 1 ns / 1 ps

module sprattr(
    input  wire        clk,
    input  wire        reset,
    input  wire  [3:0] io_addr,   // 4/5/6/7/8/9
    output reg   [7:0] io_rddata,
    input  wire  [7:0] io_wrdata,
    input  wire        io_wren,

    input  wire  [5:0] spr_sel,
    output wire  [8:0] spr_x,
    output wire  [7:0] spr_y,
    output wire  [8:0] spr_idx,
    output wire        spr_enable,
    output wire        spr_priority,
    output wire  [1:0] spr_palette,
    output wire        spr_h16,
    output wire        spr_vflip,
    output wire        spr_hflip
);

    reg [5:0] q_sprsel;

    localparam NUMBITS = 33;

    wire [(NUMBITS-1):0] a_rddata;
    wire [(NUMBITS-1):0] a_wrdata;
    wire [(NUMBITS-1):0] a_wren;

    wire [(NUMBITS-1):0] b_rddata;
    assign {
        spr_enable,
        spr_priority,
        spr_palette,
        spr_h16,
        spr_vflip,
        spr_hflip,
        spr_idx,
        spr_y,
        spr_x
    } = b_rddata;

    assign a_wrdata = {
        io_wrdata,      // VSPRATTR
        io_wrdata,      // VSPRIDX
        io_wrdata,      // VSPRY
        io_wrdata[0],   // VSPRX_H
        io_wrdata       // VSPRX_L
    };

    always @* case (io_addr)
        4'h4: io_rddata = {2'b0, q_sprsel};     // VSPRSEL
        4'h5: io_rddata = a_rddata[7:0];        // VSPRX_L
        4'h6: io_rddata = {7'b0, a_rddata[8]};  // VSPRX_H
        4'h7: io_rddata = a_rddata[16:9];       // VSPRY
        4'h8: io_rddata = a_rddata[24:17];      // VSPRIDX
        4'h9: io_rddata = a_rddata[32:25];      // VSPRATTR
        default: io_rddata = 8'h00;
    endcase

    always @(posedge clk or posedge reset)
        if (reset)
            q_sprsel <= 6'b0;
        else if (io_wren && io_addr == 4'h4)
            q_sprsel <= io_wrdata[5:0];

    wire wren_vsprx_l  = io_wren && io_addr == 4'h5;
    wire wren_vsprx_h  = io_wren && io_addr == 4'h6;
    wire wren_vspry    = io_wren && io_addr == 4'h7;
    wire wren_vspridx  = io_wren && io_addr == 4'h8;
    wire wren_vsprattr = io_wren && io_addr == 4'h9;

    assign a_wren = {
        wren_vsprattr, wren_vsprattr, wren_vsprattr, wren_vsprattr, wren_vsprattr, wren_vsprattr, wren_vsprattr, wren_vsprattr,
        wren_vspridx,  wren_vspridx,  wren_vspridx,  wren_vspridx,  wren_vspridx,  wren_vspridx,  wren_vspridx,  wren_vspridx,
        wren_vspry,    wren_vspry,    wren_vspry,    wren_vspry,    wren_vspry,    wren_vspry,    wren_vspry,    wren_vspry,
        wren_vsprx_h,
        wren_vsprx_l,  wren_vsprx_l,  wren_vsprx_l,  wren_vsprx_l,  wren_vsprx_l,  wren_vsprx_l,  wren_vsprx_l,  wren_vsprx_l
    };

    generate
        genvar i;
        for (i=0; i<NUMBITS; i=i+1) begin: sprattr_gen
            ram64x1d ram(
                .a_clk(clk), .a_addr(q_sprsel), .a_rddata(a_rddata[i]), .a_wrdata(a_wrdata[i]), .a_wren(a_wren[i]),
                .b_addr(spr_sel), .b_rddata(b_rddata[i]));
        end
    endgenerate

endmodule
