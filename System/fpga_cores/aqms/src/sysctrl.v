module sysctrl(
    input  wire sysclk,
    inout  wire ebus_reset_n,
    input  wire reset_req,

    input  wire turbo_mode,

    output wire ebus_phi,
    output wire reset);

    //////////////////////////////////////////////////////////////////////////
    // Generate PHI signal @ 3.58MHz (or 7.16MHz in turbo mode)
    //////////////////////////////////////////////////////////////////////////
    reg       phi_r = 1'b0;
    reg [1:0] phi_div_r = 3'd0;

    wire [1:0] toggle_val = turbo_mode ? 2'd1 : 2'd3;
    wire       phi_reload = (phi_div_r == 2'd0);

    assign ebus_phi = phi_r;
    always @(posedge sysclk) begin
        if (phi_reload) begin
            phi_r <= !phi_r;
            phi_div_r <= toggle_val;
        end else begin
            phi_div_r <= phi_div_r - 3'd1;
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Generate external reset signal
    //////////////////////////////////////////////////////////////////////////
    reg [3:0] ext_reset_cnt_r = 0;
    reg       ext_reset;

    always @(posedge sysclk) begin
        ext_reset <= 1'b0;
        if (ext_reset_cnt_r < 4'd15) begin
            ext_reset <= 1'b1;
            if (phi_reload)
                ext_reset_cnt_r <= ext_reset_cnt_r + 4'd1;
        end
        if (reset_req)
            ext_reset_cnt_r <= 4'd0;
    end

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
        .reset_out(reset));

endmodule
