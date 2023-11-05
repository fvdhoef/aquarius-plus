module lineattrbuf(
    input  wire        clk,
    input  wire  [7:0] idx1,
    input  wire  [1:0] wrdata1,
    input  wire        wren1,

    input  wire  [7:0] idx2,
    output wire  [1:0] rddata2);

    wire [15:0] rd_bit0;
    wire [15:0] rd_bit1;
    assign rddata2[0] = rd_bit0[idx2[7:4]];
    assign rddata2[1] = rd_bit1[idx2[7:4]];

    reg [15:0] wren;
    always @* begin
        wren = 16'b0;
        wren[idx1[7:4]] = wren1;
    end

    generate
        genvar i;
        for (i=0; i<16; i=i+1) begin: lineattr_gen
            RAM16X1D ram0(
                // Port 1
                .WCLK(clk),
                .A3(idx1[3]), .A2(idx1[2]), .A1(idx1[1]), .A0(idx1[0]), .SPO(),
                .D(wrdata1[0]), .WE(wren[i]),

                // Port 2
                .DPRA3(idx2[3]), .DPRA2(idx2[2]), .DPRA1(idx2[1]), .DPRA0(idx2[0]),
                .DPO(rd_bit0[i]));

            RAM16X1D ram1(
                // Port 1
                .WCLK(clk),
                .A3(idx1[3]), .A2(idx1[2]), .A1(idx1[1]), .A0(idx1[0]), .SPO(),
                .D(wrdata1[1]), .WE(wren[i]),

                // Port 2
                .DPRA3(idx2[3]), .DPRA2(idx2[2]), .DPRA1(idx2[1]), .DPRA0(idx2[0]),
                .DPO(rd_bit1[i]));
        end
    endgenerate

endmodule
