#include "EmuState.h"
#include "AqpVideo.h"
#include "DCBlock.h"
#include "FPGA.h"
#include "UartProtocol.h"
#include "fpgarom.h"
#include "FpgaCore.h"
#include "Keyboard.h"
#include "imgui.h"
#include "tinyfiledialogs.h"
#include "DisplayOverlay/DisplayOverlay.h"
#include "Z80Core.h"
#include "AY8910.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "MemoryEditor.h"

// 3579545 Hz -> 59659 cycles / frame
// 7159090 Hz -> 119318 cycles / frame

// 455x262=119210 -> 60.05 Hz
// 51.2us + 1.5us + 4.7us + 6.2us = 63.6 us
// 366 active pixels

#define HCYCLES_PER_LINE   (455)
#define HCYCLES_PER_SAMPLE (162)

class AqpEmuState : public EmuState {
public:
    Z80Core             z80Core;
    AqpVideo            video;
    uint8_t             keybMatrix[8] = {0};
    std::deque<uint8_t> kbBuf;
    const unsigned      kbBufSize         = 16;
    bool                cartridgeInserted = false;
    uint8_t             videoMode         = 0;
    DCBlock             dcBlockLeft;
    DCBlock             dcBlockRight;
    std::string         typeInStr;
    std::mutex          mutexTypeInStr;
    uint8_t             mainRam[512 * 1024];
    uint8_t             cartRom[16 * 1024];

    // Debugging
    bool showMemEdit      = false;
    int  memEditMemSelect = 0;
    bool showIoRegsWindow = false;
    bool hasSaved         = false;

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

    // IO space
    uint8_t audioDAC    = 0;               // $EC   : Audio DAC sample
    uint8_t irqMask     = 0;               // $EE   : Interrupt mask register
    uint8_t irqStatus   = 0;               // $EF   : Interrupt status register
    uint8_t bankRegs[4] = {0};             // $F0-F3: Banking registers
    AY8910  ay1;                           // $F6/F7: AY-3-8910 emulation state
    AY8910  ay2;                           // $F8/F9: AY-3-8910 emulation state
    bool    sysCtrlDisableExt     = false; // $FB<0>: Disable access to extended registers
    bool    sysCtrlAyDisable      = false; // $FB<1>: Disable AY PSGs
    bool    sysCtrlTurbo          = false; // $FB<2>: Turbo mode
    bool    sysCtrlTurboUnlimited = false; // $FB<3>: Turbo unlimited mode
    bool    sysCtrlWarmBoot       = false; // $FB<7>: R0:Cold boot, R1:Warm boot
    bool    soundOutput           = false; // $FC<1>: Cassette/Sound output
    bool    cpmRemap              = false; // $FD<1>: Remap memory for CP/M
    bool    forceTurbo            = false;

    AqpEmuState() {
        coreType         = 1;
        coreFlags        = 0x1E;
        coreVersionMajor = 1;
        coreVersionMinor = 0;
        memcpy(coreName, "Aquarius+       ", sizeof(coreName));

        z80Core.hasIrq        = [this] { return (irqStatus & irqMask) != 0; };
        z80Core.memRead       = [this](uint16_t addr) { return memRead(addr); };
        z80Core.memWrite      = [this](uint16_t addr, uint8_t data) { memWrite(addr, data); };
        z80Core.ioRead        = [this](uint16_t addr) { return ioRead(addr); };
        z80Core.ioWrite       = [this](uint16_t addr, uint8_t data) { ioWrite(addr, data); };
        z80Core.showInMemEdit = [this](uint16_t addr) {
            showMemEdit      = true;
            memEditMemSelect = 0;
            memEdit.gotoAddr = addr;
        };

        memset(keybMatrix, 0xFF, sizeof(keybMatrix));
        for (unsigned i = 0; i < sizeof(mainRam); i++)
            mainRam[i] = rand();

        loadConfig();
        reset(true);
    }

    void unloading() override {
        saveConfig();
    }

    virtual ~AqpEmuState() {
        unloading();
    }

    void reset(bool cold) override {
        // Reset registers
        audioDAC              = 0;
        irqMask               = 0;
        irqStatus             = 0;
        bankRegs[0]           = 0xC0 | 0;
        bankRegs[1]           = 33;
        bankRegs[2]           = 34;
        bankRegs[3]           = 19;
        sysCtrlDisableExt     = false;
        sysCtrlAyDisable      = false;
        sysCtrlTurbo          = false;
        sysCtrlTurboUnlimited = false;
        soundOutput           = false;
        cpmRemap              = false;
        sysCtrlWarmBoot       = !cold;

        z80Core.reset();
        ay1.reset();
        ay2.reset();
        kbBuf.clear();
    }

    void loadConfig() {
        auto root        = Config::instance()->loadConfigFile("aqplus.json");
        showMemEdit      = getBoolValue(root, "showMemEdit", false);
        memEditMemSelect = getIntValue(root, "memEditMemSelect", 0);
        showIoRegsWindow = getBoolValue(root, "showIoRegsWindow", false);

        z80Core.loadConfig(root);
        cJSON_Delete(root);
    }

    void saveConfig() {
        if (hasSaved)
            return;
        hasSaved  = true;
        auto root = cJSON_CreateObject();
        cJSON_AddBoolToObject(root, "showMemEdit", showMemEdit);
        cJSON_AddNumberToObject(root, "memEditMemSelect", memEditMemSelect);
        cJSON_AddBoolToObject(root, "showIoRegsWindow", showIoRegsWindow);

        z80Core.saveConfig(root);
        Config::instance()->saveConfigFile("aqplus.json", root);
    }

    void getVideoSize(int &w, int &h) override {
        w = videoMode ? 640 : AqpVideo::activeWidth;
        h = AqpVideo::activeHeight * 2;
    }

    void getPixels(void *pixels, int pitch) override {
        const uint16_t *fb = video.getFb();

        auto activeWidth  = AqpVideo::activeWidth;
        auto activeHeight = AqpVideo::activeHeight;
        int  w            = videoMode ? 640 : AqpVideo::activeWidth;

        for (int j = 0; j < activeHeight * 2; j++) {
            for (int i = 0; i < w; i++) {
                ((uint32_t *)((uintptr_t)pixels + j * pitch))[i] =
                    col12_to_col32(fb[(j / 2) * activeWidth + (videoMode ? (i + 32) : i)]);
            }
        }
        renderOverlay(pixels, pitch);
    }

    void spiTx(const void *data, size_t length) override {
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
                    ay1.portRdData[0] = txBuf[1];
                    ay1.portRdData[1] = txBuf[2];
                }
                break;
            }

            case CMD_SET_VIDMODE: {
                if (txBuf.size() == 1 + 1) {
                    videoMode = txBuf[1];
                }
                break;
            }

            case CMD_FORCE_TURBO: {
                if (txBuf.size() == 1 + 1) {
                    forceTurbo = txBuf[1];
                }
                break;
            }

            case CMD_WRITE_KBBUF: {
                if (txBuf.size() == 1 + 1 && kbBuf.size() < kbBufSize) {
                    kbBuf.push_back(txBuf[1]);
                }
                break;
            }
        }
    }

    void pasteText(const std::string &str) override {
        std::lock_guard lock(mutexTypeInStr);
        typeInStr = str;
    }
    bool pasteIsDone() override {
        std::lock_guard lock(mutexTypeInStr);
        return typeInStr.empty();
    }

    uint8_t memRead(uint16_t addr) {
        // Handle CPM remap bit
        if (cpmRemap) {
            if (addr < 0x4000)
                addr += 0xC000;
            if (addr >= 0xC000)
                addr -= 0xC000;
        }

        // Get and decode banking register
        uint8_t  bankReg    = bankRegs[addr >> 14];
        unsigned page       = bankReg & 0x3F;
        bool     overlayRam = (bankReg & (1 << 6)) != 0;

        addr &= 0x3FFF;

        if (overlayRam && addr >= 0x3000 && addr < 0x3800) {
            return video.readScreenOrColorRam(addr);
        }

        if (page == 0) {
            if (addr < sizeof(fpgarom_start))
                return fpgarom_start[addr];
            return 0;
        } else if (page == 19) {
            return cartridgeInserted ? cartRom[addr] : 0xFF;
        } else if (page == 20) {
            return video.videoRam[addr];
        } else if (page == 21) {
            if (addr < 0x800) {
                return video.charRam[addr];
            }
        } else if (page >= 32 && page < 64) {
            return mainRam[(page - 32) * 0x4000 + addr];
        }
        return 0xFF;
    }

    void memWrite(uint16_t addr, uint8_t data) {
        // Handle CPM remap bit
        if (cpmRemap) {
            if (addr < 0x4000)
                addr += 0xC000;
            if (addr >= 0xC000)
                addr -= 0xC000;
        }

        // Get and decode banking register
        uint8_t  bankReg    = bankRegs[addr >> 14];
        unsigned page       = bankReg & 0x3F;
        bool     overlayRam = (bankReg & (1 << 6)) != 0;
        bool     readonly   = (bankReg & (1 << 7)) != 0;
        addr &= 0x3FFF;

        if (overlayRam && addr >= 0x3000 && addr < 0x3800) {
            video.writeScreenOrColorRam(addr, data);
            return;
        }

        if (readonly && !(overlayRam && addr >= 0x3800))
            return;

        if (page < 16) {
            // System ROM is readonly
            return;
        } else if (page == 19) {
            // Game ROM is readonly
            return;
        } else if (page == 20) {
            video.videoRam[addr] = data;
        } else if (page == 21) {
            if (addr < 0x800) {
                video.charRam[addr] = data;
            }
        } else if (page >= 32 && page < 64) {
            mainRam[(page - 32) * 0x4000 + addr] = data;
        }
    }

    uint8_t ioRead(uint16_t addr) {
        uint8_t addr8 = addr & 0xFF;

        if (!sysCtrlDisableExt) {
            if (addr8 >= 0xE0 && addr8 <= 0xED) {
                return video.readReg(addr8);
            }
            switch (addr8) {
                case 0xEE: return irqMask;
                case 0xEF: return irqStatus;
                case 0xF0: return bankRegs[0];
                case 0xF1: return bankRegs[1];
                case 0xF2: return bankRegs[2];
                case 0xF3: return bankRegs[3];
                case 0xF4: return UartProtocol::instance()->readCtrl();
                case 0xF5: return UartProtocol::instance()->readData();
            }
        }

        switch (addr8) {
            case 0xF6:
            case 0xF7:
                if (!sysCtrlAyDisable)
                    return ay1.read();
                return 0xFF;

            case 0xF8:
            case 0xF9:
                if (!(sysCtrlAyDisable || sysCtrlDisableExt))
                    return ay2.read();
                return 0xFF;

            case 0xFA: {
                uint8_t result = 0;
                if (!kbBuf.empty()) {
                    result = kbBuf.front();
                    kbBuf.pop_front();
                }
                return result;
            }
            case 0xFB: return (
                (sysCtrlWarmBoot ? (1 << 7) : 0) |
                (sysCtrlTurboUnlimited ? (1 << 3) : 0) |
                (sysCtrlTurbo ? (1 << 2) : 0) |
                (sysCtrlAyDisable ? (1 << 1) : 0) |
                (sysCtrlDisableExt ? (1 << 0) : 0));

            case 0xFC: /* printf("Cassette port input (%04x)\n", addr); */ return 0xFF;
            case 0xFD: return video.readReg(addr & 0xFF);
            case 0xFE: /* printf("Clear to send status (%04x)\n", addr); */ return 0xFF;
            case 0xFF: {
                // Keyboard matrix. Selected rows are passed in the upper 8 address lines.
                uint8_t rows = addr >> 8;

                // Wire-AND all selected rows.
                uint8_t result = 0xFF;
                for (int i = 0; i < 8; i++) {
                    if ((rows & (1 << i)) == 0) {
                        result &= keybMatrix[i];
                    }
                }
                return result;
            }
            default: break;
        }

        printf("ioRead(0x%02x)\n", addr8);
        return 0xFF;
    }

    void ioWrite(uint16_t addr, uint8_t data) {
        uint8_t addr8 = addr & 0xFF;

        if (!sysCtrlDisableExt) {
            if ((addr8 >= 0xE0 && addr8 <= 0xEB) || (addr8 == 0xED)) {
                video.writeReg(addr8, data);
                return;
            }

            switch (addr8) {
                case 0xEC: audioDAC = data; return;
                case 0xEE: irqMask = data & 3; return;
                case 0xEF: irqStatus &= ~data; return;
                case 0xF0: bankRegs[0] = data; return;
                case 0xF1: bankRegs[1] = data; return;
                case 0xF2: bankRegs[2] = data; return;
                case 0xF3: bankRegs[3] = data; return;
                case 0xF4: UartProtocol::instance()->writeCtrl(data); return;
                case 0xF5: UartProtocol::instance()->writeData(data); return;
                case 0xFA: kbBuf.clear(); return;
            }
        }

        switch (addr8) {
            case 0xF6:
            case 0xF7:
                if (!sysCtrlAyDisable)
                    ay1.write(addr8, data);
                return;

            case 0xF8:
            case 0xF9:
                if (!(sysCtrlAyDisable || sysCtrlDisableExt))
                    ay2.write(addr8, data);
                return;

            case 0xFB:
                sysCtrlDisableExt     = (data & (1 << 0)) != 0;
                sysCtrlAyDisable      = (data & (1 << 1)) != 0;
                sysCtrlTurbo          = (data & (1 << 2)) != 0;
                sysCtrlTurboUnlimited = (data & (1 << 3)) != 0;
                if ((data & (1 << 7)) != 0)
                    reset(false);
                return;

            case 0xFC: soundOutput = (data & 1) != 0; break;
            case 0xFD: cpmRemap = (data & 1) != 0; break;
            case 0xFE: /* printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); */ break;
            case 0xFF: break;
            default: printf("ioWrite(0x%02x, 0x%02x)\n", addr8, data); break;
        }
    }

    void emulateFrame(int16_t *audioBuf, unsigned numSamples) override {
        z80Core.setEnableDebugger(enableDebugger);

        int lineHalfCycles   = 0; // Half-cycles for this line
        int sampleHalfCycles = 0; // Half-cycles for this sample
        video.videoLine      = 0;

        for (unsigned aidx = 0; aidx < numSamples; aidx++) {
            int speedMultiplier  = forceTurbo ? 4 : (sysCtrlTurbo ? (sysCtrlTurboUnlimited ? 8 : 2) : 1);
            int hcyclesPerSample = HCYCLES_PER_SAMPLE * speedMultiplier;
            int hcyclesPerLine   = HCYCLES_PER_LINE * speedMultiplier;

            // Emulate for the duration of one audio sample
            while (sampleHalfCycles < hcyclesPerSample) {
                int halfCycles = z80Core.emulate() * 2;
                lineHalfCycles += halfCycles;
                sampleHalfCycles += halfCycles;

                // Render video line
                if (lineHalfCycles >= hcyclesPerLine) {
                    lineHalfCycles -= hcyclesPerLine;
                    video.drawLine(video.videoLine++);
                    irqStatus |= video.isOnVideoIrqLine() ? (1 << 1) : 0;
                    irqStatus |= video.isOnStartOfVBlank() ? (1 << 0) : 0;
                }
            }
            sampleHalfCycles -= hcyclesPerSample;

            // Render audio
            if (audioBuf != nullptr) {
                // Take average of 5 AY8910 samples to match sampling rate (16*5*44100 = 3.528MHz)
                unsigned audioLeft  = 0;
                unsigned audioRight = 0;

                for (int i = 0; i < 5; i++) {
                    uint16_t abc[3];
                    ay1.render(abc);
                    audioLeft += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
                    audioRight += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];

                    ay2.render(abc);
                    audioLeft += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
                    audioRight += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];

                    audioLeft += (audioDAC << 4);
                    audioRight += (audioDAC << 4);
                }

                uint16_t beep = soundOutput ? 10000 : 0;
                audioLeft += beep;
                audioRight += beep;

                float l = audioLeft / 65535.0f;
                float r = audioRight / 65535.0f;
                l       = dcBlockLeft.filter(l);
                r       = dcBlockRight.filter(r);
                l       = std::min(std::max(l, -1.0f), 1.0f);
                r       = std::min(std::max(r, -1.0f), 1.0f);

                audioBuf[aidx * 2 + 0] = (int16_t)(l * 32767.0f);
                audioBuf[aidx * 2 + 1] = (int16_t)(r * 32767.0f);
            }

            keyboardTypeIn();
        }
    }

    bool loadCartridgeROM(const std::string &path) {
        auto ifs = std::ifstream(path, std::ifstream::binary);
        if (!ifs.good()) {
            return false;
        }

        ifs.seekg(0, ifs.end);
        auto fileSize = ifs.tellg();
        ifs.seekg(0, ifs.beg);

        if (fileSize == 8192) {
            ifs.read((char *)(cartRom + 8192), fileSize);
            // Mirror ROM to $C000
            memcpy(cartRom, cartRom + 8192, 8192);

        } else if (fileSize == 16384) {
            ifs.read((char *)cartRom, fileSize);

        } else {
            fprintf(stderr, "Invalid cartridge ROM file: %u, should be either exactly 8 or 16KB.\n", (unsigned)fileSize);
            return false;
        }
        ifs.close();

        cartridgeInserted = true;

        return true;
    }

    void keyboardTypeIn() {
        std::lock_guard lock(mutexTypeInStr);
        if (kbBuf.size() < kbBufSize && !typeInStr.empty()) {
            char ch = typeInStr.front();
            typeInStr.erase(typeInStr.begin());
            Keyboard::instance()->pressKey(ch);
        }
    }

    void fileMenu() override {
        if (ImGui::MenuItem("Load cartridge ROM...", "")) {
            char const *lFilterPatterns[1] = {"*.rom"};
            char       *romFile            = tinyfd_openFileDialog("Open ROM file", "", 1, lFilterPatterns, "ROM files", 0);
            if (romFile) {
                if (loadCartridgeROM(romFile)) {
                    reset(true);
                }
            }
        }
        if (ImGui::MenuItem("Eject cartridge", "", false, cartridgeInserted)) {
            cartridgeInserted = false;
            reset(true);
        }
        ImGui::Separator();
    }

    void dbgMenu() override {
        if (!enableDebugger)
            return;

        ImGui::MenuItem("Memory editor", "", &showMemEdit);
        ImGui::MenuItem("IO Registers", "", &showIoRegsWindow);
        z80Core.dbgMenu();

        ImGui::Separator();
        if (ImGui::MenuItem("Clear memory (0x00) & reset Aquarius+", "")) {
            memset(video.screenRam, 0, sizeof(video.screenRam));
            memset(video.colorRam, 0, sizeof(video.colorRam));
            memset(mainRam, 0, sizeof(mainRam));
            memset(video.videoRam, 0, sizeof(video.videoRam));
            memset(video.charRam, 0, sizeof(video.charRam));
            reset(true);
        }
        if (ImGui::MenuItem("Clear memory (0xA5) & reset Aquarius+", "")) {
            memset(video.screenRam, 0xA5, sizeof(video.screenRam));
            memset(video.colorRam, 0xA5, sizeof(video.colorRam));
            memset(mainRam, 0xA5, sizeof(mainRam));
            memset(video.videoRam, 0xA5, sizeof(video.videoRam));
            memset(video.charRam, 0xA5, sizeof(video.charRam));
            reset(true);
        }
    }

    void dbgWindows() override {
        if (!enableDebugger)
            return;

        z80Core.dbgWindows();

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
            if (ImGui::CollapsingHeader("Interrupt")) {
                ImGui::Text("$EE IRQMASK: $%02X %s%s", irqMask, irqMask & 2 ? "[LINE]" : "", irqMask & 1 ? "[VBLANK]" : "");
                ImGui::Text("$EF IRQSTAT: $%02X %s%s", irqStatus, irqStatus & 2 ? "[LINE]" : "", irqStatus & 1 ? "[VBLANK]" : "");
            }
            if (ImGui::CollapsingHeader("Banking")) {
                ImGui::Text("$F0 BANK0: $%02X - page:%u%s%s", bankRegs[0], bankRegs[0] & 0x3F, bankRegs[0] & 0x80 ? " RO" : "", bankRegs[0] & 0x40 ? " OVL" : "");
                ImGui::Text("$F1 BANK1: $%02X - page:%u%s%s", bankRegs[1], bankRegs[1] & 0x3F, bankRegs[1] & 0x80 ? " RO" : "", bankRegs[1] & 0x40 ? " OVL" : "");
                ImGui::Text("$F2 BANK2: $%02X - page:%u%s%s", bankRegs[2], bankRegs[2] & 0x3F, bankRegs[2] & 0x80 ? " RO" : "", bankRegs[2] & 0x40 ? " OVL" : "");
                ImGui::Text("$F3 BANK3: $%02X - page:%u%s%s", bankRegs[3], bankRegs[3] & 0x3F, bankRegs[3] & 0x80 ? " RO" : "", bankRegs[3] & 0x40 ? " OVL" : "");
            }
            if (ImGui::CollapsingHeader("Key buffer")) {
                {
                    uint8_t val = kbBuf.empty() ? 0 : kbBuf.front();
                    ImGui::Text("$FA KEYBUF: $%02X (%c)", val, val > 32 && val < 127 ? val : '.');
                }

                // auto keyMode = Keyboard::instance()->getKeyMode();
                // ImGui::Text(
                //     "   KEYMODE: $%02X %s%s%s\n",
                //     keyMode,
                //     (keyMode & 1) ? "[Enable]" : "",
                //     (keyMode & 2) ? "[ASCII]" : "[ScanCode]",
                //     (keyMode & 4) ? "[Repeat]" : "");

                std::string str = "Key buffer: ";

                for (unsigned i = 0; i < kbBuf.size(); i++) {
                    // if (keyMode & 2) {
                    uint8_t val = kbBuf[i];
                    str += fmtstr("%c", val > 32 && val < 127 ? val : '.');
                    // } else {
                    //     str += fmtstr("%02X ", kbBuf[i]);
                    // }
                }
                ImGui::Text("%s", str.c_str());
            }
            if (ImGui::CollapsingHeader("Other")) {
                uint8_t sysctrl =
                    ((sysCtrlTurboUnlimited ? (1 << 3) : 0) |
                     (sysCtrlTurbo ? (1 << 2) : 0) |
                     (sysCtrlAyDisable ? (1 << 1) : 0) |
                     (sysCtrlDisableExt ? (1 << 0) : 0));
                ImGui::Text(
                    "$FB SYSCTRL: $%02X %s%s%s%s", sysctrl,
                    sysctrl & 8 ? "[UNLIMITED]" : "",
                    sysctrl & 4 ? "[TURBO]" : "",
                    sysctrl & 2 ? "[AYDIS]" : "",
                    sysctrl & 1 ? "[EXTDIS]" : "");
            }
            if (ImGui::CollapsingHeader("Audio AY1")) {
                ay1.dbgDrawIoRegs();
            }
            if (ImGui::CollapsingHeader("Audio AY2")) {
                ay2.dbgDrawIoRegs();
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

    void dbgWndMemEdit(bool *p_open) {
        if (memAreas.empty()) {
            memAreas.emplace_back("Z80 memory", nullptr, 0x10000);
            memAreas.emplace_back("Screen RAM", video.screenRam, sizeof(video.screenRam));
            memAreas.emplace_back("Color RAM", video.colorRam, sizeof(video.colorRam));
            memAreas.emplace_back("Page 19: Cartridge ROM", cartRom, sizeof(cartRom));
            memAreas.emplace_back("Page 20: Video RAM", video.videoRam, sizeof(video.videoRam));
            memAreas.emplace_back("Page 21: Character RAM", video.charRam, sizeof(video.charRam));

            for (int i = 32; i < 64; i++) {
                char tmp[256];
                snprintf(tmp, sizeof(tmp), "Page %d: Main RAM $%05X-$%05X", i, (i - 32) * 16384, ((i + 1) - 32) * 16384 - 1);
                memAreas.emplace_back(tmp, mainRam + (i - 32) * 16384, 16384);
            }
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
                    return (ImU8)memRead((uint16_t)off);
                };
                memEdit.writeFn = [this](ImU8 *data, size_t off, ImU8 d) {
                    memWrite((uint16_t)off, d);
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
};

std::shared_ptr<EmuState> newAqpEmuState() {
    return std::make_shared<AqpEmuState>();
}
