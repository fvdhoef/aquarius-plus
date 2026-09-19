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


    wire [16:0] sram_m_addr;
    wire [31:0] sram_m_wrdata;
    wire        sram_m_wren;
    wire        sram_m_strobe;
    wire        sram_m_wait;
    wire [31:0] sram_m_rddata;

    sram_cache sram_cache(
        .clk(clk),
        .reset(reset),

        // Slave bus interface (from CPU)
        .s_addr(bus_addr),
        .s_wrdata(bus_wrdata),
        .s_bytesel(bus_bytesel),
        .s_wren(bus_wren),
        .s_strobe(bus_strobe),
        .s_wait(bus_wait),
        .s_rddata(bus_rddata),

        // Memory command interface
        .m_addr(sram_m_addr),
        .m_wrdata(sram_m_wrdata),
        .m_wren(sram_m_wren),
        .m_strobe(sram_m_strobe),
        .m_wait(sram_m_wait),
        .m_rddata(sram_m_rddata));

    wire [18:0] sram_a;
    wire        sram_ce_n;
    wire        sram_oe_n;
    wire        sram_we_n;
    wire  [7:0] sram_dq;

    sram_ctrl sram_ctrl(
        .clk(clk),
        .reset(reset),

        // Command interface
        .bus_addr(sram_m_addr),
        .bus_wrdata(sram_m_wrdata),
        .bus_bytesel(4'b1111),
        .bus_wren(sram_m_wren),
        .bus_strobe(sram_m_strobe),
        .bus_wait(sram_m_wait),
        .bus_rddata(sram_m_rddata),

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

    task memwr;
        input [16:0] addr;
        input [31:0] data;

        begin
            bus_addr    = addr;
            bus_wrdata  = data;
            bus_bytesel = 4'b1111;
            bus_wren    = 1;
            bus_strobe  = 1;

            @(posedge clk);
            while (bus_wait) @(posedge clk);

            bus_strobe  = 0;
        end
    endtask

    task memrd;
        input [16:0] addr;

        begin
            bus_addr    = addr;
            bus_bytesel = 4'b1111;
            bus_wren    = 0;
            bus_strobe  = 1;

            @(posedge clk);
            while (bus_wait) @(posedge clk);

            bus_strobe  = 0;
        end
    endtask


    initial begin
        bus_addr    = 0;
        bus_wrdata  = 0;
        bus_bytesel = 0;
        bus_wren    = 0;
        bus_strobe  = 0;

        @(negedge(reset));
        @(posedge(clk));


        memrd(17'h00000);
        memrd(17'h00001);
        memrd(17'h00002);
        memrd(17'h00003);

        @(posedge(clk));
        @(posedge(clk));
        @(posedge(clk));
        @(posedge(clk));

        memrd(17'h00000);
        memrd(17'h00001);
        memrd(17'h00002);
        memrd(17'h00003);

        memwr(17'h10000, 32'hDEADBEEF);
        memwr(17'h10000, 32'hCAFEBABE);

        memrd(17'h00001);
        memrd(17'h10000);
        memrd(17'h00000);
        memrd(17'h10000);

        // memwr(17'h00000, 32'h12345678);



        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;

        // // @(posedge clk);
        // // @(posedge clk);
        // // @(posedge clk);

        // bus_wrdata  = 32'h55AABEEF;
        // bus_bytesel = 4'b1111;
        // bus_wren    = 1;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;


        // bus_wrdata  = 32'h12345678;
        // bus_bytesel = 4'b1000;
        // bus_wren    = 1;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;

        // bus_wrdata  = 32'h12345678;
        // bus_bytesel = 4'b0100;
        // bus_wren    = 1;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;

        // bus_wrdata  = 32'h12345678;
        // bus_bytesel = 4'b0010;
        // bus_wren    = 1;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;

        // bus_wrdata  = 32'h12345678;
        // bus_bytesel = 4'b0001;
        // bus_wren    = 1;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;




        // bus_wren    = 0;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;

        // bus_addr    = 17'h10000;
        // bus_wrdata  = 32'h55AA1234;
        // bus_bytesel = 4'b1111;
        // bus_wren    = 1;

        // bus_strobe  = 1;
        // @(posedge clk);
        // while (bus_wait) @(posedge clk);
        // bus_strobe  = 0;

    end

endmodule
