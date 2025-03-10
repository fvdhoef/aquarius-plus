// VGA video timing module
`default_nettype none
`timescale 1 ns / 1 ps

module aqp_video_timing(
    input  wire       clk,     // 25.175MHz (640x480) / 28.63636MHz (704x480)
    input  wire       mode,    // (mode 0=704x480, 1=640x480)

    output wire [9:0] hpos,
    output wire       hsync,
    output wire       hblank,
    output wire       hlast,

    output wire [9:0] vpos,
    output wire       vsync,
    output wire       vblank,
    output wire       vnext,
    output reg        vnewframe,

    output wire       blank);

    reg q_mode = 0;

    wire [9:0] hcnt_blank  = q_mode ? 10'd640 : 10'd704;
    wire [9:0] hcnt_hsync1 = q_mode ? 10'd656 : 10'd746;
    wire [9:0] hcnt_hsync2 = q_mode ? 10'd752 : 10'd854;
    wire [9:0] hcnt_last   = q_mode ? 10'd799 : 10'd909;
    wire [9:0] vcnt_blank  = 10'd480;
    wire [9:0] vcnt_hsync1 = 10'd490;
    wire [9:0] vcnt_hsync2 = 10'd491;
    wire [9:0] vcnt_last   = 10'd524;

    //////////////////////////////////////////////////////////////////////////
    // Horizontal timing
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] q_hcnt = 10'd0;

    assign hlast = (q_hcnt == hcnt_last);

    always @(posedge(clk))
        if (hlast) q_hcnt <= 10'd0;
        else       q_hcnt <= q_hcnt + 10'd1;

    assign hpos   = q_hcnt;
    assign hblank = !(q_hcnt < hcnt_blank);
    assign hsync  = !(q_hcnt >= hcnt_hsync1 && q_hcnt < hcnt_hsync2);

    //////////////////////////////////////////////////////////////////////////
    // Vertical timing
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] q_vcnt = 10'd0;

    wire vlast = hlast && q_vcnt == vcnt_last;

    always @(posedge(clk))
        if (vlast)      q_vcnt <= 10'd0;
        else if (hlast) q_vcnt <= q_vcnt + 10'd1;

    assign vpos   = q_vcnt;
    assign vblank = !(q_vcnt < vcnt_blank);
    assign vsync  = !(q_vcnt >= vcnt_hsync1 && q_vcnt <= vcnt_hsync2);
    assign vnext  = q_vcnt[0] && hlast;

    always @(posedge clk) vnewframe <= (q_vcnt == vcnt_blank) && hlast;

    // Sync mode on end of frame
    always @(posedge clk) if (vlast) q_mode <= mode;

    //////////////////////////////////////////////////////////////////////////
    // Blanking
    //////////////////////////////////////////////////////////////////////////
    assign blank = hblank || vblank;

endmodule
