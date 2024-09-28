`default_nettype none
`timescale 1 ns / 1 ps

module aqp_overlay(
    input  wire        clk,

    input  wire [10:0] xcnt,
    input  wire        xcnt_last,
    input  wire        vactive,

    output wire  [3:0] ovl_r,
    output wire  [3:0] ovl_g,
    output wire  [3:0] ovl_b,
    output wire  [3:0] ovl_a,

    // Overlay memory interface
    input  wire        ovl_clk,

    input  wire  [9:0] ovl_text_addr,
    input  wire [15:0] ovl_text_wrdata,
    input  wire        ovl_text_wr,

    input  wire [10:0] ovl_font_addr,
    input  wire  [7:0] ovl_font_wrdata,
    input  wire        ovl_font_wr,

    input  wire  [3:0] ovl_palette_addr,
    input  wire [15:0] ovl_palette_wrdata,
    input  wire        ovl_palette_wr
);

    localparam XSTART = 10;
    localparam XEND   = 650;

    //////////////////////////////////////////////////////////////////////////
    // Overlay timing
    //////////////////////////////////////////////////////////////////////////
    wire ovl_hactive = (xcnt >= XSTART) && (xcnt < XEND);
    wire ovl_active  = ovl_hactive && vactive;

    reg [1:0] q_subpixel;
    wire pixel_next = (q_subpixel == 2'd1);
    always @(posedge clk) q_subpixel <= (!ovl_hactive || pixel_next) ? 2'd0 : (q_subpixel + 2'd1);

    reg [1:0] q_subline;
    wire line_next = (q_subline == 2'd1);
    always @(posedge clk) if (xcnt_last) q_subline <= (!vactive || line_next) ? 2'd0 : (q_subline + 2'd1);

    reg [2:0] q_pixelsel, q2_pixelsel, q3_pixelsel, q4_pixelsel;
    always @(posedge clk) q_pixelsel  <= !ovl_hactive ? 3'd0 : (pixel_next ? (q_pixelsel + 3'd1) : q_pixelsel);
    always @(posedge clk) q2_pixelsel <= q_pixelsel;
    always @(posedge clk) q3_pixelsel <= q2_pixelsel;
    always @(posedge clk) q4_pixelsel <= q3_pixelsel;

    reg [2:0] q_font_line;
    wire font_line_next = line_next && (q_font_line == 3'd7);
    always @(posedge clk) q_font_line <= !vactive ? 3'd0 : (xcnt_last && line_next ? (q_font_line + 3'd1) : q_font_line);

    reg [9:0] q_text_addr_line;
    reg [9:0] q_text_addr;

    always @(posedge clk) begin
        if (!vactive) begin
            q_text_addr_line <= 10'b0;
            q_text_addr      <= 10'b0;
        end else begin
            if (xcnt_last) begin
                if (font_line_next)
                    q_text_addr_line <= q_text_addr;
                else
                    q_text_addr <= q_text_addr_line;
            end else begin
                if (q_pixelsel == 3'd7 && pixel_next)
                    q_text_addr <= q_text_addr + 10'd1;
            end
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Text / attribute RAM
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] text_data;

    aqp_ovl_text_ram text_ram(
        .wrclk(ovl_clk),
        .wraddr(ovl_text_addr),
        .wrdata(ovl_text_wrdata),
        .wren(ovl_text_wr),

        .rdclk(clk),
        .rdaddr(q_text_addr),
        .rddata(text_data));

    //////////////////////////////////////////////////////////////////////////
    // Font RAM
    //////////////////////////////////////////////////////////////////////////
    wire  [7:0] font_data;
    wire [10:0] font_addr = {text_data[7:0], q_font_line};

    // Compensate for pipeline delay in Font RAM
    reg [7:0] q_text_attr;
    always @(posedge clk) q_text_attr <= text_data[15:8];

    wire [7:0] ovl_font_rddata; // unused

    aqp_ovl_font_ram font_ram(
        .wrclk(ovl_clk),
        .wraddr(ovl_font_addr),
        .wrdata(ovl_font_wrdata),
        .wren(ovl_font_wr),

        .rdclk(clk),
        .rdaddr(font_addr),
        .rddata(font_data));

    wire       font_pixel = font_data[~q4_pixelsel];
    wire [3:0] color_idx  = font_pixel ? q_text_attr[3:0] : q_text_attr[7:4];

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] color;

    aqp_ovl_palette palette(
        .wrclk(ovl_clk),
        .wraddr(ovl_palette_addr),
        .wrdata(ovl_palette_wrdata),
        .wren(ovl_palette_wr),

        .rdaddr(color_idx),
        .rddata(color));

    reg q_ovl_active, q2_ovl_active, q3_ovl_active, q4_ovl_active;
    always @(posedge clk) q_ovl_active  <= ovl_active;
    always @(posedge clk) q2_ovl_active <= q_ovl_active;
    always @(posedge clk) q3_ovl_active <= q2_ovl_active;
    always @(posedge clk) q4_ovl_active <= q3_ovl_active;

    assign ovl_a = q4_ovl_active ? color[15:12] : 4'h0;
    assign ovl_r = color[11:8];
    assign ovl_g = color[7:4];
    assign ovl_b = color[3:0];

endmodule
