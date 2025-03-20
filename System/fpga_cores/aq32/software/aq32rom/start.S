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

    la      t0, boot
    jr      t0
