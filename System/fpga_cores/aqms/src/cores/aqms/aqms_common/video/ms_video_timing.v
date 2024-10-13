// VGA video timing module - 640x480
`default_nettype none
`timescale 1 ns / 1 ps

module ms_video_timing(
    input  wire       clk,     // 25.175MHz (640x480)
    input  wire       mode,    // (mode 0=704x480, 1=640x480)

    input  wire       left_col_blank,

    output wire [7:0] hpos,
    output wire       hsync,
    output wire       hblank,
    output wire       hlast,

    output wire [7:0] vpos,
    output wire       vsync,
    output wire       vblank,
    output wire       vnext,

    output wire [7:0] render_line,
    output reg        render_start,
    output wire       vblank_irq_pulse,

    output wire       border,

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

    wire   [8:0] hcnt    = q_hcnt[9:1];
    wire   [8:0] hpos9   = hactive ? (hcnt - 9'd32) : 9'b0;

    assign hpos   = hpos9[7:0];
    assign hblank = !(q_hcnt < hcnt_blank);
    assign hsync  = !(q_hcnt >= hcnt_hsync1 && q_hcnt < hcnt_hsync2);

    wire         hborder = !hblank && (hcnt < (left_col_blank ? 9'd40 : 9'd32) || hcnt >= 9'd288);
    wire         hactive = !(hborder || hblank);

    //////////////////////////////////////////////////////////////////////////
    // Vertical timing
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] q_vcnt = 10'd0;

    wire vlast = hlast && q_vcnt == vcnt_last;

    always @(posedge(clk))
        if (vlast)      q_vcnt <= 10'd0;
        else if (hlast) q_vcnt <= q_vcnt + 10'd1;

    wire [8:0] vcnt = q_vcnt[9:1];

    reg [8:0] vpos9;
    assign vpos = vpos9[7:0];
    always @* begin
        vpos9 = vcnt - 9'd24;
        if (vcnt < 9'd24)
            vpos9 = vcnt + 9'd232;
        if (vcnt > 9'd242)
            vpos9 = vcnt - 9'd30;
    end

    assign vblank = !(q_vcnt < vcnt_blank);
    assign vsync  = !(q_vcnt >= vcnt_hsync1 && q_vcnt <= vcnt_hsync2);
    assign vnext  = q_vcnt[0] && hlast;

    wire   vborder          = !vblank && (vcnt < 9'd24 || vcnt >= 9'd216);
    wire   vactive          = !(vborder || vblank);
    assign vblank_irq_pulse = vnext && vpos == 8'd192;

    //////////////////////////////////////////////////////////////////////////
    // Common
    //////////////////////////////////////////////////////////////////////////
    reg  [7:0] q_render_line  = 8'd0;
    wire [8:0] d_render_line9 = vcnt - 9'd22;
    wire [7:0] d_render_line  = d_render_line9[7:0];

    always @(posedge (clk)) begin
        render_start <= 1'b0;

        if (vnext) begin
            if (d_render_line <= 8'd192)
                render_start <= 1'b1;
            q_render_line <= d_render_line;
        end
    end

    assign render_line = q_render_line;

    assign border = hborder || vborder;


    // Sync mode on end of frame
    always @(posedge clk) if (vlast) q_mode <= mode;

    //////////////////////////////////////////////////////////////////////////
    // Blanking
    //////////////////////////////////////////////////////////////////////////
    assign blank  = hblank  || vblank;

endmodule
