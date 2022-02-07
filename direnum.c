#include "direnum.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <dirent.h>
#    include <unistd.h>

struct direnum_ctx {
    DIR  *dir;
    char *path;
};

#else
#    include <io.h>
#    include <time.h>

struct direnum_ctx {
    intptr_t           handle;
    struct _finddata_t fileinfo;
    bool               first;
};
#endif

direnum_ctx_t *direnum_open(const char *path) {
    struct direnum_ctx *ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        perror("direnum_open");
        exit(1);
    }

#ifndef _WIN32
    ctx->dir = opendir(path);
    if (ctx->dir == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->path = strdup(path);
#else
    char path2[1024];
    snprintf(path2, sizeof(path2), "%s/*.*", path);

    ctx->handle = _findfirst(path2, &ctx->fileinfo);
    if (ctx->handle == 0) {
        free(ctx);
        return NULL;
    }
    ctx->first = true;

#endif

    return (direnum_ctx_t *)ctx;
}

bool direnum_read(direnum_ctx_t *_ctx, struct direnum_ent *dee) {
    if (_ctx == NULL || dee == NULL) {
        return false;
    }
    struct direnum_ctx *ctx = (struct direnum_ctx *)_ctx;

#ifndef _WIN32
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
#    ifdef __APPLE__
    dee->t = st.st_mtimespec.tv_sec;
#    else
    dee->t = st.st_mtim.tv_sec;
#    endif
#else
    if (!ctx->first) {
        if (_findnext(ctx->handle, &ctx->fileinfo) != 0) {
            return false;
        }
    }
    ctx->first = false;

    // Return file entry
    snprintf(dee->filename, sizeof(dee->filename), "%s", ctx->fileinfo.name);
    dee->size = (ctx->fileinfo.attrib & _A_SUBDIR) ? 0 : ctx->fileinfo.size;
    dee->attr = (ctx->fileinfo.attrib & _A_SUBDIR) ? DE_DIR : 0;
    dee->t    = ctx->fileinfo.time_write;
#endif

    return true;
}

void direnum_close(direnum_ctx_t *_ctx) {
    if (_ctx == NULL) {
        return;
    }
    struct direnum_ctx *ctx = (struct direnum_ctx *)_ctx;
#ifndef _WIN32
    closedir(ctx->dir);
    free(ctx->path);
#else
    _findclose(ctx->handle);
#endif
    free(ctx);
}
