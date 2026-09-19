`default_nettype none
`timescale 1 ns / 1 ps

module t80s(
    input  wire        reset,
    input  wire        clk,
    input  wire        clken,
    output wire [15:0] bus_addr,
    output wire  [7:0] bus_wrdata,
    output reg         bus_rd,
    output wire        bus_t1_rd,
    output reg         bus_wr,
    output wire        bus_wr_d,
    output wire        bus_wrcycle,
    output wire        bus_iorq,
    input  wire        bus_wait,
    input  wire  [7:0] bus_rddata,
    input  wire        bus_int,
    input  wire        bus_nmi
);

    wire       int_cycle_n;
    wire       noread;
    wire       write;
    reg  [7:0] q_di;
    wire [2:0] mcycle;
    wire [2:0] tstate;

    wire       m1_n_unused;
    wire       rfsh_n_unused;
    wire       halt_n_unused;
    wire       busak_n_unused;
    wire       int_e_unused;
    wire       stop_unused;
    wire [211:0] reg_unused;

    assign bus_wrcycle = write;

    T80 #(
        .Mode       ( 0                ),
        .IOWait     ( 0                )
    ) T80 (
        .RESET_n    ( !reset           ),
        .CLK_n      ( clk              ),
        .CEN        ( clken            ),
        .WAIT_n     ( !bus_wait        ),
        .INT_n      ( !bus_int         ),
        .NMI_n      ( !bus_nmi         ),
        .BUSRQ_n    ( 1'b1             ),
        .M1_n       ( m1_n_unused      ),
        .IORQ       ( bus_iorq         ),
        .NoRead     ( noread           ),
        .Write      ( write            ),
        .RFSH_n     ( rfsh_n_unused    ),
        .HALT_n     ( halt_n_unused    ),
        .BUSAK_n    ( busak_n_unused   ),
        .A          ( bus_addr         ),
        .DInst      ( bus_rddata       ),
        .DI         ( q_di             ),
        .DO         ( bus_wrdata       ),
        .MC         ( mcycle           ),
        .TS         ( tstate           ),
        .IntCycle_n ( int_cycle_n      ),
        .IntE       ( int_e_unused     ),
        .Stop       ( stop_unused      ),
        .R800_mode  ( 1'b0             ),
        .out0       ( 1'b0             ),
        .REG        ( reg_unused       ),
        .DIRSet     ( 1'b0             ),
        .DIR        ( 212'b0           )
    );

    assign bus_t1_rd = tstate == 1 && !noread && !write;

    assign bus_wr_d = mcycle != 1 && (tstate == 1 || (tstate == 2 && bus_wait)) && write;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            bus_rd <= 0;
            bus_wr <= 0;
            q_di   <= 8'h00;

        end else if (clken) begin
            bus_rd <= 0;
            bus_wr <= 0;

            if (mcycle == 1) begin
                if (tstate == 1 || (tstate == 2 && bus_wait)) begin
                    bus_rd <= int_cycle_n;
                end

            end else begin
                if ((tstate == 1 || (tstate == 2 && bus_wait)) && !noread && !write) begin
                    bus_rd <= 1;
                end
                if (bus_wr_d) begin
                    bus_wr <= 1;
                end
            end

            if (tstate == 2 && !bus_wait) begin
                q_di <= bus_rddata;
            end
        end
    end

endmodule
