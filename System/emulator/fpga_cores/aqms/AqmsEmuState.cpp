#include "EmuState.h"
#include "DCBlock.h"
#include "Z80Core.h"
#include "VDP.h"
#include "SN76489.h"
#include "aqmsrom.h"
#include "UartProtocol.h"
#include "FPGA.h"

#define HCYCLES_PER_LINE   (455)
#define HCYCLES_PER_SAMPLE (162)

class AqmsEmuState : public EmuState {
public:
    Z80Core z80Core;
    VDP     vdp;
    SN76489 psg;
    DCBlock dcBlockLeft;
    DCBlock dcBlockRight;
    bool    startupMode = true;
    uint8_t cartRom[512 * 1024]; // Cartridge ROM
    uint8_t systemRam[8 * 1024]; // System RAM ($C000-$DFFF / $E000-$FFFF)
    uint8_t romFrame2Page = 2;   // $8000-$BFFF
    uint8_t romFrame1Page = 1;   // $4000-$7FFF
    uint8_t romFrame0Page = 0;   // $0400-$3FFF
    uint8_t regionBits    = 3;
    uint8_t keybMatrix[8] = {0};

    AqmsEmuState() {
        coreType         = 1;
        coreFlags        = 0;
        coreVersionMajor = 1;
        coreVersionMinor = 0;
        memcpy(coreName, "Master System   ", sizeof(coreName));

        z80Core.hasIrq   = [this] { return vdp.isIrqPending(); };
        z80Core.memRead  = [this](uint16_t addr) { return memRead(addr); };
        z80Core.memWrite = [this](uint16_t addr, uint8_t data) { memWrite(addr, data); };
        z80Core.ioRead   = [this](uint16_t addr) { return ioRead(addr); };
        z80Core.ioWrite  = [this](uint16_t addr, uint8_t data) { ioWrite(addr, data); };

        memset(keybMatrix, 0xFF, sizeof(keybMatrix));
        loadConfig();
        reset(true);
    }

    virtual ~AqmsEmuState() {
        saveConfig();
    }

    void reset(bool cold = false) override {
        z80Core.reset();
        vdp.reset();
        psg.reset();
        startupMode = true;
        regionBits  = 3;
    }

    void loadConfig() {
        auto root = Config::instance()->loadConfigFile("aqms.json");
        // showMemEdit      = getBoolValue(root, "showMemEdit", false);
        // memEditMemSelect = getIntValue(root, "memEditMemSelect", 0);
        // showIoRegsWindow = getBoolValue(root, "showIoRegsWindow", false);

        z80Core.loadConfig(root);
        cJSON_Delete(root);
    }

    void saveConfig() {
        auto root = cJSON_CreateObject();
        // cJSON_AddBoolToObject(root, "showMemEdit", showMemEdit);
        // cJSON_AddNumberToObject(root, "memEditMemSelect", memEditMemSelect);
        // cJSON_AddBoolToObject(root, "showIoRegsWindow", showIoRegsWindow);

        z80Core.saveConfig(root);
        Config::instance()->saveConfigFile("aqms.json", root);
    }

    void getVideoSize(int &w, int &h) override {
        w = 640;
        h = 480;
    }

    void getPixels(void *pixels, int pitch) override {
        auto fb = vdp.getFramebuffer();

        for (int j = 0; j < 480; j++) {
            int y = (j - 48) / 2;

            for (int i = 0; i < 640; i++) {
                int x = (i - 64) / 2;

                uint32_t col = 0;
                if (y >= 0 && y < 192 && x >= 0 && x < 256)
                    col = fb[y * 256 + x];

                ((uint32_t *)((uintptr_t)pixels + j * pitch))[i] = col;
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
        }
    }

    uint8_t memRead(uint16_t addr) {
        if (startupMode) {
            if (addr < 0x4000) {
                if (addr < aqmsrom_cim_len) {
                    return aqmsrom_cim[addr];
                } else {
                    return 0;
                }
            }
        }
        if (addr < 0x0400)
            return cartRom[addr];
        if (addr < 0x4000)
            return cartRom[((uint32_t)romFrame0Page << 14) | (addr & 0x3FFF)];
        if (addr < 0x8000)
            return cartRom[((uint32_t)romFrame1Page << 14) | (addr & 0x3FFF)];
        if (addr < 0xC000) {
            return cartRom[((uint32_t)romFrame2Page << 14) | (addr & 0x3FFF)];
        }
        return systemRam[addr & 0x1FFF];
    }

    void memWrite(uint16_t addr, uint8_t data) {
        if (startupMode && addr >= 0x4000 && addr < 0x8000) {
            cartRom[((uint32_t)romFrame1Page << 14) | (addr & 0x3FFF)] = data;
        }

        if (addr == 0xFFFF) {
            romFrame2Page = data & 31; //(romPageCount - 1);
        } else if (addr == 0xFFFE) {
            romFrame1Page = data & 31; //(romPageCount - 1);
        } else if (addr == 0xFFFD) {
            romFrame0Page = data & 31; //(romPageCount - 1);
        } else if (addr == 0xFFFC) {
            startupMode = false;
        }
        if (addr >= 0xC000) {
            systemRam[addr & 0x1FFF] = data;
        }
    }

    uint8_t getIoDC() {
        uint8_t joypadData = 0xFF;
        if ((keybMatrix[5] & (1 << 5)) == 0) // x
            joypadData &= ~(1 << 5);
        if ((keybMatrix[6] & (1 << 3)) == 0) // z
            joypadData &= ~(1 << 4);
        if ((keybMatrix[1] & (1 << 7)) == 0) // right
            joypadData &= ~(1 << 3);
        if ((keybMatrix[2] & (1 << 6)) == 0) // left
            joypadData &= ~(1 << 2);
        if ((keybMatrix[2] & (1 << 7)) == 0) // down
            joypadData &= ~(1 << 1);
        if ((keybMatrix[1] & (1 << 6)) == 0) // up
            joypadData &= ~(1 << 0);
        return joypadData;
    }

    uint8_t getIoDD() {
        return (regionBits << 6) | 0x3F;
    }

    uint8_t ioRead(uint16_t addr) {
        // SMS only decodes A7/A6/A0, so mask other bits
        addr &= 0xFF;
        if (startupMode && addr == 0x10) {
            return UartProtocol::instance()->readCtrl();

        } else if (startupMode && addr == 0x11) {
            return UartProtocol::instance()->readData();

        } else {
            switch (addr & 0xC1) {
                // case 0x00: return port3E;                   // 0x3E
                case 0x01: return 0xFF;                  // 0x3F
                case 0x40: return vdp.regVCounterRead(); // 0x7E
                case 0x41: return vdp.regHCounterRead(); // 0x7F
                case 0x80: return vdp.readDataPort();    // 0xBE  - VDP data port
                case 0x81: return vdp.readControlPort(); // 0xBF  - VDP control port -> status
                case 0xC0: return getIoDC();             // 0xDC - Joypad port 1
                case 0xC1: return getIoDD();             // 0xDD - Joypad port 2
                default: break;
            }
        }
        return 0xFF;
    }

    void ioWrite(uint16_t addr, uint8_t data) {
        // Apparently SMS only decodes A7/A6/A0, so mask other bits
        addr &= 0xFF;
        if (startupMode && addr == 0x10) {
            UartProtocol::instance()->writeCtrl(data);
            return;

        } else if (startupMode && addr == 0x11) {
            UartProtocol::instance()->writeData(data);
            return;

        } else {
            switch (addr & 0xC1) {
                // case 0x00:
                //     port3E = data;
                //     cout << "port3E: " << toHex(data) << endl;
                //     return;  // 0x3E - Memory enables
                case 0x01: { // 0x3F
                    regionBits = (((data >> 7) & 1) << 1) | ((data >> 5) & 1);
                    return;
                }

                case 0x40:                                     // 0x7E
                case 0x41: psg.write(data); return;            // 0x7F  - SN76489 PSG
                case 0x80: vdp.writeDataPort(data); return;    // 0xBE  - VDP data port
                case 0x81: vdp.writeControlPort(data); return; // 0xBF  - VDP control port
                case 0xC0: return;                             // 0xDC, 0xDE
                case 0xC1: return;                             // 0xDD, 0xDF
                default: break;
            }
        }
    }

    void emulateFrame(int16_t *audioBuf, unsigned numSamples) override {
        z80Core.setEnableDebugger(enableDebugger);

        int lineHalfCycles   = 0; // Half-cycles for this line
        int sampleHalfCycles = 0; // Half-cycles for this sample
        vdp.line             = 0;

        for (unsigned aidx = 0; aidx < numSamples; aidx++) {
            int hcyclesPerSample = HCYCLES_PER_SAMPLE;
            int hcyclesPerLine   = HCYCLES_PER_LINE;

            // Emulate for the duration of one audio sample
            while (sampleHalfCycles < hcyclesPerSample) {
                int halfCycles = z80Core.emulate() * 2;
                lineHalfCycles += halfCycles;
                sampleHalfCycles += halfCycles;

                // Render video line
                if (lineHalfCycles >= hcyclesPerLine) {
                    lineHalfCycles -= hcyclesPerLine;
                    vdp.renderLine();
                }
            }
            sampleHalfCycles -= hcyclesPerSample;

            // Render audio
            if (audioBuf != nullptr) {
                unsigned audioLeft  = 0;
                unsigned audioRight = 0;

                for (int i = 0; i < 5; i++) {
                    uint16_t val = psg.render() * 3;
                    audioLeft += val;
                    audioRight += val;
                }

                float l = audioLeft / 65535.0f;
                float r = audioRight / 65535.0f;
                l       = dcBlockLeft.filter(l);
                r       = dcBlockRight.filter(r);
                l       = std::min(std::max(l, -1.0f), 1.0f);
                r       = std::min(std::max(r, -1.0f), 1.0f);

                audioBuf[aidx * 2 + 0] = (int16_t)(l * 32767.0f);
                audioBuf[aidx * 2 + 1] = (int16_t)(r * 32767.0f);
            }
        }
    }

    void dbgMenu() override {
        if (!enableDebugger)
            return;

        // ImGui::MenuItem("Memory editor", "", &showMemEdit);
        // ImGui::MenuItem("IO Registers", "", &showIoRegsWindow);
        z80Core.dbgMenu();
    }

    void dbgWindows() override {
        if (!enableDebugger)
            return;

        z80Core.dbgWindows();

        // if (showMemEdit)
        //     dbgWndMemEdit(&showMemEdit);
        // if (showIoRegsWindow)
        //     dbgWndIoRegs(&showIoRegsWindow);
    }
};

std::shared_ptr<EmuState> newAqmsEmuState() {
    return std::make_shared<AqmsEmuState>();
}
