
`timescale 1 ps / 1 ps 

  module PLL_BASE (
    CLKFBOUT,
    CLKOUT0,
    CLKOUT1,
    CLKOUT2,
    CLKOUT3,
    CLKOUT4,
    CLKOUT5,
    LOCKED,
    CLKFBIN,
    CLKIN,
    RST
  );

  parameter BANDWIDTH = "OPTIMIZED";
  parameter integer CLKFBOUT_MULT = 1;
  parameter real CLKFBOUT_PHASE = 0.0;
  parameter real CLKIN_PERIOD = 0.000;
  parameter integer CLKOUT0_DIVIDE = 1;
  parameter real CLKOUT0_DUTY_CYCLE = 0.5;
  parameter real CLKOUT0_PHASE = 0.0;
  parameter integer CLKOUT1_DIVIDE = 1;
  parameter real CLKOUT1_DUTY_CYCLE = 0.5;
  parameter real CLKOUT1_PHASE = 0.0;
  parameter integer CLKOUT2_DIVIDE = 1;
  parameter real CLKOUT2_DUTY_CYCLE = 0.5;
  parameter real CLKOUT2_PHASE = 0.0;
  parameter integer CLKOUT3_DIVIDE = 1;
  parameter real CLKOUT3_DUTY_CYCLE = 0.5;
  parameter real CLKOUT3_PHASE = 0.0;
  parameter integer CLKOUT4_DIVIDE = 1;
  parameter real CLKOUT4_DUTY_CYCLE = 0.5;
  parameter real CLKOUT4_PHASE = 0.0;
  parameter integer CLKOUT5_DIVIDE = 1;
  parameter real CLKOUT5_DUTY_CYCLE = 0.5;
  parameter real CLKOUT5_PHASE = 0.0;
  parameter CLK_FEEDBACK = "CLKFBOUT";
  parameter COMPENSATION = "SYSTEM_SYNCHRONOUS";
  parameter integer DIVCLK_DIVIDE = 1;
  parameter real REF_JITTER = 0.100;
  parameter RESET_ON_LOSS_OF_LOCK = "FALSE";

  output CLKFBOUT;
  output CLKOUT0;
  output CLKOUT1;
  output CLKOUT2;
  output CLKOUT3;
  output CLKOUT4;
  output CLKOUT5;
  output LOCKED;
  
  input CLKFBIN;
  input CLKIN;
  input RST;

endmodule
