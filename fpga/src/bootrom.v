module bootrom(
    input  wire [7:0] addr,
    output reg  [7:0] data);

    always @(addr)
        case (addr)
            8'h00: data <= 8'h31;
            8'h01: data <= 8'hA0;
            8'h02: data <= 8'h38;
            8'h03: data <= 8'h21;
            8'h04: data <= 8'h01;
            8'h05: data <= 8'h30;
            8'h06: data <= 8'h34;
            8'h07: data <= 8'hCD;
            8'h08: data <= 8'h0C;
            8'h09: data <= 8'h00;
            8'h0A: data <= 8'h18;
            8'h0B: data <= 8'hF7;
            8'h0C: data <= 8'h21;
            8'h0D: data <= 8'h00;
            8'h0E: data <= 8'h00;
            8'h0F: data <= 8'h2B;
            8'h10: data <= 8'hA8;
            8'h11: data <= 8'hAF;
            8'h12: data <= 8'h3D;
            8'h13: data <= 8'h20;
            8'h14: data <= 8'hFD;
            8'h15: data <= 8'h05;
            8'h16: data <= 8'h20;
            8'h17: data <= 8'hF9;
            8'h18: data <= 8'hC9;
            default: data <= 8'h00; // NOP
        endcase

endmodule
