onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_sram_ctrl/sram_ctrl/clk
add wave -noupdate /tb_sram_ctrl/sram_ctrl/reset
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_addr
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_wrdata
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_bytesel
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_wren
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_strobe
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_wait
add wave -noupdate /tb_sram_ctrl/sram_ctrl/bus_rddata
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_a
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_ce_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_oe_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_we_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_dq
add wave -noupdate /tb_sram_ctrl/sram_ctrl/d_state
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_state
add wave -noupdate /tb_sram_ctrl/sram_ctrl/d_sram_a
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_sram_a
add wave -noupdate /tb_sram_ctrl/sram_ctrl/d_sram_we_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_sram_we_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/d_sram_oe_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_sram_oe_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/d_bus_wait
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_bus_wait
add wave -noupdate /tb_sram_ctrl/sram_ctrl/d_bus_rddata
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_bus_rddata
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {424340 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 275
configure wave -valuecolwidth 88
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {0 ps} {884736 ps}
