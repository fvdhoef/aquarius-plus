#include "direnum.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

struct direnum_ctx {
    DIR  *dir;
    char *path;
};

direnum_ctx_t direnum_open(const char *path) {
    struct direnum_ctx *ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        perror("direnum_open");
        exit(1);
    }

    ctx->dir = opendir(path);
    if (ctx->dir == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->path = strdup(path);

    return (direnum_ctx_t)ctx;
}

bool direnum_read(direnum_ctx_t _ctx, struct direnum_ent *dee) {
    if (_ctx == NULL || dee == NULL) {
        return false;
    }
    struct direnum_ctx *ctx = (struct direnum_ctx *)_ctx;

    // Read directory entry
    struct dirent *de = readdir(ctx->dir);
    if (de == NULL) {
        return false;
    }

    // Read additional file stats
    size_t fullpath_len = strlen(ctx->path) + strlen(de->d_name) + 2;
    char  *fullpath     = malloc(fullpath_len);
    snprintf(fullpath, fullpath_len, "%s/%s", ctx->path, de->d_name);

    struct stat st;
    int         result = stat(fullpath, &st);
    free(fullpath);
    if (result < 0) {
        return false;
    }

    // Return file entry
    snprintf(dee->filename, sizeof(dee->filename), "%s", de->d_name);
    dee->size = (de->d_type == DT_DIR) ? 0 : st.st_size;
    dee->attr = (de->d_type == DT_DIR) ? DE_DIR : 0;
    dee->t    = st.st_mtim.tv_sec;
    return true;
}

void direnum_close(direnum_ctx_t _ctx) {
    if (_ctx == NULL) {
        return;
    }
    struct direnum_ctx *ctx = (struct direnum_ctx *)_ctx;
    closedir(ctx->dir);
    free(ctx->path);
    free(ctx);
}
