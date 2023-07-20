// Platform-independent directory enumeration
// Supports Windows, Linux, macOS

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

enum {
    DE_DIR = (1 << 0),
};

struct direnum_ent {
    char    *filename;
    uint32_t size;
    uint8_t  attr;
    uint16_t ftime;
    uint16_t fdate;
};

typedef void *direnum_ctx_t;

direnum_ctx_t       direnum_open(const char *path);
struct direnum_ent *direnum_read(direnum_ctx_t ctx);
void                direnum_close(direnum_ctx_t ctx);
