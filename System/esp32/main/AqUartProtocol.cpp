#include "AqUartProtocol.h"
#include <driver/uart.h>
#include "SDCardVFS.h"
#include "EspVFS.h"
#include <algorithm>
#include <esp_ota_ops.h>
#include <esp_app_format.h>

static const char *TAG = "AqUartProtocol";

enum {
    ESPCMD_RESET    = 0x01, // Indicate to ESP that system has been reset
    ESPCMD_VERSION  = 0x02, // Get version string
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

#define BUF_SIZE (1024)
#define UART_NUM (UART_NUM_1)
#define ESP_PREFIX "esp:"

#if 0
#    define DBGF(...) ESP_LOGI(TAG, __VA_ARGS__)
#else
#    define DBGF(...)
#endif

AqUartProtocol::AqUartProtocol() {
    rxBufIdx = 0;
    for (int i = 0; i < MAX_FDS; i++) {
        fdVfs[i] = nullptr;
        fds[i]   = 0;
    }
    for (int i = 0; i < MAX_DDS; i++) {
        deCtxs[i] = nullptr;
        deIdx[i]  = 0;
    }
}

AqUartProtocol &AqUartProtocol::instance() {
    static AqUartProtocol *obj = nullptr;
    if (obj == nullptr)
        obj = new AqUartProtocol();
    return *obj;
}

void AqUartProtocol::init() {
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
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uartQueue, 0));
    uart_pattern_queue_reset(UART_NUM, 20);

    uint32_t baudrate;
    ESP_ERROR_CHECK(uart_get_baudrate(UART_NUM, &baudrate));
    ESP_LOGI(TAG, "Actual baudrate: %lu", baudrate);

    EspVFS::instance().init();
    xTaskCreate(_uartEventTask, "uart_event_task", 6144, this, 1, nullptr);
}

void AqUartProtocol::_uartEventTask(void *param) {
    static_cast<AqUartProtocol *>(param)->uartEventTask();
}

void AqUartProtocol::uartEventTask() {
    uart_event_t                  event;
    std::array<uint8_t, BUF_SIZE> buf;

    while (1) {
        // Waiting for UART event.
        if (xQueueReceive(uartQueue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            buf.fill(0);
            switch (event.type) {
                // UART data available
                case UART_DATA:
                    uart_read_bytes(UART_NUM, buf.data(), event.size, portMAX_DELAY);
                    for (unsigned i = 0; i < event.size; i++) {
                        receivedByte(buf[i]);
                    }
                    break;

                // HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "HW FIFO overflow");
                    uart_flush_input(UART_NUM);
                    xQueueReset(uartQueue);
                    break;

                // UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "ring buffer full");
                    uart_flush_input(UART_NUM);
                    xQueueReset(uartQueue);
                    break;

                // UART RX break detected
                case UART_BREAK:
                    rxBufIdx = 0;
                    break;

                case UART_PARITY_ERR: ESP_LOGW(TAG, "UART parity error"); break;
                case UART_FRAME_ERR: ESP_LOGW(TAG, "UART frame error"); break;
                default: ESP_LOGW(TAG, "UART event type: %d", event.type); break;
            }
        }
    }
}

void AqUartProtocol::txFifoWrite(uint8_t data) {
    uart_write_bytes(UART_NUM, &data, 1);
}

void AqUartProtocol::txFifoWrite(const void *buf, size_t length) {
    uart_write_bytes(UART_NUM, buf, length);
}

void AqUartProtocol::splitPath(const std::string &path, std::vector<std::string> &result) {
    const char *delimiters = "/\\";
    size_t      start;
    size_t      end = 0;
    while ((start = path.find_first_not_of(delimiters, end)) != std::string::npos) {
        end = path.find_first_of(delimiters, start);
        result.push_back(path.substr(start, end - start));
    }
}

std::string AqUartProtocol::resolvePath(std::string path, VFS **vfs) {
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
        if (currentPath.starts_with(ESP_PREFIX)) {
            splitPath(currentPath.substr(strlen(ESP_PREFIX)), parts);
            *vfs = &EspVFS::instance();
        } else {
            splitPath(currentPath, parts);
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
    return result;
}

void AqUartProtocol::closeAllDescriptors() {
    // Close any open descriptors
    for (int i = 0; i < MAX_FDS; i++) {
        if (fdVfs[i] != nullptr) {
            fdVfs[i]->close(fds[i]);
            fdVfs[i] = nullptr;
        }
    }
    for (int i = 0; i < MAX_DDS; i++) {
        deCtxs[i] = nullptr;
    }
}

void AqUartProtocol::receivedByte(uint8_t data) {
    if (rxBufIdx == 0 && data == 0) {
        // Ignore zero-bytes after break
        return;
    }

    rxBuf[rxBufIdx] = data;
    if (rxBufIdx < sizeof(rxBuf) - 1) {
        rxBufIdx++;
    }

    if (rxBufIdx > 0) {
        uint8_t cmd = rxBuf[0];

        switch (cmd) {
            case ESPCMD_RESET: {
                // Close any open descriptors
                ESP_LOGI(TAG, "RESET");
                cmdReset();
                break;
            }
            case ESPCMD_VERSION: {
                cmdVersion();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_OPEN: {
                if (data == 0 && rxBufIdx >= 3) {
                    uint8_t     flags   = rxBuf[1];
                    const char *pathArg = (const char *)&rxBuf[2];
                    cmdOpen(flags, pathArg);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_CLOSE: {
                if (rxBufIdx == 2) {
                    uint8_t fd = rxBuf[1];
                    cmdClose(fd);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_READ: {
                if (rxBufIdx == 4) {
                    uint8_t  fd   = rxBuf[1];
                    uint16_t size = rxBuf[2] | (rxBuf[3] << 8);
                    cmdRead(fd, size);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_WRITE: {
                if (rxBufIdx >= 4) {
                    uint8_t     fd   = rxBuf[1];
                    unsigned    size = rxBuf[2] | (rxBuf[3] << 8);
                    const void *data = &rxBuf[4];
                    if (rxBufIdx == 4 + size) {
                        cmdWrite(fd, size, data);
                        rxBufIdx = 0;
                    }
                }
                break;
            }
            case ESPCMD_SEEK: {
                if (rxBufIdx == 6) {
                    uint8_t  fd     = rxBuf[1];
                    uint32_t offset = (rxBuf[2] << 0) | (rxBuf[3] << 8) | (rxBuf[4] << 16) | (rxBuf[5] << 24);
                    cmdSeek(fd, offset);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_TELL: {
                if (rxBufIdx == 2) {
                    uint8_t fd = rxBuf[1];
                    cmdTell(fd);
                    rxBufIdx = 0;
                }
                break;
            }

            case ESPCMD_OPENDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *pathArg = (const char *)&rxBuf[1];
                    cmdOpenDir(pathArg);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_CLOSEDIR: {
                if (rxBufIdx == 2) {
                    uint8_t dd = rxBuf[1];
                    cmdCloseDir(dd);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_READDIR: {
                if (rxBufIdx == 2) {
                    uint8_t dd = rxBuf[1];
                    cmdReadDir(dd);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_DELETE: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *pathArg = (const char *)&rxBuf[1];
                    cmdDelete(pathArg);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_RENAME: {
                static const char *newPath;
                if (rxBufIdx == 1) {
                    newPath = nullptr;
                }

                if (data == 0) {
                    const char *oldPath = (const char *)&rxBuf[1];
                    if (newPath == nullptr) {
                        newPath = (const char *)&rxBuf[rxBufIdx];
                    } else {
                        cmdRename(oldPath, newPath);
                        newPath  = nullptr;
                        rxBufIdx = 0;
                    }
                }
                break;
            }
            case ESPCMD_MKDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdMkDir((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_CHDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdChDir((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_STAT: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdStat((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_GETCWD: {
                cmdGetCwd();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_CLOSEALL: {
                cmdCloseAll();
                rxBufIdx = 0;
                break;
            }
            default: {
                ESP_LOGE(TAG, "Invalid command: 0x%02X\n", cmd);
                break;
            }
        }
    }
}

void AqUartProtocol::cmdReset() {
    closeAllDescriptors();
    currentPath.clear();
}

void AqUartProtocol::cmdVersion() {
    DBGF("VERSION");

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t         running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        const char *p = running_app_info.version;
        while (*p) {
            txFifoWrite(*(p++));
        }
    }
    txFifoWrite(0);
}

void AqUartProtocol::cmdOpen(uint8_t flags, const char *pathArg) {
    DBGF("OPEN: flags: 0x%02X '%s'\n", flags, pathArg);

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (fdVfs[i] == nullptr) {
            fd = i;
            break;
        }
    }
    if (fd == -1) {
        // Error
        txFifoWrite(ERR_TOO_MANY_OPEN);
        return;
    }

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int vfs_fd = vfs->open(flags, path);
    if (vfs_fd < 0) {
        txFifoWrite(vfs_fd);
    } else {
        fdVfs[fd] = vfs;
        fds[fd]   = vfs_fd;
        txFifoWrite(fd);
    }
}

void AqUartProtocol::cmdClose(uint8_t fd) {
    DBGF("CLOSE fd: %d\n", fd);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    txFifoWrite(fdVfs[fd]->close(fds[fd]));
    fdVfs[fd] = nullptr;
}

void AqUartProtocol::cmdRead(uint8_t fd, uint16_t size) {
    DBGF("READ fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->read(fds[fd], size, rxBuf);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);
        txFifoWrite((result >> 0) & 0xFF);
        txFifoWrite((result >> 8) & 0xFF);
        txFifoWrite(rxBuf, result);
    }
}

void AqUartProtocol::cmdWrite(uint8_t fd, uint16_t size, const void *data) {
    DBGF("WRITE fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->write(fds[fd], size, data);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);
        txFifoWrite((result >> 0) & 0xFF);
        txFifoWrite((result >> 8) & 0xFF);
    }
}

void AqUartProtocol::cmdSeek(uint8_t fd, uint32_t offset) {
    DBGF("SEEK fd: %d  offset: %lu\n", fd, offset);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    txFifoWrite(fdVfs[fd]->seek(fds[fd], offset));
}

void AqUartProtocol::cmdTell(uint8_t fd) {
    DBGF("TELL fd: %d\n", fd);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->tell(fds[fd]);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);
        txFifoWrite((result >> 0) & 0xFF);
        txFifoWrite((result >> 8) & 0xFF);
        txFifoWrite((result >> 16) & 0xFF);
        txFifoWrite((result >> 24) & 0xFF);
    }
}

void AqUartProtocol::cmdOpenDir(const char *pathArg) {
    DBGF("OPENDIR: '%s'\n", pathArg);

    // Find free directory descriptor
    int dd = -1;
    for (int i = 0; i < MAX_DDS; i++) {
        if (deCtxs[i] == nullptr) {
            dd = i;
            break;
        }
    }
    if (dd == -1) {
        // Error
        txFifoWrite(ERR_TOO_MANY_OPEN);
        return;
    }

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    auto deCtx = vfs->direnum(path);
    if (!deCtx) {
        txFifoWrite(ERR_NOT_FOUND);
    } else {
        std::sort(deCtx->begin(), deCtx->end(), [](auto &a, auto &b) {
            // Sort directories at the top
            if ((a.attr & DE_DIR) != (b.attr & DE_DIR)) {
                return (a.attr & DE_DIR) != 0;
            }
            return strcasecmp(a.filename.c_str(), b.filename.c_str()) < 0;
        });

        deCtxs[dd] = deCtx;
        deIdx[dd]  = 0;
        txFifoWrite(dd);
    }
}

void AqUartProtocol::cmdCloseDir(uint8_t dd) {
    DBGF("CLOSEDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || deCtxs[dd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }
    deCtxs[dd] = nullptr;
    txFifoWrite(0);
}

void AqUartProtocol::cmdReadDir(uint8_t dd) {
    DBGF("READDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || deCtxs[dd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    auto ctx = deCtxs[dd];
    if (deIdx[dd] >= (*ctx).size()) {
        txFifoWrite(ERR_EOF);
        return;
    }

    auto &de = (*ctx)[deIdx[dd]++];
    txFifoWrite(0);
    txFifoWrite((de.fdate >> 0) & 0xFF);
    txFifoWrite((de.fdate >> 8) & 0xFF);
    txFifoWrite((de.ftime >> 0) & 0xFF);
    txFifoWrite((de.ftime >> 8) & 0xFF);
    txFifoWrite(de.attr);
    txFifoWrite((de.size >> 0) & 0xFF);
    txFifoWrite((de.size >> 8) & 0xFF);
    txFifoWrite((de.size >> 16) & 0xFF);
    txFifoWrite((de.size >> 24) & 0xFF);
    txFifoWrite(de.filename.c_str(), de.filename.size());
    txFifoWrite(0);
}

void AqUartProtocol::cmdDelete(const char *pathArg) {
    DBGF("DELETE %s\n", pathArg);

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }
    txFifoWrite(vfs->delete_(path));
}

void AqUartProtocol::cmdRename(const char *old_arg, const char *new_arg) {
    DBGF("RENAME %s -> %s\n", old_arg, new_arg);

    VFS *vfs1    = nullptr;
    VFS *vfs2    = nullptr;
    auto oldPath = resolvePath(old_arg, &vfs1);
    auto newPath = resolvePath(new_arg, &vfs2);
    if (!vfs1 || vfs1 != vfs2) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    txFifoWrite(vfs1->rename(oldPath, newPath));
}

void AqUartProtocol::cmdMkDir(const char *pathArg) {
    DBGF("MKDIR %s\n", pathArg);

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }
    txFifoWrite(vfs->mkdir(pathArg));
}

void AqUartProtocol::cmdChDir(const char *pathArg) {
    DBGF("CHDIR %s\n", pathArg);

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    struct stat st;
    int         result = vfs->stat(path, &st);
    txFifoWrite(result);

    if (result == 0 && (st.st_mode & S_IFDIR) != 0) {
        if (vfs == &EspVFS::instance()) {
            currentPath = std::string(ESP_PREFIX) + path;
        } else {
            currentPath = path;
        }
    }
}

void AqUartProtocol::cmdStat(const char *pathArg) {
    DBGF("STAT %s\n", pathArg);

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    struct stat st;
    int         result = vfs->stat(pathArg, &st);

    txFifoWrite(result);
    if (result < 0)
        return;

    time_t t = st.st_mtim.tv_sec;

    struct tm *tm       = localtime(&t);
    uint16_t   fat_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
    uint16_t   fat_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

    txFifoWrite((fat_date >> 0) & 0xFF);
    txFifoWrite((fat_date >> 8) & 0xFF);
    txFifoWrite((fat_time >> 0) & 0xFF);
    txFifoWrite((fat_time >> 8) & 0xFF);
    txFifoWrite((st.st_mode & S_IFDIR) != 0 ? DE_DIR : 0);
    txFifoWrite((st.st_size >> 0) & 0xFF);
    txFifoWrite((st.st_size >> 8) & 0xFF);
    txFifoWrite((st.st_size >> 16) & 0xFF);
    txFifoWrite((st.st_size >> 24) & 0xFF);
}

void AqUartProtocol::cmdGetCwd() {
    DBGF("GETCWD\n");

    txFifoWrite(0);
    int len = currentPath.size();

    if (len == 0) {
        txFifoWrite('/');
    } else {
        for (int i = 0; i < len; i++) {
            txFifoWrite(currentPath[i]);
        }
    }
    txFifoWrite(0);
}

void AqUartProtocol::cmdCloseAll() {
    DBGF("CLOSEALL\n");
    closeAllDescriptors();
    txFifoWrite(0);
}
