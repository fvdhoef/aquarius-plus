#include "CoreAquariusPlus.h"
#include "FPGA.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"
#include "GameCtrl.h"
#include <math.h>
#include <nvs_flash.h>

#include "CoreAquariusPlus.h"
#include "AqKeyboardDefs.h"

enum {
    // Aq+ command
    CMD_RESET           = 0x01,
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

class CoreAquariusPlus : public FpgaCore {
public:
    SemaphoreHandle_t mutex;
    uint64_t          prevMatrix      = 0;
    uint64_t          keybMatrix      = 0;
    GamePadData       gamepads[2]     = {0};
    uint8_t           videoTimingMode = 0;

    // Mouse state
    bool    mousePresent = false;
    float   mouseX       = 0;
    float   mouseY       = 0;
    uint8_t mouseButtons = 0;
    int     mouseWheel   = 0;

    uint8_t mouseSensitivityDiv = 4;

    CoreAquariusPlus() {
        mutex = xSemaphoreCreateRecursiveMutex();
    }

    virtual ~CoreAquariusPlus() {
        vSemaphoreDelete(mutex);
    }

    bool loadBitstream(const void *data, size_t length) override {
        if (data == nullptr) {
            extern const uint8_t fpgaImageStart[] asm("_binary_aqp_top_bit_start");
            extern const uint8_t fpgaImageEnd[] asm("_binary_aqp_top_bit_end");
            data   = fpgaImageStart;
            length = fpgaImageEnd - fpgaImageStart;
        }
        bool result = getFPGA()->loadBitstream(data, length);
        if (result) {
            applySettings();
        }
        return result;
    }

    void applySettings() {
        nvs_handle_t h;
        if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
            if (nvs_get_u8(h, "videoTiming", &videoTimingMode) != ESP_OK) {
                videoTimingMode = 0;
            }

            uint8_t mouseDiv = 0;
            if (nvs_get_u8(h, "mouseDiv", &mouseDiv) == ESP_OK) {
                mouseSensitivityDiv = mouseDiv;
            }

            nvs_close(h);
        }
        aqpSetVideoMode(videoTimingMode);
    }

    void aqpWriteKeybBuffer(uint8_t ch) {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_WRITE_KBBUF, ch};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpReset() {
        auto               fpga = getFPGA();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_RESET};
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
                    aqpReset();
                } else if (combinedModifiers == (ModLShift | ModLCtrl)) {
                    // // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                    // FPGA::instance().aqpAqcuireBus();
                    // FPGA::instance().aqpReset();
                    // esp_restart();
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

    void addMainMenuItems(Menu &menu) override {
        {
            auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Restart Aquarius+ (CTRL-ESC)");
            item.onEnter = [this]() {
                aqpReset();
            };
        }
        menu.items.emplace_back(MenuItemType::separator);
        {
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
        {
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
    }
};

std::shared_ptr<FpgaCore> newCoreAquariusPlus() {
    return std::make_shared<CoreAquariusPlus>();
}
