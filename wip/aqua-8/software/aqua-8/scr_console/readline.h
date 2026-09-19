#pragma once

#include "common.h"

typedef struct {
    char    *buf;
    unsigned buf_size;
    unsigned len;
    unsigned idx;
} readline_ctx_t;

void readline_init(readline_ctx_t *ctx, char *buf, size_t buf_size);
int  readline_process(readline_ctx_t *ctx, uint8_t ch);
