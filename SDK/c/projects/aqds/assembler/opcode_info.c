#include "opcode_info.h"

// Opcode descriptor:
//
//    7     6     5     4     3     2     1     0
// +-----+-----------------+-----------------------+
// |More |   Prefix type   |     Argument type     |
// +-----+-----------------+-----------------------+
//
//    More:
//      0: Last entry
//      1: More entries following
//
//    Prefix type:
//      0: Instruction not implemented
//      1: None
//      2: CB
//      3: DDCB     // Always (IX+d)
//      4: FDCB     // Always (IY+d)
//      5: DD
//      6: ED
//      7: FD
//
//    Argument type:
//      0: No arguments (implicit last entry)
//      1: 8-bit direct
//      2: 16-bit direct
//      F: Argument descriptor following
//
//    Num args: 0/1/2
//
// Argument descriptor:
//
//    7     6     5     4     3     2     1     0
// +-----------------------+-----------------------+
// |    Argument 1 type    |    Argument 2 type    |
// +-----------------------+-----------------------+
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

static const uint8_t opinf_adc[]  = {0};
static const uint8_t opinf_add[]  = {0};
static const uint8_t opinf_and[]  = {0};
static const uint8_t opinf_bit[]  = {0};
static const uint8_t opinf_call[] = {0};
static const uint8_t opinf_cp[]   = {0};
static const uint8_t opinf_dec[]  = {0};
static const uint8_t opinf_djnz[] = {0};
static const uint8_t opinf_ex[]   = {0};
static const uint8_t opinf_im[]   = {0};
static const uint8_t opinf_in[]   = {0};
static const uint8_t opinf_inc[]  = {0};
static const uint8_t opinf_jp[]   = {PREFIX_NONE | ARG_IMM16, 0xC3};
static const uint8_t opinf_jr[]   = {0};
static const uint8_t opinf_ld[]   = {0};
static const uint8_t opinf_or[]   = {0};
static const uint8_t opinf_out[]  = {0};
static const uint8_t opinf_pop[]  = {0};
static const uint8_t opinf_push[] = {0};
static const uint8_t opinf_res[]  = {0};
static const uint8_t opinf_rl[]   = {0};
static const uint8_t opinf_rlc[]  = {0};
static const uint8_t opinf_rr[]   = {0};
static const uint8_t opinf_rrc[]  = {0};
static const uint8_t opinf_rst[]  = {0};
static const uint8_t opinf_sbc[]  = {0};
static const uint8_t opinf_set[]  = {0};
static const uint8_t opinf_sla[]  = {0};
static const uint8_t opinf_sll[]  = {0};
static const uint8_t opinf_sra[]  = {0};
static const uint8_t opinf_srl[]  = {0};
static const uint8_t opinf_sub[]  = {0};
static const uint8_t opinf_xor[]  = {0};

static const uint8_t opinf_ccf[]  = {PREFIX_NONE | ARG_NONE, 0x3F};
static const uint8_t opinf_cpd[]  = {PREFIX_ED | ARG_NONE, 0xA9};
static const uint8_t opinf_cpdr[] = {PREFIX_ED | ARG_NONE, 0xB9};
static const uint8_t opinf_cpi[]  = {PREFIX_ED | ARG_NONE, 0xA1};
static const uint8_t opinf_cpir[] = {PREFIX_ED | ARG_NONE, 0xB1};
static const uint8_t opinf_cpl[]  = {PREFIX_NONE | ARG_NONE, 0x2F};
static const uint8_t opinf_daa[]  = {PREFIX_NONE | ARG_NONE, 0x27};
static const uint8_t opinf_di[]   = {PREFIX_NONE | ARG_NONE, 0xF3};
static const uint8_t opinf_ei[]   = {PREFIX_NONE | ARG_NONE, 0xFB};
static const uint8_t opinf_exx[]  = {PREFIX_NONE | ARG_NONE, 0xD9};
static const uint8_t opinf_halt[] = {PREFIX_NONE | ARG_NONE, 0x76};
static const uint8_t opinf_ind[]  = {PREFIX_ED | ARG_NONE, 0xAA};
static const uint8_t opinf_indr[] = {PREFIX_ED | ARG_NONE, 0xBA};
static const uint8_t opinf_ini[]  = {PREFIX_ED | ARG_NONE, 0xA2};
static const uint8_t opinf_inir[] = {PREFIX_ED | ARG_NONE, 0xB2};
static const uint8_t opinf_ldd[]  = {PREFIX_ED | ARG_NONE, 0xA8};
static const uint8_t opinf_lddr[] = {PREFIX_ED | ARG_NONE, 0xB8};
static const uint8_t opinf_ldi[]  = {PREFIX_ED | ARG_NONE, 0xA0};
static const uint8_t opinf_ldir[] = {PREFIX_ED | ARG_NONE, 0xB0};
static const uint8_t opinf_neg[]  = {PREFIX_ED | ARG_NONE, 0x44};
static const uint8_t opinf_nop[]  = {PREFIX_NONE | ARG_NONE, 0x00};
static const uint8_t opinf_otdr[] = {PREFIX_ED | ARG_NONE, 0xBB};
static const uint8_t opinf_otir[] = {PREFIX_ED | ARG_NONE, 0xB3};
static const uint8_t opinf_outd[] = {PREFIX_ED | ARG_NONE, 0xAB};
static const uint8_t opinf_outi[] = {PREFIX_ED | ARG_NONE, 0xA3};
static const uint8_t opinf_ret[]  = {PREFIX_NONE | ARG_NONE, 0xC9};
static const uint8_t opinf_reti[] = {PREFIX_NONE | ARG_NONE, 0xED, 0x4D};
static const uint8_t opinf_retn[] = {PREFIX_NONE | ARG_NONE, 0xED, 0x45};
static const uint8_t opinf_rla[]  = {PREFIX_NONE | ARG_NONE, 0x17};
static const uint8_t opinf_rlca[] = {PREFIX_NONE | ARG_NONE, 0x07};
static const uint8_t opinf_rld[]  = {PREFIX_ED | ARG_NONE, 0x6F};
static const uint8_t opinf_rra[]  = {PREFIX_NONE | ARG_NONE, 0x1F};
static const uint8_t opinf_rrca[] = {PREFIX_NONE | ARG_NONE, 0x0F};
static const uint8_t opinf_rrd[]  = {PREFIX_ED | ARG_NONE, 0x67};
static const uint8_t opinf_scf[]  = {PREFIX_NONE | ARG_NONE, 0x37};

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
