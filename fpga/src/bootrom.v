module bootrom(
    input  wire [7:0] addr,
    output reg  [7:0] data);

    always @(addr)
        case (addr)
            8'h00: data <= 8'h21;
            8'h01: data <= 8'h10;
            8'h02: data <= 8'h00;
            8'h03: data <= 8'h7E;
            8'h04: data <= 8'hB7;
            8'h05: data <= 8'h28;
            8'h06: data <= 8'h05;
            8'h07: data <= 8'hD3;
            8'h08: data <= 8'hF5;
            8'h09: data <= 8'h23;
            8'h0A: data <= 8'h18;
            8'h0B: data <= 8'hF7;
            8'h0C: data <= 8'h18;
            8'h0D: data <= 8'hF2;
            8'h0E: data <= 8'h18;
            8'h0F: data <= 8'hFE;
            8'h10: data <= 8'h48;
            8'h11: data <= 8'h65;
            8'h12: data <= 8'h6C;
            8'h13: data <= 8'h6C;
            8'h14: data <= 8'h6F;
            8'h15: data <= 8'h20;
            8'h16: data <= 8'h77;
            8'h17: data <= 8'h6F;
            8'h18: data <= 8'h72;
            8'h19: data <= 8'h6C;
            8'h1A: data <= 8'h64;
            8'h1B: data <= 8'h21;
            8'h1C: data <= 8'h0D;
            8'h1D: data <= 8'h0A;
            8'h1E: data <= 8'h00;
            default: data <= 8'h00; // NOP
        endcase

endmodule
