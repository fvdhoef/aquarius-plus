// VGA video timing module - 640x480
`default_nettype none
`timescale 1 ns / 1 ps

module ms_video_timing(
    input  wire       clk,     // 25.175MHz (640x480)
    input  wire       left_col_blank,

    output wire [7:0] hpos,
    output wire [7:0] vpos,

    output wire [7:0] render_line,
    output reg        render_start,
    output wire       vblank_irq_pulse,

    output wire       next_line,
    output wire       hsync,
    output wire       vsync,
    output wire       border,
    output wire       blank);

    //////////////////////////////////////////////////////////////////////////
    // Horizontal timing
    //////////////////////////////////////////////////////////////////////////
    wire hlast;

    reg [9:0] q_hcnt = 10'd0;
    always @(posedge(clk)) q_hcnt <= hlast ? 10'd0 : (q_hcnt + 10'd1);

    wire   [8:0] hcnt    = q_hcnt[9:1];
    wire   [8:0] hpos9   = hactive ? (hcnt - 9'd32) : 9'b0;
    assign       hpos    = hpos9[7:0];
    wire         hblank  = hcnt >= 9'd320;
    wire         hborder = !hblank && (hcnt < (left_col_blank ? 9'd40 : 9'd32) || hcnt >= 9'd288);
    wire         hactive = !(hborder || hblank);
    assign       hsync   = !(hcnt >= 9'd328 && hcnt < 9'd376);
    assign       hlast   = q_hcnt == 10'd799;

    //////////////////////////////////////////////////////////////////////////
    // Vertical timing
    //////////////////////////////////////////////////////////////////////////
    wire vlast;

    reg [9:0] q_vcnt = 10'd522;
    always @(posedge(clk))
        if (hlast) q_vcnt <= vlast ? 10'd0 : (q_vcnt + 10'd1);

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

    wire   vblank           = vcnt >= 9'd240;
    wire   vborder          = !vblank && (vcnt < 9'd24 || vcnt >= 9'd216);
    wire   vactive          = !(vborder || vblank);
    assign vlast            = q_vcnt == 10'd523;
    assign vsync            = !(vcnt == 9'd245);
    assign next_line        = hlast && q_vcnt[0];
    assign vblank_irq_pulse = next_line && vpos == 8'd192;

    //////////////////////////////////////////////////////////////////////////
    // Common
    //////////////////////////////////////////////////////////////////////////
    reg  [7:0] q_render_line  = 8'd0;
    wire [8:0] d_render_line9 = vcnt - 9'd22;
    wire [7:0] d_render_line  = d_render_line9[7:0];

    always @(posedge (clk)) begin
        render_start <= 1'b0;

        if (next_line) begin
            if (d_render_line <= 8'd192)
                render_start <= 1'b1;
            q_render_line <= d_render_line;
        end
    end

    assign render_line = q_render_line;

    assign border = hborder || vborder;
    assign blank  = hblank  || vblank;

endmodule
