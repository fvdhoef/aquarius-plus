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
    output wire  [4:0] ebus_ba,
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

    reg   [7:0] reg_bank0_r;             // IO $F0
    reg   [7:0] reg_bank1_r;             // IO $F1
    reg   [7:0] reg_bank2_r;             // IO $F2
    reg   [7:0] reg_bank3_r;             // IO $F3

    wire [7:0] rddata_rom;
    wire [7:0] rddata_ram;
    wire [7:0] rddata_espctrl;
    wire [7:0] rddata_espdata;
    wire [7:0] rddata_io_video;

    wire [7:0] video_vcnt;
    wire [7:0] video_hcnt;
    wire       video_irq;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk, vclk;
    wire video_mode;

    clkctrl clkctrl(
        .clk_in(sysclk),    // 14.31818MHz
        .clk_out(clk),      // 28.63636MHz

        .vclk(vclk),        // 25.175MHz
        .video_mode(1'b0)
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

    // Select banking register based on upper address bits
    reg [7:0] reg_bank;
    always @* case (ebus_a[15:14])
        2'd0: reg_bank = reg_bank0_r;
        2'd1: reg_bank = reg_bank1_r;
        2'd2: reg_bank = reg_bank2_r;
        2'd3: reg_bank = reg_bank3_r;
    endcase

    wire [5:0] reg_bank_page    = reg_bank[5:0];
    wire       reg_bank_ro      = reg_bank[7];
    wire       reg_bank_overlay = reg_bank[6];

    // Register data from external bus
    reg [7:0] wrdata;
    always @(posedge clk) if (!ebus_wr_n) wrdata <= ebus_d;

    reg [2:0] ebus_wr_n_r;
    reg [2:0] ebus_rd_n_r;
    always @(posedge clk) ebus_wr_n_r <= {ebus_wr_n_r[1:0], ebus_wr_n};
    always @(posedge clk) ebus_rd_n_r <= {ebus_rd_n_r[1:0], ebus_rd_n};

    wire bus_read      = ebus_rd_n_r[2:1] == 3'b10;
    wire bus_read_done = ebus_rd_n_r[2:1] == 3'b01;
    wire bus_write     = ebus_wr_n_r[2:1] == 3'b10;

    // Memory space decoding
    wire sel_mem_tram    = !ebus_mreq_n && reg_bank_overlay && ebus_a[13:11] == 3'b110;   // $3000-$37FF
    wire sel_mem_sysram  = !ebus_mreq_n && reg_bank_overlay && ebus_a[13:11] == 3'b111;   // $3800-$3FFF
    wire sel_mem_vram    = !ebus_mreq_n && reg_bank_page == 6'd20;                        // Page 20
    wire sel_mem_chram   = !ebus_mreq_n && reg_bank_page == 6'd21;                        // Page 21
    wire sel_mem_rom     = !ebus_mreq_n && reg_bank_page <= 6'd3 && !sel_mem_sysram;      // Page 0-3
    wire sel_mem_ram     = !ebus_mreq_n && ebus_a[15:14] == 2'b11;

    // IO space decoding
    wire io_addr = {ebus_a[7], ebus_a[6], ebus_a[0]};
    wire sel_io_vcnt     = !ebus_iorq_n && io_addr == 3'b010;
    wire sel_io_hcnt     = !ebus_iorq_n && io_addr == 3'b011;
    wire sel_io_vdp_data = !ebus_iorq_n && io_addr == 3'b100;
    wire sel_io_vdp_ctrl = !ebus_iorq_n && io_addr == 3'b101;
    wire sel_io_joy1     = !ebus_iorq_n && io_addr == 3'b110;
    wire sel_io_joy2     = !ebus_iorq_n && io_addr == 3'b111;

    wire sel_internal = 1'b1;

    wire ram_wren        = sel_mem_ram && bus_write;
    wire io_video_wren   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_write;
    wire io_video_rddone = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_read_done;

    assign ebus_ram_ce_n = 1'b1;
    assign ebus_ram_we_n = 1'b1;

    reg [7:0] rddata;
    always @* begin
        rddata <= 8'h00;
        if (sel_mem_rom)              rddata <= rddata_rom;
        if (sel_mem_ram)              rddata <= rddata_ram;

        if (sel_io_vcnt)              rddata <= video_vcnt;
        if (sel_io_hcnt)              rddata <= video_hcnt;
        if (sel_io_vdp_data)          rddata <= rddata_io_video;
        if (sel_io_vdp_ctrl)          rddata <= rddata_io_video;
        if (sel_io_joy1)              rddata <= 8'hFF;
        if (sel_io_joy2)              rddata <= 8'hFF;
    end

    wire   ebus_d_en  = !ebus_rd_n && sel_internal;
    assign ebus_ba    = 4'b0;
    assign ebus_d     = (spibm_en && spibm_wrdata_en) ? spibm_wrdata : (ebus_d_en ? rddata : 8'bZ);
    assign ebus_int_n = video_irq ? 1'b0 : 1'bZ;

    //////////////////////////////////////////////////////////////////////////
    // System ROM
    //////////////////////////////////////////////////////////////////////////
    wire rom_p2_wren;

    rom rom(
        .clk(clk),
        .addr(ebus_a[14:0]),
        .rddata(rddata_rom),

        .p2_addr(spibm_a[14:0]),
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
    wire esp_txvalid = 1'b0;    //sel_io_espdata && bus_write;
    wire esp_txbreak = 1'b0;    //sel_io_espctrl && bus_write && wrdata[7];
    wire esp_txbusy;

    wire sel_io_espctrl = 1'b0;

    wire esp_rxfifo_not_empty;
    wire esp_rxfifo_read = 1'b0;    //sel_io_espdata && bus_read;
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
    // PWM DAC
    //////////////////////////////////////////////////////////////////////////
    wire        next_sample = 1'b1;
    wire [15:0] left_data   = 16'b0;
    wire [15:0] right_data  = 16'b0;

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
