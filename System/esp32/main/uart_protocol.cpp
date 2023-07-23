#include "uart_protocol.h"
#include <driver/uart.h>
#include "SDCardVFS.h"
#include "VFS.h"
#include "EspVFS.h"
#include <algorithm>

static const char *TAG = "uart_protocol";
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define UART_NUM (UART_NUM_1)

#if 0
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

#define MAX_FDS (10)
#define MAX_DDS (10)
#define MAX_COMPONENTS (20)

#define ESP_PREFIX "esp:"

struct state {
    uint8_t  rxbuf[16 + 0x10000];
    unsigned rxbuf_idx;

    VFS    *fd_vfs[MAX_FDS];
    uint8_t fd[MAX_FDS];

    DirEnumCtx dd[MAX_DDS];
    int        ddIdx[MAX_DDS];
};

static std::string current_path;

static struct state *state;

static QueueHandle_t uart_queue;

static void txfifo_write(uint8_t data) {
    uart_write_bytes(UART_NUM, &data, 1);
}

static void splitPath(const std::string &path, std::vector<std::string> &result) {
    const char *delimiters = "/\\";
    size_t      start;
    size_t      end = 0;
    while ((start = path.find_first_not_of(delimiters, end)) != std::string::npos) {
        end = path.find_first_of(delimiters, start);
        result.push_back(path.substr(start, end - start));
    }
}

static std::string resolve_path(std::string path, VFS **vfs) {
    *vfs = &SDCardVFS::instance();

    bool useCwd = true;
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
        useCwd = false;
    } else if (strncasecmp(path.c_str(), ESP_PREFIX, strlen(ESP_PREFIX)) == 0) {
        useCwd = false;
        *vfs   = &EspVFS::instance();
        path   = path.substr(strlen(ESP_PREFIX));
    }

    // Split the path into parts
    std::vector<std::string> parts;
    if (useCwd) {
        if (current_path.starts_with(ESP_PREFIX)) {
            splitPath(current_path.substr(strlen(ESP_PREFIX)), parts);
            *vfs = &EspVFS::instance();
        } else {
            splitPath(current_path, parts);
        }
    }
    splitPath(path, parts);

    // Resolve path
    int idx = 0;
    while (idx < parts.size()) {
        if (parts[idx] == ".") {
            parts.erase(parts.begin() + idx);
            continue;
        }
        if (parts[idx] == "..") {
            auto iterLast = parts.begin() + idx + 1;
            if (idx > 0)
                idx--;
            auto iterFirst = parts.begin() + idx;
            parts.erase(iterFirst, iterLast);
            continue;
        }
        idx++;
    }

    // Compose resolved path
    std::string result;
    for (auto &part : parts) {
        if (!result.empty())
            result += '/';
        result += part;
    }

    printf("Resolved path: %s\n", result.c_str());

    return result;
}

static void close_all_descriptors(void) {
    // Close any open descriptors
    for (int i = 0; i < MAX_FDS; i++) {
        if (state->fd_vfs[i] != nullptr) {
            state->fd_vfs[i]->close(state->fd[i]);
            state->fd_vfs[i] = nullptr;
        }
    }
    for (int i = 0; i < MAX_DDS; i++) {
        state->dd[i] = nullptr;
    }
}

static void esp_reset(void) {
    close_all_descriptors();
    current_path.clear();
}

static void esp_open(uint8_t flags, const char *path_arg) {
    DBGF("OPEN: flags: 0x%02X '%s'\n", flags, path_arg);

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (state->fd_vfs[i] == nullptr) {
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
    VFS *vfs  = nullptr;
    auto path = resolve_path(path_arg, &vfs);
    if (!vfs) {
        txfifo_write(ERR_PARAM);
        return;
    }

    int vfs_fd = vfs->open(flags, path);
    if (vfs_fd < 0) {
        txfifo_write(vfs_fd);
    } else {
        state->fd_vfs[fd] = vfs;
        state->fd[fd]     = vfs_fd;
        txfifo_write(fd);
    }
}

static void esp_close(uint8_t fd) {
    DBGF("CLOSE fd: %d\n", fd);

    if (fd >= MAX_FDS || state->fd_vfs[fd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }

    txfifo_write(state->fd_vfs[fd]->close(state->fd[fd]));
    state->fd_vfs[fd] = nullptr;
}

static void esp_read(uint8_t fd, uint16_t size) {
    DBGF("READ fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || state->fd_vfs[fd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }

    int result = state->fd_vfs[fd]->read(state->fd[fd], size, state->rxbuf);
    if (result < 0) {
        txfifo_write(result);
    } else {
        txfifo_write(0);
        txfifo_write((result >> 0) & 0xFF);
        txfifo_write((result >> 8) & 0xFF);
        for (int i = 0; i < result; i++) {
            txfifo_write(state->rxbuf[i]);
        }
    }
}

static void esp_write(uint8_t fd, uint16_t size, const void *data) {
    DBGF("WRITE fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || state->fd_vfs[fd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }

    int result = state->fd_vfs[fd]->write(state->fd[fd], size, data);
    if (result < 0) {
        txfifo_write(result);
    } else {
        txfifo_write(0);
        txfifo_write((result >> 0) & 0xFF);
        txfifo_write((result >> 8) & 0xFF);
    }
}

static void esp_seek(uint8_t fd, uint32_t offset) {
    DBGF("SEEK fd: %d  offset: %lu\n", fd, offset);

    if (fd >= MAX_FDS || state->fd_vfs[fd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }

    txfifo_write(state->fd_vfs[fd]->seek(state->fd[fd], offset));
}

static void esp_tell(uint8_t fd) {
    DBGF("TELL fd: %d\n", fd);

    if (fd >= MAX_FDS || state->fd_vfs[fd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }

    int result = state->fd_vfs[fd]->tell(state->fd[fd]);
    if (result < 0) {
        txfifo_write(result);
    } else {
        txfifo_write(0);
        txfifo_write((result >> 0) & 0xFF);
        txfifo_write((result >> 8) & 0xFF);
        txfifo_write((result >> 16) & 0xFF);
        txfifo_write((result >> 24) & 0xFF);
    }
}

static void esp_opendir(const char *path_arg) {
    DBGF("OPENDIR: '%s'\n", path_arg);

    // Find free directory descriptor
    int dd = -1;
    for (int i = 0; i < MAX_DDS; i++) {
        if (state->dd[i] == nullptr) {
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
    VFS *vfs  = nullptr;
    auto path = resolve_path(path_arg, &vfs);
    if (!vfs) {
        txfifo_write(ERR_PARAM);
        return;
    }

    auto deCtx = vfs->direnum(path);
    if (!deCtx) {
        txfifo_write(ERR_NOT_FOUND);
    } else {
        std::sort(deCtx->begin(), deCtx->end(), [](auto &a, auto &b) {
            // Sort directories at the top
            if ((a.attr & DE_DIR) != (b.attr & DE_DIR)) {
                return (a.attr & DE_DIR) != 0;
            }
            return strcasecmp(a.filename.c_str(), b.filename.c_str()) < 0;
        });

        state->dd[dd]    = deCtx;
        state->ddIdx[dd] = 0;
        txfifo_write(dd);
    }
}

static void esp_closedir(uint8_t dd) {
    DBGF("CLOSEDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || state->dd[dd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }
    state->dd[dd] = nullptr;
    txfifo_write(0);
}

static void esp_readdir(uint8_t dd) {
    DBGF("READDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || state->dd[dd] == nullptr) {
        txfifo_write(ERR_PARAM);
        return;
    }

    auto ctx = state->dd[dd];
    if (state->ddIdx[dd] >= (*ctx).size()) {
        txfifo_write(ERR_EOF);
        return;
    }

    auto &de = (*ctx)[state->ddIdx[dd]++];
    txfifo_write(0);
    txfifo_write((de.fdate >> 0) & 0xFF);
    txfifo_write((de.fdate >> 8) & 0xFF);
    txfifo_write((de.ftime >> 0) & 0xFF);
    txfifo_write((de.ftime >> 8) & 0xFF);
    txfifo_write(de.attr);
    txfifo_write((de.size >> 0) & 0xFF);
    txfifo_write((de.size >> 8) & 0xFF);
    txfifo_write((de.size >> 16) & 0xFF);
    txfifo_write((de.size >> 24) & 0xFF);

    const char *p = de.filename.c_str();
    while (*p) {
        txfifo_write(*(p++));
    }
    txfifo_write(0);
}

static void esp_delete(const char *path_arg) {
    DBGF("DELETE %s\n", path_arg);

    VFS *vfs  = nullptr;
    auto path = resolve_path(path_arg, &vfs);
    if (!vfs) {
        txfifo_write(ERR_PARAM);
        return;
    }
    txfifo_write(vfs->delete_(path));
}

static void esp_rename(const char *old_arg, const char *new_arg) {
    DBGF("RENAME %s -> %s\n", old_arg, new_arg);

    VFS *vfs1     = nullptr;
    VFS *vfs2     = nullptr;
    auto old_path = resolve_path(old_arg, &vfs1);
    auto new_path = resolve_path(new_arg, &vfs2);
    if (!vfs1 || vfs1 != vfs2) {
        txfifo_write(ERR_PARAM);
        return;
    }

    txfifo_write(vfs1->rename(old_path, new_path));
}

static void esp_mkdir(const char *path_arg) {
    DBGF("MKDIR %s\n", path_arg);

    VFS *vfs  = nullptr;
    auto path = resolve_path(path_arg, &vfs);
    if (!vfs) {
        txfifo_write(ERR_PARAM);
        return;
    }
    txfifo_write(vfs->mkdir(path_arg));
}

static void esp_chdir(const char *path_arg) {
    DBGF("CHDIR %s\n", path_arg);

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolve_path(path_arg, &vfs);
    if (!vfs) {
        txfifo_write(ERR_PARAM);
        return;
    }

    struct stat st;
    int         result = vfs->stat(path, &st);
    txfifo_write(result);

    if (result == 0 && (st.st_mode & S_IFDIR) != 0) {
        if (vfs == &EspVFS::instance()) {
            current_path = std::string(ESP_PREFIX) + path;
        } else {
            current_path = path;
        }
    }
}

static void esp_stat(const char *path_arg) {
    DBGF("STAT %s\n", path_arg);

    VFS *vfs  = nullptr;
    auto path = resolve_path(path_arg, &vfs);
    if (!vfs) {
        txfifo_write(ERR_PARAM);
        return;
    }

    struct stat st;
    int         result = vfs->stat(path_arg, &st);

    txfifo_write(result);
    if (result < 0)
        return;

    time_t t = st.st_mtim.tv_sec;

    struct tm *tm       = localtime(&t);
    uint16_t   fat_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
    uint16_t   fat_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

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
    int len = current_path.size();

    if (len == 0) {
        txfifo_write('/');
    } else {
        for (int i = 0; i < len; i++) {
            txfifo_write(current_path[i]);
        }
    }
    txfifo_write(0);
}

static void esp_closeall(void) {
    DBGF("CLOSEALL\n");
    close_all_descriptors();
    txfifo_write(0);
}

static void esp32_write_data(uint8_t data) {
    if (state->rxbuf_idx == 0 && data == 0) {
        // Ignore zero-bytes after break
        return;
    }

    state->rxbuf[state->rxbuf_idx] = data;
    if (state->rxbuf_idx < sizeof(state->rxbuf) - 1) {
        state->rxbuf_idx++;
    }

    if (state->rxbuf_idx > 0) {
        uint8_t cmd = state->rxbuf[0];

        switch (cmd) {
            case ESPCMD_RESET: {
                // Close any open descriptors
                ESP_LOGI(TAG, "RESET");
                esp_reset();
                break;
            }

            case ESPCMD_OPEN: {
                if (data == 0 && state->rxbuf_idx >= 3) {
                    uint8_t     flags    = state->rxbuf[1];
                    const char *path_arg = (const char *)&state->rxbuf[2];
                    esp_open(flags, path_arg);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_CLOSE: {
                if (state->rxbuf_idx == 2) {
                    uint8_t fd = state->rxbuf[1];
                    esp_close(fd);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_READ: {
                if (state->rxbuf_idx == 4) {
                    uint8_t  fd   = state->rxbuf[1];
                    uint16_t size = state->rxbuf[2] | (state->rxbuf[3] << 8);
                    esp_read(fd, size);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_WRITE: {
                if (state->rxbuf_idx >= 4) {
                    uint8_t     fd   = state->rxbuf[1];
                    unsigned    size = state->rxbuf[2] | (state->rxbuf[3] << 8);
                    const void *data = &state->rxbuf[4];
                    if (state->rxbuf_idx == 4 + size) {
                        esp_write(fd, size, data);
                        state->rxbuf_idx = 0;
                    }
                }
                break;
            }
            case ESPCMD_SEEK: {
                if (state->rxbuf_idx == 6) {
                    uint8_t  fd     = state->rxbuf[1];
                    uint32_t offset = (state->rxbuf[2] << 0) | (state->rxbuf[3] << 8) | (state->rxbuf[4] << 16) | (state->rxbuf[5] << 24);
                    esp_seek(fd, offset);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_TELL: {
                if (state->rxbuf_idx == 2) {
                    uint8_t fd = state->rxbuf[1];
                    esp_tell(fd);
                    state->rxbuf_idx = 0;
                }
                break;
            }

            case ESPCMD_OPENDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *path_arg = (const char *)&state->rxbuf[1];
                    esp_opendir(path_arg);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_CLOSEDIR: {
                if (state->rxbuf_idx == 2) {
                    uint8_t dd = state->rxbuf[1];
                    esp_closedir(dd);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_READDIR: {
                if (state->rxbuf_idx == 2) {
                    uint8_t dd = state->rxbuf[1];
                    esp_readdir(dd);
                    state->rxbuf_idx = 0;
                }
                break;
            }

            case ESPCMD_DELETE: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *path_arg = (const char *)&state->rxbuf[1];
                    esp_delete(path_arg);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_RENAME: {
                static const char *new_path;
                if (state->rxbuf_idx == 1) {
                    new_path = nullptr;
                }

                if (data == 0) {
                    const char *old_path = (const char *)&state->rxbuf[1];
                    if (new_path == nullptr) {
                        new_path = (const char *)&state->rxbuf[state->rxbuf_idx];
                    } else {
                        esp_rename(old_path, new_path);
                        new_path         = nullptr;
                        state->rxbuf_idx = 0;
                    }
                }
                break;
            }
            case ESPCMD_MKDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_mkdir((const char *)&state->rxbuf[1]);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_CHDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_chdir((const char *)&state->rxbuf[1]);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_STAT: {
                // Wait for zero-termination of path
                if (data == 0) {
                    esp_stat((const char *)&state->rxbuf[1]);
                    state->rxbuf_idx = 0;
                }
                break;
            }
            case ESPCMD_GETCWD: {
                esp_getcwd();
                state->rxbuf_idx = 0;
                break;
            }
            case ESPCMD_CLOSEALL: {
                esp_closeall();
                state->rxbuf_idx = 0;
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
            switch (event.type) {
                // Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    // Read data from the UART
                    uart_read_bytes(UART_NUM, dtmp, event.size, portMAX_DELAY);
                    for (unsigned i = 0; i < event.size; i++) {
                        esp32_write_data(dtmp[i]);
                    }
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
                case UART_BREAK:
                    state->rxbuf_idx = 0;
                    break;

                case UART_PARITY_ERR: ESP_LOGI(TAG, "uart parity error"); break;
                case UART_FRAME_ERR: ESP_LOGI(TAG, "uart frame error"); break;
                default: ESP_LOGI(TAG, "uart event type: %d", event.type); break;
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
        .source_clk          = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, IOPIN_ESP_RX, IOPIN_ESP_TX, IOPIN_ESP_CTS, IOPIN_ESP_RTS));

    // Setup UART buffered IO with event queue
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0));
    uart_pattern_queue_reset(UART_NUM, 20);

    uint32_t baudrate;
    ESP_ERROR_CHECK(uart_get_baudrate(UART_NUM, &baudrate));
    ESP_LOGI(TAG, "Actual baudrate: %lu", baudrate);

    state = (struct state *)calloc(sizeof(*state), 1);
    assert(state != nullptr);

    EspVFS::instance().init();
    xTaskCreate(uart_event_task, "uart_event_task", 6144, nullptr, 1, nullptr);
}
