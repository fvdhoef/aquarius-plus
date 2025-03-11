#!/bin/sh
set -e
rm -rf work/
vlib work
vmap work work
vlog /opt/Xilinx/14.7/ISE_DS/ISE/verilog/src/glbl.v
vlog is61c5128as.v tb_sram_ctrl.v ../mem/sram_ctrl.v
vsim -voptargs="+acc=npr" -L xilinx work.glbl work.tb_sram_ctrl -do "do wave_sram_ctrl.do; run 3 us"
