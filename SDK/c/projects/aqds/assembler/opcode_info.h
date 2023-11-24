#ifndef _OPCODE_INFO_H
#define _OPCODE_INFO_H

#include "common.h"
#include "tokens.h"

#define PREFIX_MASK  0x70
#define ARGTYPE_MASK 0x0F

#define MORE        0x80
#define PREFIX_NONE 0x10
#define PREFIX_CB   0x20
#define PREFIX_DDCB 0x30
#define PREFIX_FDCB 0x40
#define PREFIX_DD   0x50
#define PREFIX_ED   0x60
#define PREFIX_FD   0x70

#define ARG_NONE  0
#define ARG_IMM8  1
#define ARG_IMM16 2

extern const uint8_t *opcode_info[TOK_OPCODE_LAST - TOK_OPCODE_FIRST + 1];

#endif
