`default_nettype none
`timescale 1 ns / 1 ps

module div(
    input  wire        clk,
    input  wire        reset,

    input  wire [31:0] operand_l,
    input  wire [31:0] operand_r,
    input  wire        is_signed,
    input  wire        start,

    output reg         busy,
    output reg         done,
    output reg  [31:0] quotient,
    output reg  [31:0] remainder
);

    reg [31:0] q_dividend;
    reg [62:0] q_divisor;
    reg [31:0] q_quotient;
    reg [31:0] q_quotient_msk;

    always @(posedge clk or posedge reset)
        if (reset) begin
            busy           <= 0;
            done           <= 0;
            quotient       <= 0;
            remainder      <= 0;

            q_dividend     <= 0;
            q_divisor      <= 0;
            q_quotient     <= 0;
            q_quotient_msk <= 0;
            
        end else begin
            done <= 0;

            if (start) begin
                busy           <= 1;
                q_dividend     <=  (is_signed && operand_l[31]) ? -operand_l : operand_l;
                q_divisor      <= {(is_signed && operand_r[31]) ? -operand_r : operand_r, 31'b0};
                q_quotient     <= 32'b0;
                q_quotient_msk <= 32'h80000000;

            end else if (q_quotient_msk == 0 && busy) begin
                busy      <= 0;
                done      <= 1;
                quotient  <= (is_signed && operand_l[31] != operand_r[31] && operand_r != 32'b0) ? -q_quotient : q_quotient;
                remainder <= (is_signed && operand_l[31]) ? -q_dividend : q_dividend;

            end else begin
                if (q_divisor <= {31'b0, q_dividend}) begin
                    q_dividend <= q_dividend - q_divisor[31:0];
                    q_quotient <= q_quotient | q_quotient_msk;
                end

                q_divisor      <= {1'b0, q_divisor[62:1]};
                q_quotient_msk <= {1'b0, q_quotient_msk[31:1]};
            end
        end

endmodule
