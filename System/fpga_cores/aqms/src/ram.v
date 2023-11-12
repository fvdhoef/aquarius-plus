module ram(
    input  wire        clk,
    input  wire [12:0] addr,
    output reg   [7:0] rddata,
    input  wire  [7:0] wrdata,
    input  wire        wren);

    reg [7:0] ram [8191:0];

    always @(posedge clk) begin
        if (wren) ram[addr] <= wrdata;
        rddata <= ram[addr];
    end

endmodule
