module aqp_sysctrl(
    input  wire sysclk,
    inout  wire ebus_reset_n,
    input  wire reset_req,

    input  wire turbo_mode,

    output wire ebus_phi,
    output wire reset);

    //////////////////////////////////////////////////////////////////////////
    // Generate external reset signal
    //////////////////////////////////////////////////////////////////////////
    wire ext_reset;

`ifdef __ICARUS__

    // Simulation: only have a short reset duration
    reg [4:0] ext_reset_cnt_r = 0;
    always @(posedge sysclk) begin
        if (!ext_reset_cnt_r[4])
            ext_reset_cnt_r <= ext_reset_cnt_r + 5'b1;
        if (reset_req)
            ext_reset_cnt_r <= 5'b0;
    end

    assign ext_reset = !ext_reset_cnt_r[4];

`else

    // Synthesis: reset duration ~146ms
    reg [22:0] ext_reset_cnt_r = 0;
    always @(posedge sysclk) begin
        if (!ext_reset_cnt_r[22])
            ext_reset_cnt_r <= ext_reset_cnt_r + 23'b1;
        if (reset_req)
            ext_reset_cnt_r <= 23'b0;
    end

    assign ext_reset = !ext_reset_cnt_r[22];

`endif

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

    reg [4:0] reset_cnt_r = 0;
    always @(posedge sysclk)
        if (ext_reset_synced)
            reset_cnt_r <= 5'b0;
        else if (!reset_cnt_r[4])
            reset_cnt_r <= reset_cnt_r + 5'b1;

    assign reset = !reset_cnt_r[4];

    //////////////////////////////////////////////////////////////////////////
    // Generate phi signal @ 3.58MHz
    //////////////////////////////////////////////////////////////////////////
    reg       phi_r = 1'b0;
    reg [1:0] phi_div_r = 3'd0;

    wire [1:0] toggle_val = turbo_mode ? 2'd1 : 2'd3;

    assign ebus_phi = phi_r;
    always @(posedge sysclk) begin
        if (phi_div_r == toggle_val) begin
            phi_r <= !phi_r;
            phi_div_r <= 3'd0;
        end else begin
            phi_div_r <= phi_div_r + 3'd1;
        end
    end

endmodule
