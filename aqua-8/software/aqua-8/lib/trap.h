#pragma once

// struct trap_regs offsets
#define TR_EPC    0
#define TR_RA     4
#define TR_SP     8
#define TR_GP     12
#define TR_TP     16
#define TR_T0     20
#define TR_T1     24
#define TR_T2     28
#define TR_S0     32
#define TR_S1     36
#define TR_A0     40
#define TR_A1     44
#define TR_A2     48
#define TR_A3     52
#define TR_A4     56
#define TR_A5     60
#define TR_A6     64
#define TR_A7     68
#define TR_S2     72
#define TR_S3     76
#define TR_S4     80
#define TR_S5     84
#define TR_S6     88
#define TR_S7     92
#define TR_S8     96
#define TR_S9     100
#define TR_S10    104
#define TR_S11    108
#define TR_T3     112
#define TR_T4     116
#define TR_T5     120
#define TR_T6     124
#define TR_CAUSE  128
#define TR_STATUS 132
#define TR_TVAL   136
#define TR_SIZE   144 // Must be rounded up to multiple of 8

#ifndef __ASSEMBLER__

#include "common.h"

// Structure as saved on stack when entering trap handler
struct trap_regs {
    uint32_t epc;    // (  0) xepc CSR value (Exception program counter)
    uint32_t ra;     // (  4) [  x1] Return address
    uint32_t sp;     // (  8) [  x2] Stack pointer
    uint32_t gp;     // ( 12) [  x3] Global pointer
    uint32_t tp;     // ( 16) [  x4] Thread pointer
    uint32_t t0;     // ( 20) [  x5] Temporary register 0 / alternate link register
    uint32_t t1;     // ( 24) [  x6] Temporary register 1
    uint32_t t2;     // ( 28) [  x7] Temporary register 2
    uint32_t s0;     // ( 32) [  x8] Saved register 0 / frame pointer
    uint32_t s1;     // ( 36) [  x9] Saved register 1
    uint32_t a0;     // ( 40) [ x10] Function argument 0 / return value
    uint32_t a1;     // ( 44) [ x11] Function argument 1 / return value
    uint32_t a2;     // ( 48) [ x12] Function argument 2
    uint32_t a3;     // ( 52) [ x13] Function argument 3
    uint32_t a4;     // ( 56) [ x14] Function argument 4
    uint32_t a5;     // ( 60) [ x15] Function argument 5
    uint32_t a6;     // ( 64) [ x16] Function argument 6
    uint32_t a7;     // ( 68) [ x17] Function argument 7
    uint32_t s2;     // ( 72) [ x18] Saved register 2
    uint32_t s3;     // ( 76) [ x19] Saved register 3
    uint32_t s4;     // ( 80) [ x20] Saved register 4
    uint32_t s5;     // ( 84) [ x21] Saved register 5
    uint32_t s6;     // ( 88) [ x22] Saved register 6
    uint32_t s7;     // ( 92) [ x23] Saved register 7
    uint32_t s8;     // ( 96) [ x24] Saved register 8
    uint32_t s9;     // (100) [ x25] Saved register 9
    uint32_t s10;    // (104) [ x26] Saved register 10
    uint32_t s11;    // (108) [ x27] Saved register 11
    uint32_t t3;     // (112) [ x28] Temporary register 3
    uint32_t t4;     // (116) [ x29] Temporary register 4
    uint32_t t5;     // (120) [ x30] Temporary register 5
    uint32_t t6;     // (124) [ x31] Temporary register 6
    uint32_t cause;  // (128) mcause CSR value
    uint32_t status; // (132) mstatus CSR value
    uint32_t tval;   // (136) mtval CSR value
};

#define VBLANK_IRQn (16)
#define KEYBUF_IRQn (19)
#define UART_IRQn   (20)

#endif
