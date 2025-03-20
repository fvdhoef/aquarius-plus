#include "riscv.h"

static const char *regname[32] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0/fp", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"};

std::string instrToString(uint32_t instr, uint32_t curpc) {
    unsigned    rd_idx  = (instr >> 7) & 0x1F;
    unsigned    rs1_idx = (instr >> 15) & 0x1F;
    unsigned    rs2_idx = (instr >> 20) & 0x1F;
    unsigned    shamt   = rs2_idx;
    const char *rd_str  = regname[rd_idx];
    const char *rs1_str = regname[rs1_idx];
    const char *rs2_str = regname[rs2_idx];
    unsigned    funct3  = (instr >> 12) & 7;

    switch (instr & 0x7f) {
        case 0b0110111: return fmtstr("lui    %s,0x%08X", rd_str, instr & 0xFFFFF000);             // LUI
        case 0b0010111: return fmtstr("auipc  %s,[0x%08X]", rd_str, curpc + (instr & 0xFFFFF000)); // AUIPC
        case 0b1101111: {                                                                          // JAL
            int32_t imm =
                ((instr & 0x80000000) >> 11) | ((instr & 0x7FE00000) >> 20) | ((instr & 0x00100000) >> 9) | ((instr & 0x000FF000));
            if (imm & 0x00100000)
                imm |= 0xFFE00000;
            return fmtstr("jal    %s,[0x%08X]", rd_str, curpc + imm);
        }
        case 0b1100111: { // JALR
            int32_t imm = instr >> 20;
            if (imm & 0x800)
                imm |= 0xFFFFF000;
            return fmtstr("jalr   %s,%d(%s)", rd_str, imm, rs1_str);
        }
        case 0b1100011: { // Branch
            int32_t imm = ((instr & 0xF00) >> 7) | ((instr & 0x7E000000) >> 20) | ((instr & 0x80) << 4) | ((instr >> 31) << 12);
            if (imm & 0x1000)
                imm |= 0xFFFFE000;

            uint32_t target_pc = curpc + imm;

            switch (funct3) {
                case 0b000: return fmtstr("beq    %s,%s,[0x%08X]", rs1_str, rs2_str, target_pc); // BEQ
                case 0b001: return fmtstr("bne    %s,%s,[0x%08X]", rs1_str, rs2_str, target_pc); // BNE
                case 0b100: return fmtstr("blt    %s,%s,[0x%08X]", rs1_str, rs2_str, target_pc); // BLT
                case 0b101: return fmtstr("bge    %s,%s,[0x%08X]", rs1_str, rs2_str, target_pc); // BGE
                case 0b110: return fmtstr("bltu   %s,%s,[0x%08X]", rs1_str, rs2_str, target_pc); // BLTU
                case 0b111: return fmtstr("bgeu   %s,%s,[0x%08X]", rs1_str, rs2_str, target_pc); // BGEU
                default: break;
            }
            break;
        }
        case 0b0000011: { // Load
            int32_t imm = instr >> 20;
            if (imm & 0x800)
                imm |= 0xFFFFF000;

            switch (funct3) {
                case 0b000: return fmtstr("lb     %s,%d(%s)", rd_str, imm, rs1_str); // LB
                case 0b001: return fmtstr("lh     %s,%d(%s)", rd_str, imm, rs1_str); // LH
                case 0b010: return fmtstr("lw     %s,%d(%s)", rd_str, imm, rs1_str); // LW
                case 0b100: return fmtstr("lbu    %s,%d(%s)", rd_str, imm, rs1_str); // LBU
                case 0b101: return fmtstr("lhu    %s,%d(%s)", rd_str, imm, rs1_str); // LHU
                default: break;
            }
            break;
        }

        case 0b0100011: { // Store
            int32_t imm = ((instr >> 7) & 0x1F) | ((instr & 0xFE000000) >> 20);
            if (imm & 0x800)
                imm |= 0xFFFFF000;

            switch (funct3) {
                case 0b000: return fmtstr("sb     %s,%d(%s)", rs2_str, imm, rs1_str); // SB
                case 0b001: return fmtstr("sh     %s,%d(%s)", rs2_str, imm, rs1_str); // SH
                case 0b010: return fmtstr("sw     %s,%d(%s)", rs2_str, imm, rs1_str); // SW
                default: break;
            }
            break;
        }

        case 0b0010011: { // ALU immediate
            uint32_t imm = instr >> 20;
            if (imm & 0x800)
                imm |= 0xFFFFF000;

            switch (funct3) {
                case 0b000: return fmtstr("addi   %s,%s,%d", rd_str, rs1_str, (int32_t)imm);                                     // ADDI
                case 0b001: return fmtstr("slli   %s,%s,%u", rd_str, rs1_str, shamt);                                            // SLLI
                case 0b010: return fmtstr("slti   %s,%s,%d", rd_str, rs1_str, (int32_t)imm);                                     // SLTI
                case 0b011: return fmtstr("sltiu  %s,%s,%u", rd_str, rs1_str, imm);                                              // SLTIU
                case 0b100: return fmtstr("xori   %s,%s,0x%X", rd_str, rs1_str, imm);                                            // XORI
                case 0b101: return fmtstr((instr & 0x40000000) ? "srai   %s,%s,%u" : "srli   %s,%s,%u", rd_str, rs1_str, shamt); // SRL/SRA
                case 0b110: return fmtstr("ori    %s,%s,0x%X", rd_str, rs1_str, imm);                                            // ORI
                case 0b111: return fmtstr("andi   %s,%s,0x%X", rd_str, rs1_str, imm);                                            // ANDI
                default: break;
            }
            break;
        }

        case 0b0110011: { // ALU register
            if ((instr >> 25) == 1) {
                switch (funct3) {
                    case 0b000: return fmtstr("mul    %s,%s,%s", rd_str, rs1_str, rs2_str); // MUL
                    case 0b001: return fmtstr("mulh   %s,%s,%s", rd_str, rs1_str, rs2_str); // MULH
                    case 0b010: return fmtstr("mulhsu %s,%s,%s", rd_str, rs1_str, rs2_str); // MULHSU
                    case 0b011: return fmtstr("mulhu  %s,%s,%s", rd_str, rs1_str, rs2_str); // MULHU
                    case 0b100: return fmtstr("div    %s,%s,%s", rd_str, rs1_str, rs2_str); // DIV
                    case 0b101: return fmtstr("divu   %s,%s,%s", rd_str, rs1_str, rs2_str); // DIVU
                    case 0b110: return fmtstr("rem    %s,%s,%s", rd_str, rs1_str, rs2_str); // REM
                    case 0b111: return fmtstr("remu   %s,%s,%s", rd_str, rs1_str, rs2_str); // REMU
                    default: break;
                }
            } else {
                switch (funct3) {
                    case 0b000: return fmtstr((instr & 0x40000000) ? "sub    %s,%s,%s" : "add    %s,%s,%s", rd_str, rs1_str, rs2_str); // ADD/SUB
                    case 0b001: return fmtstr("sll    %s,%s,%s", rd_str, rs1_str, rs2_str);                                            // SLL
                    case 0b010: return fmtstr("slt    %s,%s,%s", rd_str, rs1_str, rs2_str);                                            // SLT
                    case 0b011: return fmtstr("sltu   %s,%s,%s", rd_str, rs1_str, rs2_str);                                            // SLTU
                    case 0b100: return fmtstr("xor    %s,%s,%s", rd_str, rs1_str, rs2_str);                                            // XOR
                    case 0b101: return fmtstr((instr & 0x40000000) ? "sra    %s,%s,%s" : "srl    %s,%s,%s", rd_str, rs1_str, rs2_str); // SRL/SRA
                    case 0b110: return fmtstr("or     %s,%s,%s", rd_str, rs1_str, rs2_str);                                            // OR
                    case 0b111: return fmtstr("and    %s,%s,%s", rd_str, rs1_str, rs2_str);                                            // AND
                    default: break;
                }
            }
            break;
        }

        case 0b0001111: return "fence"; // FENCE, ignore

        case 0b1110011: { // SYSTEM
            unsigned csr = instr >> 20;

            if (funct3 == 0b000) {
                switch (csr) {
                    case 0b000000000000: return "ecall";
                    case 0b000000000001: return "ebreak";
                    case 0b001100000010: return "mret";
                    default: break;
                }
                break;

            } else if (funct3 == 0b100) {
                // Hypervisor Virtual-Machine Load and Store Instructions
                break;

            } else {
                std::string csrStr;
                switch (csr) {
                    case 0x300: csrStr = "mstatus"; break;
                    case 0x304: csrStr = "mie"; break;
                    case 0x305: csrStr = "mtvec"; break;
                    case 0x340: csrStr = "mscratch"; break;
                    case 0x341: csrStr = "mepc"; break;
                    case 0x342: csrStr = "mcause"; break;
                    case 0x343: csrStr = "mtval"; break;
                    case 0x344: csrStr = "mip"; break;
                    case 0x7C0: csrStr = "masic"; break;
                    case 0xB00: csrStr = "mcycle"; break;
                    case 0xB80: csrStr = "mcycleh"; break;
                    default: csrStr = fmtstr("0x%X", csr); break;
                }

                // CSR instructions
                switch (funct3) {
                    case 0b001: return fmtstr("csrrw  %s,%s,%s", rd_str, csrStr.c_str(), regname[(instr >> 15) & 0x1F]); // CSRRW - Atomic Read/Write
                    case 0b010: return fmtstr("csrrs  %s,%s,%s", rd_str, csrStr.c_str(), regname[(instr >> 15) & 0x1F]); // CSRRS - Atomic Read and Set Bits
                    case 0b011: return fmtstr("csrrc  %s,%s,%s", rd_str, csrStr.c_str(), regname[(instr >> 15) & 0x1F]); // CSRRC - Atomic Read and Clear Bits
                    case 0b101: return fmtstr("csrrwi %s,%s,%d", rd_str, csrStr.c_str(), (instr >> 15) & 0x1F);          // CSRRWI - Atomic Read/Write
                    case 0b110: return fmtstr("csrrsi %s,%s,%d", rd_str, csrStr.c_str(), (instr >> 15) & 0x1F);          // CSRRSI - Atomic Read and Set Bits
                    case 0b111: return fmtstr("csrrci %s,%s,%d", rd_str, csrStr.c_str(), (instr >> 15) & 0x1F);          // CSRRCI - Atomic Read and Clear Bits
                    default: break;
                }
            }
            break;
        }
        default: break;
    }
    return "Unsupported";
}
