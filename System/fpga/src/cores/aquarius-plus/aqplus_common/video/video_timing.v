`default_nettype none
`timescale 1 ns / 1 ps

// VGA video timing module - 704x480 visible (910x525 total)

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

    output wire        blank);

    // Sync mode on end of frame
    reg mode_r = 0;
    always @(posedge clk)
        if (vnext)
            mode_r <= mode;

    //////////////////////////////////////////////////////////////////////////
    // Horizontal timing
    //////////////////////////////////////////////////////////////////////////
    reg  [9:0] hcnt_r = 10'd0;

    wire [9:0] hcnt_blank  = mode_r ? 10'd640 : 10'd704;
    wire [9:0] hcnt_hsync1 = mode_r ? 10'd656 : 10'd746;
    wire [9:0] hcnt_hsync2 = mode_r ? 10'd752 : 10'd854;
    wire [9:0] hcnt_last   = mode_r ? 10'd799 : 10'd909;

    wire hcnt_done = (hcnt_r == hcnt_last);

    always @(posedge(clk))
        if (hcnt_done)
            hcnt_r <= 10'd0;
        else
            hcnt_r <= hcnt_r + 10'd1;

    assign hpos    = hcnt_r;
    assign hblank  = !(hcnt_r < hcnt_blank);
    assign hsync   = !(hcnt_r >= hcnt_hsync1 && hcnt_r < hcnt_hsync2);
    assign hlast   = hcnt_done;

    //////////////////////////////////////////////////////////////////////////
    // Vertical timing
    //////////////////////////////////////////////////////////////////////////
    reg  [9:0] vcnt_r = 10'd0;
    wire [8:0] vcnt = vcnt_r[9:1];

    wire vcnt_done = hcnt_done && vcnt_r == 10'd524;

    always @(posedge(clk))
        if (vcnt_done)
            vcnt_r <= 10'd0;
        else if (hcnt_done)
            vcnt_r <= vcnt_r + 10'd1;

    assign vpos    = vcnt[7:0];
    assign vsync   = !(vcnt == 9'd245);
    assign vblank  = !(vcnt < 9'd240);
    assign vnext   = vcnt_r[0] & hcnt_done;

    //////////////////////////////////////////////////////////////////////////
    // Blanking
    //////////////////////////////////////////////////////////////////////////
    assign blank = hblank || vblank;

endmodule
