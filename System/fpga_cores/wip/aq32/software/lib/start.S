    .section ".text.init"
    .global _start
_start:
    // Initialize GP
.option push
.option norelax
    la      gp, __global_pointer$
.option pop

    // Initialize SP
    la      sp, __stack_end

    // Initialize BSS
    la      t0, __bss_start
    la      t1, __bss_end
    j       2f
1:  sw      zero, (t0)
    add     t0, t0, 4
2:  bltu    t0, t1, 1b

    call    __libc_init_array
    j       _c_start

    .global load_executable
load_executable:
    j       4
