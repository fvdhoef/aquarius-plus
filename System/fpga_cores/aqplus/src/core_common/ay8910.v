`default_nettype none
`timescale 1 ns / 1 ps

module ay8910(
    input  wire       clk,
    input  wire       reset,

    input  wire       a0,
    input  wire       wren,
    input  wire [7:0] wrdata,
    output reg  [7:0] rddata,

    input  wire [7:0] ioa_in_data,
    output wire [7:0] ioa_out_data,
    output wire       ioa_oe,

    input  wire [7:0] iob_in_data,
    output wire [7:0] iob_out_data,
    output wire       iob_oe,

    output wire [9:0] ch_a,
    output wire [9:0] ch_b,
    output wire [9:0] ch_c);

    //////////////////////////////////////////////////////////////////////////
    // Bus register interface
    //////////////////////////////////////////////////////////////////////////

    // Address latch
    reg [3:0] q_reg_addr;
    always @(posedge clk) if (wren && a0) q_reg_addr <= wrdata[3:0];

    // Bus registers
    reg [11:0] q_a_period,  q_b_period,  q_c_period;
    reg  [4:0] q_a_volume,  q_b_volume,  q_c_volume;
    reg        q_a_tone_n,  q_b_tone_n,  q_c_tone_n;
    reg        q_a_noise_n, q_b_noise_n, q_c_noise_n;
    reg        q_ioa_out,   q_iob_out;
    reg  [4:0] q_noise_period;
    reg [15:0] q_envelope_period;
    reg        q_envelope_hold, q_envelope_alternate, q_envelope_attack, q_envelope_continue;
    reg  [7:0] q_ioa_out_data, q_iob_out_data;

    assign ioa_out_data = q_ioa_out_data;
    assign ioa_oe       = q_ioa_out;
    assign iob_out_data = q_iob_out_data;
    assign iob_oe       = q_iob_out;

    wire eashape_wr = wren && !a0 && q_reg_addr == 4'hD;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_a_period           <= 12'b0;
            q_b_period           <= 12'b0;
            q_c_period           <= 12'b0;
            q_a_volume           <= 5'b0;
            q_b_volume           <= 5'b0;
            q_c_volume           <= 5'b0;
            q_a_tone_n           <= 1'b1;
            q_b_tone_n           <= 1'b1;
            q_c_tone_n           <= 1'b1;
            q_a_noise_n          <= 1'b1;
            q_b_noise_n          <= 1'b1;
            q_c_noise_n          <= 1'b1;
            q_ioa_out            <= 1'b0;
            q_iob_out            <= 1'b0;
            q_noise_period       <= 5'b0;
            q_envelope_period    <= 16'b0;
            q_envelope_hold      <= 1'b0;
            q_envelope_alternate <= 1'b0;
            q_envelope_attack    <= 1'b0;
            q_envelope_continue  <= 1'b0;
            q_ioa_out_data       <= 8'b0;
            q_iob_out_data       <= 8'b0;

        end else if (wren && !a0) begin
            case (q_reg_addr)
                4'h0: q_a_period[ 7:0]        <= wrdata;
                4'h1: q_a_period[11:8]        <= wrdata[3:0];
                4'h2: q_b_period[ 7:0]        <= wrdata;
                4'h3: q_b_period[11:8]        <= wrdata[3:0];
                4'h4: q_c_period[ 7:0]        <= wrdata;
                4'h5: q_c_period[11:8]        <= wrdata[3:0];
                4'h6: q_noise_period          <= wrdata[4:0];
                4'h7: { q_iob_out, q_ioa_out, q_c_noise_n, q_b_noise_n, q_a_noise_n, q_c_tone_n, q_b_tone_n, q_a_tone_n } <= wrdata;
                4'h8: q_a_volume              <= wrdata[4:0];
                4'h9: q_b_volume              <= wrdata[4:0];
                4'hA: q_c_volume              <= wrdata[4:0];
                4'hB: q_envelope_period[7:0]  <= wrdata;
                4'hC: q_envelope_period[15:8] <= wrdata;
                4'hD: { q_envelope_continue, q_envelope_attack, q_envelope_alternate, q_envelope_hold } <= wrdata[3:0];
                4'hE: q_ioa_out_data          <= wrdata;
                4'hF: q_iob_out_data          <= wrdata;
            endcase
        end

    always @*
        case (q_reg_addr)
            4'h0: rddata = q_a_period[ 7:0];
            4'h1: rddata = { 4'b0, q_a_period[11:8] };
            4'h2: rddata = q_b_period[ 7:0];
            4'h3: rddata = { 4'b0, q_b_period[11:8] };
            4'h4: rddata = q_c_period[ 7:0];
            4'h5: rddata = { 4'b0, q_c_period[11:8] };
            4'h6: rddata = { 3'b0, q_noise_period };
            4'h7: rddata = { q_iob_out, q_ioa_out, q_c_noise_n, q_b_noise_n, q_a_noise_n, q_c_tone_n, q_b_tone_n, q_a_tone_n };
            4'h8: rddata = { 3'b0, q_a_volume };
            4'h9: rddata = { 3'b0, q_b_volume };
            4'hA: rddata = { 3'b0, q_c_volume };
            4'hB: rddata = q_envelope_period[7:0];
            4'hC: rddata = q_envelope_period[15:8];
            4'hD: rddata = { 4'b0, q_envelope_continue, q_envelope_attack, q_envelope_alternate, q_envelope_hold };
            4'hE: rddata = q_ioa_out ? q_ioa_out_data : ioa_in_data;
            4'hF: rddata = q_iob_out ? q_iob_out_data : iob_in_data;
        endcase

    //////////////////////////////////////////////////////////////////////////
    // Clock divider (/128)
    //////////////////////////////////////////////////////////////////////////
    reg [6:0] q_div = 7'b0;
    always @(posedge clk) q_div <= q_div + 7'b1;
    wire tick = (q_div == 7'b0);

    //////////////////////////////////////////////////////////////////////////
    // Tone generators
    //////////////////////////////////////////////////////////////////////////
    reg [11:0] q_a_count,  q_b_count,  q_c_count;
    reg        q_a_output, q_b_output, q_c_output;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_a_count  <= 12'b0;
            q_b_count  <= 12'b0;
            q_c_count  <= 12'b0;
            q_a_output <= 1'b0;
            q_b_output <= 1'b0;
            q_c_output <= 1'b0;

        end else if (tick) begin
            // Channel A
            if (q_a_count >= q_a_period) begin
                q_a_output <= !q_a_output;
                q_a_count  <= 12'd0;
            end else begin
                q_a_count  <= q_a_count + 12'd1;
            end

            // Channel B
            if (q_b_count >= q_b_period) begin
                q_b_output <= !q_b_output;
                q_b_count  <= 12'd0;
            end else begin
                q_b_count  <= q_b_count + 12'd1;
            end

            // Channel C
            if (q_c_count >= q_c_period) begin
                q_c_output <= !q_c_output;
                q_c_count  <= 12'd0;
            end else begin
                q_c_count  <= q_c_count + 12'd1;
            end
        end

    //////////////////////////////////////////////////////////////////////////
    // Noise generator
    //////////////////////////////////////////////////////////////////////////
    reg  [4:0] q_noise_count;
    reg        q_prescale_noise;
    reg [16:0] q_lfsr;
    wire       noise_val = q_lfsr[0];

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_noise_count    <= 5'd0;
            q_prescale_noise <= 1'b0;
            q_lfsr           <= 17'd1;

        end else if (tick) begin
            if (q_noise_count >= q_noise_period) begin
                q_noise_count <= 5'd0;
                q_prescale_noise <= !q_prescale_noise;
                if (q_prescale_noise) q_lfsr <= {q_lfsr[0] ^ q_lfsr[3], q_lfsr[16:1]};
            end else begin
                q_noise_count <= q_noise_count + 5'd1;
            end
        end

    //////////////////////////////////////////////////////////////////////////
    // Channel values
    //////////////////////////////////////////////////////////////////////////
    wire a_val = (q_a_output | q_a_tone_n) & (noise_val | q_a_noise_n);
    wire b_val = (q_b_output | q_b_tone_n) & (noise_val | q_b_noise_n);
    wire c_val = (q_c_output | q_c_tone_n) & (noise_val | q_c_noise_n);

    //////////////////////////////////////////////////////////////////////////
    // Envelope generator
    //////////////////////////////////////////////////////////////////////////

    // Period counter
    reg [16:0] q_envelope_period_cnt;
    wire       envelope_period_done = q_envelope_period_cnt >= {q_envelope_period, 1'b0};

    always @(posedge clk or posedge reset)
        if (reset)
            q_envelope_period_cnt <= 17'd0;
        else if (tick)
            q_envelope_period_cnt <= envelope_period_done ? 17'd0 : (q_envelope_period_cnt + 17'd1);

    // Envelope counter
    reg  [3:0] q_envelope_cnt;
    reg        q_envelope_updated;
    reg        q_envelope_cnt_up;
    reg  [3:0] q_envelope_volume;
    reg        q_envelope_stop;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_envelope_cnt     <= 4'd0;
            q_envelope_updated <= 1'b0;
            q_envelope_cnt_up  <= 1'b0;
            q_envelope_volume  <= 4'd0;
            q_envelope_stop    <= 1'b0;

        end else begin
            if (tick && envelope_period_done) begin
                if (!q_envelope_stop) begin
                    q_envelope_cnt <= q_envelope_cnt - 4'd1;

                    if (q_envelope_cnt == 4'd0) begin
                        if (!q_envelope_continue || q_envelope_hold) begin
                            q_envelope_cnt <= 4'd0;
                            q_envelope_stop <= 1'b1;
                        end

                        if (( q_envelope_continue && q_envelope_alternate) ||
                            (!q_envelope_continue && q_envelope_attack)) begin

                            q_envelope_cnt_up <= !q_envelope_cnt_up;
                        end
                    end
                end

                q_envelope_volume <= q_envelope_cnt_up ? (q_envelope_cnt ^ 4'hF) : q_envelope_cnt;

                if (q_envelope_updated) begin
                    q_envelope_updated <= 1'b0;
                    q_envelope_cnt     <= 4'd15;
                    q_envelope_cnt_up  <= q_envelope_attack;
                    q_envelope_stop    <= 1'b0;
                end
            end

            if (eashape_wr) begin
                q_envelope_updated <= 1'b1;
            end
        end

    //////////////////////////////////////////////////////////////////////////
    // Mixer
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] a_volume = a_val ? (q_a_volume[4] ? q_envelope_volume : q_a_volume[3:0]) : 4'd0;
    wire [3:0] b_volume = b_val ? (q_b_volume[4] ? q_envelope_volume : q_b_volume[3:0]) : 4'd0;
    wire [3:0] c_volume = c_val ? (q_c_volume[4] ? q_envelope_volume : q_c_volume[3:0]) : 4'd0;

    reg [9:0] q_ch_a = 10'd0;
    always @(posedge clk)
        if (tick) case (a_volume)
            4'h0: q_ch_a <= 0;
            4'h1: q_ch_a <= 6;
            4'h2: q_ch_a <= 9;
            4'h3: q_ch_a <= 13;
            4'h4: q_ch_a <= 19;
            4'h5: q_ch_a <= 27;
            4'h6: q_ch_a <= 39;
            4'h7: q_ch_a <= 56;
            4'h8: q_ch_a <= 80;
            4'h9: q_ch_a <= 116;
            4'hA: q_ch_a <= 166;
            4'hB: q_ch_a <= 239;
            4'hC: q_ch_a <= 344;
            4'hD: q_ch_a <= 495;
            4'hE: q_ch_a <= 712;
            4'hF: q_ch_a <= 1023;
        endcase

    reg [9:0] q_ch_b = 10'd0;
    always @(posedge clk)
        if (tick) case (b_volume)
            4'h0: q_ch_b <= 0;
            4'h1: q_ch_b <= 6;
            4'h2: q_ch_b <= 9;
            4'h3: q_ch_b <= 13;
            4'h4: q_ch_b <= 19;
            4'h5: q_ch_b <= 27;
            4'h6: q_ch_b <= 39;
            4'h7: q_ch_b <= 56;
            4'h8: q_ch_b <= 80;
            4'h9: q_ch_b <= 116;
            4'hA: q_ch_b <= 166;
            4'hB: q_ch_b <= 239;
            4'hC: q_ch_b <= 344;
            4'hD: q_ch_b <= 495;
            4'hE: q_ch_b <= 712;
            4'hF: q_ch_b <= 1023;
        endcase

    reg [9:0] q_ch_c = 10'd0;
    always @(posedge clk)
        if (tick) case (c_volume)
            4'h0: q_ch_c <= 0;
            4'h1: q_ch_c <= 6;
            4'h2: q_ch_c <= 9;
            4'h3: q_ch_c <= 13;
            4'h4: q_ch_c <= 19;
            4'h5: q_ch_c <= 27;
            4'h6: q_ch_c <= 39;
            4'h7: q_ch_c <= 56;
            4'h8: q_ch_c <= 80;
            4'h9: q_ch_c <= 116;
            4'hA: q_ch_c <= 166;
            4'hB: q_ch_c <= 239;
            4'hC: q_ch_c <= 344;
            4'hD: q_ch_c <= 495;
            4'hE: q_ch_c <= 712;
            4'hF: q_ch_c <= 1023;
        endcase

    assign ch_a = q_ch_a;
    assign ch_b = q_ch_b;
    assign ch_c = q_ch_c;

endmodule
