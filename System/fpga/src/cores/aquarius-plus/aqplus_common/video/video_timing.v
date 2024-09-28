// VGA video timing module - 704x480 visible (910x525 total)
`default_nettype none
`timescale 1 ns / 1 ps

module video_timing(
    input  wire        clk,     // 25.175MHz (640x480) / 28.63636MHz (704x480)
    input  wire        mode,    // (mode 0=704x480, 1=640x480)

    output wire [9:0]  hpos,
    output wire        hsync,
    output wire        hblank,
    output wire        hlast,

    output wire [7:0]  vpos,
    output wire        vsync,
    output wire        vblank,
    output wire        vnext,
    output reg         vnewframe,
    output reg         voddline,

    output wire        blank);

    // Sync mode on end of frame
    reg q_mode = 0;
    always @(posedge clk) if (vnext) q_mode <= mode;

    //////////////////////////////////////////////////////////////////////////
    // Horizontal timing
    //////////////////////////////////////////////////////////////////////////
    reg  [9:0] q_hcnt = 10'd0;

    wire [9:0] hcnt_blank  = q_mode ? 10'd640 : 10'd704;
    wire [9:0] hcnt_hsync1 = q_mode ? 10'd656 : 10'd746;
    wire [9:0] hcnt_hsync2 = q_mode ? 10'd752 : 10'd854;
    wire [9:0] hcnt_last   = q_mode ? 10'd799 : 10'd909;

    wire hcnt_done = (q_hcnt == hcnt_last);

    always @(posedge(clk))
        if (hcnt_done) q_hcnt <= 10'd0;
        else           q_hcnt <= q_hcnt + 10'd1;

    assign hpos    = q_hcnt;
    assign hblank  = !(q_hcnt < hcnt_blank);
    assign hsync   = !(q_hcnt >= hcnt_hsync1 && q_hcnt < hcnt_hsync2);
    assign hlast   = hcnt_done;

    //////////////////////////////////////////////////////////////////////////
    // Vertical timing
    //////////////////////////////////////////////////////////////////////////
    reg  [9:0] q_vcnt = 10'd0;
    wire [8:0] vcnt = q_vcnt[9:1];

    wire vcnt_done = hcnt_done && q_vcnt == 10'd524;

    always @(posedge(clk))
        if (vcnt_done)      q_vcnt <= 10'd0;
        else if (hcnt_done) q_vcnt <= q_vcnt + 10'd1;

    assign vpos    = vcnt[7:0];
    assign vsync   = !(vcnt == 9'd245);
    assign vblank  = !(vcnt < 9'd240);
    assign vnext   = q_vcnt[0] & hcnt_done;

    always @(posedge clk) vnewframe <= (vcnt == 9'd240) && hcnt_done;
    always @(posedge clk) voddline  <= q_vcnt[0];

    //////////////////////////////////////////////////////////////////////////
    // Blanking
    //////////////////////////////////////////////////////////////////////////
    assign blank = hblank || vblank;

endmodule
