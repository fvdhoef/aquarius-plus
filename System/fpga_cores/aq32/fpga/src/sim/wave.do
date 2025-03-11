onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb/top_inst/sysctrl/ebus_phi
add wave -noupdate /tb/top_inst/sysctrl/ebus_phi_clken
add wave -noupdate -expand -group aqp_top /tb/top_inst/sysclk
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_reset_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_phi
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/ebus_a
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/ebus_d
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_rd_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_wr_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_mreq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_iorq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_int_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_busreq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_busack_n
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/ebus_ba
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_ram_ce_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_cart_ce_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_ram_we_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/audio_l
add wave -noupdate -expand -group aqp_top /tb/top_inst/audio_r
add wave -noupdate -expand -group aqp_top /tb/top_inst/cassette_out
add wave -noupdate -expand -group aqp_top /tb/top_inst/cassette_in
add wave -noupdate -expand -group aqp_top /tb/top_inst/printer_out
add wave -noupdate -expand -group aqp_top /tb/top_inst/printer_in
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/exp
add wave -noupdate -expand -group aqp_top /tb/top_inst/has_z80
add wave -noupdate -expand -group aqp_top /tb/top_inst/hc1
add wave -noupdate -expand -group aqp_top /tb/top_inst/hc2
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/vga_r
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/vga_g
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/vga_b
add wave -noupdate -expand -group aqp_top /tb/top_inst/vga_hsync
add wave -noupdate -expand -group aqp_top /tb/top_inst/vga_vsync
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_tx
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rx
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rts
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_cts
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_ssel_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_sclk
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_mosi
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_miso
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_notify
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/spibm_a
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/spibm_wrdata
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_wrdata_en
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_en
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_rd_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_wr_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_mreq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_iorq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/spibm_busreq
add wave -noupdate -expand -group aqp_top /tb/top_inst/use_t80
add wave -noupdate -expand -group aqp_top /tb/top_inst/clk
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_clk
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_mode
add wave -noupdate -expand -group aqp_top /tb/top_inst/reset_req
add wave -noupdate -expand -group aqp_top /tb/top_inst/turbo
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_phi_clken
add wave -noupdate -expand -group aqp_top /tb/top_inst/reset
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_int_n_pushpull
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/ebus_d_out
add wave -noupdate -expand -group aqp_top /tb/top_inst/ebus_d_oe
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/ebus_d_in
add wave -noupdate -expand -group aqp_top /tb/top_inst/q_ebus_wr_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/q_ebus_rd_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/bus_read
add wave -noupdate -expand -group aqp_top /tb/top_inst/bus_write
add wave -noupdate -expand -group aqp_top /tb/top_inst/common_ebus_stb
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/esp_tx_data
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_tx_wr
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_tx_fifo_full
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rx_data
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rx_rd
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rx_empty
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rx_fifo_overflow
add wave -noupdate -expand -group aqp_top /tb/top_inst/esp_rx_framing_error
add wave -noupdate -expand -group aqp_top /tb/top_inst/hc1_in
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/hc1_out
add wave -noupdate -expand -group aqp_top /tb/top_inst/hc1_oe
add wave -noupdate -expand -group aqp_top /tb/top_inst/hc2_in
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/hc2_out
add wave -noupdate -expand -group aqp_top /tb/top_inst/hc2_oe
add wave -noupdate -expand -group aqp_top /tb/top_inst/spi_msg_end
add wave -noupdate -expand -group aqp_top /tb/top_inst/spi_cmd
add wave -noupdate -expand -group aqp_top /tb/top_inst/spi_rxdata
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_text_addr
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_text_wrdata
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_text_wr
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_font_addr
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_font_wrdata
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_font_wr
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_palette_addr
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_palette_wrdata
add wave -noupdate -expand -group aqp_top /tb/top_inst/ovl_palette_wr
add wave -noupdate -expand -group aqp_top -radix decimal /tb/top_inst/common_audio_l
add wave -noupdate -expand -group aqp_top -radix decimal /tb/top_inst/common_audio_r
add wave -noupdate -expand -group aqp_top /tb/top_inst/turbo_unlimited
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/video_r
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/video_g
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/video_b
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_de
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_hsync
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_vsync
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_newframe
add wave -noupdate -expand -group aqp_top /tb/top_inst/video_oddline
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/t80_addr
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/t80_dq_out
add wave -noupdate -expand -group aqp_top -radix hexadecimal /tb/top_inst/t80_dq_in
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_dq_oe
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_mreq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_iorq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_rd_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_wr_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_busrq_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_busak_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_int_n
add wave -noupdate -expand -group aqp_top /tb/top_inst/t80_nmi_n
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/clk
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/mode
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hpos
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/hsync
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/hblank
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/hlast
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/vpos
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/vsync
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/vblank
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/vnext
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/vnewframe
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/voddline
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/blank
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/q_mode
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/q_hcnt
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_blank
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_hsync1
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_hsync2
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_last
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/hcnt_done
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/q_vcnt
add wave -noupdate -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/vcnt
add wave -noupdate -group video_timing /tb/top_inst/common/video/video_timing/vcnt_done
add wave -noupdate -expand -group aqt80 -radix hexadecimal /tb/top_inst/aqt80/t80/IR
add wave -noupdate -expand -group aqt80 -radix hexadecimal /tb/top_inst/aqt80/t80/ACC
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/clk
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/reset
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/clken
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/phi
add wave -noupdate -expand -group aqt80 -radix hexadecimal /tb/top_inst/aqt80/addr
add wave -noupdate -expand -group aqt80 -radix hexadecimal /tb/top_inst/aqt80/dq_out
add wave -noupdate -expand -group aqt80 -radix hexadecimal /tb/top_inst/aqt80/dq_in
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/dq_oe
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/mreq_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/iorq_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/rd_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/wr_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/stb
add wave -noupdate -expand -group aqt80 /tb/top_inst/common_ebus_stb
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/busrq_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/busak_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/int_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/nmi_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/q_phi
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/phi_rising
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/phi_falling
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/q_mreq
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/q_read
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/MReq_Inhibit
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/Req_Inhibit
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/IORQ_t1
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/IORQ_t2
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/IORQ_int
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/IORQ_int_inhibit
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/WR_t2
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/t80_iorq
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/t80_noread
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/t80_write
add wave -noupdate -expand -group aqt80 -radix unsigned /tb/top_inst/aqt80/t80_mc
add wave -noupdate -expand -group aqt80 -radix unsigned /tb/top_inst/aqt80/t80_ts
add wave -noupdate -expand -group aqt80 -radix hexadecimal /tb/top_inst/aqt80/q_t80_di
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/t80_int_cycle_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/t80_m1_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/t80_rfsh_n
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/mreq_rw
add wave -noupdate -expand -group aqt80 /tb/top_inst/aqt80/iorq_rw
add wave -noupdate -expand -group SRAM -radix hexadecimal /tb/sram/A
add wave -noupdate -expand -group SRAM -radix hexadecimal /tb/sram/IO
add wave -noupdate -expand -group SRAM /tb/sram/CE_n
add wave -noupdate -expand -group SRAM /tb/sram/OE_n
add wave -noupdate -expand -group SRAM /tb/sram/WE_n
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1850760 ps} 0} {{Cursor 2} {639277345 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 411
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
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {13256250 ps}
bookmark add wave bookmark0 {{0 ps} {212100032 ps}} 0
