`default_nettype none
`timescale 1 ns / 1 ps

module aqp_pwm_dac(
    input  wire        clk,
    input  wire        reset,

    // Sample input
    input  wire        next_sample,
    input  wire [15:0] left_data,
    input  wire [15:0] right_data,

    // PWM audio output
    output reg         audio_l,
    output reg         audio_r);

    reg [15:0] q_left_sample;
    reg [15:0] q_right_sample;

    always @(posedge clk) begin
        if (next_sample) begin
            q_left_sample  <= left_data;
            q_right_sample <= right_data;
        end
    end

    // PWM output
    reg [16:0] q_pwmacc_left;
    reg [16:0] q_pwmacc_right;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_pwmacc_left  <= 0;
            q_pwmacc_right <= 0;

        end else begin
            q_pwmacc_left  <= {1'b0, q_pwmacc_left[15:0]}  + {1'b0, q_left_sample};
            q_pwmacc_right <= {1'b0, q_pwmacc_right[15:0]} + {1'b0, q_right_sample};
        end
    end

    always @(posedge clk) begin
        audio_l <= q_pwmacc_left[16];
        audio_r <= q_pwmacc_right[16];
    end

endmodule
