module gfx(
    input  wire        clk,
    input  wire        reset,

    // Register values
    input  wire        hscroll_inhibit,
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

    reg linesel_r, linesel_next;

    linebuf linebuf(
        .clk(clk),

        .linesel(linesel_r),

        .idx1(wridx),
        .rddata1(),
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

    reg   [7:0] vscroll_r,   vscroll_next;
    reg   [7:0] hscroll_r,   hscroll_next;

    reg   [5:0] col_r,       col_next;
    reg  [12:0] vaddr_r,     vaddr_next;
    reg   [3:0] state_r,     state_next;
    reg   [3:0] nxtstate_r,  nxtstate_next;
    reg  [15:0] map_entry_r, map_entry_next;
    reg         busy_r,      busy_next;

    reg   [6:0] spr_idx_r,   spr_idx_next;
    reg   [7:0] spr_y_r,     spr_y_next;
    reg   [3:0] spr_cnt_r,   spr_cnt_next;

    wire  [7:0] line_idx      = line;
    wire  [8:0] tline         = {1'b0, line_idx} + {1'b0, vscroll_r};
    wire  [4:0] row           = (tline[8:3] < 6'd28) ? tline[7:3] : (tline[8:3] - 6'd28);  // Handle 28 rows
    wire  [4:0] column        = col_next[4:0];

    wire [15:0] map_entry     = map_entry_next;
    wire  [8:0] tile_idx      = map_entry[8:0];
    wire        tile_hflip    = map_entry[9];
    wire        tile_vflip    = map_entry[10];
    wire        tile_palette  = map_entry[11];
    wire        tile_priority = map_entry[12];

    wire [12:0] map_addr    = {base_nt, row, column};
    wire [12:0] pat_addr    = {tile_idx, (tile_vflip ? ~tline[2:0] : tline[2:0]), 1'b0};
    wire [12:0] spr_addr_y  = {base_sprattr, 2'b0, spr_idx_r[5:1]};
    wire [12:0] spr_addr_nx = {base_sprattr, 1'b1, spr_idx_r[5:0]};

    assign vaddr = vaddr_next;

    // Determine if sprite is on current line
    wire [3:0] spr_height  = (spr_h16 ? 4'd15 : 4'd7);
    wire [7:0] ydiff       = line_idx - spr_y_r;
    wire       spr_on_line = (ydiff <= {4'd0, spr_height});
    wire [3:0] spr_line    = ydiff[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Renderer
    //////////////////////////////////////////////////////////////////////////
    reg  [7:0] render_idx_r,       render_idx_next;
    reg [31:0] render_data_r,      render_data_next;
    reg        render_start;
    reg        render_is_sprite_r, render_is_sprite_next;
    reg        render_hflip_r,     render_hflip_next;
    reg        render_palette_r,   render_palette_next;
    reg        render_priority_r,  render_priority_next;
    wire       render_last_pixel;
    wire       render_busy;

    // Convert from bitplanes to chunky format
    wire [31:0] render_data = {
        render_data_next[7], render_data_next[15], render_data_next[23], render_data_next[31], 
        render_data_next[6], render_data_next[14], render_data_next[22], render_data_next[30], 
        render_data_next[5], render_data_next[13], render_data_next[21], render_data_next[29], 
        render_data_next[4], render_data_next[12], render_data_next[20], render_data_next[28], 
        render_data_next[3], render_data_next[11], render_data_next[19], render_data_next[27], 
        render_data_next[2], render_data_next[10], render_data_next[18], render_data_next[26], 
        render_data_next[1], render_data_next[ 9], render_data_next[17], render_data_next[25], 
        render_data_next[0], render_data_next[ 8], render_data_next[16], render_data_next[24] 
    };

    renderer renderer(
        .clk(clk),
        .reset(reset),

        // Data interface
        .render_idx(render_idx_r),
        .render_data(render_data),
        .render_start(render_start),
        .is_sprite(render_is_sprite_r),
        .hflip(render_hflip_r),
        .palette(render_palette_r),
        .priority(render_priority_r),
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
    wire [7:0] spr_y    = spr_idx_r[0] ? vdata[15:8] : vdata[7:0];
    wire [7:0] spr_tile = vdata[15:8];
    wire [7:0] spr_x    = vdata[7:0];

    always @* begin
        hscroll_next          = hscroll_r;
        vscroll_next          = vscroll_r;

        col_next              = col_r;
        vaddr_next            = vaddr_r;
        state_next            = state_r;
        nxtstate_next         = nxtstate_r;
        map_entry_next        = map_entry_r;
        busy_next             = busy_r;
        render_idx_next       = render_idx_r;
        linesel_next          = linesel_r;
        render_data_next      = render_data_r;
        spr_idx_next          = spr_idx_r;
        spr_y_next            = spr_y_r;
        spr_cnt_next          = spr_cnt_r;
        render_is_sprite_next = render_is_sprite_r;
        render_hflip_next     = render_hflip_r;
        render_palette_next   = render_palette_r;
        render_priority_next  = render_priority_r;
        render_start          = 1'b0;
        spr_overflow          = 1'b0;

        if (start) begin
            if (hscroll_inhibit && line < 8'd16)
                hscroll_next      = 8'd0;
            else
                hscroll_next      = hscroll;

            if (line == 8'd0)
                vscroll_next      = vscroll;

            busy_next             = 1'b1;
            linesel_next          = !linesel_r;
            spr_idx_next          = 7'd0;
            spr_cnt_next          = 4'd0;

            state_next            = ST_MAP1;
            render_idx_next       = hscroll_next;
            col_next              = 5'd0;

        end else if (busy_r) begin
            case (state_r)
                ST_DONE: begin
                end

                ST_MAP1: begin
                    if (col_r[5]) begin
                        state_next = ST_DONE;
                        state_next = ST_SPR1;
                    end else begin
                        vaddr_next = map_addr;
                        state_next = ST_MAP2;
                    end
                end

                ST_MAP2: begin
                    map_entry_next        = vdata;

                    vaddr_next            = pat_addr;

                    state_next            = ST_PAT1;
                    nxtstate_next         = ST_MAP1;
                    col_next              = col_r + 6'd1;

                    render_hflip_next     = tile_hflip;
                    render_palette_next   = tile_palette;
                    render_priority_next  = tile_priority;
                    render_is_sprite_next = 1'b0;
                end

                ST_SPR1: begin
                    if (spr_idx_r[6]) begin
                        state_next = ST_DONE;
                    end else begin
                        vaddr_next = spr_addr_y;
                        state_next = ST_SPR2;
                    end
                end

                ST_SPR2: begin
                    spr_y_next = spr_y + 8'd1;
                    vaddr_next = spr_addr_nx;
                    state_next = ST_SPR3;
                end

                ST_SPR3: begin
                    if (spr_y_r == 8'hD1) begin
                        // Terminator detected, skip remaining sprites
                        state_next = ST_DONE;

                    end else if (spr_on_line) begin
                        if (spr_cnt_r == 4'd8) begin
                            // Draw a maximum of 8 sprites on a line
                            state_next   = ST_DONE;
                            spr_overflow = 1'b1;
                            
                        end else begin
                            // Draw sprite
                            state_next   = ST_PAT1;
                            spr_cnt_next = spr_cnt_r + 4'd1;
                        end

                    end else begin
                        // Sprite not on this line, next
                        state_next = ST_SPR1;
                    end

                    nxtstate_next         = ST_SPR1;

                    vaddr_next[12]        = base_sprpat;
                    vaddr_next[11:5]      = spr_tile[7:1];
                    vaddr_next[4]         = spr_h16 ? spr_line[3] : spr_tile[0];
                    vaddr_next[3:1]       = spr_line[2:0];
                    vaddr_next[0]         = 1'b0;

                    render_idx_next       = spr_x;
                    render_hflip_next     = 1'b0;
                    render_palette_next   = 1'b1;
                    render_priority_next  = 1'b0;
                    render_is_sprite_next = 1'b1;

                    spr_idx_next          = spr_idx_r + 7'd1;
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

            endcase

            if (render_start) render_idx_next = render_idx_r + 8'd8;

        end
    end

    always @(posedge clk) begin
        if (reset) begin
            hscroll_r          <= 8'd0;
            vscroll_r          <= 8'd0;

            col_r              <= 6'd0;
            vaddr_r            <= 13'b0;
            state_r            <= ST_DONE;
            nxtstate_r         <= ST_DONE;
            map_entry_r        <= 16'b0;
            busy_r             <= 1'b0;
            render_idx_r       <= 9'd0;
            linesel_r          <= 1'b0;
            render_data_r      <= 32'b0;
            spr_idx_r          <= 7'b0;
            spr_y_r            <= 8'b0;
            spr_cnt_r          <= 4'b0;
            render_is_sprite_r <= 1'b0;
            render_hflip_r     <= 1'b0;
            render_palette_r   <= 2'b0;
            render_priority_r  <= 1'b0;

        end else begin
            hscroll_r          <= hscroll_next;
            vscroll_r          <= vscroll_next;

            col_r              <= col_next;
            vaddr_r            <= vaddr_next;
            state_r            <= state_next;
            nxtstate_r         <= nxtstate_next;
            map_entry_r        <= map_entry_next;
            busy_r             <= busy_next;
            render_idx_r       <= render_idx_next;
            linesel_r          <= linesel_next;
            render_data_r      <= render_data_next;
            spr_idx_r          <= spr_idx_next;
            spr_y_r            <= spr_y_next;
            spr_cnt_r          <= spr_cnt_next;
            render_is_sprite_r <= render_is_sprite_next;
            render_hflip_r     <= render_hflip_next;
            render_palette_r   <= render_palette_next;
            render_priority_r  <= render_priority_next;
        end
    end

endmodule
