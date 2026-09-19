onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/clk_25_175
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/clk_28_63636
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/core_type
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/core_flags
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/core_version
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/core_name
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/spi_msg_end
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/spi_cmd
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/spi_rxdata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/spi_txdata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/spi_txdata_valid
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_hpos
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_hlast
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_vpos
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_vlast
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_r
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_g
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_b
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/audio_l
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/audio_r
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/esp_tx
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/esp_rx
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/esp_rts
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/esp_cts
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/reset_req
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/q_reset_cnt
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/reset
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/palette_idx
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/palette_wrdata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/palette_wren
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/palette_rddata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/video_mode
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/base_addr
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/next_line_addr_wrdata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/next_line_addr_wren
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/next_line_addr_rddata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/mem_addr
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/mem_strobe
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/mem_wait
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/mem_rddata
add wave -noupdate -group {FPGA core} /tb/top_inst/fpga_core/mem_rddata_valid
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/clk
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/reset
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_addr
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_wrdata
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_wren
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_strobe
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_wait
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_rddata
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/mem_rddata_valid
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/sram_a
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/sram_ce_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/sram_oe_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/sram_we_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/sram_dq
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_state
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_state
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_sram_a
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_sram_a
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_sram_we_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_sram_we_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_sram_oe_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_sram_oe_n
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_mem_wait
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_mem_wait
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_mem_rddata
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_mem_rddata
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_mem_rddata_valid
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_mem_rddata_valid
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_dq_wrdata
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_dq_wrdata
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/d_dq_oe
add wave -noupdate -expand -group {SRAM controller} /tb/top_inst/fpga_core/sram_ctrl/q_dq_oe
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {64383591 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 427
configure wave -valuecolwidth 104
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
WaveRestoreZoom {0 ps} {104445573 ps}
