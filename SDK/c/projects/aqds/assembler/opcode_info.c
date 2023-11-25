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
//    Argument type:
//      0: No arguments
//      1: 8-bit direct
//      2: 16-bit direct
//
//    Argument type:
//      0: None
//      1: register A
//      2: register R
//      3: register I
//      4: register IX
//      5: register IY
//      6: register SP
//      7: register DE
//      8: register HL
//      9: register (BC)
//      9: register (DE)
//      9: register (HL)
//     10: register (IX)
//     11: register (IY)
//     12: register (SP)
//     13: (IX+d)
//     14: (IY+d)
//     15: value 0
//     16: relative address (e)
//     17: 8-bit direct n
//     18: 8-bit indirect (n)
//     19: 16-bit direct nn
//     20: 16-bit indirect (nn)
//
//          Encoded 8-bit register in [2:0]
//          |  0 |  1 |  2 |  3 |  4 |  5 | 6  |  7 |
//          |----|----|----|----|----|----|----|----|
//     21:  |  B |  C |  D |  E |  H |  L |(HL)|  A |  rs
//     22:  |  B |  C |  D |  E | IXh| IXl| -  |  A |  rx
//     23:  |  B |  C |  D |  E | IYh| IYl| -  |  A |  ry
//     24:  |    |    |    |    | IXh| IXl|    |    |  rixhl
//     25:  |    |    |    |    | IYh| IYl|    |    |  riyhl
//
//          Encoded 8-bit register in [5:3]
//          |  0 |  1 |  2 |  3 |  4 |  5 | 6  |  7 |
//          |----|----|----|----|----|----|----|----|
//     26:  |  B |  C |  D |  E |  H |  L |(HL)|  A |  rd
//     27:  |  B |  C |  D |  E |  H |  L | F  |  A |  rf
//     28:  |  B |  C |  D |  E |  H |  L | -  |  A |  rq
//     29:  |  B |  C |  D |  E |  - |  - | -  |  A |  rp
//     30:  |    |    |    |    | IXh| IXl|    |    |  rixhlb
//     31:  |    |    |    |    | IYh| IYl|    |    |  riyhlb
//
//          Encoded 16-bit register in [5:4]
//          |  0 |  1 |  2 |  3 |
//          |----|----|----|----|
//     32:  | BC | DE | HL | AF | qq
//     33:  | BC | DE | IX | SP | pp
//     34:  | BC | DE | IY | SP | rr
//     35:  | BC | DE | HL | SP | ss
//
//          Encoded condition (c) in [5:3]
//          |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
//          |----|----|----|----|----|----|----|----|
//     36:  | NZ |  Z | NC |  C | PO | PE |  P |  M |
//
//          Encoded condition (cs) in [4:3]
//          |  0 |  1 |  2 |  3 |
//          |----|----|----|----|
//     37:  | NZ |  Z | NC |  C |
//
//     38: Encoded bit number (b) 0-7 in [5:3]
//     39: Encoded reset offset (t) in [5:3] 0:$00 1:$08 2:$10 3:$18 4:$20 5:$28 6:$30 7:$38
//     40: Encoded interrupt mode (i) in [4:3]  0:IM0 2:IM1 3:IM2
//

#define DESC(more, prefix, arg1, arg2, opcode) (((more) << 7) | (arg1)), (((prefix) << 6) | (arg2)), (opcode)

static const uint8_t opinf_adc[] = {0x3F};
static const uint8_t opinf_add[] = {0x3F};
static const uint8_t opinf_and[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8, OD_AT_NONE, 0x06), // AND n
};
static const uint8_t opinf_bit[]  = {0x3F};
static const uint8_t opinf_call[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM16, OD_AT_NONE, 0xCD), // CALL nn
};
static const uint8_t opinf_cp[]   = {0x3F};
static const uint8_t opinf_dec[]  = {0x3F};
static const uint8_t opinf_djnz[] = {0x3F};
static const uint8_t opinf_ex[]   = {0x3F};
static const uint8_t opinf_im[]   = {0x3F};
static const uint8_t opinf_in[]   = {
    DESC(0, OD_PREFIX_NONE, OD_AT_A, OD_AT_IMM8_IND, 0xDB), // IN A,(n)
};
static const uint8_t opinf_inc[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_SP, OD_AT_NONE, 0x03), // INC ss
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
    DESC(1, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_SP, OD_AT_IMM16, 0x01), // LD dd,nn
    DESC(0, OD_PREFIX_NONE, OD_AT_53_REG_ALL, OD_AT_IMM8, 0x06),      // LD r,n
};
static const uint8_t opinf_or[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_20_REG_ALL, OD_AT_NONE, 0xB0), // OR r
};
static const uint8_t opinf_out[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_IMM8_IND, OD_AT_A, 0xD3), // OUT (n),A
};
static const uint8_t opinf_pop[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_AF, OD_AT_NONE, 0xC1), // POP qq
};
static const uint8_t opinf_push[] = {
    DESC(0, OD_PREFIX_NONE, OD_AT_54_BC_DE_HL_AF, OD_AT_NONE, 0xC5), // PUSH qq
};
static const uint8_t opinf_res[] = {0x3F};
static const uint8_t opinf_rl[]  = {0x3F};
static const uint8_t opinf_rlc[] = {0x3F};
static const uint8_t opinf_rr[]  = {0x3F};
static const uint8_t opinf_rrc[] = {0x3F};
static const uint8_t opinf_rst[] = {0x3F};
static const uint8_t opinf_sbc[] = {0x3F};
static const uint8_t opinf_set[] = {0x3F};
static const uint8_t opinf_sla[] = {0x3F};
static const uint8_t opinf_sll[] = {0x3F};
static const uint8_t opinf_sra[] = {0x3F};
static const uint8_t opinf_srl[] = {0x3F};
static const uint8_t opinf_sub[] = {0x3F};
static const uint8_t opinf_xor[] = {0x3F};

static const uint8_t opinf_ccf[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x3F)};
static const uint8_t opinf_cpd[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA9)};
static const uint8_t opinf_cpdr[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB9)};
static const uint8_t opinf_cpi[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA1)};
static const uint8_t opinf_cpir[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB1)};
static const uint8_t opinf_cpl[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x2F)};
static const uint8_t opinf_daa[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x27)};
static const uint8_t opinf_di[]   = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xF3)};
static const uint8_t opinf_ei[]   = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xFB)};
static const uint8_t opinf_exx[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xD9)};
static const uint8_t opinf_halt[] = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x76)};
static const uint8_t opinf_ind[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xAA)};
static const uint8_t opinf_indr[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xBA)};
static const uint8_t opinf_ini[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA2)};
static const uint8_t opinf_inir[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB2)};
static const uint8_t opinf_ldd[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA8)};
static const uint8_t opinf_lddr[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB8)};
static const uint8_t opinf_ldi[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA0)};
static const uint8_t opinf_ldir[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB0)};
static const uint8_t opinf_neg[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x44)};
static const uint8_t opinf_nop[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x00)};
static const uint8_t opinf_otdr[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xBB)};
static const uint8_t opinf_otir[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xB3)};
static const uint8_t opinf_outd[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xAB)};
static const uint8_t opinf_outi[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0xA3)};
static const uint8_t opinf_ret[]  = {
    DESC(1, OD_PREFIX_NONE, OD_AT_53_COND, OD_AT_NONE, 0xC0), // RET cc
    DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0xC9)     // RET
};
static const uint8_t opinf_reti[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x4D)};
static const uint8_t opinf_retn[] = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x45)};
static const uint8_t opinf_rla[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x17)};
static const uint8_t opinf_rlca[] = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x07)};
static const uint8_t opinf_rld[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x6F)};
static const uint8_t opinf_rra[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x1F)};
static const uint8_t opinf_rrca[] = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x0F)};
static const uint8_t opinf_rrd[]  = {DESC(0, OD_PREFIX_ED, OD_AT_NONE, OD_AT_NONE, 0x67)};
static const uint8_t opinf_scf[]  = {DESC(0, OD_PREFIX_NONE, OD_AT_NONE, OD_AT_NONE, 0x37)};

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
