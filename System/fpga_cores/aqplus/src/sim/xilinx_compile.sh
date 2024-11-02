#!/bin/sh
set -e
rm -rf xilinx/
vlib xilinx
vlog -work xilinx /opt/Xilinx/14.7/ISE_DS/ISE/verilog/src/unisims/*.v
