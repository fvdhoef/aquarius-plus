#ifndef _OPCODE_INFO_H
#define _OPCODE_INFO_H

#include "common.h"
#include "tokens.h"

#define OD_PREFIX_NONE 0
#define OD_PREFIX_CB   1
#define OD_PREFIX_ED   2
#define OD_2BYTE       3

enum {
    OD_AT_NONE = 0,       // no arguments
    OD_AT_IMM_0,          // immediate value 0, non-encoded
    OD_AT_53_IMM,         // immediate value (0-7) encoded in 5:3
    OD_AT_43_IMODE,       // interrupt mode encoded in 4:3 - 0:IM0,2:IM1,3:IM2
    OD_AT_53_RST,         // reset offset encoded in 5:3 - 0:$00,1:$08,2:$10,3:$18,4:$20,5:$28,6:$30,7:$38
    OD_AT_IMM8,           //  8-bit immediate
    OD_AT_IMM8_IND,       //  8-bit immediate indirect
    OD_AT_IMM16,          // 16-bit immediate
    OD_AT_IMM16_IND,      // 16-bit immediate indirect
    OD_AT_REL_ADDR,       //  8-bit relative address
    OD_AT_A,              //  8-bit register a
    OD_AT_R,              //  8-bit register r
    OD_AT_I,              //  8-bit register i
    OD_AT_DE,             // 16-bit register de
    OD_AT_HL,             // 16-bit register hl
    OD_AT_SP,             // 16-bit register sp
    OD_AT_AF,             // 16-bit register af
    OD_AT_AF_ALT,         // 16-bit register af'
    OD_AT_C_IND,          //  8-bit register (c)
    OD_AT_BC_IND,         // 16-bit register (bc)
    OD_AT_DE_IND,         // 16-bit register (de)
    OD_AT_HL_IND,         // 16-bit register (hl)
    OD_AT_SP_IND,         // 16-bit register (sp)
    OD_AT_20_REG_ALL,     //  8-bit register encoded in 2:0 - 0:b,1:c,2:d,3:e,4:h,5:l,6:(hl),7:a
    OD_AT_20_BCDEHLA,     //  8-bit register encoded in 2:0 - 0:b,1:c,2:d,3:e,4:h,5:l,7:a
    OD_AT_53_REG_ALL,     //  8-bit register encoded in 5:3 - 0:b,1:c,2:d,3:e,4:h,5:l,6:(hl),7:a
    OD_AT_53_BCDEHLFA,    //  8-bit register encoded in 5:3 - 0:b,1:c,2:d,3:e,4:h,5:l,6:f,7:a
    OD_AT_53_BCDEHLA,     //  8-bit register encoded in 5:3 - 0:b,1:c,2:d,3:e,4:h,5:l,7:a
    OD_AT_53_COND,        // condition       encoded in 5:3 - 0:nz,1:z,2:nc,3:c,4:po,5:pe,6:p,7:m
    OD_AT_43_COND,        // condition       encoded in 4:3 - 0:nz,1:z,2:nc,3:c
    OD_AT_54_BC_DE_HL_AF, // 16-bit register encoded in 5:4 - 0:bc,1:de,2:hl,3:af
    OD_AT_54_BC_DE_HL_SP, // 16-bit register encoded in 5:4 - 0:bc,1:de,2:hl,3:sp
    OD_AT_IX,             // 16-bit register ix
    OD_AT_IX_IND,         // 16-bit register (ix)
    OD_AT_IX_IND_OFFS,    // 16-bit register (ix+d)
    OD_AT_20_IXHL,        //  8-bit register encoded in 2:0 - 4:ixh,5:ixl
    OD_AT_53_IXHL,        //  8-bit register encoded in 5:3 - 4:ixh,5:ixl
    OD_AT_20_IXHL_ALL,    //  8-bit register encoded in 2:0 - 0:b,1:c,2:d,3:e,4:ixh,5:ixl,7:a
    OD_AT_53_IXHL_ALL,    //  8-bit register encoded in 5:3 - 0:b,1:c,2:d,3:e,4:ixh,5:ixl,7:a
    OD_AT_54_BC_DE_IY_SP, // 16-bit register encoded in 5:4 - 0:bc,1:de,2:ix,3:sp
    OD_AT_IY,             // 16-bit register iy
    OD_AT_IY_IND,         // 16-bit register (iy)
    OD_AT_IY_IND_OFFS,    // 16-bit register (iy+d)
    OD_AT_20_IYHL,        //  8-bit register encoded in 2:0 - 4:iyh,5:iyl
    OD_AT_53_IYHL,        //  8-bit register encoded in 5:3 - 4:iyh,5:iyl
    OD_AT_20_IYHL_ALL,    //  8-bit register encoded in 2:0 - 0:b,1:c,2:d,3:e,4:iyh,5:iyl,7:a
    OD_AT_53_IYHL_ALL,    //  8-bit register encoded in 5:3 - 0:b,1:c,2:d,3:e,4:iyh,5:iyl,7:a
    OD_AT_54_BC_DE_IX_SP, // 16-bit register encoded in 5:4 - 0:bc,1:de,2:iy,3:sp
};

extern const uint8_t *opcode_info[TOK_OPCODE_LAST - TOK_OPCODE_FIRST + 1];

#endif
