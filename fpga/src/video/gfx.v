module gfx(
    input  wire        clk,
    input  wire        reset,

    // Register values
    input  wire  [1:0] gfx_mode,
    input  wire        sprites_enable,
    input  wire  [8:0] scrx,
    input  wire  [7:0] scry,

    // Video RAM interface
    output wire [12:0] vaddr,
    input  wire [15:0] vdata,

    // Render parameters
    input  wire  [7:0] vline,
    input  wire        start,

    // Line buffer interface
    input  wire  [8:0] linebuf_rdidx,
    output wire  [5:0] linebuf_data);

    reg  [8:0] render_idx_r,   render_idx_next;
    reg        render_start;
    wire       render_last_pixel;
    wire       render_busy;

    wire [8:0] wridx;
    wire [5:0] wrdata;
    wire       wren;

    //////////////////////////////////////////////////////////////////////////
    // Line buffer
    //////////////////////////////////////////////////////////////////////////
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
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [1:0] palette;
    reg [15:0] render_data;

    renderer renderer(
        .clk(clk),
        .reset(reset),

        // Data interface
        .render_idx(render_idx_r),
        .render_data(render_data),
        .render_start(render_start),
        .palette(palette),
        .last_pixel(render_last_pixel),
        .busy(render_busy),

        // Line buffer interface
        .wridx(wridx),
        .wrdata(wrdata),
        .wren(wren));

    // Data fetching
    localparam
        ST_MAP  = 2'd0,
        ST_PAT1 = 2'd1,
        ST_PAT2 = 2'd2,
        ST_DONE = 2'd3;

    reg [12:0] vaddr_r, vaddr_next;
    assign vaddr = vaddr_next;

    reg [1:0] state_r, state_next;

    wire [7:0] bmline = vline - 8'd15;
    wire [7:0] tline  = bmline + scry;
    wire [4:0] row    = tline[7:3];

    reg  [5:0] col_cnt_r, col_cnt_next;
    reg  [5:0] col_r, col_next;

    reg  [15:0] map_entry_r, map_entry_next;
    wire [15:0] map_entry = map_entry_next;

    wire [8:0] tile_idx      = map_entry[8:0];
    wire       tile_hflip    = map_entry[9];
    wire       tile_vflip    = map_entry[10];
    wire [1:0] tile_palette  = map_entry[13:12];
    wire       tile_priority = map_entry[14];

    wire [12:0] vaddr_map   = {2'b0, row, col_next};
    wire [12:0] vaddr_pat_l = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
    wire [12:0] vaddr_pat_h = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b1};

    reg busy_r, busy_next;

    wire [15:0] patdata         = {vdata[7:0], vdata[15:8]};
    wire [15:0] patdata_flipped = {patdata[3:0], patdata[7:4], patdata[11:8], patdata[15:12]};

    //////////////////////////////////////////////////////////////////////////
    // Data fetching
    //////////////////////////////////////////////////////////////////////////
    always @* begin
        if (reset) begin
            col_next        = 6'd0;
            col_cnt_next    = 6'd0;
            vaddr_next      = 13'b0;
            state_next      = 2'b00;
            map_entry_next  = 16'b0;
            render_start    = 1'b0;
            busy_next       = 1'b0;
            render_idx_next = 9'd0;
            linesel_next    = 1'b0;
            palette         = 2'b0;
            render_data     = 16'b0;

        end else begin
            col_next        = col_r;
            col_cnt_next    = col_cnt_r;
            vaddr_next      = vaddr_r;
            state_next      = state_r;
            map_entry_next  = map_entry_r;
            render_start    = 1'b0;
            busy_next       = busy_r;
            render_idx_next = render_idx_r;
            linesel_next    = linesel_r;
            palette         = 2'b0;
            render_data     = 16'b0;

            if (start) begin
                col_next        = scrx[8:3];
                col_cnt_next    = 6'd0;
                vaddr_next      = vaddr_map;
                state_next      = ST_MAP;
                busy_next       = 1'b1;
                render_idx_next = 9'd0 - {6'd0, scrx[2:0]};
                linesel_next    = !linesel_r;

            end else if (busy_r) begin
                case (state_r)
                    ST_MAP: begin
                        map_entry_next = vdata;
                        vaddr_next     = tile_hflip ? vaddr_pat_h : vaddr_pat_l;
                        state_next     = ST_PAT1;
                        col_next       = col_r + 6'd1;
                        col_cnt_next   = col_cnt_r + 6'd1;
                    end

                    ST_PAT1: begin
                        if (!render_busy || render_last_pixel) begin
                            palette      = tile_palette;
                            render_start = 1'b1;
                            render_data  = tile_hflip ? patdata_flipped : patdata;
                            vaddr_next   = tile_hflip ? vaddr_pat_l : vaddr_pat_h;
                            state_next   = ST_PAT2;
                        end
                    end

                    ST_PAT2: begin
                        if (render_last_pixel) begin
                            palette      = tile_palette;
                            render_start = 1'b1;
                            render_data  = tile_hflip ? patdata_flipped : patdata;
                            vaddr_next   = vaddr_map;
                            state_next   = (col_cnt_r == 6'd41) ? ST_DONE : ST_MAP;
                        end
                    end

                    ST_DONE: begin

                    end
                endcase

                if (render_start) render_idx_next = render_idx_r + 9'd4;

            end
        end
    end

    always @(posedge clk) begin
        col_r        <= col_next;
        col_cnt_r    <= col_cnt_next;
        vaddr_r      <= vaddr;
        state_r      <= state_next;
        map_entry_r  <= map_entry_next;
        busy_r       <= busy_next;
        render_idx_r <= render_idx_next;
        linesel_r    <= linesel_next;
    end

endmodule
