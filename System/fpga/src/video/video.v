module video(
    input  wire        clk,
    input  wire        reset,

    // IO register interface
    input  wire  [3:0] io_addr,
    output reg   [7:0] io_rddata,
    input  wire  [7:0] io_wrdata,
    input  wire        io_wren,
    output wire        irq,

    // Text RAM interface
    input  wire [10:0] tram_addr,
    output wire  [7:0] tram_rddata,
    input  wire  [7:0] tram_wrdata,
    input  wire        tram_wren,

    // Char RAM interface
    input  wire [10:0] chram_addr,
    output wire  [7:0] chram_rddata,
    input  wire  [7:0] chram_wrdata,
    input  wire        chram_wren,

    // Video RAM interface
    input  wire [13:0] vram_addr,
    output wire  [7:0] vram_rddata,
    input  wire  [7:0] vram_wrdata,
    input  wire        vram_wren,

    // VGA output
    output reg   [3:0] vga_r,
    output reg   [3:0] vga_g,
    output reg   [3:0] vga_b,
    output reg         vga_hsync,
    output reg         vga_vsync,
    
    output wire        vga_vblank);

    wire [7:0] vpos;
    wire       vblank;

    reg vblank_r;
    always @(posedge clk) vblank_r <= vblank;

    wire [7:0] rddata_vpaldata;
    wire [7:0] rddata_sprattr;

    reg        vctrl_text_priority_r;   // IO $E0 [4]
    reg        vctrl_sprites_enable_r;  // IO $E0 [3]
    reg  [1:0] vctrl_gfx_mode_r;        // IO $E0 [2:1]
    reg        vctrl_text_enable_r;     // IO $E0 [0]
    reg  [8:0] vscrx_r;                 // IO $E1/2
    reg  [7:0] vscry_r;                 // IO $E3
    reg  [6:0] vpalsel_r;               // IO $EA
    reg  [7:0] virqline_r;              // IO $ED
    reg        irqmask_line_r;          // IO $EE [0]
    reg        irqmask_vblank_r;        // IO $EE [1]
    reg        irqstat_line_r;          // IO $EF [0]
    reg        irqstat_vblank_r;        // IO $EF [1]

    wire irqline_match = (vpos == virqline_r);
    reg irqline_match_r;
    always @(posedge clk) irqline_match_r <= irqline_match;
    wire irqline_detect = (!irqline_match_r && irqline_match);

    assign irq = ({irqstat_line_r, irqstat_vblank_r} & {irqmask_line_r, irqmask_vblank_r}) != 2'b00;

    //////////////////////////////////////////////////////////////////////////
    // IO registers
    //////////////////////////////////////////////////////////////////////////
    wire sel_io_vctrl    = (io_addr == 4'h0);
    wire sel_io_vscrx_l  = (io_addr == 4'h1);
    wire sel_io_vscrx_h  = (io_addr == 4'h2);
    wire sel_io_vscry    = (io_addr == 4'h3);
    wire sel_io_vpalsel  = (io_addr == 4'hA);
    wire sel_io_vpaldata = (io_addr == 4'hB);
    wire sel_io_vline    = (io_addr == 4'hC);
    wire sel_io_virqline = (io_addr == 4'hD);
    wire sel_io_irqmask  = (io_addr == 4'hE);
    wire sel_io_irqstat  = (io_addr == 4'hF);

    always @* begin
        io_rddata <= rddata_sprattr;
        if (sel_io_vctrl)    io_rddata <= {3'b0, vctrl_text_priority_r, vctrl_sprites_enable_r, vctrl_gfx_mode_r, vctrl_text_enable_r};
        if (sel_io_vscrx_l)  io_rddata <= vscrx_r[7:0];                             // IO $E1
        if (sel_io_vscrx_h)  io_rddata <= {7'b0, vscrx_r[8]};                       // IO $E2
        if (sel_io_vscry)    io_rddata <= vscry_r;                                  // IO $E3
        if (sel_io_vpalsel)  io_rddata <= {1'b0, vpalsel_r};                        // IO $EA
        if (sel_io_vpaldata) io_rddata <= rddata_vpaldata;                          // IO $EB
        if (sel_io_vline)    io_rddata <= vpos;                                     // IO $EC
        if (sel_io_virqline) io_rddata <= virqline_r;                               // IO $ED
        if (sel_io_irqmask)  io_rddata <= {6'b0, irqmask_line_r, irqmask_vblank_r}; // IO $EE
        if (sel_io_irqstat)  io_rddata <= {6'b0, irqstat_line_r, irqstat_vblank_r}; // IO $EF
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            vctrl_text_priority_r  <= 1'b0;
            vctrl_sprites_enable_r <= 1'b0;
            vctrl_gfx_mode_r       <= 2'b0;
            vctrl_text_enable_r    <= 1'b0;
            vscrx_r                <= 9'b0;
            vscry_r                <= 8'b0;
            vpalsel_r              <= 7'b0;
            virqline_r             <= 8'b0;
            irqmask_line_r         <= 1'b0;
            irqmask_vblank_r       <= 1'b0;
            irqstat_line_r         <= 1'b0;
            irqstat_vblank_r       <= 1'b0;

        end else begin
            if (io_wren) begin
                if (sel_io_vctrl) begin
                    vctrl_text_priority_r  <= io_wrdata[4];
                    vctrl_sprites_enable_r <= io_wrdata[3];
                    vctrl_gfx_mode_r       <= io_wrdata[2:1];
                    vctrl_text_enable_r    <= io_wrdata[0];
                end
                if (sel_io_vscrx_l)  vscrx_r[7:0] <= io_wrdata;
                if (sel_io_vscrx_h)  vscrx_r[8]   <= io_wrdata[0];
                if (sel_io_vscry)    vscry_r      <= io_wrdata;
                if (sel_io_vpalsel)  vpalsel_r    <= io_wrdata[6:0];
                if (sel_io_virqline) virqline_r   <= io_wrdata;
                if (sel_io_irqmask) begin
                    irqmask_line_r   <= io_wrdata[1];
                    irqmask_vblank_r <= io_wrdata[0];
                end
                if (sel_io_irqstat) begin
                    irqstat_line_r   <= irqstat_line_r   & !io_wrdata[1];
                    irqstat_vblank_r <= irqstat_vblank_r & !io_wrdata[0];
                end
            end

            if (!irqline_match_r && irqline_match) irqstat_line_r   <= 1'b1;
            if (!vblank_r        && vblank)        irqstat_vblank_r <= 1'b1;
        end

    //////////////////////////////////////////////////////////////////////////
    // Video timing
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] hpos;
    wire       hsync, hblank, hlast;
    wire       vsync, vnext;
    wire       blank;

    video_timing video_timing(
        .clk(clk),

        .hpos(hpos),
        .hsync(hsync),
        .hblank(hblank),
        .hlast(hlast),
        
        .vpos(vpos),
        .vsync(vsync),
        .vblank(vblank),
        .vnext(vnext),
        
        .blank(blank));

    assign vga_vblank = vblank;

    wire hborder = hpos < 9'd16 || hpos >= 9'd336;
    wire vborder = vpos < 9'd16 || vpos >= 9'd216;

    reg [8:0] hpos_r, hpos_rr;
    always @(posedge clk) hpos_r <= hpos;
    always @(posedge clk) hpos_rr <= hpos_r;

    reg blank_r, hsync_r, vsync_r;
    always @(posedge clk) blank_r <= blank;
    always @(posedge clk) hsync_r <= hsync;
    always @(posedge clk) vsync_r <= vsync;

    reg blank_rr, hsync_rr, vsync_rr;
    always @(posedge clk) blank_rr <= blank_r;
    always @(posedge clk) hsync_rr <= hsync_r;
    always @(posedge clk) vsync_rr <= vsync_r;

    //////////////////////////////////////////////////////////////////////////
    // Character address
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] row_addr_r = 10'd0;
    reg [9:0] char_addr_r = 10'd0;

    wire       next_row = (vpos >= 9'd23) && vnext && (vpos[2:0] == 3'd7);
    wire [9:0] row_addr_next = row_addr_r + 10'd40;

    always @(posedge(clk))
        if (vblank)
            row_addr_r <= 10'd0;
        else if (next_row)
            row_addr_r <= row_addr_next;

    wire       next_char = (hpos[2:0] == 3'd7);
    wire [5:0] column = hpos[8:3];

    always @(posedge(clk))
        if (next_char) begin
            if (vborder || column == 6'd0 || column >= 6'd41)
                char_addr_r <= 10'd0;
            else if (column == 6'd1)
                char_addr_r <= row_addr_r;
            else
                char_addr_r <= char_addr_r + 10'd1;
        end

    //////////////////////////////////////////////////////////////////////////
    // Text RAM
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] textram_rddata;
    wire [10:0] p1_addr = {tram_addr[9:0], tram_addr[10]};

    textram textram(
        .p1_clk(clk),
        .p1_addr(p1_addr),
        .p1_rddata(tram_rddata),
        .p1_wrdata(tram_wrdata),
        .p1_wren(tram_wren),

        .p2_clk(clk),
        .p2_addr(char_addr_r),
        .p2_rddata(textram_rddata));

    wire [7:0] text_data  = textram_rddata[7:0];
    wire [7:0] color_data = textram_rddata[15:8];

    reg [7:0] color_data_r;
    always @(posedge clk) color_data_r <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Character RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos[2:0]};
    wire  [7:0] charram_data;

    charram charram(
        .clk(clk),

        .addr1(chram_addr),
        .rddata1(chram_rddata),
        .wrdata1(chram_wrdata),
        .wren1(chram_wren),

        .addr2(charram_addr),
        .rddata2(charram_data));

    wire [2:0] pixel_sel    = hpos_rr[2:0] ^ 3'b111;
    wire       char_pixel   = charram_data[pixel_sel];
    wire [3:0] text_colidx  = char_pixel ? color_data_r[7:4] : color_data_r[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Sprite attribute RAM
    //////////////////////////////////////////////////////////////////////////
    wire  [5:0] spr_sel;
    wire  [8:0] spr_x;
    wire  [7:0] spr_y;
    wire  [8:0] spr_idx;
    wire        spr_enable;
    wire        spr_priority;
    wire  [1:0] spr_palette;
    wire        spr_h16;
    wire        spr_vflip;
    wire        spr_hflip;

    sprattr sprattr(
        .clk(clk),
        .reset(reset),

        .io_addr(io_addr),
        .io_rddata(rddata_sprattr),
        .io_wrdata(io_wrdata),
        .io_wren(io_wren),

        .spr_sel(spr_sel),
        .spr_x(spr_x),
        .spr_y(spr_y),
        .spr_idx(spr_idx),
        .spr_enable(spr_enable),
        .spr_priority(spr_priority),
        .spr_palette(spr_palette),
        .spr_h16(spr_h16),
        .spr_vflip(spr_vflip),
        .spr_hflip(spr_hflip)
    );

    //////////////////////////////////////////////////////////////////////////
    // VRAM
    //////////////////////////////////////////////////////////////////////////
    wire [12:0] vram_addr2;     // = 13'b0;
    wire [15:0] vram_rddata2;

    vram vram(
        // First port - CPU access
        .p1_clk(clk),
        .p1_addr(vram_addr),
        .p1_rddata(vram_rddata),
        .p1_wrdata(vram_wrdata),
        .p1_wren(vram_wren),

        // Second port - Video access
        .p2_clk(clk),
        .p2_addr(vram_addr2),
        .p2_rddata(vram_rddata2));

    //////////////////////////////////////////////////////////////////////////
    // Graphics
    //////////////////////////////////////////////////////////////////////////
    wire [5:0] linebuf_data;
    reg  [8:0] linebuf_rdidx;

    always @(posedge clk) linebuf_rdidx <= hpos - 9'd16;

    reg hborder_r, hborder_rr;
    always @(posedge clk) hborder_r <= hborder;
    always @(posedge clk) hborder_rr <= hborder_r;

    reg gfx_start_r;
    always @(posedge clk) gfx_start_r <= vnext;

    gfx gfx(
        .clk(clk),
        .reset(reset),

        // Register values
        .gfx_mode(vctrl_gfx_mode_r),
        .sprites_enable(vctrl_sprites_enable_r),
        .scrx(vscrx_r),
        .scry(vscry_r),

        // Sprite attribute interface
        .spr_sel(spr_sel),
        .spr_x(spr_x),
        .spr_y(spr_y),
        .spr_idx(spr_idx),
        .spr_enable(spr_enable),
        .spr_priority(spr_priority),
        .spr_palette(spr_palette),
        .spr_h16(spr_h16),
        .spr_vflip(spr_vflip),
        .spr_hflip(spr_hflip),

        // Video RAM interface
        .vaddr(vram_addr2),
        .vdata(vram_rddata2),

        // Render parameters
        .vline(vpos),
        .start(gfx_start_r),

        // Line buffer interface
        .linebuf_rdidx(linebuf_rdidx),
        .linebuf_data(linebuf_data));

    //////////////////////////////////////////////////////////////////////////
    // Compositing
    //////////////////////////////////////////////////////////////////////////
    reg  [5:0] pixel_colidx;
    reg        render_pixel;
    wire       active = !vborder && !hborder_rr;

    always @* begin
        render_pixel = (linebuf_data[3:0] != 4'd0) || (!vctrl_text_enable_r && vctrl_gfx_mode_r != 2'b00);

        pixel_colidx = 6'b0;
        if (vctrl_text_enable_r)
            pixel_colidx = {2'b0, text_colidx};

        if (active) begin
            if (vctrl_text_priority_r && (pixel_colidx[3:0] != 4'd0))
                render_pixel = 1'b0;

            if (render_pixel)
                pixel_colidx = linebuf_data;
        end 
    end

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] pal_r, pal_g, pal_b;

    palette palette(
        .clk(clk),

        .addr(vpalsel_r),
        .rddata(rddata_vpaldata),
        .wrdata(io_wrdata),
        .wren(io_wren && sel_io_vpaldata),

        .palidx(pixel_colidx),
        .pal_r(pal_r),
        .pal_g(pal_g),
        .pal_b(pal_b));

    //////////////////////////////////////////////////////////////////////////
    // Output registers
    //////////////////////////////////////////////////////////////////////////
    always @(posedge(clk))
        if (blank_rr) begin
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else begin
            vga_r <= pal_r;
            vga_g <= pal_g;
            vga_b <= pal_b;
        end

    always @(posedge clk) vga_hsync <= hsync_rr;
    always @(posedge clk) vga_vsync <= vsync_rr;

endmodule
