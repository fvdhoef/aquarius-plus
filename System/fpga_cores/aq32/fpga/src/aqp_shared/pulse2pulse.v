`default_nettype none
`timescale 1 ns / 1 ps

module pulse2pulse(
    input  wire in_clk,
    input  wire in_pulse,
    input  wire out_clk,
    output wire out_pulse);

    // in_clk domain
    reg q_toggle /* synthesis syn_keep=1 */ = 0;
    always @(posedge in_clk) if (in_pulse) q_toggle <= !q_toggle;

    // out_clk domain
    reg [2:0] q_toggle_sync /* synthesis syn_keep=1 */ = 0;
    always @(posedge out_clk) q_toggle_sync <= {q_toggle_sync[1:0], q_toggle};
    assign out_pulse = q_toggle_sync[1] ^ q_toggle_sync[2];

endmodule
