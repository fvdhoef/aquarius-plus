vdel -all
vlib work
vmap work work
vlog glbl.v is61c5128as.v is61lv5128al.v tb.v ../*.v ../util/*.v ../wrappers/*.v ../cores/aquarius-plus/*.v ../cores/aquarius-plus/aqplus_common/*.v ../cores/aquarius-plus/aqplus_common/video/*.v ../cores/aquarius-plus/wrappers/*.v
vcom ../cores/aquarius-plus/t80/T80_Pack.vhd ../cores/aquarius-plus/t80/T80_MCode.vhd ../cores/aquarius-plus/t80/T80_ALU.vhd ../cores/aquarius-plus/t80/T80_Reg.vhd ../cores/aquarius-plus/t80/T80.vhd
vsim -L xilinx work.glbl work.tb
do wave.do
run 1 us
