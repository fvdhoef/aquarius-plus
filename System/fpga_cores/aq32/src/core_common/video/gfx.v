`default_nettype none
`timescale 1 ns / 1 ps

module gfx(
    input  wire        clk,
    input  wire        reset,

    // Register values
    input  wire  [1:0] gfx_mode,
    input  wire        sprites_enable,
    input  wire  [8:0] scrx,
    input  wire  [7:0] scry,

    // Sprite attribute interface
    output wire  [5:0] spr_sel,
    input  wire  [8:0] spr_x,
    input  wire  [7:0] spr_y,
    input  wire  [8:0] spr_idx,
    input  wire        spr_enable,
    input  wire        spr_priority,
    input  wire  [1:0] spr_palette,
    input  wire        spr_h16,
    input  wire        spr_vflip,
    input  wire        spr_hflip,

    // Video RAM interface
    output wire [12:0] vaddr,
    input  wire [15:0] vdata,

    // Render parameters
    input  wire  [7:0] vline,
    input  wire        start,

    // Line buffer interface
    input  wire  [8:0] linebuf_rdidx,
    output wire  [5:0] linebuf_data);

    reg  [6:0] d_spr_sel, q_spr_sel;

    assign spr_sel = q_spr_sel[5:0];

    //////////////////////////////////////////////////////////////////////////
    // Line buffer
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] wridx;
    wire [5:0] wrdata;
    wire       wren;

    reg d_linesel, q_linesel;

    wire [7:0] rddata;  // Unused

    linebuf linebuf(
        .clk(clk),

        .linesel(q_linesel),

        .idx1(wridx),
        .rddata1(rddata),
        .wrdata1({2'b0, wrdata}),
        .wren1(wren),

        .idx2(linebuf_rdidx),
        .rddata2(linebuf_data));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching state
    //////////////////////////////////////////////////////////////////////////
    localparam [3:0]
        ST_DONE    = 4'd0,
        ST_MAP1    = 4'd1,
        ST_MAP2    = 4'd2,
        ST_BM1     = 4'd3,
        ST_BM2     = 4'd4,
        ST_BM3     = 4'd5,
        ST_SPR     = 4'd6,
        ST_PAT1    = 4'd7,
        ST_PAT2    = 4'd8,
        ST_BM4BPP  = 4'd9,
        ST_BM4BPP2 = 4'd10;

    localparam [1:0]
        MODE_DISABLED = 2'b00,
        MODE_TILE     = 2'b01,
        MODE_BM1BPP   = 2'b10,
        MODE_BM4BPP   = 2'b11;

    reg   [5:0] d_col,       q_col;
    reg   [5:0] d_col_cnt,   q_col_cnt;
    reg  [12:0] d_vaddr,     q_vaddr;
    reg   [3:0] d_state,     q_state;
    reg   [3:0] d_nxtstate,  q_nxtstate;
    reg  [15:0] d_map_entry, q_map_entry;
    reg         d_blankout,  q_blankout;
    reg         d_busy,      q_busy;

    wire  [7:0] line_idx = vline - 8'd15;
    wire  [7:0] tline    = line_idx + scry;
    wire  [4:0] row      = tline[7:3];

    wire [15:0] map_entry     = d_map_entry;
    wire  [8:0] tile_idx      = map_entry[8:0];
    wire        tile_hflip    = map_entry[9];
    wire        tile_vflip    = map_entry[10];
    wire  [1:0] tile_palette  = map_entry[13:12];
    wire        tile_priority = map_entry[14];

    assign vaddr = d_vaddr;

    wire [15:0] vdata2 = q_blankout ? 16'b0 : vdata;

    // Determine if sprite is on current line
    wire [3:0] spr_height  = (spr_h16 ? 4'd15 : 4'd7);
    wire [7:0] ydiff       = line_idx - spr_y;
    wire       spr_on_line = spr_enable && (ydiff <= {4'd0, spr_height});
    wire [3:0] spr_line    = spr_vflip ? (spr_height - ydiff[3:0]) : ydiff[3:0];

    // Bitmap address calculation
    wire [12:0] bm_addr  = (line_idx      * 13'd20) + {8'b0, q_col_cnt[5:1]};
    wire [11:0] bmc_offs = (line_idx[7:3] * 12'd20) + {7'b0, q_col_cnt[5:1]};
    wire [12:0] bmc_addr = {1'b1, bmc_offs};

    wire [7:0] bm_data  = q_col_cnt[0] ? q_map_entry[7:0] : q_map_entry[15:8];
    wire [7:0] bmc_data = q_col_cnt[0] ? vdata[7:0] : vdata[15:8];
    wire [3:0] bmc_fg   = bmc_data[7:4];
    wire [3:0] bmc_bg   = bmc_data[3:0];

    wire [31:0] bm_render_data;
    assign bm_render_data[31:28] = bm_data[7] ? bmc_fg : bmc_bg;
    assign bm_render_data[27:24] = bm_data[6] ? bmc_fg : bmc_bg;
    assign bm_render_data[23:20] = bm_data[5] ? bmc_fg : bmc_bg;
    assign bm_render_data[19:16] = bm_data[4] ? bmc_fg : bmc_bg;
    assign bm_render_data[15:12] = bm_data[3] ? bmc_fg : bmc_bg;
    assign bm_render_data[11: 8] = bm_data[2] ? bmc_fg : bmc_bg;
    assign bm_render_data[ 7: 4] = bm_data[1] ? bmc_fg : bmc_bg;
    assign bm_render_data[ 3: 0] = bm_data[0] ? bmc_fg : bmc_bg;

    //////////////////////////////////////////////////////////////////////////
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [8:0] d_render_idx,       q_render_idx;
    reg [31:0] d_render_data,      q_render_data;
    reg        d_render_is_sprite, q_render_is_sprite;
    reg        d_render_hflip,     q_render_hflip;
    reg  [1:0] d_render_palette,   q_render_palette;
    reg        d_render_priority,  q_render_priority;
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
        .is_sprite(q_render_is_sprite),
        .hflip(q_render_hflip),
        .palette(q_render_palette),
        .render_priority(q_render_priority),
        .last_pixel(render_last_pixel),
        .busy(render_busy),

        // Line buffer interface
        .wridx(wridx),
        .wrdata(wrdata),
        .wren(wren));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching
    //////////////////////////////////////////////////////////////////////////
    always @* begin
        d_col         = q_col;
        d_col_cnt     = q_col_cnt;
        d_vaddr       = q_vaddr;
        d_state       = q_state;
        d_nxtstate    = q_nxtstate;
        d_map_entry   = q_map_entry;
        d_busy        = q_busy;
        d_render_idx  = q_render_idx;
        d_linesel     = q_linesel;
        d_render_data = q_render_data;
        d_spr_sel     = q_spr_sel;
        d_blankout    = q_blankout;

        d_render_is_sprite = q_render_is_sprite;
        d_render_hflip     = q_render_hflip;
        d_render_palette   = q_render_palette;
        d_render_priority  = q_render_priority;
        render_start          = 1'b0;

        if (start) begin
            d_busy             = 1'b1;
            d_linesel          = !q_linesel;
            d_render_is_sprite = 1'b0;
            d_col_cnt          = 6'd0;
            d_spr_sel          = 7'd0;

            case (gfx_mode)
                MODE_BM1BPP: d_state = ST_BM1;
                MODE_BM4BPP: d_state = ST_BM4BPP;
                default:     d_state = ST_MAP1;
            endcase

            d_blankout   = (gfx_mode == MODE_DISABLED);
            d_render_idx = (gfx_mode == MODE_TILE) ? (9'd0 - {6'd0, scrx[2:0]}) : 9'd0;
            d_col        = scrx[8:3];

        end else if (q_busy) begin
            case (q_state)
                ST_DONE: begin
                end

                ST_MAP1: begin
                    if (q_col_cnt == 6'd41) begin
                        d_blankout = 1'b0;
                        d_state    = sprites_enable ? ST_SPR : ST_DONE;
                    end else begin
                        d_vaddr = {2'b0, row, d_col};
                        d_state = ST_MAP2;
                    end
                end

                ST_MAP2: begin
                    d_map_entry       = vdata;
                    d_vaddr           = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
                    d_state           = ST_PAT1;
                    d_nxtstate        = ST_MAP1;
                    d_col             = q_col + 6'd1;
                    d_col_cnt         = q_col_cnt + 6'd1;
                    d_render_hflip    = tile_hflip;
                    d_render_palette  = tile_palette;
                    d_render_priority = tile_priority;
                end

                ST_BM1: begin
                    if (q_col_cnt == 6'd40) begin
                        d_state = sprites_enable ? ST_SPR : ST_DONE;
                    end else begin
                        d_vaddr = bm_addr;
                        d_state = ST_BM2;
                    end
                end

                ST_BM2: begin
                    d_map_entry       = vdata;
                    d_vaddr           = bmc_addr;
                    d_state           = ST_BM3;
                    d_col_cnt         = q_col_cnt + 6'd1;
                    d_render_hflip    = 1'b0;
                    d_render_palette  = 2'b01;
                    d_render_priority = 1'b0;
                end

                ST_BM3: begin
                    if (!render_busy || render_last_pixel) begin
                        d_render_data = bm_render_data;
                        render_start  = 1'b1;
                        d_state       = ST_BM1;
                    end
                end

                ST_SPR: begin
                    d_render_is_sprite = 1'b1;

                    if (q_spr_sel[6]) begin
                        d_state = ST_DONE;

                    end else if (spr_on_line) begin
                        d_render_idx      = spr_x;
                        d_render_hflip    = spr_hflip;
                        d_render_palette  = spr_palette;
                        d_render_priority = spr_priority;
                        d_vaddr           = {spr_idx[8:1], spr_idx[0] ^ spr_line[3], spr_line[2:0], 1'b0};
                        d_state           = ST_PAT1;
                        d_nxtstate        = ST_SPR;
                    end

                    d_spr_sel = q_spr_sel + 7'd1;
                end

                ST_PAT1: begin
                    d_render_data[31:24] = vdata2[ 7:0];
                    d_render_data[23:16] = vdata2[15:8];
                    d_vaddr[0]           = 1'b1;
                    d_state              = ST_PAT2;
                end

                ST_PAT2: begin
                    if (!render_busy || render_last_pixel) begin
                        d_render_data[15:8] = vdata2[ 7:0];
                        d_render_data[7:0]  = vdata2[15:8];
                        render_start        = 1'b1;
                        d_state             = q_nxtstate;
                    end
                end

                ST_BM4BPP: begin
                    if (q_col_cnt == 6'd40) begin
                        d_state       = sprites_enable ? ST_SPR : ST_DONE;
                    end else begin
                        d_vaddr       = (line_idx * 13'd40) + {7'b0, q_col_cnt};
                        d_state       = ST_BM4BPP2;
                    end

                    d_col_cnt         = q_col_cnt + 6'd1;
                    d_render_hflip    = 1'b0;
                    d_render_palette  = 2'b01;
                    d_render_priority = 1'b0;
                end

                ST_BM4BPP2: begin
                    if (!render_busy || render_last_pixel) begin
                        d_render_data = {
                            vdata2[ 7: 4], vdata2[ 7: 4], vdata2[ 3: 0], vdata2[ 3: 0],
                            vdata2[15:12], vdata2[15:12], vdata2[11: 8], vdata2[11: 8]
                        };
                        render_start = 1'b1;
                        d_state      = ST_BM4BPP;
                    end
                end

                default: begin end
            endcase

            if (render_start) d_render_idx = q_render_idx + 9'd8;
        end
    end

    always @(posedge clk) begin
        if (reset) begin
            q_col              <= 6'd0;
            q_col_cnt          <= 6'd0;
            q_vaddr            <= 13'b0;
            q_state            <= ST_DONE;
            q_nxtstate         <= ST_DONE;
            q_map_entry        <= 16'b0;
            q_busy             <= 1'b0;
            q_render_idx       <= 9'd0;
            q_linesel          <= 1'b0;
            q_render_data      <= 32'b0;
            q_spr_sel          <= 7'b0;
            q_blankout         <= 1'b0;
            q_render_is_sprite <= 1'b0;
            q_render_hflip     <= 1'b0;
            q_render_palette   <= 2'b0;
            q_render_priority  <= 1'b0;

        end else begin
            q_col              <= d_col;
            q_col_cnt          <= d_col_cnt;
            q_vaddr            <= d_vaddr;
            q_state            <= d_state;
            q_nxtstate         <= d_nxtstate;
            q_map_entry        <= d_map_entry;
            q_busy             <= d_busy;
            q_render_idx       <= d_render_idx;
            q_linesel          <= d_linesel;
            q_render_data      <= d_render_data;
            q_spr_sel          <= d_spr_sel;
            q_blankout         <= d_blankout;
            q_render_is_sprite <= d_render_is_sprite;
            q_render_hflip     <= d_render_hflip;
            q_render_palette   <= d_render_palette;
            q_render_priority  <= d_render_priority;
        end
    end

endmodule
