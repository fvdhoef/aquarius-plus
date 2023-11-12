module pwm_dac(
    input  wire        rst,
    input  wire        clk,

    // Sample input
    input  wire        next_sample,
    input  wire [15:0] left_data,
    input  wire [15:0] right_data,

    // PWM audio output
    output reg         audio_l,
    output reg         audio_r);

    reg [15:0] left_sample_r;
    reg [15:0] right_sample_r;

    always @(posedge clk) begin
        if (next_sample) begin
            // Convert to unsigned data
            left_sample_r  <= left_data;
            right_sample_r <= right_data;
        end
    end

    // PWM output
    reg [16:0] pwmacc_left;
    reg [16:0] pwmacc_right;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            pwmacc_left  <= 0;
            pwmacc_right <= 0;

        end else begin
            pwmacc_left  <= {1'b0, pwmacc_left[15:0]}  + {1'b0, left_sample_r};
            pwmacc_right <= {1'b0, pwmacc_right[15:0]} + {1'b0, right_sample_r};
        end
    end

    always @(posedge clk) begin
        audio_l <= pwmacc_left[16];
        audio_r <= pwmacc_right[16];
    end

endmodule
