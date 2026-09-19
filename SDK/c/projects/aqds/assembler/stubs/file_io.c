#include "file_io.h"
#include "regs.h"
#include <stdio.h>
#include <errno.h>

#define MAX_FDS (10)

struct state {
    FILE *fds[MAX_FDS];
};
static struct state state;

static int mapErrNoResult(void) {
    switch (errno) {
        case EEXIST: return ERR_EXISTS;
        case EACCES:
        case ENOENT: return ERR_NOT_FOUND;
        case EINVAL:
        case EBADF: return ERR_PARAM;
        case ENOTEMPTY: return ERR_NOT_EMPTY;
    }
    return ERR_OTHER;
}

int8_t esp_open(const char *path, uint8_t flags) {
    // Translate flags
    int  mi = 0;
    char mode[5];

    // if (flags & FO_CREATE)
    //     oflag |= O_CREAT;

    switch (flags & FO_ACCMODE) {
        case FO_RDONLY:
            mode[mi++] = 'r';
            break;
        case FO_WRONLY:
            if (flags & FO_APPEND) {
                mode[mi++] = 'a';
            } else {
                mode[mi++] = 'w';
            }
            if (flags & FO_EXCL) {
                mode[mi++] = 'x';
            }

            break;
        case FO_RDWR:
            if (flags & FO_APPEND) {
                mode[mi++] = 'a';
                mode[mi++] = '+';
            } else if (flags & FO_TRUNC) {
                mode[mi++] = 'w';
                mode[mi++] = '+';
            } else {
                mode[mi++] = 'r';
                mode[mi++] = '+';
            }
            if (flags & FO_EXCL) {
                mode[mi++] = 'x';
            }
            break;

        default: {
            // Error
            return ERR_PARAM;
        }
    }
    mode[mi++] = 'b';
    mode[mi]   = 0;

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (state.fds[i] == NULL) {
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return ERR_TOO_MANY_OPEN;

    FILE *f = fopen(path, mode);
    if (f == NULL) {
        return mapErrNoResult();
    }
    state.fds[fd] = f;
    return fd;
}
int8_t esp_close(int8_t fd) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    fclose(f);
    state.fds[fd] = NULL;
    return 0;
}
int16_t esp_read(int8_t fd, void *buf, uint16_t length) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)fread(buf, 1, length, f);
    return (result < 0) ? mapErrNoResult() : result;
}
int16_t esp_readline(int8_t fd, void *buf, uint16_t max_length) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;

    FILE *f = state.fds[fd];
    if (fgets((char *)buf, max_length, f) == NULL) {
        if (feof(f))
            return ERR_EOF;
        errno = ferror(f);
        return mapErrNoResult();
    }

    char *p = buf;
    while (*p) {
        if (*p == '\r' || *p == '\n')
            break;
        p++;
    }
    *p = 0;

    return p - (char *)buf;
}
int16_t esp_write(int8_t fd, const void *buf, uint16_t length) {
    if (fd >= MAX_FDS || state.fds[fd] == NULL)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)fwrite(buf, 1, length, f);
    return (result < 0) ? mapErrNoResult() : result;
}
