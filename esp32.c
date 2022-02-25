#include "esp32.h"
#include "direnum.h"

enum {
    ESP_OPEN     = 0x10, //  Open / create file
    ESP_CLOSE    = 0x11, //  Close open file
    ESP_READ     = 0x12, //  Read from file
    ESP_WRITE    = 0x13, //  Write to file
    ESP_SEEK     = 0x14, //  Move read/write pointer
    ESP_TELL     = 0x15, //  Get current read/write
    ESP_OPENDIR  = 0x16, //  Open directory
    ESP_CLOSEDIR = 0x17, //  Close open directory
    ESP_READDIR  = 0x18, //  Read from directory
    ESP_UNLINK   = 0x19, //  Remove file or directory
    ESP_RENAME   = 0x1A, //  Rename / move file or directory
    ESP_MKDIR    = 0x1B, //  Create directory
    ESP_CHDIR    = 0x1C, //  Change directory
    ESP_STAT     = 0x1D, //  Get file status
};

enum {
    ERR_NOT_FOUND     = -1, // File / directory not found
    ERR_TOO_MANY_OPEN = -2, // Too many open files / directories
    ERR_INVALID_DESC  = -3, // Invalid descriptor
    ERR_EOF           = -4, // End of file / directory
};

#define MAX_FDS (10)
#define MAX_DDS (10)

struct state {
    char         *basepath;
    char         *current_path;
    uint8_t       rxbuf[256];
    unsigned      rxbuf_idx;
    uint8_t       txfifo[256];
    unsigned      txfifo_wridx;
    unsigned      txfifo_rdidx;
    unsigned      txfifo_cnt;
    direnum_ctx_t dds[MAX_DDS];
    int           fds[MAX_FDS];
};

static struct state state;

#define MAX_COMPONENTS (20)

static void txfifo_write(uint8_t data) {
    if (state.txfifo_cnt >= sizeof(state.txfifo)) {
        return;
    }

    state.txfifo[state.txfifo_wridx++] = data;
    state.txfifo_cnt++;
    if (state.txfifo_wridx >= sizeof(state.txfifo)) {
        state.txfifo_wridx = 0;
    }
}

static int txfifo_read(void) {
    int result = -1;
    if (state.txfifo_cnt > 0) {
        result = state.txfifo[state.txfifo_rdidx++];
        state.txfifo_cnt--;
        if (state.txfifo_rdidx >= sizeof(state.txfifo)) {
            state.txfifo_rdidx = 0;
        }
    }
    return result;
}

static char *resolve_path(const char *path) {
    // printf("resolve_path: '%s'\n", path);

    size_t tmppath_size = strlen(state.current_path) + 1 + strlen(path) + 1;
    char  *tmppath      = malloc(tmppath_size);
    tmppath[0]          = 0;
    if (path[0] != '/' && path[0] != '\\') {
        strcat(tmppath, state.current_path);
        strcat(tmppath, "/");
    }
    strcat(tmppath, path);

    char *components[MAX_COMPONENTS];
    int   num_components = 0;
    char *result         = malloc(strlen(tmppath) + 1);

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

    return result;
}

void esp32_init(const char *basepath) {
    // Copy basepath (without trailing slash if present)
    size_t basepath_len = strlen(basepath);
    while (basepath[basepath_len - 1] == '/' ||
           basepath[basepath_len - 1] == '\\') {
        basepath_len--;
    }
    state.basepath = malloc(basepath_len + 1);
    strncpy(state.basepath, basepath, basepath_len);
    state.current_path = strdup("");

    for (int i = 0; i < MAX_FDS; i++) {
        state.fds[i] = -1;
    }
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

    char *path      = resolve_path(path_arg);
    char *full_path = malloc(strlen(state.basepath) + strlen(path) + 1);
    strcpy(full_path, state.basepath);
    strcat(full_path, path);
    free(path);

    printf("OPENDIR: '%s'\n", full_path);

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
}

static void esp_readdir(uint8_t dd) {
    if (dd > MAX_DDS || state.dds[dd] == NULL) {
        txfifo_write(ERR_INVALID_DESC);
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
    txfifo_write((de.size >> 0) & 0xFF);
    txfifo_write((de.size >> 8) & 0xFF);
    txfifo_write((de.size >> 16) & 0xFF);
    txfifo_write((de.size >> 24) & 0xFF);
    txfifo_write((fat_date >> 0) & 0xFF);
    txfifo_write((fat_date >> 8) & 0xFF);
    txfifo_write((fat_time >> 0) & 0xFF);
    txfifo_write((fat_time >> 8) & 0xFF);
    txfifo_write(de.attr);

    const char *p = de.filename;
    while (*p) {
        txfifo_write(*(p++));
    }
    txfifo_write(0);
}

static void esp_closedir(uint8_t dd) {
    if (dd > MAX_DDS || state.dds[dd] == NULL) {
        txfifo_write(ERR_INVALID_DESC);
        return;
    }
    direnum_ctx_t ctx = state.dds[dd];

    direnum_close(ctx);
    state.dds[dd] = NULL;

    txfifo_write(0);
}

void esp32_write_data(uint8_t data) {
    printf("esp32_write_data: %02X\n", data);

    state.rxbuf[state.rxbuf_idx] = data;
    if (state.rxbuf_idx < sizeof(state.rxbuf) - 1) {
        state.rxbuf_idx++;
    }

    if (state.rxbuf_idx > 0) {
        uint8_t cmd = state.rxbuf[0];

        switch (cmd) {
            case ESP_OPENDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_opendir((const char *)&state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            case ESP_READDIR: {
                if (state.rxbuf_idx == 2) {
                    esp_readdir(state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            case ESP_CLOSEDIR: {
                if (state.rxbuf_idx == 2) {
                    esp_closedir(state.rxbuf[1]);
                    state.rxbuf_idx = 0;
                }
                break;
            }

            default: {
                printf("Invalid command: 0x%02X\n", cmd);
                break;
            }
        }
    }
}

uint8_t esp32_read_data(void) {
    int data = txfifo_read();
    if (data < 0) {
        printf("esp32_read_data - Empty!\n");
        return 0;
    }
    return data;
}

void esp32_write_ctrl(uint8_t data) {
    printf("esp32_write_ctrl: %02X\n", data);
}

uint8_t esp32_read_ctrl(void) {
    printf("esp32_read_ctrl\n");
    return 0;
}
