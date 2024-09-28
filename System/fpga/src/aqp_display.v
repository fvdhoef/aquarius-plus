`default_nettype none
`timescale 1 ns / 1 ps

module aqp_display(
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

    reg [10:0] q_xcnt = 0;
    always @(posedge video_clk) begin
        q_xcnt <= q_xcnt + 11'd1;

        if (!video_hsync)
            q_xcnt <= 11'd0;
    end

    wire xcnt_last = q_xcnt == 11'd748;

    reg [10:0] q_ycnt = 0;
    always @(posedge video_clk) begin
        if (xcnt_last)
            q_ycnt <= q_ycnt + 11'd1;

        if (!video_vsync)
            q_ycnt <= 11'd0;
    end

    wire vactive = q_ycnt < 11'd400;

    //////////////////////////////////////////////////////////////////////////
    // Overlay
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] ovl_r, ovl_g, ovl_b, ovl_a;

    aqp_overlay overlay(
        .clk(video_clk),

        .xcnt(q_xcnt),
        .xcnt_last(xcnt_last),
        .vactive(vactive),

        .ovl_r(ovl_r),
        .ovl_g(ovl_g),
        .ovl_b(ovl_b),
        .ovl_a(ovl_a),

        // Overlay memory interface
        .ovl_clk(ovl_clk),

        .ovl_text_addr(ovl_text_addr),
        .ovl_text_wrdata(ovl_text_wrdata),
        .ovl_text_wr(ovl_text_wr),

        .ovl_font_addr(ovl_font_addr),
        .ovl_font_wrdata(ovl_font_wrdata),
        .ovl_font_wr(ovl_font_wr),

        .ovl_palette_addr(ovl_palette_addr),
        .ovl_palette_wrdata(ovl_palette_wrdata),
        .ovl_palette_wr(ovl_palette_wr)
    );

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
