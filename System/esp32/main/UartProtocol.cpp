#include "UartProtocol.h"

#include <driver/uart.h>
#include <esp_ota_ops.h>
#include <esp_app_format.h>

#include "VFS.h"
#include "Keyboard.h"
#include "FpgaCore.h"

static const char *TAG = "UartProtocol";

#define UART_NUM (UART_NUM_1)
#define BUF_SIZE (1024)
#define MAX_FDS  (10)
#define MAX_DDS  (10)

#define ESP_PREFIX "esp:"

#if 0
#define DBGF(...) ESP_LOGI(TAG, __VA_ARGS__)
#else
#define DBGF(...)
#endif

class UartProtocolInt : public UartProtocol {
public:
    QueueHandle_t uartQueue;
    std::string   currentPath;
    uint8_t       rxBuf[16 + 0x10000];
    int           rxBufIdx = -1;
    bool          rxEscape = false;
    uint8_t       txBuf[256];
    int           txBufIdx = 0;
    VFS          *fdVfs[MAX_FDS];
    uint8_t       fds[MAX_FDS];
    DirEnumCtx    deCtxs[MAX_DDS];
    int           deIdx[MAX_DDS];
    const char   *newPath = nullptr;

    UartProtocolInt() {
        for (int i = 0; i < MAX_FDS; i++) {
            fdVfs[i] = nullptr;
            fds[i]   = 0;
        }
        for (int i = 0; i < MAX_DDS; i++) {
            deCtxs[i] = nullptr;
            deIdx[i]  = 0;
        }
    }

    void init() override {
        // Initialize UART to FPGA
        uart_config_t uart_config = {
            .baud_rate           = CONFIG_UARTPROTOCOL_BAUDRATE,
            .data_bits           = UART_DATA_8_BITS,
            .parity              = UART_PARITY_DISABLE,
            .stop_bits           = UART_STOP_BITS_1,
            .flow_ctrl           = UART_HW_FLOWCTRL_CTS_RTS,
            .rx_flow_ctrl_thresh = 122,
            .source_clk          = UART_SCLK_DEFAULT,
        };
        ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM, IOPIN_UART_TX, IOPIN_UART_RX, IOPIN_UART_RTS, IOPIN_UART_CTS));

        // Setup UART buffered IO with event queue
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uartQueue, 0));
        uart_pattern_queue_reset(UART_NUM, 20);

        uint32_t baudrate;
        ESP_ERROR_CHECK(uart_get_baudrate(UART_NUM, &baudrate));
        ESP_LOGI(TAG, "Actual baudrate: %lu", baudrate);

        getEspVFS()->init();
        getHttpVFS()->init();
        getTcpVFS()->init();

        if (xTaskCreate(_uartEventTask, "uartEvent", 6144, this, 1, nullptr) != pdPASS) {
            ESP_LOGE(TAG, "Error creating uartEvent task");
        }
    }

    static void _uartEventTask(void *param) { static_cast<UartProtocolInt *>(param)->uartEventTask(); }

    void uartEventTask() {
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
                            auto val = buf[i];
                            if (val == 0x7E) {
                                // Start of frame
                                rxBufIdx = 0;
                                rxEscape = false;
                                continue;
                            }
                            if (rxBufIdx < 0)
                                continue;
                            if (val == 0x7D) {
                                rxEscape = true;
                                continue;
                            }
                            if (rxEscape) {
                                val ^= 0x20;
                                rxEscape = false;
                            }
                            receivedByte(val);
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
                        ESP_LOGW(TAG, "rx break detected");
                        break;

                    case UART_PARITY_ERR: ESP_LOGW(TAG, "UART parity error"); break;
                    case UART_FRAME_ERR: ESP_LOGW(TAG, "UART frame error"); break;
                    default: ESP_LOGW(TAG, "UART event type: %d", event.type); break;
                }
            }
        }
    }

    void txBufFlush() {
        if (txBufIdx > 0) {
            // ESP_LOG_BUFFER_HEXDUMP(TAG, txBuf, txBufIdx, ESP_LOG_INFO);
            uart_write_bytes(UART_NUM, txBuf, txBufIdx);
        }
        txBufIdx = 0;
    }
    void txBufPush(uint8_t val) {
        txBuf[txBufIdx++] = val;
        if (txBufIdx >= sizeof(txBuf))
            txBufFlush();
    }
    void txStart() override {
        // Aq+ can't handle this for now:
        // txBufPush(0x7E);
    }
    void txWrite(uint8_t data) override {
        if (data == 0x7D || data == 0x7E) {
            txBufPush(0x7D);
            txBufPush(data ^ 0x20);
        } else {
            txBufPush(data);
        }
    }
    void txWrite(const void *buf, size_t length) override {
        auto p = static_cast<const uint8_t *>(buf);
        while (length--)
            txWrite(*(p++));
    }

    void receivedByte(uint8_t data) {
        rxBuf[rxBufIdx] = data;
        if (rxBufIdx < (int)sizeof(rxBuf) - 1) {
            rxBufIdx++;
        }
        // ESP_LOG_BUFFER_HEXDUMP(TAG, rxBuf, rxBufIdx, ESP_LOG_INFO);

        switch (rxBuf[0]) {
            case ESPCMD_RESET: {
                cmdReset();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_VERSION: {
                cmdVersion();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_GETDATETIME: {
                if (rxBufIdx == 2) {
                    uint8_t type = rxBuf[1];
                    cmdGetDateTime(type);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_KEYMODE: {
                if (rxBufIdx == 2) {
                    uint8_t keyMode = rxBuf[1];
                    cmdKeyMode(keyMode);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_GETGAMECTRL: {
                if (rxBufIdx == 2) {
                    uint8_t ctrlIdx = rxBuf[1];
                    cmdGetGameCtrl(ctrlIdx);
                    rxBufIdx = 0;
                }
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
                    const void *buf  = &rxBuf[4];
                    if (rxBufIdx == 4 + size) {
                        cmdWrite(fd, size, buf);
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
                    cmdOpenDirExt(pathArg, 0, 0);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_OPENDIR83: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *pathArg = (const char *)&rxBuf[1];
                    cmdOpenDirExt(pathArg, DE_FLAG_MODE83, 0);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_OPENDIREXT: {
                if (data == 0 && rxBufIdx >= 5) {
                    uint8_t     flags      = rxBuf[1];
                    uint16_t    skip_count = rxBuf[2] | (rxBuf[3] << 8);
                    const char *pathArg    = (const char *)&rxBuf[4];
                    cmdOpenDirExt(pathArg, flags, skip_count);
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
            case ESPCMD_READLINE: {
                if (rxBufIdx == 4) {
                    uint8_t  fd   = rxBuf[1];
                    uint16_t size = rxBuf[2] | (rxBuf[3] << 8);
                    cmdReadLine(fd, size);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_LOADFPGA: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdLoadFpga((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            default: {
                int result = -1;

                auto core = getFpgaCore();
                if (core) {
                    result = core->uartCommand(rxBuf[0], rxBuf + 1, rxBufIdx - 1);
                }

                if (result < 0) {
                    DBGF("Invalid command: 0x%02X", rxBuf[0]);
                }
                if (result != 0)
                    rxBufIdx = 0;
                break;
            }
        }

        txBufFlush();
    }

    void cmdReset() {
        DBGF("RESET()");
        closeAllDescriptors();
        currentPath.clear();
    }
    void cmdVersion() {
        DBGF("VERSION()");

        const char            *p       = "Unknown";
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_app_desc_t         running_app_info;
        if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
            p = running_app_info.version;
        }

        txStart();
        txWrite(p, strlen(p) + 1);
    }
    void cmdGetDateTime(uint8_t type) {
        DBGF("GETDATETIME(type=0x%02X)", type);
        txStart();

        if (type != 0) {
            txWrite(ERR_PARAM);
            return;
        }

        time_t now;
        time(&now);
        struct tm timeinfo = *localtime(&now);

        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%Y%m%d%H%M%S", &timeinfo);

        txWrite(0);
        txWrite(strftime_buf, strlen(strftime_buf) + 1);
    }
    void cmdKeyMode(uint8_t mode) {
        DBGF("KEYMODE(mode=0x%02X)", mode);
        getKeyboard()->setKeyMode(mode);
        txStart();
        txWrite(0);
    }
    void cmdGetGameCtrl(uint8_t idx) {
        DBGF("GETGAMECTRL(idx=%u)", idx);
        // FIXME
        txStart();
        txWrite(ERR_NOT_FOUND);
    }
    void cmdOpen(uint8_t flags, const char *pathArg) {
        DBGF("OPEN(flags=0x%02X, path='%s')", flags, pathArg);
        txStart();

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
            txWrite(ERR_TOO_MANY_OPEN);
            return;
        }

        // Compose full path
        VFS *vfs  = nullptr;
        auto path = resolvePath(pathArg, &vfs);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }

        int vfs_fd = vfs->open(flags, path);
        if (vfs_fd < 0) {
            txWrite(vfs_fd);
        } else {
            fdVfs[fd] = vfs;
            fds[fd]   = vfs_fd;
            txWrite(fd);
        }
    }
    void cmdClose(uint8_t fd) {
        DBGF("CLOSE(fd=%u)", fd);
        txStart();

        if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        txWrite(fdVfs[fd]->close(fds[fd]));
        fdVfs[fd] = nullptr;
    }
    void cmdRead(uint8_t fd, uint16_t size) {
        DBGF("READ(fd=%u, size=%u)", fd, size);
        txStart();

        if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        int result = fdVfs[fd]->read(fds[fd], size, rxBuf);
        if (result < 0) {
            txWrite(result);
        } else {
            txWrite(0);
            txWrite((result >> 0) & 0xFF);
            txWrite((result >> 8) & 0xFF);
            txWrite(rxBuf, result);
        }
    }
    void cmdReadLine(uint8_t fd, uint16_t size) {
        DBGF("READLINE(fd=%u, size=%u)", fd, size);
        txStart();

        if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        int result = fdVfs[fd]->readline(fds[fd], size, rxBuf);
        if (result < 0) {
            txWrite(result);
        } else {
            txWrite(0);

            const uint8_t *p = rxBuf;
            while (*p) {
                if (*p == '\r' || *p == '\n')
                    break;
                txWrite(*(p++));
            }
            txWrite(0);
        }
    }
    void cmdWrite(uint8_t fd, uint16_t size, const void *data) {
        DBGF("WRITE(fd=%u, size=%u, data=...)", fd, size);
        txStart();

        if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        int result = fdVfs[fd]->write(fds[fd], size, data);
        if (result < 0) {
            txWrite(result);
        } else {
            txWrite(0);
            txWrite((result >> 0) & 0xFF);
            txWrite((result >> 8) & 0xFF);
        }
    }
    void cmdSeek(uint8_t fd, uint32_t offset) {
        DBGF("SEEK(fd=%u, offset=%u)", fd, (unsigned)offset);
        txStart();

        if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        txWrite(fdVfs[fd]->seek(fds[fd], offset));
    }
    void cmdTell(uint8_t fd) {
        DBGF("TELL(fd=%u)", fd);
        txStart();

        if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        int result = fdVfs[fd]->tell(fds[fd]);
        if (result < 0) {
            txWrite(result);
        } else {
            txWrite(0);
            txWrite((result >> 0) & 0xFF);
            txWrite((result >> 8) & 0xFF);
            txWrite((result >> 16) & 0xFF);
            txWrite((result >> 24) & 0xFF);
        }
    }
    void cmdOpenDirExt(const char *pathArg, uint8_t flags, uint16_t skipCount) {
        DBGF("OPENDIREXT(path='%s', flags=0x%02X, skipCount=%u)", pathArg, flags, skipCount);
        txStart();

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
            txWrite(ERR_TOO_MANY_OPEN);
            return;
        }

        // Compose full path
        VFS        *vfs = nullptr;
        std::string wildCard;
        auto        path = resolvePath(pathArg, &vfs, &wildCard);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }

        auto [result, deCtx] = vfs->direnum(path, flags);
        if (result < 0) {
            txWrite(result);
        } else {
            if (!path.empty() && (flags & DE_FLAG_DOTDOT) != 0) {
                deCtx->push_back(DirEnumEntry("..", 0, DE_ATTR_DIR, 0, 0));
            }

            if (!wildCard.empty()) {
                deCtx->erase(
                    std::remove_if(deCtx->begin(), deCtx->end(), [&](DirEnumEntry &de) {
                        if ((de.attr & DE_ATTR_DIR) != 0 && (flags & DE_FLAG_ALWAYS_DIRS))
                            return false;
                        return !wildcardMatch(de.filename, wildCard);
                    }),
                    deCtx->end());
            }

            std::sort(deCtx->begin(), deCtx->end(), [](auto &a, auto &b) {
                // Sort directories at the top
                if ((a.attr & DE_ATTR_DIR) != (b.attr & DE_ATTR_DIR)) {
                    return (a.attr & DE_ATTR_DIR) != 0;
                }
                return strcasecmp(a.filename.c_str(), b.filename.c_str()) < 0;
            });

            deCtxs[dd] = deCtx;
            deIdx[dd]  = skipCount;
            txWrite(dd);
        }
    }
    void cmdCloseDir(uint8_t dd) {
        DBGF("CLOSEDIR(dd=%u)", dd);
        txStart();

        if (dd >= MAX_DDS || deCtxs[dd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }
        deCtxs[dd] = nullptr;
        txWrite(0);
    }
    void cmdReadDir(uint8_t dd) {
        DBGF("READDIR(dd=%u)", dd);
        txStart();

        if (dd >= MAX_DDS || deCtxs[dd] == nullptr) {
            txWrite(ERR_PARAM);
            return;
        }

        auto ctx = deCtxs[dd];
        if (deIdx[dd] >= (int)((*ctx).size())) {
            txWrite(ERR_EOF);
            return;
        }

        auto &de = (*ctx)[deIdx[dd]++];
        txWrite(0);
        txWrite((de.fdate >> 0) & 0xFF);
        txWrite((de.fdate >> 8) & 0xFF);
        txWrite((de.ftime >> 0) & 0xFF);
        txWrite((de.ftime >> 8) & 0xFF);
        txWrite(de.attr);
        txWrite((de.size >> 0) & 0xFF);
        txWrite((de.size >> 8) & 0xFF);
        txWrite((de.size >> 16) & 0xFF);
        txWrite((de.size >> 24) & 0xFF);
        txWrite(de.filename.c_str(), de.filename.size());
        txWrite(0);
    }
    void cmdDelete(const char *pathArg) {
        DBGF("DELETE(path='%s')", pathArg);
        txStart();

        VFS *vfs  = nullptr;
        auto path = resolvePath(pathArg, &vfs);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }
        txWrite(vfs->delete_(path));
    }
    void cmdRename(const char *oldArg, const char *newArg) {
        DBGF("RENAME(old='%s', new='%s')", oldArg, newArg);
        txStart();

        VFS *vfs1     = nullptr;
        VFS *vfs2     = nullptr;
        auto _oldPath = resolvePath(oldArg, &vfs1);
        auto _newPath = resolvePath(newArg, &vfs2);
        if (!vfs1 || vfs1 != vfs2) {
            txWrite(ERR_PARAM);
            return;
        }

        txWrite(vfs1->rename(_oldPath, _newPath));
    }
    void cmdMkDir(const char *pathArg) {
        DBGF("MKDIR(path='%s)", pathArg);
        txStart();

        VFS *vfs  = nullptr;
        auto path = resolvePath(pathArg, &vfs);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }
        txWrite(vfs->mkdir(path));
    }
    void cmdChDir(const char *pathArg) {
        DBGF("CHDIR(path='%s')", pathArg);
        txStart();

        // Compose full path
        VFS *vfs  = nullptr;
        auto path = resolvePath(pathArg, &vfs);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }

        struct stat st;
        int         result = vfs->stat(path, &st);
        if (result == 0) {
            if (st.st_mode & S_IFDIR) {
                if (vfs == getEspVFS()) {
                    currentPath = std::string(ESP_PREFIX) + path;
                } else {
                    currentPath = path;
                }
            } else {
                result = ERR_PARAM;
            }
        }
        txWrite(result);
    }
    void cmdStat(const char *pathArg) {
        DBGF("STAT(path='%s')", pathArg);
        txStart();

        VFS *vfs  = nullptr;
        auto path = resolvePath(pathArg, &vfs);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }

        struct stat st;
        int         result = vfs->stat(path, &st);

        txWrite(result);
        if (result < 0)
            return;

        time_t t = st.st_mtim.tv_sec;

        struct tm *tm       = localtime(&t);
        uint16_t   fat_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
        uint16_t   fat_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

        txWrite((fat_date >> 0) & 0xFF);
        txWrite((fat_date >> 8) & 0xFF);
        txWrite((fat_time >> 0) & 0xFF);
        txWrite((fat_time >> 8) & 0xFF);
        txWrite((st.st_mode & S_IFDIR) != 0 ? DE_ATTR_DIR : 0);
        txWrite((st.st_size >> 0) & 0xFF);
        txWrite((st.st_size >> 8) & 0xFF);
        txWrite((st.st_size >> 16) & 0xFF);
        txWrite((st.st_size >> 24) & 0xFF);
    }
    void cmdGetCwd() {
        DBGF("GETCWD()");
        txStart();

        txWrite(0);
        txWrite('/');
        txWrite(currentPath.c_str(), currentPath.size() + 1);
    }
    void cmdCloseAll() {
        DBGF("CLOSEALL()");
        txStart();

        closeAllDescriptors();
        txWrite(0);
    }
    void cmdLoadFpga(const char *pathArg) {
        DBGF("LOADFPGA(path='%s')", pathArg);
        txStart();

        VFS *vfs  = nullptr;
        auto path = resolvePath(pathArg, &vfs);
        if (!vfs) {
            txWrite(ERR_PARAM);
            return;
        }

        struct stat st;
        int         result = vfs->stat(path, &st);
        if (result < 0) {
            txWrite(result);
            return;
        }
        if ((st.st_mode & S_IFREG) == 0) {
            txWrite(ERR_PARAM);
            return;
        }

        void *buf = malloc(st.st_size);
        if (buf == nullptr) {
            printf("Insufficient memory to allocate buffer for bitstream!\n");
            txWrite(ERR_OTHER);
            return;
        }

        int vfs_fd = vfs->open(FO_RDONLY, path);
        if (vfs_fd < 0) {
            txWrite(vfs_fd);
        } else {
            vfs->read(vfs_fd, st.st_size, buf);
            vfs->close(vfs_fd);
            txWrite(0);

            printf("Loading bitstream: %s (%u bytes)\n", pathArg, (unsigned)st.st_size);

            auto newCore = loadFpgaCore(FpgaCoreType::AquariusPlus, buf, st.st_size);
            if (!newCore) {
                printf("Failed! Loading default bitstream\n");

                // Restore Aq+ firmware
                loadFpgaCore(FpgaCoreType::AquariusPlus);
            }
            cmdCloseAll();
        }

        free(buf);
    }

    std::string resolvePath(std::string path, VFS **vfs, std::string *wildCard = nullptr) {
        *vfs = getSDCardVFS();

        if (startsWith(path, "http://") || startsWith(path, "https://")) {
            *vfs = getHttpVFS();
            return path;
        }
        if (startsWith(path, "tcp://")) {
            *vfs = getTcpVFS();
            return path;
        }

        bool useCwd = true;
        if (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
            useCwd = false;
        } else if (startsWith(path, ESP_PREFIX)) {
            useCwd = false;
            *vfs   = getEspVFS();
            path   = path.substr(strlen(ESP_PREFIX));
        }

        // Split the path into parts
        std::vector<std::string> parts;
        if (useCwd) {
            if (startsWith(currentPath, ESP_PREFIX)) {
                splitPath(currentPath.substr(strlen(ESP_PREFIX)), parts);
                *vfs = getEspVFS();
            } else {
                splitPath(currentPath, parts);
            }
        }
        splitPath(path, parts);

        // Resolve path
        int idx = 0;
        while (idx < (int)parts.size()) {
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

        if (!parts.empty() && wildCard != nullptr) {
            const auto &lastPart = parts.back();
            if (lastPart.find_first_of("?*") != lastPart.npos) {
                // Contains wildcard, return it separately
                *wildCard = lastPart;
                parts.pop_back();
            }
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

    static bool wildcardMatch(const std::string &text, const std::string &pattern) {
        // Initialize the pointers to the current positions in the text and pattern strings.
        int textPos    = 0;
        int patternPos = 0;

        // Loop while we have not reached the end of either string.
        while (textPos < (int)text.size() && patternPos < (int)pattern.size()) {
            if (pattern[patternPos] == '*') {
                // Skip asterisk (and any following asterisks)
                while (patternPos < (int)pattern.size() && pattern[patternPos] == '*') {
                    patternPos++;
                }
                // If end of the pattern then we have a match
                if (patternPos == (int)pattern.size()) {
                    return true;
                }
                // Skip characters in text until we find one that matches the current pattern character
                while (tolower(text[textPos]) != tolower(pattern[patternPos])) {
                    textPos++;

                    // Reached end of text, but not end of pattern, no match
                    if (textPos == (int)text.size())
                        return false;
                }
                continue;
            }

            // Check character match
            if (pattern[patternPos] != '?' && (tolower(text[textPos]) != tolower(pattern[patternPos])))
                return false;

            textPos++;
            patternPos++;
        }

        // If we reached the end of both strings, then the match is successful.
        return (textPos == (int)text.size() && patternPos == (int)pattern.size());
    }

    void closeAllDescriptors() {
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
};

UartProtocol *getUartProtocol() {
    static UartProtocolInt *obj = nullptr;
    if (!obj) {
        obj = new UartProtocolInt();
        assert(obj != nullptr);
    }
    return obj;
}
