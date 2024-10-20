`default_nettype none
`timescale 1 ns / 1 ps

module renderer(
    input  wire        clk,
    input  wire        reset,

    // Data interface
    input  wire  [7:0] render_idx,
    input  wire [31:0] render_data,
    input  wire        render_start,
    input  wire        is_sprite,
    input  wire        hflip,
    input  wire        palette,
    input  wire        render_priority,
    output wire        last_pixel,
    output reg         spr_collision,
    output wire        busy,

    // Line buffer interface
    output wire  [7:0] wridx,
    output wire  [4:0] wrdata,
    output wire        wren
);

    reg [31:0] d_render_data, q_render_data;
    reg        d_palette,     q_palette;
    reg  [7:0] d_wridx,       q_wridx;
    reg  [4:0] d_wrdata,      q_wrdata;
    reg        d_wren,        q_wren;
    reg  [2:0] d_datasel,     q_datasel;
    reg        d_busy,        q_busy;
    reg        d_last_pixel,  q_last_pixel;
    reg        d_is_sprite,   q_is_sprite;
    reg        d_hflip,       q_hflip;
    reg        d_priority,    q_priority;

    assign wridx      = q_wridx;
    assign wrdata     = q_wrdata;
    assign wren       = q_wren;
    assign busy       = q_busy;
    assign last_pixel = q_last_pixel;

    reg [3:0] pixel_data;
    always @* case (d_datasel ^ (d_hflip ? 3'b111 : 3'b000))
        3'b000: pixel_data = d_render_data[31:28];
        3'b001: pixel_data = d_render_data[27:24];
        3'b010: pixel_data = d_render_data[23:20];
        3'b011: pixel_data = d_render_data[19:16];
        3'b100: pixel_data = d_render_data[15:12];
        3'b101: pixel_data = d_render_data[11:8];
        3'b110: pixel_data = d_render_data[7:4];
        3'b111: pixel_data = d_render_data[3:0];
    endcase

    wire [1:0] lab_wrdata = {q_is_sprite, (q_priority && (wrdata[3:0] != 4'd0))};
    wire       lab_is_sprite, lab_priority;

    lineattrbuf lab(
        .clk(clk),
        .idx1(wridx),
        .wrdata1(lab_wrdata),
        .wren1(wren),

        .idx2(d_wridx),
        .rddata2({lab_is_sprite, lab_priority}));

    always @* begin
        d_render_data = q_render_data;
        d_palette     = q_palette;
        d_wridx       = q_wridx;
        d_wrdata      = q_wrdata;
        d_wren        = 1'b0;
        d_datasel     = q_datasel;
        d_busy        = q_busy;
        d_last_pixel  = 1'b0;
        d_is_sprite   = q_is_sprite;
        d_hflip       = q_hflip;
        d_priority    = q_priority;
        spr_collision = 1'b0;

        if (render_start) begin
            d_render_data = render_data;
            d_palette     = palette;
            d_datasel     = 3'b0;
            d_wren        = 1'b1;
            d_busy        = 1'b1;
            d_wridx       = render_idx;
            d_is_sprite   = is_sprite;
            d_hflip       = hflip;
            d_priority    = render_priority;

        end else if (q_busy) begin
            d_datasel = q_datasel + 3'd1;
            d_wren    = 1'b1;
            d_wridx   = q_wridx + 8'd1;

            if (q_datasel == 3'd7) begin
                d_busy = 1'b0;
                d_wren = 1'b0;
            end
            if (q_datasel == 3'd6) begin
                d_last_pixel = 1'b1;
            end
        end

        d_wrdata[4]   = d_palette;
        d_wrdata[3:0] = pixel_data;

        if (d_is_sprite) begin
            // Don't render transparent sprite pixels
            if (pixel_data == 4'd0 || lab_is_sprite || lab_priority)
                d_wren = 1'b0;

            // Check for collision
            if (pixel_data != 4'd0 && lab_is_sprite)
                spr_collision = 1'b1;
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_render_data <= 32'b0;
            q_palette     <= 1'b0;
            q_wridx       <= 8'd0;
            q_wrdata      <= 5'b0;
            q_wren        <= 1'b0;
            q_datasel     <= 3'b0;
            q_busy        <= 1'b0;
            q_last_pixel  <= 1'b0;
            q_is_sprite   <= 1'b0;
            q_hflip       <= 1'b0;
            q_priority    <= 1'b0;

        end else begin
            q_render_data <= d_render_data;
            q_palette     <= d_palette;
            q_wridx       <= d_wridx;
            q_wrdata      <= d_wrdata;
            q_wren        <= d_wren;
            q_datasel     <= d_datasel;
            q_busy        <= d_busy;
            q_last_pixel  <= d_last_pixel;
            q_is_sprite   <= d_is_sprite;
            q_hflip       <= d_hflip;
            q_priority    <= d_priority;
        end
    end

endmodule
