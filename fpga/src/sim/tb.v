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

    // Generate reset signal
    reg reset_n = 0;
    always #350 reset_n = 1;

    wire [7:0] bus_d;
    wire [7:0] bus_de;

    top top_inst(
        .sysclk(sysclk),
        .reset_n(reset_n),

        .bus_a(16'h0),
        .bus_d(bus_d),
        .bus_rd_n(1'b1),
        .bus_wr_n(1'b1),
        .bus_mreq_n(1'b1),
        .bus_iorq_n(1'b1),
        .bus_int_n(),

        .bus_de(bus_de),
        .ram_cs_n(),
        .rom_cs_n(),
        .bank_a(),

        .audio_l(),
        .audio_r(),

        .cassette_out(),
        .cassette_in(1'b0),
        .printer_out(),
        .printer_in(1'b0),

        .vga_r(),
        .vga_g(),
        .vga_b(),
        .vga_hsync(),
        .vga_vsync(),

        .esp_tx(),
        .esp_rx(1'b1),

        .esp_cs_n(1'b1),
        .esp_sclk(1'b0),
        .esp_mosi(1'b0),
        .esp_miso());






endmodule
