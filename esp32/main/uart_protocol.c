#include "uart_protocol.h"
#include <driver/uart.h>
#include "util.h"
#include "direnum.h"
#include "sdcard.h"
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

static const char *TAG = "uart_protocol";
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define UART_NUM (UART_NUM_1)

#if 1
#    define DBGF(...) printf(__VA_ARGS__)
#else
#    define DBGF(...)
#endif

enum {
    ESPCMD_RESET    = 0x01, // Reset ESP
    ESPCMD_OPEN     = 0x10, // Open / create file
    ESPCMD_CLOSE    = 0x11, // Close open file
    ESPCMD_READ     = 0x12, // Read from file
    ESPCMD_WRITE    = 0x13, // Write to file
    ESPCMD_SEEK     = 0x14, // Move read/write pointer
    ESPCMD_TELL     = 0x15, // Get current read/write
    ESPCMD_OPENDIR  = 0x16, // Open directory
    ESPCMD_CLOSEDIR = 0x17, // Close open directory
    ESPCMD_READDIR  = 0x18, // Read from directory
    ESPCMD_DELETE   = 0x19, // Remove file or directory
    ESPCMD_RENAME   = 0x1A, // Rename / move file or directory
    ESPCMD_MKDIR    = 0x1B, // Create directory
    ESPCMD_CHDIR    = 0x1C, // Change directory
    ESPCMD_STAT     = 0x1D, // Get file status
    ESPCMD_GETCWD   = 0x1E, // Get current working directory
    ESPCMD_CLOSEALL = 0x1F, // Close any open file/directory descriptor
};

enum {
    ERR_NOT_FOUND     = -1, // File / directory not found
    ERR_TOO_MANY_OPEN = -2, // Too many open files / directories
    ERR_PARAM         = -3, // Invalid parameter
    ERR_EOF           = -4, // End of file / directory
    ERR_EXISTS        = -5, // File already exists
    ERR_OTHER         = -6, // Other error
    ERR_NO_DISK       = -7, // No disk
    ERR_NOT_EMPTY     = -8, // Not empty
};

enum {
    FO_RDONLY  = 0x00, // Open for reading only
    FO_WRONLY  = 0x01, // Open for writing only
    FO_RDWR    = 0x02, // Open for reading and writing
    FO_ACCMODE = 0x03, // Mask for above modes

    FO_APPEND = 0x04, // Append mode
    FO_CREATE = 0x08, // Create if non-existant
    FO_TRUNC  = 0x10, // Truncate to zero length
    FO_EXCL   = 0x20, // Error if already exists
};

#define MAX_FDS (10)
#define MAX_DDS (10)

struct state {
    char         *current_path;
    uint8_t       rxbuf[0x1000];
    unsigned      rxbuf_idx;
    direnum_ctx_t dds[MAX_DDS];
    FILE         *fds[MAX_FDS];
    const char   *new_path;
};

static struct state state;

static QueueHandle_t uart_queue;

#define MAX_COMPONENTS (20)

static void txfifo_write(uint8_t data) {
    uart_write_bytes(UART_NUM, &data, 1);
}

static char *resolve_path(const char *path) {
    // DBGF("resolve_path: '%s'\n", path);

    size_t tmppath_size = strlen(state.current_path) + 1 + strlen(path) + 1;
    char  *tmppath      = malloc(tmppath_size);
    assert(tmppath != NULL);
    tmppath[0] = 0;
    if (path[0] != '/' && path[0] != '\\') {
        strcat(tmppath, state.current_path);
        strcat(tmppath, "/");
    }
    strcat(tmppath, path);

    char *components[MAX_COMPONENTS];
    int   num_components = 0;
    char *result         = malloc(strlen(tmppath) + 1);
    assert(result != NULL);

    const char *ps = tmppath;
    char       *pd = result;

    while (*ps != 0) {
        // Skip leading slashes
        if (ps[0] == '/' || ps[0] == '\\') {
            ps++;
            continue;
        }

        // Handle './'
        if (ps[0] == '.') {
            if (ps[1] == '/' || ps[1] == '\\') {
                ps += 2;
                continue;
            }
            if (ps[1] == 0) {
                ps++;
                break;
            }
        }

        // Handle '../'
        if (ps[0] == '.' && ps[1] == '.') {
            bool undo_component = false;
            if (ps[2] == '/' || ps[2] == '\\') {
                ps += 3;
                undo_component = true;
            } else if (ps[2] == 0) {
                ps += 2;
                undo_component = true;
            }
            if (undo_component) {
                if (num_components > 0) {
                    pd = components[--num_components];
                } else {
                    pd = result;
                }
                continue;
            }
        }

        // Keep track of start of path components in result string to be able to undo
        if (num_components < MAX_COMPONENTS) {
            components[num_components++] = pd;
        }

        // Add leading slash
        *(pd++) = '/';

        // Copy path component from source to result string
        while (ps[0] != '/' && ps[0] != '\\' && ps[0] != 0) {
            *(pd++) = *(ps++);
        }
    }

    // Zero terminate result
    *pd = 0;
    // printf("result: '%s'  num_components: %d\n", result, num_components);

    free(tmppath);

    return result;
}

static void esp_open(uint8_t flags, const char *path_arg) {
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
            txfifo_write(ERR_PARAM);
            return;
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
    if (fd == -1) {
        // Error
        txfifo_write(ERR_TOO_MANY_OPEN);
        return;
    }

    // Compose full path
    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(MOUNT_POINT) + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, path);
    free(path);

    DBGF("OPEN: flags: 0x%02X (%s) '%s'\n", flags, mode, full_path);

    FILE *f      = fopen(full_path, mode);
    int   err_no = errno;
    free(full_path);

    if (f == NULL) {
        uint8_t err = ERR_NOT_FOUND;
        switch (err_no) {
            case EACCES: err = ERR_NOT_FOUND; break;
            case EEXIST: err = ERR_EXISTS; break;
            default: err = ERR_NOT_FOUND; break;
        }
        // Error
        txfifo_write(err);
        return;
    }
    state.fds[fd] = f;

    txfifo_write(fd);

    DBGF("OPEN fd: %d\n", fd);
}

static void esp_close(uint8_t fd) {
    DBGF("CLOSE fd: %d\n", fd);

    if (fd >= MAX_FDS || state.fds[fd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    FILE *f = state.fds[fd];

    fclose(f);
    state.fds[fd] = NULL;
    txfifo_write(0);
}

static void esp_read(uint8_t fd, uint16_t size) {
    DBGF("READ fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || state.fds[fd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    FILE *f = state.fds[fd];

    static uint8_t tmpbuf[0x10000];
    int            result = (int)fread(tmpbuf, 1, size, f);
    if (result < 0) {
        txfifo_write(ERR_OTHER);
        return;
    }

    txfifo_write(0);
    txfifo_write((result >> 0) & 0xFF);
    txfifo_write((result >> 8) & 0xFF);
    for (int i = 0; i < result; i++) {
        txfifo_write(tmpbuf[i]);
    }
}

static void esp_write(uint8_t fd, uint16_t size, const void *data) {
    DBGF("WRITE fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || state.fds[fd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    FILE *f = state.fds[fd];

    int result = (int)fwrite(data, 1, size, f);
    if (result < 0) {
        txfifo_write(ERR_OTHER);
        return;
    }

    txfifo_write(0);
    txfifo_write((result >> 0) & 0xFF);
    txfifo_write((result >> 8) & 0xFF);
}

static void esp_seek(uint8_t fd, uint32_t offset) {
    DBGF("SEEK fd: %d  offset: %u\n", fd, offset);

    if (fd >= MAX_FDS || state.fds[fd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    FILE *f = state.fds[fd];

    int result = fseek(f, offset, SEEK_SET);
    if (result < 0) {
        txfifo_write(ERR_OTHER);
        return;
    }
    txfifo_write(0);
}

static void esp_tell(uint8_t fd) {
    DBGF("TELL fd: %d\n", fd);

    if (fd >= MAX_FDS || state.fds[fd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    FILE *f = state.fds[fd];

    int result = ftell(f);
    if (result < 0) {
        txfifo_write(ERR_OTHER);
        return;
    }
    txfifo_write(0);
    txfifo_write((result >> 0) & 0xFF);
    txfifo_write((result >> 8) & 0xFF);
    txfifo_write((result >> 16) & 0xFF);
    txfifo_write((result >> 24) & 0xFF);
}

static void esp_opendir(const char *path_arg) {
    // Find free directory descriptor
    int dd = -1;
    for (int i = 0; i < MAX_DDS; i++) {
        if (state.dds[i] == NULL) {
            dd = i;
            break;
        }
    }
    if (dd == -1) {
        // Error
        txfifo_write(ERR_TOO_MANY_OPEN);
        return;
    }

    // Compose full path
    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(MOUNT_POINT) + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, path);
    free(path);

    DBGF("OPENDIR: '%s'\n", full_path);

    direnum_ctx_t ctx = direnum_open(full_path);
    free(full_path);

    if (ctx == NULL) {
        // Error
        txfifo_write(ERR_NOT_FOUND);
        return;
    }

    // Return directory descriptor
    state.dds[dd] = ctx;
    txfifo_write(dd);

    DBGF("OPENDIR dd: %d\n", dd);
}

static void esp_closedir(uint8_t dd) {
    DBGF("CLOSEDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || state.dds[dd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    direnum_ctx_t ctx = state.dds[dd];

    direnum_close(ctx);
    state.dds[dd] = NULL;

    txfifo_write(0);
}

static void esp_readdir(uint8_t dd) {
    // DBGF("READDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || state.dds[dd] == NULL) {
        txfifo_write(ERR_PARAM);
        return;
    }
    direnum_ctx_t ctx = state.dds[dd];

    struct direnum_ent de;
    if (!direnum_read(ctx, &de)) {
        txfifo_write(ERR_EOF);
        return;
    }

    struct tm *tm       = localtime(&de.t);
    uint16_t   fat_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
    uint16_t   fat_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

    txfifo_write(0);
    txfifo_write((fat_date >> 0) & 0xFF);
    txfifo_write((fat_date >> 8) & 0xFF);
    txfifo_write((fat_time >> 0) & 0xFF);
    txfifo_write((fat_time >> 8) & 0xFF);
    txfifo_write(de.attr);
    txfifo_write((de.size >> 0) & 0xFF);
    txfifo_write((de.size >> 8) & 0xFF);
    txfifo_write((de.size >> 16) & 0xFF);
    txfifo_write((de.size >> 24) & 0xFF);

    // DBGF(
    //     "%02u-%02u-%02u %02u:%02u ",
    //     tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
    //     tm->tm_hour, tm->tm_min);
    // if (de.attr & DE_DIR) {
    //     DBGF("<DIR>");
    // } else {
    //     DBGF("%5u", de.size);
    // }
    // DBGF(" %s\n", de.filename);

    const char *p = de.filename;
    while (*p) {
        txfifo_write(*(p++));
    }
    txfifo_write(0);
}

static void esp_delete(const char *path_arg) {
    // Compose full path
    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(MOUNT_POINT) + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, path);
    free(path);

    DBGF("DELETE %s\n", full_path);

    int result = unlink(full_path);
    if (result < 0) {
        result = rmdir(full_path);
    }

    free(full_path);

    if (result < 0) {
        // Error
        if (errno == ENOTEMPTY) {
            txfifo_write(ERR_NOT_EMPTY);
        } else {
            txfifo_write(ERR_NOT_FOUND);
        }
        return;
    }
    txfifo_write(0);
}

static void esp_rename(const char *old_arg, const char *new_arg) {
    // Compose full path
    char *old_path      = resolve_path(old_arg);
    char *old_full_path = malloc(strlen(MOUNT_POINT) + strlen(old_path) + 1);
    assert(old_full_path != NULL);
    strcpy(old_full_path, MOUNT_POINT);
    strcat(old_full_path, old_path);
    free(old_path);

    char *new_path      = resolve_path(new_arg);
    char *new_full_path = malloc(strlen(MOUNT_POINT) + strlen(new_path) + 1);
    assert(new_full_path != NULL);
    strcpy(new_full_path, MOUNT_POINT);
    strcat(new_full_path, new_path);
    free(new_path);

    DBGF("RENAME %s -> %s\n", old_full_path, new_full_path);

    int result = rename(old_full_path, new_full_path);
    free(old_full_path);
    free(new_full_path);

    if (result < 0) {
        // Error
        txfifo_write(ERR_NOT_FOUND);
        return;
    }
    txfifo_write(0);
}

static void esp_mkdir(const char *path_arg) {
    // Compose full path
    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(MOUNT_POINT) + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, path);
    free(path);

    DBGF("MKDIR %s\n", full_path);

#if _WIN32
    int result = mkdir(full_path);
#else
    int result = mkdir(full_path, 0775);
#endif
    free(full_path);
    if (result < 0) {
        // Error
        txfifo_write(ERR_OTHER);
        return;
    }
    txfifo_write(0);
}

static void esp_chdir(const char *path_arg) {
    // Compose full path
    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(MOUNT_POINT) + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, path);

    DBGF("CHDIR %s\n", full_path);

    struct stat st;
    int         result = stat(full_path, &st);
    free(full_path);

    if (result == 0 && (st.st_mode & S_IFDIR) != 0) {
        free(state.current_path);
        state.current_path = path;
        txfifo_write(0);

    } else {
        free(path);

        // Error
        txfifo_write(ERR_NOT_FOUND);
    }
}

static void esp_stat(const char *path_arg) {
    // Compose full path
    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(MOUNT_POINT) + strlen(path) + 1);
    assert(full_path != NULL);
    strcpy(full_path, MOUNT_POINT);
    strcat(full_path, path);
    free(path);

    DBGF("STAT %s\n", full_path);

    struct stat st;
    int         result = stat(full_path, &st);
    free(full_path);

    if (result < 0) {
        // Error
        txfifo_write(ERR_NOT_FOUND);
        return;
    }

    time_t t;
#ifdef __APPLE__
    t = st.st_mtimespec.tv_sec;
#elif _WIN32
    t          = st.st_mtime;
#else
    t = st.st_mtim.tv_sec;
#endif

    struct tm *tm       = localtime(&t);
    uint16_t   fat_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
    uint16_t   fat_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

    txfifo_write(0);
    txfifo_write((fat_date >> 0) & 0xFF);
    txfifo_write((fat_date >> 8) & 0xFF);
    txfifo_write((fat_time >> 0) & 0xFF);
    txfifo_write((fat_time >> 8) & 0xFF);
    txfifo_write((st.st_mode & S_IFDIR) != 0 ? DE_DIR : 0);
    txfifo_write((st.st_size >> 0) & 0xFF);
    txfifo_write((st.st_size >> 8) & 0xFF);
    txfifo_write((st.st_size >> 16) & 0xFF);
    txfifo_write((st.st_size >> 24) & 0xFF);
}

static void esp_getcwd(void) {
    DBGF("GETCWD\n");

    txfifo_write(0);
    int len = (int)strlen(state.current_path);

    if (len == 0) {
        txfifo_write('/');
    } else {
        for (int i = 0; i < len + 1; i++) {
            txfifo_write(state.current_path[i]);
        }
    }
    txfifo_write(0);
}

static void close_all_descriptors(void) {
    // Close any open descriptors
    for (int i = 0; i < MAX_DDS; i++) {
        if (state.dds[i] != NULL) {
            direnum_close(state.dds[i]);
            state.dds[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_FDS; i++) {
        if (state.fds[i] != NULL) {
            fclose(state.fds[i]);
            state.fds[i] = NULL;
        }
    }
}

static void esp_closeall(void) {
    DBGF("CLOSEALL\n");
    close_all_descriptors();
    txfifo_write(0);
}

static void esp32_write_data(uint8_t data) {
    if (state.current_path == NULL) {
        state.current_path = strdup("");
    }
    if (state.rxbuf_idx == 0 && data == 0) {
        // Ignore zero-bytes after break
        return;
    }

    state.rxbuf[state.rxbuf_idx] = data;
    if (state.rxbuf_idx < sizeof(state.rxbuf) - 1) {
        state.rxbuf_idx++;
    }

    if (state.rxbuf_idx > 0) {
        uint8_t cmd = state.rxbuf[0];

        switch (cmd) {
            case ESPCMD_RESET: {
                // Close any open descriptors
                ESP_LOGI(TAG, "RESET\n");
                //     close_all_descriptors();
                free(state.current_path);
                state.current_path = strdup("");
                break;
            }

            case ESPCMD_OPEN: {
                if (data == 0 && state.rxbuf_idx >= 3) {
                    esp_open(state.rxbuf[1], (const char *)&state.rxbuf[2]);
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_CLOSE: {
                if (state.rxbuf_idx == 2) {
                    esp_close(state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_READ: {
                if (state.rxbuf_idx == 4) {
                    esp_read(state.rxbuf[1], state.rxbuf[2] | (state.rxbuf[3] << 8));
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_WRITE: {
                if (state.rxbuf_idx >= 4) {
                    unsigned len = state.rxbuf[2] | (state.rxbuf[3] << 8);
                    if (state.rxbuf_idx == 4 + len) {
                        esp_write(state.rxbuf[1], len, (const char *)&state.rxbuf[4]);
                        state.rxbuf_idx = 0;
                    }
                }
                break;
            }
            case ESPCMD_SEEK: {
                if (state.rxbuf_idx == 6) {
                    esp_seek(
                        state.rxbuf[1],
                        ((state.rxbuf[2] << 0) |
                         (state.rxbuf[3] << 8) |
                         (state.rxbuf[4] << 16) |
                         (state.rxbuf[5] << 24)));
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_TELL: {
                if (state.rxbuf_idx == 2) {
                    esp_tell(state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            case ESPCMD_OPENDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_opendir((const char *)&state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            case ESPCMD_CLOSEDIR: {
                if (state.rxbuf_idx == 2) {
                    esp_closedir(state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            case ESPCMD_READDIR: {
                if (state.rxbuf_idx == 2) {
                    esp_readdir(state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            case ESPCMD_DELETE: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_delete((const char *)&state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_RENAME: {
                if (data == 0) {
                    if (state.new_path == NULL) {
                        state.new_path = (const char *)&state.rxbuf[state.rxbuf_idx];
                    } else {
                        esp_rename((const char *)&state.rxbuf[1], state.new_path);
                        state.new_path  = NULL;
                        state.rxbuf_idx = 0;
                    }
                }
                break;
            }
            case ESPCMD_MKDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_mkdir((const char *)&state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_CHDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_chdir((const char *)&state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_STAT: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_stat((const char *)&state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_GETCWD: {
                esp_getcwd();
                state.rxbuf_idx = 0;
                break;
            }
            case ESPCMD_CLOSEALL: {
                esp_closeall();
                state.rxbuf_idx = 0;
                break;
            }
            default: {
                printf("Invalid command: 0x%02X\n", cmd);
                break;
            }
        }
    }
}

static void uart_event_task(void *pvParameters) {
    uart_event_t event;
    uint8_t     *dtmp = (uint8_t *)malloc(BUF_SIZE);

    while (1) {
        // Waiting for UART event.
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event:", UART_NUM);
            switch (event.type) {
                // Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    // ESP_LOGI(TAG, "[UART DATA]: %d", event.size);

                    // Read data from the UART
                    uart_read_bytes(UART_NUM, dtmp, event.size, portMAX_DELAY);
                    // hexdump(dtmp, event.size);
                    for (unsigned i = 0; i < event.size; i++) {
                        esp32_write_data(dtmp[i]);
                    }

                    // ESP_LOGI(TAG, "[DATA EVT]:");
                    // uart_write_bytes(UART_NUM, (const char *)dtmp, event.size);
                    break;

                // Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM);
                    xQueueReset(uart_queue);
                    break;

                // Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider increasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM);
                    xQueueReset(uart_queue);
                    break;

                // Event of UART RX break detected
                case UART_BREAK: {
                    state.rxbuf_idx = 0;
                    state.new_path  = NULL;
                    break;
                }

                // Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;

                // Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;

                    // UART_PATTERN_DET
                    // case UART_PATTERN_DET:
                    //     uart_get_buffered_data_len(UART_NUM, &buffered_size);
                    //     int pos = uart_pattern_pop_pos(UART_NUM);
                    //     ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    //     if (pos == -1) {
                    //         // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    //         // record the position. We should set a larger queue size.
                    //         // As an example, we directly flush the rx buffer here.
                    //         uart_flush_input(UART_NUM);
                    //     } else {
                    //         uart_read_bytes(UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    //         uint8_t pat[PATTERN_CHR_NUM + 1];
                    //         memset(pat, 0, sizeof(pat));
                    //         uart_read_bytes(UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    //         ESP_LOGI(TAG, "read data: %s", dtmp);
                    //         ESP_LOGI(TAG, "read pat : %s", pat);
                    //     }
                    //     break;
                    // Others

                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
}

void uart_protocol_init(void) {
    // Initialize UART to FPGA
    uart_config_t uart_config = {
        .baud_rate           = 1789773,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, IOPIN_ESP_RX, IOPIN_ESP_TX, IOPIN_ESP_CTS, IOPIN_ESP_RTS));

    // Setup UART buffered IO with event queue
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0));
    uart_pattern_queue_reset(UART_NUM, 20);

    uint32_t baudrate;
    ESP_ERROR_CHECK(uart_get_baudrate(UART_NUM, &baudrate));
    ESP_LOGI(TAG, "Actual baudrate: %u", baudrate);

    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 12, NULL);
}
