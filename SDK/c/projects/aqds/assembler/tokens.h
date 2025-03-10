#ifndef _TOKENS_H
#define _TOKENS_H

#include <stdint.h>

extern const uint8_t *keywords[];
extern const uint8_t  num_keywords[];

enum {
    TOK_UNKNOWN = 0,

    // Directives
    TOK_DIR_FIRST = 1,
    TOK_ASSERT    = 1,
    TOK_DB        = 2,
    TOK_DEFB      = 3,
    TOK_DEFS      = 4,
    TOK_DEFW      = 5,
    TOK_DEPHASE   = 6,
    TOK_DS        = 7,
    TOK_DW        = 8,
    TOK_END       = 9,
    TOK_EQU       = 10,
    TOK_INCBIN    = 11,
    TOK_INCLUDE   = 12,
    TOK_ORG       = 13,
    TOK_PHASE     = 14,
    TOK_DIR_LAST  = 14,

    // Opcodes
    TOK_OPCODE_FIRST = 15,
    TOK_ADC          = 15,
    TOK_ADD          = 16,
    TOK_AND          = 17,
    TOK_BIT          = 18,
    TOK_CALL         = 19,
    TOK_CCF          = 20,
    TOK_CP           = 21,
    TOK_CPD          = 22,
    TOK_CPDR         = 23,
    TOK_CPI          = 24,
    TOK_CPIR         = 25,
    TOK_CPL          = 26,
    TOK_DAA          = 27,
    TOK_DEC          = 28,
    TOK_DI           = 29,
    TOK_DJNZ         = 30,
    TOK_EI           = 31,
    TOK_EX           = 32,
    TOK_EXX          = 33,
    TOK_HALT         = 34,
    TOK_IM           = 35,
    TOK_IN           = 36,
    TOK_INC          = 37,
    TOK_IND          = 38,
    TOK_INDR         = 39,
    TOK_INI          = 40,
    TOK_INIR         = 41,
    TOK_JP           = 42,
    TOK_JR           = 43,
    TOK_LD           = 44,
    TOK_LDD          = 45,
    TOK_LDDR         = 46,
    TOK_LDI          = 47,
    TOK_LDIR         = 48,
    TOK_NEG          = 49,
    TOK_NOP          = 50,
    TOK_OR           = 51,
    TOK_OTDR         = 52,
    TOK_OTIR         = 53,
    TOK_OUT          = 54,
    TOK_OUTD         = 55,
    TOK_OUTI         = 56,
    TOK_POP          = 57,
    TOK_PUSH         = 58,
    TOK_RES          = 59,
    TOK_RET          = 60,
    TOK_RETI         = 61,
    TOK_RETN         = 62,
    TOK_RL           = 63,
    TOK_RLA          = 64,
    TOK_RLC          = 65,
    TOK_RLCA         = 66,
    TOK_RLD          = 67,
    TOK_RR           = 68,
    TOK_RRA          = 69,
    TOK_RRC          = 70,
    TOK_RRCA         = 71,
    TOK_RRD          = 72,
    TOK_RST          = 73,
    TOK_SBC          = 74,
    TOK_SCF          = 75,
    TOK_SET          = 76,
    TOK_SLA          = 77,
    TOK_SLL          = 78,
    TOK_SRA          = 79,
    TOK_SRL          = 80,
    TOK_SUB          = 81,
    TOK_XOR          = 82,
    TOK_OPCODE_LAST  = 82,
};

#endif
