#include "riscv.h"
#include "Common.h"
#include "EmuState.h"

int64_t riscv::tlb_lookup(uint32_t vaddr) {
    uint32_t paddr = 0;
    if (vaddr & (1 << 31)) {
        paddr = vaddr;
    } else {
        uint32_t tlbEntry = emuState.tlb[((vaddr >> 14) & 0x3FF) ^ (masid << 4)];

        paddr             = RAM_BASE + (((tlbEntry & 0xFFFF0000) >> 2) | (vaddr & 0x3FFF));
        unsigned tlbVaddr = ((tlbEntry & 0xFC00) << 14) | (vaddr & 0xFFFFFF);
        unsigned tlbAsid  = (tlbEntry >> 2) & 0x3F;
        unsigned tlbFlags = tlbEntry & 0x3;

        bool tlbMiss = tlbVaddr != vaddr || tlbAsid != masid || tlbFlags == 0;
        if (tlbMiss)
            return -1;
    }
    return paddr;
}

inline uint32_t riscv::addr_translate(uint32_t vaddr, unsigned accessType) {
    uint32_t paddr           = 0;
    bool     accessViolation = false;
    bool     tlbMiss         = false;
    bool     bus_m_mode      = (accessType > 0 && m_mode && mstatus_mprv) ? mstatus_mpp : m_mode;

    if (vaddr & (1 << 31)) {
        paddr = vaddr;

        // Directly mapped memory (only allowed for M-mode)
        if (!bus_m_mode && !(paddr >= ROM_BASE && paddr < ROM_BASE + ROM_SIZE))
            accessViolation = true;

    } else {
        uint32_t tlbEntry = emuState.tlb[((vaddr >> 14) & 0x3FF) ^ (masid << 4)];

        paddr             = RAM_BASE + (((tlbEntry & 0xFFFF0000) >> 2) | (vaddr & 0x3FFF));
        unsigned tlbVaddr = ((tlbEntry & 0xFC00) << 14) | (vaddr & 0xFFFFFF);
        unsigned tlbAsid  = (tlbEntry >> 2) & 0x3F;
        unsigned tlbFlags = tlbEntry & 0x3;

        tlbMiss = tlbVaddr != vaddr || tlbAsid != masid || tlbFlags == 0;

        accessViolation = !tlbMiss && (!bus_m_mode && (tlbFlags == 1 || (tlbFlags == 2 && accessType == 2)));
    }

    if (tlbMiss) {
        if (accessType == 0)
            this->trap = TRAP_INSTR_PAGE_FAULT;
        else if (accessType == 1)
            this->trap = TRAP_LD_PAGE_FAULT;
        else if (accessType == 2)
            this->trap = TRAP_ST_PAGE_FAULT;

        this->mtval = vaddr;

    } else if (accessViolation) {
        if (accessType == 0)
            this->trap = TRAP_INSTR_ACCESS_FAULT;
        else if (accessType == 1)
            this->trap = TRAP_LD_ACCESS_FAULT;
        else if (accessType == 2)
            this->trap = TRAP_ST_ACCESS_FAULT;

        this->mtval = vaddr;
    }
    return paddr;
}

int64_t riscv::addr_translate2(uint32_t vaddr) {
    uint32_t paddr = 0;
    if (vaddr & (1 << 31)) {
        paddr = vaddr;

    } else {
        // // Segment mapped memory
        // uint32_t segreg = this->msegment[vaddr >> 28];
        // uint32_t offset = vaddr & 0x0FFFFFFF;
        // uint32_t size   = (((segreg & 0xFFF0) >> 4) + 1) << 16;

        // if (offset >= size)
        //     return -1;
        // paddr = (segreg & 0xFFFF0000) + offset;
    }
    return paddr;
}

inline void riscv::riscv_write(uint32_t vaddr, uint32_t val, uint32_t mask) {
    uint32_t paddr = addr_translate(vaddr, 2);
    if (this->trap)
        return;

    if (!emuState.memWrite(paddr, val, mask)) {
        // Access fault!
        this->trap  = TRAP_ST_ACCESS_FAULT;
        this->mtval = vaddr;
    }
}

inline uint32_t riscv::riscv_read(uint32_t vaddr) {
    uint32_t paddr = addr_translate(vaddr, 1);
    if (this->trap)
        return 0;

    int64_t result = emuState.memRead(paddr);
    if (result < 0) {
        // Access fault!
        this->trap  = TRAP_LD_ACCESS_FAULT;
        this->mtval = vaddr;
        return 0;
    }
    return result & 0xFFFFFFFF;
}

inline uint32_t riscv::read_instr(uint32_t vaddr) {
    uint32_t paddr = addr_translate(vaddr, 0);
    if (this->trap)
        return 0;

    int64_t result = emuState.memRead(paddr);
    if (result < 0) {
        // Access fault!
        this->trap  = TRAP_INSTR_ACCESS_FAULT;
        this->mtval = vaddr;
        return 0;
    }
    return result & 0xFFFFFFFF;
}

void riscv::emulate(int count) {
    while (count--) {
        mcycle += 3;

        this->trap     = TRAP_NONE;
        uint32_t curpc = this->pc;
        uint32_t newpc = curpc + 4;
        uint32_t instr = read_instr(curpc & ~3);

        if (!this->trap) {
            uint32_t rd_val = 0;
            uint32_t rd_idx = (instr >> 7) & 0x1F;

            switch (instr & 0x7f) {
                case 0b0110111: rd_val = (instr & 0xFFFFF000); break;         // LUI
                case 0b0010111: rd_val = curpc + (instr & 0xFFFFF000); break; // AUIPC
                case 0b1101111: {                                             // JAL
                    int32_t imm =
                        ((instr & 0x80000000) >> 11) | ((instr & 0x7FE00000) >> 20) | ((instr & 0x00100000) >> 9) | ((instr & 0x000FF000));
                    if (imm & 0x00100000)
                        imm |= 0xFFE00000;

                    rd_val = curpc + 4;
                    newpc  = curpc + imm;
                    break;
                }

                case 0b1100111: { // JALR
                    int32_t imm = instr >> 20;
                    if (imm & 0x800)
                        imm |= 0xFFFFF000;

                    rd_val = curpc + 4;
                    newpc  = (this->regs[(instr >> 15) & 0x1F] + imm) & ~3;
                    break;
                }

                case 0b1100011: { // Branch
                    int32_t imm = ((instr & 0xF00) >> 7) | ((instr & 0x7E000000) >> 20) | ((instr & 0x80) << 4) | ((instr >> 31) << 12);
                    if (imm & 0x1000)
                        imm |= 0xFFFFE000;

                    int32_t  rs1       = this->regs[(instr >> 15) & 0x1F];
                    int32_t  rs2       = this->regs[(instr >> 20) & 0x1F];
                    uint32_t target_pc = curpc + imm;
                    rd_idx             = 0;

                    // clang-format off
                    switch ((instr >> 12) & 0x7) {
                        case 0b000: if (          rs1 ==           rs2) newpc = target_pc; break; // BEQ
                        case 0b001: if (          rs1 !=           rs2) newpc = target_pc; break; // BNE
                        case 0b100: if (          rs1 <            rs2) newpc = target_pc; break; // BLT
                        case 0b101: if (          rs1 >=           rs2) newpc = target_pc; break; // BGE
                        case 0b110: if ((uint32_t)rs1 <  (uint32_t)rs2) newpc = target_pc; break; // BLTU
                        case 0b111: if ((uint32_t)rs1 >= (uint32_t)rs2) newpc = target_pc; break; // BGEU
                        default: this->trap = TRAP_INSTR_ILLEGAL; break;
                    }
                    // clang-format on

                    break;
                }

                case 0b0000011: { // Load
                    mcycle++;
                    int32_t imm = instr >> 20;
                    if (imm & 0x800)
                        imm |= 0xFFFFF000;

                    uint32_t rs1  = this->regs[(instr >> 15) & 0x1F];
                    uint32_t addr = rs1 + imm;

                    switch ((instr >> 12) & 0x7) {
                        case 0b000: rd_val = (int8_t)mem_read8(addr); break;   // LB
                        case 0b001: rd_val = (int16_t)mem_read16(addr); break; // LH
                        case 0b010: rd_val = mem_read32(addr); break;          // LW
                        case 0b100: rd_val = mem_read8(addr); break;           // LBU
                        case 0b101: rd_val = mem_read16(addr); break;          // LHU
                        default: this->trap = TRAP_INSTR_ILLEGAL; break;
                    }
                    break;
                }

                case 0b0100011: { // Store
                    mcycle++;
                    int32_t imm = ((instr >> 7) & 0x1F) | ((instr & 0xFE000000) >> 20);
                    if (imm & 0x800)
                        imm |= 0xFFFFF000;

                    uint32_t rs1  = this->regs[(instr >> 15) & 0x1F];
                    uint32_t rs2  = this->regs[(instr >> 20) & 0x1F];
                    uint32_t addr = rs1 + imm;
                    rd_idx        = 0;

                    switch ((instr >> 12) & 0x7) {
                        case 0b000: mem_write8(addr, rs2); break;  // SB
                        case 0b001: mem_write16(addr, rs2); break; // SH
                        case 0b010: mem_write32(addr, rs2); break; // SW
                        default: this->trap = TRAP_INSTR_ILLEGAL; break;
                    }
                    break;
                }

                case 0b0101111: { // Atomic instructions
                    uint32_t rs1 = this->regs[(instr >> 15) & 0x1F];
                    uint32_t rs2 = this->regs[(instr >> 20) & 0x1F];

                    // clang-format off
                    switch ((instr >> 27) & 0x1F) {
                        case 0b00010: rd_val = mem_read32(rs1); reservation = rs1; break;                                               // LR.W
                        case 0b00011:                                                                                                   // SC.W
                            // clang-format on
                            if (reservation == rs1) {
                                rd_val = 0;
                                mem_write32(rs1, rs2);
                            } else {
                                rd_val = 1;
                            }
                            reservation = -1;
                            break;
                            // clang-format off

                        case 0b00000: rd_val = mem_read32(rs1); mem_write32(rs1, rd_val + rs2); break;                                  // AMOADD.W
                        case 0b00001: rd_val = mem_read32(rs1); mem_write32(rs1, rs2); break;                                           // AMOSWAP.W
                        case 0b00100: rd_val = mem_read32(rs1); mem_write32(rs1, rs2 ^ rd_val); break;                                  // AMOXOR.W
                        case 0b01000: rd_val = mem_read32(rs1); mem_write32(rs1, rs2 | rd_val); break;                                  // AMOOR.W
                        case 0b01100: rd_val = mem_read32(rs1); mem_write32(rs1, rs2 & rd_val); break;                                  // AMOAND.W
                        case 0b10000: rd_val = mem_read32(rs1); mem_write32(rs1, (int32_t)rs2 < (int32_t)rd_val ? rs2 : rd_val); break; // AMOMIN.W
                        case 0b10100: rd_val = mem_read32(rs1); mem_write32(rs1, (int32_t)rs2 > (int32_t)rd_val ? rs2 : rd_val); break; // AMOMAX.W
                        case 0b11000: rd_val = mem_read32(rs1); mem_write32(rs1, rs2 < rd_val ? rs2 : rd_val); break;                   // AMOMINU.W
                        case 0b11100: rd_val = mem_read32(rs1); mem_write32(rs1, rs2 > rd_val ? rs2 : rd_val); break;                   // AMOMAXU.W
                        default: this->trap = TRAP_INSTR_ILLEGAL; break;
                    }
                    // clang-format on
                    break;
                }

                case 0b0010011:   // ALU immediate
                case 0b0110011: { // ALU register
                    uint32_t imm = instr >> 20;
                    if (imm & 0x800)
                        imm |= 0xFFFFF000;

                    uint32_t rs1    = this->regs[(instr >> 15) & 0x1F];
                    bool     is_reg = (instr & 0b0100000) != 0;
                    uint32_t rs2    = is_reg ? this->regs[imm & 0x1F] : imm;

                    if (is_reg && (instr >> 25) == 1) {
                        switch ((instr >> 12) & 7) {
                            case 0b000: rd_val = rs1 * rs2; break;                                               // MUL
                            case 0b001: rd_val = ((int64_t)(int32_t)rs1 * (int64_t)(int32_t)rs2) >> 32; break;   // MULH
                            case 0b010: rd_val = ((int64_t)(int32_t)rs1 * (int64_t)(uint32_t)rs2) >> 32; break;  // MULHSU
                            case 0b011: rd_val = ((int64_t)(uint32_t)rs1 * (int64_t)(uint32_t)rs2) >> 32; break; // MULHU
                            case 0b100: {                                                                        // DIV
                                int32_t dividend = rs1;
                                int32_t divisor  = rs2;
                                if (dividend == (int32_t)0x80000000 && divisor == -1) {
                                    rd_val = dividend;
                                } else if (divisor == 0) {
                                    rd_val = 0xFFFFFFFF;
                                } else {
                                    rd_val = dividend / divisor;
                                }
                                break;
                            }
                            case 0b101: { // DIVU
                                uint32_t dividend = rs1;
                                uint32_t divisor  = rs2;
                                if (divisor == 0) {
                                    rd_val = 0xFFFFFFFF;
                                } else {
                                    rd_val = dividend / divisor;
                                }
                                break;
                            }
                            case 0b110: { // REM
                                int32_t dividend = rs1;
                                int32_t divisor  = rs2;
                                if (dividend == (int32_t)0x80000000 && divisor == -1) {
                                    rd_val = 0;
                                } else if (divisor == 0) {
                                    rd_val = dividend;
                                } else {
                                    rd_val = dividend % divisor;
                                }
                                break;
                            }
                            case 0b111: { // REMU
                                uint32_t dividend = rs1;
                                uint32_t divisor  = rs2;
                                if (divisor == 0) {
                                    rd_val = dividend;
                                } else {
                                    rd_val = dividend % divisor;
                                }
                                break;
                            }
                        }

                    } else {
                        switch ((instr >> 12) & 7) {
                            case 0b000: rd_val = (is_reg && (instr & 0x40000000)) ? (rs1 - rs2) : (rs1 + rs2); break;  // ADDI/ADD/SUB
                            case 0b001: rd_val = rs1 << rs2; break;                                                    // SLL
                            case 0b010: rd_val = (int32_t)rs1 < (int32_t)rs2; break;                                   // SLT/SLTI
                            case 0b011: rd_val = rs1 < rs2; break;                                                     // SLTU/SLTIU
                            case 0b100: rd_val = rs1 ^ rs2; break;                                                     // XOR/XORI
                            case 0b101: rd_val = (instr & 0x40000000) ? (((int32_t)rs1) >> rs2) : (rs1 >> rs2); break; // SRL/SRA
                            case 0b110: rd_val = rs1 | rs2; break;                                                     // OR/ORI
                            case 0b111: rd_val = rs1 & rs2; break;                                                     // AND/ANDI
                        }
                    }
                    break;
                }

                case 0b0001111: // FENCE, ignore
                    rd_idx = 0;

                    if ((instr & 0x7FFF) == 0x200f) {
                        uint32_t rs1 = this->regs[(instr >> 15) & 0x1F];

                        // CBO
                        switch (instr >> 20) {
                            case 0:
                                // printf("Cache invalidate @ 0x%08X\n", rs1);
                                break;
                            case 1:
                                // printf("Cache clean @ 0x%08X\n", rs1);
                                break;
                            case 2:
                                // printf("Cache flush @ 0x%08X\n", rs1);
                                break;
                            default: this->trap = TRAP_INSTR_ILLEGAL; break;
                        }
                    }

                    break;

                case 0b1110011: { // SYSTEM
                    unsigned funct3 = (instr >> 12) & 7;

                    if (funct3 == 0b000) {
                        switch (instr >> 20) {
                            case 0b000000000000: this->trap = m_mode ? TRAP_ECALL_M : TRAP_ECALL_U; break;
                            case 0b000000000001: this->trap = TRAP_BREAKPOINT; break;
                            case 0b001100000010:
                                if (m_mode) { // MRET
                                    m_mode = mstatus_mpp;
                                    if (!mstatus_mpp)
                                        mstatus_mprv = false;

                                    newpc        = this->mepc;
                                    mstatus_mpp  = false;
                                    mstatus_mie  = mstatus_mpie;
                                    mstatus_mpie = true;
                                    break;
                                }
                                // fallthrough
                            default: this->trap = TRAP_INSTR_ILLEGAL; break;
                        }
                        break;

                    } else if (funct3 == 0b100) {
                        // Hypervisor Virtual-Machine Load and Store Instructions
                        this->trap = TRAP_INSTR_ILLEGAL;
                        break;

                    } else {
                        // CSR instructions
                        unsigned csr = instr >> 20;

                        // Read from CSR
                        rd_val = 0;
                        if (!m_mode) {
                            this->trap = TRAP_INSTR_ILLEGAL;
                            break;
                        }

                        switch (csr) {
                            case 0x300: {
                                rd_val = 0;
                                if (mstatus_mie)
                                    rd_val |= (1 << 3);
                                if (mstatus_mpie)
                                    rd_val |= (1 << 7);
                                if (mstatus_mpp)
                                    rd_val |= (3 << 11);
                                if (mstatus_mprv)
                                    rd_val |= (1 << 17);
                                break;
                            }
                            case 0x304: rd_val = this->mie; break;
                            case 0x305: rd_val = this->mtvec; break;
                            case 0x340: rd_val = this->mscratch; break;
                            case 0x341: rd_val = this->mepc; break;
                            case 0x342: rd_val = this->mcause; break;
                            case 0x343: rd_val = this->mtval; break;
                            case 0x344: rd_val = this->mip; break;
                            case 0x7C0: rd_val = this->masid; break;
                            case 0xB00: rd_val = this->mcycle & 0xFFFFFFFF; break;
                            case 0xB80: rd_val = this->mcycle >> 32; break;
                        }

                        // Determine new CSR value
                        uint32_t newcsr = 0;
                        uint32_t val    = (funct3 & 4) ? ((instr >> 15) & 0x1F) : this->regs[(instr >> 15) & 0x1F];
                        switch (funct3 & 3) {
                            case 0b01: newcsr = val; break;           // CSRRW(I) - Atomic Read/Write
                            case 0b10: newcsr = rd_val | val; break;  // CSRRS(I) - Atomic Read and Set Bits
                            case 0b11: newcsr = rd_val & ~val; break; // CSRRC(I) - Atomic Read and Clear Bits
                        }

                        // Write CSR
                        switch (csr) {
                            case 0x300: {
                                mstatus_mie  = (newcsr & (1 << 3)) != 0;
                                mstatus_mpie = (newcsr & (1 << 7)) != 0;
                                mstatus_mpp  = (newcsr & (1 << 12)) != 0;
                                mstatus_mprv = (newcsr & (1 << 17)) != 0;
                                break;
                            }
                            case 0x304: this->mie = newcsr; break;
                            case 0x305: this->mtvec = newcsr; break;
                            case 0x340: this->mscratch = newcsr; break;
                            case 0x341: this->mepc = newcsr; break;
                            case 0x342: this->mcause = newcsr; break;
                            case 0x343: this->mtval = newcsr; break;
                            case 0x344: this->mip = newcsr; break;
                            case 0x7C0: this->masid = newcsr & 0x3F; break;
                        }
                    }
                    break;
                }

                default: this->trap = TRAP_INSTR_ILLEGAL; break; // Fault: Invalid opcode.
            }

            if (this->trap) {
                rd_idx = 0;
            }

            if (rd_idx != 0)
                this->regs[rd_idx] = rd_val;

            if (this->trap == 0) {
                uint32_t pending = this->mip & this->mie;
                if (pending != 0 && mstatus_mie) {
                    unsigned irq_num = 31 - __builtin_clz(pending);
                    this->trap       = TRAP_INTERRUPT | irq_num;
                }
            }
        }

        if (this->trap) {
            reservation = -1;

            if (this->trap == TRAP_INSTR_ILLEGAL)
                this->mtval = instr;

            // MPIE=MIE
            mstatus_mpie = mstatus_mie;
            mstatus_mie  = false;
            mstatus_mpp  = m_mode;
            m_mode       = true;

            this->mcause = (this->trap & TRAP_INTERRUPT) ? this->trap : (this->trap - 1);
            this->mepc   = (this->trap & TRAP_INTERRUPT) ? newpc : curpc;
            newpc        = this->mtvec;

            // fprintf(stderr, "Trap @ PC:%08X\n", this->pc);
            // abort();
        }

        this->pc = newpc;
    }
}
