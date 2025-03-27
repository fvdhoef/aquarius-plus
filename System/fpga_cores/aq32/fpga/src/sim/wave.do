onerror {resume}
quietly virtual signal -install /tb/top_inst { (context /tb/top_inst )&{ebus_ba , ebus_a[13:0] }} sram_addr
quietly WaveActivateNextPane {} 0
add wave -noupdate -group Top /tb/top_inst/sysclk
add wave -noupdate -group Top /tb/top_inst/ebus_reset_n
add wave -noupdate -group Top /tb/top_inst/ebus_phi
add wave -noupdate -group Top /tb/top_inst/sram_addr
add wave -noupdate -group Top /tb/top_inst/ebus_d
add wave -noupdate -group Top /tb/top_inst/ebus_rd_n
add wave -noupdate -group Top /tb/top_inst/ebus_ram_ce_n
add wave -noupdate -group Top /tb/top_inst/ebus_ram_we_n
add wave -noupdate -group Top /tb/top_inst/audio_l
add wave -noupdate -group Top /tb/top_inst/audio_r
add wave -noupdate -group Top /tb/top_inst/vga_r
add wave -noupdate -group Top /tb/top_inst/vga_g
add wave -noupdate -group Top /tb/top_inst/vga_b
add wave -noupdate -group Top /tb/top_inst/vga_hsync
add wave -noupdate -group Top /tb/top_inst/vga_vsync
add wave -noupdate -group Top /tb/top_inst/esp_tx
add wave -noupdate -group Top /tb/top_inst/esp_rx
add wave -noupdate -group Top /tb/top_inst/esp_rts
add wave -noupdate -group Top /tb/top_inst/esp_cts
add wave -noupdate -group Top /tb/top_inst/esp_ssel_n
add wave -noupdate -group Top /tb/top_inst/esp_sclk
add wave -noupdate -group Top /tb/top_inst/esp_mosi
add wave -noupdate -group Top /tb/top_inst/esp_miso
add wave -noupdate -group Top /tb/top_inst/esp_notify
add wave -noupdate -group Top /tb/top_inst/video_clk
add wave -noupdate -group Top -color Gold /tb/top_inst/clk
add wave -noupdate -group Top /tb/top_inst/reset_req
add wave -noupdate -group Top /tb/top_inst/ebus_phi_clken
add wave -noupdate -group Top /tb/top_inst/reset
add wave -noupdate -group Top -color Gold /tb/top_inst/cpu_addr
add wave -noupdate -group Top /tb/top_inst/cpu_wrdata
add wave -noupdate -group Top /tb/top_inst/cpu_bytesel
add wave -noupdate -group Top /tb/top_inst/cpu_wren
add wave -noupdate -group Top /tb/top_inst/cpu_strobe
add wave -noupdate -group Top /tb/top_inst/cpu_wait
add wave -noupdate -group Top /tb/top_inst/cpu_rddata
add wave -noupdate -group Top /tb/top_inst/cpu_irq
add wave -noupdate -group Top /tb/top_inst/sram_a
add wave -noupdate -group Top /tb/top_inst/sram_ctrl_strobe
add wave -noupdate -group Top /tb/top_inst/sram_ctrl_wait
add wave -noupdate -group Top /tb/top_inst/sram_ctrl_rddata
add wave -noupdate -group Top -color Gold /tb/top_inst/bootrom_strobe
add wave -noupdate -group Top -color Gold /tb/top_inst/bootrom_rddata
add wave -noupdate -group Top /tb/top_inst/q_cpu_addr
add wave -noupdate -group Top /tb/top_inst/esp_tx_data
add wave -noupdate -group Top /tb/top_inst/esp_tx_wr
add wave -noupdate -group Top /tb/top_inst/esp_tx_fifo_full
add wave -noupdate -group Top /tb/top_inst/esp_rx_data
add wave -noupdate -group Top /tb/top_inst/esp_rx_rd
add wave -noupdate -group Top /tb/top_inst/esp_rx_empty
add wave -noupdate -group Top /tb/top_inst/esp_rx_fifo_overflow
add wave -noupdate -group Top /tb/top_inst/esp_rx_framing_error
add wave -noupdate -group Top /tb/top_inst/spi_msg_end
add wave -noupdate -group Top /tb/top_inst/spi_cmd
add wave -noupdate -group Top /tb/top_inst/spi_rxdata
add wave -noupdate -group Top /tb/top_inst/spi_txdata
add wave -noupdate -group Top /tb/top_inst/spi_txdata_valid
add wave -noupdate -group Top /tb/top_inst/ovl_text_addr
add wave -noupdate -group Top /tb/top_inst/ovl_text_wrdata
add wave -noupdate -group Top /tb/top_inst/ovl_text_wr
add wave -noupdate -group Top /tb/top_inst/ovl_font_addr
add wave -noupdate -group Top /tb/top_inst/ovl_font_wrdata
add wave -noupdate -group Top /tb/top_inst/ovl_font_wr
add wave -noupdate -group Top /tb/top_inst/ovl_palette_addr
add wave -noupdate -group Top /tb/top_inst/ovl_palette_wrdata
add wave -noupdate -group Top /tb/top_inst/ovl_palette_wr
add wave -noupdate -group Top /tb/top_inst/common_audio_l
add wave -noupdate -group Top /tb/top_inst/common_audio_r
add wave -noupdate -group Top /tb/top_inst/video_r
add wave -noupdate -group Top /tb/top_inst/video_g
add wave -noupdate -group Top /tb/top_inst/video_b
add wave -noupdate -group Top /tb/top_inst/video_de
add wave -noupdate -group Top /tb/top_inst/video_hsync
add wave -noupdate -group Top /tb/top_inst/video_vsync
add wave -noupdate -group Top /tb/top_inst/video_newframe
add wave -noupdate -group Top /tb/top_inst/video_oddline
add wave -noupdate -group Top /tb/top_inst/rddata_tram
add wave -noupdate -group Top /tb/top_inst/rddata_chram
add wave -noupdate -group Top /tb/top_inst/rddata_vram
add wave -noupdate -group Top /tb/top_inst/video_irq
add wave -noupdate -group Top /tb/top_inst/tram_wren
add wave -noupdate -group Top /tb/top_inst/chram_wren
add wave -noupdate -group Top /tb/top_inst/vram_wren
add wave -noupdate -group Bootrom /tb/top_inst/bootrom/clk
add wave -noupdate -group Bootrom /tb/top_inst/bootrom/addr
add wave -noupdate -group Bootrom /tb/top_inst/bootrom/rddata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/clk
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/reset
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_addr
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_wrdata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_bytesel
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_wren
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_strobe
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_wait
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/bus_rddata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/sram_a
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/sram_ce_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/sram_oe_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/sram_we_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/sram_dq
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_state
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_state
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_sram_a
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_sram_a
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_sram_we_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_sram_we_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_sram_oe_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_sram_oe_n
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_bus_wait
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_bus_wait
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_bus_rddata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_bus_rddata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_dq_wrdata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_dq_wrdata
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/d_dq_oe
add wave -noupdate -group {SRAM controller} /tb/top_inst/sram_ctrl/q_dq_oe
add wave -noupdate -group CPU /tb/top_inst/cpu/clk
add wave -noupdate -group CPU /tb/top_inst/cpu/reset
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_addr
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_wrdata
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_bytesel
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_wren
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_strobe
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_wait
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_rddata
add wave -noupdate -group CPU /tb/top_inst/cpu/irq
add wave -noupdate -group CPU /tb/top_inst/cpu/d_pc
add wave -noupdate -group CPU /tb/top_inst/cpu/q_pc
add wave -noupdate -group CPU /tb/top_inst/cpu/d_instr
add wave -noupdate -group CPU -color Gold /tb/top_inst/cpu/q_instr
add wave -noupdate -group CPU /tb/top_inst/cpu/d_exec_first
add wave -noupdate -group CPU /tb/top_inst/cpu/q_exec_first
add wave -noupdate -group CPU /tb/top_inst/cpu/d_state
add wave -noupdate -group CPU /tb/top_inst/cpu/q_state
add wave -noupdate -group CPU /tb/top_inst/cpu/d_addr
add wave -noupdate -group CPU /tb/top_inst/cpu/q_addr
add wave -noupdate -group CPU /tb/top_inst/cpu/d_wrdata
add wave -noupdate -group CPU /tb/top_inst/cpu/q_wrdata
add wave -noupdate -group CPU /tb/top_inst/cpu/d_bytesel
add wave -noupdate -group CPU /tb/top_inst/cpu/q_bytesel
add wave -noupdate -group CPU /tb/top_inst/cpu/d_wren
add wave -noupdate -group CPU /tb/top_inst/cpu/q_wren
add wave -noupdate -group CPU /tb/top_inst/cpu/d_stb
add wave -noupdate -group CPU /tb/top_inst/cpu/q_stb
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mstatus_mie
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mstatus_mie
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mstatus_mpie
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mstatus_mpie
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mie
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mie
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mtvec
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mtvec
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mscratch
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mscratch
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mepc
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mepc
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mcause_irq
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mcause_irq
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mcause_code
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mcause_code
add wave -noupdate -group CPU /tb/top_inst/cpu/d_mtval
add wave -noupdate -group CPU /tb/top_inst/cpu/q_mtval
add wave -noupdate -group CPU /tb/top_inst/cpu/opcode
add wave -noupdate -group CPU /tb/top_inst/cpu/funct3
add wave -noupdate -group CPU /tb/top_inst/cpu/funct7
add wave -noupdate -group CPU /tb/top_inst/cpu/rs1_idx
add wave -noupdate -group CPU /tb/top_inst/cpu/rs2_idx
add wave -noupdate -group CPU /tb/top_inst/cpu/rd_idx
add wave -noupdate -group CPU /tb/top_inst/cpu/imm_i
add wave -noupdate -group CPU /tb/top_inst/cpu/imm_s
add wave -noupdate -group CPU /tb/top_inst/cpu/imm_b
add wave -noupdate -group CPU /tb/top_inst/cpu/imm_u
add wave -noupdate -group CPU /tb/top_inst/cpu/imm_j
add wave -noupdate -group CPU /tb/top_inst/cpu/csr
add wave -noupdate -group CPU /tb/top_inst/cpu/is_lui
add wave -noupdate -group CPU /tb/top_inst/cpu/is_auipc
add wave -noupdate -group CPU /tb/top_inst/cpu/is_jal
add wave -noupdate -group CPU /tb/top_inst/cpu/is_jalr
add wave -noupdate -group CPU /tb/top_inst/cpu/is_branch
add wave -noupdate -group CPU /tb/top_inst/cpu/is_load
add wave -noupdate -group CPU /tb/top_inst/cpu/is_store
add wave -noupdate -group CPU /tb/top_inst/cpu/is_alu_imm
add wave -noupdate -group CPU /tb/top_inst/cpu/is_alu_reg
add wave -noupdate -group CPU /tb/top_inst/cpu/is_system
add wave -noupdate -group CPU /tb/top_inst/cpu/is_fence
add wave -noupdate -group CPU /tb/top_inst/cpu/is_ecall
add wave -noupdate -group CPU /tb/top_inst/cpu/is_ebreak
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mret
add wave -noupdate -group CPU /tb/top_inst/cpu/is_csr
add wave -noupdate -group CPU /tb/top_inst/cpu/is_valid_instruction
add wave -noupdate -group CPU /tb/top_inst/cpu/rd_data
add wave -noupdate -group CPU /tb/top_inst/cpu/rd_wr
add wave -noupdate -group CPU /tb/top_inst/cpu/regfile
add wave -noupdate -group CPU /tb/top_inst/cpu/rs1_data
add wave -noupdate -group CPU /tb/top_inst/cpu/rs2_data
add wave -noupdate -group CPU /tb/top_inst/cpu/rs2_data_s
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg0_zero
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg1_ra
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg2_sp
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg3_gp
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg4_tp
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg5_t0
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg6_t1
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg7_t2
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg8_s0_fp
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg9_s1
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg10_a0
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg11_a1
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg12_a2
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg13_a3
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg14_a4
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg15_a5
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg16_a6
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg17_a7
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg18_s2
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg19_s3
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg20_s4
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg21_s5
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg22_s6
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg23_s7
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg24_s8
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg25_s9
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg26_s10
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg27_s11
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg28_t3
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg29_t4
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg30_t5
add wave -noupdate -group CPU -group Registers /tb/top_inst/cpu/reg31_t6
add wave -noupdate -group CPU /tb/top_inst/cpu/i
add wave -noupdate -group CPU /tb/top_inst/cpu/l_operand
add wave -noupdate -group CPU /tb/top_inst/cpu/r_operand
add wave -noupdate -group CPU /tb/top_inst/cpu/shamt
add wave -noupdate -group CPU /tb/top_inst/cpu/alu_add
add wave -noupdate -group CPU /tb/top_inst/cpu/alu_sub
add wave -noupdate -group CPU /tb/top_inst/cpu/is_eq
add wave -noupdate -group CPU /tb/top_inst/cpu/is_lt
add wave -noupdate -group CPU /tb/top_inst/cpu/is_ltu
add wave -noupdate -group CPU /tb/top_inst/cpu/shl0
add wave -noupdate -group CPU /tb/top_inst/cpu/shl1
add wave -noupdate -group CPU /tb/top_inst/cpu/shl2
add wave -noupdate -group CPU /tb/top_inst/cpu/shl4
add wave -noupdate -group CPU /tb/top_inst/cpu/shl8
add wave -noupdate -group CPU /tb/top_inst/cpu/shl16
add wave -noupdate -group CPU /tb/top_inst/cpu/shr_msb
add wave -noupdate -group CPU /tb/top_inst/cpu/shr0
add wave -noupdate -group CPU /tb/top_inst/cpu/shr1
add wave -noupdate -group CPU /tb/top_inst/cpu/shr2
add wave -noupdate -group CPU /tb/top_inst/cpu/shr4
add wave -noupdate -group CPU /tb/top_inst/cpu/shr8
add wave -noupdate -group CPU /tb/top_inst/cpu/shr16
add wave -noupdate -group CPU /tb/top_inst/cpu/mult_l
add wave -noupdate -group CPU /tb/top_inst/cpu/mult_r
add wave -noupdate -group CPU /tb/top_inst/cpu/mult_result
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mul_div
add wave -noupdate -group CPU /tb/top_inst/cpu/is_div_rem
add wave -noupdate -group CPU /tb/top_inst/cpu/div_done
add wave -noupdate -group CPU /tb/top_inst/cpu/div_quotient
add wave -noupdate -group CPU /tb/top_inst/cpu/div_remainder
add wave -noupdate -group CPU /tb/top_inst/cpu/div_busy
add wave -noupdate -group CPU /tb/top_inst/cpu/div_start
add wave -noupdate -group CPU /tb/top_inst/cpu/alu_result
add wave -noupdate -group CPU /tb/top_inst/cpu/load_store_addr
add wave -noupdate -group CPU /tb/top_inst/cpu/do_branch
add wave -noupdate -group CPU /tb/top_inst/cpu/lb_data
add wave -noupdate -group CPU /tb/top_inst/cpu/lh_data
add wave -noupdate -group CPU /tb/top_inst/cpu/load_data
add wave -noupdate -group CPU /tb/top_inst/cpu/bus_rddata_s
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mstatus
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mie
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mtvec
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mscratch
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mepc
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mcause
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mtval
add wave -noupdate -group CPU /tb/top_inst/cpu/is_mip
add wave -noupdate -group CPU /tb/top_inst/cpu/mstatus
add wave -noupdate -group CPU /tb/top_inst/cpu/csr_rdata
add wave -noupdate -group CPU /tb/top_inst/cpu/csr_operand
add wave -noupdate -group CPU /tb/top_inst/cpu/csr_wdata
add wave -noupdate -group CPU /tb/top_inst/cpu/csr_write
add wave -noupdate -group CPU /tb/top_inst/cpu/q_irq
add wave -noupdate -group CPU /tb/top_inst/cpu/irq_code
add wave -noupdate -group CPU /tb/top_inst/cpu/irq_pending
add wave -noupdate -group CPU /tb/top_inst/cpu/pc_plus4
add wave -noupdate -group CPU /tb/top_inst/cpu/pc_plus_imm
add wave -noupdate -group CPU /tb/top_inst/cpu/do_trap
add wave -noupdate -group CPU /tb/top_inst/cpu/trap_is_irq
add wave -noupdate -group CPU /tb/top_inst/cpu/trap_code
add wave -noupdate -group CPU /tb/top_inst/cpu/trap_mtval
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/clk
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/reset
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_addr
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_bytesel
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_wren
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_strobe
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_wait
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_rddata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_addr
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_wren
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_strobe
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_wait
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_rddata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_data_write
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_data_write
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_tag_write
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_tag_write
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_tag_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_tag_rddata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_tag_valid
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_tag_dirty
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/s_tag
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_tag_wrdata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/m_tag_rddata
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/ds_state
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/qs_state
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/dm_addr
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/qm_addr
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/dm_wren
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/qm_wren
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/dm_strobe
add wave -noupdate -expand -group {SRAM cache} /tb/top_inst/sram_cache/qm_strobe
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {99890078 ps} 0}
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
WaveRestoreZoom {0 ps} {105 us}
