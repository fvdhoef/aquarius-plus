// TODO:
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

`default_nettype none
`timescale 1 ns / 1 ps

module video(
    input  wire        clk,
    input  wire        reset,

    input  wire        video_clk,            // 25.175MHz

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
    reg  [1:0] q_vdp_code;
    reg [13:0] q_vdp_addr;
    reg        q_bf_toggle;

    reg  [3:0] q_vdp_reg_idx;
    reg  [7:0] q_vdp_reg_data;
    reg        q_vdp_reg_wr;

    wire       vsync_irq_pend;
    wire       line_irq_pend;
    wire       spr_overflow;
    wire       spr_collision;

    reg        q_vsync_irq_pending;
    reg        q_line_irq_pending;
    reg        q_spr_overflow;
    reg        q_spr_collision;

    wire       ctrl_write  = io_wren   &&  io_portsel;
    wire       data_write  = io_wren   && !io_portsel;

    wire       ctrl_rddone = io_rddone &&  io_portsel;
    wire       data_rddone = io_rddone && !io_portsel;
    wire       data_wrdone = io_wrdone && !io_portsel;

    wire       vram_wren    = data_write && q_vdp_code != 2'd3;
    wire       palette_wren = data_write && q_vdp_code == 2'd3;

    reg        q_vsync_irq_en;
    reg        q_line_irq_en;

    wire [7:0] vdp_status = {q_vsync_irq_pending, q_spr_overflow, q_spr_collision, 5'b0};

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_vdp_code          <= 2'b0;
            q_vdp_addr          <= 14'b0;
            q_bf_toggle         <= 1'b0;

            q_vdp_reg_idx       <= 4'd0;
            q_vdp_reg_data      <= 8'd0;
            q_vdp_reg_wr        <= 1'b0;

            q_vsync_irq_pending <= 1'b0;
            q_line_irq_pending  <= 1'b0;
            q_spr_overflow      <= 1'b0;
            q_spr_collision     <= 1'b0;
            q_vsync_irq_en      <= 1'b0;
            q_line_irq_en       <= 1'b0;

        end else begin
            q_vdp_reg_wr    <= 1'b0;

            if (ctrl_write) begin
                if (!q_bf_toggle)
                    q_vdp_addr <= {q_vdp_addr[13:8], io_wrdata};
                else begin
                    q_vdp_code <= io_wrdata[7:6];
                    q_vdp_addr <= {io_wrdata[5:0], q_vdp_addr[7:0]};

                    if (io_wrdata[7:6] == 2'd0) begin
                        // Read VDP RAM

                    end else if (io_wrdata[7:6] == 2'd2) begin
                        // Write VDP register
                        q_vdp_reg_idx  <= io_wrdata[3:0];
                        q_vdp_reg_data <= q_vdp_addr[7:0];
                        q_vdp_reg_wr   <= 1'b1;

                        if (io_wrdata[3:0] == 4'd0)
                            q_line_irq_en <= q_vdp_addr[4];
                        if (io_wrdata[3:0] == 4'd1)
                            q_vsync_irq_en <= q_vdp_addr[5];

                    end
                end

                q_bf_toggle <= !q_bf_toggle;
            end

            // Reset status after writing control port
            if (ctrl_rddone) begin
                q_bf_toggle         <= 1'b0;
                q_vsync_irq_pending <= 1'b0;
                q_line_irq_pending  <= 1'b0;
                q_spr_overflow      <= 1'b0;
                q_spr_collision     <= 1'b0;
            end

            // Increment VDP address after access to data port
            if (data_rddone || data_wrdone) begin
                q_bf_toggle <= 1'b0;
                q_vdp_addr  <= q_vdp_addr + 14'd1;
            end

            // Latch interrupt pend signals
            if (vsync_irq_pend) q_vsync_irq_pending <= 1'b1;
            if (line_irq_pend)  q_line_irq_pending  <= 1'b1;
            if (spr_overflow)   q_spr_overflow      <= 1'b1;
            if (spr_collision)  q_spr_collision     <= 1'b1;
        end

    // Generate interrupt signal
    assign irq = (q_vsync_irq_pending && q_vsync_irq_en) || (q_line_irq_pending && q_line_irq_en);

    always @* io_rddata = io_portsel ? vdp_status : vram_rddata;

    //////////////////////////////////////////////////////////////////////////
    // VRAM
    //////////////////////////////////////////////////////////////////////////
    wire [12:0] vram_addr2;
    wire [15:0] vram_rddata2;

    vram vram(
        // First port - CPU access
        .p1_clk(clk),
        .p1_addr(q_vdp_addr),
        .p1_rddata(vram_rddata),
        .p1_wrdata(io_wrdata),
        .p1_wren(vram_wren),

        // Second port - Video access
        .p2_clk(video_clk),
        .p2_addr(vram_addr2),
        .p2_rddata(vram_rddata2));

    //////////////////////////////////////////////////////////////////////////
    // Sync reset to video_clk domain
    //////////////////////////////////////////////////////////////////////////
    wire vclk_reset;
    reset_sync ressync_vclk(.async_rst_in(reset), .clk(video_clk), .reset_out(vclk_reset));

    //////////////////////////////////////////////////////////////////////////
    // Sync register write to video_clk domain
    //////////////////////////////////////////////////////////////////////////
    wire vdp_reg_wr;
    pulse2pulse p2p_reg_wr(.in_clk(clk), .in_pulse(q_vdp_reg_wr), .out_clk(video_clk), .out_pulse(vdp_reg_wr));

    //////////////////////////////////////////////////////////////////////////
    // VDP registers
    //////////////////////////////////////////////////////////////////////////
    reg       q_reg0_vscroll_inhibit;   // 1: Disable vertical scrolling for columns 24-31
    reg       q_reg0_hscroll_inhibit;   // 1: Disable horizontal scrolling for rows 0-1
    reg       q_reg0_left_col_blank;    // 1: Mask column 0 with overscan color from register #7
    reg       q_reg0_line_irq_en;       // 1: Line interrupt enable
    reg       q_reg0_sprite_shift_bit;  // 1: Shift sprites left by 8 pixels
    reg       q_reg1_screen_en;         // 1: Display visible, 0: display blanked.
    reg       q_reg1_vblank_irq_en;     // 1: Frame interrupt enable.
    reg       q_reg1_spr_h16;           // Sprites are 1:8x16, 0:8x8
    reg       q_reg1_spr_mag;           // Sprite pixels are doubled in size
    reg [7:0] q_reg2_scrmap_base;       // [3:1] = bit [13:11] of Name Table Base Address
    reg [7:0] q_reg5_sprattr_base;      // [6:1] = bit [13:8] of Sprite Attribute Table Base Address
    reg [7:0] q_reg6_sprpat_base;       // [2] = bit [13] of Sprite Pattern Generator Base Address
    reg [3:0] q_reg7_border_colidx;     // [3:0] = Overscan/Backdrop Color
    reg [7:0] q_reg8_hscroll;           // Background X Scroll
    reg [7:0] q_reg9_vscroll;           // Background Y Scroll
    reg [7:0] q_reg10_rasterirq_line;   // Line counter

    always @(posedge video_clk or posedge vclk_reset)
        if (vclk_reset) begin
            q_reg0_vscroll_inhibit  <= 1'b0;
            q_reg0_hscroll_inhibit  <= 1'b0;
            q_reg0_left_col_blank   <= 1'b0;
            q_reg0_line_irq_en      <= 1'b0;
            q_reg0_sprite_shift_bit <= 1'b0;
            q_reg1_screen_en        <= 1'b0;
            q_reg1_vblank_irq_en    <= 1'b0;
            q_reg1_spr_h16          <= 1'b0;
            q_reg1_spr_mag          <= 1'b0;
            q_reg2_scrmap_base      <= 8'b0;
            q_reg5_sprattr_base     <= 8'b0;
            q_reg6_sprpat_base      <= 8'b0;
            q_reg7_border_colidx    <= 4'b0;
            q_reg8_hscroll          <= 8'b0;
            q_reg9_vscroll          <= 8'b0;
            q_reg10_rasterirq_line  <= 8'b0;

        end else begin
            if (vdp_reg_wr) begin
                if (q_vdp_reg_idx == 4'd0) begin
                    q_reg0_vscroll_inhibit  <= q_vdp_reg_data[7];
                    q_reg0_hscroll_inhibit  <= q_vdp_reg_data[6];
                    q_reg0_left_col_blank   <= q_vdp_reg_data[5];
                    q_reg0_line_irq_en      <= q_vdp_reg_data[4];
                    q_reg0_sprite_shift_bit <= q_vdp_reg_data[3];
                end
                if (q_vdp_reg_idx == 4'd1) begin
                    q_reg1_screen_en        <= q_vdp_reg_data[6];
                    q_reg1_vblank_irq_en    <= q_vdp_reg_data[5];
                    q_reg1_spr_h16          <= q_vdp_reg_data[1];
                    q_reg1_spr_mag          <= q_vdp_reg_data[0];
                end
                if (q_vdp_reg_idx == 4'd2)  q_reg2_scrmap_base     <= q_vdp_reg_data;
                if (q_vdp_reg_idx == 4'd5)  q_reg5_sprattr_base    <= q_vdp_reg_data;
                if (q_vdp_reg_idx == 4'd6)  q_reg6_sprpat_base     <= q_vdp_reg_data;
                if (q_vdp_reg_idx == 4'd7)  q_reg7_border_colidx   <= q_vdp_reg_data[3:0];
                if (q_vdp_reg_idx == 4'd8)  q_reg8_hscroll         <= q_vdp_reg_data;
                if (q_vdp_reg_idx == 4'd9)  q_reg9_vscroll         <= q_vdp_reg_data;
                if (q_vdp_reg_idx == 4'd10) q_reg10_rasterirq_line <= q_vdp_reg_data;
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
        .clk(video_clk),
        .left_col_blank(q_reg0_left_col_blank),

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
    reg q_hsync, q_vsync, q_border, q_blank;
    always @(posedge video_clk) q_hsync  <= hsync;
    always @(posedge video_clk) q_vsync  <= vsync;
    always @(posedge video_clk) q_border <= border;
    always @(posedge video_clk) q_blank  <= blank;

    //////////////////////////////////////////////////////////////////////////
    // Line IRQ 
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] q_lineirq_cnt;

    always @(posedge (video_clk)) begin
        vid_line_irq_pend <= 1'b0;

        if (next_line) begin
            if (vpos <= 8'd192) begin
                if (q_lineirq_cnt == 8'd0) begin
                    vid_line_irq_pend <= 1'b1;
                    q_lineirq_cnt <= q_reg10_rasterirq_line;
                end else begin
                    q_lineirq_cnt <= q_lineirq_cnt - 8'd1;
                end

            end else begin
                q_lineirq_cnt <= q_reg10_rasterirq_line;
            end
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Synchronization of signal from video_clk to clk domain
    //////////////////////////////////////////////////////////////////////////
    reg  vid_line_irq_pend;
    wire vid_spr_overflow;
    wire vid_spr_collision;

    pulse2pulse p2p_vsync_irq(    .in_clk(video_clk), .in_pulse(vblank_irq_pulse),  .out_clk(clk), .out_pulse(vsync_irq_pend));
    pulse2pulse p2p_line_irq(     .in_clk(video_clk), .in_pulse(vid_line_irq_pend), .out_clk(clk), .out_pulse(line_irq_pend));
    pulse2pulse p2p_spr_overflow( .in_clk(video_clk), .in_pulse(vid_spr_overflow),  .out_clk(clk), .out_pulse(spr_overflow));
    pulse2pulse p2p_spr_collision(.in_clk(video_clk), .in_pulse(vid_spr_collision), .out_clk(clk), .out_pulse(spr_collision));

    //////////////////////////////////////////////////////////////////////////
    // Graphics
    //////////////////////////////////////////////////////////////////////////
    wire [4:0] linebuf_data;

    gfx gfx(
        .clk(video_clk),
        .reset(vclk_reset),

        // Register values
        .hscroll_inhibit(q_reg0_hscroll_inhibit),
        .vscroll_inhibit(q_reg0_vscroll_inhibit),
        .hscroll(q_reg8_hscroll),
        .vscroll(q_reg9_vscroll),
        .base_nt(q_reg2_scrmap_base[3:1]),
        .base_sprattr(q_reg5_sprattr_base[6:1]),
        .base_sprpat(q_reg6_sprpat_base[2]),
        .spr_h16(q_reg1_spr_h16),

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

    reg q2_reg1_screen_en;
    always @(posedge (video_clk)) if (render_start) q2_reg1_screen_en <= q_reg1_screen_en;

    wire [4:0] palidx = (!q2_reg1_screen_en || q_border) ? {1'b1, q_reg7_border_colidx} : linebuf_data;

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [1:0] pal_r, pal_g, pal_b;

    palette palette(
        .clk(clk),
        .addr(q_vdp_addr[4:0]),
        .wrdata(io_wrdata),
        .wren(palette_wren),

        .palidx(palidx),
        .pal_r(pal_r),
        .pal_g(pal_g),
        .pal_b(pal_b));

    //////////////////////////////////////////////////////////////////////////
    // Output registers
    //////////////////////////////////////////////////////////////////////////
    always @(posedge(video_clk))
        if (q_blank) begin
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else begin
            vga_r <= {pal_r, pal_r};
            vga_g <= {pal_g, pal_g};
            vga_b <= {pal_b, pal_b};
        end

    always @(posedge video_clk) vga_hsync <= q_hsync;
    always @(posedge video_clk) vga_vsync <= q_vsync;

endmodule
