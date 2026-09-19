onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_sram_ctrl/sram_ctrl/clk
add wave -noupdate /tb_sram_ctrl/sram_ctrl/reset
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_addr
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_wrdata
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_wren
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_strobe
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_wait
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_rddata
add wave -noupdate /tb_sram_ctrl/sram_ctrl/mem_rddata_valid
add wave -noupdate -divider SRAM
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_a
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_ce_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_oe_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_we_n
add wave -noupdate /tb_sram_ctrl/sram_ctrl/sram_dq
add wave -noupdate -divider Internal
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_state
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_wrdata
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_wren
add wave -noupdate /tb_sram_ctrl/sram_ctrl/q_read
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {540000 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 342
configure wave -valuecolwidth 100
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
WaveRestoreZoom {0 ps} {1050 ns}
