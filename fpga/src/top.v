module top(
    input  wire        sysclk,          // 14.31818MHz
    input  wire        usbclk,          // 48MHz

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
    input  wire        ebus_m1_n,
    output wire        ebus_wait_n,     // Open-drain output
    output wire        ebus_busreq_n,   // Open-drain output
    input  wire        ebus_busack_n,
    output wire  [4:0] ebus_ba,
    inout  wire  [7:0] ebus_cart_d,     // Cartridge data bus, possibly scrambled
    output wire        ebus_cart_d_oe_n,
    output wire        ebus_ram_ce_n,   // 512KB RAM
    output wire        ebus_rom_ce_n,   // 256KB Flash memory
    output wire        ebus_cart_ce_n,  // Cartridge

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output reg         cassette_out,
    input  wire        cassette_in,
    output reg         printer_out,
    input  wire        printer_in,

    // USB
    inout  wire        usb_dp1,
    inout  wire        usb_dm1,
    inout  wire        usb_dp2,
    inout  wire        usb_dm2,

    // Misc
    output wire  [6:0] exp,

    // Hand controller interface
    output wire        hctrl_clk,
    output wire        hctrl_load_n,
    input  wire        hctrl_data,

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

    wire [7:0] spibm_wrdata;
    wire       spibm_wrdata_en;
    wire       spibm_en;

    wire       reset_req;
    wire       vga_vblank;

    wire [7:0] rddata_bootrom;          // MEM $0000-$1FFF
    wire [7:0] rddata_vram;             // MEM $3000-$37FF

    wire [7:0] rddata_espctrl;          // IO $F4
    wire [7:0] rddata_espdata;          // IO $F5
    wire [7:0] rddata_keyboard;         // IO $FF:R

    reg  [7:0] reg_bank0_r;             // IO $F0
    reg  [7:0] reg_bank1_r;             // IO $F1
    reg  [7:0] reg_bank2_r;             // IO $F2
    reg  [7:0] reg_bank3_r;             // IO $F3
    reg        reg_cpm_remap_r;         // IO $FD:W
    reg  [7:0] reg_scramble_value_r;    // IO $FF:W

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset;

    sysctrl sysctrl(
        .sysclk(sysclk),
        .ebus_reset_n(ebus_reset_n),
        .reset_req(reset_req),

        .ebus_phi(ebus_phi),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] wrdata;
    always @(posedge sysclk) if (!ebus_wr_n) wrdata <= ebus_d;

    reg [2:0] ebus_wr_n_r;
    reg [2:0] ebus_rd_n_r;
    always @(posedge sysclk) ebus_wr_n_r <= {ebus_wr_n_r[1:0], ebus_wr_n};
    always @(posedge sysclk) ebus_rd_n_r <= {ebus_rd_n_r[1:0], ebus_rd_n};

    wire bus_read  = ebus_rd_n_r[2:1] == 3'b10;
    wire bus_write = ebus_wr_n_r[2:1] == 3'b10;

    // Memory space decoding
    wire sel_mem_bootrom = !ebus_mreq_n && ebus_a[15:13] == 3'b000;       // $0000-$1FFF
    wire sel_mem_vram    = !ebus_mreq_n && ebus_a[15:11] == 5'b00110;     // $3000-$37FF
    wire sel_mem_sysram  = !ebus_mreq_n && ebus_a[15:11] == 5'b00111;     // $3800-$3FFF

    wire bank1 = !ebus_mreq_n && ebus_a[15:14] == 2'b01;
    wire bank2 = !ebus_mreq_n && ebus_a[15:14] == 2'b10;
    wire bank3 = !ebus_mreq_n && ebus_a[15:14] == 2'b11;

    assign ebus_ram_ce_n = !(sel_mem_sysram | bank1 | bank2);

    // IO space decoding
    wire sel_io_bank0             = !ebus_iorq_n && ebus_a[7:0] == 8'hF0;
    wire sel_io_bank1             = !ebus_iorq_n && ebus_a[7:0] == 8'hF1;
    wire sel_io_bank2             = !ebus_iorq_n && ebus_a[7:0] == 8'hF2;
    wire sel_io_bank3             = !ebus_iorq_n && ebus_a[7:0] == 8'hF3;
    wire sel_io_espctrl           = !ebus_iorq_n && ebus_a[7:0] == 8'hF4;
    wire sel_io_espdata           = !ebus_iorq_n && ebus_a[7:0] == 8'hF5;
    wire sel_io_cassette          = !ebus_iorq_n && ebus_a[7:0] == 8'hFC;
    wire sel_io_vsync_r_cpm_w     = !ebus_iorq_n && ebus_a[7:0] == 8'hFD;
    wire sel_io_printer           = !ebus_iorq_n && ebus_a[7:0] == 8'hFE;
    wire sel_io_keyb_r_scramble_w = !ebus_iorq_n && ebus_a[7:0] == 8'hFF;

    wire sel_internal =
        sel_mem_bootrom | sel_mem_vram |
        sel_io_bank0 | sel_io_bank1 | sel_io_bank2 | sel_io_bank3 |
        sel_io_espctrl | sel_io_espdata |
        sel_io_cassette | sel_io_vsync_r_cpm_w | sel_io_printer | sel_io_keyb_r_scramble_w;

    reg [7:0] rddata;
    always @* begin
        rddata <= 8'h00;
        if (sel_mem_bootrom)          rddata <= rddata_bootrom;         // ROM  $0000-$1FFF 
        if (sel_mem_vram)             rddata <= rddata_vram;            // VRAM $3000-$37FF

        if (sel_io_bank0)             rddata <= reg_bank0_r;            // IO $F0
        if (sel_io_bank1)             rddata <= reg_bank1_r;            // IO $F1
        if (sel_io_bank2)             rddata <= reg_bank2_r;            // IO $F2
        if (sel_io_bank3)             rddata <= reg_bank3_r;            // IO $F3
        if (sel_io_espctrl)           rddata <= rddata_espctrl;         // IO $F4
        if (sel_io_espdata)           rddata <= rddata_espdata;         // IO $F5
        if (sel_io_cassette)          rddata <= {7'b0, cassette_in};    // IO $FC
        if (sel_io_vsync_r_cpm_w)     rddata <= {7'b0, !vga_vblank};    // IO $FD
        if (sel_io_printer)           rddata <= {7'b0, printer_in};     // IO $FE
        if (sel_io_keyb_r_scramble_w) rddata <= rddata_keyboard;        // IO $FF
    end

    wire ebus_d_enable = !ebus_rd_n && sel_internal;

    assign ebus_d = (spibm_en && spibm_wrdata_en) ? spibm_wrdata : (ebus_d_enable ? rddata : 8'bZ);

    assign ebus_int_n       = 1'bZ;
    assign ebus_wait_n      = 1'bZ;
    assign ebus_cart_d_oe_n = 1'b1;
    assign ebus_cart_d      = 8'bZ;
    assign ebus_cart_ce_n   = 1'b1;
    assign ebus_ba          = 5'b0;
    assign ebus_rom_ce_n    = 1'b1;

    assign exp              = 7'b0;

    always @(posedge sysclk or posedge reset)
        if (reset) begin
            reg_bank0_r          <= 8'h00;
            reg_bank1_r          <= 8'h00;
            reg_bank2_r          <= 8'h00;
            reg_bank3_r          <= 8'h00;
            cassette_out         <= 1'b0;
            reg_cpm_remap_r      <= 1'b0;
            printer_out          <= 1'b0;
            reg_scramble_value_r <= 8'b0;

        end else begin
            if (sel_io_bank0             && bus_write) reg_bank0_r          <= wrdata;
            if (sel_io_bank1             && bus_write) reg_bank1_r          <= wrdata;
            if (sel_io_bank2             && bus_write) reg_bank2_r          <= wrdata;
            if (sel_io_bank3             && bus_write) reg_bank3_r          <= wrdata;
            if (sel_io_cassette          && bus_write) cassette_out         <= wrdata[0];
            if (sel_io_vsync_r_cpm_w     && bus_write) reg_cpm_remap_r      <= wrdata[0];
            if (sel_io_printer           && bus_write) printer_out          <= wrdata[0];
            if (sel_io_keyb_r_scramble_w && bus_write) reg_scramble_value_r <= wrdata;
        end

    //////////////////////////////////////////////////////////////////////////
    // Boot ROM
    //////////////////////////////////////////////////////////////////////////
    bootrom bootrom(
        .clk(sysclk),
        .addr(ebus_a[12:0]),
        .rddata(rddata_bootrom));

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
    always @(posedge sysclk or posedge reset) begin
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
        .clk(sysclk),

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
    wire [10:0] vram_addr   = ebus_a[10:0];
    wire  [7:0] vram_wrdata = wrdata;
    wire        vram_wren   = sel_mem_vram && bus_write;

    video video(
        .clk(sysclk),
        .reset(reset),

        .vram_addr(vram_addr),
        .vram_rddata(rddata_vram),
        .vram_wrdata(vram_wrdata),
        .vram_wren(vram_wren),

        .vga_r(vga_r),
        .vga_g(vga_g),
        .vga_b(vga_b),
        .vga_hsync(vga_hsync),
        .vga_vsync(vga_vsync),
        
        .vga_vblank(vga_vblank));

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] hctrl1_data;
    wire [7:0] hctrl2_data;

    handctrl handctrl(
        .clk(sysclk),
        .reset(reset),

        .hctrl_clk(hctrl_clk),
        .hctrl_load_n(hctrl_load_n),
        .hctrl_data(hctrl_data),

        .hctrl1_data(hctrl1_data),
        .hctrl2_data(hctrl2_data));

    //////////////////////////////////////////////////////////////////////////
    // SPI interface
    //////////////////////////////////////////////////////////////////////////
    wire [63:0] keys;

    wire [15:0] spibm_a;
    wire        spibm_rd_n, spibm_wr_n, spibm_mreq_n, spibm_iorq_n;
    wire        spibm_busreq;

    assign spibm_en      = spibm_busreq && !ebus_busack_n;
    assign ebus_a        = spibm_en ? spibm_a      : 16'bZ;
    assign ebus_rd_n     = spibm_en ? spibm_rd_n   : 1'bZ;
    assign ebus_wr_n     = spibm_en ? spibm_wr_n   : 1'bZ;
    assign ebus_mreq_n   = spibm_en ? spibm_mreq_n : 1'bZ;
    assign ebus_iorq_n   = spibm_en ? spibm_iorq_n : 1'bZ;
    assign ebus_busreq_n = spibm_busreq ? 1'b0 : 1'bZ;

    spiregs spiregs(
        .clk(sysclk),

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
        .keys(keys));

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
    // PWM DAC
    //////////////////////////////////////////////////////////////////////////
    wire        next_sample = 1'b0;
    wire [15:0] left_data   = 16'b0;
    wire [15:0] right_data  = 16'b0;

    pwm_dac pwm_dac(
        .rst(reset),
        .clk(sysclk),

        // Sample input
        .next_sample(next_sample),
        .left_data(left_data),
        .right_data(right_data),

        // PWM audio output
        .audio_l(audio_l),
        .audio_r(audio_r));

endmodule
