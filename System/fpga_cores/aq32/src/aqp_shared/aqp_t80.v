`default_nettype none
`timescale 1 ns / 1 ps

module aqp_t80(
    input  wire        clk,
    input  wire        reset,
    input  wire        clken,
    input  wire        phi,

    output wire [15:0] addr,        // should tristate when busak_n == 0
    output wire  [7:0] dq_out,
    input  wire  [7:0] dq_in,
    output wire        dq_oe,

    output wire        mreq_n,      // should tristate when busak_n == 0
    output wire        iorq_n,      // should tristate when busak_n == 0
    output wire        rd_n,        // should tristate when busak_n == 0
    output wire        wr_n,        // should tristate when busak_n == 0
    input  wire        wait_n,

    input  wire        busrq_n,
    output wire        busak_n,

    input  wire        int_n,
    input  wire        nmi_n
);

    reg q_phi;
    always @(posedge clk) q_phi <= phi;

    wire   phi_rising  = clken && !phi;
    wire   phi_falling = clken &&  phi;

    reg        q_mreq;
    reg        q_read;
    reg        MReq_Inhibit;
    reg        Req_Inhibit;
    reg        IORQ_t1;            // 30/10/19 Charlie Ingley - add IORQ control
    reg        IORQ_t2;            // 30/10/19 Charlie Ingley - add IORQ control
    reg        IORQ_int;           // 30/10/19 Charlie Ingley - add IORQ interrupt control
    reg  [2:0] IORQ_int_inhibit;
    reg        WR_t2;              // 30/10/19 Charlie Ingley - add WR control

    wire       t80_iorq;
    wire       t80_noread;
    wire       t80_write;
    wire [2:0] t80_mc;
    wire [2:0] t80_ts;
    reg  [7:0] q_t80_di;
    wire       t80_int_cycle_n;

    // Unused connections
    wire         t80_m1_n;
    wire         t80_rfsh_n;
    wire         t80_halt_n;
    wire         t80_inte;
    wire         t80_stop;
    wire [211:0] t80_reg;

    wire   mreq_rw = q_mreq   && (Req_Inhibit || MReq_Inhibit);  // added MREQ timing control
    wire   iorq_rw = t80_iorq && !(IORQ_t1 || IORQ_t2);          // added IORQ generation timing control

    assign mreq_n  = !mreq_rw;
    assign iorq_n  = !((IORQ_int && !IORQ_int_inhibit[2]) || iorq_rw);
    assign rd_n    = !(q_read && (mreq_rw || iorq_rw));
    assign wr_n    = !(t80_write && ((WR_t2 && mreq_rw) || iorq_rw));
    assign dq_oe   = busak_n && t80_write;

    T80 #(
        .Mode(0),
        .IOWait(1)
    ) t80(
        .RESET_n(!reset),
        .CLK_n(clk),
        .CEN(phi_rising),
        .WAIT_n(wait_n),
        .INT_n(int_n),
        .NMI_n(nmi_n),
        .BUSRQ_n(busrq_n),
        .M1_n(t80_m1_n),
        .IORQ(t80_iorq),
        .NoRead(t80_noread),
        .Write(t80_write),
        .RFSH_n(t80_rfsh_n),
        .HALT_n(t80_halt_n),
        .BUSAK_n(busak_n),
        .A(addr),
        .DInst(dq_in),
        .DI(q_t80_di),
        .DO(dq_out),
        .MC(t80_mc),
        .TS(t80_ts),
        .IntCycle_n(t80_int_cycle_n),
        .IntE(t80_inte),
        .Stop(t80_stop),
        .R800_mode(1'b0),
        .out0(1'b0),
        .REG(t80_reg),
        .DIRSet(1'b0),
        .DIR(212'b0)
    );

    always @(posedge clk) if (phi_falling && t80_ts == 3'd3 && busak_n) q_t80_di <= dq_in;

    // 30/10/19 Charlie Ingley - Generate WR_t2 to correct MREQ/WR timing
    always @(posedge clk or posedge reset)
        if (reset) begin
            WR_t2 <= 1'b0;

        end else if (phi_falling) begin
            if (t80_ts == 3'd2 && t80_mc != 3'd1) WR_t2 <= t80_write;  // WR starts on falling edge of T2 for MREQ
            if (t80_ts == 3'd3)                   WR_t2 <= 1'b0;       // end WR
        end

    // Generate Req_Inhibit
    always @(posedge clk or posedge reset)
        if (reset)           Req_Inhibit <= 1'b1;                                // Charlie Ingley 30/10/19 - changed Req_Inhibit polarity
        else if (phi_rising) Req_Inhibit <= !(t80_mc == 3'd1 && t80_ts == 3'd2); // by Fabio Belavenuto - fix behavior of Wait_n

    // Generate MReq_Inhibit
    always @(posedge clk or posedge reset)
        if (reset)            MReq_Inhibit <= 1'b1;                                // Charlie Ingley 30/10/19 - changed Req_Inhibit polarity
        else if (phi_falling) MReq_Inhibit <= !(t80_mc == 3'd1 && t80_ts == 3'd2); // by Fabio Belavenuto - fix behavior of Wait_n

    // Generate q_read for MREQ
    always @(posedge clk or posedge reset)
        if (reset) begin
            q_read <= 1'b0;
            q_mreq <= 1'b0;
        end else if (phi_falling) begin
            if (t80_mc == 3'd1) begin
                if (t80_ts == 3'd1) begin
                    q_read <= t80_int_cycle_n;
                    q_mreq <= t80_int_cycle_n;
                end
                if (t80_ts == 3'd3) begin
                    q_read <= 1'b0;
                    q_mreq <= 1'b1;
                end
                if (t80_ts == 3'd4) begin
                    q_mreq <= 1'b0;
                end

            end else begin
                if (t80_ts == 3'd1 && !t80_noread) begin
                    q_read <= !t80_write;
                    q_mreq <= !t80_iorq;
                end
                if (t80_ts == 3'd3) begin
                    q_read <= 1'b0;
                    q_mreq <= 1'b0;
                end
            end
        end

    // 30/10/19 Charlie Ingley - Generate IORQ_int for IORQ interrupt timing control
    always @(posedge clk or posedge reset)
        if (reset) begin
            IORQ_int <= 1'b0;
        end else if (phi_rising) begin
            if (t80_mc == 3'd1) begin
                if (t80_ts == 3'd1) IORQ_int <= !t80_int_cycle_n;
                if (t80_ts == 3'd2) IORQ_int <= 1'b0;
            end
        end

    always @(posedge clk or posedge reset)
        if (reset) begin
            IORQ_int_inhibit <= 3'd7;
        end else if (phi_falling) begin
            if (!t80_int_cycle_n) begin
                if (t80_mc == 3'd1) IORQ_int_inhibit <= {IORQ_int_inhibit[1:0], 1'b0};
                if (t80_mc == 3'd2) IORQ_int_inhibit <= 3'd7;
            end
        end

    // 30/10/19 Charlie Ingley - Generate IORQ_t1 for IORQ timing control
    always @(posedge clk or posedge reset)
        if (reset) begin
            IORQ_t1 <= 1'b1;
        end else if (phi_falling) begin
            if (t80_ts == 3'd1) IORQ_t1 <= !t80_int_cycle_n;
            if (t80_ts == 3'd3) IORQ_t1 <= 1'b1;
        end

    // 30/10/19 Charlie Ingley - Generate IORQ_t2 for IORQ timing control
    always @(posedge clk or posedge reset)
        if (reset)           IORQ_t2 <= 1'b1;
        else if (phi_rising) IORQ_t2 <= IORQ_t1;

endmodule
