module clkctrl(
    input  wire clk_in,
    output wire clk_out
);

`ifdef __ICARUS__
    assign clk_out = clk_in;
`else
    DCM_SP #(
        .CLKDV_DIVIDE(2.0),
        .CLKFX_DIVIDE(1),
        .CLKFX_MULTIPLY(4),
        .CLKIN_DIVIDE_BY_2("FALSE"),
        .CLKIN_PERIOD(69.841274),
        .CLKOUT_PHASE_SHIFT("NONE"),
        .CLK_FEEDBACK("1X"),
        .DESKEW_ADJUST("SYSTEM_SYNCHRONOUS"), 
        .DFS_FREQUENCY_MODE("LOW"),
        .DLL_FREQUENCY_MODE("LOW"),
        .DSS_MODE("NONE"),
        .DUTY_CYCLE_CORRECTION("TRUE"),
        .FACTORY_JF(16'hc080),
        .PHASE_SHIFT(0),
        .STARTUP_WAIT("FALSE")
    )
    dcm(
        .CLK0(clk_out),
        .CLK2X(),
        .LOCKED(),
        .CLKFB(clk_out),
        .CLKIN(clk_in),
        .DSSEN(1'b0),
        .PSCLK(1'b0),
        .PSEN(1'b0),
        .PSINCDEC(1'b0), 
        .RST(1'b0)
    );
`endif

endmodule
