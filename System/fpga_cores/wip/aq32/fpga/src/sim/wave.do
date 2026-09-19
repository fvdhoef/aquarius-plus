onerror {resume}
quietly virtual signal -install /tb/top_inst { (context /tb/top_inst )&{ebus_ba , ebus_a[13:0] }} sram_addr
quietly virtual function -install /tb/top_inst/video/gfx -env /tb/top_inst/video/gfx { &{/tb/top_inst/video/gfx/clk, /tb/top_inst/video/gfx/reset, /tb/top_inst/video/gfx/tilemode, /tb/top_inst/video/gfx/sprites_enable, /tb/top_inst/video/gfx/scroll_x, /tb/top_inst/video/gfx/scroll_y, /tb/top_inst/video/gfx/spr_sel, /tb/top_inst/video/gfx/spr_x, /tb/top_inst/video/gfx/spr_y, /tb/top_inst/video/gfx/spr_idx, /tb/top_inst/video/gfx/spr_enable, /tb/top_inst/video/gfx/spr_priority, /tb/top_inst/video/gfx/spr_palette, /tb/top_inst/video/gfx/spr_h16, /tb/top_inst/video/gfx/spr_vflip, /tb/top_inst/video/gfx/spr_hflip, /tb/top_inst/video/gfx/vaddr, /tb/top_inst/video/gfx/vdata, /tb/top_inst/video/gfx/reg_line, /tb/top_inst/video/gfx/start, /tb/top_inst/video/gfx/linebuf_rdidx, /tb/top_inst/video/gfx/linebuf_data, /tb/top_inst/video/gfx/d_spr_sel, /tb/top_inst/video/gfx/q_spr_sel, /tb/top_inst/video/gfx/wridx, /tb/top_inst/video/gfx/wrdata, /tb/top_inst/video/gfx/wren, /tb/top_inst/video/gfx/d_linesel, /tb/top_inst/video/gfx/q_linesel, /tb/top_inst/video/gfx/rddata, /tb/top_inst/video/gfx/d_col, /tb/top_inst/video/gfx/q_col, /tb/top_inst/video/gfx/d_col_cnt, /tb/top_inst/video/gfx/q_col_cnt, /tb/top_inst/video/gfx/d_vaddr, /tb/top_inst/video/gfx/q_vaddr, /tb/top_inst/video/gfx/d_state, /tb/top_inst/video/gfx/q_state, /tb/top_inst/video/gfx/d_nxtstate, /tb/top_inst/video/gfx/q_nxtstate, /tb/top_inst/video/gfx/d_map_entry, /tb/top_inst/video/gfx/q_map_entry, /tb/top_inst/video/gfx/d_blankout, /tb/top_inst/video/gfx/q_blankout, /tb/top_inst/video/gfx/d_busy, /tb/top_inst/video/gfx/q_busy, /tb/top_inst/video/gfx/line_idx, /tb/top_inst/video/gfx/tline, /tb/top_inst/video/gfx/row, /tb/top_inst/video/gfx/map_entry, /tb/top_inst/video/gfx/tile_idx, /tb/top_inst/video/gfx/tile_hflip, /tb/top_inst/video/gfx/tile_vflip, /tb/top_inst/video/gfx/tile_palette, /tb/top_inst/video/gfx/tile_priority, /tb/top_inst/video/gfx/vdata2, /tb/top_inst/video/gfx/spr_height, /tb/top_inst/video/gfx/ydiff, /tb/top_inst/video/gfx/spr_on_line, /tb/top_inst/video/gfx/spr_line, /tb/top_inst/video/gfx/d_render_idx, /tb/top_inst/video/gfx/q_render_idx, /tb/top_inst/video/gfx/d_render_data, /tb/top_inst/video/gfx/q_render_data, /tb/top_inst/video/gfx/d_render_is_sprite, /tb/top_inst/video/gfx/q_render_is_sprite, /tb/top_inst/video/gfx/d_render_hflip, /tb/top_inst/video/gfx/q_render_hflip, /tb/top_inst/video/gfx/d_render_palette, /tb/top_inst/video/gfx/q_render_palette, /tb/top_inst/video/gfx/d_render_priority, /tb/top_inst/video/gfx/q_render_priority, /tb/top_inst/video/gfx/render_start, /tb/top_inst/video/gfx/render_last_pixel, /tb/top_inst/video/gfx/render_busy }} Gfx
quietly WaveActivateNextPane {} 0
add wave -noupdate -group Video /tb/top_inst/video/clk
add wave -noupdate -group Video /tb/top_inst/video/reset
add wave -noupdate -group Video /tb/top_inst/video/vclk
add wave -noupdate -group Video /tb/top_inst/video/reg_text_mode80
add wave -noupdate -group Video /tb/top_inst/video/reg_text_priority
add wave -noupdate -group Video /tb/top_inst/video/reg_sprites_enable
add wave -noupdate -group Video /tb/top_inst/video/reg_gfx_tilemode
add wave -noupdate -group Video /tb/top_inst/video/reg_text_enable
add wave -noupdate -group Video /tb/top_inst/video/reg_scroll_x
add wave -noupdate -group Video /tb/top_inst/video/reg_scroll_y
add wave -noupdate -group Video /tb/top_inst/video/reg_line
add wave -noupdate -group Video /tb/top_inst/video/reg_irqline
add wave -noupdate -group Video /tb/top_inst/video/irq_line
add wave -noupdate -group Video /tb/top_inst/video/irq_vblank
add wave -noupdate -group Video /tb/top_inst/video/tram_addr
add wave -noupdate -group Video /tb/top_inst/video/tram_rddata
add wave -noupdate -group Video /tb/top_inst/video/tram_wrdata
add wave -noupdate -group Video /tb/top_inst/video/tram_bytesel
add wave -noupdate -group Video /tb/top_inst/video/tram_wren
add wave -noupdate -group Video /tb/top_inst/video/chram_addr
add wave -noupdate -group Video /tb/top_inst/video/chram_rddata
add wave -noupdate -group Video /tb/top_inst/video/chram_wrdata
add wave -noupdate -group Video /tb/top_inst/video/chram_wren
add wave -noupdate -group Video /tb/top_inst/video/pal_addr
add wave -noupdate -group Video /tb/top_inst/video/pal_rddata
add wave -noupdate -group Video /tb/top_inst/video/pal_wrdata
add wave -noupdate -group Video /tb/top_inst/video/pal_wren
add wave -noupdate -group Video /tb/top_inst/video/vram_addr
add wave -noupdate -group Video /tb/top_inst/video/vram_rddata
add wave -noupdate -group Video /tb/top_inst/video/vram_wrdata
add wave -noupdate -group Video /tb/top_inst/video/vram_bytesel
add wave -noupdate -group Video /tb/top_inst/video/vram_wren
add wave -noupdate -group Video /tb/top_inst/video/video_r
add wave -noupdate -group Video /tb/top_inst/video/video_g
add wave -noupdate -group Video /tb/top_inst/video/video_b
add wave -noupdate -group Video /tb/top_inst/video/video_de
add wave -noupdate -group Video /tb/top_inst/video/video_hsync
add wave -noupdate -group Video /tb/top_inst/video/video_vsync
add wave -noupdate -group Video /tb/top_inst/video/video_newframe
add wave -noupdate -group Video /tb/top_inst/video/video_oddline
add wave -noupdate -group Video /tb/top_inst/video/vclk_reset
add wave -noupdate -group Video /tb/top_inst/video/vpos
add wave -noupdate -group Video /tb/top_inst/video/vblank
add wave -noupdate -group Video /tb/top_inst/video/q_vblank
add wave -noupdate -group Video /tb/top_inst/video/rddata_sprattr
add wave -noupdate -group Video /tb/top_inst/video/irqline_match
add wave -noupdate -group Video /tb/top_inst/video/q_irqline_match
add wave -noupdate -group Video /tb/top_inst/video/hpos
add wave -noupdate -group Video /tb/top_inst/video/hsync
add wave -noupdate -group Video /tb/top_inst/video/hblank
add wave -noupdate -group Video /tb/top_inst/video/hlast
add wave -noupdate -group Video /tb/top_inst/video/vpos10
add wave -noupdate -group Video /tb/top_inst/video/vsync
add wave -noupdate -group Video /tb/top_inst/video/vnext
add wave -noupdate -group Video /tb/top_inst/video/blank
add wave -noupdate -group Video /tb/top_inst/video/hborder
add wave -noupdate -group Video /tb/top_inst/video/vborder
add wave -noupdate -group Video /tb/top_inst/video/q_hpos
add wave -noupdate -group Video /tb/top_inst/video/q2_hpos
add wave -noupdate -group Video /tb/top_inst/video/q_blank
add wave -noupdate -group Video /tb/top_inst/video/q_hsync
add wave -noupdate -group Video /tb/top_inst/video/q_vsync
add wave -noupdate -group Video /tb/top_inst/video/q2_blank
add wave -noupdate -group Video /tb/top_inst/video/q2_hsync
add wave -noupdate -group Video /tb/top_inst/video/q2_vsync
add wave -noupdate -group Video /tb/top_inst/video/q_mode80
add wave -noupdate -group Video /tb/top_inst/video/q_row_addr
add wave -noupdate -group Video /tb/top_inst/video/q_char_addr
add wave -noupdate -group Video /tb/top_inst/video/next_row
add wave -noupdate -group Video /tb/top_inst/video/d_row_addr
add wave -noupdate -group Video /tb/top_inst/video/border_char_addr
add wave -noupdate -group Video /tb/top_inst/video/next_char
add wave -noupdate -group Video /tb/top_inst/video/border
add wave -noupdate -group Video /tb/top_inst/video/q_border
add wave -noupdate -group Video /tb/top_inst/video/start_active
add wave -noupdate -group Video /tb/top_inst/video/d_char_addr
add wave -noupdate -group Video /tb/top_inst/video/textram_rddata
add wave -noupdate -group Video /tb/top_inst/video/text_data
add wave -noupdate -group Video /tb/top_inst/video/color_data
add wave -noupdate -group Video /tb/top_inst/video/q_color_data
add wave -noupdate -group Video /tb/top_inst/video/charram_addr
add wave -noupdate -group Video /tb/top_inst/video/charram_data
add wave -noupdate -group Video /tb/top_inst/video/pixel_sel
add wave -noupdate -group Video /tb/top_inst/video/char_pixel
add wave -noupdate -group Video /tb/top_inst/video/text_colidx
add wave -noupdate -group Video /tb/top_inst/video/spr_sel
add wave -noupdate -group Video /tb/top_inst/video/spr_x
add wave -noupdate -group Video /tb/top_inst/video/spr_y
add wave -noupdate -group Video /tb/top_inst/video/spr_idx
add wave -noupdate -group Video /tb/top_inst/video/spr_enable
add wave -noupdate -group Video /tb/top_inst/video/spr_priority
add wave -noupdate -group Video /tb/top_inst/video/spr_palette
add wave -noupdate -group Video /tb/top_inst/video/spr_h16
add wave -noupdate -group Video /tb/top_inst/video/spr_vflip
add wave -noupdate -group Video /tb/top_inst/video/spr_hflip
add wave -noupdate -group Video /tb/top_inst/video/vram_addr2
add wave -noupdate -group Video /tb/top_inst/video/vram_rddata2
add wave -noupdate -group Video /tb/top_inst/video/linebuf_data
add wave -noupdate -group Video /tb/top_inst/video/q_linebuf_rdidx
add wave -noupdate -group Video /tb/top_inst/video/q_hborder
add wave -noupdate -group Video /tb/top_inst/video/q2_hborder
add wave -noupdate -group Video /tb/top_inst/video/q_gfx_start
add wave -noupdate -group Video /tb/top_inst/video/pixel_colidx
add wave -noupdate -group Video /tb/top_inst/video/active
add wave -noupdate -group Video /tb/top_inst/video/pal_r
add wave -noupdate -group Video /tb/top_inst/video/pal_g
add wave -noupdate -group Video /tb/top_inst/video/pal_b
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/clk
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/reset
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tilemode
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/sprites_enable
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/scroll_x
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/scroll_y
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_sel
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_x
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_y
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_idx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_enable
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_priority
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_palette
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_h16
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_vflip
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_hflip
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/vaddr
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/vdata
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/reg_line
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/start
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/linebuf_rdidx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/linebuf_data
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_spr_sel
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_spr_sel
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/wridx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/wrdata
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/wren
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_linesel
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_linesel
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/rddata
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_col
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_col
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_col_cnt
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_col_cnt
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_vaddr
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_vaddr
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_state
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_state
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_nxtstate
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_nxtstate
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_map_entry
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_map_entry
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_blankout
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_blankout
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_busy
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_busy
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/line_idx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tline
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/row
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/map_entry
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tile_idx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tile_hflip
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tile_vflip
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tile_palette
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/tile_priority
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/vdata2
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_height
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/ydiff
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_on_line
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/spr_line
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_render_idx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_render_idx
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_render_data
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_render_data
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_render_is_sprite
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_render_is_sprite
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_render_hflip
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_render_hflip
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_render_palette
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_render_palette
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/d_render_priority
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/q_render_priority
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/render_start
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/render_last_pixel
add wave -noupdate -group Gfx /tb/top_inst/video/gfx/render_busy
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/clk
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/reset
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/render_idx
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/render_data
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/render_start
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/is_sprite
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/hflip
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/palette
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/render_priority
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/last_pixel
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/busy
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/wridx
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/wrdata
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/wren
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_render_data
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_render_data
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_palette
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_palette
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_wridx
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_wridx
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_wrdata
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_wrdata
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_wren
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_wren
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_datasel
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_datasel
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_busy
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_busy
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_last_pixel
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_last_pixel
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_is_sprite
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_is_sprite
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_hflip
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_hflip
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/d_priority
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/q_priority
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/pixel_data
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/lab_priority
add wave -noupdate -expand -group Renderer /tb/top_inst/video/gfx/renderer/lab_wrdata
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {73272998 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 249
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
WaveRestoreZoom {72691104 ps} {74331730 ps}
