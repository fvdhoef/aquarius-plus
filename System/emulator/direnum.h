// Platform-independent directory enumeration
// Supports Windows, Linux, macOS

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "VFS.h"

struct direnum_ent {
    char     filename[1024];
    uint32_t size;
    uint8_t  attr;
    time_t   t;
};

typedef void *direnum_ctx_t;

direnum_ctx_t direnum_open(const char *path);
bool          direnum_read(direnum_ctx_t ctx, struct direnum_ent *dee);
void          direnum_close(direnum_ctx_t ctx);
