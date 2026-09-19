#include "EmuState.h"
#include "DCBlock.h"
#include "UartProtocol.h"
#include "FPGA.h"
#include "../aq32/cpu/riscv.h"
#include "Config.h"
#include "bootrom.h"
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

#define BASE_BOOTROM       0x00000
#define REG_ESPCTRL        0x02000
#define REG_ESPDATA        0x02004
#define REG_KEYBUF         0x02010
#define REG_HCTRL          0x02014
#define REG_KEYS_L         0x02018
#define REG_KEYS_H         0x0201C
#define REG_GAMEPAD1_L     0x02018
#define REG_GAMEPAD1_H     0x0201C
#define REG_GAMEPAD2_L     0x02018
#define REG_GAMEPAD2_H     0x0201C
#define BASE_PALETTE       0x20000
#define BASE_REG_POSX      0x20020
#define BASE_REG_POSY      0x20024
#define BASE_REG_COLOR     0x20028
#define BASE_REG_REMAP_T   0x2002C
#define BASE_REG_REMAP     0x20030
#define BASE_REG_CLIP_RECT 0x20040
#define BASE_REG_FLAGS     0x20044
#define BASE_REG_WR1BPP    0x20048
#define BASE_REG_WR4BPP    0x2004C
#define BASE_REG_PAGE      0x20050
#define BASE_VRAM          0x28000
#define BASE_VRAM4BIT      0x30000
#define BASE_MAINRAM       0x80000

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

class Aqua8EmuState : public EmuState {
public:
    riscv                cpu;
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

    struct {
        uint16_t palette[16]; // Video palette
        int16_t  posx;
        int16_t  posy;
        uint8_t  color;
        uint16_t remap_t;
        uint8_t  remap[16];
        uint8_t  clip_x1;
        uint8_t  clip_x2;
        uint8_t  clip_y1;
        uint8_t  clip_y2;
        uint8_t  flags;
        uint8_t  page;
        alignas(4) uint8_t vram[32 * 1024]; // Video RAM
    } video;

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

    Aqua8EmuState() {
        coreType         = 2;
        coreFlags        = 0x02;
        coreVersionMajor = 0;
        coreVersionMinor = 1;
        memcpy(coreName, "aqua-8          ", sizeof(coreName));

        cpu.dataWrite8  = [this](uint32_t addr, uint8_t val) { _memWrite8(addr, val); };
        cpu.dataWrite16 = [this](uint32_t addr, uint16_t val) { _memWrite16(addr, val); };
        cpu.dataWrite32 = [this](uint32_t addr, uint32_t val) { _memWrite32(addr, val); };
        cpu.dataRead8   = [this](uint32_t addr) { return _memRead8(addr); };
        cpu.dataRead16  = [this](uint32_t addr) { return _memRead16(addr); };
        cpu.dataRead32  = [this](uint32_t addr) { return _memRead32(addr); };
        cpu.instrRead   = [this](uint32_t addr) { return (uint32_t)memRead(addr); };

        memset(&keybMatrix, 0xFF, sizeof(keybMatrix));
        memcpy(bootRom, bootrom_bin, bootrom_bin_len);

        memset(&video, 0, sizeof(video));
        video.clip_x1 = 0;
        video.clip_x2 = 200;
        video.clip_y1 = 0;
        video.clip_y2 = 163;

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

    virtual ~Aqua8EmuState() {
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
    }

    void loadConfig() {
        auto root   = Config::instance()->loadConfigFile("aqua-8.json");
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

        Config::instance()->saveConfigFile("aqua-8.json", root);
    }

    void getVideoSize(int &w, int &h) override {
        std::lock_guard lock(mutex);
        w = 640;
        h = 480;
    }

    void getPixels(void *pixels, int pitch) override {
        std::lock_guard lock(mutex);
        memset(pixels, 0, 480 * pitch);

        for (int y = 0; y < 160; y++) {
            for (int x = 0; x < 200; x++) {
                uint32_t color = col12_to_col32(video.palette[(video.vram[y * 100 + x / 2] >> ((x & 1) * 4)) & 0xF]);

                for (int j = 0; j < 3; j++) {
                    for (int i = 0; i < 3; i++) {
                        ((uint32_t *)((uintptr_t)pixels + (y * 3 + j) * pitch))[20 + x * 3 + i] = color;
                    }
                }
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

    void setPixel(int x, int y, unsigned color) {
        if (video.remap_t & (1 << color))
            return;
        color = video.remap[color & 0xF] & 0xF;
        if (x >= 200 || x < video.clip_x1 || x >= video.clip_x2)
            return;
        if (y >= 163 || y < video.clip_y1 || y >= video.clip_y2)
            return;

        unsigned loc = y * 200 + x;
        uint8_t *p   = &video.vram[loc / 2];

        if (loc & 1) {
            *p = (*p & 0xF) | (color << 4);
        } else {
            *p = (*p & 0xF0) | color;
        }
    }

    void writeDone() {
        if (video.flags & 1)
            video.posy++;
        if (video.flags & 2)
            video.posx += 8;
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
        } else if (addr >= BASE_PALETTE && addr < (BASE_PALETTE + sizeof(video.palette))) {
            // Palette (16b)
            uint16_t val = video.palette[(addr & (sizeof(video.palette) - 1)) / 2];
            return val | (val << 16);
        } else if ((addr & ~3) == BASE_REG_POSX) {
            return video.posx << 16;
        } else if ((addr & ~3) == BASE_REG_POSY) {
            return video.posy << 16;
        } else if (addr == BASE_REG_COLOR) {
            return video.color;
        } else if (addr == BASE_REG_REMAP_T) {
            return video.remap_t;
        } else if (addr >= BASE_REG_REMAP && addr < (BASE_REG_REMAP + sizeof(video.remap))) {
            uint8_t val = video.remap[addr & (sizeof(video.remap) - 1)];
            return val | (val << 16);
        } else if ((addr & ~3) == BASE_REG_CLIP_RECT) {
            return (video.clip_y2 << 24) | (video.clip_y1 << 16) | (video.clip_x2 << 8) | (video.clip_x1 << 0);
        } else if (addr == BASE_REG_FLAGS) {
            return video.flags;
        } else if (addr == BASE_REG_WR1BPP) {
            return 0;
        } else if (addr == BASE_REG_WR4BPP) {
            return 0;
        } else if (addr == BASE_REG_PAGE) {
            return video.page;
        } else if (addr >= BASE_VRAM && addr < (BASE_VRAM + sizeof(video.vram))) {
            // Video RAM (8/16/32b)
            return reinterpret_cast<uint32_t *>(video.vram)[(addr & (sizeof(video.vram) - 1)) / 4];
        } else if (addr >= BASE_VRAM4BIT && addr < (BASE_VRAM4BIT + 2 * sizeof(video.vram))) {
            // Video RAM (8/16/32b)
            uint16_t val16 = reinterpret_cast<uint16_t *>(video.vram)[(addr & (2 * sizeof(video.vram) - 1)) / 4];
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
        } else if (addr >= BASE_PALETTE && addr < (BASE_PALETTE + sizeof(video.palette))) {
            // Palette (16b)
            unsigned idx       = (addr & (sizeof(video.palette) - 1)) / 2;
            video.palette[idx] = ((val >> 16) | (val & 0xFFFF)) & 0xFFF;
        } else if ((addr & ~3) == BASE_REG_POSX) {
            video.posx = val >> 16;
        } else if ((addr & ~3) == BASE_REG_POSY) {
            video.posy = val >> 16;
        } else if (addr == BASE_REG_COLOR) {
            video.color = val & 0xF;
        } else if (addr == BASE_REG_REMAP_T) {
            video.remap_t = val & 0xFFFF;
        } else if (addr >= BASE_REG_REMAP && addr < (BASE_REG_REMAP + sizeof(video.remap))) {
            unsigned idx     = addr & (sizeof(video.remap) - 1);
            video.remap[idx] = val & 0xF;
        } else if ((addr & ~3) == BASE_REG_CLIP_RECT) {
            if (mask & 0x000000FF)
                video.clip_x1 = (val >> 0) & 0xFF;
            if (mask & 0x0000FF00)
                video.clip_x2 = (val >> 8) & 0xFF;
            if (mask & 0x00FF0000)
                video.clip_y1 = (val >> 16) & 0xFF;
            if (mask & 0xFF000000)
                video.clip_y2 = (val >> 24) & 0xFF;
        } else if (addr == BASE_REG_FLAGS) {
            video.flags = val;
        } else if (addr == BASE_REG_WR1BPP) {
            for (int i = 0; i < 8; i++) {
                if (val & (1 << i))
                    setPixel(video.posx + i, video.posy, video.color);
            }
            writeDone();

        } else if (addr == BASE_REG_WR4BPP) {
            for (int i = 0; i < 8; i++) {
                setPixel(video.posx + i, video.posy, (val >> (i * 4)) & 0xF);
            }
            writeDone();

        } else if (addr == BASE_REG_PAGE) {
            video.page = val;
        } else if (addr >= BASE_VRAM && addr < (BASE_VRAM + sizeof(video.vram))) {
            // Video RAM (8/16/32b)
            auto p = &reinterpret_cast<uint32_t *>(video.vram)[(addr & (sizeof(video.vram) - 1)) / 4];
            *p     = (*p & ~mask) | (val & mask);
        } else if (addr >= BASE_VRAM4BIT && addr < (BASE_VRAM4BIT + 2 * sizeof(video.vram))) {
            uint16_t *p     = &reinterpret_cast<uint16_t *>(video.vram)[(addr & (2 * sizeof(video.vram) - 1)) / 4];
            uint16_t  val16 = *p;
            uint32_t  val32 =
                ((val16 & 0xF000) << 12) |
                ((val16 & 0x0F00) << 8) |
                ((val16 & 0x00F0) << 4) |
                (val16 & 0x000F);

            val32 = (val32 & ~mask) | (val & mask);
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

    void emulateFrame(int16_t *audioBuf, unsigned numSamples) override {
        std::lock_guard lock(mutex);

        if (emuMode != Em_Halted) {
            unsigned stepsPerFrame = 10000000 / 60;
            while (emuMode != Em_Halted && stepsPerFrame--) {
                if (!kbBuf.empty())
                    cpu.pendInterrupt(1 << 19);
                if (UartProtocol::instance()->readCtrl() & 1)
                    cpu.pendInterrupt(1 << 20);

                cpu.emulate();

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
            }

            cpu.pendInterrupt(1 << 16);
            keyboardTypeIn();
        }

        if (audioBuf != nullptr) {
            memset(audioBuf, 0, numSamples * sizeof(*audioBuf) * 2);
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
            }
            if (ImGui::CollapsingHeader("Palette")) {
                if (ImGui::BeginTable("Table", 8, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter)) {
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Pal", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Idx", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("G", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Color");
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    ImGuiListClipper clipper;
                    clipper.Begin(128);
                    while (clipper.Step()) {
                        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                            int r = (video.palette[row_n] >> 8) & 0xF;
                            int g = (video.palette[row_n] >> 4) & 0xF;
                            int b = (video.palette[row_n] >> 0) & 0xF;

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%2d", row_n);
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", row_n / 16);
                            ImGui::TableNextColumn();
                            ImGui::Text("%2d", row_n & 15);
                            ImGui::TableNextColumn();
                            ImGui::Text("%03X", video.palette[row_n]);
                            ImGui::TableNextColumn();
                            ImGui::Text("%2d", r);
                            ImGui::TableNextColumn();
                            ImGui::Text("%2d", g);
                            ImGui::TableNextColumn();
                            ImGui::Text("%2d", b);
                            ImGui::TableNextColumn();
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32((ImVec4)ImColor((r << 4) | r, (g << 4) | g, (b << 4) | b)));
                        }
                    }
                    ImGui::EndTable();
                }
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
                cpu.emulate();
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

std::shared_ptr<EmuState> newAqua8EmuState() {
    return std::make_shared<Aqua8EmuState>();
}
