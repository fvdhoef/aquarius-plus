`default_nettype none
`timescale 1 ns / 1 ps

module distram64s#(
    parameter WIDTH = 32
) (
    input  wire               clk,
    input  wire         [5:0] addr,
    output wire [(WIDTH-1):0] rddata,
    input  wire [(WIDTH-1):0] wrdata,
    input  wire [(WIDTH-1):0] wren
);

    generate
        genvar i;
        for (i=0; i<WIDTH; i=i+1) begin: ram_gen
            RAM64X1S ram(
                .WCLK(clk), .D(wrdata[i]), .WE(wren[i]),
                .A5(addr[5]), .A4(addr[4]), .A3(addr[3]), .A2(addr[2]), .A1(addr[1]), .A0(addr[0]), .O(rddata[i]));
        end
    endgenerate

endmodule
