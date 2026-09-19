#include "FpgaCore.h"

#include "Common.h"
#include "FPGA.h"
#include "KbHcEmu.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"

#include <nvs_flash.h>

class CoreAquarius32 : public FpgaCore {
public:
    SemaphoreHandle_t mutex;
    KbHcEmu           kbHcEmu;

    // Mouse state
    bool    mousePresent = false;
    float   mouseX       = 0;
    float   mouseY       = 0;
    uint8_t mouseButtons = 0;
    int     mouseWheel   = 0;

    uint8_t mouseSensitivityDiv = 4;

    CoreAquarius32() {
        mutex = xSemaphoreCreateRecursiveMutex();

        kbHcEmu.coreName         = getCoreInfo()->name;
        kbHcEmu.updateHandCtrl   = [this](uint8_t hctrl1, uint8_t hctrl2) { aqpUpdateHandCtrl(hctrl1, hctrl2); };
        kbHcEmu.updateKeybMatrix = [this](uint64_t val) { aqpUpdateKeybMatrix(val); };
        kbHcEmu.updateGamePad    = [this](unsigned idx, const GamePadData &data) { aqpUpdateKeybMatrix(idx, data); };

        UartProtocol::instance()->setBaudrate(25175000 / 6);
        loadSettings();
        Keyboard::instance()->reset(false);
    }

    virtual ~CoreAquarius32() {
        vSemaphoreDelete(mutex);
    }

    void loadSettings() {
        kbHcEmu.loadSettings();

        nvs_handle_t h;
        if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
            uint8_t mouseDiv = 0;
            if (nvs_get_u8(h, "mouseDiv", &mouseDiv) == ESP_OK) {
                mouseSensitivityDiv = mouseDiv;
            }
            nvs_close(h);
        }
        resetCore();
    }

    void aqpWriteKeybBuffer16(uint16_t data) {
        // | Bit | Description                  |
        // | --: | ---------------------------- |
        // |  14 | Scancode(1) / Character(0)   |
        // |  13 | Scancode key up(0) / down(1) |
        // |  12 | Repeated                     |
        // |  11 | Modifier: Gui                |
        // |  10 | Modifier: Alt                |
        // |   9 | Modifier: Shift              |
        // |   8 | Modifier: Ctrl               |
        // | 7:0 | Character / Scancode         |

        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_WRITE_KBBUF16, (uint8_t)(data & 0xFF), (uint8_t)(data >> 8)};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpUpdateKeybMatrix(uint64_t keybMatrix) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[9];
        cmd[0] = CMD_SET_KEYB_MATRIX;
        memcpy(&cmd[1], &keybMatrix, 8);
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpUpdateKeybMatrix(unsigned idx, const GamePadData &data) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[9];
        cmd[0] = (idx == 0) ? CMD_WRITE_GAMEPAD1 : CMD_WRITE_GAMEPAD2;
        memcpy(&cmd[1], &data, 8);
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpUpdateHandCtrl(uint8_t hctrl1, uint8_t hctrl2) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_SET_HCTRL, hctrl1, hctrl2};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void resetCore() override {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());

        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_RESET, 0};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    bool keyScancode(uint8_t modifiers, unsigned scanCode, bool keyDown) override {
        RecursiveMutexLock lock(mutex);
        if (kbHcEmu.keyScancode(modifiers, scanCode, keyDown))
            return true;

        if (scanCode <= 255) {
            aqpWriteKeybBuffer16(
                (1 << 14) | // Scancode
                (keyDown ? (1 << 13) : 0) |
                (((modifiers >> 4) | (modifiers & 0xF)) << 8) |
                scanCode);
        }

        // Special keys
        {
            uint8_t combinedModifiers = (modifiers & 0xF) | (modifiers >> 4);
            if (scanCode == SCANCODE_ESCAPE && keyDown) {
                if (combinedModifiers == ModLCtrl) {
                    resetCore();
                    return true;
                } else if (combinedModifiers == (ModLShift | ModLCtrl)) {
                    // CTRL-SHIFT-ESCAPE -> reset ESP32 (somewhat equivalent to power cycle)
                    SystemRestart();
                    return true;
                }
            }
        }
        return false;
    }

    void keyChar(uint8_t ch, bool isRepeat, uint8_t modifiers) override {
        RecursiveMutexLock lock(mutex);
        aqpWriteKeybBuffer16(
            (0 << 14) | // Character
            (isRepeat ? (1 << 12) : 0) |
            (((modifiers >> 4) | (modifiers & 0xF)) << 8) |
            ch);
    }

    void mouseReport(int dx, int dy, uint8_t buttonMask, int dWheel, bool absPos) override {
        RecursiveMutexLock lock(mutex);

        if (absPos) {
            if (dx < 0 || dy < 0)
                return;

            dy -= 32;
            dy /= 2;
            dx /= 2;
        }

        float sensitivity = 1.0f / (float)mouseSensitivityDiv;
        mouseX            = std::max(0.0f, std::min(319.0f, absPos ? dx : (mouseX + (float)(dx * sensitivity))));
        mouseY            = std::max(0.0f, std::min(199.0f, absPos ? dy : (mouseY + (float)(dy * sensitivity))));
        mouseButtons      = buttonMask;
        mousePresent      = true;
        mouseWheel        = mouseWheel + dWheel;
    }

    void gamepadReport(unsigned idx, const GamePadData &data) override {
        RecursiveMutexLock lock(mutex);
        kbHcEmu.gamepadReport(idx, data);
    }

    bool getGamePadData(unsigned idx, GamePadData &data) override {
        RecursiveMutexLock lock(mutex);
        return kbHcEmu.getGamePadData(idx, data);
    }

    void cmdGetMouse() {
        // DBGF("GETMOUSE()");

        auto up = UartProtocol::instance();
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

    int uartCommand(uint8_t cmd, const uint8_t *buf, size_t len) override {
        RecursiveMutexLock lock(mutex);
        switch (cmd) {
            case ESPCMD_GETMOUSE: {
                cmdGetMouse();
                return 1;
            }
            case ESPCMD_GETGAMECTRL: {
                if (len == 1) {
                    kbHcEmu.cmdGetGameCtrl(buf[0]);
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
            auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Reset CPU (CTRL-ESC)");
            item.onEnter = [this]() {
                resetCore();
            };
        }
        menu.items.emplace_back(MenuItemType::separator);
        kbHcEmu.addMainMenuItems(menu);
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
    }
};

std::shared_ptr<FpgaCore> newCoreAquarius32() {
    return std::make_shared<CoreAquarius32>();
}
