#!/bin/sh
set -e
rm -rf work/
vlib work
vmap work work
vlog /opt/Xilinx/14.7/ISE_DS/ISE/verilog/src/glbl.v
vlog tb_fmsynth.v ../audio/*.v
vsim -voptargs="+acc=npr" -L xilinx work.glbl work.tb_fmsynth -do "do wave_fmsynth.do; run 10 ms"
