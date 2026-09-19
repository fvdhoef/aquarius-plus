#pragma once

#include "common.h"
#include "mem_access.h"

#define SIZE_BUF_BYTECODE 0x10000

extern uint8_t  buf_bytecode[SIZE_BUF_BYTECODE];
extern uint8_t *buf_bytecode_end;

static inline uint16_t buf_bytecode_get_cur_offset(void) {
    return buf_bytecode_end - buf_bytecode;
}

static inline void buf_bytecode_patch_u16(uint16_t offset, uint16_t value) {
    write_u16(buf_bytecode + offset, value);
}

void  buf_reinit(void);
void *buf_malloc(size_t sz);
void *buf_calloc(size_t sz);
void *buf_realloc(void *p, size_t sz);
void  buf_free(void *p);
