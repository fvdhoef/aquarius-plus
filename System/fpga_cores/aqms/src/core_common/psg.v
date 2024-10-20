// SN76489 PSG

// Registers:
// 0 (000): Tone 1 frequency
// 1 (001): Tone 1 attenuation
// 2 (010): Tone 2 frequency
// 3 (011): Tone 2 attenuation
// 4 (100): Tone 3 frequency
// 5 (101): Tone 3 attenuation
// 6 (110): Noise control
// 7 (111): Noise attenuation
//
// Update frequency:
//  7 6 5 4 3 2 1 0    7 6 5 4 3 2 1 0 
// +-+---+-+-------+  +-+-+-----------+
// |1|r r|0| F[9:6]|  |0|X|   F[5:0]  |
// +-+---+-+-------+  +-+-+-----------+
//
// Update attenuator:
//  7 6 5 4 3 2 1 0
// +-+---+-+-------+
// |1|r r|1|  Vol  |
// +-+---+-+-------+
//
// Update noise control:
//  7 6 5 4 3 2 1 0
// +-+---+-+-------+
// |1|1 1|0|       |
// +-+---+-+-------+
//

`default_nettype none
`timescale 1 ns / 1 ps

module psg(
    input  wire        clk,
    input  wire        reset,

    input  wire  [7:0] wrdata,
    input  wire        wren,

    output wire [15:0] sample);

    //////////////////////////////////////////////////////////////////////////
    // Registers
    //////////////////////////////////////////////////////////////////////////
    reg  [1:0] q_latched_ch;
    reg  [9:0] q_ch1_freq_div;
    reg  [9:0] q_ch2_freq_div;
    reg  [9:0] q_ch3_freq_div;
    reg  [9:0] q_ch4_freq_div;
    reg  [3:0] q_ch1_atten;
    reg  [3:0] q_ch2_atten;
    reg  [3:0] q_ch3_atten;
    reg  [3:0] q_ch4_atten;
    reg        q_noise_fb;
    reg        q_noise_use_ch3_freq;
    reg        q_reset_lfsr;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_latched_ch         <= 2'b0;
            q_ch1_freq_div       <= 10'd0;
            q_ch2_freq_div       <= 10'd0;
            q_ch3_freq_div       <= 10'd0;
            q_ch4_freq_div       <= 10'd0;
            q_ch1_atten          <= 4'd15;
            q_ch2_atten          <= 4'd15;
            q_ch3_atten          <= 4'd15;
            q_ch4_atten          <= 4'd15;
            q_noise_fb           <= 1'b0;
            q_noise_use_ch3_freq <= 1'b0;
            q_reset_lfsr         <= 1'b1;
            
        end else begin
            q_reset_lfsr <= 1'b0;

            if (wren) begin
                if (wrdata[7]) begin
                    q_latched_ch <= wrdata[6:5];
                end else begin
                    if (q_latched_ch == 2'b00) q_ch1_freq_div[9:4] <= wrdata[5:0];
                    if (q_latched_ch == 2'b01) q_ch2_freq_div[9:4] <= wrdata[5:0];
                    if (q_latched_ch == 2'b10) q_ch3_freq_div[9:4] <= wrdata[5:0];
                end

                if (wrdata[7:4] == 4'b1000) q_ch1_freq_div[3:0] <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1001) q_ch1_atten[3:0]    <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1010) q_ch2_freq_div[3:0] <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1011) q_ch2_atten[3:0]    <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1100) q_ch3_freq_div[3:0] <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1101) q_ch3_atten[3:0]    <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1110) begin
                    q_noise_fb   <= wrdata[2];
                    q_noise_use_ch3_freq <= 1'b0;
                    q_reset_lfsr <= 1'b0;

                    if (wrdata[1:0] == 2'b00)
                        q_ch4_freq_div <= 10'h10;
                    else if (wrdata[1:0] == 2'b01)
                        q_ch4_freq_div <= 10'h20;
                    else if (wrdata[1:0] == 2'b10)
                        q_ch4_freq_div <= 10'h40;
                    else
                        q_noise_use_ch3_freq <= 1'b1;
                end
                if (wrdata[7:4] == 4'b1111) q_ch4_atten[3:0] <= wrdata[3:0];
            end
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Clock divider (/256)
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] q_div = 8'd0;
    always @(posedge clk) q_div <= q_div + 8'd1;
    wire tick = (q_div == 8'd0);

    //////////////////////////////////////////////////////////////////////////
    // Sound generation
    //////////////////////////////////////////////////////////////////////////
    reg [15:0] q_noise_lfsr;
    reg  [9:0] q_ch1_cnt;
    reg  [9:0] q_ch2_cnt;
    reg  [9:0] q_ch3_cnt;
    reg  [9:0] q_ch4_cnt;
    reg        q_ch1_val;
    reg        q_ch2_val;
    reg        q_ch3_val;
    reg        q_ch4_val;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_noise_lfsr <= 16'h4000;
            q_ch1_cnt    <= 10'd0;
            q_ch2_cnt    <= 10'd0;
            q_ch3_cnt    <= 10'd0;
            q_ch4_cnt    <= 10'd0;
            q_ch1_val    <= 1'b0;
            q_ch2_val    <= 1'b0;
            q_ch3_val    <= 1'b0;
            q_ch4_val    <= 1'b0;

        end else begin
            if (tick) begin
                if (q_ch1_cnt == 10'd0) begin
                    q_ch1_cnt <= q_ch1_freq_div;
                    q_ch1_val <= !q_ch1_val;
                end else begin
                    q_ch1_cnt <= q_ch1_cnt - 10'd1;
                end

                if (q_ch2_cnt == 10'd0) begin
                    q_ch2_cnt <= q_ch2_freq_div;
                    q_ch2_val <= !q_ch2_val;
                end else begin
                    q_ch2_cnt <= q_ch2_cnt - 10'd1;
                end

                if (q_ch3_cnt == 10'd0) begin
                    q_ch3_cnt <= q_ch3_freq_div;
                    q_ch3_val <= !q_ch3_val;
                end else begin
                    q_ch3_cnt <= q_ch3_cnt - 10'd1;
                end

                if (q_ch4_cnt == 10'd0) begin
                    q_ch4_cnt <= q_noise_use_ch3_freq ? q_ch3_freq_div : q_ch4_freq_div;

                    q_ch4_val <= q_noise_lfsr[0];
                    q_noise_lfsr <= {1'b0, q_noise_lfsr[15:1]} ^ (q_noise_lfsr[0] ? (q_noise_fb ? 16'hF037 : 16'h8000) : 16'h0);

                end else begin
                    q_ch4_cnt <= q_ch4_cnt - 10'd1;
                end
            end

            if (q_reset_lfsr) q_noise_lfsr <= 16'h4000;
        
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Volume control
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] ch1_vol = q_ch1_val ? q_ch1_atten : 4'hF;
    wire [3:0] ch2_vol = q_ch2_val ? q_ch2_atten : 4'hF;
    wire [3:0] ch3_vol = q_ch3_val ? q_ch3_atten : 4'hF;
    wire [3:0] ch4_vol = q_ch4_val ? q_ch4_atten : 4'hF;

    reg [9:0] q_ch1;
    always @(posedge clk)
        if (tick) case (ch1_vol)
            4'h0: q_ch1 <= 10'd1023;
            4'h1: q_ch1 <= 10'd813;
            4'h2: q_ch1 <= 10'd646;
            4'h3: q_ch1 <= 10'd513;
            4'h4: q_ch1 <= 10'd407;
            4'h5: q_ch1 <= 10'd323;
            4'h6: q_ch1 <= 10'd257;
            4'h7: q_ch1 <= 10'd205;
            4'h8: q_ch1 <= 10'd162;
            4'h9: q_ch1 <= 10'd128;
            4'hA: q_ch1 <= 10'd102;
            4'hB: q_ch1 <= 10'd81;
            4'hC: q_ch1 <= 10'd64;
            4'hD: q_ch1 <= 10'd51;
            4'hE: q_ch1 <= 10'd40;
            4'hF: q_ch1 <= 10'd0;
        endcase

    reg [9:0] q_ch2;
    always @(posedge clk)
        if (tick) case (ch2_vol)
            4'h0: q_ch2 <= 10'd1023;
            4'h1: q_ch2 <= 10'd813;
            4'h2: q_ch2 <= 10'd646;
            4'h3: q_ch2 <= 10'd513;
            4'h4: q_ch2 <= 10'd407;
            4'h5: q_ch2 <= 10'd323;
            4'h6: q_ch2 <= 10'd257;
            4'h7: q_ch2 <= 10'd205;
            4'h8: q_ch2 <= 10'd162;
            4'h9: q_ch2 <= 10'd128;
            4'hA: q_ch2 <= 10'd102;
            4'hB: q_ch2 <= 10'd81;
            4'hC: q_ch2 <= 10'd64;
            4'hD: q_ch2 <= 10'd51;
            4'hE: q_ch2 <= 10'd40;
            4'hF: q_ch2 <= 10'd0;
        endcase

    reg [9:0] q_ch3;
    always @(posedge clk)
        if (tick) case (ch3_vol)
            4'h0: q_ch3 <= 10'd1023;
            4'h1: q_ch3 <= 10'd813;
            4'h2: q_ch3 <= 10'd646;
            4'h3: q_ch3 <= 10'd513;
            4'h4: q_ch3 <= 10'd407;
            4'h5: q_ch3 <= 10'd323;
            4'h6: q_ch3 <= 10'd257;
            4'h7: q_ch3 <= 10'd205;
            4'h8: q_ch3 <= 10'd162;
            4'h9: q_ch3 <= 10'd128;
            4'hA: q_ch3 <= 10'd102;
            4'hB: q_ch3 <= 10'd81;
            4'hC: q_ch3 <= 10'd64;
            4'hD: q_ch3 <= 10'd51;
            4'hE: q_ch3 <= 10'd40;
            4'hF: q_ch3 <= 10'd0;
        endcase

    reg [9:0] q_ch4;
    always @(posedge clk)
        if (tick) case (ch4_vol)
            4'h0: q_ch4 <= 10'd1023;
            4'h1: q_ch4 <= 10'd813;
            4'h2: q_ch4 <= 10'd646;
            4'h3: q_ch4 <= 10'd513;
            4'h4: q_ch4 <= 10'd407;
            4'h5: q_ch4 <= 10'd323;
            4'h6: q_ch4 <= 10'd257;
            4'h7: q_ch4 <= 10'd205;
            4'h8: q_ch4 <= 10'd162;
            4'h9: q_ch4 <= 10'd128;
            4'hA: q_ch4 <= 10'd102;
            4'hB: q_ch4 <= 10'd81;
            4'hC: q_ch4 <= 10'd64;
            4'hD: q_ch4 <= 10'd51;
            4'hE: q_ch4 <= 10'd40;
            4'hF: q_ch4 <= 10'd0;
        endcase

    assign sample =
        {2'b0, q_ch1, 4'b0} +
        {2'b0, q_ch2, 4'b0} +
        {2'b0, q_ch3, 4'b0} +
        {2'b0, q_ch4, 4'b0};

endmodule
