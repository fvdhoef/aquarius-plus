`default_nettype none
`timescale 1 ns / 1 ps

module aqp_top(
    input  wire        sysclk,          // 14.31818MHz

    // Z80 bus interface
    inout  wire        ebus_reset_n,
    output wire        ebus_phi,        // 3.579545MHz
    inout  wire [15:0] ebus_a,
    inout  wire  [7:0] ebus_d,
    inout  wire        ebus_rd_n,
    inout  wire        ebus_wr_n,
    inout  wire        ebus_mreq_n,
    inout  wire        ebus_iorq_n,
    output wire        ebus_int_n,      // Open-drain output
    output wire        ebus_busreq_n,   // Open-drain output
    input  wire        ebus_busack_n,
    output wire  [4:0] ebus_ba,
    output wire        ebus_ram_ce_n,   // 512KB RAM
    output wire        ebus_cart_ce_n,  // Cartridge
    output wire        ebus_ram_we_n,

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output reg         cassette_out,
    input  wire        cassette_in,
    output reg         printer_out,
    input  wire        printer_in,

    // Misc
    output wire  [9:0] exp,

    // Hand controller interface
    inout  wire  [8:0] hc1,
    inout  wire  [8:0] hc2,

    // VGA output
    output wire  [3:0] vga_r,
    output wire  [3:0] vga_g,
    output wire  [3:0] vga_b,
    output wire        vga_hsync,
    output wire        vga_vsync,

    // ESP32 serial interface
    output wire        esp_tx,
    input  wire        esp_rx,
    output wire        esp_rts,
    input  wire        esp_cts,

    // ESP32 SPI interface (also used for loading FPGA image)
    input  wire        esp_ssel_n,
    input  wire        esp_sclk,        // Connected to EXP7
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    assign exp = 10'b0;

    wire [15:0] spibm_a;
    wire  [7:0] spibm_wrdata;
    wire        spibm_wrdata_en;
    wire        spibm_en;

    wire        reset_req;
    wire        reg_fd_val;

    wire  [7:0] rddata_tram;             // MEM $3000-$37FF
    wire  [7:0] rddata_chram;
    wire  [7:0] rddata_vram;
    wire  [7:0] rddata_rom;

    wire  [7:0] rddata_io_video;         // IO $E0-$EF
    wire  [7:0] rddata_espctrl;          // IO $F4
    wire  [7:0] rddata_espdata;          // IO $F5
    wire  [7:0] rddata_ay8910;           // IO $F6/F7
    wire  [7:0] rddata_ay8910_2;         // IO $F8/F9
    wire  [7:0] rddata_kbbuf;            // IO $FA
    wire  [7:0] rddata_keyboard;         // IO $FF:R

    reg   [7:0] q_audio_dac;             // IO $EC
    reg   [7:0] q_reg_bank0;             // IO $F0
    reg   [7:0] q_reg_bank1;             // IO $F1
    reg   [7:0] q_reg_bank2;             // IO $F2
    reg   [7:0] q_reg_bank3;             // IO $F3
    reg         q_reg_cpm_remap;         // IO $FD:W

    reg         q_sysctrl_turbo;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk, vclk;
    wire video_mode;

    aqp_clkctrl clkctrl(
        .clk_in(sysclk),    // 14.31818MHz
        .clk_out(clk),      // 28.63636MHz

        .vclk(vclk),
        .video_mode(video_mode)
    );

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset;

    aqp_sysctrl sysctrl(
        .sysclk(clk),
        .ebus_reset_n(ebus_reset_n),
        .reset_req(reset_req),

        .turbo_mode(q_sysctrl_turbo),

        .ebus_phi(ebus_phi),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // Synchronize cassette and printer input
    //////////////////////////////////////////////////////////////////////////
    reg [2:0] q_cassette_in;
    always @(posedge ebus_phi) q_cassette_in <= {q_cassette_in[1:0], cassette_in};

    reg [2:0] q_printer_in;
    always @(posedge ebus_phi) q_printer_in <= {q_printer_in[1:0], printer_in};

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    reg q_sysctrl_dis_regs;
    reg q_sysctrl_dis_psgs;

    // Select banking register based on upper address bits
    reg [7:0] reg_bank;
    always @* case (ebus_a[15:14])
        2'd0: reg_bank = q_reg_bank0;
        2'd1: reg_bank = q_reg_bank1;
        2'd2: reg_bank = q_reg_bank2;
        2'd3: reg_bank = q_reg_bank3;
    endcase

    wire [5:0] reg_bank_page    = reg_bank[5:0];
    wire       reg_bank_ro      = reg_bank[7];
    wire       reg_bank_overlay = reg_bank[6];

    // Register data from external bus
    reg [7:0] wrdata;
    always @(posedge clk) if (!ebus_wr_n) wrdata <= ebus_d;

    reg [2:0] q_ebus_wr_n;
    reg [2:0] q_ebus_rd_n;
    always @(posedge clk) q_ebus_wr_n <= {q_ebus_wr_n[1:0], ebus_wr_n};
    always @(posedge clk) q_ebus_rd_n <= {q_ebus_rd_n[1:0], ebus_rd_n};

    wire bus_read  = q_ebus_rd_n[2:1] == 2'b10;
    wire bus_write = q_ebus_wr_n[2:1] == 2'b10;

    // Memory space decoding
    wire sel_mem_tram    = !ebus_mreq_n && reg_bank_overlay && ebus_a[13:11] == 3'b110;   // $3000-$37FF
    wire sel_mem_sysram  = !ebus_mreq_n && reg_bank_overlay && ebus_a[13:11] == 3'b111;   // $3800-$3FFF
    wire sel_mem_vram    = !ebus_mreq_n && reg_bank_page == 6'd20;                        // Page 20
    wire sel_mem_chram   = !ebus_mreq_n && reg_bank_page == 6'd21;                        // Page 21
    wire sel_mem_rom     = !ebus_mreq_n && reg_bank_page <= 6'd3;                         // Page 0-3

    assign ebus_ba = reg_bank_page[4:0];

    // IO space decoding
    wire sel_io_video             = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:4] == 4'hE;
    wire sel_io_audio_dac         = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hEC;
    wire sel_io_bank0             = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hF0;
    wire sel_io_bank1             = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hF1;
    wire sel_io_bank2             = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hF2;
    wire sel_io_bank3             = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hF3;
    wire sel_io_espctrl           = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hF4;
    wire sel_io_espdata           = !q_sysctrl_dis_regs && !ebus_iorq_n && ebus_a[7:0] == 8'hF5;
    wire sel_io_ay8910            = !q_sysctrl_dis_psgs && !ebus_iorq_n && (ebus_a[7:0] == 8'hF6 || ebus_a[7:0] == 8'hF7);
    wire sel_io_ay8910_2          = !q_sysctrl_dis_regs && !q_sysctrl_dis_psgs && !ebus_iorq_n && (ebus_a[7:0] == 8'hF8 || ebus_a[7:0] == 8'hF9);
    wire sel_io_kbbuf             = !ebus_iorq_n && ebus_a[7:0] == 8'hFA;
    wire sel_io_sysctrl           = !ebus_iorq_n && ebus_a[7:0] == 8'hFB;
    wire sel_io_cassette          = !ebus_iorq_n && ebus_a[7:0] == 8'hFC;
    wire sel_io_vsync_r_cpm_w     = !ebus_iorq_n && ebus_a[7:0] == 8'hFD;
    wire sel_io_printer           = !ebus_iorq_n && ebus_a[7:0] == 8'hFE;
    wire sel_io_keyb_r_scramble_w = !ebus_iorq_n && ebus_a[7:0] == 8'hFF;

    wire sel_internal =
        sel_mem_tram | sel_mem_vram | sel_mem_chram | sel_mem_rom |
        sel_io_video |
        sel_io_bank0 | sel_io_bank1 | sel_io_bank2 | sel_io_bank3 |
        sel_io_espctrl | sel_io_espdata | sel_io_ay8910 | sel_io_ay8910_2 | sel_io_kbbuf | sel_io_sysctrl |
        sel_io_cassette | sel_io_vsync_r_cpm_w | sel_io_printer | sel_io_keyb_r_scramble_w;

    wire sel_mem_cart    = !ebus_mreq_n && !sel_internal && reg_bank_page[5:2] == 4'b0100;          // Page 16-19
    wire sel_mem_ram     = !ebus_mreq_n && !sel_internal && reg_bank_page[5];                       // Page 32-63

    assign ebus_ram_we_n  = !(sel_mem_ram && !ebus_wr_n && (!reg_bank_ro || sel_mem_sysram));
    assign ebus_ram_ce_n  = !sel_mem_ram;
    assign ebus_cart_ce_n = !sel_mem_cart;

    reg [7:0] rddata;
    always @* begin
        rddata = 8'h00;
        if (sel_mem_rom)              rddata = rddata_rom;
        if (sel_mem_tram)             rddata = rddata_tram;            // TRAM $3000-$37FF
        if (sel_mem_vram)             rddata = rddata_vram;
        if (sel_mem_chram)            rddata = rddata_chram;

        if (sel_io_video)             rddata = rddata_io_video;                                // IO $E0-$EF
        if (sel_io_bank0)             rddata = q_reg_bank0;                                    // IO $F0
        if (sel_io_bank1)             rddata = q_reg_bank1;                                    // IO $F1
        if (sel_io_bank2)             rddata = q_reg_bank2;                                    // IO $F2
        if (sel_io_bank3)             rddata = q_reg_bank3;                                    // IO $F3
        if (sel_io_espctrl)           rddata = rddata_espctrl;                                 // IO $F4
        if (sel_io_espdata)           rddata = rddata_espdata;                                 // IO $F5
        if (sel_io_ay8910)            rddata = rddata_ay8910;                                  // IO $F6/F7
        if (sel_io_ay8910_2)          rddata = rddata_ay8910_2;                                // IO $F8/F9
        if (sel_io_kbbuf)             rddata = rddata_kbbuf;                                   // IO $FA
        if (sel_io_sysctrl)           rddata = {5'b0, q_sysctrl_turbo, q_sysctrl_dis_psgs, q_sysctrl_dis_regs}; // IO $FB
        if (sel_io_cassette)          rddata = {7'b0, !q_cassette_in[2]};                      // IO $FC
        if (sel_io_vsync_r_cpm_w)     rddata = {7'b0, reg_fd_val};                             // IO $FD
        if (sel_io_printer)           rddata = {7'b0, q_printer_in[2]};                        // IO $FE
        if (sel_io_keyb_r_scramble_w) rddata = rddata_keyboard;                                // IO $FF
    end

    wire ebus_d_enable = !ebus_rd_n && sel_internal;

    assign ebus_d = (spibm_en && spibm_wrdata_en) ? spibm_wrdata : (ebus_d_enable ? rddata : 8'bZ);

    wire video_irq;

    assign ebus_int_n = video_irq ? 1'b0 : 1'bZ;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_audio_dac         <= 8'b0;
            q_reg_bank0         <= {2'b00, 6'd0};
            q_reg_bank1         <= {2'b00, 6'd0};
            q_reg_bank2         <= {2'b00, 6'd0};
            q_reg_bank3         <= {2'b00, 6'd0};
            q_sysctrl_dis_regs  <= 1'b0;
            q_sysctrl_dis_psgs  <= 1'b0;
            q_sysctrl_turbo     <= 1'b0;
            cassette_out        <= 1'b0;
            q_reg_cpm_remap     <= 1'b0;
            printer_out         <= 1'b0;

        end else begin
            if (sel_io_audio_dac     && bus_write) q_audio_dac     <= wrdata;
            if (sel_io_bank0         && bus_write) q_reg_bank0     <= wrdata;
            if (sel_io_bank1         && bus_write) q_reg_bank1     <= wrdata;
            if (sel_io_bank2         && bus_write) q_reg_bank2     <= wrdata;
            if (sel_io_bank3         && bus_write) q_reg_bank3     <= wrdata;
            if (sel_io_cassette      && bus_write) cassette_out    <= wrdata[0];
            if (sel_io_vsync_r_cpm_w && bus_write) q_reg_cpm_remap <= wrdata[0];
            if (sel_io_printer       && bus_write) printer_out     <= wrdata[0];

            if (sel_io_sysctrl && bus_write) begin
                q_sysctrl_turbo    <= wrdata[2];
                q_sysctrl_dis_psgs <= wrdata[1];
                q_sysctrl_dis_regs <= wrdata[0];
            end
        end

    //////////////////////////////////////////////////////////////////////////
    // System ROM
    //////////////////////////////////////////////////////////////////////////
    rom rom(
        .clk(clk),
        .addr(ebus_a[7:0]),
        .rddata(rddata_rom));

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    wire esp_txvalid = sel_io_espdata && bus_write;
    wire esp_txbreak = sel_io_espctrl && bus_write && wrdata[7];
    wire esp_txbusy;

    wire esp_rxfifo_not_empty;
    wire esp_rxfifo_read = sel_io_espdata && bus_read;
    wire esp_rxfifo_overflow, esp_rx_framing_error, esp_rx_break;

    reg [2:0] q_esp_ctrl_status;
    always @(posedge clk or posedge reset) begin
        if (reset)
            q_esp_ctrl_status <= 3'b0;
        else begin
            if (sel_io_espctrl && bus_write) q_esp_ctrl_status <= q_esp_ctrl_status & ~wrdata[4:2];

            if (esp_rxfifo_overflow)  q_esp_ctrl_status[2] <= 1'b1;
            if (esp_rx_framing_error) q_esp_ctrl_status[1] <= 1'b1;
            if (esp_rx_break)         q_esp_ctrl_status[0] <= 1'b1;
        end
    end

    assign rddata_espctrl = {3'b0, q_esp_ctrl_status, esp_txbusy, esp_rxfifo_not_empty};

    aqp_esp_uart esp_uart(
        .clk(clk),
        .reset(reset),

        .tx_data(wrdata),
        .tx_valid(esp_txvalid),
        .tx_break(esp_txbreak),
        .tx_busy(esp_txbusy),

        .rxfifo_data(rddata_espdata),
        .rxfifo_not_empty(esp_rxfifo_not_empty),
        .rxfifo_read(esp_rxfifo_read),
        .rxfifo_overflow(esp_rxfifo_overflow),
        .rx_framing_error(esp_rx_framing_error),
        .rx_break(esp_rx_break),

        .uart_rxd(esp_rx),
        .uart_txd(esp_tx),
        .uart_cts(esp_cts),
        .uart_rts(esp_rts));

    //////////////////////////////////////////////////////////////////////////
    // Video
    //////////////////////////////////////////////////////////////////////////
    wire tram_wren     = sel_mem_tram    && bus_write;
    wire vram_wren     = sel_mem_vram    && bus_write;
    wire chram_wren    = sel_mem_chram   && bus_write;
    wire io_video_wren = sel_io_video    && bus_write;

    wire video_de, video_newframe, video_oddline;   // unused

    video video(
        .clk(clk),
        .reset(reset),

        .vclk(vclk),
        .video_mode(video_mode),

        .io_addr(ebus_a[3:0]),
        .io_rddata(rddata_io_video),
        .io_wrdata(wrdata),
        .io_wren(io_video_wren),
        .irq(video_irq),

        .tram_addr(ebus_a[10:0]),
        .tram_rddata(rddata_tram),
        .tram_wrdata(wrdata),
        .tram_wren(tram_wren),

        .chram_addr(ebus_a[10:0]),
        .chram_rddata(rddata_chram),
        .chram_wrdata(wrdata),
        .chram_wren(chram_wren),

        .vram_addr(ebus_a[13:0]),
        .vram_rddata(rddata_vram),
        .vram_wrdata(wrdata),
        .vram_wren(vram_wren),

        .video_r(vga_r),
        .video_g(vga_g),
        .video_b(vga_b),
        .video_de(video_de),
        .video_hsync(vga_hsync),
        .video_vsync(vga_vsync),
        .video_newframe(video_newframe),
        .video_oddline(video_oddline),
        
        .reg_fd_val(reg_fd_val));

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] spi_hctrl1, spi_hctrl2;

    wire [7:0] hc1_out_data;
    wire       hc1_oe;
    wire [7:0] hc2_out_data;
    wire       hc2_oe;

    assign hc1[7:0] = hc1_oe ? hc1_out_data : 8'bZ;
    assign hc2[7:0] = hc2_oe ? hc2_out_data : 8'bZ;
    assign hc1[8] = 1'b0;
    assign hc2[8] = 1'b0;

    wire [7:0] hctrl1 = hc1[7:0];
    wire [7:0] hctrl2 = hc2[7:0];

    // Synchronize inputs
    reg [7:0] q_hctrl1, q2_hctrl1;
    reg [7:0] q_hctrl2, q2_hctrl2;
    always @(posedge clk) q_hctrl1  <= hctrl1;
    always @(posedge clk) q2_hctrl1 <= q_hctrl1;
    always @(posedge clk) q_hctrl2  <= hctrl2;
    always @(posedge clk) q2_hctrl2 <= q_hctrl2;

    // Combine data from ESP with data from handcontroller input
    wire [7:0] hctrl1_data = q2_hctrl1 & spi_hctrl1;
    wire [7:0] hctrl2_data = q2_hctrl2 & spi_hctrl2;

    //////////////////////////////////////////////////////////////////////////
    // SPI interface
    //////////////////////////////////////////////////////////////////////////
    wire [63:0] keys;

    wire        spibm_rd_n, spibm_wr_n, spibm_mreq_n, spibm_iorq_n;
    wire        spibm_busreq;

    wire  [7:0] kbbuf_data;
    wire        kbbuf_wren;

    assign spibm_en      = spibm_busreq && !ebus_busack_n;
    assign ebus_a        = spibm_en ? spibm_a      : 16'bZ;
    assign ebus_rd_n     = spibm_en ? spibm_rd_n   : 1'bZ;
    assign ebus_wr_n     = spibm_en ? spibm_wr_n   : 1'bZ;
    assign ebus_mreq_n   = spibm_en ? spibm_mreq_n : 1'bZ;
    assign ebus_iorq_n   = spibm_en ? spibm_iorq_n : 1'bZ;
    assign ebus_busreq_n = spibm_busreq ? 1'b0 : 1'bZ;

    aqp_spiregs spiregs(
        .clk(clk),
        .reset(reset),

        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),

        .ebus_phi(ebus_phi),

        .spibm_a(spibm_a),
        .spibm_rddata(ebus_d),
        .spibm_wrdata(spibm_wrdata),
        .spibm_wrdata_en(spibm_wrdata_en),
        .spibm_rd_n(spibm_rd_n),
        .spibm_wr_n(spibm_wr_n),
        .spibm_mreq_n(spibm_mreq_n),
        .spibm_iorq_n(spibm_iorq_n),
        .spibm_busreq(spibm_busreq),

        .reset_req(reset_req),
        .keys(keys),
        .hctrl1(spi_hctrl1),
        .hctrl2(spi_hctrl2),
        
        .kbbuf_data(kbbuf_data),
        .kbbuf_wren(kbbuf_wren),
        
        .video_mode(video_mode));

    assign esp_notify = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Keyboard
    //////////////////////////////////////////////////////////////////////////
    assign rddata_keyboard =
        (ebus_a[15] ? 8'hFF : keys[63:56]) &
        (ebus_a[14] ? 8'hFF : keys[55:48]) &
        (ebus_a[13] ? 8'hFF : keys[47:40]) &
        (ebus_a[12] ? 8'hFF : keys[39:32]) &
        (ebus_a[11] ? 8'hFF : keys[31:24]) &
        (ebus_a[10] ? 8'hFF : keys[23:16]) &
        (ebus_a[ 9] ? 8'hFF : keys[15: 8]) &
        (ebus_a[ 8] ? 8'hFF : keys[ 7: 0]);

    //////////////////////////////////////////////////////////////////////////
    // Keyboard buffer
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] kbbuf_rddata;
    wire       kbbuf_rden = sel_io_kbbuf && bus_read;
    wire       kbbuf_rst  = (sel_io_kbbuf && bus_write) || reset;

    kbbuf kbbuf(
        .clk(clk),
        .rst(kbbuf_rst),

        .wrdata(kbbuf_data),
        .wr_en(kbbuf_wren),

        .rddata(rddata_kbbuf),
        .rd_en(kbbuf_rden)
    );

    //////////////////////////////////////////////////////////////////////////
    // AY-3-8910
    //////////////////////////////////////////////////////////////////////////
    wire       ay8910_wren   = sel_io_ay8910   && bus_write;
    wire       ay8910_2_wren = sel_io_ay8910_2 && bus_write;
    wire [9:0] ay8910_ch_a,   ay8910_ch_b,   ay8910_ch_c;
    wire [9:0] ay8910_2_ch_a, ay8910_2_ch_b, ay8910_2_ch_c;

    wire [9:0] beep = cassette_out ? 10'd1023 : 10'd0;

    wire [7:0] ay8190_2_ioa_out_data;
    wire       ay8190_2_ioa_oe;
    wire [7:0] ay8190_2_iob_out_data;
    wire       ay8190_2_iob_oe;

    ay8910 ay8910(
        .clk(clk),
        .reset(reset),

        .a0(ebus_a[0]),
        .wren(ay8910_wren),
        .wrdata(wrdata),
        .rddata(rddata_ay8910),

        .ioa_in_data(hctrl1_data),
        .ioa_out_data(hc1_out_data),
        .ioa_oe(hc1_oe),

        .iob_in_data(hctrl2_data),
        .iob_out_data(hc2_out_data),
        .iob_oe(hc2_oe),

        .ch_a(ay8910_ch_a),
        .ch_b(ay8910_ch_b),
        .ch_c(ay8910_ch_c));

    ay8910 ay8910_2(
        .clk(clk),
        .reset(reset),

        .a0(ebus_a[0]),
        .wren(ay8910_2_wren),
        .wrdata(wrdata),
        .rddata(rddata_ay8910_2),

        .ioa_in_data(8'h00),
        .ioa_out_data(ay8190_2_ioa_out_data),
        .ioa_oe(ay8190_2_ioa_oe),

        .iob_in_data(8'h00),
        .iob_out_data(ay8190_2_iob_out_data),
        .iob_oe(ay8190_2_iob_oe),

        .ch_a(ay8910_2_ch_a),
        .ch_b(ay8910_2_ch_b),
        .ch_c(ay8910_2_ch_c));

    // Create stereo mix of output channels and system beep (cassette output)
    wire [13:0] mix_l =
        {2'b0, ay8910_ch_a,   1'b0} + {2'b0, ay8910_ch_b,   1'b0} + {4'b0, ay8910_ch_c  } +
        {2'b0, ay8910_2_ch_a, 1'b0} + {2'b0, ay8910_2_ch_b, 1'b0} + {4'b0, ay8910_2_ch_c} +
        {2'b0, q_audio_dac,   4'b0} + {4'b0, beep};

    wire [13:0] mix_r =
        {4'b0, ay8910_ch_a  }     + {2'b0, ay8910_ch_b,   1'b0} + {2'b0, ay8910_ch_c,   1'b0} +
        {4'b0, ay8910_2_ch_a}     + {2'b0, ay8910_2_ch_b, 1'b0} + {2'b0, ay8910_2_ch_c, 1'b0} +
        {2'b0, q_audio_dac, 4'b0} + {4'b0, beep};

    //////////////////////////////////////////////////////////////////////////
    // PWM DAC
    //////////////////////////////////////////////////////////////////////////
    wire        next_sample = 1'b1;
    wire [15:0] left_data   = {mix_l, 2'b0};
    wire [15:0] right_data  = {mix_r, 2'b0};

    aqp_pwm_dac pwm_dac(
        .clk(clk),
        .reset(reset),

        // Sample input
        .next_sample(next_sample),
        .left_data(left_data),
        .right_data(right_data),

        // PWM audio output
        .audio_l(audio_l),
        .audio_r(audio_r));

endmodule
