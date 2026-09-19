`default_nettype none
`timescale 1 ns / 1 ps

module distram128d#(
    parameter WIDTH = 32
) (
    input  wire               clk,
    input  wire         [6:0] a_addr,
    output wire [(WIDTH-1):0] a_rddata,
    input  wire [(WIDTH-1):0] a_wrdata,
    input  wire [(WIDTH-1):0] a_wren,
    input  wire         [6:0] b_addr,
    output wire [(WIDTH-1):0] b_rddata
);

    generate
        genvar i;
        for (i=0; i<WIDTH; i=i+1) begin: ram_gen
            RAM128X1D ram(
                .WCLK(clk), .D(a_wrdata[i]), .WE(a_wren[i]),
                .A(a_addr), .SPO(a_rddata[i]),
                .DPRA(b_addr), .DPO(b_rddata[i]));
        end
    endgenerate

endmodule
