#include "opcode_info.h"

// Opcode descriptor:
//
//    7     6     5     4     3     2     1     0
// +-----------+-----------------------------------+
// |More |     |          Argument 1 type          |    First descriptor byte
// +-----+-----+-----------------------------------+
// |Prefix type|          Argument 2 type          |    Second descriptor byte
// +-----------+-----------------------------------+
//
//    More:
//      0: Last entry
//      1: More entries following
//
//    Prefix type:
//      0: None
//      1: CB
//      2: ED
//      3: Second descriptor byte following (prefix type in second byte)
//
//      If argument has IX -> extra prefix of DD
//      If argument has IY -> extra prefix of FD
//

#define DESC(more, prefix, arg1, arg2, opcode) (((unsigned)(more) << 7) | (arg1)), (((unsigned)(prefix) << 6) | (arg2)), (opcode)

static const uint8_t opinf_adc[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0x88),    // ADC A,r
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0x88),       // ADC A,IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0x88),       // ADC A,IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0x8E),   // ADC A,(IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0x8E),   // ADC A,(IY+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xCE),          // ADC A,n
    DESC(0, OD_PREFIX_ED, OD_AT_HL, OD_AT_54_BC_DE_HL_SP, 0x4A), // ADC HL,ss
};
static const uint8_t opinf_add[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0x80),      // ADD A,r
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0x80),         // ADD A,IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0x80),         // ADD A,IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0x86),     // ADD A,(IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0x86),     // ADD A,(IY+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xC6),            // ADD A,n
    DESC(1, OD_PREFIX_NONE, OD_AT_HL, OD_AT_54_BC_DE_HL_SP, 0x09), // ADD HL,ss
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_54_BC_DE_IX_SP, 0x09), // ADD IX,pp
    DESC(0, OD_PREFIX_NONE, OD_AT_IY, OD_AT_54_BC_DE_IY_SP, 0x09), // ADD IY,rr
};
static const uint8_t opinf_and[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0xA0),     // AND A,r        (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0xA0),        // AND A,IXh/IXl  (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0xA0),        // AND A,IYh/IYl  (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0xA6),    // AND A,(IX+d)   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0xA6),    // AND A,(IY+d)   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xE6),           // AND A,n        (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0xA0),  // AND r
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IXHL, OD_AT_NONE, 0xA0),     // AND IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IYHL, OD_AT_NONE, 0xA0),     // AND IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0xA6), // AND (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0xA6), // AND (IY+d)
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0xE6),        // AND n
};
static const uint8_t opinf_bit[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_20_REG_ALL, 0x40),  // BIT r
    DESC(1, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_IX_IND_OFFS, 0x46), // BIT (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_IY_IND_OFFS, 0x46), // BIT (IY+d)
};
static const uint8_t opinf_call[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_53_COND, OD_AT_IMM16, 0xC4), // CALL cc,nn
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM16, OD_AT_NONE, 0xCD),    // CALL nn
};
static const uint8_t opinf_ccf[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x3F), // CCF
};
static const uint8_t opinf_cp[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0xB8),     // CP A,r         (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0xB8),        // CP A,IXh/IXl   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0xB8),        // CP A,IYh/IYl   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0xBE),    // CP A,(IX+d)    (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0xBE),    // CP A,(IY+d)    (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xFE),           // CP A,n         (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0xB8),  // CP r
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IXHL, OD_AT_NONE, 0xB8),     // CP IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IYHL, OD_AT_NONE, 0xB8),     // CP IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0xBE), // CP (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0xBE), // CP (IY+d)
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0xFE),        // CP n
};
static const uint8_t opinf_cpd[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA9), // CPD
};
static const uint8_t opinf_cpdr[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB9), // CPDR
};
static const uint8_t opinf_cpi[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA1), // CPI
};
static const uint8_t opinf_cpir[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB1), // CPIR
};
static const uint8_t opinf_cpl[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x2F), // CPL
};
static const uint8_t opinf_daa[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x27), // DAA
};
static const uint8_t opinf_dec[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_53_REG_ALL, OD_AT_NONE, 0x05),     // DEC r
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IXHL, OD_AT_NONE, 0x05),        // DEC IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IYHL, OD_AT_NONE, 0x05),        // DEC IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x35),    // DEC (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x35),    // DEC (IY+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_SP, OD_AT_NONE, 0x0B), // DEC ss
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_NONE, 0x2B),             // DEC IX
    DESC(0, OD_PREFIX_NONE, OD_AT_IY, OD_AT_NONE, 0x2B),             // DEC IY
};
static const uint8_t opinf_di[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xF3), // DI
};
static const uint8_t opinf_djnz[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_REL_ADDR, OD_AT_NONE, 0x10), // DJNZ e
};
static const uint8_t opinf_ei[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xFB), // EI
};
static const uint8_t opinf_ex[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_SP_IND, OD_AT_HL, 0xE3), // EX (SP),HL
    DESC(1, OD_PREFIX_NONE, OD_AT_SP_IND, OD_AT_IX, 0xE3), // EX (SP),IX
    DESC(1, OD_PREFIX_NONE, OD_AT_SP_IND, OD_AT_IY, 0xE3), // EX (SP),IY
    DESC(1, OD_PREFIX_NONE, OD_AT_AF, OD_AT_AF_ALT, 0x08), // EX AF,AF'
    DESC(0, OD_PREFIX_NONE, OD_AT_DE, OD_AT_HL, 0xEB),     // EX DE,HL
};
static const uint8_t opinf_exx[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xD9), // EXX
};
static const uint8_t opinf_halt[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x76), // HALT
};
static const uint8_t opinf_im[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_43_IMODE, OD_AT_NONE, 0x46), // IM 0/1/2
};
static const uint8_t opinf_in[] = {
    DESC(1, OD_PREFIX_ED, OD_AT_53_BCDEHLFA, OD_AT_C_IND, 0x40), // IN r,(C)
    DESC(0, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8_IND, 0xDB),      // IN A,(n)
};
static const uint8_t opinf_inc[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_53_REG_ALL, OD_AT_NONE, 0x04),     // INC r
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IXHL, OD_AT_NONE, 0x04),        // INC IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IYHL, OD_AT_NONE, 0x04),        // INC IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x34),    // INC (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x34),    // INC (IY+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_SP, OD_AT_NONE, 0x03), // INC ss
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_NONE, 0x23),             // INC IX
    DESC(0, OD_PREFIX_NONE, OD_AT_IY, OD_AT_NONE, 0x23),             // INC IY
};
static const uint8_t opinf_ind[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xAA), // IND
};
static const uint8_t opinf_indr[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xBA), // INDR
};
static const uint8_t opinf_ini[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA2), // INI
};
static const uint8_t opinf_inir[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB2), // INIR
};
static const uint8_t opinf_jp[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_HL_IND, OD_AT_NONE, 0xE9),   // JP (HL)
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND, OD_AT_NONE, 0xE9),   // JP (IX)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND, OD_AT_NONE, 0xE9),   // JP (IY)
    DESC(1, OD_PREFIX_NONE, OD_AT_53_COND, OD_AT_IMM16, 0xC2), // JP cc,nn
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM16, OD_AT_NONE, 0xC3),    // JP nn
};
static const uint8_t opinf_jr[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_43_COND, OD_AT_REL_ADDR, 0x20), // JR cs,e
    DESC(0, OD_PREFIX_NONE, OD_AT_REL_ADDR, OD_AT_NONE, 0x18),    // JR e
};
static const uint8_t opinf_ld[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_BC_IND, OD_AT_A, 0x02),                // LD (BC),A
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_BC_IND, 0x0A),                // LD A,(BC)
    DESC(1, OD_PREFIX_NONE, OD_AT_DE_IND, OD_AT_A, 0x12),                // LD (DE),A
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_DE_IND, 0x1A),                // LD A,(DE)
    DESC(1, OD_PREFIX_NONE, OD_AT_53_REG_ALL, OD_AT_20_REG_ALL, 0x40),   // LD r,r'
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IXHL_ALL, OD_AT_20_IXHL_ALL, 0x40), // LD r,r' (ixhl)
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IYHL_ALL, OD_AT_20_IYHL_ALL, 0x40), // LD r,r' (iyhl)
    DESC(1, OD_PREFIX_ED, OD_AT_I, OD_AT_A, 0x47),                       // LD I,A
    DESC(1, OD_PREFIX_ED, OD_AT_R, OD_AT_A, 0x4F),                       // LD R,A
    DESC(1, OD_PREFIX_ED, OD_AT_A, OD_AT_I, 0x57),                       // LD A,I
    DESC(1, OD_PREFIX_ED, OD_AT_A, OD_AT_R, 0x5F),                       // LD A,R
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_20_BCDEHLA, 0x70),  // LD (IX+d),r
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_20_BCDEHLA, 0x70),  // LD (IY+d),r
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_IMM8, 0x36),        // LD (IX+d),n
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_IMM8, 0x36),        // LD (IY+d),n
    DESC(1, OD_PREFIX_NONE, OD_AT_53_BCDEHLA, OD_AT_IX_IND_OFFS, 0x46),  // LD r,(IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_53_BCDEHLA, OD_AT_IY_IND_OFFS, 0x46),  // LD r,(IY+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_53_REG_ALL, OD_AT_IMM8, 0x06),         // LD r,n
    DESC(1, OD_PREFIX_NONE, OD_AT_IMM16_IND, OD_AT_A, 0x32),             // LD (nn),A
    DESC(1, OD_PREFIX_NONE, OD_AT_IMM16_IND, OD_AT_HL, 0x22),            // LD (nn),HL
    DESC(1, OD_PREFIX_NONE, OD_AT_IMM16_IND, OD_AT_IX, 0x22),            // LD (nn),IX
    DESC(1, OD_PREFIX_NONE, OD_AT_IMM16_IND, OD_AT_IY, 0x22),            // LD (nn),IY
    DESC(1, OD_PREFIX_ED, OD_AT_IMM16_IND, OD_AT_54_BC_DE_HL_SP, 0x43),  // LD (nn),dd
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM16_IND, 0x3A),             // LD A,(nn)
    DESC(1, OD_PREFIX_NONE, OD_AT_SP, OD_AT_HL, 0xF9),                   // LD SP,HL
    DESC(1, OD_PREFIX_NONE, OD_AT_SP, OD_AT_IX, 0xF9),                   // LD SP,IX
    DESC(1, OD_PREFIX_NONE, OD_AT_SP, OD_AT_IY, 0xF9),                   // LD SP,IY
    DESC(1, OD_PREFIX_NONE, OD_AT_HL, OD_AT_IMM16_IND, 0x2A),            // LD HL,(nn)
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_IMM16_IND, 0x2A),            // LD IX,(nn)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY, OD_AT_IMM16_IND, 0x2A),            // LD IY,(nn)
    DESC(1, OD_PREFIX_ED, OD_AT_54_BC_DE_HL_SP, OD_AT_IMM16_IND, 0x4B),  // LD dd,(nn)
    DESC(1, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_SP, OD_AT_IMM16, 0x01),    // LD dd,nn
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_IMM16, 0x21),                // LD IX,nn
    DESC(1, OD_PREFIX_NONE, OD_AT_IY, OD_AT_IMM16, 0x21),                // LD IY,nn
    DESC(1, OD_PREFIX_NONE, OD_AT_53_IXHL, OD_AT_IMM8, 0x06),            // LD IXh/l,n
    DESC(0, OD_PREFIX_NONE, OD_AT_53_IYHL, OD_AT_IMM8, 0x06),            // LD IYh/l,n
};
static const uint8_t opinf_ldd[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA8), // LDD
};
static const uint8_t opinf_lddr[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB8), // LDDR
};
static const uint8_t opinf_ldi[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA0), // LDI
};
static const uint8_t opinf_ldir[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB0), // LDIR
};
static const uint8_t opinf_neg[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x44), // NEG
};
static const uint8_t opinf_nop[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x00),
};
static const uint8_t opinf_or[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0xB0),     // OR A,r         (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0xB0),        // OR A,IXh/IXl   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0xB0),        // OR A,IYh/IYl   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0xB6),    // OR A,(IX+d)    (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0xB6),    // OR A,(IY+d)    (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xF6),           // OR A,n         (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0xB0),  // OR r
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IXHL, OD_AT_NONE, 0xB0),     // OR IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IYHL, OD_AT_NONE, 0xB0),     // OR IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0xB6), // OR (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0xB6), // OR (IY+d)
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0xF6),        // OR n
};
static const uint8_t opinf_otdr[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xBB), // OTDR
};
static const uint8_t opinf_otir[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB3), // OTIR
};
static const uint8_t opinf_out[] = {
    DESC(1, OD_PREFIX_ED, OD_AT_C_IND, OD_AT_IMM_0, 0x71),      // OUT (C),0
    DESC(1, OD_PREFIX_ED, OD_AT_C_IND, OD_AT_53_BCDEHLA, 0x41), // OUT (C),r
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8_IND, OD_AT_A, 0xD3),     // OUT (n),A
};
static const uint8_t opinf_outd[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xAB), // OUTD
};
static const uint8_t opinf_outi[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA3), // OUTI
};
static const uint8_t opinf_pop[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_NONE, 0xE1),             // POP IX
    DESC(1, OD_PREFIX_NONE, OD_AT_IY, OD_AT_NONE, 0xE1),             // POP IY
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_NONE, 0xF1),              // POP A      (unofficial variant)
    DESC(0, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_AF, OD_AT_NONE, 0xC1), // POP qq
};
static const uint8_t opinf_push[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_IX, OD_AT_NONE, 0xE5),             // PUSH IX
    DESC(1, OD_PREFIX_NONE, OD_AT_IY, OD_AT_NONE, 0xE5),             // PUSH IY
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_NONE, 0xF5),              // PUSH A      (unofficial variant)
    DESC(0, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_AF, OD_AT_NONE, 0xC5), // PUSH qq
};
static const uint8_t opinf_res[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_20_REG_ALL, 0x80),  // RES b,r
    DESC(1, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_IX_IND_OFFS, 0x86), // RES b,(IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_IY_IND_OFFS, 0x86), // RES b,(IY+d)
};
static const uint8_t opinf_ret[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_53_COND, OD_AT_NONE, 0xC0), // RET cc
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xC9)     // RET
};
static const uint8_t opinf_reti[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x4D), // RETI
};
static const uint8_t opinf_retn[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x45), // RETN
};
static const uint8_t opinf_rl[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x10),  // RL r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x16), // RL (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x16), // RL (IY+d)
};
static const uint8_t opinf_rla[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x17), // RLA
};
static const uint8_t opinf_rlc[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x00),  // RLC r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x06), // RLC (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x06), // RLC (IY+d)
};
static const uint8_t opinf_rlca[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x07), // RLCA
};
static const uint8_t opinf_rld[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x6F), // RLD
};
static const uint8_t opinf_rr[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x18),  // RR r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x1E), // RR (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x1E), // RR (IY+d)
};
static const uint8_t opinf_rra[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x1F), // RRA
};
static const uint8_t opinf_rrc[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x08),  // RRC r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x0E), // RRC (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x0E), // RRC (IY+d)
};
static const uint8_t opinf_rrca[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x0F), // RRCA
};
static const uint8_t opinf_rrd[] = {
    DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x67), // RRD
};
static const uint8_t opinf_rst[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_53_RST, OD_AT_NONE, 0xC7), // RST p
};
static const uint8_t opinf_sbc[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0x98),     // SBC A,r
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0x98),        // SBC A,IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0x98),        // SBC A,IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0x9E),    // SBC A,(IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0x9E),    // SBC A,(IY+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xDE),           // SBC A,n
    DESC(1, OD_PREFIX_ED, OD_AT_HL, OD_AT_54_BC_DE_HL_SP, 0x42),  // SBC HL,ss
    DESC(1, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0x98),  // SBC r          (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IXHL, OD_AT_NONE, 0x98),     // SBC IXh/IXl    (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IYHL, OD_AT_NONE, 0x98),     // SBC IYh/IYl    (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x9E), // SBC (IX+d)     (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x9E), // SBC (IY+d)     (unofficial variant)
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0xDE),        // SBC n          (unofficial variant)
};
static const uint8_t opinf_scf[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x37), // SCF
};
static const uint8_t opinf_set[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_20_REG_ALL, 0xC0),  // SET b,r
    DESC(1, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_IX_IND_OFFS, 0xC6), // SET b,(IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_53_IMM, OD_AT_IY_IND_OFFS, 0xC6), // SET b,(IY+d)
};
static const uint8_t opinf_sla[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x20),  // SLA r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x26), // SLA (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x26), // SLA (IY+d)
};
static const uint8_t opinf_sll[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x30),  // SLL r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x36), // SLL (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x36), // SLL (IY+d)
};
static const uint8_t opinf_sra[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x28),  // SRA r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x2E), // SRA (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x2E), // SRA (IY+d)
};
static const uint8_t opinf_srl[] = {
    DESC(1, OD_PREFIX_CB, OD_AT_20_REG_ALL, OD_AT_NONE, 0x38),  // SRL r
    DESC(1, OD_PREFIX_CB, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x3E), // SRL (IX+d)
    DESC(0, OD_PREFIX_CB, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x3E), // SRL (IY+d)
};
static const uint8_t opinf_sub[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_REG_ALL, 0x90),     // SUB A,r        (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IXHL, 0x90),        // SUB A,IXh/IXl  (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_20_IYHL, 0x90),        // SUB A,IYh/IYl  (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IX_IND_OFFS, 0x96),    // SUB A,(IX+d)   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IY_IND_OFFS, 0x96),    // SUB A,(IY+d)   (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8, 0xD6),           // SUB A,n        (unofficial variant)
    DESC(1, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0x90),  // SUB r
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IXHL, OD_AT_NONE, 0x90),     // SUB IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IYHL, OD_AT_NONE, 0x90),     // SUB IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0x96), // SUB (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0x96), // SUB (IY+d)
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0xD6),        // SUB n
};
static const uint8_t opinf_xor[] = {
    DESC(1, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0xA8),  // XOR r
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IXHL, OD_AT_NONE, 0xA8),     // XOR IXh/IXl
    DESC(1, OD_PREFIX_NONE, OD_AT_20_IYHL, OD_AT_NONE, 0xA8),     // XOR IYh/IYl
    DESC(1, OD_PREFIX_NONE, OD_AT_IX_IND_OFFS, OD_AT_NONE, 0xAE), // XOR (IX+d)
    DESC(1, OD_PREFIX_NONE, OD_AT_IY_IND_OFFS, OD_AT_NONE, 0xAE), // XOR (IY+d)
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0xEE),        // XOR n
};

const uint8_t *opcode_info[TOK_OPCODE_LAST - TOK_OPCODE_FIRST + 1] = {
    opinf_adc,
    opinf_add,
    opinf_and,
    opinf_bit,
    opinf_call,
    opinf_ccf,
    opinf_cp,
    opinf_cpd,
    opinf_cpdr,
    opinf_cpi,
    opinf_cpir,
    opinf_cpl,
    opinf_daa,
    opinf_dec,
    opinf_di,
    opinf_djnz,
    opinf_ei,
    opinf_ex,
    opinf_exx,
    opinf_halt,
    opinf_im,
    opinf_in,
    opinf_inc,
    opinf_ind,
    opinf_indr,
    opinf_ini,
    opinf_inir,
    opinf_jp,
    opinf_jr,
    opinf_ld,
    opinf_ldd,
    opinf_lddr,
    opinf_ldi,
    opinf_ldir,
    opinf_neg,
    opinf_nop,
    opinf_or,
    opinf_otdr,
    opinf_otir,
    opinf_out,
    opinf_outd,
    opinf_outi,
    opinf_pop,
    opinf_push,
    opinf_res,
    opinf_ret,
    opinf_reti,
    opinf_retn,
    opinf_rl,
    opinf_rla,
    opinf_rlc,
    opinf_rlca,
    opinf_rld,
    opinf_rr,
    opinf_rra,
    opinf_rrc,
    opinf_rrca,
    opinf_rrd,
    opinf_rst,
    opinf_sbc,
    opinf_scf,
    opinf_set,
    opinf_sla,
    opinf_sll,
    opinf_sra,
    opinf_srl,
    opinf_sub,
    opinf_xor,
};
