#include "CoreAquariusPlus.h"
#include "FPGA.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"
#include "GameCtrl.h"
#include <math.h>
#include <nvs_flash.h>
#include "XzDecompress.h"
#include "DisplayOverlay/DisplayOverlay.h"

#include "CoreAquariusPlus.h"
#include "AqKeyboardDefs.h"

enum {
    IO_VCTRL    = 0xE0,
    IO_VPALSEL  = 0xEA,
    IO_VPALDATA = 0xEB,
    IO_BANK0    = 0xF0,
    IO_BANK1    = 0xF1,
    IO_BANK2    = 0xF2,
    IO_BANK3    = 0xF3,
};

enum {
    // Aq+ command
    CMD_RESET           = 0x01,
    CMD_FORCE_TURBO     = 0x02,
    CMD_SET_KEYB_MATRIX = 0x10,
    CMD_SET_HCTRL       = 0x11,
    CMD_WRITE_KBBUF     = 0x12,
    CMD_BUS_ACQUIRE     = 0x20,
    CMD_BUS_RELEASE     = 0x21,
    CMD_MEM_WRITE       = 0x22,
    CMD_MEM_READ        = 0x23,
    CMD_IO_WRITE        = 0x24,
    CMD_IO_READ         = 0x25,
    CMD_ROM_WRITE       = 0x30,
    CMD_SET_VIDMODE     = 0x40,
};

enum {
    FLAG_HAS_Z80       = (1 << 0),
    FLAG_MOUSE_SUPPORT = (1 << 1),
    FLAG_VIDEO_TIMING  = (1 << 2),
    FLAG_AQPLUS        = (1 << 3),
    FLAG_FORCE_TURBO   = (1 << 4),
};

class CoreAquariusPlus : public FpgaCore {
public:
    SemaphoreHandle_t mutex;
    uint64_t          prevMatrix        = 0;
    uint64_t          keybMatrix        = 0;
    GamePadData       gamepads[2]       = {0};
    uint8_t           videoTimingMode   = 0;
    bool              useT80            = false;
    bool              forceTurbo        = false;
    bool              bypassStartScreen = false;
    TimerHandle_t     bypassStartTimer  = nullptr;
    bool              bypassStartCancel = false;
    CoreInfo          coreInfo;

    // Mouse state
    bool    mousePresent = false;
    float   mouseX       = 0;
    float   mouseY       = 0;
    uint8_t mouseButtons = 0;
    int     mouseWheel   = 0;

    uint8_t mouseSensitivityDiv = 4;

    CoreAquariusPlus() {
        mutex            = xSemaphoreCreateRecursiveMutex();
        bypassStartTimer = xTimerCreate("", pdMS_TO_TICKS(CONFIG_BYPASS_START_TIME_MS), pdFALSE, this, _onBypassStartTimer);
    }

    virtual ~CoreAquariusPlus() {
        vSemaphoreDelete(mutex);
        if (bypassStartTimer)
            xTimerDelete(bypassStartTimer, portMAX_DELAY);
    }

    static void _onBypassStartTimer(TimerHandle_t xTimer) { static_cast<CoreAquariusPlus *>(pvTimerGetTimerID(xTimer))->onBypassStartTimer(); }

    void onBypassStartTimer() {
        // 'Press' enter key to bypass the Aquarius start screen
        if (!bypassStartCancel)
            keyChar('\r', false);
    }

    bool loadBitstream(const void *data, size_t length) override {
        bool result = false;
        if (data == nullptr) {
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
            extern const uint8_t fpgaImageXzhStart[] asm("_binary_aqp_top_bit_xzh_start");
            extern const uint8_t fpgaImageXzhEnd[] asm("_binary_aqp_top_bit_xzh_end");
            auto                 fpgaImage = xzhDecompress(fpgaImageXzhStart, fpgaImageXzhEnd - fpgaImageXzhStart);
            result                         = getFPGA()->loadBitstream(fpgaImage.data(), fpgaImage.size());
#else
            extern const uint8_t fpgaImageStart[] asm("_binary_morphbook_impl1_bit_start");
            extern const uint8_t fpgaImageEnd[] asm("_binary_morphbook_impl1_bit_end");
            data   = fpgaImageStart;
            length = fpgaImageEnd - fpgaImageStart;
            result = getFPGA()->loadBitstream(data, length);
#endif
        } else {
            result = getFPGA()->loadBitstream(data, length);
        }

        memset(&coreInfo, 0, sizeof(coreInfo));
        if (result) {
            getFPGA()->getCoreInfo(&coreInfo);

            applySettings();
        }
        return result;
    }

    void applySettings() {
        if ((coreInfo.flags & FLAG_HAS_Z80) == 0) {
            useT80 = true;
        }

        nvs_handle_t h;
        if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
            uint8_t mouseDiv = 0;
            if (nvs_get_u8(h, "mouseDiv", &mouseDiv) == ESP_OK) {
                mouseSensitivityDiv = mouseDiv;
            }

            uint8_t val8 = 0;
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
            if (nvs_get_u8(h, "videoTiming", &videoTimingMode) != ESP_OK) {
                videoTimingMode = 0;
            }

            if (coreInfo.flags & FLAG_HAS_Z80) {
                if (nvs_get_u8(h, "useT80", &val8) == ESP_OK) {
                    useT80 = val8 != 0;
                }
            }
#endif

            if (nvs_get_u8(h, "bypassStart", &val8) == ESP_OK) {
                bypassStartScreen = val8 != 0;
            }

            nvs_close(h);
        }
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        if (coreInfo.flags & FLAG_VIDEO_TIMING)
            aqpSetVideoMode(videoTimingMode);
#endif
        resetCore();
    }

    void aqpWriteKeybBuffer(uint8_t ch) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_WRITE_KBBUF, ch};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void resetCore() override {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t resetCfg = useT80 ? 1 : 0;
        uint8_t cmd[]    = {CMD_RESET, resetCfg};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);

        forceTurbo = false;

        bypassStartCancel = false;
        if (bypassStartScreen) {
            xTimerReset(bypassStartTimer, pdMS_TO_TICKS(CONFIG_BYPASS_START_TIME_MS));
        }
    }

    void aqpForceTurbo(bool en) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cfg   = en ? 1 : 0;
        uint8_t cmd[] = {CMD_FORCE_TURBO, cfg};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpUpdateKeybMatrix(uint64_t keybMatrix) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[9];
        cmd[0] = CMD_SET_KEYB_MATRIX;
        memcpy(&cmd[1], &keybMatrix, 8);
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpUpdateHandCtrl(uint8_t hctrl1, uint8_t hctrl2) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_SET_HCTRL, hctrl1, hctrl2};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpSetVideoMode(uint8_t mode) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_SET_VIDMODE, mode};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

#ifdef CONFIG_MACHINE_TYPE_AQPLUS
    void aqpAqcuireBus() {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_BUS_ACQUIRE};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpReleaseBus() {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_BUS_RELEASE};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpWriteMem(uint16_t addr, uint8_t data) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_MEM_WRITE, (uint8_t)(addr & 0xFF), (uint8_t)(addr >> 8), data};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    uint8_t aqpReadMem(uint16_t addr) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_MEM_READ, (uint8_t)(addr & 0xFF), (uint8_t)(addr >> 8)};
        fpga->spiTx(cmd, sizeof(cmd));

        uint8_t result[2];
        fpga->spiRx(result, 2);
        fpga->spiSel(false);
        return result[1];
    }

    void aqpWriteIO(uint16_t addr, uint8_t data) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_IO_WRITE, (uint8_t)(addr & 0xFF), (uint8_t)(addr >> 8), data};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    uint8_t aqpReadIO(uint16_t addr) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_IO_READ, (uint8_t)(addr & 0xFF), (uint8_t)(addr >> 8)};
        fpga->spiTx(cmd, sizeof(cmd));

        uint8_t result[2];
        fpga->spiRx(result, 2);
        fpga->spiSel(false);
        return result[1];
    }
#endif

    void keyScancode(uint8_t modifiers, unsigned scanCode, bool keyDown) override {
        RecursiveMutexLock lock(mutex);

        // Keyboard matrix emulation
        {
            int key = -1;
            switch (scanCode) {
                case SCANCODE_EQUALS: key = KEY_EQUALS; break;
                case SCANCODE_BACKSPACE: key = KEY_BACKSPACE; break;
                case SCANCODE_APOSTROPHE: key = KEY_COLON; break;
                case SCANCODE_RETURN: key = KEY_RETURN; break;
                case SCANCODE_SEMICOLON: key = KEY_SEMICOLON; break;
                case SCANCODE_PERIOD: key = KEY_PERIOD; break;
                case SCANCODE_INSERT: key = KEY_INSERT; break;
                case SCANCODE_DELETE: key = KEY_DELETE; break;
                case SCANCODE_MINUS: key = KEY_MINUS; break;
                case SCANCODE_SLASH: key = KEY_SLASH; break;
                case SCANCODE_0: key = KEY_0; break;
                case SCANCODE_P: key = KEY_P; break;
                case SCANCODE_L: key = KEY_L; break;
                case SCANCODE_COMMA: key = KEY_COMMA; break;
                case SCANCODE_UP: key = KEY_UP; break;
                case SCANCODE_RIGHT: key = KEY_RIGHT; break;
                case SCANCODE_9: key = KEY_9; break;
                case SCANCODE_O: key = KEY_O; break;
                case SCANCODE_K: key = KEY_K; break;
                case SCANCODE_M: key = KEY_M; break;
                case SCANCODE_N: key = KEY_N; break;
                case SCANCODE_J: key = KEY_J; break;
                case SCANCODE_LEFT: key = KEY_LEFT; break;
                case SCANCODE_DOWN: key = KEY_DOWN; break;
                case SCANCODE_8: key = KEY_8; break;
                case SCANCODE_I: key = KEY_I; break;
                case SCANCODE_7: key = KEY_7; break;
                case SCANCODE_U: key = KEY_U; break;
                case SCANCODE_H: key = KEY_H; break;
                case SCANCODE_B: key = KEY_B; break;
                case SCANCODE_HOME: key = KEY_HOME; break;
                case SCANCODE_END: key = KEY_END; break;
                case SCANCODE_6: key = KEY_6; break;
                case SCANCODE_Y: key = KEY_Y; break;
                case SCANCODE_G: key = KEY_G; break;
                case SCANCODE_V: key = KEY_V; break;
                case SCANCODE_C: key = KEY_C; break;
                case SCANCODE_F: key = KEY_F; break;
                case SCANCODE_PAGEUP: key = KEY_PGUP; break;
                case SCANCODE_PAGEDOWN: key = KEY_PGDN; break;
                case SCANCODE_5: key = KEY_5; break;
                case SCANCODE_T: key = KEY_T; break;
                case SCANCODE_4: key = KEY_4; break;
                case SCANCODE_R: key = KEY_R; break;
                case SCANCODE_D: key = KEY_D; break;
                case SCANCODE_X: key = KEY_X; break;
                case SCANCODE_PAUSE: key = KEY_PAUSE; break;
                case SCANCODE_PRINTSCREEN: key = KEY_PRTSCR; break;
                case SCANCODE_3: key = KEY_3; break;
                case SCANCODE_E: key = KEY_E; break;
                case SCANCODE_S: key = KEY_S; break;
                case SCANCODE_Z: key = KEY_Z; break;
                case SCANCODE_SPACE: key = KEY_SPACE; break;
                case SCANCODE_A: key = KEY_A; break;
                case SCANCODE_APPLICATION: key = KEY_MENU; break;
                case SCANCODE_TAB: key = KEY_TAB; break;
                case SCANCODE_2: key = KEY_2; break;
                case SCANCODE_W: key = KEY_W; break;
                case SCANCODE_1: key = KEY_1; break;
                case SCANCODE_Q: key = KEY_Q; break;
                default: break;
            }
            if (key >= 0) {
                if (keyDown)
                    keybMatrix |= (1ULL << key);
                else
                    keybMatrix &= ~(1ULL << key);
            }

            if (modifiers & (ModLShift | ModRShift))
                keybMatrix |= (1ULL << KEY_SHIFT);
            else
                keybMatrix &= ~(1ULL << KEY_SHIFT);

            if (modifiers & (ModLAlt | ModRAlt))
                keybMatrix |= (1ULL << KEY_ALT);
            else
                keybMatrix &= ~(1ULL << KEY_ALT);

            if (modifiers & (ModLCtrl | ModRCtrl))
                keybMatrix |= (1ULL << KEY_CTRL);
            else
                keybMatrix &= ~(1ULL << KEY_CTRL);

            if (modifiers & (ModLGui | ModRGui))
                keybMatrix |= (1ULL << KEY_GUI);
            else
                keybMatrix &= ~(1ULL << KEY_GUI);

            // Handle ESCAPE as if CTRL-C is pressed
            if (scanCode == SCANCODE_ESCAPE) {
                if (keyDown) {
                    keybMatrix |= (1ULL << KEY_C) | (1ULL << KEY_CTRL);
                } else {
                    keybMatrix &= ~((1ULL << KEY_C) | (1ULL << KEY_CTRL));
                }
            }
        }

        // Special keys
        {
            uint8_t combinedModifiers = (modifiers & 0xF) | (modifiers >> 4);
            if (scanCode == SCANCODE_ESCAPE && keyDown) {
                if (combinedModifiers == ModLCtrl) {
                    resetCore();
                } else if (combinedModifiers == (ModLShift | ModLCtrl)) {
                    // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                    esp_restart();
                }
            }
        }

        if (prevMatrix != keybMatrix) {
            // printf("keybMatrix: %016llx\n", keybMatrix);

            uint64_t tmpMatrix = ~keybMatrix;
            aqpUpdateKeybMatrix(tmpMatrix);
            prevMatrix = keybMatrix;
        }
    }

    void keyChar(uint8_t ch, bool isRepeat) override {
        bypassStartCancel = true;

        RecursiveMutexLock lock(mutex);
        aqpWriteKeybBuffer(ch);
    }

    void mouseReport(int dx, int dy, uint8_t buttonMask, int dWheel) override {
        RecursiveMutexLock lock(mutex);
        // printf("mouse %d %d %d %d\n", dx, dy, buttonMask, dWheel);

        float sensitivity = 1.0f / (float)mouseSensitivityDiv;
        mouseX            = std::max(0.0f, std::min(319.0f, mouseX + (float)(dx * sensitivity)));
        mouseY            = std::max(0.0f, std::min(199.0f, mouseY + (float)(dy * sensitivity)));
        mouseButtons      = buttonMask;
        mousePresent      = true;
        mouseWheel        = mouseWheel + dWheel;
    }

    void gameCtrlUpdated() {
        // Update hand controller
        uint8_t handCtrl[2] = {0xFF, 0xFF};

        for (int i = 0; i < 2; i++) {
            if (gamepads[i].buttons & GCB_A)
                handCtrl[i] &= ~(1 << 6);
            if (gamepads[i].buttons & GCB_B)
                handCtrl[i] &= ~((1 << 7) | (1 << 2));
            if (gamepads[i].buttons & GCB_X)
                handCtrl[i] &= ~((1 << 7) | (1 << 5));
            if (gamepads[i].buttons & GCB_Y)
                handCtrl[i] &= ~((1 << 5));
            if (gamepads[i].buttons & GCB_LB)
                handCtrl[i] &= ~((1 << 7) | (1 << 1));
            if (gamepads[i].buttons & GCB_RB)
                handCtrl[i] &= ~((1 << 7) | (1 << 0));

            // Map D-pad on hand controller disc
            unsigned p = 0;
            if ((gamepads[i].buttons & GCB_DPAD_UP) == GCB_DPAD_UP)
                p = 13;
            else if ((gamepads[i].buttons & (GCB_DPAD_UP | GCB_DPAD_RIGHT)) == (GCB_DPAD_UP | GCB_DPAD_RIGHT))
                p = 15;
            else if ((gamepads[i].buttons & GCB_DPAD_RIGHT) == GCB_DPAD_RIGHT)
                p = 1;
            else if ((gamepads[i].buttons & (GCB_DPAD_DOWN | GCB_DPAD_RIGHT)) == (GCB_DPAD_DOWN | GCB_DPAD_RIGHT))
                p = 3;
            else if ((gamepads[i].buttons & GCB_DPAD_DOWN) == GCB_DPAD_DOWN)
                p = 5;
            else if ((gamepads[i].buttons & (GCB_DPAD_DOWN | GCB_DPAD_LEFT)) == (GCB_DPAD_DOWN | GCB_DPAD_LEFT))
                p = 7;
            else if ((gamepads[i].buttons & GCB_DPAD_LEFT) == GCB_DPAD_LEFT)
                p = 9;
            else if ((gamepads[i].buttons & (GCB_DPAD_UP | GCB_DPAD_LEFT)) == (GCB_DPAD_UP | GCB_DPAD_LEFT))
                p = 11;

            {
                float x = gamepads[i].lx / 128.0f;
                float y = gamepads[i].ly / 128.0f;

                float len   = sqrtf(x * x + y * y);
                float angle = 0;
                if (len > 0.4f) {
                    angle = atan2f(y, x) / (float)M_PI * 180.0f + 180.0f;
                    p     = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
                }
            }

            switch (p) {
                case 1: handCtrl[i] &= ~((1 << 1)); break;
                case 2: handCtrl[i] &= ~((1 << 4) | (1 << 1)); break;
                case 3: handCtrl[i] &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
                case 4: handCtrl[i] &= ~((1 << 1) | (1 << 0)); break;
                case 5: handCtrl[i] &= ~((1 << 0)); break;
                case 6: handCtrl[i] &= ~((1 << 4) | (1 << 0)); break;
                case 7: handCtrl[i] &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
                case 8: handCtrl[i] &= ~((1 << 3) | (1 << 0)); break;
                case 9: handCtrl[i] &= ~((1 << 3)); break;
                case 10: handCtrl[i] &= ~((1 << 4) | (1 << 3)); break;
                case 11: handCtrl[i] &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
                case 12: handCtrl[i] &= ~((1 << 3) | (1 << 2)); break;
                case 13: handCtrl[i] &= ~((1 << 2)); break;
                case 14: handCtrl[i] &= ~((1 << 4) | (1 << 2)); break;
                case 15: handCtrl[i] &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
                case 16: handCtrl[i] &= ~((1 << 2) | (1 << 1)); break;
                default: break;
            }
        }

        aqpUpdateHandCtrl(handCtrl[0], handCtrl[1]);
    }

    void gamepadReport(unsigned idx, const GamePadData &data) override {
        if (idx > 1)
            return;

        RecursiveMutexLock lock(mutex);

        uint16_t pressed  = (~gamepads[idx].buttons & data.buttons);
        uint16_t released = (gamepads[idx].buttons & ~data.buttons);
        uint16_t changed  = (pressed | released);

        // printf("idx=%u pressed=%04X\n", idx, pressed);

        if (idx == 0) {
            auto kb = getKeyboard();
            if (pressed & GCB_GUIDE) {
                kb->handleScancode(SCANCODE_LCTRL, true);
                kb->handleScancode(SCANCODE_TAB, true);
                kb->handleScancode(SCANCODE_TAB, false);
                kb->handleScancode(SCANCODE_LCTRL, false);
            }

            if (getDisplayOverlay()->isVisible()) {
                if (changed & GCB_DPAD_UP)
                    kb->handleScancode(SCANCODE_UP, (pressed & GCB_DPAD_UP) != 0);
                if (changed & GCB_DPAD_DOWN)
                    kb->handleScancode(SCANCODE_DOWN, (pressed & GCB_DPAD_DOWN) != 0);
                if (changed & GCB_DPAD_LEFT)
                    kb->handleScancode(SCANCODE_LEFT, (pressed & GCB_DPAD_LEFT) != 0);
                if (changed & GCB_DPAD_RIGHT)
                    kb->handleScancode(SCANCODE_RIGHT, (pressed & GCB_DPAD_RIGHT) != 0);
                if (changed & GCB_A)
                    kb->handleScancode(SCANCODE_RETURN, (pressed & GCB_A) != 0);
                if (changed & GCB_B)
                    kb->handleScancode(SCANCODE_ESCAPE, (pressed & GCB_B) != 0);
            }
        }

        gamepads[idx] = data;
        gameCtrlUpdated();
    }

    void cmdGetMouse() {
        // DBGF("GETMOUSE()");

        auto up = getUartProtocol();
        up->txStart();
        if (!mousePresent) {
            up->txWrite(ERR_NOT_FOUND);
            return;
        }

        up->txWrite(0);

        uint16_t x = (uint16_t)mouseX;
        uint8_t  y = (uint8_t)mouseY;
        up->txWrite(x & 0xFF);
        up->txWrite(x >> 8);
        up->txWrite(y);
        up->txWrite(mouseButtons);
        up->txWrite((int8_t)std::max(-128, std::min(mouseWheel, 127)));
        mouseWheel = 0;
    }

    void cmdGetGameCtrl(uint8_t idx) {
        // DBGF("GETGAMECTRL");

        auto up = getUartProtocol();
        up->txStart();
        if (idx > 1) {
            up->txWrite(ERR_NOT_FOUND);
            return;
        }
        up->txWrite(0);
        up->txWrite(gamepads[idx].lx);
        up->txWrite(gamepads[idx].ly);
        up->txWrite(gamepads[idx].rx);
        up->txWrite(gamepads[idx].ry);
        up->txWrite(gamepads[idx].lt);
        up->txWrite(gamepads[idx].rt);
        up->txWrite(gamepads[idx].buttons & 0xFF);
        up->txWrite(gamepads[idx].buttons >> 8);
    }

    int uartCommand(uint8_t cmd, const uint8_t *buf, size_t len) override {
        RecursiveMutexLock lock(mutex);
        switch (cmd) {
            case ESPCMD_GETMOUSE: {
                cmdGetMouse();
                return 1;
            }
            case ESPCMD_GETGAMECTRL: {
                if (len == 1) {
                    cmdGetGameCtrl(buf[0]);
                    return 1;
                }
                return 0;
            }
            default: break;
        }
        return -1;
    }

#ifdef CONFIG_MACHINE_TYPE_AQPLUS
    void takeScreenshot(Menu &menu) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());

        menu.drawMessage("Taking screenshot");

        std::vector<uint8_t> buf;

        // Read text RAM
        {
            // Get state
            aqpAqcuireBus();
            uint8_t vctrl   = aqpReadIO(IO_VCTRL);
            uint8_t vpalsel = aqpReadIO(IO_VPALSEL);
            uint8_t bank0   = aqpReadIO(IO_BANK0);

            if (vctrl & 1) {
                aqpWriteIO(IO_BANK0, (3 << 6) | 0);

                bool mode80 = (vctrl & 0x40) != 0;
                buf.reserve(mode80 ? (4096 + 32 + 1) : (2048 + 32 + 1));

                // Read text and color RAM
                {
                    if (mode80)
                        aqpWriteIO(IO_VCTRL, vctrl & ~0x80);

                    for (int i = 0; i < 2048; i++)
                        buf.push_back(aqpReadMem(0x3000 + i));

                    if (mode80) {
                        aqpWriteIO(IO_VCTRL, vctrl | 0x80);
                        for (int i = 0; i < 2048; i++)
                            buf.push_back(aqpReadMem(0x3000 + i));
                    }
                }

                // Read palette
                for (int i = 0; i < 32; i++) {
                    aqpWriteIO(IO_VPALSEL, i);
                    buf.push_back(aqpReadIO(IO_VPALDATA));
                }

                // Save video mode
                buf.push_back(vctrl & 0x61);
            }

            // Restore state
            aqpWriteIO(IO_BANK0, bank0);
            aqpWriteIO(IO_VPALSEL, vpalsel);
            aqpWriteIO(IO_VCTRL, vctrl);
            aqpReleaseBus();
        }

        if (!buf.empty()) {
            std::string fileName = "screenshot.scr";
            if (menu.editString("Enter filename for screenshot", fileName, 32)) {
                // Save cartridge contents to file
                auto vfs = getSDCardVFS();
                int  fd;
                if ((fd = vfs->open(FO_WRONLY | FO_CREATE, fileName.c_str())) >= 0) {
                    vfs->write(fd, buf.size(), buf.data());
                    vfs->close(fd);
                }
            }
        }
    }

    void dumpCartridge(Menu &menu) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());

        menu.drawMessage("Reading cartridge");

        std::vector<uint8_t> buf;
        buf.reserve(16384);

        // Read cartridge
        {
            // Get state
            aqpAqcuireBus();
            uint8_t bank0 = aqpReadIO(IO_BANK0);

            aqpWriteIO(IO_BANK0, 19);
            for (int i = 0; i < 16384; i++)
                buf.push_back(aqpReadMem(i));

            // Restore state
            aqpWriteIO(IO_BANK0, bank0);
            aqpReleaseBus();
        }

        // Check contents
        bool hasData = false;
        for (int i = 0; i < 16384; i++) {
            if (buf[i] != 0xFF) {
                hasData = true;
                break;
            }
        }
        if (!hasData) {
            buf.clear();
            menu.drawMessage("No cartridge found");
            vTaskDelay(pdMS_TO_TICKS(2000));
        } else {
            if (memcmp(buf.data(), buf.data() + 8192, 8192) == 0) {
                // 8KB cartridge
                buf.erase(buf.begin() + 8192, buf.end());
            }
        }

        if (!buf.empty()) {
            std::string fileName = "cart.rom";
            if (menu.editString("Enter filename for cartridge", fileName, 32)) {
                // Save cartridge contents to file
                auto vfs = getSDCardVFS();
                int  fd;
                if ((fd = vfs->open(FO_WRONLY | FO_CREATE, fileName.c_str())) >= 0) {
                    vfs->write(fd, buf.size(), buf.data());
                    vfs->close(fd);
                }
            }
        }
    }
#endif

    void addMainMenuItems(Menu &menu) override {
        {
            auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Reset CPU (CTRL-ESC)");
            item.onEnter = [this]() {
                resetCore();
            };
        }
        menu.items.emplace_back(MenuItemType::separator);
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        if (coreInfo.flags & FLAG_AQPLUS) {
            {
                auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Screenshot (text)");
                item.onEnter = [this, &menu]() { takeScreenshot(menu); };
            }
            {
                auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Dump cartridge");
                item.onEnter = [this, &menu]() { dumpCartridge(menu); };
            }
            menu.items.emplace_back(MenuItemType::separator);
        }
#endif
        if (coreInfo.flags & FLAG_FORCE_TURBO) {
            auto &item  = menu.items.emplace_back(MenuItemType::onOff, "Force turbo mode");
            item.setter = [this](int newVal) {
                forceTurbo = (newVal != 0);
                aqpForceTurbo(forceTurbo);
            };
            item.getter = [this]() { return forceTurbo ? 1 : 0; };
        }
        if (coreInfo.flags & FLAG_MOUSE_SUPPORT) {
            auto &item  = menu.items.emplace_back(MenuItemType::percentage, "Mouse sensitivity");
            item.setter = [this](int newVal) {
                newVal = std::max(1, std::min(newVal, 8));
                if (newVal != mouseSensitivityDiv) {
                    mouseSensitivityDiv = newVal;

                    nvs_handle_t h;
                    if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                        if (nvs_set_u8(h, "mouseDiv", mouseSensitivityDiv) == ESP_OK) {
                            nvs_commit(h);
                        }
                        nvs_close(h);
                    }
                }
            };
            item.getter = [this]() { return mouseSensitivityDiv; };
        }
        if (coreInfo.flags & FLAG_AQPLUS) {
            auto &item  = menu.items.emplace_back(MenuItemType::onOff, "Auto-bypass start screen");
            item.setter = [this](int newVal) {
                bypassStartScreen = (newVal != 0);

                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_u8(h, "bypassStart", bypassStartScreen ? 1 : 0) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            };
            item.getter = [this]() { return bypassStartScreen ? 1 : 0; };
        }
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        if (coreInfo.flags & FLAG_HAS_Z80) {
            auto &item  = menu.items.emplace_back(MenuItemType::onOff, "Use external Z80");
            item.setter = [this, &menu](int newVal) {
                bool newUseT80 = (newVal == 0);
                if (useT80 != newUseT80) {
                    useT80 = (newVal == 0);

                    nvs_handle_t h;
                    if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                        if (nvs_set_u8(h, "useT80", useT80 ? 1 : 0) == ESP_OK) {
                            nvs_commit(h);
                        }
                        nvs_close(h);
                    }

                    menu.drawMessage("Please reset CPU");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
            };
            item.getter = [this]() { return useT80 ? 0 : 1; };
        }
        if (coreInfo.flags & FLAG_VIDEO_TIMING) {
            auto &item   = menu.items.emplace_back(MenuItemType::subMenu, videoTimingMode ? "Video timing: 640x480" : "Video timing: 704x480");
            item.onEnter = [this, &menu]() {
                videoTimingMode = (videoTimingMode == 0) ? 1 : 0;
                aqpSetVideoMode(videoTimingMode);

                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_u8(h, "videoTiming", videoTimingMode) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }

                menu.setNeedsUpdate();
            };
        }
#endif
    }

    void getCoreInfo(CoreInfo *_coreInfo) override {
        *_coreInfo = coreInfo;
    }
};

std::shared_ptr<FpgaCore> newCoreAquariusPlus() {
    return std::make_shared<CoreAquariusPlus>();
}
