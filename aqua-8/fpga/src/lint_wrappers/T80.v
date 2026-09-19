`default_nettype none
`timescale 1 ns / 1 ps

module T80 #(
    parameter Mode   = 0,
    parameter IOWait = 0
) (
    input  wire         RESET_n,
    input  wire         CLK_n,
    input  wire         CEN,
    input  wire         WAIT_n,
    input  wire         INT_n,
    input  wire         NMI_n,
    input  wire         BUSRQ_n,
    output wire         M1_n,
    output wire         IORQ,
    output wire         NoRead,
    output wire         Write,
    output wire         RFSH_n,
    output wire         HALT_n,
    output wire         BUSAK_n,
    output wire  [15:0] A,
    input  wire   [7:0] DInst,
    input  wire   [7:0] DI,
    output wire   [7:0] DO,
    output wire   [2:0] MC,
    output wire   [2:0] TS,
    output wire         IntCycle_n,
    output wire         IntE,
    output wire         Stop,
    input  wire         R800_mode,
    input  wire         out0,
    output wire [211:0] REG,
    input  wire         DIRSet,
    input  wire [211:0] DIR
);

endmodule
