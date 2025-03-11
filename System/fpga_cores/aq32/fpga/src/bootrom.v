`default_nettype none
`timescale 1 ns / 1 ps

module bootrom(
    input  wire        clk,
    input  wire  [8:0] addr,
    output reg  [31:0] rddata
);

    always @(posedge clk) case (addr)
        9'h000:  rddata <= 32'h00001197;
        9'h004:  rddata <= 32'h00018193;
        9'h008:  rddata <= 32'h00080117;
        9'h00C:  rddata <= 32'h7F810113;
        9'h010:  rddata <= 32'h00000293;
        9'h014:  rddata <= 32'h00000313;
        9'h018:  rddata <= 32'h00C0006F;
        9'h01C:  rddata <= 32'h0002A023;
        9'h020:  rddata <= 32'h00428293;
        9'h024:  rddata <= 32'hFE62ECE3;
        9'h028:  rddata <= 32'h00000293;
        9'h02C:  rddata <= 32'h00000313;
        9'h030:  rddata <= 32'h00000397;
        9'h034:  rddata <= 32'h03038393;
        9'h038:  rddata <= 32'h0140006F;
        9'h03C:  rddata <= 32'h0003AE03;
        9'h040:  rddata <= 32'h00438393;
        9'h044:  rddata <= 32'h01C2A023;
        9'h048:  rddata <= 32'h00428293;
        9'h04C:  rddata <= 32'hFE62E8E3;
        9'h050:  rddata <= 32'h00000297;
        9'h054:  rddata <= 32'h00C28293;
        9'h058:  rddata <= 32'h00028067;
        9'h05C:  rddata <= 32'h0000006F;
        default: rddata <= 32'h00000000;
    endcase

endmodule
