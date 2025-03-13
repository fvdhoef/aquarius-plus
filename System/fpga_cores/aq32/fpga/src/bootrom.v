`default_nettype none
`timescale 1 ns / 1 ps

module bootrom(
    input  wire        clk,
    input  wire  [8:0] addr,
    output reg  [31:0] rddata
);

    always @(posedge clk) case (addr)
        9'h000:  rddata <= 32'h00001197;
        9'h001:  rddata <= 32'h00018193;
        9'h002:  rddata <= 32'h00080117;
        9'h003:  rddata <= 32'h7F810113;
        9'h004:  rddata <= 32'h00000293;
        9'h005:  rddata <= 32'h00000313;
        9'h006:  rddata <= 32'h00C0006F;
        9'h007:  rddata <= 32'h0002A023;
        9'h008:  rddata <= 32'h00428293;
        9'h009:  rddata <= 32'hFE62ECE3;
        9'h00A:  rddata <= 32'h00000293;
        9'h00B:  rddata <= 32'h00000313;
        9'h00C:  rddata <= 32'h00000397;
        9'h00D:  rddata <= 32'h04838393;
        9'h00E:  rddata <= 32'h0140006F;
        9'h00F:  rddata <= 32'h0003AE03;
        9'h010:  rddata <= 32'h00438393;
        9'h011:  rddata <= 32'h01C2A023;
        9'h012:  rddata <= 32'h00428293;
        9'h013:  rddata <= 32'hFE62E8E3;
        9'h014:  rddata <= 32'h00000297;
        9'h015:  rddata <= 32'h00C28293;
        9'h016:  rddata <= 32'h00028067;
        9'h017:  rddata <= 32'hFF0007B7;
        9'h018:  rddata <= 32'h34100713;
        9'h019:  rddata <= 32'h00E79023;
        9'h01A:  rddata <= 32'hFF3007B7;
        9'h01B:  rddata <= 32'h00100713;
        9'h01C:  rddata <= 32'h00E78023;
        9'h01D:  rddata <= 32'h0000006F;
        default: rddata <= 32'h00000000;
    endcase

endmodule
