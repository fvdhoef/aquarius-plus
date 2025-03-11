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

    aqp_top top_inst(
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
    // is61lv5128al sram(
        .A({ebus_ba, ebus_a[13:0]}),
        .IO(ebus_d),
        .CE_n(ebus_ram_ce_n),
        .OE_n(ebus_rd_n),
        .WE_n(ebus_ram_we_n)
    );

    //////////////////////////////////////////////////////////////////////////
    // Testbench
    //////////////////////////////////////////////////////////////////////////
    task iowr;
        input [15:0] addr;
        input  [7:0] data;

        begin
            bus_wrdata  = data;

            // T1
            @(posedge ebus_phi)
            #110
            q_ebus_a       = addr;

            // T2
            @(posedge ebus_phi)
            bus_wren    = 1'b1;
            #65
            q_ebus_wr_n    = 1'b0;
            #10
            q_ebus_iorq_n  = 1'b0;

            // T3
            @(posedge ebus_phi)
            @(negedge ebus_phi);
            #80
            q_ebus_wr_n    = 1'b1;
            #5
            q_ebus_iorq_n  = 1'b1;
            #55
            bus_wren    = 1'b0;
        end
    endtask

    task iord;
        input [15:0] addr;

        begin
            // T1
            @(posedge ebus_phi)
            #110
            q_ebus_a       = addr;

            // T2
            @(posedge ebus_phi)
            #65
            q_ebus_rd_n    = 1'b0;
            #10
            q_ebus_iorq_n  = 1'b0;

            // T3
            @(posedge ebus_phi)
            @(negedge ebus_phi);
            #80
            q_ebus_rd_n    = 1'b1;
            #5
            q_ebus_iorq_n  = 1'b1;
            #55;
        end
    endtask

    task memwr;
        input [15:0] addr;
        input  [7:0] data;

        begin
            bus_wrdata  = data;

            // T1
            @(posedge ebus_phi)
            #110
            q_ebus_a     = addr;

            // T2
            @(posedge ebus_phi)
            bus_wren    = 1'b1;
            #65
            q_ebus_wr_n    = 1'b0;
            #10
            q_ebus_mreq_n  = 1'b0;

            // T3
            @(posedge ebus_phi)
            @(negedge ebus_phi);
            #80
            q_ebus_wr_n    = 1'b1;
            #5
            q_ebus_mreq_n  = 1'b1;
            #55
            bus_wren    = 1'b0;
        end
    endtask

    task memrd;
        input [15:0] addr;

        begin
            // T1
            @(posedge ebus_phi)
            #110
            q_ebus_a       = addr;

            // T2
            @(posedge ebus_phi)
            #65
            q_ebus_rd_n    = 1'b0;
            #10
            q_ebus_mreq_n  = 1'b0;

            // T3
            @(posedge ebus_phi)
            @(negedge ebus_phi);
            #80
            q_ebus_rd_n    = 1'b1;
            #5
            q_ebus_mreq_n  = 1'b1;
            #55;
        end
    endtask

    task esptx;
        input [7:0] data;

        integer i;

        begin
            #560 esp_rx = 1'b0;
            for (i=0; i<8; i=i+1)
                #560 esp_rx = data[i];
            #560 esp_rx = 1'b1;
        end
    endtask

    task esptx_fe;
        input [7:0] data;

        integer i;

        begin
            #560 esp_rx = 1'b0;
            for (i=0; i<8; i=i+1)
                #560 esp_rx = data[i];
            #560 esp_rx = 1'b0;
            #560 esp_rx = 1'b1;
        end
    endtask

    localparam TSPI = 500;

    task spi_tx;
        input [7:0] data;

        integer i;

        begin
            for (i=0; i<8; i=i+1) begin
                #TSPI;
                esp_sclk = 1'b0;
                esp_mosi = data[7-i];
                
                #TSPI;
                esp_sclk = 1'b1;
            end
            #TSPI esp_sclk = 1'b0;
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
        #2500
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h40);
        // spi_tx(8'h01);
        // esp_ssel_n <= 1'b1;

        esp_ssel_n <= 1'b0;
        spi_tx(8'hF8);
        spi_tx(8'h00);

        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_tx(8'h00);
        spi_tx(8'h00);
        esp_ssel_n <= 1'b1;

        @(posedge ebus_phi);

    end

    initial begin
        // #2500;
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);

        // iowr(16'hF3, 8'h33);
        // memwr(16'hFFFF, 8'h01);
        // memwr(16'hFFFE, 8'h02);

        // memwr(16'hC000, 8'h42);
        // memwr(16'hD000, 8'h42);
        // memwr(16'hE000, 8'h42);
        // memwr(16'hF000, 8'h42);
        // memwr(16'hF800, 8'h42);

        // memrd(16'hC000);
        // memrd(16'hD000);
        // memrd(16'hE000);
        // memrd(16'hF000);
        // memrd(16'hF800);

        // memrd(16'hFFFF);
        // memrd(16'hFFFE);


        // iowr(16'hF4, 8'h80);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // iowr(16'hF5, 8'h01);


        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h12);
        // spi_tx(8'hA5);
        // esp_ssel_n <= 1'b1;
     
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);
        // @(posedge ebus_phi);

        // iord(16'h00FA);

        // iord(16'h00FA);

        // // iowr(16'hEC, 8'd255);
        // iowr(16'hEC, 8'd128);
        // iowr(16'hEC, 8'd0);

        // iowr(16'hE0, 8'd06);

        // iowr(16'hED, 8'd02);
        // iowr(16'hEF, 8'd03);


        // iowr(16'hF0, 8'd20);

        // memwr(16'h0F00, 8'h00);
        // memwr(16'h0F01, 8'h01);

        // memwr(16'h0F02, 8'h01);
        // memwr(16'h0F03, 8'h31);

        // memwr(16'h0F04, 8'h02);
        // memwr(16'h0F05, 8'h01);

        // memwr(16'h0F06, 8'h03);
        // memwr(16'h0F07, 8'h01);

        // for (integer i=0; i<256; i=i+1) begin
        //     memwr(16'h2000 + i, i);
        // end

        // memwr(16'h2000, 8'h00);
        // memwr(16'h2001, 8'h01);
        // memwr(16'h2002, 8'h02);
        // memwr(16'h2003, 8'h03);
        // memwr(16'h2004, 8'h04);
        // memwr(16'h2005, 8'h05);
        // memwr(16'h2006, 8'h06);
        // memwr(16'h2007, 8'h07);
        // memwr(16'h2008, 8'h08);
        // memwr(16'h2009, 8'h09);
        // memwr(16'h200a, 8'h0a);
        // memwr(16'h200b, 8'h0b);
        // memwr(16'h200c, 8'h0c);
        // memwr(16'h200d, 8'h0d);
        // memwr(16'h200e, 8'h0e);
        // memwr(16'h200f, 8'h0f);
        // memwr(16'h2010, 8'h10);
        // memwr(16'h2011, 8'h11);
        // memwr(16'h2012, 8'h12);
        // memwr(16'h2013, 8'h13);
        // memwr(16'h2014, 8'h14);
        // memwr(16'h2015, 8'h15);
        // memwr(16'h2016, 8'h16);
        // memwr(16'h2017, 8'h17);
        // memwr(16'h2018, 8'h18);
        // memwr(16'h2019, 8'h19);
        // memwr(16'h201a, 8'h1a);
        // memwr(16'h201b, 8'h1b);
        // memwr(16'h201c, 8'h1c);
        // memwr(16'h201d, 8'h1d);
        // memwr(16'h201e, 8'h1e);
        // memwr(16'h201f, 8'h1f);

        // memwr(16'h3000, 8'h5A);
        // memwr(16'h3400, 8'h5A);


        // ay_write(4'h0, 8'd254);
        // ay_write(4'h8, 8'hF);
        // ay_write(4'h6, 8'h3E);

        // ay_write(4'hB, 8'h04);
        // ay_write(4'hC, 8'h00);
        // ay_write(4'hD, 8'h0F);


        // iowr(16'hEA, 8'h00); iowr(16'hEB, 8'h11);
        // iowr(16'hEA, 8'h01); iowr(16'hEB, 8'h01);
        // iowr(16'hEA, 8'h02); iowr(16'hEB, 8'h11);
        // iowr(16'hEA, 8'h03); iowr(16'hEB, 8'h0F);


        ////////
        // Flash programming
        ////////

        // // fpga_bus_acquire
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h20);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // set_bank(0, 0);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h24);
        // spi_tx(8'hF0);
        // spi_tx(8'h00);
        // spi_tx(8'h00);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // set_bank(1, 1);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h24);
        // spi_tx(8'hF1);
        // spi_tx(8'h00);
        // spi_tx(8'h01);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // set_bank(2, addr >> 14);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h24);
        // spi_tx(8'hF2);
        // spi_tx(8'h00);
        // spi_tx(8'h00);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // fpga_mem_write(0x5555, 0xAA);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'h55);
        // spi_tx(8'h55);
        // spi_tx(8'hAA);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // fpga_mem_write(0x2AAA, 0x55);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'hAA);
        // spi_tx(8'h2A);
        // spi_tx(8'h55);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // fpga_mem_write(0x5555, 0xA0);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'h55);
        // spi_tx(8'h55);
        // spi_tx(8'hA0);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // fpga_mem_write(0x8000 + (addr & 0x3FFF), val);
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h22);
        // spi_tx(8'h00);
        // spi_tx(8'h80);
        // spi_tx(8'h42);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);

        // // fpga_bus_release
        // @(posedge ebus_phi);
        // esp_ssel_n <= 1'b0;
        // spi_tx(8'h21);
        // esp_ssel_n <= 1'b1;
        // @(posedge ebus_phi);


        // iowr(16'd244, 8'd128);
        // iowr(16'd245, 8'd128);


/*
        esp_ssel_n <= 1'b0;
        spi_tx(8'h01);
        esp_ssel_n <= 1'b1;

        #1000;

        @(posedge ebus_phi);
        esp_ssel_n <= 1'b0;
        spi_tx(8'h20);
        esp_ssel_n <= 1'b1;
        @(posedge ebus_phi);

        @(posedge ebus_phi);
        esp_ssel_n <= 1'b0;
        spi_tx(8'h22);
        spi_tx(8'h00);
        spi_tx(8'h30);
        spi_tx(8'h5A);
        esp_ssel_n <= 1'b1;
        @(posedge ebus_phi);

        #10000;

        @(posedge ebus_phi);
        esp_ssel_n <= 1'b0;
        spi_tx(8'h23);
        spi_tx(8'h00);
        spi_tx(8'h30);
        spi_tx(8'h00);
        spi_tx(8'h00);
        esp_ssel_n <= 1'b1;
        @(posedge ebus_phi);

        #10000;

        @(posedge ebus_phi);
        esp_ssel_n <= 1'b0;
        spi_tx(8'h21);
        esp_ssel_n <= 1'b1;
        @(posedge ebus_phi);

        #10000;

        @(posedge ebus_phi);
        esp_ssel_n <= 1'b0;
        spi_tx(8'h10);
        spi_tx(8'h01);
        spi_tx(8'h23);
        spi_tx(8'h45);
        spi_tx(8'h67);
        spi_tx(8'h89);
        spi_tx(8'hAB);
        spi_tx(8'hCD);
        spi_tx(8'hEF);
        esp_ssel_n <= 1'b1;

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
