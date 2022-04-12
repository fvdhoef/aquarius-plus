module sysctrl(
    input  wire    sysclk,
    inout  wire    ext_reset_n,

    output wire    phi,
    output wire    reset);

    //////////////////////////////////////////////////////////////////////////
    // Generate external reset signal
    //////////////////////////////////////////////////////////////////////////
    wire ext_reset;

`ifdef __ICARUS__

    // Simulation: only have a short reset duration
    reg [4:0] ext_reset_cnt_r = 0;
    always @(posedge sysclk)
        if (!ext_reset_cnt_r[4])
            ext_reset_cnt_r <= ext_reset_cnt_r + 5'b1;
    assign ext_reset = !ext_reset_cnt_r[4];

`else

    // Synthesis: reset duration ~146ms
    reg [21:0] ext_reset_cnt_r = 0;
    always @(posedge sysclk)
        if (!ext_reset_cnt_r[21])
            ext_reset_cnt_r <= ext_reset_cnt_r + 22'b1;
    assign ext_reset = !ext_reset_cnt_r[21];

`endif

    // Tristate reset output
    assign ext_reset_n = ext_reset ? 1'b0 : 1'bZ;

    //////////////////////////////////////////////////////////////////////////
    // Generate internal reset signal
    //////////////////////////////////////////////////////////////////////////
    wire ext_reset_synced;

    // Synchronize external reset to internal clock
    reset_sync ext_reset_sync(
        .async_rst_in(!ext_reset_n),
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
    // Generate phi signal
    //////////////////////////////////////////////////////////////////////////
    reg [1:0] phi_div_r = 2'b0;
    always @(posedge sysclk) phi_div_r <= phi_div_r + 2'b1;
    assign phi = phi_div_r[1];

endmodule
