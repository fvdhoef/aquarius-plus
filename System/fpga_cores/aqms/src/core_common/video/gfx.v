`default_nettype none
`timescale 1 ns / 1 ps

module gfx(
    input  wire        clk,
    input  wire        reset,

    // Register values
    input  wire        hscroll_inhibit,
    input  wire        vscroll_inhibit,
    input  wire  [7:0] hscroll,
    input  wire  [7:0] vscroll,
    input  wire  [2:0] base_nt,         // bit [13:11] of Name Table Base Address
    input  wire  [5:0] base_sprattr,    // bit [13:8] of Sprite Attribute Table Base Address
    input  wire        base_sprpat,     // bit [13] of Sprite Pattern Generator Base Address
    input  wire        spr_h16,

    // Render parameters
    input  wire  [7:0] line,
    input  wire        start,

    // Output signals
    output reg         spr_overflow,
    output wire        spr_collision,

    // Video RAM interface
    output wire [12:0] vaddr,
    input  wire [15:0] vdata,

    // Line buffer interface
    input  wire  [7:0] linebuf_rdidx,
    output wire  [4:0] linebuf_data);

    //////////////////////////////////////////////////////////////////////////
    // Line buffer
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] wridx;
    wire [4:0] wrdata;
    wire       wren;

    reg d_linesel, q_linesel;

    wire [7:0] linebuf_rddata1;  // unused

    linebuf linebuf(
        .clk(clk),

        .linesel(q_linesel),

        .idx1(wridx),
        .rddata1(linebuf_rddata1),
        .wrdata1({3'b0, wrdata}),
        .wren1(wren),

        .idx2(linebuf_rdidx),
        .rddata2(linebuf_data));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching state
    //////////////////////////////////////////////////////////////////////////
    localparam
        ST_DONE    = 4'd0,
        ST_MAP1    = 4'd1,
        ST_MAP2    = 4'd2,
        ST_SPR1    = 4'd3,
        ST_SPR2    = 4'd4,
        ST_SPR3    = 4'd5,
        ST_PAT1    = 4'd7,
        ST_PAT2    = 4'd8;

    reg   [7:0] d_vscroll,   q_vscroll;
    reg   [7:0] d_hscroll,   q_hscroll;
    reg   [5:0] d_col,       q_col;
    reg  [12:0] d_vaddr,     q_vaddr;
    reg   [3:0] d_state,     q_state;
    reg   [3:0] d_nxtstate,  q_nxtstate;
    reg  [15:0] d_map_entry, q_map_entry;
    reg         d_busy,      q_busy;
    reg   [6:0] d_spr_idx,   q_spr_idx;
    reg   [7:0] d_spr_y,     q_spr_y;
    reg   [3:0] d_spr_cnt,   q_spr_cnt;

    wire  [4:0] column        = d_col[4:0];
    wire  [7:0] vscroll2      = (column [4:3] == 2'b11 && vscroll_inhibit) ? 8'd0 : q_vscroll;
    wire  [7:0] line_idx      = line;
    wire  [8:0] tline         = {1'b0, line_idx} + {1'b0, vscroll2};
    wire  [5:0] row_minus28   = tline[8:3] - 6'd28;
    wire  [4:0] row           = (tline[8:3] < 6'd28) ? tline[7:3] : row_minus28[4:0];  // Handle 28 rows

    wire [15:0] map_entry     = d_map_entry;
    wire  [8:0] tile_idx      = map_entry[8:0];
    wire        tile_hflip    = map_entry[9];
    wire        tile_vflip    = map_entry[10];
    wire        tile_palette  = map_entry[11];
    wire        tile_priority = map_entry[12];

    wire [12:0] map_addr    = {base_nt, row, column};
    wire [12:0] pat_addr    = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
    wire [12:0] spr_addr_y  = {base_sprattr, 2'b0, q_spr_idx[5:1]};
    wire [12:0] spr_addr_nx = {base_sprattr, 1'b1, q_spr_idx[5:0]};

    assign vaddr = d_vaddr;

    // Determine if sprite is on current line
    wire [3:0] spr_height  = (spr_h16 ? 4'd15 : 4'd7);
    wire [7:0] ydiff       = line_idx - q_spr_y;
    wire       spr_on_line = (ydiff <= {4'd0, spr_height});
    wire [3:0] spr_line    = ydiff[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [7:0] d_render_idx,       q_render_idx;
    reg [31:0] d_render_data,      q_render_data;
    reg        d_render_is_sprite, q_render_is_sprite;
    reg        d_render_hflip,     q_render_hflip;
    reg        d_render_palette,   q_render_palette;
    reg        d_render_priority,  q_render_priority;
    reg        render_start;
    wire       render_last_pixel;
    wire       render_busy;

    // Convert from bitplanes to chunky format
    wire [31:0] render_data = {
        d_render_data[7], d_render_data[15], d_render_data[23], d_render_data[31], 
        d_render_data[6], d_render_data[14], d_render_data[22], d_render_data[30], 
        d_render_data[5], d_render_data[13], d_render_data[21], d_render_data[29], 
        d_render_data[4], d_render_data[12], d_render_data[20], d_render_data[28], 
        d_render_data[3], d_render_data[11], d_render_data[19], d_render_data[27], 
        d_render_data[2], d_render_data[10], d_render_data[18], d_render_data[26], 
        d_render_data[1], d_render_data[ 9], d_render_data[17], d_render_data[25], 
        d_render_data[0], d_render_data[ 8], d_render_data[16], d_render_data[24] 
    };

    renderer renderer(
        .clk(clk),
        .reset(reset),

        // Data interface
        .render_idx(q_render_idx),
        .render_data(render_data),
        .render_start(render_start),
        .is_sprite(q_render_is_sprite),
        .hflip(q_render_hflip),
        .palette(q_render_palette),
        .render_priority(q_render_priority),
        .last_pixel(render_last_pixel),
        .spr_collision(spr_collision),
        .busy(render_busy),

        // Line buffer interface
        .wridx(wridx),
        .wrdata(wrdata),
        .wren(wren));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] spr_y    = q_spr_idx[0] ? vdata[15:8] : vdata[7:0];
    wire [7:0] spr_tile = vdata[15:8];
    wire [7:0] spr_x    = vdata[7:0];

    always @* begin
        d_hscroll          = q_hscroll;
        d_vscroll          = q_vscroll;
        d_col              = q_col;
        d_vaddr            = q_vaddr;
        d_state            = q_state;
        d_nxtstate         = q_nxtstate;
        d_map_entry        = q_map_entry;
        d_busy             = q_busy;
        d_render_idx       = q_render_idx;
        d_linesel          = q_linesel;
        d_render_data      = q_render_data;
        d_spr_idx          = q_spr_idx;
        d_spr_y            = q_spr_y;
        d_spr_cnt          = q_spr_cnt;
        d_render_is_sprite = q_render_is_sprite;
        d_render_hflip     = q_render_hflip;
        d_render_palette   = q_render_palette;
        d_render_priority  = q_render_priority;
        render_start       = 1'b0;
        spr_overflow       = 1'b0;

        if (start) begin
            if (hscroll_inhibit && line < 8'd16)
                d_hscroll = 8'd0;
            else
                d_hscroll = hscroll;

            if (line == 8'd0)
                d_vscroll = vscroll;

            d_busy         = 1'b1;
            d_linesel      = !q_linesel;
            d_spr_idx      = 7'd0;
            d_spr_cnt      = 4'd0;

            d_state        = ST_MAP1;
            d_render_idx   = d_hscroll;
            d_col          = 6'd0;

        end else if (q_busy) begin
            case (q_state)
                ST_DONE: begin
                end

                ST_MAP1: begin
                    if (q_col[5]) begin
                        d_state = ST_DONE;
                        d_state = ST_SPR1;
                    end else begin
                        d_vaddr = map_addr;
                        d_state = ST_MAP2;
                    end
                end

                ST_MAP2: begin
                    d_map_entry        = vdata;

                    d_vaddr            = pat_addr;

                    d_state            = ST_PAT1;
                    d_nxtstate         = ST_MAP1;
                    d_col              = q_col + 6'd1;

                    d_render_hflip     = tile_hflip;
                    d_render_palette   = tile_palette;
                    d_render_priority  = tile_priority;
                    d_render_is_sprite = 1'b0;
                end

                ST_SPR1: begin
                    if (q_spr_idx[6]) begin
                        d_state = ST_DONE;
                    end else begin
                        d_vaddr = spr_addr_y;
                        d_state = ST_SPR2;
                    end
                end

                ST_SPR2: begin
                    d_spr_y = spr_y + 8'd1;
                    d_vaddr = spr_addr_nx;
                    d_state = ST_SPR3;
                end

                ST_SPR3: begin
                    if (q_spr_y == 8'hD1) begin
                        // Terminator detected, skip remaining sprites
                        d_state = ST_DONE;

                    end else if (spr_on_line) begin
                        if (q_spr_cnt == 4'd8) begin
                            // Draw a maximum of 8 sprites on a line
                            d_state      = ST_DONE;
                            spr_overflow = 1'b1;
                            
                        end else begin
                            // Draw sprite
                            d_state   = ST_PAT1;
                            d_spr_cnt = q_spr_cnt + 4'd1;
                        end

                    end else begin
                        // Sprite not on this line, next
                        d_state = ST_SPR1;
                    end

                    d_nxtstate         = ST_SPR1;

                    d_vaddr[12]        = base_sprpat;
                    d_vaddr[11:5]      = spr_tile[7:1];
                    d_vaddr[4]         = spr_h16 ? spr_line[3] : spr_tile[0];
                    d_vaddr[3:1]       = spr_line[2:0];
                    d_vaddr[0]         = 1'b0;

                    d_render_idx       = spr_x;
                    d_render_hflip     = 1'b0;
                    d_render_palette   = 1'b1;
                    d_render_priority  = 1'b0;
                    d_render_is_sprite = 1'b1;

                    d_spr_idx          = q_spr_idx + 7'd1;
                end

                ST_PAT1: begin
                    d_render_data[31:24] = vdata[ 7:0];
                    d_render_data[23:16] = vdata[15:8];
                    d_vaddr[0]           = 1'b1;
                    d_state              = ST_PAT2;
                end

                ST_PAT2: begin
                    if (!render_busy || render_last_pixel) begin
                        d_render_data[15:8] = vdata[ 7:0];
                        d_render_data[7:0]  = vdata[15:8];
                        render_start        = 1'b1;
                        d_state             = q_nxtstate;
                    end
                end

                default: d_state = ST_DONE;
            endcase

            if (render_start) d_render_idx = q_render_idx + 8'd8;
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_hscroll          <= 8'd0;
            q_vscroll          <= 8'd0;
            q_col              <= 6'd0;
            q_vaddr            <= 13'b0;
            q_state            <= ST_DONE;
            q_nxtstate         <= ST_DONE;
            q_map_entry        <= 16'b0;
            q_busy             <= 1'b0;
            q_render_idx       <= 8'd0;
            q_linesel          <= 1'b0;
            q_render_data      <= 32'b0;
            q_spr_idx          <= 7'b0;
            q_spr_y            <= 8'b0;
            q_spr_cnt          <= 4'b0;
            q_render_is_sprite <= 1'b0;
            q_render_hflip     <= 1'b0;
            q_render_palette   <= 1'b0;
            q_render_priority  <= 1'b0;

        end else begin
            q_hscroll          <= d_hscroll;
            q_vscroll          <= d_vscroll;
            q_col              <= d_col;
            q_vaddr            <= d_vaddr;
            q_state            <= d_state;
            q_nxtstate         <= d_nxtstate;
            q_map_entry        <= d_map_entry;
            q_busy             <= d_busy;
            q_render_idx       <= d_render_idx;
            q_linesel          <= d_linesel;
            q_render_data      <= d_render_data;
            q_spr_idx          <= d_spr_idx;
            q_spr_y            <= d_spr_y;
            q_spr_cnt          <= d_spr_cnt;
            q_render_is_sprite <= d_render_is_sprite;
            q_render_hflip     <= d_render_hflip;
            q_render_palette   <= d_render_palette;
            q_render_priority  <= d_render_priority;
        end
    end

endmodule
