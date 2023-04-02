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

    reg  [6:0] spr_sel_r, spr_sel_next;

    assign spr_sel = spr_sel_r[5:0];

    //////////////////////////////////////////////////////////////////////////
    // Line buffer
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] wridx;
    wire [5:0] wrdata;
    wire       wren;

    reg linesel_r, linesel_next;

    linebuf linebuf(
        .clk(clk),

        .linesel(linesel_r),

        .idx1(wridx),
        .rddata1(),
        .wrdata1({2'b0, wrdata}),
        .wren1(wren),

        .idx2(linebuf_rdidx),
        .rddata2(linebuf_data));

    //////////////////////////////////////////////////////////////////////////
    // Data fetching state
    //////////////////////////////////////////////////////////////////////////

    localparam
        ST_DONE = 3'd0,
        ST_MAP1 = 3'd1,
        ST_MAP2 = 3'd2,
        ST_PAT1 = 3'd3,
        ST_PAT2 = 3'd4,
        ST_SPR = 3'd5;

    reg   [5:0] col_r,       col_next;
    reg   [5:0] col_cnt_r,   col_cnt_next;
    reg  [12:0] vaddr_r,     vaddr_next;
    reg   [2:0] state_r,     state_next;
    reg   [2:0] nxtstate_r,  nxtstate_next;
    reg  [15:0] map_entry_r, map_entry_next;
    reg         busy_r,      busy_next;

    wire  [7:0] line_idx = vline - 8'd15;
    wire  [7:0] tline    = line_idx + scry;
    wire  [4:0] row      = tline[7:3];

    wire [15:0] map_entry     = map_entry_next;
    wire  [8:0] tile_idx      = map_entry[8:0];
    wire        tile_hflip    = map_entry[9];
    wire        tile_vflip    = map_entry[10];
    wire  [1:0] tile_palette  = map_entry[13:12];
    wire        tile_priority = map_entry[14];

    assign vaddr = vaddr_next;

    // Sprite searching

    //v input  wire  [8:0] spr_x,
    //v input  wire  [7:0] spr_y,
    //v input  wire  [8:0] spr_idx,
    //v input  wire        spr_enable,
    // input  wire        spr_priority,
    //v input  wire  [1:0] spr_palette,
    //v input  wire        spr_h16,
    //v input  wire        spr_vflip,
    //v input  wire        spr_hflip,


    // Determine if sprite is on current line
    wire [3:0] spr_height  = (spr_h16 ? 4'd15 : 4'd7);
    wire [7:0] ydiff       = line_idx - spr_y;
    wire       spr_on_line = spr_enable && (ydiff <= {4'd0, spr_height});
    wire [3:0] spr_line    = spr_vflip ? (spr_height - ydiff[3:0]) : ydiff[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [8:0] render_idx_r,  render_idx_next;
    reg [31:0] render_data_r, render_data_next;
    reg        render_start;
    reg        render_is_sprite_r, render_is_sprite_next;
    reg        render_hflip_r, render_hflip_next;
    reg  [1:0] render_palette_r, render_palette_next;
    wire       render_last_pixel;
    wire       render_busy;

    renderer renderer(
        .clk(clk),
        .reset(reset),

        // Data interface
        .render_idx(render_idx_r),
        .render_data(render_data_next),
        .render_start(render_start),
        .is_sprite(render_is_sprite_r),
        .hflip(render_hflip_r),
        .palette(render_palette_r),
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
        col_next         = col_r;
        col_cnt_next     = col_cnt_r;
        vaddr_next       = vaddr_r;
        state_next       = state_r;
        nxtstate_next    = nxtstate_r;
        map_entry_next   = map_entry_r;
        busy_next        = busy_r;
        render_idx_next  = render_idx_r;
        linesel_next     = linesel_r;
        render_data_next = render_data_r;
        spr_sel_next     = spr_sel_r;

        render_is_sprite_next = render_is_sprite_r;
        render_hflip_next     = render_hflip_r;
        render_palette_next   = render_palette_r;
        render_start          = 1'b0;

        if (start) begin
            col_next              = scrx[8:3];
            col_cnt_next          = 6'd0;
            state_next            = ST_MAP1;
            busy_next             = 1'b1;
            render_idx_next       = 9'd0 - {6'd0, scrx[2:0]};
            linesel_next          = !linesel_r;
            spr_sel_next          = 7'd0;
            render_is_sprite_next = 1'b0;

        end else if (busy_r) begin
            case (state_r)
                ST_DONE: begin
                end

                ST_MAP1: begin
                    if (col_cnt_r == 6'd41) begin
                        state_next = ST_SPR;
                    end else begin
                        vaddr_next = {2'b0, row, col_next};
                        state_next = ST_MAP2;
                    end
                end

                ST_MAP2: begin
                    map_entry_next      = vdata;
                    vaddr_next          = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
                    state_next          = ST_PAT1;
                    nxtstate_next       = ST_MAP1;
                    col_next            = col_r + 6'd1;
                    col_cnt_next        = col_cnt_r + 6'd1;
                    render_hflip_next   = tile_hflip;
                    render_palette_next = tile_palette;
                end

                ST_PAT1: begin
                    render_data_next[31:24] = vdata[ 7:0];
                    render_data_next[23:16] = vdata[15:8];
                    vaddr_next[0]           = 1'b1;
                    state_next              = ST_PAT2;
                end

                ST_PAT2: begin
                    if (!render_busy || render_last_pixel) begin
                        render_data_next[15:8] = vdata[ 7:0];
                        render_data_next[7:0]  = vdata[15:8];
                        render_start           = 1'b1;
                        state_next             = nxtstate_r;
                    end
                end

                ST_SPR: begin
                    render_is_sprite_next = 1'b1;

                    if (spr_sel_r[6]) begin
                        state_next = ST_DONE;

                    end else if (spr_on_line) begin
                        render_idx_next     = spr_x;
                        render_hflip_next   = spr_hflip;
                        render_palette_next = spr_palette;
                        vaddr_next          = {spr_idx[8:1], spr_idx[0] ^ spr_line[3], spr_line[2:0], 1'b0};
                        state_next          = ST_PAT1;
                    end

                    spr_sel_next = spr_sel_r + 7'd1;
                end
            endcase

            if (render_start) render_idx_next = render_idx_r + 9'd8;

        end
    end

    always @(posedge clk) begin
        if (reset) begin
            col_r              <= 6'd0;
            col_cnt_r          <= 6'd0;
            vaddr_r            <= 13'b0;
            state_r            <= ST_DONE;
            nxtstate_r         <= ST_DONE;
            map_entry_r        <= 16'b0;
            busy_r             <= 1'b0;
            render_idx_r       <= 9'd0;
            linesel_r          <= 1'b0;
            render_data_r      <= 32'b0;
            spr_sel_r          <= 7'b0;
            render_is_sprite_r <= 1'b0;
            render_hflip_r     <= 1'b0;
            render_palette_r   <= 2'b0;

        end else begin
            col_r              <= col_next;
            col_cnt_r          <= col_cnt_next;
            vaddr_r            <= vaddr_next;
            state_r            <= state_next;
            nxtstate_r         <= nxtstate_next;
            map_entry_r        <= map_entry_next;
            busy_r             <= busy_next;
            render_idx_r       <= render_idx_next;
            linesel_r          <= linesel_next;
            render_data_r      <= render_data_next;
            spr_sel_r          <= spr_sel_next;
            render_is_sprite_r <= render_is_sprite_next;
            render_hflip_r     <= render_hflip_next;
            render_palette_r   <= render_palette_next;
        end
    end

endmodule
