// VGA video timing module - 704x240 visible (910x525 total)

module video_timing(
    input  wire        clk,     // 28.63636MHz

    output wire [9:0]  hpos,
    output wire        hsync,
    output wire        hblank,
    output wire        hlast,

    output wire [7:0]  vpos,
    output wire        vsync,
    output wire        vblank,
    output wire        vnext,

    output wire        blank);

    //////////////////////////////////////////////////////////////////////////
    // Horizontal timing
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] hcnt_r = 10'd0;

    wire hcnt_done = (hcnt_r == 10'd909);

    always @(posedge(clk))
        if (hcnt_done)
            hcnt_r <= 10'd0;
        else
            hcnt_r <= hcnt_r + 10'd1;

    assign hpos    = hcnt_r;
    assign hsync   = !(hcnt_r >= 10'd746 && hcnt_r < 10'd854);
    assign hblank  = !(hcnt_r < 10'd704);
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
