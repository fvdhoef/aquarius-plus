module video(
    input  wire        clk,
    input  wire        reset,

    input  wire        vclk,            // 28.63636MHz (video_mode = 0) or 25.175MHz (video_mode = 1)
    input  wire        video_mode,

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
    
    // Register $FD value
    output wire        reg_fd_val);

    // Sync reset to vclk domain
    wire vclk_reset;
    reset_sync ressync_vclk(.async_rst_in(reset), .clk(vclk), .reset_out(vclk_reset));


    wire [7:0] vpos;
    wire       vblank;

    reg vblank_r;
    always @(posedge vclk) vblank_r <= vblank;

    wire [7:0] rddata_vpaldata;
    wire [7:0] rddata_sprattr;

    reg        vctrl_tram_page_r;       // IO $E0 [7]
    reg        vctrl_80_columns_r;      // IO $E0 [6]
    reg        vctrl_border_remap_r;    // IO $E0 [5]
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
    always @(posedge vclk) irqline_match_r <= irqline_match;

    wire irq_line, irq_vblank;
    pulse2pulse p2p_irq_line(  .in_clk(vclk), .in_pulse(!irqline_match_r && irqline_match), .out_clk(clk), .out_pulse(irq_line));
    pulse2pulse p2p_irq_vblank(.in_clk(vclk), .in_pulse(!vblank_r        && vblank),        .out_clk(clk), .out_pulse(irq_vblank));

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
        if (sel_io_vctrl)    io_rddata <= {vctrl_tram_page_r, vctrl_80_columns_r, vctrl_border_remap_r, vctrl_text_priority_r, vctrl_sprites_enable_r, vctrl_gfx_mode_r, vctrl_text_enable_r};
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
            vctrl_tram_page_r      <= 1'b0;
            vctrl_80_columns_r     <= 1'b0;
            vctrl_border_remap_r   <= 1'b0;
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
                    vctrl_tram_page_r      <= io_wrdata[7];
                    vctrl_80_columns_r     <= io_wrdata[6];
                    vctrl_border_remap_r   <= io_wrdata[5];
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

            if (irq_line)   irqstat_line_r   <= 1'b1;
            if (irq_vblank) irqstat_vblank_r <= 1'b1;
        end

    //////////////////////////////////////////////////////////////////////////
    // Video timing
    //////////////////////////////////////////////////////////////////////////
    wire [9:0] hpos;
    wire       hsync, hblank, hlast;
    wire       vsync, vnext;
    wire       blank;

    video_timing video_timing(
        .clk(vclk),
        .mode(video_mode),

        .hpos(hpos),
        .hsync(hsync),
        .hblank(hblank),
        .hlast(hlast),
        
        .vpos(vpos),
        .vsync(vsync),
        .vblank(vblank),
        .vnext(vnext),
        
        .blank(blank));

    wire hborder = video_mode ? blank : (hpos < 10'd32 || hpos >= 10'd672);
    wire vborder = vpos < 9'd16 || vpos >= 9'd216;

    reg [9:0] hpos_r, hpos_rr;
    always @(posedge vclk) hpos_r <= hpos;
    always @(posedge vclk) hpos_rr <= hpos_r;

    reg blank_r, hsync_r, vsync_r;
    always @(posedge vclk) blank_r <= blank;
    always @(posedge vclk) hsync_r <= hsync;
    always @(posedge vclk) vsync_r <= vsync;

    reg blank_rr, hsync_rr, vsync_rr;
    always @(posedge vclk) blank_rr <= blank_r;
    always @(posedge vclk) hsync_rr <= hsync_r;
    always @(posedge vclk) vsync_rr <= vsync_r;

    //////////////////////////////////////////////////////////////////////////
    // Character address
    //////////////////////////////////////////////////////////////////////////
    reg  mode80_r = 1'b0;

    reg  [10:0] row_addr_r  = 11'd0;
    reg  [10:0] char_addr_r = 11'd0;

    wire        next_row         = (vpos >= 9'd23) && vnext && (vpos[2:0] == 3'd7);
    wire [10:0] row_addr_next    = row_addr_r + (mode80_r ? 11'd80 : 11'd40);
    wire [10:0] border_char_addr = vctrl_border_remap_r ? (mode80_r ? 11'h7FF : 11'h3FF) : 11'h0;

    always @(posedge(vclk))
        if (vblank) begin
            mode80_r <= vctrl_80_columns_r;
            row_addr_r <= 11'd0;
        end else if (next_row)
            row_addr_r <= row_addr_next;

    wire       next_char = mode80_r ? (hpos[2:0] == 3'd0) : (hpos[3:0] == 4'd0);
    wire [6:0] column    = mode80_r ? hpos[9:3] : {1'b0, hpos[9:4]};

    wire border = vborder || hborder;
    reg border_r;
    always @(posedge vclk) border_r <= border;

    wire start_active = border_r && !border;

    reg  [10:0] char_addr_next;
    always @* begin
        char_addr_next = char_addr_r;
        if (border)
            char_addr_next = border_char_addr;
        else if (start_active)
            char_addr_next = row_addr_r;
        else if (next_char)
            char_addr_next = char_addr_r + 11'd1;

        if (!mode80_r)
            char_addr_next[10] = vctrl_tram_page_r;
    end

    always @(posedge(vclk)) char_addr_r <= char_addr_next;


    //////////////////////////////////////////////////////////////////////////
    // Text RAM
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] textram_rddata;
    wire [11:0] tram_p1_addr = vctrl_80_columns_r ? {tram_addr[10:0], vctrl_tram_page_r} : {vctrl_tram_page_r, tram_addr[9:0], tram_addr[10]};

    textram textram(
        // First port - CPU access
        .p1_clk(clk),
        .p1_addr(tram_p1_addr),
        .p1_rddata(tram_rddata),
        .p1_wrdata(tram_wrdata),
        .p1_wren(tram_wren),

        // Second port - Video access
        .p2_clk(vclk),
        .p2_addr(char_addr_next),
        .p2_rddata(textram_rddata));

    wire [7:0] text_data  = textram_rddata[7:0];
    wire [7:0] color_data = textram_rddata[15:8];

    reg [7:0] color_data_r;
    always @(posedge vclk) color_data_r <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Character RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos[2:0]};
    wire  [7:0] charram_data;

    charram charram(
        .clk(vclk),

        .addr1(chram_addr),
        .rddata1(chram_rddata),
        .wrdata1(chram_wrdata),
        .wren1(chram_wren),

        .addr2(charram_addr),
        .rddata2(charram_data));

    wire [2:0] pixel_sel    = (mode80_r ? hpos_rr[2:0] : hpos_rr[3:1]) ^ 3'b111;
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
        // First port - CPU access
        .clk(clk),
        .reset(reset),
        .io_addr(io_addr),
        .io_rddata(rddata_sprattr),
        .io_wrdata(io_wrdata),
        .io_wren(io_wren),

        // Second port - Video access
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
    wire [12:0] vram_addr2;
    wire [15:0] vram_rddata2;

    vram vram(
        // First port - CPU access
        .p1_clk(clk),
        .p1_addr(vram_addr),
        .p1_rddata(vram_rddata),
        .p1_wrdata(vram_wrdata),
        .p1_wren(vram_wren),

        // Second port - Video access
        .p2_clk(vclk),
        .p2_addr(vram_addr2),
        .p2_rddata(vram_rddata2));

    //////////////////////////////////////////////////////////////////////////
    // Graphics
    //////////////////////////////////////////////////////////////////////////
    wire [5:0] linebuf_data;
    reg  [8:0] linebuf_rdidx;

    always @(posedge vclk) linebuf_rdidx <= video_mode ? hpos[9:1] : (hpos[9:1] - 9'd16);

    reg hborder_r, hborder_rr;
    always @(posedge vclk) hborder_r <= hborder;
    always @(posedge vclk) hborder_rr <= hborder_r;

    reg gfx_start_r;
    always @(posedge vclk) gfx_start_r <= vnext;

    gfx gfx(
        .clk(vclk),
        .reset(vclk_reset),

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
    wire       active = !vborder && !hborder_rr;

    assign reg_fd_val = !vborder;

    always @* begin
        pixel_colidx = 6'b0;
        if (!active) begin
            if (vctrl_text_enable_r)
                pixel_colidx = {2'b0, text_colidx};

        end else begin
            if (vctrl_text_enable_r && !vctrl_text_priority_r)
                pixel_colidx = {2'b0, text_colidx};
            if (!vctrl_text_enable_r || vctrl_text_priority_r || linebuf_data[3:0] != 4'd0)
                pixel_colidx = linebuf_data;
            if (vctrl_text_enable_r && vctrl_text_priority_r && text_colidx != 4'd0)
                pixel_colidx = {2'b0, text_colidx};
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
    always @(posedge(vclk))
        if (blank_rr) begin
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else begin
            vga_r <= pal_r;
            vga_g <= pal_g;
            vga_b <= pal_b;
        end

    always @(posedge vclk) vga_hsync <= hsync_rr;
    always @(posedge vclk) vga_vsync <= vsync_rr;

endmodule
