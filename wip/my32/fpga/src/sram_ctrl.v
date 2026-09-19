`default_nettype none
`timescale 1 ns / 1 ps

module sram_ctrl(
    input  wire        clk,
    input  wire        reset,

    // Internal memory interface (pipelined)
    input  wire [18:0] mem_addr,
    input  wire  [7:0] mem_wrdata,
    input  wire        mem_wren,
    input  wire        mem_strobe,
    output wire        mem_wait,
    output reg   [7:0] mem_rddata,
    output reg         mem_rddata_valid,

    // SRAM memory interface
    output reg  [18:0] sram_a,
    output wire        sram_ce_n,
    output reg         sram_oe_n,
    output reg         sram_we_n,
    inout  wire  [7:0] sram_dq
);

    reg [1:0] q_state  = 0;
    reg [7:0] q_wrdata = 0;
    reg       q_wren   = 0;
    reg       q_read   = 0;

    assign mem_wait  = ((mem_strobe && mem_wren) || (q_state != 2'd0)) && !(q_state == 2'd3);
    assign sram_ce_n = 0;
    assign sram_dq   = q_wren ? q_wrdata : 8'bZ;

    always @(posedge clk) begin
        q_wren           <= 0;
        q_read           <= 0;
        mem_rddata_valid <= 0;
        sram_we_n        <= 1;
        sram_oe_n        <= 0;

        if (q_state == 2'd0) begin
            if (mem_strobe) begin
                sram_a <= mem_addr;

                if (!mem_wren) begin
                    q_read    <= 1;
                end else begin
                    q_wrdata  <= mem_wrdata;
                    sram_oe_n <= 1;
                    q_state   <= 2'd1;
                end
            end

        end else if (q_state == 2'd1) begin
            sram_we_n <= 0;
            sram_oe_n <= 1;
            q_wren    <= 1;
            q_state   <= 2'd2;

        end else if (q_state == 2'd2) begin
            sram_we_n <= 1;
            sram_oe_n <= 1;
            q_wren    <= 1;
            q_state   <= 2'd3;

        end else begin
            q_wren    <= 0;
            q_state   <= 2'd0;
        end

        if (q_read) begin
            mem_rddata       <= sram_dq;
            mem_rddata_valid <= 1;
        end
    end

endmodule
