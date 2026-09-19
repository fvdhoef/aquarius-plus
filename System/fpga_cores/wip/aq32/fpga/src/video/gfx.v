`default_nettype none
`timescale 1 ns / 1 ps

module gfx(
    input  wire        clk,
    input  wire        reset,

    // Register values
    input  wire        gfx_enable,
    input  wire        tilemode,    // 0:bitmap, 1:tile mode
    input  wire        bm_wrap,
    input  wire        sprites_enable,
    input  wire        layer2_enable,
    input  wire  [8:0] layer1_scrx,
    input  wire  [7:0] layer1_scry,
    input  wire  [8:0] layer2_scrx,
    input  wire  [7:0] layer2_scry,

    // Sprite attribute interface
    output wire  [7:0] spr_sel,
    input  wire  [8:0] spr_x,
    input  wire  [7:0] spr_y,
    input  wire  [9:0] spr_idx,
    input  wire  [1:0] spr_zdepth,
    input  wire  [2:0] spr_palette,
    input  wire        spr_h16,
    input  wire        spr_vflip,
    input  wire        spr_hflip,

    // Video RAM interface
    output wire [13:0] vaddr,
    input  wire [15:0] vdata,

    // Render parameters
    input  wire  [7:0] vline,
    input  wire        start,

    // Line buffer interface
    input  wire  [8:0] linebuf_rdidx,
    output wire  [6:0] linebuf_data);

    reg  [8:0] d_spr_sel, q_spr_sel;

    assign spr_sel = q_spr_sel[7:0];

    //////////////////////////////////////////////////////////////////////////
    // Line buffer
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] wridx;
    wire [6:0] wrdata;
    wire       wren;

    reg d_linesel, q_linesel;

    wire [7:0] rddata;  // Unused

    linebuf linebuf(
        .clk(clk),

        .linesel(q_linesel),

        .idx1(wridx),
        .rddata1(rddata),
        .wrdata1({1'b0, wrdata}),
        .wren1(wren),

        .idx2(linebuf_rdidx),
        .rddata2(linebuf_data));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching state
    //////////////////////////////////////////////////////////////////////////
    localparam [2:0]
        ST_DONE    = 3'd0,
        ST_MAP1    = 3'd1,
        ST_MAP2    = 3'd2,
        ST_SPR     = 3'd3,
        ST_PAT1    = 3'd4,
        ST_PAT2    = 3'd5;

    reg   [5:0] d_col,       q_col;
    reg   [5:0] d_col_cnt,   q_col_cnt;
    reg  [13:0] d_vaddr,     q_vaddr;
    reg   [2:0] d_state,     q_state;
    reg   [2:0] d_nxtstate,  q_nxtstate;
    reg  [15:0] d_map_entry, q_map_entry;
    reg         d_blankout,  q_blankout;
    reg         d_busy,      q_busy;
    reg         d_layer,     q_layer;

    wire  [7:0] line_idx = vline;
    wire  [8:0] tline    = {1'b0, line_idx} + {1'b0, q_layer ? layer2_scry : layer1_scry};
    wire  [4:0] row      = tline[7:3];

    reg   [8:0] bm_line;
    always @* begin
        if (bm_wrap) begin
            bm_line = tline;
            if (tline >= 9'd400)
                bm_line = tline - 9'd400;
            else if (tline >= 9'd200)
                bm_line = tline - 9'd200;
        end else begin
            bm_line = {1'b0, tline[7:0]};
        end
    end
    
    wire [15:0] map_entry     = d_map_entry;
    wire  [9:0] tile_idx      = map_entry[9:0];
    wire        tile_priority = map_entry[10];
    wire        tile_hflip    = map_entry[11];
    wire        tile_vflip    = map_entry[12];
    wire  [2:0] tile_palette  = map_entry[15:13];

    assign      vaddr  = d_vaddr;
    wire [15:0] vdata2 = q_blankout ? 16'b0 : vdata;

    // Determine if sprite is on current line
    wire [3:0] spr_height  = (spr_h16 ? 4'd15 : 4'd7);
    wire [7:0] ydiff       = line_idx - spr_y;
    wire       spr_on_line = (ydiff <= {4'd0, spr_height}) && spr_zdepth != 2'd0;
    wire [3:0] spr_line    = spr_vflip ? (spr_height - ydiff[3:0]) : ydiff[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [8:0] d_render_idx,         q_render_idx;
    reg [31:0] d_render_data,        q_render_data;
    reg        d_render_hflip,       q_render_hflip;
    reg  [2:0] d_render_palette,     q_render_palette;
    reg  [2:0] d_render_zdepth,      q_render_zdepth;
    reg        d_render_zdepth_init, q_render_zdepth_init;
    reg        render_start;
    wire       render_last_pixel;
    wire       render_busy;

    renderer renderer(
        .clk(clk),
        .reset(reset),

        // Data interface
        .render_idx(q_render_idx),
        .render_data(d_render_data),
        .render_start(render_start),
        .hflip(q_render_hflip),
        .palette(q_render_palette),
        .zdepth(d_render_zdepth),
        .zdepth_init(q_render_zdepth_init),
        .last_pixel(render_last_pixel),
        .busy(render_busy),

        // Line buffer interface
        .wridx(wridx),
        .wrdata(wrdata),
        .wren(wren));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching
    //////////////////////////////////////////////////////////////////////////
    reg layer_start;

    always @* begin
        d_col                = q_col;
        d_col_cnt            = q_col_cnt;
        d_vaddr              = q_vaddr;
        d_state              = q_state;
        d_nxtstate           = q_nxtstate;
        d_map_entry          = q_map_entry;
        d_busy               = q_busy;
        d_layer              = q_layer;
        d_render_idx         = q_render_idx;
        d_linesel            = q_linesel;
        d_render_data        = q_render_data;
        d_spr_sel            = q_spr_sel;
        d_blankout           = q_blankout;
        d_render_zdepth_init = q_render_zdepth_init;
        d_render_hflip       = q_render_hflip;
        d_render_palette     = q_render_palette;
        d_render_zdepth      = q_render_zdepth;
        render_start         = 0;
        layer_start          = 0;

        if (start) begin
            d_busy               = 1;
            d_linesel            = !q_linesel;
            d_render_zdepth_init = 1;
            d_spr_sel            = 0;
            d_layer              = 0;
            layer_start          = 1;
            d_state              = ST_MAP1;
            d_blankout           = !gfx_enable;

        end else if (q_busy) begin
            case (q_state)
                ST_DONE: begin
                end

                ST_MAP1: begin
                    if (q_col_cnt == 6'd41) begin
                        d_render_zdepth_init = 0;

                        if (!q_layer && layer2_enable && tilemode) begin
                            d_layer     = 1;
                            layer_start = 1;
                        end else begin
                            d_state = sprites_enable ? ST_SPR : ST_DONE;
                        end

                    end else begin
                        d_vaddr = {2'b11, !q_layer, row, q_col};
                        d_state = ST_MAP2;
                    end
                end

                ST_MAP2: begin
                    d_map_entry = vdata;

                    if (tilemode)
                        d_vaddr = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
                    else
                        d_vaddr = ({5'b0, bm_line} * 14'd80) + {7'b0, q_col, 1'b0};

                    d_state           = ST_PAT1;
                    d_nxtstate        = ST_MAP1;
                    d_render_hflip    = tilemode ? tile_hflip : 0;
                    d_render_palette  = tilemode ? tile_palette : 3'd1;
                    d_render_zdepth   = tilemode ? {q_layer ? 2'd3 : 2'd2, tile_priority} : {2'd2, 1'b0};
                end

                ST_SPR: begin
                    d_blankout = 0;

                    if (q_spr_sel[8]) begin
                        d_state = ST_DONE;

                    end else if (spr_on_line) begin
                        d_render_idx      = spr_x;
                        d_render_hflip    = spr_hflip;
                        d_render_palette  = spr_palette;
                        d_render_zdepth   = {spr_zdepth, 1'b0};
                        d_vaddr           = {spr_idx[9:1], spr_idx[0] ^ spr_line[3], spr_line[2:0], 1'b0};
                        d_state           = ST_PAT1;
                        d_nxtstate        = ST_SPR;
                    end

                    d_spr_sel = q_spr_sel + 9'd1;
                end

                ST_PAT1: begin
                    d_render_data[31:16] = {vdata2[7:0], vdata2[15:8]};
                    d_vaddr[0]           = 1;
                    d_state              = ST_PAT2;
                end

                ST_PAT2: begin
                    d_render_data[15:0] = {vdata2[7:0], vdata2[15:8]};
                    if (!tilemode && !bm_wrap && (q_col >= 6'd40 || row >= 5'd25))
                        d_render_data = 0;

                    if (!render_busy || render_last_pixel) begin
                        render_start = 1;
                        d_render_idx = q_render_idx + 9'd8;
                        d_state      = q_nxtstate;

                        d_col        = q_col + 6'd1;
                        d_col_cnt    = q_col_cnt + 6'd1;

                        if (bm_wrap && q_col == 6'd39)
                            d_col = 6'd0;
                    end
                end

                default: begin end
            endcase
        end

        if (layer_start) begin
            d_render_idx = 9'd0 - {6'd0, d_layer ? layer2_scrx[2:0] : layer1_scrx[2:0]};
            d_col        = d_layer ? layer2_scrx[8:3] : layer1_scrx[8:3];
            d_col_cnt    = 0;
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_col                <= 0;
            q_col_cnt            <= 0;
            q_vaddr              <= 0;
            q_state              <= ST_DONE;
            q_nxtstate           <= ST_DONE;
            q_map_entry          <= 0;
            q_busy               <= 0;
            q_layer              <= 0;
            q_render_idx         <= 0;
            q_linesel            <= 0;
            q_render_data        <= 0;
            q_spr_sel            <= 0;
            q_blankout           <= 0;
            q_render_zdepth_init <= 0;
            q_render_hflip       <= 0;
            q_render_palette     <= 0;
            q_render_zdepth      <= 0;

        end else begin
            q_col                <= d_col;
            q_col_cnt            <= d_col_cnt;
            q_vaddr              <= d_vaddr;
            q_state              <= d_state;
            q_nxtstate           <= d_nxtstate;
            q_map_entry          <= d_map_entry;
            q_busy               <= d_busy;
            q_layer              <= d_layer;
            q_render_idx         <= d_render_idx;
            q_linesel            <= d_linesel;
            q_render_data        <= d_render_data;
            q_spr_sel            <= d_spr_sel;
            q_blankout           <= d_blankout;
            q_render_zdepth_init <= d_render_zdepth_init;
            q_render_hflip       <= d_render_hflip;
            q_render_palette     <= d_render_palette;
            q_render_zdepth      <= d_render_zdepth;
        end
    end

endmodule
