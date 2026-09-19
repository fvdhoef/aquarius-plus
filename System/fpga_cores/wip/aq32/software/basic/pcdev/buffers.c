#include "basic/common/buffers.h"
#include "malloc.h"

uint8_t  buf_bytecode[SIZE_BUF_BYTECODE];
uint8_t *buf_bytecode_end;

void  buf_reinit(void) {}
void *buf_malloc(size_t sz) { return malloc(sz); }
void *buf_calloc(size_t sz) { return calloc(1, sz); }
void *buf_realloc(void *p, size_t sz) { return realloc(p, sz); }
void  buf_free(void *p) { free(p); }
