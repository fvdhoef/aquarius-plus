`timescale 1 ns / 1 ps

module tb();

    // Generate approx. 14.31818MHz sysclk
    reg sysclk = 0;
    always #34.92 sysclk = !sysclk;

    reg  [7:0] bus_wrdata = 8'b0;
    reg        bus_wren   = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // aqp_top
    //////////////////////////////////////////////////////////////////////////
    reg  [15:0] q_ebus_a      = 16'h0000;
    reg         q_ebus_rd_n   = 1'b1;
    reg         q_ebus_wr_n   = 1'b1;
    reg         q_ebus_mreq_n = 1'b1;
    reg         q_ebus_iorq_n = 1'b1;

    reg         ebus_busack_n = 1'bZ;

    wire        ebus_phi;
    wire        ebus_reset_n;
    wire [15:0] ebus_a      = (ebus_busack_n == 1'b0) ? 16'bZ : q_ebus_a;
    wire  [7:0] ebus_d      = bus_wren ? bus_wrdata : 8'bZ;
    wire        ebus_rd_n   = (ebus_busack_n == 1'b0) ? 1'bZ  : q_ebus_rd_n;
    wire        ebus_wr_n   = (ebus_busack_n == 1'b0) ? 1'bZ  : q_ebus_wr_n;
    wire        ebus_mreq_n = (ebus_busack_n == 1'b0) ? 1'bZ  : q_ebus_mreq_n;
    wire        ebus_iorq_n = (ebus_busack_n == 1'b0) ? 1'bZ  : q_ebus_iorq_n;
    wire        ebus_int_n;
    wire        ebus_busreq_n;
    wire  [4:0] ebus_ba;
    wire        ebus_ram_ce_n;
    wire        ebus_cart_ce_n;
    wire        ebus_ram_we_n;

    wire        audio_l;
    wire        audio_r;

    wire        cassette_out;
    wire        cassette_in = 1'b0;
    wire        printer_out;
    wire        printer_in = 1'b1;

    wire  [8:0] exp;
    wire        has_z80 = 1'b0;

    wire  [8:0] hc1;
    wire  [8:0] hc2;

    wire  [3:0] vga_r;
    wire  [3:0] vga_g;
    wire  [3:0] vga_b;
    wire        vga_hsync;
    wire        vga_vsync;

    wire        esp_tx;
    reg         esp_rx = 1'b1;
    wire        esp_rts;
    wire        esp_cts = 1'b0;

    reg         esp_ssel_n = 1'b1;
    reg         esp_sclk = 1'b0;
    reg         esp_mosi = 1'b0;
    wire        esp_miso;
    wire        esp_notify;

    pullup(ebus_reset_n);
    pullup(ebus_busreq_n);
    pullup(hc1[7]);
    pullup(hc1[6]);
    pullup(hc1[5]);
    pullup(hc1[4]);
    pullup(hc1[3]);
    pullup(hc1[2]);
    pullup(hc1[1]);
    pullup(hc1[0]);
    pullup(hc2[7]);
    pullup(hc2[6]);
    pullup(hc2[5]);
    pullup(hc2[4]);
    pullup(hc2[3]);
    pullup(hc2[2]);
    pullup(hc2[1]);
    pullup(hc2[0]);

    pullup(ebus_d[7]);
    pullup(ebus_d[6]);
    pullup(ebus_d[5]);
    pullup(ebus_d[4]);
    pullup(ebus_d[3]);
    pullup(ebus_d[2]);
    pullup(ebus_d[1]);
    pullup(ebus_d[0]);

    always @(posedge ebus_phi) ebus_busack_n <= ebus_busreq_n;

    aq32_top top_inst(
        .sysclk(sysclk),

        // Z80 bus interface
        .ebus_reset_n(ebus_reset_n),
        .ebus_phi(ebus_phi),
        .ebus_a(ebus_a),
        .ebus_d(ebus_d),
        .ebus_rd_n(ebus_rd_n),
        .ebus_wr_n(ebus_wr_n),
        .ebus_mreq_n(ebus_mreq_n),
        .ebus_iorq_n(ebus_iorq_n),
        .ebus_int_n(ebus_int_n),
        .ebus_busreq_n(ebus_busreq_n),
        .ebus_busack_n(ebus_busack_n),
        .ebus_ba(ebus_ba),
        .ebus_ram_ce_n(ebus_ram_ce_n),
        .ebus_cart_ce_n(ebus_cart_ce_n),
        .ebus_ram_we_n(ebus_ram_we_n),

        // PWM audio outputs
        .audio_l(audio_l),
        .audio_r(audio_r),

        // Other
        .cassette_out(cassette_out),
        .cassette_in(cassette_in),
        .printer_out(printer_out),
        .printer_in(printer_in),

        // Misc
        .exp(exp),
        .has_z80(has_z80),

        // Hand controller interface
        .hc1(hc1),
        .hc2(hc2),

        // VGA output
        .vga_r(vga_r),
        .vga_g(vga_g),
        .vga_b(vga_b),
        .vga_hsync(vga_hsync),
        .vga_vsync(vga_vsync),

        // ESP32 serial interface
        .esp_tx(esp_tx),
        .esp_rx(esp_rx),
        .esp_rts(esp_rts),
        .esp_cts(esp_cts),

        // ESP32 SPI interface (also used for loading FPGA image)
        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),
        .esp_notify(esp_notify));

    //////////////////////////////////////////////////////////////////////////
    // IS61C5128AS RAM
    //////////////////////////////////////////////////////////////////////////
    is61c5128as sram(
        .A({ebus_ba, ebus_a[13:0]}),
        .IO(ebus_d),
        .CE_n(ebus_ram_ce_n),
        .OE_n(ebus_rd_n),
        .WE_n(ebus_ram_we_n)
    );

endmodule
