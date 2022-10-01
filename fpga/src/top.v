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
    output wire  [4:0] bus_ba,
    inout  wire  [7:0] bus_de,          // External data bus, possibly scrambled
    output wire        bus_de_oe_n,
    output wire        ram_ce_n,        // 512KB RAM
    output wire        rom_ce_n,        // 256KB Flash memory
    output wire        cart_ce_n,       // Cartridge

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
    input  wire        esp_cs_n,
    input  wire        esp_sclk,        // Connected to EXP7
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    wire       vga_vblank;

    reg        reg_cpm_remap_r;         // $FD:W
    reg  [7:0] reg_scramble_value_r;    // $FF:W

    wire [7:0] rddata_bootrom;
    wire [7:0] rddata_vram;
    wire [7:0] rddata_sysram;
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
    reg [7:0] wrdata;
    always @(posedge sysclk) if (!bus_wr_n) wrdata <= bus_d;

    reg [2:0] bus_wr_n_r;
    reg [2:0] bus_rd_n_r;
    always @(posedge sysclk) bus_wr_n_r <= {bus_wr_n_r[1:0], bus_wr_n};
    always @(posedge sysclk) bus_rd_n_r <= {bus_rd_n_r[1:0], bus_rd_n};

    wire bus_read  = bus_rd_n_r[2:1] == 3'b10;
    wire bus_write = bus_wr_n_r[2:1] == 3'b10;

    // Memory space decoding
    wire sel_mem_bootrom = !bus_mreq_n && bus_a[15:13] == 3'b000;       // $0000-$1FFF
    wire sel_mem_vram    = !bus_mreq_n && bus_a[15:11] == 5'b00110;     // $3000-$37FF
    wire sel_mem_sysram  = !bus_mreq_n && bus_a[15:11] == 5'b00111;     // $3800-$3FFF

    // IO space decoding
    wire sel_io_espctrl           = !bus_iorq_n && bus_a[7:0] == 8'hF4;
    wire sel_io_espdata           = !bus_iorq_n && bus_a[7:0] == 8'hF5;
    wire sel_io_cassette          = !bus_iorq_n && bus_a[7:0] == 8'hFC;
    wire sel_io_vsync_r_cpm_w     = !bus_iorq_n && bus_a[7:0] == 8'hFD;
    wire sel_io_printer           = !bus_iorq_n && bus_a[7:0] == 8'hFE;
    wire sel_io_keyb_r_scramble_w = !bus_iorq_n && bus_a[7:0] == 8'hFF;

    wire sel_internal =
        sel_mem_bootrom | sel_mem_vram | sel_mem_sysram |
        sel_io_espctrl | sel_io_espdata |
        sel_io_cassette | sel_io_vsync_r_cpm_w | sel_io_printer | sel_io_keyb_r_scramble_w;

    reg [7:0] rddata;
    always @* begin
        rddata <= 8'h00;
        if (sel_mem_bootrom)          rddata <= rddata_bootrom;         // ROM  $0000-$1FFF 
        if (sel_mem_vram)             rddata <= rddata_vram;            // VRAM $3000-$37FF
        if (sel_mem_sysram)           rddata <= rddata_sysram;          // RAM  $3800-$3FFF
        if (sel_io_espctrl)           rddata <= rddata_espctrl;         // IO $F4
        if (sel_io_espdata)           rddata <= rddata_espdata;         // IO $F5
        if (sel_io_cassette)          rddata <= {7'b0, cassette_in};    // IO $FC
        if (sel_io_vsync_r_cpm_w)     rddata <= {7'b0, !vga_vblank};    // IO $FD
        if (sel_io_printer)           rddata <= {7'b0, printer_in};     // IO $FE
        if (sel_io_keyb_r_scramble_w) rddata <= 8'hFF;                  // IO $FF
    end

    wire bus_d_enable = !bus_rd_n && sel_internal;

    assign bus_d = bus_d_enable ? rddata : 8'bZ;

    assign bus_int_n    = 1'bZ;
    assign bus_wait_n   = 1'bZ;
    assign bus_busreq_n = 1'bZ;
    assign bus_de_oe_n  = 1'b1;
    assign cart_ce_n    = 1'b1;

    assign bus_ba = 5'b0;
    assign ram_ce_n = 1'b1;
    assign rom_ce_n = 1'b1;

    assign exp          = 7'b0;

    assign esp_notify   = 1'b0;

    always @(posedge sysclk or posedge reset)
        if (reset) begin
            cassette_out         <= 1'b0;
            reg_cpm_remap_r      <= 1'b0;
            printer_out          <= 1'b0;
            reg_scramble_value_r <= 8'b0;
        end else begin
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
        .addr(bus_a[12:0]),
        .rddata(rddata_bootrom));

    //////////////////////////////////////////////////////////////////////////
    // System RAM (2kB)
    //////////////////////////////////////////////////////////////////////////
    wire sysram_wren = sel_mem_sysram && bus_write;

    sysram sysram(
        .clk(sysclk),
        .addr(bus_a[10:0]),
        .rddata(rddata_sysram),
        .wrdata(wrdata),
        .wren(sysram_wren));

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
    wire [10:0] vram_addr   = bus_a[10:0];
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
