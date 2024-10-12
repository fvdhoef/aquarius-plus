`default_nettype none
`timescale 1 ns / 1 ps

module ram(
    input  wire        clk,
    input  wire [12:0] addr,
    output reg   [7:0] rddata,
    input  wire  [7:0] wrdata,
    input  wire        wren);

    reg [7:0] mem [0:8191];

    always @(posedge clk) begin
        if (wren) mem[addr] <= wrdata;
        rddata <= mem[addr];
    end

endmodule
