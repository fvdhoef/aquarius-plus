`default_nettype none
`timescale 1 ns / 1 ps

module aqms_common(
    input  wire        clk,             // 28.63636MHz
    input  wire        reset,

    output wire        reset_req,
    output wire        use_t80,
    input  wire        has_z80,

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
    output reg   [4:0] ebus_ba,
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
    output wire        video_mode,

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
    input  wire        esp_rx_framing_error,

    // Other
    output wire        turbo,
    output wire        turbo_unlimited,

    // Hand controller interface
    input  wire  [7:0] hc1_in,
    input  wire  [7:0] hc2_in
);

    assign video_mode = 1'b1;

    wire [7:0] rddata_rom;
    wire [7:0] rddata_ram;
    wire [7:0] rddata_espctrl;
    wire [7:0] rddata_espdata;
    wire [7:0] rddata_io_video;

    wire [7:0] video_vcnt;
    wire [7:0] video_hcnt;
    wire       video_irq;

    wire       force_turbo;
    reg        q_sysctrl_turbo           = 1'b0;
    reg        q_sysctrl_turbo_unlimited = 1'b0;

    assign turbo           = force_turbo || q_sysctrl_turbo;
    assign turbo_unlimited = force_turbo || q_sysctrl_turbo_unlimited;

    reg q_startup_mode;

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    reg [4:0] q_reg_bank0;
    reg [4:0] q_reg_bank1;
    reg [4:0] q_reg_bank2;
    reg [7:0] q_reg_ramctrl;

    // Select banking register based on upper address bits
    always @* begin
        ebus_ba = 5'b0;

        if (ebus_a[15:14] == 2'b00 && ebus_a >= 16'h400)
            ebus_ba = q_reg_bank0;
        else if (ebus_a[15:14] == 2'b01)
            ebus_ba = q_reg_bank1;
        else if (ebus_a[15:14] == 2'b10)
            ebus_ba = q_reg_bank2;
    end

    wire [7:0] wrdata = ebus_d_in;

    reg [2:0] ebus_wr_n_r;
    reg [2:0] ebus_rd_n_r;
    always @(posedge clk) ebus_wr_n_r <= {ebus_wr_n_r[1:0], ebus_wr_n};
    always @(posedge clk) ebus_rd_n_r <= {ebus_rd_n_r[1:0], ebus_rd_n};

    wire bus_read       = ebus_rd_n_r[2:1] == 2'b10;
    wire bus_read_done  = ebus_rd_n_r[2:1] == 2'b01;
    wire bus_write      = ebus_wr_n_r[2:1] == 2'b10;
    wire bus_write_done = ebus_wr_n_r[2:1] == 2'b01;

    // Memory space decoding
    wire sel_mem_introm = q_startup_mode && !ebus_mreq_n && ebus_a[15:14] == 2'b00;
    wire sel_mem_intram = !ebus_mreq_n && ebus_a[15:14] == 2'b11;

    wire sel_mem_bank2   = !ebus_mreq_n && ebus_a == 16'hFFFF;
    wire sel_mem_bank1   = !ebus_mreq_n && ebus_a == 16'hFFFE;
    wire sel_mem_bank0   = !ebus_mreq_n && ebus_a == 16'hFFFD;
    wire sel_mem_ramctrl = !ebus_mreq_n && ebus_a == 16'hFFFC;

    // IO space decoding
    wire [2:0] io_addr         = {ebus_a[7], ebus_a[6], ebus_a[0]};
    wire       sel_io_espctrl  = q_startup_mode && !ebus_iorq_n && ebus_a[7:0] == 8'h10;
    wire       sel_io_espdata  = q_startup_mode && !ebus_iorq_n && ebus_a[7:0] == 8'h11;
    wire       sel_8bit        = sel_io_espctrl | sel_io_espdata;

    wire       sel_io_3f       = !sel_8bit && !ebus_iorq_n && io_addr == 3'b001;
    wire       sel_io_vcnt     = !sel_8bit && !ebus_iorq_n && io_addr == 3'b010;
    wire       sel_io_hcnt     = !sel_8bit && !ebus_iorq_n && io_addr == 3'b011;
    wire       sel_io_psg      = !sel_8bit && !ebus_iorq_n && io_addr == 3'b011;
    wire       sel_io_vdp_data = !sel_8bit && !ebus_iorq_n && io_addr == 3'b100;
    wire       sel_io_vdp_ctrl = !sel_8bit && !ebus_iorq_n && io_addr == 3'b101;
    wire       sel_io_dc       = !sel_8bit && !ebus_iorq_n && io_addr == 3'b110;
    wire       sel_io_dd       = !sel_8bit && !ebus_iorq_n && io_addr == 3'b111;

    wire       sel_internal    = !ebus_iorq_n | sel_mem_intram | sel_mem_introm;
    wire       allow_sel_mem   = !ebus_mreq_n && !sel_internal && (ebus_wr_n || (!ebus_wr_n && q_startup_mode));
    wire       sel_mem_ram     = allow_sel_mem;

    assign     ebus_ram_ce_n   = !sel_mem_ram;
    assign     ebus_ram_we_n   = !(!ebus_wr_n && sel_mem_ram && q_startup_mode);

    wire       ram_wren        = sel_mem_intram && bus_write;
    wire       io_video_wren   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_write;
    wire       io_video_rden   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_read;

    wire       io_psg_wren     = sel_io_psg && bus_write;

    // Generate rddone signal for video
    reg io_video_rddone;
    reg q_io_video_reading;
    always @(posedge clk) begin
        io_video_rddone <= 1'b0;
        if (io_video_rden) q_io_video_reading <= 1'b1;
        if (q_io_video_reading && bus_read_done) begin
            q_io_video_reading <= 1'b0;
            io_video_rddone <= 1'b1;
        end
    end

    // Generate wrdone signal for video
    reg io_video_wrdone, q_io_video_writing;
    always @(posedge clk) begin
        io_video_wrdone <= 1'b0;
        if (io_video_wren) q_io_video_writing <= 1'b1;
        if (q_io_video_writing && bus_write_done) begin
            q_io_video_writing <= 1'b0;
            io_video_wrdone <= 1'b1;
        end
    end

    // Handle region detection at port $3F
    reg [1:0] q_region_bits;
    always @(posedge clk or posedge reset)
        if (reset)                       q_region_bits <= 2'b11;
        else if (sel_io_3f && bus_write) q_region_bits <= {wrdata[7], wrdata[5]};

    wire [7:0] port_dc, port_dd;

    reg [7:0] rddata;
    always @* begin
        rddata = 8'hFF;

        if (sel_mem_introm)  rddata = rddata_rom;
        if (sel_mem_intram)  rddata = rddata_ram;

        if (sel_io_espctrl)  rddata = rddata_espctrl;
        if (sel_io_espdata)  rddata = rddata_espdata;
        if (sel_io_vcnt)     rddata = video_vcnt;
        if (sel_io_hcnt)     rddata = video_hcnt;
        if (sel_io_vdp_data) rddata = rddata_io_video;
        if (sel_io_vdp_ctrl) rddata = rddata_io_video;
        if (sel_io_dc)       rddata = port_dc;
        if (sel_io_dd)       rddata = port_dd;
    end

    assign ebus_d_oe  = !ebus_rd_n && sel_internal;
    assign ebus_d_out = rddata;
    assign ebus_int_n = video_irq ? 1'b0 : 1'b1;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_startup_mode <= 1'b1;
            q_reg_bank0    <= 5'd0;
            q_reg_bank1    <= 5'd1;
            q_reg_bank2    <= 5'd2;
            q_reg_ramctrl  <= 8'd0;

        end else begin
            if (sel_mem_ramctrl && bus_write) begin
                q_startup_mode <= 1'b0;
                q_reg_ramctrl  <= wrdata;
            end
            if (sel_mem_bank0 && bus_write) q_reg_bank0 <= wrdata[4:0];
            if (sel_mem_bank1 && bus_write) q_reg_bank1 <= wrdata[4:0];
            if (sel_mem_bank2 && bus_write) q_reg_bank2 <= wrdata[4:0];
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // System ROM
    //////////////////////////////////////////////////////////////////////////
    rom rom(
        .clk(clk),
        .addr(ebus_a[12:0]),
        .rddata(rddata_rom));

    //////////////////////////////////////////////////////////////////////////
    // System RAM
    //////////////////////////////////////////////////////////////////////////
    ram ram(
        .clk(clk),
        .addr(ebus_a[12:0]),
        .rddata(rddata_ram),
        .wrdata(wrdata),
        .wren(ram_wren));

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
    video video(
        .clk(clk),
        .reset(reset),

        .video_clk(video_clk),

        .io_portsel(ebus_a[0]),
        .io_rddata(rddata_io_video),
        .io_wrdata(wrdata),
        .io_wren(io_video_wren),
        .io_wrdone(io_video_wrdone),
        .io_rddone(io_video_rddone),
        .irq(video_irq),

        .vcnt(video_vcnt),
        .hcnt(video_hcnt),

        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b),
        .video_de(video_de),
        .video_hsync(video_hsync),
        .video_vsync(video_vsync),
        .video_newframe(video_newframe),
        .video_oddline(video_oddline));

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] spi_hctrl1, spi_hctrl2;

    wire [7:0] hctrl1 = hc1_in[7:0];
    wire [7:0] hctrl2 = hc2_in[7:0];

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

    spiregs spiregs(
        .clk(clk),
        .reset(reset),

        .spi_msg_end(spi_msg_end),
        .spi_cmd(spi_cmd),
        .spi_rxdata(spi_rxdata),
        .spi_txdata(spi_txdata),
        .spi_txdata_valid(spi_txdata_valid),

        .reset_req(reset_req),
        .keys(keys),
        .hctrl1(spi_hctrl1),
        .hctrl2(spi_hctrl2),

        .use_t80(use_t80),
        .has_z80(has_z80),
        .force_turbo(force_turbo));

    wire joypad_a_tr    = keys[45] && hctrl1_data[6]; // X
    wire joypad_a_tl    = keys[51] && hctrl1_data[5]; // Z
    wire joypad_a_right = keys[15] && hctrl1_data[1];
    wire joypad_a_left  = keys[22] && hctrl1_data[3];
    wire joypad_a_down  = keys[23] && hctrl1_data[0];
    wire joypad_a_up    = keys[14] && hctrl1_data[2];

    wire joypad_b_tr    = hctrl2_data[6];
    wire joypad_b_tl    = hctrl2_data[5];
    wire joypad_b_right = hctrl2_data[1];
    wire joypad_b_left  = hctrl2_data[3];
    wire joypad_b_down  = hctrl2_data[0];
    wire joypad_b_up    = hctrl2_data[2];

    assign port_dc = {joypad_b_down, joypad_b_up, joypad_a_tr, joypad_a_tl, joypad_a_right, joypad_a_left, joypad_a_down, joypad_a_up};
    assign port_dd = {q_region_bits, 2'b11, joypad_b_tr, joypad_b_tl, joypad_b_right, joypad_b_left};

    //////////////////////////////////////////////////////////////////////////
    // SN76489 PSG
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] psg_sample;
    
    psg psg(
        .clk(clk),
        .reset(reset),

        .wrdata(wrdata),
        .wren(io_psg_wren),

        .sample(psg_sample)
    );

    always @* audio_l = {~psg_sample[15], psg_sample[14:0]};
    always @* audio_r = {~psg_sample[15], psg_sample[14:0]};

endmodule
