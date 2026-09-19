#include "riscv.h"
#include "Common.h"
#include "EmuState.h"

#ifdef WIN32
#include <intrin.h>
static uint32_t __inline __builtin_clz(uint32_t x) {
    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return (31 - r);
}
#endif

void riscv::emulate() {
    mcycle += 2;

    this->trap     = TRAP_NONE;
    uint32_t curpc = this->pc;
    uint32_t newpc = curpc + 4;
    uint32_t instr = instrRead(curpc & ~3);

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
                mcycle += 1;
                int32_t imm = instr >> 20;
                if (imm & 0x800)
                    imm |= 0xFFFFF000;

                uint32_t rs1  = this->regs[(instr >> 15) & 0x1F];
                uint32_t addr = rs1 + imm;

                switch ((instr >> 12) & 0x7) {
                    case 0b000: rd_val = (int8_t)dataRead8(addr); break;   // LB
                    case 0b001: rd_val = (int16_t)dataRead16(addr); break; // LH
                    case 0b010: rd_val = dataRead32(addr); break;          // LW
                    case 0b100: rd_val = dataRead8(addr); break;           // LBU
                    case 0b101: rd_val = dataRead16(addr); break;          // LHU
                    default: this->trap = TRAP_INSTR_ILLEGAL; break;
                }
                break;
            }

            case 0b0100011: { // Store
                mcycle += 1;
                int32_t imm = ((instr >> 7) & 0x1F) | ((instr & 0xFE000000) >> 20);
                if (imm & 0x800)
                    imm |= 0xFFFFF000;

                uint32_t rs1  = this->regs[(instr >> 15) & 0x1F];
                uint32_t rs2  = this->regs[(instr >> 20) & 0x1F];
                uint32_t addr = rs1 + imm;
                rd_idx        = 0;

                switch ((instr >> 12) & 0x7) {
                    case 0b000: dataWrite8(addr, rs2); break;  // SB
                    case 0b001: dataWrite16(addr, rs2); break; // SH
                    case 0b010: dataWrite32(addr, rs2); break; // SW
                    default: this->trap = TRAP_INSTR_ILLEGAL; break;
                }
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
                break;

            case 0b1110011: { // SYSTEM
                unsigned funct3 = (instr >> 12) & 7;

                if (funct3 == 0b000) {
                    switch (instr >> 20) {
                        case 0b000000000000: this->trap = TRAP_ECALL_M; break;
                        case 0b000000000001: this->trap = TRAP_BREAKPOINT; break;
                        case 0b001100000010: { // MRET
                            newpc        = this->mepc;
                            mstatus_mie  = mstatus_mpie;
                            mstatus_mpie = true;
                            break;
                        }
                        default: this->trap = TRAP_INSTR_ILLEGAL; break;
                    }
                    break;

                } else {
                    // CSR instructions
                    unsigned csr = instr >> 20;

                    // Read from CSR
                    rd_val = 0;

                    switch (csr) {
                        case 0x300: {
                            rd_val = 0;
                            if (mstatus_mie)
                                rd_val |= (1 << 3);
                            if (mstatus_mpie)
                                rd_val |= (1 << 7);
                            break;
                        }
                        case 0x304: rd_val = this->mie; break;
                        case 0x305: rd_val = this->mtvec; break;
                        case 0x340: rd_val = this->mscratch; break;
                        case 0x341: rd_val = this->mepc; break;
                        case 0x342: rd_val = this->mcause; break;
                        case 0x343: rd_val = this->mtval; break;
                        case 0x344: rd_val = this->mip; break;
                        case 0xC00:
                        case 0xB00: rd_val = this->mcycle & 0xFFFFFFFF; break;
                        case 0xC80:
                        case 0xB80: rd_val = this->mcycle >> 32; break;
                        case 0xC01: rd_val = this->mtime & 0xFFFFFFFF; break;
                        case 0xC81: rd_val = this->mtime >> 32; break;
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
                            break;
                        }
                        case 0x304: this->mie = newcsr; break;
                        case 0x305: this->mtvec = newcsr; break;
                        case 0x340: this->mscratch = newcsr; break;
                        case 0x341: this->mepc = newcsr; break;
                        case 0x342: this->mcause = newcsr; break;
                        case 0x343: this->mtval = newcsr; break;
                        case 0x344: this->mip = newcsr; break;
                        case 0xC00:
                        case 0xB00: this->mcycle = (this->mcycle & ~(uint64_t)0xFFFFFFFFUL) | newcsr; break;
                        case 0xC80:
                        case 0xB80: this->mcycle = (this->mcycle & (uint64_t)0xFFFFFFFFUL) | ((uint64_t)newcsr << 32U); break;
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
        // printf("Trap: %u  pc: %08X\n", this->trap, this->pc);

        if (this->trap == TRAP_INSTR_ILLEGAL)
            this->mtval = instr;

        // MPIE=MIE
        mstatus_mpie = mstatus_mie;
        mstatus_mie  = false;

        this->mcause = (this->trap & TRAP_INTERRUPT) ? this->trap : (this->trap - 1);
        this->mepc   = (this->trap & TRAP_INTERRUPT) ? newpc : curpc;
        newpc        = this->mtvec;

        // fprintf(stderr, "Trap @ PC:%08X\n", this->pc);
        // abort();
    }

    this->pc = newpc;
}
