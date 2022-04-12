`timescale 1 ns / 1 ps

module tb();

    initial begin
        $dumpfile("tb.vcd");
        $dumpvars(0, tb);
    end

    initial begin
        #300000 $finish;
    end

    // Generate approx. 14.31818MHz sysclk
    reg sysclk = 0;
    always #35 sysclk = !sysclk;

    // Generate 48MHz usbclk
    reg usbclk = 0;
    always #10.42 usbclk = !usbclk;

    wire [7:0] bus_d;
    wire [7:0] bus_de;

    wire [15:0] bus_a = 16'h0000;

    wire reset_n = 1'bZ;

    top top_inst(
        .sysclk(sysclk),
        .usbclk(usbclk),

        // Z80 bus interface
        .reset_n(reset_n),
        .phi(),
        .bus_a(bus_a),
        .bus_d(bus_d),
        .bus_rd_n(1'b0),
        .bus_wr_n(1'b0),
        .bus_mreq_n(1'b0),
        .bus_iorq_n(1'b0),
        .bus_int_n(),
        .bus_m1_n(1'bZ),
        .bus_wait_n(),
        .bus_busreq_n(),
        .bus_busack_n(1'bZ),
        .bus_ba(),
        .bus_de(),
        .bus_de_oe_n(),
        .ram_ce_n(),
        .rom_ce_n(),
        .cart_ce_n(),

        // PWM audio outputs
        .audio_l(),
        .audio_r(),

        // Other
        .cassette_out(),
        .cassette_in(1'b0),
        .printer_out(),
        .printer_in(1'b1),

        // USB
        .usb_dp1(),
        .usb_dm1(),
        .usb_dp2(),
        .usb_dm2(),

        // Misc
        .exp(),

        // Hand controller interface
        .hctrl_clk(),
        .hctrl_load_n(),
        .hctrl_data(1'b0),

        // VGA output
        .vga_r(),
        .vga_g(),
        .vga_b(),
        .vga_hsync(),
        .vga_vsync(),

        // ESP32 serial interface
        .esp_tx(),
        .esp_rx(1'b1),
        .esp_rts(),
        .esp_cts(1'b1),

        // ESP32 SPI interface (also used for loading FPGA image)
        .esp_cs_n(1'b1),
        .esp_sclk(1'b0),
        .esp_mosi(1'b0),
        .esp_miso(),
        .esp_notify());

endmodule
