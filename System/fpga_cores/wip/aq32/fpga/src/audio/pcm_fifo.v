`default_nettype none
`timescale 1 ns / 1 ps

module pcm_fifo(
    input  wire        clk,
    input  wire        reset,

    input  wire [31:0] wrdata,
    input  wire        wr_en,

    output reg  [31:0] rddata,
    input  wire        rd_en,
    
    output wire        empty,
    output wire        full,
    output wire  [9:0] count);

    reg  [9:0] q_wridx = 0, q_rdidx = 0;
    reg [31:0] mem [1023:0];

    wire [9:0] d_wridx = q_wridx + 10'd1;
    wire [9:0] d_rdidx = q_rdidx + 10'd1;
    assign     count   = q_wridx - q_rdidx;

    assign empty       = q_wridx == q_rdidx;
    assign full        = d_wridx == q_rdidx;

    always @(posedge clk) begin
        if (wr_en && !full)  mem[q_wridx] <= wrdata;
        if (rd_en && !empty) rddata <= mem[q_rdidx];
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_wridx <= 0;
            q_rdidx <= 0;
        end else begin
            if (wr_en && !full)  q_wridx <= d_wridx;
            if (rd_en && !empty) q_rdidx <= d_rdidx;
        end
    end

endmodule
