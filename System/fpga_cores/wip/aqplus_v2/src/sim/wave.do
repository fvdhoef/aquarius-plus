onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb/top_inst/clkctrl/dcm_locked
add wave -noupdate /tb/top_inst/clkctrl/clk_locked
add wave -noupdate /tb/top_inst/q_reg_bank0
add wave -noupdate /tb/top_inst/q_reg_bank1
add wave -noupdate /tb/top_inst/q_reg_bank2
add wave -noupdate /tb/top_inst/q_reg_bank3
add wave -noupdate /tb/top_inst/q_ram_we_n
add wave -noupdate /tb/top_inst/ram_we_n
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/clk
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/reset
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/wrdata
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/wr_en
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/rddata
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/rd_en
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/empty
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/full
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/almost_full
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/q_wridx
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/q_rdidx
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/d_wridx
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/d_rdidx
add wave -noupdate -group {RX FIFO} /tb/top_inst/esp_uart/rx_fifo/count
add wave -noupdate -group UART /tb/top_inst/esp_uart/clk
add wave -noupdate -group UART /tb/top_inst/esp_uart/reset
add wave -noupdate -group UART /tb/top_inst/esp_uart/txfifo_data
add wave -noupdate -group UART /tb/top_inst/esp_uart/txfifo_wr
add wave -noupdate -group UART /tb/top_inst/esp_uart/txfifo_full
add wave -noupdate -group UART /tb/top_inst/esp_uart/rxfifo_data
add wave -noupdate -group UART /tb/top_inst/esp_uart/rxfifo_rd
add wave -noupdate -group UART /tb/top_inst/esp_uart/rxfifo_empty
add wave -noupdate -group UART /tb/top_inst/esp_uart/esp_tx
add wave -noupdate -group UART /tb/top_inst/esp_uart/esp_rx
add wave -noupdate -group UART /tb/top_inst/esp_uart/esp_rts
add wave -noupdate -group UART /tb/top_inst/esp_uart/esp_cts
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_cts
add wave -noupdate -group UART /tb/top_inst/esp_uart/txfifo_q
add wave -noupdate -group UART /tb/top_inst/esp_uart/tx_busy
add wave -noupdate -group UART /tb/top_inst/esp_uart/txfifo_empty
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_tx_start
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_tx_data
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_tx_state
add wave -noupdate -group UART /tb/top_inst/esp_uart/tx_valid
add wave -noupdate -group UART /tb/top_inst/esp_uart/txfifo_almost_full
add wave -noupdate -group UART /tb/top_inst/esp_uart/rx_data
add wave -noupdate -group UART /tb/top_inst/esp_uart/rx_valid
add wave -noupdate -group UART /tb/top_inst/esp_uart/rxfifo_full
add wave -noupdate -group UART /tb/top_inst/esp_uart/rxfifo_almost_full
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_rxfifo_wrdata
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_rxfifo_wr
add wave -noupdate -group UART /tb/top_inst/esp_uart/q_rx_escape
add wave -noupdate -group SRAM /tb/sram/A
add wave -noupdate -group SRAM /tb/sram/IO
add wave -noupdate -group SRAM /tb/sram/CE_n
add wave -noupdate -group SRAM /tb/sram/OE_n
add wave -noupdate -group SRAM /tb/sram/WE_n
add wave -noupdate /tb/top_inst/sel_mem_sysram
add wave -noupdate /tb/top_inst/sel_mem_rom
add wave -noupdate /tb/top_inst/sel_io_audio_dac
add wave -noupdate /tb/top_inst/sel_io_bank0
add wave -noupdate /tb/top_inst/sel_io_bank1
add wave -noupdate /tb/top_inst/sel_io_bank2
add wave -noupdate /tb/top_inst/sel_io_bank3
add wave -noupdate /tb/top_inst/sel_io_sysctrl
add wave -noupdate /tb/top_inst/sel_io_cassette
add wave -noupdate /tb/top_inst/sel_io_vsync
add wave -noupdate /tb/top_inst/sel_io_keyb
add wave -noupdate /tb/top_inst/sel_internal
add wave -noupdate /tb/top_inst/sel_mem_ram
add wave -noupdate -expand -group T80 /tb/top_inst/t80/reset
add wave -noupdate -expand -group T80 /tb/top_inst/t80/clk
add wave -noupdate -expand -group T80 /tb/top_inst/t80/clken
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_addr
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_wrdata
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_rd
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_t1_rd
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_wr
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_wrcycle
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_iorq
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_wait
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_rddata
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_int
add wave -noupdate -expand -group T80 /tb/top_inst/t80/bus_nmi
add wave -noupdate -expand -group T80 /tb/top_inst/t80/int_cycle_n
add wave -noupdate -expand -group T80 /tb/top_inst/t80/noread
add wave -noupdate -expand -group T80 /tb/top_inst/t80/write
add wave -noupdate -expand -group T80 /tb/top_inst/t80/q_di
add wave -noupdate -expand -group T80 /tb/top_inst/t80/mcycle
add wave -noupdate -expand -group T80 /tb/top_inst/t80/tstate
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1093588436 ps} 0} {{Cursor 2} {4647973 ps} 0}
quietly wave cursor active 2
configure wave -namecolwidth 273
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
WaveRestoreZoom {2490773 ps} {5772023 ps}
bookmark add wave bookmark0 {{0 ps} {212100032 ps}} 0
