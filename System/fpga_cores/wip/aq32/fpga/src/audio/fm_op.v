`default_nettype none
`timescale 1 ns / 1 ps

module fm_op(
    input  wire        clk,
    input  wire  [2:0] ws,
    input  wire  [9:0] phase,
    input  wire  [8:0] env,
    output reg  [12:0] result
);

    reg [7:0] logsin_idx;
    always @* case (ws)
        default:    logsin_idx =  phase[7:0]        ^ {8{phase[8]}};
        3'd4, 3'd5: logsin_idx = {phase[6:0], 1'b0} ^ {8{phase[7]}};
    endcase

    wire [11:0] logsin_value;
    fm_logsin_rom fm_logsin_rom(.idx(logsin_idx), .value(logsin_value));

    reg negate;
    always @* case (ws)
        default:          negate = 0;
        3'd0, 3'd6, 3'd7: negate = phase[9];
        3'd4:             negate = phase[9:8] == 2'd1;
    endcase

    reg [12:0] out;
    always @* case (ws)
        default:          out =                       {1'b0, logsin_value};
        3'd1, 3'd4, 3'd5: out = phase[9] ? 13'h1000 : {1'b0, logsin_value};
        3'd3:             out = phase[8] ? 13'h1000 : {1'b0, logsin_value};
        3'd6:             out = 0;
        3'd7:             out = phase[9] ? {1'b0, ~phase[8:0], 3'b0} : {1'b0, phase[8:0], 3'b0};
    endcase

    reg [13:0] level;
    always @* begin
        level = {1'b0, out} + {1'b0, env, 3'b0};
        if (level > 14'h1FFF)
            level = 14'h1FFF;
    end

    wire [9:0] exp_value;
    fm_exp_rom fm_exp_rom(.idx(level[7:0]), .value(exp_value));

    always @* begin
        result = ({2'b01, exp_value, 1'b0} >> level[12:8]);
        if (result != 0)
            result = result ^ {13{negate}};
    end

endmodule
