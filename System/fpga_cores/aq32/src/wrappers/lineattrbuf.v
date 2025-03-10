`default_nettype none
`timescale 1 ns / 1 ps

module lineattrbuf(
    input  wire        clk,
    input  wire  [8:0] idx1,
    input  wire        wrdata1,
    input  wire        wren1,

    input  wire  [8:0] idx2,
    output wire        rddata2);


    wire [31:0] rddata;
    assign rddata2 = rddata[idx2[8:4]];

    wire [31:0] spo;    // unused

    reg [31:0] wren;
    always @* begin
        wren = 32'b0;
        wren[idx1[8:4]] = wren1;
    end

    generate
        genvar i;
        for (i=0; i<32; i=i+1) begin: lineattr_gen
            RAM16X1D ram0(
                // Port 1
                .WCLK(clk),
                .A3(idx1[3]), .A2(idx1[2]), .A1(idx1[1]), .A0(idx1[0]), .SPO(spo[i]),
                .D(wrdata1), .WE(wren[i]),

                // Port 2
                .DPRA3(idx2[3]), .DPRA2(idx2[2]), .DPRA1(idx2[1]), .DPRA0(idx2[0]),
                .DPO(rddata[i]));
        end
    endgenerate

endmodule
