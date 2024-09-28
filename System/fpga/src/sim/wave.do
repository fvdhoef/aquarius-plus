onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -group aqp_top /tb/top_inst/sysclk
add wave -noupdate -group aqp_top /tb/top_inst/ebus_reset_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_phi
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/ebus_a
add wave -noupdate -group aqp_top /tb/top_inst/ebus_d
add wave -noupdate -group aqp_top /tb/top_inst/ebus_rd_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_wr_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_mreq_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_iorq_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_int_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_busreq_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_busack_n
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/ebus_ba
add wave -noupdate -group aqp_top /tb/top_inst/ebus_ram_ce_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_cart_ce_n
add wave -noupdate -group aqp_top /tb/top_inst/ebus_ram_we_n
add wave -noupdate -group aqp_top /tb/top_inst/audio_l
add wave -noupdate -group aqp_top /tb/top_inst/audio_r
add wave -noupdate -group aqp_top /tb/top_inst/cassette_out
add wave -noupdate -group aqp_top /tb/top_inst/cassette_in
add wave -noupdate -group aqp_top /tb/top_inst/printer_out
add wave -noupdate -group aqp_top /tb/top_inst/printer_in
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/exp
add wave -noupdate -group aqp_top /tb/top_inst/hc1
add wave -noupdate -group aqp_top /tb/top_inst/hc2
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/vga_r
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/vga_g
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/vga_b
add wave -noupdate -group aqp_top /tb/top_inst/vga_hsync
add wave -noupdate -group aqp_top /tb/top_inst/vga_vsync
add wave -noupdate -group aqp_top /tb/top_inst/esp_tx
add wave -noupdate -group aqp_top /tb/top_inst/esp_rx
add wave -noupdate -group aqp_top /tb/top_inst/esp_rts
add wave -noupdate -group aqp_top /tb/top_inst/esp_cts
add wave -noupdate -group aqp_top /tb/top_inst/esp_ssel_n
add wave -noupdate -group aqp_top /tb/top_inst/esp_sclk
add wave -noupdate -group aqp_top /tb/top_inst/esp_mosi
add wave -noupdate -group aqp_top /tb/top_inst/esp_miso
add wave -noupdate -group aqp_top /tb/top_inst/esp_notify
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/spibm_a
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/spibm_wrdata
add wave -noupdate -group aqp_top /tb/top_inst/spibm_wrdata_en
add wave -noupdate -group aqp_top /tb/top_inst/spibm_en
add wave -noupdate -group aqp_top /tb/top_inst/spibm_rd_n
add wave -noupdate -group aqp_top /tb/top_inst/spibm_wr_n
add wave -noupdate -group aqp_top /tb/top_inst/spibm_mreq_n
add wave -noupdate -group aqp_top /tb/top_inst/spibm_iorq_n
add wave -noupdate -group aqp_top /tb/top_inst/spibm_busreq
add wave -noupdate -group aqp_top /tb/top_inst/clk
add wave -noupdate -group aqp_top /tb/top_inst/video_mode
add wave -noupdate -group aqp_top /tb/top_inst/reset_req
add wave -noupdate -group aqp_top /tb/top_inst/turbo
add wave -noupdate -group aqp_top /tb/top_inst/reset
add wave -noupdate -group aqp_top /tb/top_inst/ebus_int_n_pushpull
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/ebus_d_out
add wave -noupdate -group aqp_top /tb/top_inst/ebus_d_oe
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/ebus_d_in
add wave -noupdate -group aqp_top /tb/top_inst/q_ebus_wr_n
add wave -noupdate -group aqp_top /tb/top_inst/q_ebus_rd_n
add wave -noupdate -group aqp_top /tb/top_inst/bus_read
add wave -noupdate -group aqp_top /tb/top_inst/bus_write
add wave -noupdate -group aqp_top /tb/top_inst/common_ebus_stb
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/esp_tx_data
add wave -noupdate -group aqp_top /tb/top_inst/esp_rx_data
add wave -noupdate -group aqp_top /tb/top_inst/esp_txvalid
add wave -noupdate -group aqp_top /tb/top_inst/esp_txbusy
add wave -noupdate -group aqp_top /tb/top_inst/esp_rxfifo_not_empty
add wave -noupdate -group aqp_top /tb/top_inst/esp_rxfifo_read
add wave -noupdate -group aqp_top /tb/top_inst/esp_rxfifo_overflow
add wave -noupdate -group aqp_top /tb/top_inst/esp_rx_framing_error
add wave -noupdate -group aqp_top /tb/top_inst/esp_rx_break
add wave -noupdate -group aqp_top /tb/top_inst/hc1_in
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/hc1_out
add wave -noupdate -group aqp_top /tb/top_inst/hc1_oe
add wave -noupdate -group aqp_top /tb/top_inst/hc2_in
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/hc2_out
add wave -noupdate -group aqp_top /tb/top_inst/hc2_oe
add wave -noupdate -group aqp_top /tb/top_inst/spi_msg_end
add wave -noupdate -group aqp_top /tb/top_inst/spi_cmd
add wave -noupdate -group aqp_top /tb/top_inst/spi_rxdata
add wave -noupdate -group aqp_top /tb/top_inst/ovl_text_addr
add wave -noupdate -group aqp_top /tb/top_inst/ovl_text_wrdata
add wave -noupdate -group aqp_top /tb/top_inst/ovl_text_wr
add wave -noupdate -group aqp_top /tb/top_inst/ovl_font_addr
add wave -noupdate -group aqp_top /tb/top_inst/ovl_font_wrdata
add wave -noupdate -group aqp_top /tb/top_inst/ovl_font_wr
add wave -noupdate -group aqp_top /tb/top_inst/ovl_palette_addr
add wave -noupdate -group aqp_top /tb/top_inst/ovl_palette_wrdata
add wave -noupdate -group aqp_top /tb/top_inst/ovl_palette_wr
add wave -noupdate -group aqp_top -radix decimal /tb/top_inst/common_audio_l
add wave -noupdate -group aqp_top -radix decimal /tb/top_inst/common_audio_r
add wave -noupdate -group aqp_top /tb/top_inst/turbo_unlimited
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/video_r
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/video_g
add wave -noupdate -group aqp_top -radix hexadecimal /tb/top_inst/video_b
add wave -noupdate -group aqp_top /tb/top_inst/video_de
add wave -noupdate -group aqp_top /tb/top_inst/video_hsync
add wave -noupdate -group aqp_top /tb/top_inst/video_vsync
add wave -noupdate -group aqp_top /tb/top_inst/video_newframe
add wave -noupdate -group aqp_top /tb/top_inst/video_oddline
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_clk
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/video_r
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/video_g
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/video_b
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_de
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_hsync
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_vsync
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_newframe
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_oddline
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/video_mode
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_clk
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/ovl_text_addr
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/ovl_text_wrdata
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_text_wr
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/ovl_font_addr
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/ovl_font_wrdata
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_font_wr
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/ovl_palette_addr
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/ovl_palette_wrdata
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_palette_wr
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/vga_r
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/vga_g
add wave -noupdate -expand -group aqp_display -radix hexadecimal /tb/top_inst/display/vga_b
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/vga_hsync
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/vga_vsync
add wave -noupdate -expand -group aqp_display -radix unsigned /tb/top_inst/display/q_xcnt
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/xcnt_last
add wave -noupdate -expand -group aqp_display -radix unsigned /tb/top_inst/display/q_ycnt
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/vactive
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_r
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_g
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_b
add wave -noupdate -expand -group aqp_display /tb/top_inst/display/ovl_a
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/clk
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/xcnt
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/xcnt_last
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/vactive
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_r
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_g
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_b
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_a
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_clk
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_text_addr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_text_wrdata
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_text_wr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_font_addr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_font_wrdata
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_font_wr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_palette_addr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_palette_wrdata
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_palette_wr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_hactive
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_active
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_subpixel
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/pixel_next
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_subline
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/line_next
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_pixelsel
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q2_pixelsel
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q3_pixelsel
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q4_pixelsel
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_font_line
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/font_line_next
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_text_addr_line
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_text_addr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/text_data
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/font_data
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/font_addr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_text_attr
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/ovl_font_rddata
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/font_pixel
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/color_idx
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/color
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q_ovl_active
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q2_ovl_active
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q3_ovl_active
add wave -noupdate -group aqp_overlay /tb/top_inst/display/overlay/q4_ovl_active
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/clk
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/mode
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hpos
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/hsync
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/hblank
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/hlast
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/vpos
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/vsync
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/vblank
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/vnext
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/vnewframe
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/voddline
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/blank
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/q_mode
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/q_hcnt
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_blank
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_hsync1
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_hsync2
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/hcnt_last
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/hcnt_done
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/q_vcnt
add wave -noupdate -expand -group video_timing -radix unsigned /tb/top_inst/common/video/video_timing/vcnt
add wave -noupdate -expand -group video_timing /tb/top_inst/common/video/video_timing/vcnt_done
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {27536227 ps} 0} {{Cursor 2} {639277345 ps} 0}
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
WaveRestoreZoom {0 ps} {104545183 ps}
