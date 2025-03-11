`default_nettype none
`timescale 1ns / 1ps

module tb_sram_ctrl;

    reg clk   = 0;
    reg reset = 1;

    always #20 clk = !clk;
    always #200 reset = 0;

    reg  [16:0] bus_addr;
    reg  [31:0] bus_wrdata;
    reg   [3:0] bus_bytesel;
    reg         bus_wren;
    reg         bus_strobe;
    wire        bus_wait;
    wire [31:0] bus_rddata;

    wire [18:0] sram_a;
    wire        sram_ce_n;
    wire        sram_oe_n;
    wire        sram_we_n;
    wire  [7:0] sram_dq;

    sram_ctrl sram_ctrl(
        .clk(clk),
        .reset(reset),

        // Command interface
        .bus_addr(bus_addr),
        .bus_wrdata(bus_wrdata),
        .bus_bytesel(bus_bytesel),
        .bus_wren(bus_wren),
        .bus_strobe(bus_strobe),
        .bus_wait(bus_wait),
        .bus_rddata(bus_rddata),

        // SRAM interface
        .sram_a(sram_a),
        .sram_ce_n(sram_ce_n),
        .sram_oe_n(sram_oe_n),
        .sram_we_n(sram_we_n),
        .sram_dq(sram_dq));

    is61c5128as ram(
        .A(sram_a),
        .IO(sram_dq),
        .CE_n(sram_ce_n),
        .OE_n(sram_oe_n),
        .WE_n(sram_we_n));

    initial begin
        bus_addr    = 0;
        bus_wrdata  = 0;
        bus_bytesel = 0;
        bus_wren    = 0;
        bus_strobe  = 0;

        @(negedge(reset));
        @(posedge(clk));

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;

        // @(posedge clk);
        // @(posedge clk);
        // @(posedge clk);

        bus_wrdata  = 32'h55AABEEF;
        bus_bytesel = 4'b1111;
        bus_wren    = 1;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;


        bus_wrdata  = 32'h12345678;
        bus_bytesel = 4'b1000;
        bus_wren    = 1;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;

        bus_wrdata  = 32'h12345678;
        bus_bytesel = 4'b0100;
        bus_wren    = 1;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;

        bus_wrdata  = 32'h12345678;
        bus_bytesel = 4'b0010;
        bus_wren    = 1;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;

        bus_wrdata  = 32'h12345678;
        bus_bytesel = 4'b0001;
        bus_wren    = 1;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;




        bus_wren    = 0;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;

        bus_addr = 1;

        bus_strobe  = 1;
        @(posedge clk);
        while (bus_wait) @(posedge clk);
        bus_strobe  = 0;

    end

endmodule
