`default_nettype none
`timescale 1 ns / 1 ps

module fpga_overlay(
    input wire         clk,

    // Core video interface
    output wire  [9:0] video_hpos,
    output wire        video_hlast,
    output wire  [9:0] video_vpos,
    output wire        video_vlast,
    input  wire  [3:0] video_r,
    input  wire  [3:0] video_g,
    input  wire  [3:0] video_b,

    // Overlay interface
    input  wire  [9:0] ovl_text_addr,
    input  wire [15:0] ovl_text_wrdata,
    input  wire        ovl_text_wren,

    input  wire [10:0] ovl_font_addr,
    input  wire  [7:0] ovl_font_wrdata,
    input  wire        ovl_font_wren,

    input  wire  [3:0] ovl_palette_addr,
    input  wire [15:0] ovl_palette_wrdata,
    input  wire        ovl_palette_wren,

    // VGA signals
    output reg   [3:0] vga_r,
    output reg   [3:0] vga_g,
    output reg   [3:0] vga_b,
    output reg         vga_hsync,
    output reg         vga_vsync
);

    //////////////////////////////////////////////////////////////////////////
    // VGA timing
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] q_hcnt = 0;
    reg [9:0] q_vcnt = 0;

    wire hlast = (q_hcnt == 10'd799);
    wire vlast = (q_vcnt == 10'd524);

    always @(posedge(clk)) begin
        if (hlast) q_hcnt <= 0;
        else       q_hcnt <= q_hcnt + 10'd1;

        if (hlast) begin
            if (vlast) q_vcnt <= 0;
            else       q_vcnt <= q_vcnt + 10'd1;
        end
    end

    wire [9:0] hpos   = q_hcnt;
    wire [9:0] vpos10 = q_vcnt;

    wire hsync  = !(q_hcnt >= 10'd656 && q_hcnt <= 10'd751);
    wire vsync  = !(q_vcnt >= 10'd490 && q_vcnt <= 10'd491);

    wire hblank = !(q_hcnt < 10'd640);
    wire vblank = !(q_vcnt < 10'd480);

    assign video_hpos  = q_hcnt;
    assign video_hlast = hlast;
    assign video_vpos  = q_vcnt;
    assign video_vlast = vlast;

    //////////////////////////////////////////////////////////////////////////
    // Blanking
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] vpos = vpos10[8:1];

    wire blank = hblank || vblank;

    reg [9:0] q_hpos, q2_hpos;
    always @(posedge clk) q_hpos  <= hpos;
    always @(posedge clk) q2_hpos <= q_hpos;

    reg q_blank, q_hsync, q_vsync;
    always @(posedge clk) q_blank <= blank;
    always @(posedge clk) q_hsync <= hsync;
    always @(posedge clk) q_vsync <= vsync;

    reg q2_blank, q2_hsync, q2_vsync;
    always @(posedge clk) q2_blank <= q_blank;
    always @(posedge clk) q2_hsync <= q_hsync;
    always @(posedge clk) q2_vsync <= q_vsync;

    //////////////////////////////////////////////////////////////////////////
    // Overlay character address generator
    //////////////////////////////////////////////////////////////////////////
    wire next_row = (vpos >= 8'd23) && q_vcnt[0] && hlast && (vpos[2:0] == 3'd7);

    reg  [9:0] q_row_addr = 0;
    always @(posedge(clk))
        if      (vblank)   q_row_addr <= 0;
        else if (next_row) q_row_addr <= q_row_addr + 10'd40;

    wire border = (vpos < 8'd16 || vpos >= 8'd216) || blank;
    reg q_border;
    always @(posedge clk) q_border <= border;

    reg [9:0] q_char_addr = 0;
    reg [9:0] d_char_addr;
    always @*
        if      (border)              d_char_addr = 10'h3FF;
        else if (q_border && !border) d_char_addr = q_row_addr;
        else if (hpos[3:0] == 4'd0)   d_char_addr = q_char_addr + 10'd1;
        else                          d_char_addr = q_char_addr;

    always @(posedge(clk)) q_char_addr <= d_char_addr;

    //////////////////////////////////////////////////////////////////////////
    // Overlay text RAM
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] text_data;
    reg [7:0] color_data;

    reg [15:0] ovl_textram [0:1023];
    always @(posedge clk) if (ovl_text_wren) ovl_textram[ovl_text_addr] <= ovl_text_wrdata;
    always @(posedge clk) {color_data, text_data} <= ovl_textram[d_char_addr];

    reg [7:0] q_color_data;
    always @(posedge clk) q_color_data <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Overlay font
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos[2:0]};
    reg   [7:0] charram_data;

    reg [7:0] ovl_fontram [0:2047];
    always @(posedge clk) if (ovl_font_wren) ovl_fontram[ovl_font_addr] <= ovl_font_wrdata;
    always @(posedge clk) charram_data <= ovl_fontram[charram_addr];

    wire       char_pixel  = charram_data[~q2_hpos[3:1]];
    wire [3:0] text_colidx = char_pixel ? q_color_data[7:4] : q_color_data[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Overlay palette
    //////////////////////////////////////////////////////////////////////////
    reg  [15:0] ovl_palette [0:15];
    always @(posedge clk) if (ovl_palette_wren) ovl_palette[ovl_palette_addr] <= ovl_palette_wrdata;
    wire [15:0] ovl_color = ovl_palette[text_colidx];

    //////////////////////////////////////////////////////////////////////////
    // Compose final image
    //////////////////////////////////////////////////////////////////////////
    always @(posedge clk) begin
        vga_r     <= q2_blank ? 4'b0 : (ovl_color[15] ? ovl_color[11:8] : video_r);
        vga_g     <= q2_blank ? 4'b0 : (ovl_color[15] ? ovl_color[7:4]  : video_g);
        vga_b     <= q2_blank ? 4'b0 : (ovl_color[15] ? ovl_color[3:0]  : video_b);
        vga_hsync <= q2_hsync;
        vga_vsync <= q2_vsync;
    end

endmodule
