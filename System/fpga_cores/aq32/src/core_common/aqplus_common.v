`default_nettype none
`timescale 1 ns / 1 ps

module aqplus_common(
    input  wire        clk,             // 28.63636MHz
    input  wire        reset,

    output wire        reset_req,

    // Bus interface
    input  wire [15:0] ebus_a,
    input  wire  [7:0] ebus_d_in,
    output wire  [7:0] ebus_d_out,
    output wire        ebus_d_oe,
    input  wire        ebus_rd_n,
    input  wire        ebus_wr_n,
    input  wire        ebus_mreq_n,
    input  wire        ebus_iorq_n,
    output wire        ebus_int_n,
    output wire  [4:0] ebus_ba,
    output wire        ebus_ram_ce_n,   // 512KB RAM
    output wire        ebus_ram_we_n,

    input  wire        ebus_stb,

    // Video output
    input  wire        video_clk,       // video_mode 0:28.63636MHz, 1:25.175MHz
    output wire  [3:0] video_r,
    output wire  [3:0] video_g,
    output wire  [3:0] video_b,
    output wire        video_de,
    output wire        video_hsync,
    output wire        video_vsync,
    output wire        video_newframe,
    output wire        video_oddline,

    // Audio output
    output reg  [15:0] audio_l,
    output reg  [15:0] audio_r,

    // ESP SPI interface
    input  wire        spi_msg_end,
    input  wire  [7:0] spi_cmd,
    input  wire [63:0] spi_rxdata,
    output wire [63:0] spi_txdata,
    output wire        spi_txdata_valid,

    // ESP UART interface
    output wire  [8:0] esp_tx_data,   // if bit8 set: transmit break, ignore data
    output wire        esp_tx_wr,
    input  wire        esp_tx_fifo_full,
    input  wire  [8:0] esp_rx_data,   // if bit8 set: received break, other data bits will be 0
    output wire        esp_rx_rd,
    input  wire        esp_rx_empty,
    input  wire        esp_rx_fifo_overflow,
    input  wire        esp_rx_framing_error
);

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

    wire        spi_reset_req;
    wire        reset_req_cold;
    reg         q_sysctrl_warm_boot = 1'b0;
    reg         q_sysctrl_reset_req = 1'b0;

    always @(posedge clk) if (reset_req) begin
        q_sysctrl_warm_boot <= !reset_req_cold;
    end

    assign reset_req = spi_reset_req || q_sysctrl_reset_req;

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
    // reg [7:0] wrdata;
    // always @(posedge clk) if (!ebus_wr_n) wrdata <= ebus_d_in;

    wire [7:0] wrdata = ebus_d_in;

    // reg [2:0] q_ebus_wr_n;
    // reg [2:0] q_ebus_rd_n;
    // always @(posedge clk) q_ebus_wr_n <= {q_ebus_wr_n[1:0], ebus_wr_n};
    // always @(posedge clk) q_ebus_rd_n <= {q_ebus_rd_n[1:0], ebus_rd_n};

    wire bus_read  = !ebus_rd_n && ebus_stb;    //  q_ebus_rd_n[2:1] == 2'b10;
    wire bus_write = !ebus_wr_n && ebus_stb;    //  q_ebus_wr_n[2:1] == 2'b10;

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

    wire sel_mem_ram     = !ebus_mreq_n && !sel_internal && reg_bank_page[5];                       // Page 32-63

    assign ebus_ram_we_n  = !(sel_mem_ram && !ebus_wr_n && (!reg_bank_ro || sel_mem_sysram));
    assign ebus_ram_ce_n  = !sel_mem_ram;

    reg [7:0] rddata;
    always @* begin
        rddata = 8'hFF;
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
        if (sel_io_sysctrl)           rddata = {q_sysctrl_warm_boot, 5'b0, q_sysctrl_dis_psgs, q_sysctrl_dis_regs}; // IO $FB
        if (sel_io_vsync_r_cpm_w)     rddata = {7'b0, reg_fd_val};                             // IO $FD
        if (sel_io_keyb_r_scramble_w) rddata = rddata_keyboard;                                // IO $FF
    end

    assign ebus_d_oe  = !ebus_rd_n && sel_internal;
    assign ebus_d_out = rddata;

    wire video_irq;

    assign ebus_int_n = video_irq ? 1'b0 : 1'b1;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_audio_dac               <= 8'b0;
            q_reg_bank0               <= {2'b00, 6'd0};
            q_reg_bank1               <= {2'b00, 6'd0};
            q_reg_bank2               <= {2'b00, 6'd0};
            q_reg_bank3               <= {2'b00, 6'd0};
            q_sysctrl_dis_regs        <= 1'b0;
            q_sysctrl_dis_psgs        <= 1'b0;
            q_reg_cpm_remap           <= 1'b0;

        end else begin
            if (sel_io_audio_dac     && bus_write) q_audio_dac     <= wrdata;
            if (sel_io_bank0         && bus_write) q_reg_bank0     <= wrdata;
            if (sel_io_bank1         && bus_write) q_reg_bank1     <= wrdata;
            if (sel_io_bank2         && bus_write) q_reg_bank2     <= wrdata;
            if (sel_io_bank3         && bus_write) q_reg_bank3     <= wrdata;
            if (sel_io_vsync_r_cpm_w && bus_write) q_reg_cpm_remap <= wrdata[0];

            if (sel_io_sysctrl && bus_write) begin
                q_sysctrl_dis_psgs        <= wrdata[1];
                q_sysctrl_dis_regs        <= wrdata[0];
            end
        end

    always @(posedge clk) q_sysctrl_reset_req <= (sel_io_sysctrl && bus_write && wrdata[7]);

    //////////////////////////////////////////////////////////////////////////
    // Boot ROM
    //////////////////////////////////////////////////////////////////////////
    rom rom(
        .clk(clk),
        .addr(ebus_a[7:0]),
        .rddata(rddata_rom)
    );

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    assign esp_tx_data = sel_io_espctrl ? 9'b100000000 : {1'b0, wrdata};
    assign esp_tx_wr   = bus_write && (sel_io_espdata || (sel_io_espctrl && wrdata[7]));
    assign esp_rx_rd   = bus_read  &&  sel_io_espdata;

    reg q_esp_rx_fifo_overflow, q_esp_rx_framing_error;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_esp_rx_fifo_overflow <= 1'b0;
            q_esp_rx_framing_error <= 1'b0;
        end else begin
            if (sel_io_espctrl && bus_write) begin
                q_esp_rx_fifo_overflow <= q_esp_rx_fifo_overflow & ~wrdata[4];
                q_esp_rx_framing_error <= q_esp_rx_framing_error & ~wrdata[3];
            end

            if (esp_rx_fifo_overflow) q_esp_rx_fifo_overflow <= 1'b1;
            if (esp_rx_framing_error) q_esp_rx_framing_error <= 1'b1;
        end
    end

    assign rddata_espctrl = {3'b0, q_esp_rx_fifo_overflow, q_esp_rx_framing_error, esp_rx_data[8], esp_tx_fifo_full, !esp_rx_empty};
    assign rddata_espdata = esp_rx_data[7:0];

    //////////////////////////////////////////////////////////////////////////
    // Video
    //////////////////////////////////////////////////////////////////////////
    wire tram_wren     = sel_mem_tram  && bus_write;
    wire vram_wren     = sel_mem_vram  && bus_write;
    wire chram_wren    = sel_mem_chram && bus_write;
    wire io_video_wren = sel_io_video  && bus_write;

    video video(
        .clk(clk),
        .reset(reset),

        .vclk(video_clk),

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

        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),
        .video_de(video_de),
        .video_hsync(video_hsync),
        .video_vsync(video_vsync),
        .video_newframe(video_newframe),
        .video_oddline(video_oddline),

        .reg_fd_val(reg_fd_val));

    //////////////////////////////////////////////////////////////////////////
    // SPI interface
    //////////////////////////////////////////////////////////////////////////
    wire [63:0] keys;

    wire  [7:0] kbbuf_data;
    wire        kbbuf_wren;

    spiregs spiregs(
        .clk(clk),
        .reset(reset),

        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        .spi_txdata(spi_txdata),
        .spi_txdata_valid(spi_txdata_valid),

        .reset_req(spi_reset_req),
        .reset_req_cold(reset_req_cold),
        .keys(keys),

        .kbbuf_data(kbbuf_data),
        .kbbuf_wren(kbbuf_wren));

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

    wire [7:0] ay8190_1_ioa_out_data, ay8190_2_ioa_out_data;
    wire       ay8190_1_ioa_oe,       ay8190_2_ioa_oe;
    wire [7:0] ay8190_1_iob_out_data, ay8190_2_iob_out_data;
    wire       ay8190_1_iob_oe,       ay8190_2_iob_oe;

    ay8910 ay8910(
        .clk(clk),
        .reset(reset),

        .a0(ebus_a[0]),
        .wren(ay8910_wren),
        .wrdata(wrdata),
        .rddata(rddata_ay8910),

        .ioa_in_data(8'h00),
        .ioa_out_data(ay8190_1_ioa_out_data),
        .ioa_oe(ay8190_1_ioa_oe),

        .iob_in_data(8'h00),
        .iob_out_data(ay8190_1_iob_out_data),
        .iob_oe(ay8190_1_iob_oe),

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
        {2'b0, q_audio_dac,   4'b0};

    wire [13:0] mix_r =
        {4'b0, ay8910_ch_a  }     + {2'b0, ay8910_ch_b,   1'b0} + {2'b0, ay8910_ch_c,   1'b0} +
        {4'b0, ay8910_2_ch_a}     + {2'b0, ay8910_2_ch_b, 1'b0} + {2'b0, ay8910_2_ch_c, 1'b0} +
        {2'b0, q_audio_dac, 4'b0};

    always @(posedge clk) audio_l <= {~mix_l[13], mix_l[12:0], 2'b0};
    always @(posedge clk) audio_r <= {~mix_r[13], mix_r[12:0], 2'b0};

endmodule
