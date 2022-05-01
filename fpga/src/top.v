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
    // Boot ROM
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] bootrom_data;

    bootrom bootrom(
        .addr(bus_a[7:0]),
        .data(bootrom_data));

    wire bootrom_sel  = bus_mreq_n == 1'b0 && bus_a[15:8] == 8'h00;
    wire bus_d_enable = !bus_rd_n && bootrom_sel;
    wire [7:0] rddata = bootrom_data;

    assign bus_d = bus_d_enable ? rddata : 8'bZ;

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
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
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

    assign esp_tx       = 1'b0;
    assign esp_rts      = 1'b0;

    assign esp_notify   = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    wire iord  =  bus_rd_n && bus_iorq_n;
    wire iowr  = !bus_rd_n && bus_iorq_n;
    wire memrd =  bus_rd_n && bus_mreq_n;
    wire memwr = !bus_rd_n && bus_mreq_n;

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
