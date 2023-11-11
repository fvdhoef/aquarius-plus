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

module psg(
    input  wire        clk,
    input  wire        reset,

    input  wire  [7:0] wrdata,
    input  wire        wren,

    output wire [15:0] sample);

    //////////////////////////////////////////////////////////////////////////
    // Registers
    //////////////////////////////////////////////////////////////////////////
    reg  [1:0] latched_ch_r;
    reg  [9:0] ch1_freq_div_r;
    reg  [9:0] ch2_freq_div_r;
    reg  [9:0] ch3_freq_div_r;
    reg  [9:0] ch4_freq_div_r;
    reg  [3:0] ch1_atten_r;
    reg  [3:0] ch2_atten_r;
    reg  [3:0] ch3_atten_r;
    reg  [3:0] ch4_atten_r;
    reg        noise_fb_r;
    reg        noise_use_ch3_freq_r;
    reg        reset_lfsr_r;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            latched_ch_r         <= 2'b0;
            ch1_freq_div_r       <= 10'd0;
            ch2_freq_div_r       <= 10'd0;
            ch3_freq_div_r       <= 10'd0;
            ch4_freq_div_r       <= 10'd0;
            ch1_atten_r          <= 4'd15;
            ch2_atten_r          <= 4'd15;
            ch3_atten_r          <= 4'd15;
            ch4_atten_r          <= 4'd15;
            noise_fb_r           <= 1'b0;
            noise_use_ch3_freq_r <= 1'b0;
            reset_lfsr_r         <= 1'b1;
            
        end else begin
            reset_lfsr_r   <= 1'b0;

            if (wren) begin
                if (wrdata[7]) begin
                    latched_ch_r <= wrdata[6:5];
                end else begin
                    if (latched_ch_r == 2'b00) ch1_freq_div_r[9:4] <= wrdata[5:0];
                    if (latched_ch_r == 2'b01) ch2_freq_div_r[9:4] <= wrdata[5:0];
                    if (latched_ch_r == 2'b10) ch3_freq_div_r[9:4] <= wrdata[5:0];
                end

                if (wrdata[7:4] == 4'b1000) ch1_freq_div_r[3:0] <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1001) ch1_atten_r[3:0]    <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1010) ch2_freq_div_r[3:0] <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1011) ch2_atten_r[3:0]    <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1100) ch3_freq_div_r[3:0] <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1101) ch3_atten_r[3:0]    <= wrdata[3:0];
                if (wrdata[7:4] == 4'b1110) begin
                    noise_fb_r   <= wrdata[2];
                    noise_use_ch3_freq_r <= 1'b0;
                    reset_lfsr_r <= 1'b0;

                    if (wrdata[1:0] == 2'b00)
                        ch4_freq_div_r <= 10'h10;
                    else if (wrdata[1:0] == 2'b01)
                        ch4_freq_div_r <= 10'h20;
                    else if (wrdata[1:0] == 2'b10)
                        ch4_freq_div_r <= 10'h40;
                    else
                        noise_use_ch3_freq_r <= 1'b1;
                end
                if (wrdata[7:4] == 4'b1111) ch4_atten_r[3:0]    <= wrdata[3:0];
            end
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Clock divider (/256)
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] div_r = 8'd0;
    always @(posedge clk) div_r <= div_r + 8'd1;
    wire tick = (div_r == 8'd0);

    //////////////////////////////////////////////////////////////////////////
    // Sound generation
    //////////////////////////////////////////////////////////////////////////
    reg [15:0] noise_lfsr_r;
    reg  [9:0] ch1_cnt_r;
    reg  [9:0] ch2_cnt_r;
    reg  [9:0] ch3_cnt_r;
    reg  [9:0] ch4_cnt_r;
    reg        ch1_val_r;
    reg        ch2_val_r;
    reg        ch3_val_r;
    reg        ch4_val_r;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            noise_lfsr_r <= 16'h4000;
            ch1_cnt_r    <= 10'd0;
            ch2_cnt_r    <= 10'd0;
            ch3_cnt_r    <= 10'd0;
            ch4_cnt_r    <= 10'd0;
            ch1_val_r    <= 1'b0;
            ch2_val_r    <= 1'b0;
            ch3_val_r    <= 1'b0;
            ch4_val_r    <= 1'b0;

        end else begin
            if (tick) begin
                if (ch1_cnt_r == 10'd0) begin
                    ch1_cnt_r <= ch1_freq_div_r;
                    ch1_val_r <= !ch1_val_r;
                end else begin
                    ch1_cnt_r <= ch1_cnt_r - 10'd1;
                end

                if (ch2_cnt_r == 10'd0) begin
                    ch2_cnt_r <= ch2_freq_div_r;
                    ch2_val_r <= !ch2_val_r;
                end else begin
                    ch2_cnt_r <= ch2_cnt_r - 10'd1;
                end

                if (ch3_cnt_r == 10'd0) begin
                    ch3_cnt_r <= ch3_freq_div_r;
                    ch3_val_r <= !ch3_val_r;
                end else begin
                    ch3_cnt_r <= ch3_cnt_r - 10'd1;
                end

                if (ch4_cnt_r == 10'd0) begin
                    ch4_cnt_r <= noise_use_ch3_freq_r ? ch3_freq_div_r : ch4_freq_div_r;

                    ch4_val_r <= noise_lfsr_r[0];
                    noise_lfsr_r <= {1'b0, noise_lfsr_r[15:1]} ^ (noise_lfsr_r[0] ? (noise_fb_r ? 16'hF037 : 16'h8000) : 16'h0);

                end else begin
                    ch4_cnt_r <= ch4_cnt_r - 10'd1;
                end
            end

            if (reset_lfsr_r) noise_lfsr_r <= 16'h4000;
        
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Volume control
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] ch1_vol = ch1_val_r ? ch1_atten_r : 4'hF;
    wire [3:0] ch2_vol = ch2_val_r ? ch2_atten_r : 4'hF;
    wire [3:0] ch3_vol = ch3_val_r ? ch3_atten_r : 4'hF;
    wire [3:0] ch4_vol = ch4_val_r ? ch4_atten_r : 4'hF;

    reg [9:0] ch1_r;
    always @(posedge clk)
        if (tick) case (ch1_vol)
            4'h0: ch1_r <= 10'd1023;
            4'h1: ch1_r <= 10'd813;
            4'h2: ch1_r <= 10'd646;
            4'h3: ch1_r <= 10'd513;
            4'h4: ch1_r <= 10'd407;
            4'h5: ch1_r <= 10'd323;
            4'h6: ch1_r <= 10'd257;
            4'h7: ch1_r <= 10'd205;
            4'h8: ch1_r <= 10'd162;
            4'h9: ch1_r <= 10'd128;
            4'hA: ch1_r <= 10'd102;
            4'hB: ch1_r <= 10'd81;
            4'hC: ch1_r <= 10'd64;
            4'hD: ch1_r <= 10'd51;
            4'hE: ch1_r <= 10'd40;
            4'hF: ch1_r <= 10'd0;
        endcase

    reg [9:0] ch2_r;
    always @(posedge clk)
        if (tick) case (ch2_vol)
            4'h0: ch2_r <= 10'd1023;
            4'h1: ch2_r <= 10'd813;
            4'h2: ch2_r <= 10'd646;
            4'h3: ch2_r <= 10'd513;
            4'h4: ch2_r <= 10'd407;
            4'h5: ch2_r <= 10'd323;
            4'h6: ch2_r <= 10'd257;
            4'h7: ch2_r <= 10'd205;
            4'h8: ch2_r <= 10'd162;
            4'h9: ch2_r <= 10'd128;
            4'hA: ch2_r <= 10'd102;
            4'hB: ch2_r <= 10'd81;
            4'hC: ch2_r <= 10'd64;
            4'hD: ch2_r <= 10'd51;
            4'hE: ch2_r <= 10'd40;
            4'hF: ch2_r <= 10'd0;
        endcase

    reg [9:0] ch3_r;
    always @(posedge clk)
        if (tick) case (ch3_vol)
            4'h0: ch3_r <= 10'd1023;
            4'h1: ch3_r <= 10'd813;
            4'h2: ch3_r <= 10'd646;
            4'h3: ch3_r <= 10'd513;
            4'h4: ch3_r <= 10'd407;
            4'h5: ch3_r <= 10'd323;
            4'h6: ch3_r <= 10'd257;
            4'h7: ch3_r <= 10'd205;
            4'h8: ch3_r <= 10'd162;
            4'h9: ch3_r <= 10'd128;
            4'hA: ch3_r <= 10'd102;
            4'hB: ch3_r <= 10'd81;
            4'hC: ch3_r <= 10'd64;
            4'hD: ch3_r <= 10'd51;
            4'hE: ch3_r <= 10'd40;
            4'hF: ch3_r <= 10'd0;
        endcase

    reg [9:0] ch4_r;
    always @(posedge clk)
        if (tick) case (ch4_vol)
            4'h0: ch4_r <= 10'd1023;
            4'h1: ch4_r <= 10'd813;
            4'h2: ch4_r <= 10'd646;
            4'h3: ch4_r <= 10'd513;
            4'h4: ch4_r <= 10'd407;
            4'h5: ch4_r <= 10'd323;
            4'h6: ch4_r <= 10'd257;
            4'h7: ch4_r <= 10'd205;
            4'h8: ch4_r <= 10'd162;
            4'h9: ch4_r <= 10'd128;
            4'hA: ch4_r <= 10'd102;
            4'hB: ch4_r <= 10'd81;
            4'hC: ch4_r <= 10'd64;
            4'hD: ch4_r <= 10'd51;
            4'hE: ch4_r <= 10'd40;
            4'hF: ch4_r <= 10'd0;
        endcase

    assign sample =
        {2'b0, ch1_r, 4'b0} +
        {2'b0, ch2_r, 4'b0} +
        {2'b0, ch3_r, 4'b0} +
        {2'b0, ch4_r, 4'b0};

endmodule
