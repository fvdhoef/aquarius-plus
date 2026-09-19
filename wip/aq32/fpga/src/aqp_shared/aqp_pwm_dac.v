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

    reg [15:0] q_left_sample  = 16'd0;
    reg [15:0] q_right_sample = 16'd0;

    always @(posedge clk) begin
        if (next_sample) begin
            // Convert to unsigned data
            q_left_sample  <= {~left_data[15],  left_data[14:0]};
            q_right_sample <= {~right_data[15], right_data[14:0]};
        end
    end

    // PWM output
    reg [16:0] q_pwmacc_left = 0;
    reg [16:0] q_pwmacc_right = 0;

    always @(posedge clk) begin
        q_pwmacc_left  <= {1'b0, q_pwmacc_left[15:0]}  + {1'b0, q_left_sample};
        q_pwmacc_right <= {1'b0, q_pwmacc_right[15:0]} + {1'b0, q_right_sample};
    end

    always @(posedge clk) begin
        audio_l <= q_pwmacc_left[16];
        audio_r <= q_pwmacc_right[16];
    end

endmodule
