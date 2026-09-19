`default_nettype none
`timescale 1 ns / 1 ps

module fm_phase(
    input  wire        clk,
    input  wire        reset,

    input  wire  [5:0] op_sel,
    input  wire        next,
    input  wire        restart,
    input  wire  [2:0] vib_pos,

    input  wire  [2:0] block,
    input  wire  [9:0] fnum,
    input  wire  [3:0] mult,
    input  wire        ksr,
    input  wire        dvb,
    input  wire        vib,

    output wire  [9:0] phase
);

    // Vibrato
    reg [2:0] range;
    always @* begin
        range = fnum[9:7];
        if (vib_pos[1:0] == 2'd0 || !vib)
            range = 0;
        else if (vib_pos[0])
            range = range >> 1;
        if (!dvb)
            range = range >> 1;
    end

    reg [9:0] f_num;
    always @* begin
        if (vib_pos[2])
            f_num = fnum - {7'b0, range};
        else
            f_num = fnum + {7'b0, range};
    end

    // Translate MULT parameter to multiplier value
    reg [4:0] fw_multiplier;
    always @* case (mult)
        4'd0:  fw_multiplier = 5'd1;
        4'd1:  fw_multiplier = 5'd2;
        4'd2:  fw_multiplier = 5'd4;
        4'd3:  fw_multiplier = 5'd6;
        4'd4:  fw_multiplier = 5'd8;
        4'd5:  fw_multiplier = 5'd10;
        4'd6:  fw_multiplier = 5'd12;
        4'd7:  fw_multiplier = 5'd14;
        4'd8:  fw_multiplier = 5'd16;
        4'd9:  fw_multiplier = 5'd18;
        4'd10: fw_multiplier = 5'd20;
        4'd11: fw_multiplier = 5'd20;
        4'd12: fw_multiplier = 5'd24;
        4'd13: fw_multiplier = 5'd24;
        4'd14: fw_multiplier = 5'd30;
        4'd15: fw_multiplier = 5'd30;
    endcase

    wire [16:0] fw        = {7'b0, f_num} << block;
    wire [20:0] fw_scaled = fw[16:1] * fw_multiplier;
    wire [19:0] phase_inc = fw_scaled[20:1];

    wire [18:0] d_op_phase, q_op_phase;

    distram64s#(.WIDTH(19)) fm_op_data_phase (
        .clk(clk),
        .addr(op_sel),
        .rddata(q_op_phase),
        .wrdata(d_op_phase),
        .wren({19{next}}));

    assign d_op_phase = (restart ? 19'd0 : q_op_phase) + phase_inc[18:0];

    assign phase = q_op_phase[18:9];

endmodule
