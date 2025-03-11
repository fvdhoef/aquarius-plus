    .section ".text.init"
    .global reset_handler
reset_handler:
    // Initialize GP
.option push
.option norelax
    la      gp, __global_pointer$
.option pop

    // Initialize SP
    la      sp, __ram_end

    // Initialize BSS
    la      t0, _bss_target_start
    la      t1, _bss_target_end
    j       2f
1:  sw      zero, (t0)
    add     t0, t0, 4
2:  bltu    t0, t1, 1b

    // Initialize data
    la      t0, _data_target_start
    la      t1, _data_target_end
    la      t2, _data_source_start
    j       2f
1:  lw      t3, (t2)
    addi    t2, t2, 4
    sw      t3, (t0)
    addi    t0, t0, 4
2:  bltu    t0, t1, 1b

    la      t0, boot
    jr      t0
