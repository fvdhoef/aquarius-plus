`default_nettype none
`timescale 1 ns / 1 ps

module fm_eg(
    input  wire        clk,

    input  wire  [5:0] op_sel,
    input  wire        next,
    input  wire        op_reset,

    input  wire  [3:0] ar,
    input  wire  [3:0] dr,
    input  wire  [3:0] sl,
    input  wire  [3:0] rr,
    input  wire  [5:0] tl,

    input  wire  [2:0] block,
    input  wire  [9:0] fnum,
    input  wire        ksr,
    input  wire        kon,
    input  wire        sus,
    input  wire        am,
    input  wire  [1:0] ksl,

    input  wire  [5:0] am_val,

    output reg   [8:0] env,
    output wire        restart
);

    wire  [4:0] suslvl = (sl == 4'hF) ? 5'h1F : {1'b0, sl};

    reg   [1:0] d_eg_stage;
    wire  [1:0] q_eg_stage;
    reg  [17:0] d_eg_cnt;
    wire [14:0] q_eg_cnt;
    reg   [9:0] d_eg_env;
    wire  [8:0] q_eg_env;

    distram64s#(.WIDTH(26)) fm_op_data_eg(
        .clk(clk),
        .addr(op_sel),
        .rddata({q_eg_stage, q_eg_cnt,       q_eg_env}),
        .wrdata({d_eg_stage, d_eg_cnt[14:0], d_eg_env[8:0]}),
        .wren({26{next}}));

    // KSL ROM
    reg [6:0] kslrom_val;
    always @* case (fnum[9:6])
        4'd0:  kslrom_val = 7'd0;
        4'd1:  kslrom_val = 7'd32;
        4'd2:  kslrom_val = 7'd40;
        4'd3:  kslrom_val = 7'd45;
        4'd4:  kslrom_val = 7'd48;
        4'd5:  kslrom_val = 7'd51;
        4'd6:  kslrom_val = 7'd53;
        4'd7:  kslrom_val = 7'd55;
        4'd8:  kslrom_val = 7'd56;
        4'd9:  kslrom_val = 7'd58;
        4'd10: kslrom_val = 7'd59;
        4'd11: kslrom_val = 7'd60;
        4'd12: kslrom_val = 7'd61;
        4'd13: kslrom_val = 7'd62;
        4'd14: kslrom_val = 7'd63;
        4'd15: kslrom_val = 7'd64;
    endcase

    wire [3:0] blk = 4'd8 - {1'b0, block};
    reg  [8:0] eg_ksl;
    always @* begin
        eg_ksl = {kslrom_val, 2'b0} - {blk, 5'b0};

        case (ksl)
            2'd0: eg_ksl = eg_ksl >> 8;
            2'd1: eg_ksl = eg_ksl >> 1;
            2'd2: eg_ksl = eg_ksl >> 2;
            2'd3: eg_ksl = eg_ksl >> 0;
        endcase
    end

    reg [10:0] tmp;
    always @* begin
        tmp = {2'b0, q_eg_env};
        tmp = tmp + {3'b0, tl, 2'b0} + {2'b0, eg_ksl};
        if (am)
            tmp = tmp + {5'b0, am_val};

        if (tmp > 11'd511)
            env = 9'd511;
        else
            env = tmp[8:0];
    end

    localparam
        StageAttack  = 2'd0,
        StageDecay   = 2'd1,
        StageSustain = 2'd2,
        StageRelease = 2'd3;

    // Rate offset (based on key scaling)
    wire [3:0] ksv = {block, fnum[9]};
    wire [3:0] rof = ksr ? ksv : (ksv >> 2);

    // Stage rate
    reg [3:0] stage_rate;
    assign    restart = (kon && q_eg_stage == StageRelease);

    always @* begin
        case (q_eg_stage)
            StageAttack:  stage_rate = ar;
            StageDecay:   stage_rate = dr;
            StageSustain: stage_rate = sus ? 0 : rr;
            StageRelease: stage_rate = rr;
        endcase

        if (restart)
            stage_rate = ar;
    end

    // Rate
    reg [6:0] rate;
    always @* begin
        rate = {3'b0, rof} + {1'b0, stage_rate, 2'b0};
        if (rate > 7'd63)
            rate = 7'd63;
    end

    always @* begin
        d_eg_cnt = {3'b0, q_eg_cnt};
        if (stage_rate != 4'd0)
            d_eg_cnt = d_eg_cnt + ({15'b0, 1'b1, rate[1:0]} << rate[5:2]);
    end
    wire [2:0] overflow = d_eg_cnt[17:15];

    always @* begin
        // d_eg_env_cnt = eg_env_cnt_next[23:0];
        d_eg_stage   = q_eg_stage;
        d_eg_env     = {1'b0, q_eg_env};

        case (q_eg_stage)
            StageAttack: begin
                if (q_eg_env == 9'd0)
                    d_eg_stage = StageDecay;
                else if (overflow != 3'd0)
                    d_eg_env = d_eg_env - (((q_eg_env * overflow) >> 3) + 1);
            end

            StageDecay: begin
                if (q_eg_env[8:4] >= suslvl)
                    d_eg_stage = StageSustain;
                else
                    d_eg_env = {1'b0, q_eg_env} + {7'b0, overflow};
            end

            StageSustain, StageRelease: begin
                d_eg_env = {1'b0, q_eg_env} + {7'b0, overflow};
            end
        endcase

        if (restart)
            d_eg_stage = StageAttack;
        if (!kon || op_reset)
            d_eg_stage = StageRelease;

        if (d_eg_env[9] || op_reset)
            d_eg_env = {1'b0, 9'd511};
    end

endmodule
