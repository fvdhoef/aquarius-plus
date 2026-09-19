`default_nettype none
`timescale 1 ns / 1 ps

module bootrom(
    input  wire        clk,
    input  wire  [8:0] addr,
    output reg  [31:0] rddata
);

    always @(posedge clk) case (addr)
        9'h000:  rddata <= 32'h0080006F;
        9'h001:  rddata <= 32'h0200006F;
        9'h002:  rddata <= 32'h30001073;
        9'h003:  rddata <= 32'h30401073;
        9'h004:  rddata <= 32'h000022B7;
        9'h005:  rddata <= 32'h00100513;
        9'h006:  rddata <= 32'h0F8000E7;
        9'h007:  rddata <= 32'h00000517;
        9'h008:  rddata <= 32'h12C50513;
        9'h009:  rddata <= 32'h30001073;
        9'h00A:  rddata <= 32'h30401073;
        9'h00B:  rddata <= 32'h00050F93;
        9'h00C:  rddata <= 32'h000022B7;
        9'h00D:  rddata <= 32'h01F00513;
        9'h00E:  rddata <= 32'h0F8000E7;
        9'h00F:  rddata <= 32'h134000E7;
        9'h010:  rddata <= 32'h01000513;
        9'h011:  rddata <= 32'h0F8000E7;
        9'h012:  rddata <= 32'h00000513;
        9'h013:  rddata <= 32'h120000E7;
        9'h014:  rddata <= 32'h000F8393;
        9'h015:  rddata <= 32'h0003C503;
        9'h016:  rddata <= 32'h00138393;
        9'h017:  rddata <= 32'h120000E7;
        9'h018:  rddata <= 32'hFE051AE3;
        9'h019:  rddata <= 32'h134000E7;
        9'h01A:  rddata <= 32'h01851513;
        9'h01B:  rddata <= 32'h41855513;
        9'h01C:  rddata <= 32'h08054263;
        9'h01D:  rddata <= 32'h00050F13;
        9'h01E:  rddata <= 32'h000803B7;
        9'h01F:  rddata <= 32'h40038393;
        9'h020:  rddata <= 32'h01200513;
        9'h021:  rddata <= 32'h0F8000E7;
        9'h022:  rddata <= 32'h000F0513;
        9'h023:  rddata <= 32'h120000E7;
        9'h024:  rddata <= 32'h00000513;
        9'h025:  rddata <= 32'h120000E7;
        9'h026:  rddata <= 32'h08000513;
        9'h027:  rddata <= 32'h120000E7;
        9'h028:  rddata <= 32'h134000E7;
        9'h029:  rddata <= 32'h01851513;
        9'h02A:  rddata <= 32'h41855513;
        9'h02B:  rddata <= 32'h04054463;
        9'h02C:  rddata <= 32'h134000E7;
        9'h02D:  rddata <= 32'h00050E13;
        9'h02E:  rddata <= 32'h134000E7;
        9'h02F:  rddata <= 32'h00851513;
        9'h030:  rddata <= 32'h00AE6E33;
        9'h031:  rddata <= 32'h000E0E63;
        9'h032:  rddata <= 32'h134000E7;
        9'h033:  rddata <= 32'h00A38023;
        9'h034:  rddata <= 32'h00138393;
        9'h035:  rddata <= 32'hFFFE0E13;
        9'h036:  rddata <= 32'hFE0E18E3;
        9'h037:  rddata <= 32'hFA5FF06F;
        9'h038:  rddata <= 32'h01F00513;
        9'h039:  rddata <= 32'h0F8000E7;
        9'h03A:  rddata <= 32'h134000E7;
        9'h03B:  rddata <= 32'h000F8513;
        9'h03C:  rddata <= 32'h3108006F;
        9'h03D:  rddata <= 32'h0000006F;
        9'h03E:  rddata <= 32'h0002A303;
        9'h03F:  rddata <= 32'h00137313;
        9'h040:  rddata <= 32'h00030663;
        9'h041:  rddata <= 32'h0042A303;
        9'h042:  rddata <= 32'hFF1FF06F;
        9'h043:  rddata <= 32'h0002A303;
        9'h044:  rddata <= 32'h00237313;
        9'h045:  rddata <= 32'hFE031CE3;
        9'h046:  rddata <= 32'h10000313;
        9'h047:  rddata <= 32'h0062A223;
        9'h048:  rddata <= 32'h0002A303;
        9'h049:  rddata <= 32'h00237313;
        9'h04A:  rddata <= 32'hFE031CE3;
        9'h04B:  rddata <= 32'h00A2A223;
        9'h04C:  rddata <= 32'h00008067;
        9'h04D:  rddata <= 32'h0002A303;
        9'h04E:  rddata <= 32'h00137313;
        9'h04F:  rddata <= 32'hFE030CE3;
        9'h050:  rddata <= 32'h0042A503;
        9'h051:  rddata <= 32'h00008067;
        9'h052:  rddata <= 32'h726F632F;
        9'h053:  rddata <= 32'h612F7365;
        9'h054:  rddata <= 32'h2F323371;
        9'h055:  rddata <= 32'h746F6F62;
        9'h056:  rddata <= 32'h3371612E;
        9'h057:  rddata <= 32'h00000032;
        default: rddata <= 32'h00000000;
    endcase

endmodule
