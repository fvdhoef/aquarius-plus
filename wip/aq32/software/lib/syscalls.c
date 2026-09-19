#include "common.h"
#include "console.h"
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include "esp.h"
#include "malloc.h"

#define FD_ESP_START 10
#define XFER_MAX     0xF000

static mspace *heap_space;

static void assure_init(void) {
    if (heap_space)
        return;

#ifdef SIZE_BUF_HEAP
    static __attribute__((section(".noinit"))) uint8_t buf_heap[SIZE_BUF_HEAP];
    heap_space = create_mspace_with_base(buf_heap, sizeof(buf_heap), 0);
#else
    extern char _end;
    extern char __stack_start;
    heap_space = create_mspace_with_base(&_end, &__stack_start - &_end, false);
#endif
}

static void *_malloc(size_t sz) {
    assure_init();
    return mspace_malloc(heap_space, sz);
}
static void *_calloc(size_t n_elem, size_t elem_sz) {
    assure_init();
    return mspace_calloc(heap_space, n_elem, elem_sz);
}
static void *_realloc(void *p, size_t sz) {
    assure_init();
    return mspace_realloc(heap_space, p, sz);
}
static void _free(void *p) {
    assure_init();
    return mspace_free(heap_space, p);
}

void *_malloc_r(struct _reent *, size_t sz) { return _malloc(sz); }
void *_calloc_r(struct _reent *, size_t n_elem, size_t elem_sz) { return _calloc(n_elem, elem_sz); }
void *_realloc_r(struct _reent *, void *p, size_t sz) { return _realloc(p, sz); }
void  _free_r(struct _reent *, void *p) { _free(p); }

void *malloc(size_t sz) { return _malloc(sz); }
void *calloc(size_t n_elem, size_t elem_sz) { return _calloc(n_elem, elem_sz); }
void *realloc(void *p, size_t sz) { return _realloc(p, sz); }
void  free(void *p) { return _free(p); }

int _isatty(int fd) {
    return (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO);
}

int _open(const char *path, int flags) {
    uint8_t esp_flags = 0;

    switch (flags & O_ACCMODE) {
        case O_RDONLY: esp_flags = 0; break;
        case O_WRONLY: esp_flags = 1; break;
        case O_RDWR: esp_flags = 2; break;
    }
    if (flags & O_APPEND)
        esp_flags |= 0x4;
    if (flags & O_CREAT)
        esp_flags |= 0x8;
    if (flags & O_TRUNC)
        esp_flags |= 0x10;
    if (flags & O_EXCL)
        esp_flags |= 0x20;

    int result = esp_open(path, esp_flags);
    if (result < 0)
        return esp_set_errno(result);

    return FD_ESP_START + result;
}

ssize_t _read(int fd, void *buf, size_t count) {
    if (fd >= FD_ESP_START) {
        int result = esp_read(fd - FD_ESP_START, buf, count);
        if (result == ERR_EOF)
            result = 0;

        if (result < 0 && result != ERR_EOF)
            return esp_set_errno(result);

        return result;

    } else if (fd == STDIN_FILENO) {
        uint8_t *p = buf;

        while (count) {
            uint8_t val = console_getc();
            if (val == 0) {
                // No data
                if (p > (uint8_t *)buf) {
                    // Return what we got so far
                    break;
                }
            } else {
                *(p++) = val;
                count--;
            }
        }
        return p - (uint8_t *)buf;

    } else {
        errno = EINVAL;
        return -1;
    }
}

int _write(int fd, const void *buf, size_t count) {
    if (fd >= FD_ESP_START) {
        int result = esp_write(fd - FD_ESP_START, buf, count);
        if (result < 0)
            return esp_set_errno(result);
        return result;

    } else if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        const uint8_t *p = buf;
        while (count--) {
            uint8_t ch = *(p++);
            if (ch == '\n') {
                console_putc('\r');
            }
            console_putc(ch);
        }
        return p - (uint8_t *)buf;

    } else {
        errno = EINVAL;
        return -1;
    }
}

off_t _lseek(int fd, off_t offset, int whence) {
    if (whence < 0 || whence > 2) {
        errno = EINVAL;
        return -1;
    }

    if (fd >= FD_ESP_START) {
        int result = esp_lseek(fd - FD_ESP_START, offset, whence);
        if (result < 0)
            return esp_set_errno(result);

        return result;

    } else {
        errno = ESPIPE;
        return -1;
    }
    return 0;
}

int _close(int fd) {
    if (fd >= FD_ESP_START) {
        int result = esp_close(fd - FD_ESP_START);
        if (result < 0)
            return esp_set_errno(result);
        return 0;
    }

    errno = EINVAL;
    return -1;
}

noreturn void _exit(int status) {
    STARTDATA->exit_status = status;
    load_executable("/cores/aq32/shell.aq32");
    while (1);
}

int _kill(pid_t pid, int sig) {
    return 0;
}

int _getpid(void) {
    return 0;
}

int _fstat(int fd, struct stat *st) {
    errno = ENOENT;
    return -1;
}

int _lstat(const char *path, struct stat *st) {
    errno = ENOENT;
    return -1;
}

int _ftime(struct timeb *tp) {
    errno = EIO;
    return -1;
}

int _access(const char *file, int mode) {
    errno = ENOENT;
    return -1;
}

int _faccessat(int dirfd, const char *file, int mode, int flags) {
    errno = ENOENT;
    return -1;
}

int _fstatat(int dirfd, const char *file, struct stat *st, int flags) {
    errno = ENOENT;
    return -1;
}

int _gettimeofday(struct timeval *tp, void *tzp) {
    errno = EIO;
    return -1;
}

int _link(const char *old_name, const char *new_name) {
    errno = ENOENT;
    return -1;
}

int _openat(int dirfd, const char *name, int flags, int mode) {
    errno = ENOENT;
    return -1;
}

int _stat(const char *file, struct stat *st) {
    errno = ENOENT;
    return -1;
}

char *getcwd(char *buf, size_t size) {
    esp_cmd(ESPCMD_GETCWD);
    int result = (int8_t)esp_get_byte();
    if (result < 0) {
        esp_set_errno(result);
        return NULL;
    }

    char *p  = buf;
    char *p2 = buf + size - 1;
    while (1) {
        uint8_t ch = esp_get_byte();
        if (p < p2) {
            *(p++) = ch;
            if (ch == 0)
                break;
        } else {
            buf = NULL;
        }
    }
    return buf;
}

int chdir(const char *path) {
    int result = esp_chdir(path);
    if (result < 0)
        return esp_set_errno(result);
    return 0;
}

int mkdir(const char *path, mode_t mode) {
    int result = esp_mkdir(path);
    if (result < 0)
        return esp_set_errno(result);
    return 0;
}

int rmdir(const char *path) {
    struct esp_stat st;
    int             result = esp_stat(path, &st);
    if (result < 0)
        return esp_set_errno(result);

    if ((st.attr & DE_ATTR_DIR) == 0) {
        errno = ENOTDIR;
        return -1;
    }

    result = esp_delete(path);
    if (result < 0)
        return esp_set_errno(result);
    return 0;
}

int _unlink(const char *path) {
    struct esp_stat st;
    int             result = esp_stat(path, &st);
    if (result < 0)
        return esp_set_errno(result);

    if ((st.attr & DE_ATTR_DIR) != 0) {
        errno = EISDIR;
        return -1;
    }

    result = esp_delete(path);
    if (result < 0)
        return esp_set_errno(result);
    return 0;
}

extern int main(int argc, char **argv);

void _c_start(void) {
    // Disable buffering of stdio
#ifndef NO_LIB_INIT
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    console_init();
#endif

    int status = main(STARTDATA->argc, STARTDATA->argv);
    exit(status);
}
