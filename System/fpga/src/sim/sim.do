vlib work
vdel -all
vlib work
vmap work work
vlog glbl.v is61c5128as.v is61lv5128al.v tb.v ../*.v ../aqp_shared/*.v ../core_common/*.v ../core_common/video/*.v ../wrappers/*.v
vcom ../aqp_shared/t80/T80_Pack.vhd ../aqp_shared/t80/T80_MCode.vhd ../aqp_shared/t80/T80_ALU.vhd ../aqp_shared/t80/T80_Reg.vhd ../aqp_shared/t80/T80.vhd
vsim -L xilinx work.glbl work.tb
do wave.do
run 1 us
