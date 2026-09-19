#!/bin/sh
set -e
rm -rf work/
vlib work
vmap work work
vlog /opt/Xilinx/14.7/ISE_DS/ISE/verilog/src/glbl.v
vlog tb_video.v ../wrappers/*.v ../video.v
vsim -voptargs="+acc=npr" -L xilinx work.glbl work.tb_video -do "do wave_video.do; run 17 ms"
