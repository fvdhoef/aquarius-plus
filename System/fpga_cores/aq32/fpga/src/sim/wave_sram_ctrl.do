onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/clk
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/reset
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_addr
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_wrdata
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_bytesel
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_wren
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_strobe
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_wait
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/bus_rddata
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/sram_a
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/sram_ce_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/sram_oe_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/sram_we_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/sram_dq
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_state
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_state
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_sram_a
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_sram_a
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_sram_we_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_sram_we_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_sram_oe_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_sram_oe_n
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_bus_wait
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_bus_wait
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_bus_rddata
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_bus_rddata
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_dq_wrdata
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_dq_wrdata
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/d_dq_oe
add wave -noupdate -group {SRAM ctrl} /tb_sram_ctrl/sram_ctrl/q_dq_oe
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/clk
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/reset
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_addr
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_bytesel
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_wren
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_strobe
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_wait
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_rddata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_addr
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_wren
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_strobe
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_wait
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_rddata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_data_write
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_data_write
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_tag_write
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_tag_write
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_tag_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_tag_rddata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_tag_valid
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_tag_dirty
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/s_tag
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_tag_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/m_tag_rddata
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/ds_state
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/qs_state
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/dm_addr
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/qm_addr
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/dm_wren
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/qm_wren
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/dm_strobe
add wave -noupdate -expand -group {SRAM cache} /tb_sram_ctrl/sram_cache/qm_strobe
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {2273213 ps} 0}
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
WaveRestoreZoom {0 ps} {3150 ns}
