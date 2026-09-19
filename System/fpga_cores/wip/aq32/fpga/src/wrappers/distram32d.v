`default_nettype none
`timescale 1 ns / 1 ps

module distram32d#(
    parameter WIDTH = 32
) (
    input  wire               clk,
    input  wire         [4:0] a_addr,
    output wire [(WIDTH-1):0] a_rddata,
    input  wire [(WIDTH-1):0] a_wrdata,
    input  wire [(WIDTH-1):0] a_wren,
    input  wire         [4:0] b_addr,
    output wire [(WIDTH-1):0] b_rddata
);

    generate
        genvar i;
        for (i=0; i<WIDTH; i=i+1) begin: ram_gen
            RAM32X1D ram(
                .WCLK(clk), .D(a_wrdata[i]), .WE(a_wren[i]),
                .A4   (a_addr[4]), .A3   (a_addr[3]), .A2   (a_addr[2]), .A1   (a_addr[1]), .A0   (a_addr[0]), .SPO(a_rddata[i]),
                .DPRA4(b_addr[4]), .DPRA3(b_addr[3]), .DPRA2(b_addr[2]), .DPRA1(b_addr[1]), .DPRA0(b_addr[0]), .DPO(b_rddata[i]));
        end
    endgenerate

endmodule
