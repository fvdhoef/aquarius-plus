onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_video/video/clk
add wave -noupdate /tb_video/video/reset
add wave -noupdate /tb_video/video/irq_vblank
add wave -noupdate /tb_video/video/vram_addr
add wave -noupdate /tb_video/video/vram_rddata
add wave -noupdate /tb_video/video/vram_wrdata
add wave -noupdate /tb_video/video/vram_wrsel
add wave -noupdate /tb_video/video/vram_wren
add wave -noupdate /tb_video/video/tram_addr
add wave -noupdate /tb_video/video/tram_rddata
add wave -noupdate /tb_video/video/tram_wrdata
add wave -noupdate /tb_video/video/tram_bytesel
add wave -noupdate /tb_video/video/tram_wren
add wave -noupdate /tb_video/video/chram_addr
add wave -noupdate /tb_video/video/chram_rddata
add wave -noupdate /tb_video/video/chram_wrdata
add wave -noupdate /tb_video/video/chram_wren
add wave -noupdate /tb_video/video/pal_addr
add wave -noupdate /tb_video/video/pal_rddata
add wave -noupdate /tb_video/video/pal_wrdata
add wave -noupdate /tb_video/video/pal_wren
add wave -noupdate -radix unsigned /tb_video/video/video_hpos
add wave -noupdate /tb_video/video/video_hlast
add wave -noupdate -radix unsigned /tb_video/video/video_vpos
add wave -noupdate /tb_video/video/video_vlast
add wave -noupdate /tb_video/video/video_r
add wave -noupdate /tb_video/video/video_g
add wave -noupdate /tb_video/video/video_b
add wave -noupdate -divider Internal
add wave -noupdate /tb_video/video/hblank
add wave -noupdate /tb_video/video/vblank
add wave -noupdate /tb_video/video/blank
add wave -noupdate /tb_video/video/vnext
add wave -noupdate /tb_video/video/vpos9
add wave -noupdate /tb_video/video/q_vblank
add wave -noupdate /tb_video/video/q_blank
add wave -noupdate /tb_video/video/q2_blank
add wave -noupdate /tb_video/video/rddata_sprattr
add wave -noupdate /tb_video/video/q_hpos
add wave -noupdate /tb_video/video/q2_hpos
add wave -noupdate /tb_video/video/q_vpage
add wave -noupdate /tb_video/video/vaddr
add wave -noupdate /tb_video/video/vdata
add wave -noupdate -radix unsigned /tb_video/video/q_line_addr
add wave -noupdate -radix unsigned /tb_video/video/q_pixel_addr
add wave -noupdate /tb_video/video/q_sub_pixel_cnt
add wave -noupdate /tb_video/video/q_sub_line_cnt
add wave -noupdate -divider Other
add wave -noupdate /tb_video/video/q_mode80
add wave -noupdate /tb_video/video/q_row_addr
add wave -noupdate /tb_video/video/q_char_addr
add wave -noupdate /tb_video/video/next_row
add wave -noupdate /tb_video/video/d_row_addr
add wave -noupdate /tb_video/video/border_char_addr
add wave -noupdate /tb_video/video/next_char
add wave -noupdate /tb_video/video/start_active
add wave -noupdate /tb_video/video/d_char_addr
add wave -noupdate /tb_video/video/textram_rddata
add wave -noupdate /tb_video/video/textram_color_text
add wave -noupdate /tb_video/video/text_data
add wave -noupdate /tb_video/video/color_data
add wave -noupdate /tb_video/video/q_color_data
add wave -noupdate /tb_video/video/charram_addr
add wave -noupdate /tb_video/video/charram_data
add wave -noupdate /tb_video/video/pixel_sel
add wave -noupdate /tb_video/video/char_pixel
add wave -noupdate /tb_video/video/text_colidx
add wave -noupdate /tb_video/video/pal_r
add wave -noupdate /tb_video/video/pal_g
add wave -noupdate /tb_video/video/pal_b
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {2678530204 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 281
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
WaveRestoreZoom {0 ps} {17850 us}
