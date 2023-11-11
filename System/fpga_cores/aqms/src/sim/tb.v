`timescale 1 ns / 1 ps

module tb();

    initial begin
        $dumpfile("tb.vcd");
        $dumpvars(0, tb);
    end

    initial begin
        // #2000000 $finish;
        #18000000 $finish;
    end

    // Generate approx. 28.63636MHz sysclk
    reg sysclk = 0;
    always #17 sysclk = !sysclk;

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
    wire        bus_int_n;
    pullup(bus_int_n);

    wire [15:0] bus_a      = (busack_n == 1'b0) ? 16'bZ : bus_a_r;
    wire        bus_rd_n   = (busack_n == 1'b0) ? 1'bZ  : bus_rd_n_r;
    wire        bus_wr_n   = (busack_n == 1'b0) ? 1'bZ  : bus_wr_n_r;
    wire        bus_mreq_n = (busack_n == 1'b0) ? 1'bZ  : bus_mreq_n_r;
    wire        bus_iorq_n = (busack_n == 1'b0) ? 1'bZ  : bus_iorq_n_r;

    always @(posedge phi) busack_n <= busreq_n;

    top top_inst(
        .sysclk(sysclk),

        // Z80 bus interface
        .ebus_reset_n(reset_n),
        .ebus_phi(phi),
        .ebus_a(bus_a),
        .ebus_d(bus_d),
        .ebus_rd_n(bus_rd_n),
        .ebus_wr_n(bus_wr_n),
        .ebus_mreq_n(bus_mreq_n),
        .ebus_iorq_n(bus_iorq_n),
        .ebus_int_n(bus_int_n),
        .ebus_busreq_n(busreq_n),
        .ebus_busack_n(busack_n),
        .ebus_ba(),
        .ebus_ram_ce_n(),
        .ebus_cart_ce_n(),
        .ebus_ram_we_n(),

        // PWM audio outputs
        .audio_l(),
        .audio_r(),

        // Other
        .cassette_out(),
        .cassette_in(1'b0),
        .printer_out(),
        .printer_in(1'b1),

        // Misc
        .exp(),

        // Hand controller interface
        .hc1(),
        .hc2(),

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

    initial begin
        #2500;
        @(posedge phi);
        @(posedge phi);

        iord(16'hBF);

        // Write some palette entries
        iowr(16'hBF, 8'h00); // VDP ctrl port
        iowr(16'hBF, 8'hC0);
        iowr(16'hBE, 8'h01);
        iowr(16'hBE, 8'h02);
        iowr(16'hBE, 8'h03);
        iowr(16'hBE, 8'h04);
        iowr(16'hBE, 8'h05);
        iowr(16'hBE, 8'h06);
        iowr(16'hBE, 8'h07);

        // Enable vsync interrupt
        iowr(16'hBF, 8'h20); // VDP ctrl port
        iowr(16'hBF, 8'h81);

        iowr(16'hBF, 8'h00);
        iowr(16'hBF, 8'h06);
        iowr(16'hBE, 8'hA5);
        iowr(16'hBE, 8'hA6);
        iowr(16'hBE, 8'hA6);
        iowr(16'hBE, 8'hA7);
        iowr(16'hBE, 8'hA8);

        iowr(16'hBF, 8'h00);
        iowr(16'hBF, 8'h06);
        iord(16'hBE);
        iord(16'hBE);
        iord(16'hBE);
        iord(16'hBE);
        iord(16'hBE);

        memwr(16'hC100, 8'h42);
        memwr(16'hC110, 8'hDE);
        memwr(16'hC111, 8'hAD);

        memrd(16'hE100);

        memrd(16'h0100);
        memrd(16'h7FF0);


        memwr(16'hFFFC, 8'h00);


        wait (!bus_int_n);
        @(posedge phi);
        iord(16'hBF);




        // iowr(16'hBE, 8'h00); // VDP data port



    end

endmodule
