module clkctrl(
    input  wire clk_in,
    output wire clk_out,

    output wire vclk,
    input  wire video_mode
);

`ifdef __ICARUS__
    assign clk_out = clk_in;
    assign vclk = clk_in;
`else
    wire clk0;
    wire clk28;
    assign clk_out = clk28;

    // DCM to multiply the 14.31818MHz by 2 to 28.63636MHz
    wire clk2x;
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
        .CLK0(clk0),
        .CLK2X(clk2x),
        .LOCKED(),
        .CLKFB(clk0),
        .CLKIN(clk_in),
        .DSSEN(1'b0),
        .PSCLK(1'b0),
        .PSEN(1'b0),
        .PSINCDEC(1'b0), 
        .RST(1'b0)
    );
    BUFG bufg_28(.I(clk2x), .O(clk28));

    wire clk25;
    wire pllfb;

    // PLL to synthesize 25.175MHz from the 28.63636MHz clock from the DCM
    PLL_BASE #(
        .BANDWIDTH("OPTIMIZED"),
        .CLKFBOUT_MULT(29),
        .CLKFBOUT_PHASE(0.0),
        .CLKIN_PERIOD(34.921),
        .CLKOUT0_DIVIDE(33),
        .CLKOUT1_DIVIDE(33),
        .CLKOUT2_DIVIDE(33),
        .CLKOUT3_DIVIDE(33),
        .CLKOUT4_DIVIDE(33),
        .CLKOUT5_DIVIDE(33),
        .CLKOUT0_DUTY_CYCLE(0.5),
        .CLKOUT1_DUTY_CYCLE(0.5),
        .CLKOUT2_DUTY_CYCLE(0.5),
        .CLKOUT3_DUTY_CYCLE(0.5),
        .CLKOUT4_DUTY_CYCLE(0.5),
        .CLKOUT5_DUTY_CYCLE(0.5),
        .CLKOUT0_PHASE(0.0),
        .CLKOUT1_PHASE(0.0),
        .CLKOUT2_PHASE(0.0),
        .CLKOUT3_PHASE(0.0),
        .CLKOUT4_PHASE(0.0),
        .CLKOUT5_PHASE(0.0),
        .CLK_FEEDBACK("CLKFBOUT"),
        .COMPENSATION("SYSTEM_SYNCHRONOUS"),
        .DIVCLK_DIVIDE(1),
        .REF_JITTER(0.1),
        .RESET_ON_LOSS_OF_LOCK("FALSE")
    )
    pll(
        .CLKFBOUT(pllfb),
        .CLKOUT0(clk25),
        .CLKOUT1(),
        .CLKOUT2(),
        .CLKOUT3(),
        .CLKOUT4(),
        .CLKOUT5(),
        .LOCKED(),
        .CLKFBIN(pllfb),
        .CLKIN(clk28),
        .RST(1'b0)
    );

    // Clock buffer to switch without glitches between 28.63636MHz and 25.175MHz clock
    BUFGMUX #(
        .CLK_SEL_TYPE("SYNC")
    )
    clksel(
        .O(vclk),
        .I0(clk28),
        .I1(clk25),
        .S(video_mode)
    );

`endif

endmodule
