`default_nettype none
`timescale 1ns / 1ps

module tb_sram_ctrl;

    reg clk   = 0;
    reg reset = 1;

    always #20 clk = !clk;
    always #200 reset = 0;

    wire [18:0] sram_a;
    wire        sram_ce_n;
    wire        sram_oe_n;
    wire        sram_we_n;
    wire  [7:0] sram_dq;

    reg  [18:0] mem_addr;
    reg   [7:0] mem_wrdata;
    reg         mem_wren;
    reg         mem_strobe;
    wire        mem_wait;
    wire  [7:0] mem_rddata;
    wire        mem_rddata_valid;

    sram_ctrl sram_ctrl(
        .clk(clk),
        .reset(reset),

        // Internal memory interface (pipelined)
        .mem_addr(mem_addr),
        .mem_wrdata(mem_wrdata),
        .mem_wren(mem_wren),
        .mem_strobe(mem_strobe),
        .mem_wait(mem_wait),
        .mem_rddata(mem_rddata),
        .mem_rddata_valid(mem_rddata_valid),

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
        input [18:0] addr;
        input  [7:0] data;

        begin
            mem_addr    = addr;
            mem_wrdata  = data;
            mem_wren    = 1;
            mem_strobe  = 1;

            @(posedge clk);
            while (mem_wait) @(posedge clk);

            mem_strobe  = 0;
        end
    endtask

    task memrd;
        input [18:0] addr;

        begin
            mem_addr    = addr;
            mem_wren    = 0;
            mem_strobe  = 1;

            @(posedge clk);
            while (mem_wait) @(posedge clk);

            mem_strobe  = 0;
        end
    endtask

    initial begin
        mem_addr    = 0;
        mem_wrdata  = 0;
        mem_wren    = 0;
        mem_strobe  = 0;

        @(negedge(reset));
        @(posedge(clk));

        memrd(19'h00010);
        memrd(19'h00020);
        memrd(19'h00021);
        memrd(19'h00030);
        memwr(19'h00011, 8'h55);
        memwr(19'h00012, 8'hAA);
        memrd(19'h00010);
        memrd(19'h00011);

    end

endmodule
