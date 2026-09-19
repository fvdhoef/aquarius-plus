#pragma once

#include "Common.h"

enum trap {
    TRAP_NONE               = 0,
    TRAP_INSTR_MISALIGNED   = 1 + 0,  // Instruction address misaligned
    TRAP_INSTR_ACCESS_FAULT = 1 + 1,  // Instruction access fault
    TRAP_INSTR_ILLEGAL      = 1 + 2,  // Illegal instruction
    TRAP_BREAKPOINT         = 1 + 3,  // Breakpoint
    TRAP_LD_MISALIGNED      = 1 + 4,  // Load address misaligned
    TRAP_LD_ACCESS_FAULT    = 1 + 5,  // Load access fault
    TRAP_ST_MISALIGNED      = 1 + 6,  // Store address misaligned
    TRAP_ST_ACCESS_FAULT    = 1 + 7,  // Store access fault
    TRAP_ECALL_U            = 1 + 8,  // Environment call from U-mode
    TRAP_ECALL_S            = 1 + 9,  // Environment call from S-mode
    TRAP_ECALL_M            = 1 + 11, // Environment call from M-mode
    TRAP_INSTR_PAGE_FAULT   = 1 + 12, // Instruction page fault
    TRAP_LD_PAGE_FAULT      = 1 + 13, // Load page fault
    TRAP_ST_PAGE_FAULT      = 1 + 15, // Store page fault
    TRAP_INTERRUPT          = (1 << 31),
};

struct riscv {
    uint32_t regs[32]; // Registers
    uint32_t pc;       // Program counter

    // Control and Status Registers (CSRs)
    bool     mstatus_mie;  // 0x300 (MRW) Machine status register:  <3> Machine interrupt enable
    bool     mstatus_mpie; // 0x300 (MRW) Machine status register:  <7> Machine pre-trap interrupt enable
    uint32_t mie;          // 0x304 (MRW) Machine interrupt-enable register
    uint32_t mtvec;        // 0x305 (MRW) Machine trap-handler base address
    uint32_t mscratch;     // 0x340 (MRW) Scratch register for machine trap handlers
    uint32_t mepc;         // 0x341 (MRW) Machine exception program counter
    uint32_t mcause;       // 0x342 (MRW) Machine trap cause
    uint32_t mtval;        // 0x343 (MRW) Machine bad address or instruction
    uint32_t mip;          // 0x344 (MRW) Machine interrupt pending
    uint64_t mcycle;       // 0xB00/0xB80 and 0xC00/0xC80
    uint64_t mtime;        // 0xC01/0xC81
    uint64_t mtimecmp;

    // Internal state
    uint32_t trap;

    void emulate();
    void dumpRegs();

    void pendInterrupt(uint32_t mask) { mip |= mask; }
    void clearInterrupt(uint32_t mask) { mip &= ~mask; }

    std::function<void(uint32_t vaddr, uint8_t val)>  dataWrite8;
    std::function<void(uint32_t vaddr, uint16_t val)> dataWrite16;
    std::function<void(uint32_t vaddr, uint32_t val)> dataWrite32;
    std::function<uint8_t(uint32_t vaddr)>            dataRead8;
    std::function<uint16_t(uint32_t vaddr)>           dataRead16;
    std::function<uint32_t(uint32_t vaddr)>           dataRead32;
    std::function<uint32_t(uint32_t vaddr)>           instrRead;
};

std::string instrToString(uint32_t instruction, uint32_t pc);

//    3                   2                   1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-+---------------+-+-+-+-+-+-+---+---+---+---+-+-+-+-+-+-+-+-+-+
// |0|0 0 0 0 0 0 0 0|0|0|0|0|0| |0 0|0 0|   |0 0|0| |0|0|0| |0|0|0|
// +-+---------------+-+-+-+-+-+-+---+---+---+---+-+-+-+-+-+-+-+-+-+
//                              ^          ^        ^       ^
//                              MPRV       MPP      MPIE    MIE
//
// MIE: Global interrupt enable
// MPIE: Value of MIE before trap
// MPP: Previous privilege mode (0: U-mode, 3: M-mode)
// MPRV: Modify privilege (if set load/store are access using the privilege mode in MPP)
