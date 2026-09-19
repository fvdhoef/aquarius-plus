`default_nettype none
`timescale 1 ns / 1 ps

module renderer(
    input  wire        clk,
    input  wire        reset,

    // Data interface
    input  wire  [8:0] render_idx,
    input  wire [31:0] render_data,
    input  wire        render_start,
    input  wire        hflip,
    input  wire  [2:0] palette,
    input  wire  [2:0] zdepth,
    input  wire        zdepth_init,
    output wire        last_pixel,
    output wire        busy,

    // Line buffer interface
    output wire  [8:0] wridx,
    output wire  [6:0] wrdata,
    output wire        wren
);

    reg [31:0] d_render_data, q_render_data;
    reg  [2:0] d_palette,     q_palette;
    reg  [8:0] d_wridx,       q_wridx;
    reg  [6:0] d_wrdata,      q_wrdata;
    reg        d_wren,        q_wren;
    reg  [2:0] d_datasel,     q_datasel;
    reg        d_busy,        q_busy;
    reg        d_last_pixel,  q_last_pixel;
    reg        d_zdepth_init, q_zdepth_init;
    reg        d_hflip,       q_hflip;
    reg  [2:0] d_zdepth,      q_zdepth;

    assign wridx      = q_wridx;
    assign wrdata     = q_wrdata;
    assign wren       = q_wren;
    assign busy       = q_busy;
    assign last_pixel = q_last_pixel;

    reg [3:0] pixel_data;
    always @* case (d_datasel ^ (d_hflip ? 3'b111 : 3'b000))
        3'd0: pixel_data = d_render_data[31:28];
        3'd1: pixel_data = d_render_data[27:24];
        3'd2: pixel_data = d_render_data[23:20];
        3'd3: pixel_data = d_render_data[19:16];
        3'd4: pixel_data = d_render_data[15:12];
        3'd5: pixel_data = d_render_data[11: 8];
        3'd6: pixel_data = d_render_data[ 7: 4];
        3'd7: pixel_data = d_render_data[ 3: 0];
    endcase

    wire [2:0] cur_zdepth;
    wire [2:0] lab_rddata;    // unused
    wire [2:0] new_zdepth = (q_wrdata[3:0] == 4'd0) ? 3'd0 : q_zdepth;

    distram512d #(.WIDTH(3)) lineattrbuf(
        .clk(clk),
        .a_addr(q_wridx),
        .a_rddata(lab_rddata),
        .a_wrdata(new_zdepth),
        .a_wren({3{q_wren}}),

        .b_addr(d_wridx),
        .b_rddata(cur_zdepth));

    always @* begin
        d_render_data = q_render_data;
        d_palette     = q_palette;
        d_wridx       = q_wridx;
        d_wrdata      = q_wrdata;
        d_wren        = 0;
        d_datasel     = q_datasel;
        d_busy        = q_busy;
        d_last_pixel  = 0;
        d_zdepth_init = q_zdepth_init;
        d_hflip       = q_hflip;
        d_zdepth      = q_zdepth;

        if (render_start) begin
            d_render_data = render_data;
            d_palette     = palette;
            d_datasel     = 0;
            d_wren        = 1;
            d_busy        = 1;
            d_wridx       = render_idx;
            d_zdepth_init = zdepth_init;
            d_hflip       = hflip;
            d_zdepth      = zdepth;

        end else if (q_busy) begin
            d_datasel = q_datasel + 3'd1;
            d_wren    = 1;
            d_wridx   = q_wridx + 9'd1;

            if (q_datasel == 3'd7) begin
                d_busy = 0;
                d_wren = 0;
            end
            if (q_datasel == 3'd6) begin
                d_last_pixel = 1;
            end
        end

        d_wrdata[6:4] = d_palette[2:0];
        d_wrdata[3:0] = pixel_data;

        // Don't render transparent sprite pixels
        if (!d_zdepth_init && (pixel_data == 4'd0 || d_zdepth < cur_zdepth))
            d_wren = 0;
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_render_data <= 0;
            q_palette     <= 0;
            q_wridx       <= 0;
            q_wrdata      <= 0;
            q_wren        <= 0;
            q_datasel     <= 0;
            q_busy        <= 0;
            q_last_pixel  <= 0;
            q_zdepth_init <= 0;
            q_hflip       <= 0;
            q_zdepth      <= 0;

        end else begin
            q_render_data <= d_render_data;
            q_palette     <= d_palette;
            q_wridx       <= d_wridx;
            q_wrdata      <= d_wrdata;
            q_wren        <= d_wren;
            q_datasel     <= d_datasel;
            q_busy        <= d_busy;
            q_last_pixel  <= d_last_pixel;
            q_zdepth_init <= d_zdepth_init;
            q_hflip       <= d_hflip;
            q_zdepth      <= d_zdepth;
        end
    end

endmodule
