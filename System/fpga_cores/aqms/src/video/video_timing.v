// VGA video timing module - 640x480

module video_timing(
    input  wire       clk,     // 25.175MHz (640x480)
    input  wire       left_col_blank,

    output wire [7:0] hpos,
    output reg  [7:0] vpos,

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

    reg [9:0] hcnt_r = 10'd0;
    always @(posedge(clk)) hcnt_r <= hlast ? 10'd0 : (hcnt_r + 10'd1);

    wire [8:0] hcnt = hcnt_r[9:1];

    assign hpos    = hactive ? (hcnt - 9'd32) : 8'b0;
    wire   hblank  = hcnt >= 10'd320;
    wire   hborder = !hblank && (hcnt < (left_col_blank ? 9'd40 : 9'd32) || hcnt >= 9'd288);
    wire   hactive = !(hborder || hblank);
    assign hsync   = !(hcnt >= 9'd328 && hcnt < 9'd376);
    assign hlast   = hcnt_r == 10'd799;

    //////////////////////////////////////////////////////////////////////////
    // Vertical timing
    //////////////////////////////////////////////////////////////////////////
    wire vlast;

    reg [9:0] vcnt_r = 10'd522;
    always @(posedge(clk))
        if (hlast)
            vcnt_r <= vlast ? 10'd0 : (vcnt_r + 10'd1);

    wire [8:0] vcnt = vcnt_r[9:1];

    always @* begin
        vpos = vcnt - 9'd24;
        if (vcnt < 9'd24)
            vpos = vcnt + 9'd232;
        if (vcnt > 9'd242)
            vpos = vcnt - 9'd30;
    end

    wire   vblank  = vcnt >= 9'd240;
    wire   vborder = !vblank && (vcnt < 9'd24 || vcnt >= 9'd216);
    wire   vactive = !(vborder || vblank);
    assign vsync   = !(vcnt == 9'd245);
    assign vlast   = vcnt_r == 10'd523;

    assign next_line = hlast && vcnt_r[0];

    assign vblank_irq_pulse = next_line && vpos == 9'd192;

    //////////////////////////////////////////////////////////////////////////
    // Common
    //////////////////////////////////////////////////////////////////////////
    reg  [7:0] render_line_r = 8'd0;
    wire [7:0] render_line_next = vcnt - 9'd22;

    always @(posedge (clk)) begin
        render_start <= 1'b0;

        if (next_line) begin
            if (render_line_next <= 8'd192)
                render_start  <= 1'b1;
            render_line_r <= render_line_next;
        end
    end

    assign render_line = render_line_r;

    assign border = hborder || vborder;
    assign blank  = hblank || vblank;

endmodule
