#include "direnum.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ff.h>

struct direnum_ctx {
    struct direnum_ent *entries;
    unsigned            cap_entries;
    unsigned            num_entries;
    unsigned            read_idx;
};

static void free_ctx(struct direnum_ctx *ctx) {
    if (ctx) {
        if (ctx->entries) {
            for (unsigned i = 0; i < ctx->num_entries; i++) {
                free(ctx->entries[i].filename);
            }
            free(ctx->entries);
        }
        free(ctx);
    }
}

static int de_compare(const void *a, const void *b) {
    const struct direnum_ent *de_a = a;
    const struct direnum_ent *de_b = b;

    // Sort directories at the top
    if ((de_a->attr & DE_DIR) != (de_b->attr & DE_DIR)) {
        return (de_a->attr & DE_DIR) ? -1 : 1;
    }
    // Sort case insensitive
    return strcasecmp(de_a->filename, de_b->filename);
}

static struct direnum_ctx *readall(const char *path) {
    bool                dir_valid = false;
    struct direnum_ctx *ctx       = NULL;

    // Allocate context
    if ((ctx = calloc(1, sizeof(*ctx))) == NULL)
        goto error;

    // Open directory
    FF_DIR dir;
    if (f_opendir(&dir, path) != F_OK) {
        free(ctx);
        goto error;
    }
    dir_valid = true;

    // Read directory contents
    FILINFO fno;
    while (1) {
        if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) {
            // Done
            break;
        }

        // Skip hidden and system files
        if ((fno.fattrib & (AM_SYS | AM_HID)))
            continue;

        // Allocate enough space to fit the directory entries
        if (ctx->num_entries == ctx->cap_entries) {
            ctx->cap_entries += 32;
            void *newbuf = realloc(ctx->entries, sizeof(*ctx->entries) * ctx->cap_entries);
            if (newbuf == NULL)
                goto error;
            ctx->entries = newbuf;
        }

        struct direnum_ent *de = &ctx->entries[ctx->num_entries++];
        if ((de->filename = strdup(fno.fname)) == NULL)
            goto error;

        de->size  = fno.fsize;
        de->attr  = (fno.fattrib & AM_DIR) ? DE_DIR : 0;
        de->ftime = fno.ftime;
        de->fdate = fno.fdate;
    }

    // Close directory
    f_closedir(&dir);
    dir_valid = false;

    // Sort directory entries
    qsort(ctx->entries, ctx->num_entries, sizeof(*ctx->entries), de_compare);

    return ctx;

error:
    if (dir_valid) {
        f_closedir(&dir);
    }
    free_ctx(ctx);
    return NULL;
}

direnum_ctx_t direnum_open(const char *path) {
    if (strncmp(path, "/sdcard/", 8) != 0) {
        return NULL;
    }
    path += 8;

    return readall(path);
}

struct direnum_ent *direnum_read(direnum_ctx_t _ctx) {
    if (_ctx == NULL) {
        return false;
    }
    struct direnum_ctx *ctx = (struct direnum_ctx *)_ctx;

    if (ctx->read_idx == ctx->num_entries) {
        return NULL;
    }
    return &ctx->entries[ctx->read_idx++];
}

void direnum_close(direnum_ctx_t _ctx) {
    if (_ctx == NULL) {
        return;
    }
    struct direnum_ctx *ctx = (struct direnum_ctx *)_ctx;
    free_ctx(ctx);
}
