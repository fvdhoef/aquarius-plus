#include "EmuState.h"
#include "DCBlock.h"
#include "UartProtocol.h"
#include "FPGA.h"
#include "cpu/riscv.h"
#include "Config.h"
#include "bootrom.h"
#include "Aq32Video.h"
#include "Aq32FmSynth.h"
#include "Aq32Pcm.h"
#include "imgui.h"
#include "Keyboard.h"
#include <chrono>

#ifndef WIN32
#define GDB_ENABLE
#endif

#ifdef GDB_ENABLE
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <netinet/tcp.h>
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "MemoryEditor.h"

#define HCYCLES_PER_LINE   (455)
#define HCYCLES_PER_SAMPLE (162)

#define BASE_BOOTROM      0x00000
#define REG_ESPCTRL       0x02000
#define REG_ESPDATA       0x02004
#define REG_KEYBUF        0x02010
#define REG_HCTRL         0x02014
#define REG_KEYS_L        0x02018
#define REG_KEYS_H        0x0201C
#define REG_GAMEPAD1_L    0x02018
#define REG_GAMEPAD1_H    0x0201C
#define REG_GAMEPAD2_L    0x02018
#define REG_GAMEPAD2_H    0x0201C
#define REG_VCTRL         0x02040
#define REG_VLINE         0x02048
#define REG_VIRQLINE      0x0204C
#define REG_VSCRX1        0x02050
#define REG_VSCRY1        0x02054
#define REG_VSCRX2        0x02058
#define REG_VSCRY2        0x0205C
#define REG_MTIME         0x02080
#define REG_MTIMEH        0x02084
#define REG_MTIMECMP      0x02088
#define REG_MTIMECMPH     0x0208C
#define REG_PCM_STATUS    0x02400
#define REG_PCM_FIFO_CTRL 0x02404
#define REG_PCM_RATE      0x02408
#define REG_PCM_FIFO_DATA 0x0240C
#define BASE_FMSYNTH      0x02800
#define BASE_SPRATTR      0x03000
#define BASE_SPRPOS       0x03400
#define BASE_PALETTE      0x04000
#define BASE_CHRAM        0x05000
#define BASE_TEXTRAM      0x06000
#define BASE_VRAM         0x08000
#define BASE_VRAM4BIT     0x10000
#define BASE_MAINRAM      0x80000

// #define ALLOW_UNALIGNED

#ifdef GDB_ENABLE
static std::string bufToHex(const void *buf, unsigned size) {
    std::string result;

    auto p = reinterpret_cast<const uint8_t *>(buf);
    while (size--)
        result += fmtstr("%02X", *(p++));

    return result;
}
static std::vector<uint8_t> decodeHex(const char *p, size_t size) {
    std::vector<uint8_t> data;

    for (unsigned i = 0; i < size; i++) {
        uint8_t val = 0;

        for (int j = 0; j < 2; j++) {
            uint8_t ch = *(p++);
            if (ch >= '0' && ch <= '9') {
                val = (val << 4) | (ch - '0');
            } else if (ch >= 'a' && ch <= 'f') {
                val = (val << 4) | (ch - 'a' + 10);
            } else if (ch >= 'A' && ch <= 'F') {
                val = (val << 4) | (ch - 'A' + 10);
            } else {
                return {};
            }
        }
        data.push_back(val);
    }
    return data;
}
#endif

static uint64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

class Aq32EmuState : public EmuState {
public:
    riscv                cpu;
    Aq32Video            video;
    Aq32FmSynth          fmsynth;
    Aq32Pcm              pcm;
    uint64_t             keybMatrix = 0;
    uint64_t             gamePad1   = 0;
    uint64_t             gamePad2   = 0;
    uint16_t             handCtrl   = 0xFFFF;
    std::deque<uint16_t> kbBuf;
    const unsigned       kbBufSize  = 16;
    unsigned             audioLeft  = 0;
    unsigned             audioRight = 0;
    DCBlock              dcBlockLeft;
    DCBlock              dcBlockRight;
    std::string          typeInStr;
    std::recursive_mutex mutex;
    uint32_t             mainRam[512 * 1024 / 4];
    uint32_t             bootRom[0x800 / 4];
    int                  curLineStepsRemaining = 0;
    uint64_t             mtimecmp              = 0;
    uint64_t             mtimeDiff             = 0;

#ifdef GDB_ENABLE
    // GDB interface
    std::thread gdbThread;
    bool        stopping = false;
#endif

    enum EmuMode {
        Em_Halted,
        Em_Step,
        Em_Running,
    };
    EmuMode emuMode = Em_Running;

    bool showCpuState      = false;
    bool showBreakpoints   = false;
    bool showIoRegsWindow  = false;
    bool showMemEdit       = false;
    int  memEditMemSelect  = 0;
    bool enableBreakpoints = false;
    bool hasSaved          = false;

    struct Breakpoint {
        uint32_t    addr = 0;
        std::string name;
        bool        enabled = false;
    };

    std::vector<Breakpoint> breakpoints;

    struct MemoryArea {
        MemoryArea(const std::string &_name, void *_data, size_t _size)
            : name(_name), data(_data), size(_size) {
        }
        std::string name;
        void       *data;
        size_t      size;
    };
    std::vector<MemoryArea> memAreas;
    MemoryEditor            memEdit;

    Aq32EmuState() {
        coreType         = 2;
        coreFlags        = 0x02;
        coreVersionMajor = 0;
        coreVersionMinor = 1;
        memcpy(coreName, "Aquarius32      ", sizeof(coreName));

        cpu.dataWrite8  = [this](uint32_t addr, uint8_t val) { _memWrite8(addr, val); };
        cpu.dataWrite16 = [this](uint32_t addr, uint16_t val) { _memWrite16(addr, val); };
        cpu.dataWrite32 = [this](uint32_t addr, uint32_t val) { _memWrite32(addr, val); };
        cpu.dataRead8   = [this](uint32_t addr) { return _memRead8(addr); };
        cpu.dataRead16  = [this](uint32_t addr) { return _memRead16(addr); };
        cpu.dataRead32  = [this](uint32_t addr) { return _memRead32(addr); };
        cpu.instrRead   = [this](uint32_t addr) { return (uint32_t)memRead(addr); };

        memset(&keybMatrix, 0xFF, sizeof(keybMatrix));
        memcpy(bootRom, bootrom_bin, bootrom_bin_len);
        loadConfig();
        reset();

#ifdef GDB_ENABLE
        gdbThread = std::thread([this] {
            this->gdbThreadFunc();
        });
#endif
    }

    void unloading() override {
        saveConfig();
    }

    virtual ~Aq32EmuState() {
        unloading();
#ifdef GDB_ENABLE
        stopping = true;
        gdbThread.join();
#endif
    }

    void setMtime(uint64_t newVal) {
        mtimeDiff = newVal - now_ms();
        cpu.mtime = newVal;
    }
    uint64_t getMtime() {
        return now_ms() + mtimeDiff;
    }

    void reset(bool cold = false) override {
        // CPU reset
        cpu.regs[0]      = 0;
        cpu.pc           = 0;
        cpu.mstatus_mie  = false;
        cpu.mstatus_mpie = false;
        cpu.mie          = 0;
        cpu.mtvec        = 0;
        cpu.mscratch     = 0;
        cpu.mepc         = 0;
        cpu.mcause       = 0;
        cpu.mtval        = 0;
        cpu.mip          = 0;
        cpu.trap         = 0;
        setMtime(0);
        mtimecmp = 0;

        kbBuf.clear();
        video.reset();
        fmsynth.reset();
        pcm.reset();
    }

    void loadConfig() {
        auto root   = Config::instance()->loadConfigFile("aq32.json");
        showMemEdit = getBoolValue(root, "showMemEdit", false);
        // memEditMemSelect = getIntValue(root, "memEditMemSelect", 0);
        showCpuState     = getBoolValue(root, "showCpuState", false);
        showBreakpoints  = getBoolValue(root, "showBreakpoints", false);
        showIoRegsWindow = getBoolValue(root, "showIoRegsWindow", false);

        cJSON_Delete(root);
    }

    void saveConfig() {
        if (hasSaved)
            return;
        hasSaved  = true;
        auto root = cJSON_CreateObject();
        cJSON_AddBoolToObject(root, "showMemEdit", showMemEdit);
        // cJSON_AddNumberToObject(root, "memEditMemSelect", memEditMemSelect);
        cJSON_AddBoolToObject(root, "showCpuState", showCpuState);
        cJSON_AddBoolToObject(root, "showBreakpoints", showBreakpoints);
        cJSON_AddBoolToObject(root, "showIoRegsWindow", showIoRegsWindow);

        Config::instance()->saveConfigFile("aq32.json", root);
    }

    void getVideoSize(int &w, int &h) override {
        std::lock_guard lock(mutex);
        w = Aq32Video::activeWidth;
        h = Aq32Video::activeHeight * 2;
    }

    void getPixels(void *pixels, int pitch) override {
        std::lock_guard lock(mutex);
        const uint16_t *fb = video.getFb();
        for (int j = 0; j < Aq32Video::activeHeight * 2; j++) {
            for (int i = 0; i < Aq32Video::activeWidth; i++) {
                ((uint32_t *)((uintptr_t)pixels + j * pitch))[i] =
                    col12_to_col32(fb[(j / 2) * Aq32Video::activeWidth + i]);
            }
        }
        renderOverlay(pixels, pitch);
    }

    void spiTx(const void *data, size_t length) override {
        std::lock_guard lock(mutex);
        EmuState::spiTx(data, length);
        if (!spiSelected || txBuf.empty())
            return;

        switch (txBuf[0]) {
            case CMD_SET_KEYB_MATRIX: {
                if (txBuf.size() == 1 + 8) {
                    memcpy(&keybMatrix, &txBuf[1], 8);
                }
                break;
            }

            case CMD_SET_HCTRL: {
                if (txBuf.size() == 1 + 2) {
                    memcpy(&handCtrl, &txBuf[1], 2);
                }
                break;
            }

            case CMD_WRITE_KBBUF16: {
                if (txBuf.size() == 1 + 2 && kbBuf.size() < kbBufSize) {
                    kbBuf.push_back(txBuf[1] | (txBuf[2] << 8));
                }
                break;
            }

            case CMD_WRITE_GAMEPAD1: {
                if (txBuf.size() == 1 + 8) {
                    memcpy(&gamePad1, &txBuf[1], 8);
                }
                break;
            }

            case CMD_WRITE_GAMEPAD2: {
                if (txBuf.size() == 1 + 8) {
                    memcpy(&gamePad2, &txBuf[1], 8);
                }
                break;
            }
        }
    }

    void pasteText(const std::string &str) override {
        std::lock_guard lock(mutex);
        typeInStr = str;
    }
    bool pasteIsDone() override {
        std::lock_guard lock(mutex);
        return typeInStr.empty();
    }

    uint8_t _memRead8(uint32_t addr) {
        return (memRead(addr) >> ((addr & 3) * 8)) & 0xFF;
    }
    uint16_t _memRead16(uint32_t addr) {
        unsigned offset = addr & 3;
        addr &= ~1;
#ifndef ALLOW_UNALIGNED
        offset &= ~1;
#endif
        switch (offset) {
            default:
            case 0: return (memRead(addr) >> 0) & 0xFFFF;
            case 1: return (memRead(addr) >> 8) & 0xFFFF;
            case 2: return (memRead(addr) >> 16) & 0xFFFF;
            case 3: return ((memRead(addr) >> 24) & 0xFF) | ((memRead(addr + 4) << 8) & 0xFF00);
        }
    }
    uint32_t _memRead32(uint32_t addr) {
        unsigned offset = addr & 3;
        addr &= ~3;
#ifndef ALLOW_UNALIGNED
        offset = 0;
#endif
        switch (offset) {
            default:
            case 0: return (uint32_t)memRead(addr + 0);
            case 1: return ((memRead(addr + 0) & 0xFFFFFF00) >> 8) | ((memRead(addr + 4) & 0x000000FF) << 24);
            case 2: return ((memRead(addr + 0) & 0xFFFF0000) >> 16) | ((memRead(addr + 4) & 0x0000FFFF) << 16);
            case 3: return ((memRead(addr + 0) & 0xFF000000) >> 24) | ((memRead(addr + 4) & 0x00FFFFFF) << 8);
        }
    }

    void _memWrite8(uint32_t addr, uint8_t val) {
        memWrite(addr, (val << 24) | (val << 16) | (val << 8) | val, 0xFF << ((addr & 3) * 8));
    }
    void _memWrite16(uint32_t addr, uint16_t val) {
        unsigned offset = addr & 3;
        addr &= ~3;
#ifndef ALLOW_UNALIGNED
        offset &= ~1;
#endif
        switch (offset) {
            default:
            case 0:
                memWrite(addr + 0, val, 0x0000FFFF);
                break;
            case 1:
                memWrite(addr + 0, val << 8, 0x00FFFF00);
                break;
            case 2:
                memWrite(addr + 2, val << 16, 0xFFFF0000);
                break;
            case 3:
                memWrite(addr + 2, val << 24, 0xFF000000);
                memWrite(addr + 4, val >> 8, 0x000000FF);
                break;
        }
    }
    void _memWrite32(uint32_t addr, uint32_t val) {
        unsigned offset = addr & 3;
        addr &= ~3;
#ifndef ALLOW_UNALIGNED
        offset = 0;
#endif
        switch (offset) {
            default:
            case 0:
                memWrite(addr + 0, val, 0xFFFFFFFF);
                break;
            case 1:
                memWrite(addr + 0, val << 8, 0xFFFFFF00);
                memWrite(addr + 4, val >> 24, 0x000000FF);
                break;
            case 2:
                memWrite(addr + 0, val << 16, 0xFFFF0000);
                memWrite(addr + 4, val >> 16, 0x0000FFFF);
                break;
            case 3:
                memWrite(addr + 0, val << 24, 0xFF000000);
                memWrite(addr + 4, val >> 8, 0x00FFFFFF);
                break;
        }
    }

    int64_t memRead(uint32_t addr, bool allow_side_effect = true) {
        if (/* addr >= BASE_BOOTROM && */ addr < (BASE_BOOTROM + sizeof(bootRom))) {
            return bootRom[(addr & 0x7FF) / 4];
        } else if (addr == REG_ESPCTRL) {
            if (allow_side_effect)
                return UartProtocol::instance()->readCtrl();
            else
                return 0;
        } else if (addr == REG_ESPDATA) {
            if (allow_side_effect)
                return UartProtocol::instance()->readData();
            else
                return 0;
        } else if (addr == REG_KEYBUF) {
            uint32_t result = 0;
            if (kbBuf.empty()) {
                result = 1U << 31;
            } else {
                result = kbBuf.front();
                if (allow_side_effect)
                    kbBuf.pop_front();
            }
            return result;
        } else if (addr == REG_HCTRL) {
            return handCtrl;
        } else if (addr == REG_KEYS_L) {
            return (unsigned)(keybMatrix >> 32);
        } else if (addr == REG_KEYS_H) {
            return (unsigned)(keybMatrix & 0xFFFFFFFFU);
        } else if (addr == REG_GAMEPAD1_L) {
            return (unsigned)(gamePad1 >> 32);
        } else if (addr == REG_GAMEPAD1_H) {
            return (unsigned)(gamePad1 & 0xFFFFFFFFU);
        } else if (addr == REG_GAMEPAD2_L) {
            return (unsigned)(gamePad2 >> 32);
        } else if (addr == REG_GAMEPAD2_H) {
            return (unsigned)(gamePad2 & 0xFFFFFFFFU);
        } else if (addr == REG_VCTRL) {
            return video.videoCtrl;
        } else if (addr == REG_VLINE) {
            return video.videoLine;
        } else if (addr == REG_VIRQLINE) {
            return video.videoIrqLine;
        } else if (addr == REG_VSCRX1) {
            return video.videoScrX1;
        } else if (addr == REG_VSCRY1) {
            return video.videoScrY1;
        } else if (addr == REG_VSCRX2) {
            return video.videoScrX2;
        } else if (addr == REG_VSCRY2) {
            return video.videoScrY2;
        } else if (addr == REG_MTIME) {
            uint64_t mtime = getMtime();
            return (uint32_t)(mtime & 0xFFFFFFFFUL);
        } else if (addr == REG_MTIMEH) {
            uint64_t mtime = getMtime();
            return (uint32_t)(mtime >> 32);
        } else if (addr == REG_MTIMECMP) {
            return cpu.mtimecmp;
        } else if (addr == REG_MTIMECMPH) {
            return cpu.mtimecmp >> 32;
        } else if (addr == REG_PCM_STATUS || addr == REG_PCM_FIFO_DATA) {
            return pcm.data.size();
        } else if (addr == REG_PCM_FIFO_CTRL) {
            return pcm.fifoIrqThreshold;
        } else if (addr == REG_PCM_RATE) {
            return pcm.rate;
        } else if (addr >= BASE_FMSYNTH && addr < (BASE_FMSYNTH + 256 * 4)) {
            unsigned idx = (addr >> 2) & 255;
            if (idx == 0)
                return fmsynth.reg0_ch_4op;
            else if (idx == 1)
                return fmsynth.reg1;
            else if (idx == 2)
                return fmsynth.reg2_kon;
            else if (idx >= 96 && idx <= 127)
                return fmsynth.ch_attr[idx - 96];
            else if (idx >= 128 && idx <= 191)
                return fmsynth.op_attr0[idx - 128];
            else if (idx >= 192 && idx <= 255)
                return fmsynth.op_attr1[idx - 192];
        } else if (addr >= BASE_SPRATTR && addr < (BASE_SPRATTR + 256 * 4)) {
            // Sprite attributes (32b)
            return video.spriteAttr[(addr / 4) & 255];
        } else if (addr >= BASE_SPRPOS && addr < (BASE_SPRPOS + 256 * 4)) {
            // Sprite positions (32b)
            return video.spritePos[(addr / 4) & 255];
        } else if (addr >= BASE_PALETTE && addr < (BASE_PALETTE + sizeof(video.videoPalette))) {
            // Palette (16b)
            uint16_t val = video.videoPalette[(addr & (sizeof(video.videoPalette) - 1)) / 2];
            return val | (val << 16);
        } else if (addr >= BASE_CHRAM && addr < (BASE_CHRAM + sizeof(video.charRam))) {
            // Character RAM (8b)
            uint8_t val = video.charRam[addr & (sizeof(video.charRam) - 1)];
            return val | (val << 8) | (val << 16) | (val << 24);
        } else if (addr >= BASE_TEXTRAM && addr < (BASE_TEXTRAM + sizeof(video.textRam))) {
            // Text RAM (8b/16/32b)
            return reinterpret_cast<uint32_t *>(video.textRam)[(addr & (sizeof(video.textRam) - 1)) / 4];
        } else if (addr >= BASE_VRAM && addr < (BASE_VRAM + sizeof(video.videoRam))) {
            // Video RAM (8/16/32b)
            return reinterpret_cast<uint32_t *>(video.videoRam)[(addr & (sizeof(video.videoRam) - 1)) / 4];
        } else if (addr >= BASE_VRAM4BIT && addr < (BASE_VRAM4BIT + 2 * sizeof(video.videoRam))) {
            // Video RAM (8/16/32b)
            uint16_t val16 = reinterpret_cast<uint16_t *>(video.videoRam)[(addr & (2 * sizeof(video.videoRam) - 1)) / 4];
            uint32_t val32 = ((val16 & 0xF000) << 12) |
                             ((val16 & 0x0F00) << 8) |
                             ((val16 & 0x00F0) << 4) |
                             (val16 & 0x000F);
            return val32;

        } else if (addr >= BASE_MAINRAM && addr < (BASE_MAINRAM + sizeof(mainRam))) {
            return mainRam[(addr & (sizeof(mainRam) - 1)) / 4];
        }
        return -1;
    }

    void memWrite(uint32_t addr, uint32_t val, uint32_t mask) {
        if (addr == REG_ESPCTRL) {
            UartProtocol::instance()->writeCtrl(val & 0xFF);
        } else if (addr == REG_ESPDATA) {
            if (val & 0x100) {
                UartProtocol::instance()->writeCtrl(0x80);
            } else {
                UartProtocol::instance()->writeData(val & 0xFF);
            }
        } else if (addr == REG_KEYBUF) {
            kbBuf.clear();
        } else if (addr == REG_VCTRL) {
            video.videoCtrl = val & 0xFF;
        } else if (addr == REG_VLINE) {
        } else if (addr == REG_VIRQLINE) {
        } else if (addr == REG_VSCRX1) {
            video.videoScrX1 = val & 0x1FF;
        } else if (addr == REG_VSCRY1) {
            video.videoScrY1 = val & 0xFF;
        } else if (addr == REG_VSCRX2) {
            video.videoScrX2 = val & 0x1FF;
        } else if (addr == REG_VSCRY2) {
            video.videoScrY2   = val & 0xFF;
            video.videoIrqLine = val & 0xFF;
        } else if (addr == REG_MTIME) {
            uint64_t mtime = getMtime();
            setMtime((mtime & ~(uint64_t)0xFFFFFFFFU) | val);
        } else if (addr == REG_MTIMEH) {
            uint64_t mtime = getMtime();
            setMtime((mtime & 0xFFFFFFFFU) | ((uint64_t)val << 32U));
        } else if (addr == REG_MTIMECMP) {
            cpu.mtimecmp = (cpu.mtimecmp & ~(uint64_t)0xFFFFFFFFU) | val;
        } else if (addr == REG_MTIMECMPH) {
            cpu.mtimecmp = (cpu.mtimecmp & 0xFFFFFFFFU) | ((uint64_t)val << 32U);
        } else if (addr == REG_PCM_STATUS) {
            pcm.data.clear();
        } else if (addr == REG_PCM_FIFO_CTRL) {
            pcm.fifoIrqThreshold = val & 0x3FF;
        } else if (addr == REG_PCM_RATE) {
            pcm.rate = val & 0xFFF;
        } else if (addr == REG_PCM_FIFO_DATA) {
            if (pcm.data.size() < 1023)
                pcm.data.push_back(val);
        } else if (addr >= BASE_FMSYNTH && addr < (BASE_FMSYNTH + 256 * 4)) {
            unsigned idx = (addr >> 2) & 255;
            if (idx == 0)
                fmsynth.reg0_ch_4op = val & 0xFFFF;
            else if (idx == 1)
                fmsynth.reg1 = val & 0x00C0;
            else if (idx == 2) {
                fmsynth.reg2_kon = val;
            } else if (idx >= 96 && idx <= 127)
                fmsynth.ch_attr[idx - 96] = val & 0x07FF1FFF;
            else if (idx >= 128 && idx <= 191)
                fmsynth.op_attr0[idx - 128] = val;
            else if (idx >= 192 && idx <= 255)
                fmsynth.op_attr1[idx - 192] = val & 7;
        } else if (addr >= BASE_SPRATTR && addr < (BASE_SPRATTR + 256 * 4)) {
            video.spriteAttr[(addr / 4) & 255] = val & 0x3FFFF;
        } else if (addr >= BASE_SPRPOS && addr < (BASE_SPRPOS + 256 * 4)) {
            video.spritePos[(addr / 4) & 255] = val & 0x00FF01FF;
        } else if (addr >= BASE_PALETTE && addr < (BASE_PALETTE + sizeof(video.videoPalette))) {
            // Palette (16b)
            video.videoPalette[(addr & (sizeof(video.videoPalette) - 1)) / 2] = ((val >> 16) | (val & 0xFFFF)) & 0xFFF;
        } else if (addr >= BASE_CHRAM && addr < (BASE_CHRAM + sizeof(video.charRam))) {
            // Character RAM (8b)
            video.charRam[addr & (sizeof(video.charRam) - 1)] = val & 0xFF;
        } else if (addr >= BASE_TEXTRAM && addr < (BASE_TEXTRAM + sizeof(video.textRam))) {
            // Text RAM (8b/16b)
            auto p = &reinterpret_cast<uint32_t *>(video.textRam)[(addr & (sizeof(video.textRam) - 1)) / 4];
            *p     = (*p & ~mask) | (val & mask);

        } else if (addr >= BASE_VRAM && addr < (BASE_VRAM + sizeof(video.videoRam))) {
            // Video RAM (8/16/32b)
            auto p = &reinterpret_cast<uint32_t *>(video.videoRam)[(addr & (sizeof(video.videoRam) - 1)) / 4];
            *p     = (*p & ~mask) | (val & mask);
        } else if (addr >= BASE_VRAM4BIT && addr < (BASE_VRAM4BIT + 2 * sizeof(video.videoRam))) {
            uint16_t *p     = &reinterpret_cast<uint16_t *>(video.videoRam)[(addr & (2 * sizeof(video.videoRam) - 1)) / 4];
            uint16_t  val16 = *p;
            uint32_t  val32 =
                ((val16 & 0xF000) << 12) |
                ((val16 & 0x0F00) << 8) |
                ((val16 & 0x00F0) << 4) |
                (val16 & 0x000F);

            uint32_t msk =
                ((mask & 0xFF000000) >> 8) |
                ((mask & 0x00FF0000) << 8) |
                ((mask & 0x0000FF00) >> 8) |
                ((mask & 0x000000FF) << 8);

            val32 = (val32 & ~msk) | (val & msk);
            val16 =
                ((val32 & 0x0F000000) >> 12) |
                ((val32 & 0x000F0000) >> 8) |
                ((val32 & 0x00000F00) >> 4) |
                (val32 & 0x0000000F);
            *p = val16;

        } else if (addr >= BASE_MAINRAM && addr < (BASE_MAINRAM + sizeof(mainRam))) {
            auto p = &mainRam[(addr & (sizeof(mainRam) - 1)) / 4];
            *p     = (*p & ~mask) | (val & mask);
        }
    }

    bool emulateStep(void) {
        bool end_of_frame = false;
        if (emuMode == Em_Halted)
            return true;

        if (curLineStepsRemaining <= 0) {
            video.videoLine++;
            curLineStepsRemaining = 10000000 / 60 / 262;

            if (video.isOnStartOfVBlank())
                cpu.pendInterrupt(1 << 16);
            if (video.isOnVideoIrqLine())
                cpu.pendInterrupt(1 << 17);

            if (video.videoLine == 262) {
                video.videoLine = 0;
                end_of_frame    = true;
            }

            video.drawLine(video.videoLine);
            keyboardTypeIn();

            if (pcm.hasIrq())
                cpu.pendInterrupt(1 << 18);
            if (!kbBuf.empty())
                cpu.pendInterrupt(1 << 19);
            if (UartProtocol::instance()->readCtrl() & 1)
                cpu.pendInterrupt(1 << 20);

            cpu.mtime = getMtime();
            if (cpu.mtime >= cpu.mtimecmp)
                cpu.pendInterrupt(1 << 7);
        }

        cpu.emulate();
        curLineStepsRemaining--;

        if (enableDebugger && enableBreakpoints) {
            for (auto &bp : breakpoints) {
                if (bp.enabled && bp.addr == cpu.pc) {
                    emuMode = Em_Halted;
#ifdef GDB_ENABLE
                    gdbBreakpointHit();
#endif
                    break;
                }
            }
        }
        if (emuMode == Em_Step)
            emuMode = Em_Halted;

        return end_of_frame;
    }

    void emulateFrame(int16_t *audioBuf, unsigned numSamples) override {
        std::lock_guard lock(mutex);

        if (emuMode == Em_Halted) {
            for (int line = 0; line < 262; line++)
                video.drawLine(line);
            return;
        }
        while (!emulateStep()) {
        }

        if (audioBuf != nullptr) {
            memset(audioBuf, 0, numSamples * sizeof(*audioBuf) * 2);

            for (unsigned i = 0; i < numSamples; i++) {
                int16_t fmResults[2];
                fmsynth.render(fmResults);

                int16_t pcmResults[2];
                pcm.render(pcmResults);

                audioBuf[0] = std::min(std::max(-32768, fmResults[0] + pcmResults[0]), 32767);
                audioBuf[1] = std::min(std::max(-32768, fmResults[1] + pcmResults[1]), 32767);
                audioBuf += 2;
            }
        }
    }

    void keyboardTypeIn() {
        if (kbBuf.size() < kbBufSize && !typeInStr.empty()) {
            char ch = typeInStr.front();
            typeInStr.erase(typeInStr.begin());
            Keyboard::instance()->pressKey(ch);
        }
    }

    void dbgMenu() override {
        std::lock_guard lock(mutex);
        if (!enableDebugger)
            return;

        ImGui::MenuItem("CPU state", "", &showCpuState);
        ImGui::MenuItem("Breakpoints", "", &showBreakpoints);
        ImGui::MenuItem("Memory editor", "", &showMemEdit);
        ImGui::MenuItem("IO Registers", "", &showIoRegsWindow);
    }

    void dbgWindows() override {
        std::lock_guard lock(mutex);
        if (!enableDebugger)
            return;

        if (showCpuState)
            dbgWndCpuState(&showCpuState);
        if (showBreakpoints)
            dbgWndBreakpoints(&showBreakpoints);

        if (showMemEdit)
            dbgWndMemEdit(&showMemEdit);
        if (showIoRegsWindow)
            dbgWndIoRegs(&showIoRegsWindow);
    }

    void dbgWndIoRegs(bool *p_open) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(330, FLT_MAX));
        if (ImGui::Begin("IO Registers", p_open, 0)) {
            if (ImGui::CollapsingHeader("Video")) {
                video.dbgDrawIoRegs();
            }
            if (ImGui::CollapsingHeader("Audio")) {
                fmsynth.dbgDrawIoRegs();
            }
            if (ImGui::CollapsingHeader("Sprites")) {
                video.dbgDrawSpriteRegs();
            }
            if (ImGui::CollapsingHeader("Palette")) {
                video.dbgDrawPaletteRegs();
            }
        }
        ImGui::End();
    }

    void dbgWndCpuState(bool *p_open) {
        bool open = ImGui::Begin("CPU state", p_open, ImGuiWindowFlags_AlwaysAutoResize);
        if (open) {
            ImGui::PushStyleColor(ImGuiCol_Button, emuMode == Em_Halted ? (ImVec4)ImColor(192, 0, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
            ImGui::BeginDisabled(emuMode != Em_Running);
            if (ImGui::Button("Halt")) {
                emuMode = Em_Halted;
            }
            ImGui::EndDisabled();
            ImGui::PopStyleColor();

            ImGui::BeginDisabled(emuMode == Em_Running);
            ImGui::SameLine();
            if (ImGui::Button("Step Into")) {
                emuMode = Em_Step;
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, emuMode == Em_Running ? (ImVec4)ImColor(0, 128, 0) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button("Go")) {
                emuMode = Em_Running;
            }
            ImGui::PopStyleColor();
            ImGui::EndDisabled();

            ImGui::Separator();

            {
                auto data = memRead(cpu.pc, false);
                auto str  = instrToString((uint32_t)data, cpu.pc);
                ImGui::Text("%08X %-30s", (unsigned)data, str.c_str());
            }

            ImGui::Separator();

            auto drawReg = [&](const std::string &name, uint32_t val) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%-3s", name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("0x%08X", val);
            };

            if (ImGui::BeginTable("RegTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Reg", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                static const char *regs[] = {
                    "x0/zero", "x1/ra", "x2/sp", "x3/gp",
                    "x4/tp", "x5/t0", "x6/t1", "x7/t2",
                    "x8/s0/fp", "x9/s1", "x10/a0", "x11/a1",
                    "x12/a2", "x13/a3", "x14/a4", "x15/a5",
                    "x16/a6", "x17/a7", "x18/s2", "x19/s3",
                    "x20/s4", "x21/s5", "x22/s6", "x23/s7",
                    "x24/s8", "x25/s9", "x26/s10", "x27/s11",
                    "x28/t3", "x29/t4", "x30/t5", "x31/t6"};
                drawReg("pc", cpu.pc);

                for (int i = 1; i < 32; i++) {
                    drawReg(regs[i], cpu.regs[i]);
                }

                uint32_t mstatus = 0;
                {
                    if (cpu.mstatus_mie)
                        mstatus |= (1 << 3);
                    if (cpu.mstatus_mpie)
                        mstatus |= (1 << 7);
                }

                drawReg("mstatus", mstatus);
                drawReg("mie", cpu.mie);
                drawReg("mtvec", cpu.mtvec);
                drawReg("mscratch", cpu.mscratch);
                drawReg("mepc", cpu.mepc);
                drawReg("mcause", cpu.mcause);
                drawReg("mtval", cpu.mtval);
                drawReg("mip", cpu.mip);

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void dbgWndBreakpoints(bool *p_open) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(330, 132), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("Breakpoints", p_open, 0)) {
            ImGui::Checkbox("Enable breakpoints", &enableBreakpoints);
            ImGui::SameLine(ImGui::GetWindowWidth() - 25);
            if (ImGui::Button("+")) {
                breakpoints.emplace_back();
            }
            ImGui::Separator();
            if (ImGui::BeginTable("Table", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
                ImGui::TableSetupColumn("En", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Symbol", 0);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                clipper.Begin((int)breakpoints.size());
                int eraseIdx = -1;

                while (clipper.Step()) {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                        auto &bp = breakpoints[row_n];

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Checkbox(fmtstr("##en%d", row_n).c_str(), &bp.enabled);
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x * 10);
                        ImGui::InputScalar(fmtstr("##val%d", row_n).c_str(), ImGuiDataType_U32, &bp.addr, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite);
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(-1);
                        if (ImGui::BeginCombo(fmtstr("##name%d", row_n).c_str(), bp.name.c_str())) {
                            ImGui::EndCombo();
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(fmtstr("X##del%d", row_n).c_str())) {
                            eraseIdx = row_n;
                        }
                    }
                }
                if (eraseIdx >= 0) {
                    breakpoints.erase(breakpoints.begin() + eraseIdx);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void dbgWndMemEdit(bool *p_open) {
        if (memAreas.empty()) {
            memAreas.emplace_back("Memory", nullptr, 0x100000);
            memAreas.emplace_back("Text RAM", video.textRam, sizeof(video.textRam));
        }

        if (memEditMemSelect < 0 || memEditMemSelect > (int)memAreas.size()) {
            // Invalid setting, reset to 0
            memEditMemSelect = 0;
        }

        MemoryEditor::Sizes s;
        memEdit.calcSizes(s, memAreas[memEditMemSelect].size, 0);
        ImGui::SetNextWindowSize(ImVec2(s.windowWidth, s.windowWidth * 0.60f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(s.windowWidth, 150.0f), ImVec2(s.windowWidth, FLT_MAX));

        if (ImGui::Begin("Memory editor", p_open, ImGuiWindowFlags_NoScrollbar)) {
            if (ImGui::BeginCombo("Memory select", memAreas[memEditMemSelect].name.c_str(), ImGuiComboFlags_HeightLargest)) {
                for (int i = 0; i < (int)memAreas.size(); i++) {
                    if (ImGui::Selectable(memAreas[i].name.c_str(), memEditMemSelect == i)) {
                        memEditMemSelect = i;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();

            if (memEditMemSelect == 0) {
                memEdit.readFn = [this](const ImU8 *data, size_t off) {
                    auto val = memRead((uint32_t)off, false);
                    if (val < 0)
                        return -1;

                    return (int)(uint8_t)((val >> ((off & 3) * 8)) & 0xFF);
                };
                memEdit.writeFn = [this](ImU8 *data, size_t off, ImU8 d) {
                    memWrite((uint32_t)off, d | (d << 8) | (d << 16) | (d << 24), 0xFF << (off & 3) * 8);

                    // memWrite((uint16_t)off, d);
                };
            } else {
                memEdit.readFn  = nullptr;
                memEdit.writeFn = nullptr;
            }
            memEdit.drawContents(memAreas[memEditMemSelect].data, memAreas[memEditMemSelect].size, 0);
            if (memEdit.contentsWidthChanged) {
                memEdit.calcSizes(s, memAreas[memEditMemSelect].size, 0);
                ImGui::SetWindowSize(ImVec2(s.windowWidth, ImGui::GetWindowSize().y));
            }
        }
        ImGui::End();
    }

#ifdef GDB_ENABLE
    int         conn = -1;
    std::string rxStr;
    int         rxState = 0;
    char        rxChecksum[3];

    void gdbHandleConnection() {
        {
            std::lock_guard lock(mutex);
            reset();
            emuMode = Em_Halted;
            breakpoints.clear();
        }
        while (1) {
            uint8_t buf[16384];
            int     sz = recv(conn, buf, sizeof(buf), 0);
            if (sz <= 0) {
                close(conn);
                conn = -1;
                return;
            }
            gdbParseBuf(buf, sz);
        }
    }

    void gdbThreadFunc() {
        int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket < 0)
            return;

        const int enable = 1;
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

        fcntl(listenSocket, F_SETFL, fcntl(listenSocket, F_GETFL, 0) | O_NONBLOCK);

        sockaddr_in serverAddress;
        serverAddress.sin_family      = AF_INET;
        serverAddress.sin_port        = htons(2331);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        listen(listenSocket, 5);

        while (!stopping) {
            conn = accept(listenSocket, nullptr, nullptr);
            if (conn < 0)
                continue;

            int val = 1;
            setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));

            gdbHandleConnection();

            // fcntl(conn, F_SETFL, fcntl(conn, F_GETFL, 0) | O_NONBLOCK);
            // dprintf("Got connection!\n");
        }
    }

    void gdbParseBuf(const uint8_t *p, size_t len) {
        while (len--) {
            uint8_t ch = *(p++);

            if (ch == 3) {
                // CTRL-C
                // dprintf("Halting\n");
                {
                    std::lock_guard lock(mutex);
                    emuMode = Em_Halted;
                }
                send(conn, "+", 1, 0);
                // last signal
                gdbSendStopReply();

            } else if (ch == '$') {
                rxState = 1;
                rxStr.clear();
            } else if (rxState == 1) {
                if (ch == '#') {
                    rxState = 2;
                } else {
                    rxStr += ch;
                }
            } else if (rxState == 2) {
                rxChecksum[0] = ch;
                rxState       = 3;
            } else if (rxState == 3) {
                rxChecksum[1] = ch;
                rxChecksum[2] = 0;

                uint8_t checksum = strtoul(rxChecksum, nullptr, 16);

                uint8_t checksum2 = 0;
                for (uint8_t ch : rxStr) {
                    checksum2 += ch;
                }
                if (checksum2 == checksum) {
                    // dprintf("> '%s'\n", rxStr.c_str());
                    // dprintf("< +\n");
                    send(conn, "+", 1, 0);
                    gdbReceivedCmd(rxStr);
                } else {
                    // dprintf("< -\n");
                    send(conn, "-", 1, 0);
                }

                rxState = 0;
                rxStr.clear();
            }
        }
    }

    void gdbBreakpointHit() {
        if (conn >= 0)
            gdbSendStopReply();
    }

    std::string gdbReadMemory(uintptr_t addr, size_t size) {
        // dprintf("Read memory %08lX (size:%08lX)\n", addr, size);
        std::lock_guard      lock(mutex);
        std::vector<uint8_t> respData;

        while (size) {
            auto data = memRead(addr, false);
            if (data < 0)
                break;

            if ((addr & 3) == 0) {
                respData.push_back((data >> 0) & 0xFF);
                if (size >= 2)
                    respData.push_back((data >> 8) & 0xFF);
                if (size >= 3)
                    respData.push_back((data >> 16) & 0xFF);
                if (size >= 4)
                    respData.push_back((data >> 24) & 0xFF);

                if (size <= 4)
                    break;

                size -= 4;
                addr += 4;

            } else if ((addr & 3) == 1) {
                respData.push_back((data >> 8) & 0xFF);
                size--;
                addr++;
            } else if ((addr & 3) == 2) {
                respData.push_back((data >> 16) & 0xFF);
                size--;
                addr++;
            } else if ((addr & 3) == 3) {
                respData.push_back((data >> 24) & 0xFF);
                size--;
                addr++;
            }
        }
        return bufToHex(respData.data(), respData.size());
    }

    void gdbWriteMemory(uintptr_t addr, std::vector<uint8_t> data) {
        std::lock_guard lock(mutex);

        if (addr >= BASE_MAINRAM && addr < (BASE_MAINRAM + sizeof(mainRam))) {
            addr -= BASE_MAINRAM;

            unsigned endAddr = addr + data.size();
            if (endAddr > BASE_MAINRAM + sizeof(mainRam))
                endAddr = BASE_MAINRAM + sizeof(mainRam);

            memcpy(((uint8_t *)mainRam) + addr, data.data(), endAddr - addr);

        } else {
            printf("Write memory %08lX: ", addr);
            for (auto val : data) {
                printf("%02X ", val);
            }
            printf("\n");
            // exit(1);
        }
    }

    void gdbReceivedCmd(const std::string &cmd) {
        if (cmd.empty())
            return;

        if (cmd == "?") {
            // last signal
            gdbSendStopReply();

        } else if (cmd == "qAttached") {
            gdbSendResponse("1");
        } else if (cmd == "qTStatus") {
            gdbSendResponse("T0");

            // } else if (cmd == "vCont?") {
            //     gdbSendResponse("vCont;c;s;t");

        } else if (cmd == "Hg0") {
            gdbSendResponse("OK");
        } else if (cmd == "Hc-1") {
            gdbSendResponse("OK");
        } else if (cmd == "qfThreadInfo") {
            gdbSendResponse("m00");
        } else if (cmd == "qsThreadInfo") {
            gdbSendResponse("l");
        } else if (cmd == "qC") {
            gdbSendResponse("QC0000");
        } else if (cmd == "qOffsets") {
            gdbSendResponse("Text=00;Data=00;Bss=00");
        } else if (cmd == "qSymbol") {
            gdbSendResponse("OK");

        } else if (cmd == "g") {
            // read registers
            uint32_t regs[33];
            {
                std::lock_guard lock(mutex);
                for (int i = 0; i < 32; i++) {
                    regs[i] = cpu.regs[i];
                }
                regs[32] = cpu.pc;
            }
            gdbSendResponse(bufToHex(regs, sizeof(regs)));

        } else if (cmd[0] == 'G') {
            auto regBuf = decodeHex(&cmd[1], (cmd.size() - 1) / 2);

            uint32_t *regVals = reinterpret_cast<uint32_t *>(regBuf.data());
            {
                std::lock_guard lock(mutex);
                for (unsigned i = 0; i < regBuf.size() / 4; i++) {
                    printf("P: Reg %u=%08x\n", i, regVals[i]);

                    if (i > 0 && i < 32) {
                        cpu.regs[i] = regVals[i];
                    } else if (i == 32) {
                        cpu.pc = regVals[i];
                    }
                }
            }

        } else if (cmd[0] == 'P' && cmd.size() == 12 && cmd[3] == '=') {
            // Pxx=xxxxxxxx
            auto regBuf   = decodeHex(&cmd[1], 1);
            auto valueBuf = decodeHex(&cmd[4], 4);
            if (regBuf.empty() || valueBuf.empty())
                return;

            unsigned regIdx = regBuf[0];
            uint32_t val    = *reinterpret_cast<uint32_t *>(valueBuf.data());
            // printf("P: Reg %u=%08x\n", regIdx, val);

            {
                std::lock_guard lock(mutex);
                if (regIdx > 0 && regIdx < 32) {
                    cpu.regs[regIdx] = val;
                } else if (regIdx == 32) {
                    cpu.pc = val;
                }
            }
            gdbSendResponse("OK");

        } else if (cmd[0] == 'm') {
            char    *endptr;
            unsigned addr = strtoul(cmd.c_str() + 1, &endptr, 16);
            if (endptr[0] == ',') {
                unsigned size = strtoul(endptr + 1, &endptr, 16);
                gdbSendResponse(gdbReadMemory(addr, size));
            }

        } else if (cmd[0] == 'M') {
            char    *endptr;
            unsigned addr = strtoul(cmd.c_str() + 1, &endptr, 16);
            if (endptr[0] == ',') {
                unsigned size = strtoul(endptr + 1, &endptr, 16);

                if (endptr[0] == ':') {
                    auto data = decodeHex(endptr + 1, size);
                    if (data.empty())
                        return;
                    gdbWriteMemory(addr, data);
                    gdbSendResponse("OK");
                }
            }

            // } else if (cmd[0] == 'X') {

        } else if (cmd[0] == 'H') {
            gdbSendResponse("OK");

        } else if (cmd.substr(0, 3) == "Z0," || cmd.substr(0, 3) == "Z1,") {
            // Add breakpoint
            uint32_t addr = strtoul(cmd.c_str() + 3, nullptr, 16);
            // dprintf("Add breakpoint %08X\n", addr);

            {
                std::lock_guard lock(mutex);
                enableBreakpoints = true;
                for (auto &bp : breakpoints) {
                    if (bp.addr == addr) {
                        bp.enabled = true;
                        gdbSendResponse("OK");
                        return;
                    }
                }

                Breakpoint bp;
                bp.addr    = addr;
                bp.enabled = true;
                breakpoints.push_back(bp);
            }
            gdbSendResponse("OK");

        } else if (cmd.substr(0, 3) == "z0," || cmd.substr(0, 3) == "z1,") {
            // Remove breakpoint
            uint32_t addr = strtoul(cmd.c_str() + 3, nullptr, 16);
            // dprintf("Remove breakpoint %08X\n", addr);

            {
                std::lock_guard lock(mutex);
                auto            it = breakpoints.begin();
                while (it != breakpoints.end()) {
                    if (it->addr == addr) {
                        it = breakpoints.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            gdbSendResponse("OK");

        } else if (cmd == "c") {
            // dprintf("Continue\n");
            std::lock_guard lock(mutex);
            emuMode = Em_Running;

        } else if (cmd == "s") {
            // Step
            {
                std::lock_guard lock(mutex);
                emuMode = Em_Step;
                emulateStep();
            }
            gdbSendStopReply();

        } else if (cmd.substr(0, 11) == "qSupported:") {
            gdbSendResponse("PacketSize=4000");

        } else {
            printf("Unhandled GDB cmd=%s\n", cmd.c_str());

            // Unsupported command
            gdbSendResponse("");
        }
    }

    void gdbSendResponse(const std::string &resp) {
        uint8_t checksum = 0;
        for (uint8_t ch : resp) {
            checksum += ch;
        }

        char csStr[3];
        snprintf(csStr, sizeof(csStr), "%02X", checksum);

        std::string str = "$" + resp + "#" + csStr;
        send(conn, str.c_str(), str.length(), 0);

        // dprintf("< %s\n", str.c_str());
    }

    void gdbSendStopReply(uint8_t signal = 5) {
        std::string resp = fmtstr("S%02X", signal);
        gdbSendResponse(resp);
    }
#endif
};

std::shared_ptr<EmuState> newAq32EmuState() {
    return std::make_shared<Aq32EmuState>();
}
