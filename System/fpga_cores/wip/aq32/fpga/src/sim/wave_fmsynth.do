onerror {resume}
quietly virtual signal -install /tb_fmsynth/fmsynth/fm_eg { /tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[23:15]} envelope
quietly WaveActivateNextPane {} 0
add wave -noupdate -expand -group Waveforms -format Analog-Step -height 50 -max 4095.0000000000005 -min -4096.0 -radix decimal /tb_fmsynth/fmsynth/audio_l
add wave -noupdate -expand -group Waveforms -format Analog-Step -height 50 -max 4095.0000000000005 -min -4096.0 -radix decimal /tb_fmsynth/fmsynth/audio_r
add wave -noupdate -expand -group Waveforms -format Analog-Step -height 84 -max 1023.0 -radix unsigned /tb_fmsynth/fmsynth/fm_phase/phase
add wave -noupdate -expand -group Waveforms -format Analog-Step -height 50 -max 511.0 -radix unsigned -childformat {{{/tb_fmsynth/fmsynth/fm_eg/envelope[23]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[22]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[21]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[20]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[19]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[18]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[17]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[16]} -radix unsigned} {{/tb_fmsynth/fmsynth/fm_eg/envelope[15]} -radix unsigned}} -subitemconfig {{/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[23]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[22]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[21]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[20]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[19]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[18]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[17]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[16]} {-radix unsigned} {/tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt[15]} {-radix unsigned}} /tb_fmsynth/fmsynth/fm_eg/envelope
add wave -noupdate -expand -group Waveforms -format Analog-Step -height 50 -max 511.0 -radix unsigned /tb_fmsynth/fmsynth/fm_eg/env
add wave -noupdate -expand -group Waveforms -format Analog-Step -height 50 -max 4095.0000000000005 -min -4096.0 -radix decimal /tb_fmsynth/fmsynth/fm_op/result
add wave -noupdate -format Analog-Step -height 50 -max 4095.0 -min -4095.0 -radix decimal /tb_fmsynth/fmsynth/fb_mod
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/clk
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/reset
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/bus_addr
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/bus_wrdata
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/bus_wren
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/bus_rddata
add wave -noupdate -expand -group {FM synth ports} /tb_fmsynth/fmsynth/bus_wait
add wave -noupdate -expand -group {FM synth ports} -radix decimal /tb_fmsynth/fmsynth/audio_l
add wave -noupdate -expand -group {FM synth ports} -radix decimal /tb_fmsynth/fmsynth/audio_r
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/bus_wr
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/sel_reg0
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/sel_reg1
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/sel_reg2
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/sel_ch_attr
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/sel_op_attr
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_accum_l
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_accum_r
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_kon
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_alg
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_restart
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_dam
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_dvb
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_nts
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_4op
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/ch_attr_rddata
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/op_attr_rddata
add wave -noupdate -expand -group {FM synth internal} -radix unsigned /tb_fmsynth/fmsynth/q_next_sample
add wave -noupdate -expand -group {FM synth internal} -radix unsigned /tb_fmsynth/fmsynth/q_next_sample_cnt
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_timer
add wave -noupdate -expand -group {FM synth internal} -radix unsigned /tb_fmsynth/fmsynth/q_vibpos
add wave -noupdate -expand -group {FM synth internal} -format Analog-Step -height 25 -max 105.0 -radix unsigned /tb_fmsynth/fmsynth/q_am_cnt
add wave -noupdate -expand -group {FM synth internal} -radix unsigned /tb_fmsynth/fmsynth/d_am_dir
add wave -noupdate -expand -group {FM synth internal} -radix unsigned /tb_fmsynth/fmsynth/q_am_dir
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/am_val
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_op_reset
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/phase
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/restart
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/env
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/d_fb_data
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_fb_data
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/d_op_result
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_op_result
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/fb_sum
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/fb_mod
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/op_modulation
add wave -noupdate -expand -group {FM synth internal} /tb_fmsynth/fmsynth/q_state
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/q_op_sel
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_ws
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_am
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_vib
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_sus
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_ksr
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_mult
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_ksl
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_tl
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_ar
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_dr
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_sl
add wave -noupdate -group {Operator attributes} -radix unsigned /tb_fmsynth/fmsynth/op_rr
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_sel
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_chb
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_cha
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_fb
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_kon
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_block
add wave -noupdate -group {Channel attributes} -radix unsigned /tb_fmsynth/fmsynth/ch_fnum
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/op_sel
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/next
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/block
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/fnum
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/mult
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/nts
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/ksr
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/dvb
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/vib
add wave -noupdate -group {Operator phase counter} -expand -group {Phase ports} /tb_fmsynth/fmsynth/fm_phase/phase
add wave -noupdate -group {Operator phase counter} -expand -group {Phase internal} /tb_fmsynth/fmsynth/fm_phase/fw_multiplier
add wave -noupdate -group {Operator phase counter} -expand -group {Phase internal} /tb_fmsynth/fmsynth/fm_phase/fw
add wave -noupdate -group {Operator phase counter} -expand -group {Phase internal} /tb_fmsynth/fmsynth/fm_phase/fw_scaled
add wave -noupdate -group {Operator phase counter} -expand -group {Phase internal} /tb_fmsynth/fmsynth/fm_phase/d_op_phase
add wave -noupdate -group {Operator phase counter} -expand -group {Phase internal} /tb_fmsynth/fmsynth/fm_phase/q_op_phase
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/op_sel
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/next
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/ar
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/dr
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/sl
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/rr
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/block
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/fnum
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/nts
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/ksr
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/kon
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} /tb_fmsynth/fmsynth/fm_eg/sus
add wave -noupdate -group {Envelope generator} -expand -group {EG ports} -radix unsigned /tb_fmsynth/fmsynth/fm_eg/env
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/d_eg_stage
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/q_eg_stage
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/d_eg_env_cnt
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/q_eg_env_cnt
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/stage_rate
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/eg_env_cnt_inc
add wave -noupdate -group {Envelope generator} -group {EG internal} /tb_fmsynth/fmsynth/fm_eg/eg_env_cnt_next
add wave -noupdate -group Operator -group {OP ports} /tb_fmsynth/fmsynth/fm_op/ws
add wave -noupdate -group Operator -group {OP ports} /tb_fmsynth/fmsynth/fm_op/phase
add wave -noupdate -group Operator -group {OP ports} /tb_fmsynth/fmsynth/fm_op/env
add wave -noupdate -group Operator -group {OP ports} /tb_fmsynth/fmsynth/fm_op/result
add wave -noupdate -group Operator -group {OP internal} /tb_fmsynth/fmsynth/fm_op/logsin_idx
add wave -noupdate -group Operator -group {OP internal} /tb_fmsynth/fmsynth/fm_op/logsin_value
add wave -noupdate -group Operator -group {OP internal} /tb_fmsynth/fmsynth/fm_op/exp_value
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {766112687 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 415
configure wave -valuecolwidth 98
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
WaveRestoreZoom {0 ps} {10500 us}
