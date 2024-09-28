// `default_nettype none
module kbbuf(
    input  wire       clk,
    input  wire       rst,

    input  wire [7:0] wrdata,
    input  wire       wr_en,

    output reg  [7:0] rddata,
    input  wire       rd_en);

    reg [3:0] wridx = 0, rdidx = 0;
    reg [7:0] mem_r [15:0];

    wire [3:0] wridx_next = wridx + 4'd1;
    wire [3:0] rdidx_next = rdidx + 4'd1;
    // wire [3:0] count      = wridx - rdidx;

    wire empty = wridx      == rdidx;
    wire full  = wridx_next == rdidx;
    // wire almost_full = count >= 4'd8;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            wridx  <= 4'd0;
            rdidx  <= 4'd0;
            rddata <= 8'h0;

        end else begin
            if (wr_en && !full) begin
                mem_r[wridx] <= wrdata;
                wridx <= wridx_next;
            end

            if (rd_en) begin
                rddata <= empty ? 8'h00 : mem_r[rdidx];

                if (!empty)
                    rdidx <= rdidx_next;
            end
        end
    end

endmodule
