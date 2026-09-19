//////////////////////////////////////////////////////////////////////////////
// Startup code
//////////////////////////////////////////////////////////////////////////////
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

    // Initialize mtvec
    la      t0, trap_handler_entry
    csrw    mtvec, t0

    call    __libc_init_array
    j       _c_start

//////////////////////////////////////////////////////////////////////////////
// Trap handling
//////////////////////////////////////////////////////////////////////////////
#include "trap.h"

    .section ".text.trap"
    .global trap_handler_entry
    .align  4
trap_handler_entry:
    // Save registers
    addi    sp, sp, -TR_SIZE
    sw      ra,  (TR_RA)(sp)
    sw      gp,  (TR_GP)(sp)
    sw      tp,  (TR_TP)(sp)
    sw      t0,  (TR_T0)(sp)
    sw      t1,  (TR_T1)(sp)
    sw      t2,  (TR_T2)(sp)
    sw      s0,  (TR_S0)(sp)
    sw      s1,  (TR_S1)(sp)
    sw      a0,  (TR_A0)(sp)
    sw      a1,  (TR_A1)(sp)
    sw      a2,  (TR_A2)(sp)
    sw      a3,  (TR_A3)(sp)
    sw      a4,  (TR_A4)(sp)
    sw      a5,  (TR_A5)(sp)
    sw      a6,  (TR_A6)(sp)
    sw      a7,  (TR_A7)(sp)
    sw      s2,  (TR_S2)(sp)
    sw      s3,  (TR_S3)(sp)
    sw      s4,  (TR_S4)(sp)
    sw      s5,  (TR_S5)(sp)
    sw      s6,  (TR_S6)(sp)
    sw      s7,  (TR_S7)(sp)
    sw      s8,  (TR_S8)(sp)
    sw      s9,  (TR_S9)(sp)
    sw      s10, (TR_S10)(sp)
    sw      s11, (TR_S11)(sp)
    sw      t3,  (TR_T3)(sp)
    sw      t4,  (TR_T4)(sp)
    sw      t5,  (TR_T5)(sp)
    sw      t6,  (TR_T6)(sp)
    csrr    t0,  mepc
    sw      t0,  (TR_EPC)(sp)
    csrr    t0,  mcause
    sw      t0,  (TR_CAUSE)(sp)
    csrr    t0,  mstatus
    sw      t0,  (TR_STATUS)(sp)
    csrr    t0,  mtval
    sw      t0,  (TR_TVAL)(sp)

    // Call trap handler
    mv      a0, sp
    jal     trap_handler

    // Load registers
    lw      t0,  (TR_EPC)(sp)
    csrw    mepc, t0
    lw      t0,  (TR_STATUS)(sp)
    csrw    mstatus, t0
    lw      ra,  (TR_RA)(sp)
    lw      t0,  (TR_T0)(sp)
    lw      t1,  (TR_T1)(sp)
    lw      t2,  (TR_T2)(sp)
    lw      s0,  (TR_S0)(sp)
    lw      s1,  (TR_S1)(sp)
    lw      a0,  (TR_A0)(sp)
    lw      a1,  (TR_A1)(sp)
    lw      a2,  (TR_A2)(sp)
    lw      a3,  (TR_A3)(sp)
    lw      a4,  (TR_A4)(sp)
    lw      a5,  (TR_A5)(sp)
    lw      a6,  (TR_A6)(sp)
    lw      a7,  (TR_A7)(sp)
    lw      s2,  (TR_S2)(sp)
    lw      s3,  (TR_S3)(sp)
    lw      s4,  (TR_S4)(sp)
    lw      s5,  (TR_S5)(sp)
    lw      s6,  (TR_S6)(sp)
    lw      s7,  (TR_S7)(sp)
    lw      s8,  (TR_S8)(sp)
    lw      s9,  (TR_S9)(sp)
    lw      s10, (TR_S10)(sp)
    lw      s11, (TR_S11)(sp)
    lw      t3,  (TR_T3)(sp)
    lw      t4,  (TR_T4)(sp)
    lw      t5,  (TR_T5)(sp)
    lw      t6,  (TR_T6)(sp)
    addi    sp, sp, TR_SIZE

    // Return from interrupt
    mret
