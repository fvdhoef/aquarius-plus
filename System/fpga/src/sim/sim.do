vdel -all
vlib work
vmap work work
vlog glbl.v tb.v ../*.v ../util/*.v ../wrappers/*.v ../cores/aquarius-plus/aqplus_common/*.v ../cores/aquarius-plus/aqplus_common/video/*.v ../cores/aquarius-plus/wrappers/*.v
vsim -L xilinx work.glbl work.tb
do wave.do
run 1 us
