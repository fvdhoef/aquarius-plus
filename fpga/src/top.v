module top(
    input  wire        sysclk,          // 14.31818MHz
    input  wire        usbclk,          // 48MHz

    // Z80 bus interface
    inout  wire        reset_n,
    output wire        phi,             // 3.579545MHz
    input  wire [15:0] bus_a,
    inout  wire  [7:0] bus_d,
    input  wire        bus_rd_n,
    input  wire        bus_wr_n,
    input  wire        bus_mreq_n,
    input  wire        bus_iorq_n,
    output wire        bus_int_n,       // Open-drain output
    input  wire        bus_m1_n,
    output wire        bus_wait_n,      // Open-drain output
    output wire        bus_busreq_n,    // Open-drain output
    input  wire        bus_busack_n,
    output reg   [4:0] bus_ba,
    inout  wire  [7:0] bus_de,          // External data bus, possibly scrambled
    output wire        bus_de_oe_n,
    output wire        ram_ce_n,        // 512KB RAM
    output wire        rom_ce_n,        // 256KB Flash memory
    output wire        cart_ce_n,       // Cartridge

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output wire        cassette_out,
    input  wire        cassette_in,
    output wire        printer_out,
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
    input  wire        esp_cs_n,
    input  wire        esp_sclk,        // Connected to EXP7
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    wire [7:0] rddata_bootrom;
    wire [7:0] rddata_espctrl;
    wire [7:0] rddata_espdata;

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset;

    sysctrl sysctrl(
        .sysclk(sysclk),
        .ext_reset_n(reset_n),

        .phi(phi),
        .reset(reset));

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    wire io_write  = !bus_iorq_n && !bus_wr_n;
    wire io_read   = !bus_iorq_n && !bus_rd_n;
    wire mem_write = !bus_mreq_n && !bus_wr_n;
    wire mem_read  = !bus_mreq_n && !bus_rd_n;

    reg iosel;

    reg [7:0] wrdata;
    always @(posedge sysclk) if (!bus_wr_n) wrdata <= bus_d;

    reg [2:0] bus_wr_n_r;
    reg [2:0] bus_rd_n_r;
    always @(posedge sysclk) bus_wr_n_r <= {bus_wr_n_r[1:0], bus_wr_n};
    always @(posedge sysclk) bus_rd_n_r <= {bus_rd_n_r[1:0], bus_rd_n};

    wire bus_read  = bus_rd_n_r[2:1] == 3'b10;
    wire bus_write = bus_wr_n_r[2:1] == 3'b10;

    // Memory space decoding
    wire sel_mem_bootrom = !bus_mreq_n && bus_a[15:8] == 8'h00;

    // IO space decoding
    wire sel_io_vctrl    = !bus_iorq_n && bus_a[7:0] == 8'hE0;
    wire sel_io_vscrx_l  = !bus_iorq_n && bus_a[7:0] == 8'hE1;
    wire sel_io_vscrx_h  = !bus_iorq_n && bus_a[7:0] == 8'hE2;
    wire sel_io_vscry    = !bus_iorq_n && bus_a[7:0] == 8'hE3;
    wire sel_io_vsprsel  = !bus_iorq_n && bus_a[7:0] == 8'hE4;
    wire sel_io_vsprx_l  = !bus_iorq_n && bus_a[7:0] == 8'hE5;
    wire sel_io_vsprx_h  = !bus_iorq_n && bus_a[7:0] == 8'hE6;
    wire sel_io_vspry    = !bus_iorq_n && bus_a[7:0] == 8'hE7;
    wire sel_io_vspridx  = !bus_iorq_n && bus_a[7:0] == 8'hE8;
    wire sel_io_vsprattr = !bus_iorq_n && bus_a[7:0] == 8'hE9;
    wire sel_io_vpalsel  = !bus_iorq_n && bus_a[7:0] == 8'hEA;
    wire sel_io_vpaldata = !bus_iorq_n && bus_a[7:0] == 8'hEB;
    wire sel_io_vline    = !bus_iorq_n && bus_a[7:0] == 8'hEC;
    wire sel_io_virqline = !bus_iorq_n && bus_a[7:0] == 8'hED;
    wire sel_io_irqmask  = !bus_iorq_n && bus_a[7:0] == 8'hEE;
    wire sel_io_irqstat  = !bus_iorq_n && bus_a[7:0] == 8'hEF;
    wire sel_io_bank0    = !bus_iorq_n && bus_a[7:0] == 8'hF0;
    wire sel_io_bank1    = !bus_iorq_n && bus_a[7:0] == 8'hF1;
    wire sel_io_bank2    = !bus_iorq_n && bus_a[7:0] == 8'hF2;
    wire sel_io_bank3    = !bus_iorq_n && bus_a[7:0] == 8'hF3;
    wire sel_io_espctrl  = !bus_iorq_n && bus_a[7:0] == 8'hF4;
    wire sel_io_espdata  = !bus_iorq_n && bus_a[7:0] == 8'hF5;
    wire sel_io_psg1data = !bus_iorq_n && bus_a[7:0] == 8'hF6;
    wire sel_io_psg1addr = !bus_iorq_n && bus_a[7:0] == 8'hF7;
    wire sel_io_psg2data = !bus_iorq_n && bus_a[7:0] == 8'hF8;
    wire sel_io_psg2addr = !bus_iorq_n && bus_a[7:0] == 8'hF9;
    wire sel_io_sysctrl  = !bus_iorq_n && bus_a[7:0] == 8'hFB;
    wire sel_io_cassette = !bus_iorq_n && bus_a[7:0] == 8'hFC;
    wire sel_io_cpm      = !bus_iorq_n && bus_a[7:0] == 8'hFD;
    wire sel_io_vsync    = !bus_iorq_n && bus_a[7:0] == 8'hFD;
    wire sel_io_printer  = !bus_iorq_n && bus_a[7:0] == 8'hFE;
    wire sel_io_scramble = !bus_iorq_n && bus_a[7:0] == 8'hFF;
    wire sel_io_keyboard = !bus_iorq_n && bus_a[7:0] == 8'hFF;

    wire sel_internal =
        sel_mem_bootrom |
        sel_io_vctrl | sel_io_vscrx_l | sel_io_vscrx_h | sel_io_vscry |
        sel_io_vsprsel | sel_io_vsprx_l | sel_io_vsprx_h | sel_io_vspry |
        sel_io_vspridx | sel_io_vsprattr | sel_io_vpalsel | sel_io_vpaldata |
        sel_io_vline | sel_io_virqline | sel_io_irqmask | sel_io_irqstat |
        sel_io_bank0 | sel_io_bank1 | sel_io_bank2 | sel_io_bank3 |
        sel_io_espctrl | sel_io_espdata | sel_io_psg1data | sel_io_psg1addr |
        sel_io_psg2data | sel_io_psg2addr | sel_io_sysctrl | sel_io_cassette |
        sel_io_cpm | sel_io_vsync | sel_io_printer | sel_io_scramble | sel_io_keyboard;


    reg [7:0] rddata;
    always @* begin
        rddata <= 8'h00;
        if (sel_mem_bootrom) rddata <= rddata_bootrom;
        if (sel_io_espctrl)  rddata <= rddata_espctrl;
        if (sel_io_espdata)  rddata <= rddata_espdata;
    end

    wire bus_d_enable = !bus_rd_n && sel_internal;

    assign bus_d = bus_d_enable ? rddata : 8'bZ;

    // wire  [7:0] bus_wrdata;
    // wire  [7:0] bus_rddata = bus_d;
    // wire        bus_d_oe;

    // assign bus_d = bus_d_oe ? bus_rddata : 8'bZ;

    // busif busif(
    //     .clk(sysclk),
    //     .reset(reset),

    //     .bus_a(bus_a),
    //     .bus_wrdata(bus_wrdata),
    //     .bus_rddata(bus_rddata),
    //     .bus_d_oe(bus_d_oe),
    //     .bus_rd_n(bus_rd_n),
    //     .bus_wr_n(bus_wr_n),
    //     .bus_mreq_n(bus_mreq_n),
    //     .bus_iorq_n(bus_iorq_n));

    assign bus_int_n    = 1'bZ;
    assign bus_wait_n   = 1'bZ;
    assign bus_busreq_n = 1'bZ;
    assign bus_de_oe_n  = 1'b1;
    assign ram_ce_n     = 1'b1;
    assign rom_ce_n     = 1'b1;
    assign cart_ce_n    = 1'b1;

    assign cassette_out = 1'b0;
    assign printer_out  = 1'b0;

    assign exp          = 7'b0;

    assign esp_notify   = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Boot ROM
    //////////////////////////////////////////////////////////////////////////
    bootrom bootrom(
        .addr(bus_a[7:0]),
        .data(rddata_bootrom));

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    wire       esp_txvalid = sel_io_espdata && bus_write;
    wire       esp_txbreak = sel_io_espctrl && bus_write && wrdata[7];
    wire       esp_txbusy;

    wire       esp_rxfifo_not_empty;
    wire       esp_rxfifo_read = sel_io_espdata && bus_read;
    wire       esp_rxfifo_overflow, esp_rx_framing_error, esp_rx_break;

    reg  [2:0] esp_ctrl_status_r;
    always @(posedge sysclk, posedge reset) begin
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
    video video(
        .clk(sysclk),
        .reset(reset),

        .vga_r(vga_r),
        .vga_g(vga_g),
        .vga_b(vga_b),
        .vga_hsync(vga_hsync),
        .vga_vsync(vga_vsync));

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
    // Banking registers
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] bank0_reg_r;
    reg [7:0] bank1_reg_r;
    reg [7:0] bank2_reg_r;
    reg [7:0] bank3_reg_r;  

    always @* case (bus_a[15:14])
        2'd0: bus_ba = bank0_reg_r[4:0];
        2'd1: bus_ba = bank1_reg_r[4:0];
        2'd2: bus_ba = bank2_reg_r[4:0];
        2'd3: bus_ba = bank3_reg_r[4:0];
    endcase

    //////////////////////////////////////////////////////////////////////////
    // SPI slave
    //////////////////////////////////////////////////////////////////////////
    spislave spislave(
        .clk(sysclk),
        .reset(reset),

        .esp_cs_n(esp_cs_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso));

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
