#include "buffers.h"
#include "malloc.h"

__attribute__((section(".noinit"))) uint8_t buf_bytecode[SIZE_BUF_BYTECODE];
uint8_t                                    *buf_bytecode_end;

static mspace *arena;

void buf_reinit(void) {
    extern char _end;
    extern char __stack_start;
    arena = create_mspace_with_base(&_end, &__stack_start - &_end, false);
}

void *buf_malloc(size_t sz) { return mspace_malloc(arena, sz); }
void *buf_calloc(size_t sz) { return mspace_calloc(arena, 1, sz); }
void *buf_realloc(void *p, size_t sz) { return mspace_realloc(arena, p, sz); }
void  buf_free(void *p) { mspace_free(arena, p); }
