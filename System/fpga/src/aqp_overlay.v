`default_nettype none
`timescale 1 ns / 1 ps

module aqp_overlay(
    // Core video interface
    input wire         video_clk,
    input wire   [3:0] video_r,
    input wire   [3:0] video_g,
    input wire   [3:0] video_b,
    input wire         video_de,
    input wire         video_hsync,
    input wire         video_vsync,
    input wire         video_newframe,
    input wire         video_oddline,
    input wire         video_mode,

    // Overlay interface
    input  wire        ovl_clk,

    input  wire  [9:0] ovl_text_addr,
    input  wire [15:0] ovl_text_wrdata,
    input  wire        ovl_text_wr,

    input  wire [10:0] ovl_font_addr,
    input  wire  [7:0] ovl_font_wrdata,
    input  wire        ovl_font_wr,

    input  wire  [3:0] ovl_palette_addr,
    input  wire [15:0] ovl_palette_wrdata,
    input  wire        ovl_palette_wr,

    // VGA signals
    output reg   [3:0] vga_r,
    output reg   [3:0] vga_g,
    output reg   [3:0] vga_b,
    output reg         vga_hsync,
    output reg         vga_vsync
);

    wire vclk = video_clk;

    //////////////////////////////////////////////////////////////////////////
    // Video timing
    //////////////////////////////////////////////////////////////////////////
    wire [9:0] hpos;
    wire       hsync, hblank, hlast;
    wire       vsync, vnext;
    wire       blank;

    wire [9:0] vpos10;
    wire [7:0] vpos = vpos10[8:1];
    wire       vblank;

    wire       vnewframe;

    video_timing video_timing(
        .clk(vclk),
        .mode(video_mode),

        .hpos(hpos),
        .hsync(hsync),
        .hblank(hblank),
        .hlast(hlast),

        .vpos(vpos10),
        .vsync(vsync),
        .vblank(vblank),
        .vnext(vnext),
        .vnewframe(vnewframe),

        .blank(blank));

    wire hborder = video_mode ? blank : (hpos < 10'd32 || hpos >= 10'd672);
    wire vborder = vpos < 8'd16 || vpos >= 8'd216;

    reg [9:0] q_hpos, q2_hpos;
    always @(posedge vclk) q_hpos  <= hpos;
    always @(posedge vclk) q2_hpos <= q_hpos;

    reg q_blank, q_hsync, q_vsync;
    always @(posedge vclk) q_blank <= blank;
    always @(posedge vclk) q_hsync <= hsync;
    always @(posedge vclk) q_vsync <= vsync;

    reg q2_blank, q2_hsync, q2_vsync;
    always @(posedge vclk) q2_blank <= q_blank;
    always @(posedge vclk) q2_hsync <= q_hsync;
    always @(posedge vclk) q2_vsync <= q_vsync;

    //////////////////////////////////////////////////////////////////////////
    // Character address
    //////////////////////////////////////////////////////////////////////////
    reg  [9:0] q_row_addr  = 10'd0;
    reg  [9:0] q_char_addr = 10'd0;

    wire       next_row         = (vpos >= 8'd23) && vnext && (vpos[2:0] == 3'd7);
    wire [9:0] d_row_addr       = q_row_addr + 10'd40;
    wire [9:0] border_char_addr = 10'h3FF;

    always @(posedge(vclk))
        if (vblank) begin
            q_row_addr <= 10'd0;
        end else if (next_row) begin
            q_row_addr <= d_row_addr;
        end

    wire next_char = (hpos[3:0] == 4'd0);

    wire border = vborder || hborder;
    reg q_border;
    always @(posedge vclk) q_border <= border;

    wire start_active = q_border && !border;

    reg  [9:0] d_char_addr;
    always @* begin
        d_char_addr = q_char_addr;
        if (border)
            d_char_addr = border_char_addr;
        else if (start_active)
            d_char_addr = q_row_addr;
        else if (next_char)
            d_char_addr = q_char_addr + 10'd1;
    end

    always @(posedge(vclk)) q_char_addr <= d_char_addr;

    //////////////////////////////////////////////////////////////////////////
    // Text RAM
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] textram_rddata;

    aqp_ovl_text_ram text_ram(
        .wrclk(ovl_clk),
        .wraddr(ovl_text_addr),
        .wrdata(ovl_text_wrdata),
        .wren(ovl_text_wr),

        .rdclk(vclk),
        .rdaddr(d_char_addr),
        .rddata(textram_rddata));

    wire [7:0] text_data  = textram_rddata[7:0];
    wire [7:0] color_data = textram_rddata[15:8];

    reg [7:0] q_color_data;
    always @(posedge vclk) q_color_data <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Font RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos[2:0]};
    wire  [7:0] charram_data;

    aqp_ovl_font_ram font_ram(
        .wrclk(ovl_clk),
        .wraddr(ovl_font_addr),
        .wrdata(ovl_font_wrdata),
        .wren(ovl_font_wr),

        .rdclk(vclk),
        .rdaddr(charram_addr),
        .rddata(charram_data));

    wire [2:0] pixel_sel    = q2_hpos[3:1] ^ 3'b111;
    wire       char_pixel   = charram_data[pixel_sel];
    wire [3:0] text_colidx  = char_pixel ? q_color_data[7:4] : q_color_data[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] color;

    aqp_ovl_palette palette(
        .wrclk(ovl_clk),
        .wraddr(ovl_palette_addr),
        .wrdata(ovl_palette_wrdata),
        .wren(ovl_palette_wr),

        .rdaddr(text_colidx),
        .rddata(color));

    wire [3:0] ovl_a = color[15:12];
    wire [3:0] ovl_r = color[11:8];
    wire [3:0] ovl_g = color[7:4];
    wire [3:0] ovl_b = color[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Compose final image
    //////////////////////////////////////////////////////////////////////////
    always @(posedge video_clk) begin
        if (!video_de) begin
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else begin
            vga_r <= ovl_a[3] ? ovl_r : video_r;
            vga_g <= ovl_a[3] ? ovl_g : video_g;
            vga_b <= ovl_a[3] ? ovl_b : video_b;
        end

        vga_hsync <= video_hsync;
        vga_vsync <= video_vsync;
    end

endmodule
