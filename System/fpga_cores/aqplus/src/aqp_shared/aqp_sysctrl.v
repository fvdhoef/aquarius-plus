`default_nettype none
`timescale 1 ns / 1 ps

module aqp_sysctrl(
    input  wire sysclk,
    inout  wire ebus_reset_n,
    input  wire reset_req,

    input  wire turbo_mode,
    input  wire turbo_unlimited,

    output wire ebus_phi,
    output reg  ebus_phi_clken,
    output wire reset);

    //////////////////////////////////////////////////////////////////////////
    // Generate external reset signal
    //////////////////////////////////////////////////////////////////////////
    wire ext_reset;

`ifdef MODEL_TECH
    localparam RESET_BITS = 5;  // Simulation: only have a short reset duration
`else
    localparam RESET_BITS = 24; // Synthesis: reset duration ~146ms
`endif

    reg [RESET_BITS-1:0] q_ext_reset_cnt = 0;
    always @(posedge sysclk) begin
        if (!q_ext_reset_cnt[RESET_BITS-1])
            q_ext_reset_cnt <= q_ext_reset_cnt + 1;
        if (reset_req)
            q_ext_reset_cnt <= 0;
    end

    assign ext_reset = !q_ext_reset_cnt[RESET_BITS - 1];

    // Tristate reset output
    assign ebus_reset_n = ext_reset ? 1'b0 : 1'bZ;

    //////////////////////////////////////////////////////////////////////////
    // Generate internal reset signal
    //////////////////////////////////////////////////////////////////////////
    wire ext_reset_synced;

    // Synchronize external reset to internal clock
    reset_sync ext_reset_sync(
        .async_rst_in(!ebus_reset_n),
        .clk(sysclk),
        .reset_out(ext_reset_synced));

    reg [4:0] q_reset_cnt = 0;
    always @(posedge sysclk)
        if (ext_reset_synced)
            q_reset_cnt <= 5'b0;
        else if (!q_reset_cnt[4])
            q_reset_cnt <= q_reset_cnt + 5'b1;

    assign reset = !q_reset_cnt[4];

    //////////////////////////////////////////////////////////////////////////
    // Generate phi signal @ 3.58MHz
    //////////////////////////////////////////////////////////////////////////
    reg       q_phi     = 1'b0;
    reg       q2_phi    = 1'b0;
    reg [2:0] q_phi_div = 3'd0;

    wire [2:0] toggle_val = turbo_mode ? (turbo_unlimited ? 3'd0 : 3'd3) : 3'd7;

    assign ebus_phi = q2_phi;

    always @(posedge sysclk) begin
        ebus_phi_clken <= 0;
        q2_phi         <= q_phi;

        if (q_phi_div == toggle_val) begin
            q_phi          <= !q_phi;
            q_phi_div      <= 0;
            ebus_phi_clken <= 1;

        end else begin
            q_phi_div <= q_phi_div + 3'd1;
        end
    end

endmodule
