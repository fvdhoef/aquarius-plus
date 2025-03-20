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

enum {
    MIP_USBHOST = (1 << 16),
};

struct riscv {
    uint32_t regs[32]; // Registers
    uint32_t pc;       // Program counter
    int64_t  reservation;

    // Control and Status Registers (CSRs)
    bool     mstatus_mie;  // 0x300 (MRW) Machine status register:  <3> Machine interrupt enable
    bool     mstatus_mpie; // 0x300 (MRW) Machine status register:  <7> Machine pre-trap interrupt enable
    bool     mstatus_mpp;  // 0x300 (MRW) Machine status register: <12> Machine previous privilege mode
    bool     mstatus_mprv; // 0x300 (MRW) Machine status register: <17> Modify priviledge
    uint32_t mie;          // 0x304 (MRW) Machine interrupt-enable register
    uint32_t mtvec;        // 0x305 (MRW) Machine trap-handler base address
    uint32_t mscratch;     // 0x340 (MRW) Scratch register for machine trap handlers
    uint32_t mepc;         // 0x341 (MRW) Machine exception program counter
    uint32_t mcause;       // 0x342 (MRW) Machine trap cause
    uint32_t mtval;        // 0x343 (MRW) Machine bad address or instruction
    uint32_t mip;          // 0x344 (MRW) Machine interrupt pending
    uint32_t masid;        // 0x7C0 (MRW) Address Space IDentifier
    uint64_t mcycle;       // 0xB00/0xB80

    // Internal state
    bool     m_mode;
    uint32_t trap;

    void emulate(int count);
    void dumpRegs();

    void pendInterrupt(uint32_t mask) { mip |= mask; }
    void clearInterrupt(uint32_t mask) { mip &= ~mask; }

    int64_t  tlb_lookup(uint32_t vaddr);
    uint32_t addr_translate(uint32_t vaddr, unsigned type);
    int64_t  addr_translate2(uint32_t vaddr);
    void     riscv_write(uint32_t vaddr, uint32_t val, uint32_t mask);
    uint32_t riscv_read(uint32_t vaddr);
    uint32_t read_instr(uint32_t vaddr);

    inline uint32_t mem_read32(uint32_t addr) { return riscv_read(addr); }
    inline uint16_t mem_read16(uint32_t addr) { return (riscv_read(addr) >> ((addr & 2) * 8)) & 0xFFFF; }
    inline uint8_t  mem_read8(uint32_t addr) { return (riscv_read(addr) >> ((addr & 3) * 8)) & 0xFF; }
    inline void     mem_write32(uint32_t addr, uint32_t val) { riscv_write(addr, val, 0xFFFFFFFF); }
    inline void     mem_write16(uint32_t addr, uint16_t val) { riscv_write(addr, (val << 16) | val, 0xFFFF << ((addr & 2) * 8)); }
    inline void     mem_write8(uint32_t addr, uint8_t val) { riscv_write(addr, (val << 24) | (val << 16) | (val << 8) | val, 0xFF << ((addr & 3) * 8)); }
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
