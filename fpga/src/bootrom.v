module bootrom(
    input  wire [7:0] addr,
    output reg  [7:0] data);

    always @(addr)
        case (addr)
            8'h00: data <= 8'h21;
            8'h01: data <= 8'h01;
            8'h02: data <= 8'h30;
            8'h03: data <= 8'h34;
            8'h04: data <= 8'h18;
            8'h05: data <= 8'hFD;
            default: data <= 8'h00; // NOP
        endcase

endmodule
