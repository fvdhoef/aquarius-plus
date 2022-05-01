module busif(
    input  wire        clk,
    input  wire        reset,

    input  wire [15:0] bus_a,
    input  wire  [7:0] bus_wrdata,
    output wire  [7:0] bus_rddata,
    output wire        bus_d_oe,
    input  wire        bus_rd_n,
    input  wire        bus_wr_n,
    input  wire        bus_mreq_n,
    input  wire        bus_iorq_n);

endmodule
