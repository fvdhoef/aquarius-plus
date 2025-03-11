#!/bin/sh
set -e
rm -rf work/
vlib work
vmap work work
vlog /opt/Xilinx/14.7/ISE_DS/ISE/verilog/src/glbl.v
vlog is61c5128as.v is61lv5128al.v tb.v ../cpu/*.v ../mem/*.v ../*.v ../aqp_shared/*.v ../core_common/*.v ../core_common/video/*.v ../wrappers/*.v
vsim -voptargs="+acc=npr" -L xilinx work.glbl work.tb -do "do wave.do; run 100 us"
