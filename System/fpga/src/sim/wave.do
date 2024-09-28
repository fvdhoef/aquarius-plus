onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb/top_inst/sysclk
add wave -noupdate /tb/top_inst/ebus_reset_n
add wave -noupdate /tb/top_inst/ebus_phi
add wave -noupdate -radix hexadecimal /tb/top_inst/ebus_a
add wave -noupdate /tb/top_inst/ebus_d
add wave -noupdate /tb/top_inst/ebus_rd_n
add wave -noupdate /tb/top_inst/ebus_wr_n
add wave -noupdate /tb/top_inst/ebus_mreq_n
add wave -noupdate /tb/top_inst/ebus_iorq_n
add wave -noupdate /tb/top_inst/ebus_int_n
add wave -noupdate /tb/top_inst/ebus_busreq_n
add wave -noupdate /tb/top_inst/ebus_busack_n
add wave -noupdate -radix hexadecimal /tb/top_inst/ebus_ba
add wave -noupdate /tb/top_inst/ebus_ram_ce_n
add wave -noupdate /tb/top_inst/ebus_cart_ce_n
add wave -noupdate /tb/top_inst/ebus_ram_we_n
add wave -noupdate /tb/top_inst/audio_l
add wave -noupdate /tb/top_inst/audio_r
add wave -noupdate /tb/top_inst/cassette_out
add wave -noupdate /tb/top_inst/cassette_in
add wave -noupdate /tb/top_inst/printer_out
add wave -noupdate /tb/top_inst/printer_in
add wave -noupdate -radix hexadecimal /tb/top_inst/exp
add wave -noupdate /tb/top_inst/hc1
add wave -noupdate /tb/top_inst/hc2
add wave -noupdate -radix hexadecimal /tb/top_inst/vga_r
add wave -noupdate -radix hexadecimal /tb/top_inst/vga_g
add wave -noupdate -radix hexadecimal /tb/top_inst/vga_b
add wave -noupdate /tb/top_inst/vga_hsync
add wave -noupdate /tb/top_inst/vga_vsync
add wave -noupdate /tb/top_inst/esp_tx
add wave -noupdate /tb/top_inst/esp_rx
add wave -noupdate /tb/top_inst/esp_rts
add wave -noupdate /tb/top_inst/esp_cts
add wave -noupdate /tb/top_inst/esp_ssel_n
add wave -noupdate /tb/top_inst/esp_sclk
add wave -noupdate /tb/top_inst/esp_mosi
add wave -noupdate /tb/top_inst/esp_miso
add wave -noupdate /tb/top_inst/esp_notify
add wave -noupdate -radix hexadecimal /tb/top_inst/spibm_a
add wave -noupdate -radix hexadecimal /tb/top_inst/spibm_wrdata
add wave -noupdate /tb/top_inst/spibm_wrdata_en
add wave -noupdate /tb/top_inst/spibm_en
add wave -noupdate /tb/top_inst/spibm_rd_n
add wave -noupdate /tb/top_inst/spibm_wr_n
add wave -noupdate /tb/top_inst/spibm_mreq_n
add wave -noupdate /tb/top_inst/spibm_iorq_n
add wave -noupdate /tb/top_inst/spibm_busreq
add wave -noupdate /tb/top_inst/clk
add wave -noupdate /tb/top_inst/vclk
add wave -noupdate /tb/top_inst/video_mode
add wave -noupdate /tb/top_inst/reset_req
add wave -noupdate /tb/top_inst/turbo
add wave -noupdate /tb/top_inst/reset
add wave -noupdate /tb/top_inst/ebus_int_n_pushpull
add wave -noupdate -radix hexadecimal /tb/top_inst/ebus_d_out
add wave -noupdate /tb/top_inst/ebus_d_oe
add wave -noupdate -radix hexadecimal /tb/top_inst/ebus_d_in
add wave -noupdate /tb/top_inst/q_ebus_wr_n
add wave -noupdate /tb/top_inst/q_ebus_rd_n
add wave -noupdate /tb/top_inst/bus_read
add wave -noupdate /tb/top_inst/bus_write
add wave -noupdate /tb/top_inst/common_ebus_stb
add wave -noupdate -radix hexadecimal /tb/top_inst/esp_tx_data
add wave -noupdate /tb/top_inst/esp_rx_data
add wave -noupdate /tb/top_inst/esp_txvalid
add wave -noupdate /tb/top_inst/esp_txbusy
add wave -noupdate /tb/top_inst/esp_rxfifo_not_empty
add wave -noupdate /tb/top_inst/esp_rxfifo_read
add wave -noupdate /tb/top_inst/esp_rxfifo_overflow
add wave -noupdate /tb/top_inst/esp_rx_framing_error
add wave -noupdate /tb/top_inst/esp_rx_break
add wave -noupdate /tb/top_inst/hc1_in
add wave -noupdate -radix hexadecimal /tb/top_inst/hc1_out
add wave -noupdate /tb/top_inst/hc1_oe
add wave -noupdate /tb/top_inst/hc2_in
add wave -noupdate -radix hexadecimal /tb/top_inst/hc2_out
add wave -noupdate /tb/top_inst/hc2_oe
add wave -noupdate /tb/top_inst/spi_msg_end
add wave -noupdate /tb/top_inst/spi_cmd
add wave -noupdate /tb/top_inst/spi_rxdata
add wave -noupdate -radix decimal /tb/top_inst/common_audio_l
add wave -noupdate -radix decimal /tb/top_inst/common_audio_r
add wave -noupdate /tb/top_inst/video_de
add wave -noupdate /tb/top_inst/video_newframe
add wave -noupdate /tb/top_inst/video_oddline
add wave -noupdate /tb/top_inst/turbo_unlimited
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {13129920 ps} 0} {{Cursor 2} {694193240 ps} 0}
quietly wave cursor active 2
configure wave -namecolwidth 253
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
WaveRestoreZoom {0 ps} {1156050 ns}
