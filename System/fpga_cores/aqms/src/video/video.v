// TODO:
// - V-Scroll inhibit
// - Sprite shift
// - Sprite x2 magnify
//
// NTSC, 256x192
//
// | Lines | Description       | 
// | ----- | ----------------- |
// |  192  | Active display    |
// |   24  | Bottom border     |
// |    3  | Bottom blanking   |
// |    3  | Vertical blanking |
// |   13  | Top blanking      |
// |   27  | Top border        |
//
// V counter values: 00-DA, D5-FF


module video(
    input  wire        clk,
    input  wire        reset,

    input  wire        vclk,            // 25.175MHz

    // IO register interface
    input  wire        io_portsel,      // 0:data, 1:ctrl
    output reg   [7:0] io_rddata,
    input  wire  [7:0] io_wrdata,
    input  wire        io_wren,
    input  wire        io_wrdone,
    input  wire        io_rddone,
    output wire        irq,

    output wire [7:0]  vcnt,
    output wire [7:0]  hcnt,

    // VGA output
    output reg   [3:0] vga_r,
    output reg   [3:0] vga_g,
    output reg   [3:0] vga_b,
    output reg         vga_hsync,
    output reg         vga_vsync);

    wire [7:0] vram_rddata;

    //////////////////////////////////////////////////////////////////////////
    // Register interface
    //////////////////////////////////////////////////////////////////////////
    reg  [1:0] vdp_code_r;
    reg [13:0] vdp_addr_r;
    reg        bf_toggle_r;

    reg  [3:0] vdp_reg_idx_r;
    reg  [7:0] vdp_reg_data_r;
    reg        vdp_reg_wr_r;

    wire       vsync_irq_pend;
    wire       line_irq_pend;
    wire       spr_overflow;
    wire       spr_collision;

    reg        vsync_irq_pending_r;
    reg        line_irq_pending_r;
    reg        spr_overflow_r;
    reg        spr_collision_r;

    wire       ctrl_write  = io_wren   &&  io_portsel;
    wire       data_write  = io_wren   && !io_portsel;

    wire       ctrl_rddone = io_rddone &&  io_portsel;
    wire       data_rddone = io_rddone && !io_portsel;
    wire       data_wrdone = io_wrdone && !io_portsel;

    wire       vram_wren    = data_write && vdp_code_r != 2'd3;
    wire       palette_wren = data_write && vdp_code_r == 2'd3;

    reg        vsync_irq_en_r;
    reg        line_irq_en_r;

    wire [7:0] vdp_status = {vsync_irq_pending_r, spr_overflow_r, spr_collision_r, 5'b0};

    always @(posedge clk or posedge reset)
        if (reset) begin
            vdp_code_r          <= 2'b0;
            vdp_addr_r          <= 14'b0;
            bf_toggle_r         <= 1'b0;

            vdp_reg_idx_r       <= 4'd0;
            vdp_reg_data_r      <= 8'd0;
            vdp_reg_wr_r        <= 1'b0;

            vsync_irq_pending_r <= 1'b0;
            line_irq_pending_r  <= 1'b0;
            spr_overflow_r      <= 1'b0;
            spr_collision_r     <= 1'b0;
            vsync_irq_en_r      <= 1'b0;
            line_irq_en_r       <= 1'b0;

        end else begin
            vdp_reg_wr_r    <= 1'b0;

            if (ctrl_write) begin
                if (!bf_toggle_r)
                    vdp_addr_r <= {vdp_addr_r[13:8], io_wrdata};
                else begin
                    vdp_code_r <= io_wrdata[7:6];
                    vdp_addr_r <= {io_wrdata[5:0], vdp_addr_r[7:0]};

                    if (io_wrdata[7:6] == 2'd0) begin
                        // Read VDP RAM

                    end else if (io_wrdata[7:6] == 2'd2) begin
                        // Write VDP register
                        vdp_reg_idx_r  <= io_wrdata[3:0];
                        vdp_reg_data_r <= vdp_addr_r[7:0];
                        vdp_reg_wr_r   <= 1'b1;

                        if (io_wrdata[3:0] == 4'd0)
                            line_irq_en_r <= vdp_addr_r[4];
                        if (io_wrdata[3:0] == 4'd1)
                            vsync_irq_en_r <= vdp_addr_r[5];

                    end
                end

                bf_toggle_r <= !bf_toggle_r;
            end

            // Reset status after writing control port
            if (ctrl_rddone) begin
                bf_toggle_r         <= 1'b0;
                vsync_irq_pending_r <= 1'b0;
                line_irq_pending_r  <= 1'b0;
                spr_overflow_r      <= 1'b0;
                spr_collision_r     <= 1'b0;
            end

            // Increment VDP address after access to data port
            if (data_rddone || data_wrdone) begin
                bf_toggle_r <= 1'b0;
                vdp_addr_r  <= vdp_addr_r + 14'd1;
            end

            // Latch interrupt pend signals
            if (vsync_irq_pend) vsync_irq_pending_r <= 1'b1;
            if (line_irq_pend)  line_irq_pending_r  <= 1'b1;
            if (spr_overflow)   spr_overflow_r      <= 1'b1;
            if (spr_collision)  spr_collision_r     <= 1'b1;
        end

    // Generate interrupt signal
    assign irq = (vsync_irq_pending_r && vsync_irq_en_r) || (line_irq_pending_r && line_irq_en_r);

    always @* io_rddata <= io_portsel ? vdp_status : vram_rddata;

    //////////////////////////////////////////////////////////////////////////
    // VRAM
    //////////////////////////////////////////////////////////////////////////
    wire [12:0] vram_addr2;
    wire [15:0] vram_rddata2;

    vram vram(
        // First port - CPU access
        .p1_clk(clk),
        .p1_addr(vdp_addr_r),
        .p1_rddata(vram_rddata),
        .p1_wrdata(io_wrdata),
        .p1_wren(vram_wren),

        // Second port - Video access
        .p2_clk(vclk),
        .p2_addr(vram_addr2),
        .p2_rddata(vram_rddata2));

    //////////////////////////////////////////////////////////////////////////
    // Sync reset to vclk domain
    //////////////////////////////////////////////////////////////////////////
    wire vclk_reset;
    reset_sync ressync_vclk(.async_rst_in(reset), .clk(vclk), .reset_out(vclk_reset));

    //////////////////////////////////////////////////////////////////////////
    // Sync register write to vclk domain
    //////////////////////////////////////////////////////////////////////////
    wire vdp_reg_wr;
    pulse2pulse p2p_reg_wr(.in_clk(clk), .in_pulse(vdp_reg_wr_r), .out_clk(vclk), .out_pulse(vdp_reg_wr));

    //////////////////////////////////////////////////////////////////////////
    // VDP registers
    //////////////////////////////////////////////////////////////////////////
    reg       reg0_vscroll_inhibit_r;   // 1: Disable vertical scrolling for columns 24-31
    reg       reg0_hscroll_inhibit_r;   // 1: Disable horizontal scrolling for rows 0-1
    reg       reg0_left_col_blank_r;    // 1: Mask column 0 with overscan color from register #7
    reg       reg0_line_irq_en_r;       // 1: Line interrupt enable
    reg       reg0_sprite_shift_bit_r;  // 1: Shift sprites left by 8 pixels
    reg       reg1_screen_en_r;         // 1: Display visible, 0: display blanked.
    reg       reg1_vblank_irq_en_r;     // 1: Frame interrupt enable.
    reg       reg1_spr_h16_r;           // Sprites are 1:8x16, 0:8x8
    reg       reg1_spr_mag_r;           // Sprite pixels are doubled in size
    reg [7:0] reg2_scrmap_base_r;       // [3:1] = bit [13:11] of Name Table Base Address
    reg [7:0] reg5_sprattr_base_r;      // [6:1] = bit [13:8] of Sprite Attribute Table Base Address
    reg [7:0] reg6_sprpat_base_r;       // [2] = bit [13] of Sprite Pattern Generator Base Address
    reg [3:0] reg7_border_colidx_r;     // [3:0] = Overscan/Backdrop Color
    reg [7:0] reg8_hscroll_r;           // Background X Scroll
    reg [7:0] reg9_vscroll_r;           // Background Y Scroll
    reg [7:0] reg10_rasterirq_line_r;   // Line counter

    always @(posedge vclk or posedge vclk_reset)
        if (vclk_reset) begin
            reg0_vscroll_inhibit_r  <= 1'b0;
            reg0_hscroll_inhibit_r  <= 1'b0;
            reg0_left_col_blank_r   <= 1'b0;
            reg0_line_irq_en_r      <= 1'b0;
            reg0_sprite_shift_bit_r <= 1'b0;
            reg1_screen_en_r        <= 1'b0;
            reg1_vblank_irq_en_r    <= 1'b0;
            reg1_spr_h16_r          <= 1'b0;
            reg1_spr_mag_r          <= 1'b0;
            reg2_scrmap_base_r      <= 8'b0;
            reg5_sprattr_base_r     <= 8'b0;
            reg6_sprpat_base_r      <= 8'b0;
            reg7_border_colidx_r    <= 4'b0;
            reg8_hscroll_r          <= 8'b0;
            reg9_vscroll_r          <= 8'b0;
            reg10_rasterirq_line_r  <= 8'b0;

        end else begin
            if (vdp_reg_wr) begin
                if (vdp_reg_idx_r == 4'd0) begin
                    reg0_vscroll_inhibit_r  <= vdp_reg_data_r[7];
                    reg0_hscroll_inhibit_r  <= vdp_reg_data_r[6];
                    reg0_left_col_blank_r   <= vdp_reg_data_r[5];
                    reg0_line_irq_en_r      <= vdp_reg_data_r[4];
                    reg0_sprite_shift_bit_r <= vdp_reg_data_r[3];
                end
                if (vdp_reg_idx_r == 4'd1) begin
                    reg1_screen_en_r        <= vdp_reg_data_r[6];
                    reg1_vblank_irq_en_r    <= vdp_reg_data_r[5];
                    reg1_spr_h16_r          <= vdp_reg_data_r[1];
                    reg1_spr_mag_r          <= vdp_reg_data_r[0];
                end
                if (vdp_reg_idx_r == 4'd2)  reg2_scrmap_base_r     <= vdp_reg_data_r;
                if (vdp_reg_idx_r == 4'd5)  reg5_sprattr_base_r    <= vdp_reg_data_r;
                if (vdp_reg_idx_r == 4'd6)  reg6_sprpat_base_r     <= vdp_reg_data_r;
                if (vdp_reg_idx_r == 4'd7)  reg7_border_colidx_r   <= vdp_reg_data_r[3:0];
                if (vdp_reg_idx_r == 4'd8)  reg8_hscroll_r         <= vdp_reg_data_r;
                if (vdp_reg_idx_r == 4'd9)  reg9_vscroll_r         <= vdp_reg_data_r;
                if (vdp_reg_idx_r == 4'd10) reg10_rasterirq_line_r <= vdp_reg_data_r;
            end
        end

    //////////////////////////////////////////////////////////////////////////
    // Video timing
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] hpos;
    wire [7:0] vpos;
    wire [7:0] render_line;
    wire       render_start;
    wire       vblank_irq_pulse;
    wire       next_line, hsync, vsync, border, blank;

    video_timing video_timing(
        .clk(vclk),
        .left_col_blank(reg0_left_col_blank_r),

        .hpos(hpos),
        .vpos(vpos),

        .render_line(render_line),
        .render_start(render_start),
        .vblank_irq_pulse(vblank_irq_pulse),

        .next_line(next_line),
        .hsync(hsync),
        .vsync(vsync),
        .border(border),
        .blank(blank));

    assign vcnt = vpos;
    assign hcnt = hpos;

    // Delay signals to compensate for pipeline delays
    reg hsync_r, vsync_r, border_r, blank_r;
    always @(posedge vclk) hsync_r  <= hsync;
    always @(posedge vclk) vsync_r  <= vsync;
    always @(posedge vclk) border_r <= border;
    always @(posedge vclk) blank_r  <= blank;

    //////////////////////////////////////////////////////////////////////////
    // Line IRQ 
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] lineirq_cnt_r;

    always @(posedge (vclk)) begin
        vid_line_irq_pend <= 1'b0;

        if (next_line) begin
            if (vpos <= 8'd192) begin
                if (lineirq_cnt_r == 8'd0) begin
                    vid_line_irq_pend <= 1'b1;
                    lineirq_cnt_r <= reg10_rasterirq_line_r;
                end else begin
                    lineirq_cnt_r <= lineirq_cnt_r - 8'd1;
                end

            end else begin
                lineirq_cnt_r <= reg10_rasterirq_line_r;
            end
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Synchronization of signal from vclk to clk domain
    //////////////////////////////////////////////////////////////////////////
    reg  vid_line_irq_pend;
    wire vid_spr_overflow;
    wire vid_spr_collision;

    pulse2pulse p2p_vsync_irq(     .in_clk(vclk), .in_pulse(vblank_irq_pulse),   .out_clk(clk), .out_pulse(vsync_irq_pend));
    pulse2pulse p2p_line_irq(      .in_clk(vclk), .in_pulse(vid_line_irq_pend),  .out_clk(clk), .out_pulse(line_irq_pend));
    pulse2pulse p2p_spr_overflow(  .in_clk(vclk), .in_pulse(vid_spr_overflow),   .out_clk(clk), .out_pulse(spr_overflow));
    pulse2pulse p2p_spr_collision( .in_clk(vclk), .in_pulse(vid_spr_collision),  .out_clk(clk), .out_pulse(spr_collision));

    //////////////////////////////////////////////////////////////////////////
    // Graphics
    //////////////////////////////////////////////////////////////////////////
    wire [4:0] linebuf_data;

    gfx gfx(
        .clk(vclk),
        .reset(vclk_reset),

        // Register values
        .hscroll_inhibit(reg0_hscroll_inhibit_r),
        .hscroll(reg8_hscroll_r),
        .vscroll(reg9_vscroll_r),
        .base_nt(reg2_scrmap_base_r[3:1]),
        .base_sprattr(reg5_sprattr_base_r[6:1]),
        .base_sprpat(reg6_sprpat_base_r[2]),
        .spr_h16(reg1_spr_h16_r),

        // Render parameters
        .line(render_line),
        .start(render_start),

        // Output signals
        .spr_overflow(vid_spr_overflow),
        .spr_collision(vid_spr_collision),

        // VRAM interface
        .vaddr(vram_addr2),
        .vdata(vram_rddata2),

        // Line buffer interface
        .linebuf_rdidx(hpos),
        .linebuf_data(linebuf_data)
    );

    reg reg1_screen_en_rr;
    always @(posedge (vclk)) if (render_start) reg1_screen_en_rr <= reg1_screen_en_r;

    wire [4:0] palidx = (!reg1_screen_en_rr || border_r) ? {1'b1, reg7_border_colidx_r} : linebuf_data;

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [1:0] pal_r, pal_g, pal_b;

    palette palette(
        .clk(clk),
        .addr(vdp_addr_r[4:0]),
        .wrdata(io_wrdata),
        .wren(palette_wren),

        .palidx(palidx),
        .pal_r(pal_r),
        .pal_g(pal_g),
        .pal_b(pal_b));

    //////////////////////////////////////////////////////////////////////////
    // Output registers
    //////////////////////////////////////////////////////////////////////////
    always @(posedge(vclk))
        if (blank_r) begin
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else begin
            vga_r <= {pal_r, pal_r};
            vga_g <= {pal_g, pal_g};
            vga_b <= {pal_b, pal_b};
        end

    always @(posedge vclk) vga_hsync <= hsync_r;
    always @(posedge vclk) vga_vsync <= vsync_r;

endmodule
