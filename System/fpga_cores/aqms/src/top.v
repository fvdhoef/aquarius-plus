module top(
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
    output reg   [4:0] ebus_ba,
    output wire        ebus_ram_ce_n,   // 512KB RAM
    output wire        ebus_cart_ce_n,  // Cartridge
    output wire        ebus_ram_we_n,

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output wire        cassette_out,        // Unused
    input  wire        cassette_in,         // Unused
    output wire        printer_out,         // Unused
    input  wire        printer_in,          // Unused

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
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    // Handle unused pins
    assign ebus_cart_ce_n = 1'b1;
    assign cassette_out   = 1'b0;
    assign printer_out    = 1'b1;
    assign exp            = 10'b0;
    assign esp_notify     = 1'b0;

    wire [15:0] spibm_a;
    wire  [7:0] spibm_wrdata;
    wire        spibm_wrdata_en;
    wire        spibm_en;

    wire        reset_req;

    wire [7:0] rddata_rom;
    wire [7:0] rddata_ram;
    wire [7:0] rddata_espctrl;
    wire [7:0] rddata_espdata;
    wire [7:0] rddata_io_video;

    wire [7:0] video_vcnt;
    wire [7:0] video_hcnt;
    wire       video_irq;

    reg startup_mode_r = 1'b1;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk, vclk;
    wire video_mode;

    clkctrl clkctrl(
        .clk_in(sysclk),    // 14.31818MHz
        .clk_out(clk),      // 28.63636MHz

        .vclk(vclk),        // 25.175MHz
        .video_mode(1'b1)
    );

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset;

    sysctrl sysctrl(
        .sysclk(clk),
        .ebus_reset_n(ebus_reset_n),
        .reset_req(reset_req),

        .turbo_mode(1'b0),

        .ebus_phi(ebus_phi),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    reg [4:0] reg_bank0_r;
    reg [4:0] reg_bank1_r;
    reg [4:0] reg_bank2_r;
    reg [7:0] reg_ramctrl_r;

    // Select banking register based on upper address bits
    always @* begin
        ebus_ba = 5'b0;

        if (ebus_a[15:14] == 2'b00 && ebus_a >= 16'h400)
            ebus_ba = reg_bank0_r;
        else if (ebus_a[15:14] == 2'b01)
            ebus_ba = reg_bank1_r;
        else if (ebus_a[15:14] == 2'b10)
            ebus_ba = reg_bank2_r;
    end

    // Register data from external bus
    reg [7:0] wrdata;
    always @(posedge clk) if (!ebus_wr_n) wrdata <= ebus_d;

    reg [2:0] ebus_wr_n_r;
    reg [2:0] ebus_rd_n_r;
    always @(posedge clk) ebus_wr_n_r <= {ebus_wr_n_r[1:0], ebus_wr_n};
    always @(posedge clk) ebus_rd_n_r <= {ebus_rd_n_r[1:0], ebus_rd_n};

    wire bus_read       = ebus_rd_n_r[2:1] == 3'b10;
    wire bus_read_done  = ebus_rd_n_r[2:1] == 3'b01;
    wire bus_write      = ebus_wr_n_r[2:1] == 3'b10;
    wire bus_write_done = ebus_wr_n_r[2:1] == 3'b01;

    // Memory space decoding
    wire sel_mem_introm = startup_mode_r && !ebus_mreq_n && ebus_a[15:14] == 2'b00;
    wire sel_mem_intram = !ebus_mreq_n && ebus_a[15:14] == 2'b11;

    wire sel_mem_bank2   = !ebus_mreq_n && ebus_a == 16'hFFFF;
    wire sel_mem_bank1   = !ebus_mreq_n && ebus_a == 16'hFFFE;
    wire sel_mem_bank0   = !ebus_mreq_n && ebus_a == 16'hFFFD;
    wire sel_mem_ramctrl = !ebus_mreq_n && ebus_a == 16'hFFFC;

    // IO space decoding
    wire [2:0] io_addr         = {ebus_a[7], ebus_a[6], ebus_a[0]};
    wire       sel_io_espctrl  = startup_mode_r && !ebus_iorq_n && ebus_a[7:0] == 8'h10;
    wire       sel_io_espdata  = startup_mode_r && !ebus_iorq_n && ebus_a[7:0] == 8'h11;
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
    wire       allow_sel_mem   = !ebus_mreq_n && !sel_internal && (ebus_wr_n || (!ebus_wr_n && startup_mode_r));
    wire       sel_mem_ram     = allow_sel_mem;

    assign     ebus_ram_ce_n   = !sel_mem_ram;
    assign     ebus_ram_we_n   = !(!ebus_wr_n && sel_mem_ram && startup_mode_r);

    wire       ram_wren        = sel_mem_intram && bus_write;
    wire       io_video_wren   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_write;
    wire       io_video_rden   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_read;

    wire       io_psg_wren     = sel_io_psg && bus_write;

    // Generate rddone signal for video
    reg io_video_rddone;
    reg io_video_reading_r;
    always @(posedge clk) begin
        io_video_rddone <= 1'b0;
        if (io_video_rden) io_video_reading_r <= 1'b1;
        if (io_video_reading_r && bus_read_done) begin
            io_video_reading_r <= 1'b0;
            io_video_rddone <= 1'b1;
        end
    end

    // Generate wrdone signal for video
    reg io_video_wrdone, io_video_writing_r;
    always @(posedge clk) begin
        io_video_wrdone <= 1'b0;
        if (io_video_wren) io_video_writing_r <= 1'b1;
        if (io_video_writing_r && bus_write_done) begin
            io_video_writing_r <= 1'b0;
            io_video_wrdone <= 1'b1;
        end
    end

    // Handle region detection at port $3F
    reg [1:0] region_bits_r;
    always @(posedge clk or posedge reset) begin
        if (reset)
            region_bits_r <= 2'b11;
        else begin
            if (sel_io_3f && bus_write) region_bits_r <= {wrdata[7], wrdata[5]};
        end
    end

    wire [7:0] port_dc, port_dd;

    reg [7:0] rddata;
    always @* begin
        rddata <= 8'hFF;

        if (sel_mem_introm)  rddata <= rddata_rom;
        if (sel_mem_intram)  rddata <= rddata_ram;

        if (sel_io_espctrl)  rddata <= rddata_espctrl;
        if (sel_io_espdata)  rddata <= rddata_espdata;
        if (sel_io_vcnt)     rddata <= video_vcnt;
        if (sel_io_hcnt)     rddata <= video_hcnt;
        if (sel_io_vdp_data) rddata <= rddata_io_video;
        if (sel_io_vdp_ctrl) rddata <= rddata_io_video;
        if (sel_io_dc)       rddata <= port_dc;
        if (sel_io_dd)       rddata <= port_dd;
    end

    wire   ebus_d_en  = !ebus_rd_n && sel_internal;
    assign ebus_d     = (spibm_en && spibm_wrdata_en) ? spibm_wrdata : (ebus_d_en ? rddata : 8'bZ);
    assign ebus_int_n = video_irq ? 1'b0 : 1'bZ;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            startup_mode_r <= 1'b1;

            reg_bank0_r   <= 5'd0;
            reg_bank1_r   <= 5'd1;
            reg_bank2_r   <= 5'd2;
            reg_ramctrl_r <= 8'd0;

        end else begin
            if (sel_mem_ramctrl && bus_write) begin
                startup_mode_r <= 1'b0;
                reg_ramctrl_r  <= wrdata;
            end
            if (sel_mem_bank0 && bus_write) reg_bank0_r <= wrdata[4:0];
            if (sel_mem_bank1 && bus_write) reg_bank1_r <= wrdata[4:0];
            if (sel_mem_bank2 && bus_write) reg_bank2_r <= wrdata[4:0];
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // System ROM
    //////////////////////////////////////////////////////////////////////////
    wire rom_p2_wren;

    rom rom(
        .clk(clk),
        .addr(ebus_a[12:0]),
        .rddata(rddata_rom),

        .p2_addr(spibm_a[12:0]),
        .p2_wrdata(spibm_wrdata),
        .p2_wren(rom_p2_wren));

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
    wire esp_txvalid = sel_io_espdata && bus_write;
    wire esp_txbreak = sel_io_espctrl && bus_write && wrdata[7];
    wire esp_txbusy;

    wire esp_rxfifo_not_empty;
    wire esp_rxfifo_read = sel_io_espdata && bus_read;
    wire esp_rxfifo_overflow, esp_rx_framing_error, esp_rx_break;

    reg [2:0] esp_ctrl_status_r;
    always @(posedge clk or posedge reset) begin
        if (reset)
            esp_ctrl_status_r <= 3'b0;
        else begin
            if (sel_io_espctrl && bus_write) esp_ctrl_status_r <= esp_ctrl_status_r & ~wrdata[4:2];

            if (esp_rxfifo_overflow)  esp_ctrl_status_r[2] <= 1'b1;
            if (esp_rx_framing_error) esp_ctrl_status_r[1] <= 1'b1;
            if (esp_rx_break)         esp_ctrl_status_r[0] <= 1'b1;
        end
    end

    assign rddata_espctrl = {3'b0, esp_ctrl_status_r, esp_txbusy, esp_rxfifo_not_empty};

    esp_uart esp_uart(
        .rst(reset),
        .clk(clk),

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
    video video(
        .clk(clk),
        .reset(reset),

        .vclk(vclk),

        .io_portsel(ebus_a[0]),
        .io_rddata(rddata_io_video),
        .io_wrdata(wrdata),
        .io_wren(io_video_wren),
        .io_wrdone(io_video_wrdone),
        .io_rddone(io_video_rddone),
        .irq(video_irq),

        .vcnt(video_vcnt),
        .hcnt(video_hcnt),

        .vga_r(vga_r),
        .vga_g(vga_g),
        .vga_b(vga_b),
        .vga_hsync(vga_hsync),
        .vga_vsync(vga_vsync));

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] spi_hctrl1, spi_hctrl2;

    wire [7:0] hctrl1 = hc1[7:0];
    wire [7:0] hctrl2 = hc2[7:0];
    assign hc1[8] = 1'b0;
    assign hc2[8] = 1'b0;

    // Synchronize inputs
    reg [7:0] hctrl1_r, hctrl1_rr;
    reg [7:0] hctrl2_r, hctrl2_rr;
    always @(posedge clk) hctrl1_r  <= hctrl1;
    always @(posedge clk) hctrl1_rr <= hctrl1_r;
    always @(posedge clk) hctrl2_r  <= hctrl2;
    always @(posedge clk) hctrl2_rr <= hctrl2_r;

    // Combine data from ESP with data from handcontroller input
    wire [7:0] hctrl1_data = hctrl1_rr & spi_hctrl1;
    wire [7:0] hctrl2_data = hctrl2_rr & spi_hctrl2;

    //////////////////////////////////////////////////////////////////////////
    // SPI interface
    //////////////////////////////////////////////////////////////////////////

    // Keyboard matrix
    wire [63:0] keys;

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
    assign port_dd = {region_bits_r, 2'b11, joypad_b_tr, joypad_b_tl, joypad_b_right, joypad_b_left};

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

    spiregs spiregs(
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
        .rom_p2_wren(rom_p2_wren),

        .kbbuf_data(kbbuf_data),
        .kbbuf_wren(kbbuf_wren),

        .video_mode(video_mode));

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

    //////////////////////////////////////////////////////////////////////////
    // PWM DAC
    //////////////////////////////////////////////////////////////////////////
    wire        next_sample = 1'b1;
    wire [15:0] left_data   = psg_sample;
    wire [15:0] right_data  = psg_sample;

    pwm_dac pwm_dac(
        .rst(reset),
        .clk(clk),

        // Sample input
        .next_sample(next_sample),
        .left_data(left_data),
        .right_data(right_data),

        // PWM audio output
        .audio_l(audio_l),
        .audio_r(audio_r));

endmodule
