`default_nettype none
`timescale 1 ns / 1 ps

module fmsynth(
    input  wire        clk,
    input  wire        reset,

    input  wire  [7:0] bus_addr,
    input  wire [31:0] bus_wrdata,
    input  wire        bus_wren,
    output reg  [31:0] bus_rddata,
    output wire        bus_wait,

    output reg  [15:0] audio_l,
    output reg  [15:0] audio_r
);

    wire        bus_wr      = bus_wren && !bus_wait;
    wire        sel_reg0    = bus_addr == 8'd0;
    wire        sel_reg1    = bus_addr == 8'd1;
    wire        sel_reg2    = bus_addr == 8'd2;
    wire        sel_ch_attr = bus_addr[7:5] == 3'b011;
    wire        sel_op_attr = bus_addr[7];

    reg  [18:0] q_accum_l;
    reg  [18:0] q_accum_r;
    reg  [31:0] q_kon;
    reg  [31:0] q_alg;
    reg         q_dam;
    reg         q_dvb;
    reg  [15:0] q_4op;
    wire [31:0] ch_attr_rddata;
    wire [31:0] op_attr_rddata;

    always @* begin
        bus_rddata = 32'b0;
        if (sel_reg0)    bus_rddata = {16'b0, q_4op};
        if (sel_reg1)    bus_rddata = {24'b0, q_dam, q_dvb, 6'b0};
        if (sel_reg2)    bus_rddata = q_kon;
        if (sel_ch_attr) bus_rddata = ch_attr_rddata;
        if (sel_op_attr) bus_rddata = op_attr_rddata;
    end

    //////////////////////////////////////////////////////////////////////////
    // Next sample signal
    //////////////////////////////////////////////////////////////////////////
    reg       q_next_sample;
    reg [8:0] q_next_sample_cnt;
    always @(posedge clk or posedge reset)
        if (reset) begin
            q_next_sample_cnt <= 0;
            q_next_sample     <= 0;

        end else begin
            q_next_sample <= 0;
            if (q_next_sample_cnt == 9'd505) begin
                q_next_sample_cnt <= 0;
                q_next_sample     <= 1;
            end else begin
                q_next_sample_cnt <= q_next_sample_cnt + 9'd1;
            end
        end

    //////////////////////////////////////////////////////////////////////////
    // AM / Vibrato
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] q_timer;
    reg [2:0] q_vibpos;
    reg [7:0] q_am_cnt;
    reg       d_am_dir, q_am_dir;

    always @* begin
        d_am_dir = q_am_dir;
        if (q_am_cnt == 8'd105)
            d_am_dir = 1;
        else if (q_am_cnt == 8'd0)
            d_am_dir = 0;
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_timer  <= 0;
            q_vibpos <= 0;
            q_am_cnt <= 0;
            q_am_dir <= 0;

        end else if (q_next_sample) begin
            q_timer  <= q_timer + 10'd1;
            q_am_dir <= d_am_dir;

            if (q_timer == 10'h3FF)
                q_vibpos <= q_vibpos + 3'd1;

            if (q_timer[5:0] == 6'h3F) begin
                q_am_cnt <= q_am_cnt + (q_am_dir ? 8'hFF : 8'h01);
            end
        end

    wire [5:0] am_val = q_dam ? q_am_cnt[7:2] : {2'b0, q_am_cnt[7:4]};

    //////////////////////////////////////////////////////////////////////////
    // Operator attributes
    //////////////////////////////////////////////////////////////////////////
    reg   [5:0] q_op_sel;
    wire  [2:0] op_ws;
    wire        op_am, op_vib, op_sus, op_ksr;
    wire  [3:0] op_mult;
    wire  [1:0] op_ksl;
    wire  [5:0] op_tl;
    wire  [3:0] op_ar, op_dr, op_sl, op_rr;

    wire [34:0] op_attr_a_rddata;
    assign op_attr_rddata = bus_addr[6] ? {29'b0, op_attr_a_rddata[34:32]} : op_attr_a_rddata[31:0];

    distram64d #(.WIDTH(35)) op_attr(
        .clk(clk),
        .a_addr(bus_addr[5:0]),
        .a_rddata(op_attr_a_rddata),
        .a_wrdata({bus_wrdata[2:0], bus_wrdata}),
        .a_wren({{3{sel_op_attr && bus_wr && bus_addr[6]}}, {32{sel_op_attr && bus_wr && !bus_addr[6]}}}),
        .b_addr(q_op_sel),
        .b_rddata({
            op_ws,
            op_am, op_vib, op_sus, op_ksr, op_mult,
            op_ksl, op_tl,
            op_ar, op_dr, op_sl, op_rr
        }));

    //////////////////////////////////////////////////////////////////////////
    // Channel attributes
    //////////////////////////////////////////////////////////////////////////
    wire        is_4op = q_4op[q_op_sel[5:2]];
    wire  [4:0] ch_sel = q_op_sel[5:1];
    wire  [6:0] ch_pan;
    wire  [2:0] ch_fb;
    wire        ch_kon = q_kon[q_op_sel[5:1]];
    wire  [2:0] ch_block;
    wire  [9:0] ch_fnum;

    wire        alg_2op = q_alg[q_op_sel[5:1]];
    wire  [1:0] alg_4op = {q_alg[{q_op_sel[5:2], 1'b0}], q_alg[{q_op_sel[5:2], 1'b1}]};

    assign ch_attr_rddata[31:27] = 0;
    assign ch_attr_rddata[16:13] = 0;

    distram32d #(.WIDTH(23)) ch_attr(
        .clk(clk),
        .a_addr(bus_addr[4:0]),
        .a_rddata({ch_attr_rddata[26:17], ch_attr_rddata[12:0]}),
        .a_wrdata({bus_wrdata[26:17], bus_wrdata[12:0]}),
        .a_wren({23{sel_ch_attr && bus_wr}}),
        .b_addr(ch_sel),
        .b_rddata({
            ch_pan,
            ch_fb,
            ch_block,
            ch_fnum
        }));

    reg do_fb;
    reg do_sum;
    reg do_mod;

    always @* begin
        if (!is_4op) begin
            do_fb  = !q_op_sel[0];
            do_sum =  q_op_sel[0] || alg_2op;
            do_mod = !alg_2op;

        end else begin
            do_fb  = q_op_sel[1:0] == 0;

            case (alg_4op)
                2'd0: do_sum = q_op_sel[1:0] == 2'd3;
                2'd1: do_sum = q_op_sel[0];
                2'd2: do_sum = !(q_op_sel[1] ^ q_op_sel[0]);
                2'd3: do_sum = q_op_sel[1:0] != 2'd1;
            endcase

            case (alg_4op)
                2'd0: do_mod = 1;
                2'd1: do_mod = q_op_sel[0];
                2'd2: do_mod = q_op_sel[1];
                2'd3: do_mod = q_op_sel[1:0] == 2'd2;
            endcase
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Operator envelope generator
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] env;
    wire       restart;

    reg        q_op_next;
    reg        q_op_reset;

    fm_eg fm_eg(
        .clk(clk),

        .op_sel(q_op_sel),
        .next(q_op_next),
        .op_reset(q_op_reset),

        .ar(op_ar),
        .dr(op_dr),
        .sl(op_sl),
        .rr(op_rr),
        .tl(op_tl),

        .block(ch_block),
        .fnum(ch_fnum),
        .ksr(op_ksr),
        .kon(ch_kon),
        .sus(op_sus),
        .am(op_am),
        .ksl(op_ksl),

        .am_val(am_val),

        .env(env),
        .restart(restart)
    );

    //////////////////////////////////////////////////////////////////////////
    // Operator phase counter
    //////////////////////////////////////////////////////////////////////////
    wire [9:0] phase;

    fm_phase fm_phase(
        .clk(clk),
        .reset(reset),

        .op_sel(q_op_sel),
        .next(q_op_next),
        .restart(restart),
        .vib_pos(q_vibpos),

        .block(ch_block),
        .fnum(ch_fnum),
        .mult(op_mult),
        .ksr(op_ksr),
        .dvb(q_dvb),
        .vib(op_vib),

        .phase(phase)
    );

    //////////////////////////////////////////////////////////////////////////
    // Operator
    //////////////////////////////////////////////////////////////////////////
    wire [25:0] d_fb_data,   q_fb_data;
    wire [12:0] d_op_result;
    reg  [12:0] q_op_result;
    wire [11:0] fb_mod;

    reg [9:0] op_modulation;
    always @* begin
        op_modulation = 0;
        if      (do_fb)  op_modulation = fb_mod[9:0];
        else if (do_mod) op_modulation = q_op_result[9:0];
    end

    wire [9:0] modulated_phase = phase + op_modulation;

    fm_op fm_op(
        .clk(clk),
        .ws(op_ws),
        .phase(modulated_phase),
        .env(env),
        .result(d_op_result));

    always @(posedge clk)
        if (q_op_next)
            q_op_result <= d_op_result;

    //////////////////////////////////////////////////////////////////////////
    // Feedback
    //////////////////////////////////////////////////////////////////////////
    assign d_fb_data = {q_fb_data[12:0], d_op_result};

    distram32s #(.WIDTH(26)) fm_ch_data_fb(
        .clk(clk),
        .addr(ch_sel),
        .rddata(q_fb_data),
        .wrdata(d_fb_data),
        .wren({26{q_op_next && !q_op_sel[0]}}));  // only store feedback for first operator in channel

    wire [13:0] fb_sum = {q_fb_data[25], q_fb_data[25:13]} + {q_fb_data[12], q_fb_data[12:0]};
    assign      fb_mod = (ch_fb == 0) ? 0 : ($signed(fb_sum[13:2]) >>> (~ch_fb));

    //////////////////////////////////////////////////////////////////////////
    // Panning
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] pan_l;
    wire [7:0] pan_r;
    fm_pan_l_rom fm_pan_l_rom(.idx(ch_pan), .value(pan_l));
    fm_pan_r_rom fm_pan_r_rom(.idx(ch_pan), .value(pan_r));

    wire signed [20:0] op_result_l = $signed(d_op_result) * $signed({1'b0, pan_l});
    wire signed [20:0] op_result_r = $signed(d_op_result) * $signed({1'b0, pan_r});
    wire [12:0] panned_l = op_result_l[20:8];
    wire [12:0] panned_r = op_result_r[20:8];

    //////////////////////////////////////////////////////////////////////////
    // State machine
    //////////////////////////////////////////////////////////////////////////
    localparam
        StIdle    = 2'd0,
        StStart   = 2'd1,
        StProcess = 2'd2,
        StNext    = 2'd3;

    reg [1:0] q_state;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            audio_l      <= 0;
            audio_r      <= 0;
            q_accum_l    <= 0;
            q_accum_r    <= 0;
            q_kon        <= 0;
            q_alg        <= 0;
            q_dam        <= 0;
            q_dvb        <= 0;
            q_4op        <= 0;
            q_op_sel     <= 0;
            q_op_next    <= 0;
            q_op_reset   <= 1;
            q_state      <= StIdle;

        end else begin
            q_op_next <= 0;

            case (q_state)
                StIdle: begin
                    // Wait for next sample to start
                    if (q_next_sample)
                        q_state <= StStart;
                end

                StStart: begin
                    q_state  <= StProcess;
                    q_op_sel <= 0;
                end

                StProcess: begin
                    if (!q_op_reset && do_sum) begin
                        q_accum_l <= q_accum_l + {{6{panned_l[12]}}, panned_l};
                        q_accum_r <= q_accum_r + {{6{panned_r[12]}}, panned_r};
                    end

                    q_state   <= StNext;
                    q_op_next <= 1;
                end

                StNext: begin
                    if (q_op_sel == 6'd63) begin
                        q_state    <= StIdle;
                        q_op_reset <= 0;

                        // Clamp output signal
                        audio_l <= q_accum_l[15:0];
                        if (q_accum_l[18] && q_accum_l[17:15] != 3'b111)
                            audio_l <= 16'h8000;
                        else if (!q_accum_l[18] && q_accum_l[17:15] != 3'b000)
                            audio_l <= 16'h7FFF;

                        // Clamp output signal
                        audio_r <= q_accum_r[15:0];
                        if (q_accum_r[18] && q_accum_r[17:15] != 3'b111)
                            audio_r <= 16'h8000;
                        else if (!q_accum_r[18] && q_accum_r[17:15] != 3'b000)
                            audio_r <= 16'h7FFF;

                        // Reset accumulator for next round
                        q_accum_l <= 0;
                        q_accum_r <= 0;

                    end else begin
                        q_op_sel <= q_op_sel + 6'd1;
                        q_state  <= StProcess;
                    end
                end

                default: begin end
            endcase

            if (bus_wr) begin
                if (sel_reg0) begin
                    q_4op <= bus_wrdata[15:0];
                end
                if (sel_reg1) begin
                    q_dam <= bus_wrdata[7];
                    q_dvb <= bus_wrdata[6];
                end
                if (sel_reg2) begin
                    q_kon <= bus_wrdata;
                end
                if (sel_ch_attr) begin
                    q_alg[bus_addr[4:0]] <= bus_wrdata[16];
                end
            end

        end
    end

    assign bus_wait = bus_wren && (q_state != StIdle);

endmodule
