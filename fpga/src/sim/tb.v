`timescale 1 ns / 1 ps

module tb();

    initial begin
        $dumpfile("tb.vcd");
        $dumpvars(0, tb);
    end

    initial begin
        #20000000 $finish;
    end

    // Generate approx. 14.31818MHz sysclk
    reg sysclk = 0;
    always #35 sysclk = !sysclk;

    // Generate 48MHz usbclk
    reg usbclk = 0;
    always #10.42 usbclk = !usbclk;

    reg  [7:0] bus_wrdata = 8'b0;
    reg        bus_wren = 1'b0;

    wire [7:0] bus_d = bus_wren ? bus_wrdata : 8'bZ;


    reg        esp_rx = 1'b1;

    wire reset_n = 1'bZ;
    pullup(reset_n);

    wire phi;

    reg spi_ssel_n_r = 1'b1;
    reg spi_sclk_r = 1'b0;
    reg spi_mosi_r = 1'b0;

    wire busreq_n;
    reg busack_n = 1'bZ;
    pullup(busreq_n);

    reg  [15:0] bus_a_r      = 16'h0000;
    reg         bus_rd_n_r   = 1'b1;
    reg         bus_wr_n_r   = 1'b1;
    reg         bus_mreq_n_r = 1'b1;
    reg         bus_iorq_n_r = 1'b1;

    wire [15:0] bus_a      = (busack_n == 1'b0) ? 16'bZ : bus_a_r;
    wire        bus_rd_n   = (busack_n == 1'b0) ? 1'bZ  : bus_rd_n_r;
    wire        bus_wr_n   = (busack_n == 1'b0) ? 1'bZ  : bus_wr_n_r;
    wire        bus_mreq_n = (busack_n == 1'b0) ? 1'bZ  : bus_mreq_n_r;
    wire        bus_iorq_n = (busack_n == 1'b0) ? 1'bZ  : bus_iorq_n_r;

    wire hc_clk, hc_load_n, hc_data;
    wire hc1_q7;

    always @(posedge phi) busack_n <= busreq_n;

    hc166 hc166_1(
        .pe_n(hc_load_n),
        .ds(1'b0),
        .d(8'h01),
        .mr_n(1'b1),
        .cp(hc_clk),
        .ce_n(1'b0),
        .q7(hc1_q7));

    hc166 hc166_2(
        .pe_n(hc_load_n),
        .ds(hc1_q7),
        .d(8'h80),
        .mr_n(1'b1),
        .cp(hc_clk),
        .ce_n(1'b0),
        .q7(hc_data));

    top top_inst(
        .sysclk(sysclk),
        .usbclk(usbclk),

        // Z80 bus interface
        .ebus_reset_n(reset_n),
        .ebus_phi(phi),
        .ebus_a(bus_a),
        .ebus_d(bus_d),
        .ebus_rd_n(bus_rd_n),
        .ebus_wr_n(bus_wr_n),
        .ebus_mreq_n(bus_mreq_n),
        .ebus_iorq_n(bus_iorq_n),
        .ebus_int_n(),
        .ebus_m1_n(1'bZ),
        .ebus_wait_n(),
        .ebus_busreq_n(busreq_n),
        .ebus_busack_n(busack_n),
        .ebus_ba(),
        .ebus_cart_d(),
        .ebus_cart_d_oe_n(),
        .ebus_ram_ce_n(),
        .ebus_rom_ce_n(),
        .ebus_cart_ce_n(),

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
        .hctrl_clk(hc_clk),
        .hctrl_load_n(hc_load_n),
        .hctrl_data(hc_data),

        // VGA output
        .vga_r(),
        .vga_g(),
        .vga_b(),
        .vga_hsync(),
        .vga_vsync(),

        // ESP32 serial interface
        .esp_tx(),
        .esp_rx(esp_rx),
        .esp_rts(),
        .esp_cts(1'b1),

        // ESP32 SPI interface (also used for loading FPGA image)
        .esp_ssel_n(spi_ssel_n_r),
        .esp_sclk(spi_sclk_r),
        .esp_mosi(spi_mosi_r),
        .esp_miso(),
        .esp_notify());

    task iowr;
        input [15:0] addr;
        input  [7:0] data;

        begin
            bus_wrdata  = data;

            // T1
            @(posedge phi)
            #110
            bus_a_r       = addr;

            // T2
            @(posedge phi)
            bus_wren    = 1'b1;
            #65
            bus_wr_n_r    = 1'b0;
            #10
            bus_iorq_n_r  = 1'b0;

            // T3
            @(posedge phi)
            @(negedge phi);
            #80
            bus_wr_n_r    = 1'b1;
            #5
            bus_iorq_n_r  = 1'b1;
            #55
            bus_wren    = 1'b0;
        end
    endtask

    task iord;
        input [15:0] addr;

        begin
            // T1
            @(posedge phi)
            #110
            bus_a_r       = addr;

            // T2
            @(posedge phi)
            #65
            bus_rd_n_r    = 1'b0;
            #10
            bus_iorq_n_r  = 1'b0;

            // T3
            @(posedge phi)
            @(negedge phi);
            #80
            bus_rd_n_r    = 1'b1;
            #5
            bus_iorq_n_r  = 1'b1;
            #55;
        end
    endtask

    task memwr;
        input [15:0] addr;
        input  [7:0] data;

        begin
            bus_wrdata  = data;

            // T1
            @(posedge phi)
            #110
            bus_a_r     = addr;

            // T2
            @(posedge phi)
            bus_wren    = 1'b1;
            #65
            bus_wr_n_r    = 1'b0;
            #10
            bus_mreq_n_r  = 1'b0;

            // T3
            @(posedge phi)
            @(negedge phi);
            #80
            bus_wr_n_r    = 1'b1;
            #5
            bus_mreq_n_r  = 1'b1;
            #55
            bus_wren    = 1'b0;
        end
    endtask

    task memrd;
        input [15:0] addr;

        begin
            // T1
            @(posedge phi)
            #110
            bus_a_r       = addr;

            // T2
            @(posedge phi)
            #65
            bus_rd_n_r    = 1'b0;
            #10
            bus_mreq_n_r  = 1'b0;

            // T3
            @(posedge phi)
            @(negedge phi);
            #80
            bus_rd_n_r    = 1'b1;
            #5
            bus_mreq_n_r  = 1'b1;
            #55;
        end
    endtask

    task esptx;
        input [7:0] data;

        begin
            #560 esp_rx = 1'b0;
            for (integer i=0; i<8; i++)
                #560 esp_rx = data[i];
            #560 esp_rx = 1'b1;
        end
    endtask

    task esptx_fe;
        input [7:0] data;

        begin
            #560 esp_rx = 1'b0;
            for (integer i=0; i<8; i++)
                #560 esp_rx = data[i];
            #560 esp_rx = 1'b0;
            #560 esp_rx = 1'b1;
        end
    endtask

    localparam TSPI = 500;

    task spi_tx;
        input [7:0] data;

        begin
            for (integer i=0; i<8; i++) begin
                #TSPI;
                spi_sclk_r = 1'b0;
                spi_mosi_r = data[7-i];
                
                #TSPI;
                spi_sclk_r = 1'b1;
            end
            #TSPI spi_sclk_r = 1'b0;
        end
    endtask

    task ay_write;
        input [3:0] addr;
        input [7:0] data;

        begin
            iowr(16'hF7, {4'h0, addr});
            iowr(16'hF6, data);
        end
    endtask

    initial begin
        #2500;
        @(posedge phi);
        @(posedge phi);

        memwr(16'h3000, 8'h5A);
        memwr(16'h3400, 8'h5A);


        ay_write(4'h0, 8'd254);
        ay_write(4'h8, 8'hF);
        ay_write(4'h6, 8'h3E);

        ay_write(4'hB, 8'h04);
        ay_write(4'hC, 8'h00);
        ay_write(4'hD, 8'h0F);


        iowr(16'hEA, 8'h00); iowr(16'hEB, 8'h11);
        iowr(16'hEA, 8'h01); iowr(16'hEB, 8'h01);
        iowr(16'hEA, 8'h02); iowr(16'hEB, 8'h11);
        iowr(16'hEA, 8'h03); iowr(16'hEB, 8'h0F);


        ////////
        // Flash programming
        ////////

        // // fpga_bus_acquire
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h20);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // set_bank(0, 0);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h24);
        // spi_tx(8'hF0);
        // spi_tx(8'h00);
        // spi_tx(8'h00);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // set_bank(1, 1);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h24);
        // spi_tx(8'hF1);
        // spi_tx(8'h00);
        // spi_tx(8'h01);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // set_bank(2, addr >> 14);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h24);
        // spi_tx(8'hF2);
        // spi_tx(8'h00);
        // spi_tx(8'h00);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // fpga_mem_write(0x5555, 0xAA);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'h55);
        // spi_tx(8'h55);
        // spi_tx(8'hAA);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // fpga_mem_write(0x2AAA, 0x55);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'hAA);
        // spi_tx(8'h2A);
        // spi_tx(8'h55);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // fpga_mem_write(0x5555, 0xA0);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'h55);
        // spi_tx(8'h55);
        // spi_tx(8'hA0);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // fpga_mem_write(0x8000 + (addr & 0x3FFF), val);
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'h00);
        // spi_tx(8'h80);
        // spi_tx(8'h42);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);

        // // fpga_bus_release
        // @(posedge phi);
        // spi_ssel_n_r <= 1'b0;
        // spi_tx(8'h21);
        // spi_ssel_n_r <= 1'b1;
        // @(posedge phi);


        // iowr(16'd244, 8'd128);
        // iowr(16'd245, 8'd128);


/*
        spi_ssel_n_r <= 1'b0;
        spi_tx(8'h01);
        spi_ssel_n_r <= 1'b1;

        #1000;

        @(posedge phi);
        spi_ssel_n_r <= 1'b0;
        spi_tx(8'h20);
        spi_ssel_n_r <= 1'b1;
        @(posedge phi);

        @(posedge phi);
        spi_ssel_n_r <= 1'b0;
        spi_tx(8'h22);
        spi_tx(8'h00);
        spi_tx(8'h30);
        spi_tx(8'h5A);
        spi_ssel_n_r <= 1'b1;
        @(posedge phi);

        #10000;

        @(posedge phi);
        spi_ssel_n_r <= 1'b0;
        spi_tx(8'h23);
        spi_tx(8'h00);
        spi_tx(8'h30);
        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_ssel_n_r <= 1'b1;
        @(posedge phi);

        #10000;

        @(posedge phi);
        spi_ssel_n_r <= 1'b0;
        spi_tx(8'h21);
        spi_ssel_n_r <= 1'b1;
        @(posedge phi);

        #10000;

        @(posedge phi);
        spi_ssel_n_r <= 1'b0;
        spi_tx(8'h10);
        spi_tx(8'h01);
        spi_tx(8'h23);
        spi_tx(8'h45);
        spi_tx(8'h67);
        spi_tx(8'h89);
        spi_tx(8'hAB);
        spi_tx(8'hCD);
        spi_tx(8'hEF);
        spi_ssel_n_r <= 1'b1;

        iord(16'h01FF);
        iord(16'h02FF);
        iord(16'h04FF);
        iord(16'h08FF);
        iord(16'h10FF);
        iord(16'h20FF);
        iord(16'h40FF);
        iord(16'h80FF);

        // iowr(16'h00F5, 8'h42);
        // #4500;

        // iowr(16'h00F5, 8'h5A);

        // esptx(8'hF0);
        // esptx(8'hF1);
        // esptx(8'hF2);
        // esptx(8'hF3);

        // esptx_fe(8'hAA);


        // // Break
        // #560 esp_rx = 1'b0;
        // #30000 esp_rx = 1'b1;

        // esptx(8'hF4);
        // esptx(8'hF5);
        // esptx(8'hF6);
        // esptx(8'hF7);
        // esptx(8'hF8);
        // esptx(8'hF9);
        // esptx(8'hFA);
        // esptx(8'hFB);
        // esptx(8'hFC);
        // esptx(8'hFD);
        // esptx(8'hFE);
        // esptx(8'hFF);

        // iord(16'h00F5);
        // iord(16'h00F5);

        // iowr(16'h00F4, 8'h80);

        // iowr(16'h00F0, 8'hC0);
        // iowr(16'h00F1, 8'h20);
        // iowr(16'h00F2, 8'h21);
        // iowr(16'h00F3, 8'h13);

        memrd(16'h0000);
        memrd(16'h0001);
        memrd(16'h0002);
        memrd(16'h0003);
        memrd(16'h0004);
        memrd(16'h0010);

        memrd(16'h0000);
        memrd(16'h3000);
        memrd(16'h3800);
        memrd(16'h4000);
        memrd(16'h8000);
        memrd(16'hC000);


        memrd(16'h3000);
        memwr(16'h3000, 8'h5A);
        memrd(16'h3001);
        memrd(16'h3000);

        memrd(16'h0000);
*/


        // memrd(16'h0000);
        // memrd(16'h1000);
        // memrd(16'h2000);
        // memrd(16'h3000);
        // memrd(16'h4000);
        // memrd(16'h5000);
        // memrd(16'h6000);
        // memrd(16'h7000);
        // memrd(16'h8000);
        // memrd(16'h9000);
        // memrd(16'hA000);
        // memrd(16'hB000);
        // memrd(16'hC000);
        // memrd(16'hD000);
        // memrd(16'hE000);
        // memrd(16'hF000);

    end

endmodule
