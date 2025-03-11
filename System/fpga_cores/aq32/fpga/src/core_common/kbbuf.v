`default_nettype none
`timescale 1 ns / 1 ps

module kbbuf(
    input  wire       clk,
    input  wire       rst,

    input  wire [7:0] wrdata,
    input  wire       wr_en,

    output reg  [7:0] rddata,
    input  wire       rd_en);

    reg [3:0] q_wridx = 0, q_rdidx = 0;
    reg [7:0] mem [15:0];

    wire [3:0] d_wridx = q_wridx + 4'd1;
    wire [3:0] d_rdidx = q_rdidx + 4'd1;

    wire empty = q_wridx == q_rdidx;
    wire full  = d_wridx == q_rdidx;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            q_wridx <= 4'd0;
            q_rdidx <= 4'd0;
            rddata  <= 8'h0;

        end else begin
            if (wr_en && !full) begin
                mem[q_wridx] <= wrdata;
                q_wridx      <= d_wridx;
            end

            if (rd_en) begin
                rddata <= empty ? 8'h00 : mem[q_rdidx];

                if (!empty)
                    q_rdidx <= d_rdidx;
            end
        end
    end

endmodule
