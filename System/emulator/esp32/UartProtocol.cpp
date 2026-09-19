#include "UartProtocol.h"

#include <esp_ota_ops.h>
#ifndef EMULATOR
#include <driver/uart.h>
#else
#include "EmuState.h"
#endif

#include "VFS.h"
#include "FpgaCore.h"
#include "MidiData.h"

#ifndef EMULATOR
static const char *TAG = "UartProtocol";

#define UART_NUM (UART_NUM_1)
#define BUF_SIZE (1024)
#endif

#if 0
#ifndef EMULATOR
#define DBGF(...) ESP_LOGI(TAG, __VA_ARGS__)
#else
static void dprintf(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintf(fmt, va);
    printf("\n");
    va_end(va);
}

#define DBGF(...) dprintf(__VA_ARGS__)
#endif
#else
#define DBGF(...)
#endif

class UartProtocolInt : public UartProtocol {
public:
#ifndef EMULATOR
    QueueHandle_t uartQueue;
    bool          rxEscape = false;
    uint8_t       txBuf[256];
    int           txBufIdx = 0;
#else
    uint8_t  txBuf[16 + 0x10000];
    unsigned txBufWrIdx = 0;
    unsigned txBufRdIdx = 0;
    unsigned txBufCnt   = 0;
#endif
    uint8_t     rxBuf[16 + 0x10000];
    int         rxBufIdx = -1;
    const char *newPath  = nullptr;

    UartProtocolInt() {
    }

    void init() override {
#ifndef EMULATOR
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
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 256, &uartQueue, 0));

        uint32_t baudrate;
        ESP_ERROR_CHECK(uart_get_baudrate(UART_NUM, &baudrate));
        ESP_LOGI(TAG, "Actual baudrate: %lu", baudrate);
#endif

        getEspVFS()->init();
        getHttpVFS()->init();
        getTcpVFS()->init();

#ifndef EMULATOR
        if (xTaskCreate(_uartEventTask, "uartEvent", 6144, this, 1, nullptr) != pdPASS) {
            ESP_LOGE(TAG, "Error creating uartEvent task");
        }
#endif
    }

    void setBaudrate(unsigned baudrate) override {
#ifndef EMULATOR
        ESP_LOGI(TAG, "Setting baudrate to %u bps\n", baudrate);
        uart_set_baudrate(UART_NUM, baudrate);
#endif
    }

#ifdef EMULATOR
    void writeData(uint8_t data) override {
        receivedByte(data);
    }

    void writeCtrl(uint8_t data) override {
        if (data & 0x80) {
            rxBufIdx = 0;
        }
    }

    uint8_t readData() override {
        int data = txFifoRead();
        if (data < 0) {
            printf("esp32_read_data - Empty!\n");
            return 0;
        }
        return data;
    }

    uint8_t readCtrl() override {
        uint8_t result = 0;
        if (txBufCnt > 0) {
            result |= 1;
        }
        return result;
    }

    int txFifoRead() {
        int result = -1;
        if (txBufCnt > 0) {
            result = txBuf[txBufRdIdx++];
            txBufCnt--;
            if (txBufRdIdx >= sizeof(txBuf)) {
                txBufRdIdx = 0;
            }
        }
        return result;
    }
#endif

#ifndef EMULATOR
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
                    case UART_DATA: {
                        int len = uart_read_bytes(UART_NUM, buf.data(), event.size, portMAX_DELAY);
                        assert(len >= 0);
                        for (unsigned i = 0; i < len; i++) {
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
                    }

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
#endif
    void txBufFlush() {
#ifndef EMULATOR
        if (txBufIdx > 0) {
            // ESP_LOG_BUFFER_HEXDUMP(TAG, txBuf, txBufIdx, ESP_LOG_INFO);
            uart_write_bytes(UART_NUM, txBuf, txBufIdx);
        }
        txBufIdx = 0;
#endif
    }
#ifndef EMULATOR
    void txBufPush(uint8_t val) {
        txBuf[txBufIdx++] = val;
        if (txBufIdx >= sizeof(txBuf))
            txBufFlush();
    }
#endif
    void txStart() override {
#ifndef EMULATOR
        // Aq+ can't handle this for now:
        // txBufPush(0x7E);
#endif
    }
    void txWrite(uint8_t data) override {
#ifndef EMULATOR
        if (data == 0x7D || data == 0x7E) {
            txBufPush(0x7D);
            txBufPush(data ^ 0x20);
        } else {
            txBufPush(data);
        }
#else
        if (txBufCnt >= sizeof(txBuf))
            return;

        txBuf[txBufWrIdx++] = data;
        txBufCnt++;
        if (txBufWrIdx >= sizeof(txBuf)) {
            txBufWrIdx = 0;
        }
#endif
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
                auto core = FpgaCore::get();
                if (core)
                    core->uartCommand(rxBuf[0], rxBuf + 1, 1);

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
            case ESPCMD_GETGAMECTRL: {
                if (rxBufIdx == 2) {
                    uint8_t ctrlIdx = rxBuf[1];
                    cmdGetGameCtrl(ctrlIdx);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_GETMIDIDATA: {
                if (rxBufIdx == 3) {
                    uint16_t size = rxBuf[1] | (rxBuf[2] << 8);
                    cmdGetMidiData(size);
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
                    if (rxBufIdx == (int)(4 + size)) {
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
            case ESPCMD_LSEEK: {
                if (rxBufIdx == 7) {
                    uint8_t fd     = rxBuf[1];
                    int     offset = (int)((rxBuf[2] << 0) | (rxBuf[3] << 8) | (rxBuf[4] << 16) | (rxBuf[5] << 24));
                    int     whence = rxBuf[6];
                    cmdLSeek(fd, offset, whence);
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
                    uint8_t     flags     = rxBuf[1];
                    uint16_t    skipCount = rxBuf[2] | (rxBuf[3] << 8);
                    const char *pathArg   = (const char *)&rxBuf[4];
                    cmdOpenDirExt(pathArg, flags, skipCount);
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

                auto core = FpgaCore::get();
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
        VFSContext::getDefault()->reset();
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
    void cmdGetGameCtrl(uint8_t idx) {
        DBGF("GETGAMECTRL(idx=%u)", idx);
        GamePadData data;

        auto core = FpgaCore::get();
        if (!core || !core->getGamePadData(idx, data)) {
            txStart();
            txWrite(ERR_NOT_FOUND);
        } else {
            txStart();
            txWrite(0);
            txWrite(data.lx);
            txWrite(data.ly);
            txWrite(data.rx);
            txWrite(data.ry);
            txWrite(data.lt);
            txWrite(data.rt);
            txWrite(data.buttons & 0xFF);
            txWrite(data.buttons >> 8);
        }
    }
    void cmdGetMidiData(uint16_t size) {
        DBGF("GETMIDIDATA(size=%u)", size);
        txStart();
        txWrite(0);
        auto midiData = MidiData::instance();
        auto count    = midiData->getDataCount();
        if (count * 4 > size) {
            count = size / 4;
        }
        size = count * 4;
        txWrite((size >> 0) & 0xFF);
        txWrite((size >> 8) & 0xFF);

        while (count--) {
            uint8_t buf[4];
            midiData->getData(buf);
            txWrite(buf[0]);
            txWrite(buf[1]);
            txWrite(buf[2]);
            txWrite(buf[3]);
        }
    }
    void cmdOpen(uint8_t flags, const char *pathArg) {
        DBGF("OPEN(flags=0x%02X, path='%s')", flags, pathArg);
        txStart();
        int result = VFSContext::getDefault()->open(flags, pathArg);
        txWrite(result);
    }
    void cmdClose(uint8_t fd) {
        DBGF("CLOSE(fd=%u)", fd);
        txStart();
        int result = VFSContext::getDefault()->close(fd);
        txWrite(result);
    }
    void cmdRead(uint8_t fd, uint16_t size) {
        DBGF("READ(fd=%u, size=%u)", fd, size);
        txStart();
        int result = VFSContext::getDefault()->read(fd, size, rxBuf);
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
        int result = VFSContext::getDefault()->readline(fd, size, rxBuf);
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
        int result = VFSContext::getDefault()->write(fd, size, data);
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
        int result = VFSContext::getDefault()->seek(fd, offset);
        txWrite(result);
    }
    void cmdLSeek(uint8_t fd, int offset, int whence) {
        DBGF("LSEEK(fd=%u, offset=%d, whence=%d)", fd, offset, whence);
        txStart();
        int result = VFSContext::getDefault()->lseek(fd, offset, whence);
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
    void cmdTell(uint8_t fd) {
        DBGF("TELL(fd=%u)", fd);
        txStart();
        int result = VFSContext::getDefault()->tell(fd);
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
        int result = VFSContext::getDefault()->openDirExt(pathArg, flags, skipCount);
        txWrite(result);
    }
    void cmdCloseDir(uint8_t dd) {
        DBGF("CLOSEDIR(dd=%u)", dd);
        txStart();
        int result = VFSContext::getDefault()->closeDir(dd);
        txWrite(result);
    }
    void cmdReadDir(uint8_t dd) {
        DBGF("READDIR(dd=%u)", dd);
        txStart();
        DirEnumEntry de;
        int          result = VFSContext::getDefault()->readDir(dd, &de);
        if (result < 0) {
            txWrite(result);
            return;
        }

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
        int result = VFSContext::getDefault()->delete_(pathArg);
        txWrite(result);
    }
    void cmdRename(const char *oldArg, const char *newArg) {
        DBGF("RENAME(old='%s', new='%s')", oldArg, newArg);
        txStart();
        int result = VFSContext::getDefault()->rename(oldArg, newArg);
        txWrite(result);
    }
    void cmdMkDir(const char *pathArg) {
        DBGF("MKDIR(path='%s)", pathArg);
        txStart();
        int result = VFSContext::getDefault()->mkdir(pathArg);
        txWrite(result);
    }
    void cmdChDir(const char *pathArg) {
        DBGF("CHDIR(path='%s')", pathArg);
        txStart();
        int result = VFSContext::getDefault()->chdir(pathArg);
        txWrite(result);
    }
    void cmdStat(const char *pathArg) {
        DBGF("STAT(path='%s')", pathArg);
        txStart();
        struct stat st;
        int         result = VFSContext::getDefault()->stat(pathArg, &st);

        txWrite(result);
        if (result < 0)
            return;

#ifdef __APPLE__
        time_t t = st.st_mtimespec.tv_sec;
#elif _WIN32
        time_t t = st.st_mtime;
#else
        time_t t = st.st_mtim.tv_sec;
#endif

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

        auto currentPath = VFSContext::getDefault()->getCurrentPath();
        txWrite(0);
        txWrite('/');
        txWrite(currentPath.c_str(), currentPath.size() + 1);
    }
    void cmdCloseAll() {
        DBGF("CLOSEALL()");
        txStart();

        VFSContext::getDefault()->closeAll();
        txWrite(0);
    }
    void cmdLoadFpga(const char *pathArg) {
        DBGF("LOADFPGA(path='%s')", pathArg);
        txStart();

        auto        vc = VFSContext::getDefault();
        struct stat st;
        int         result = vc->stat(pathArg, &st);
        if (result < 0) {
            txWrite(result);
            return;
        }
        if ((st.st_mode & S_IFREG) == 0) {
            txWrite(ERR_PARAM);
            return;
        }
        if (FpgaCore::loadCore(pathArg) == nullptr) {
            txWrite(ERR_OTHER);
            return;
        }
    }
};

UartProtocol *UartProtocol::instance() {
    static UartProtocolInt *obj = nullptr;
    if (obj == nullptr) {
        obj = new UartProtocolInt();
        assert(obj != nullptr);
    }
    return obj;
}
