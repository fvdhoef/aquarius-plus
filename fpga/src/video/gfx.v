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

    reg  [5:0] spr_sel_r, spr_sel_next;

    assign spr_sel = spr_sel_r;

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
    // Renderer state
    //////////////////////////////////////////////////////////////////////////

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
    wire [12:0] vaddr_pat_h = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b1};

    reg busy_r, busy_next;

    //////////////////////////////////////////////////////////////////////////
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [8:0] render_idx_r,  render_idx_next;
    reg [31:0] render_data_r, render_data_next;
    reg        render_start;
    reg        render_hlip;
    reg  [1:0] render_palette;
    wire       render_last_pixel;
    wire       render_busy;

    renderer renderer(
        .clk(clk),
        .reset(reset),

        // Data interface
        .render_idx(render_idx_r),
        .render_data(render_data_next),
        .render_start(render_start),
        .hflip(render_hlip),
        .palette(render_palette),
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
        map_entry_next   = map_entry_r;
        render_start     = 1'b0;
        busy_next        = busy_r;
        render_idx_next  = render_idx_r;
        linesel_next     = linesel_r;
        render_hlip      = 1'b0;
        render_palette   = 2'b0;
        render_data_next = render_data_r;
        spr_sel_next     = spr_sel_r;

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
                    vaddr_next     = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
                    state_next     = ST_PAT1;
                    col_next       = col_r + 6'd1;
                    col_cnt_next   = col_cnt_r + 6'd1;
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
                        render_hlip            = tile_hflip;
                        render_palette         = tile_palette;
                        render_start           = 1'b1;
                        vaddr_next             = vaddr_map;
                        state_next             = (col_cnt_r == 6'd41) ? ST_DONE : ST_MAP;
                    end
                end

                ST_DONE: begin

                end
            endcase

            if (render_start) render_idx_next = render_idx_r + 9'd8;

        end
    end

    always @(posedge clk) begin
        if (reset) begin
            col_r         <= 6'd0;
            col_cnt_r     <= 6'd0;
            vaddr_r       <= 13'b0;
            state_r       <= 2'b00;
            map_entry_r   <= 16'b0;
            busy_r        <= 1'b0;
            render_idx_r  <= 9'd0;
            linesel_r     <= 1'b0;
            render_data_r <= 32'b0;
            spr_sel_r     <= 6'b0;

        end else begin
            col_r         <= col_next;
            col_cnt_r     <= col_cnt_next;
            vaddr_r       <= vaddr_next;
            state_r       <= state_next;
            map_entry_r   <= map_entry_next;
            busy_r        <= busy_next;
            render_idx_r  <= render_idx_next;
            linesel_r     <= linesel_next;
            render_data_r <= render_data_next;
            spr_sel_r     <= spr_sel_next;
        end
    end

endmodule
