#include "FpgaCore.h"

#include "Common.h"
#include "FPGA.h"
#include "KbHcEmu.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"

#include <nvs_flash.h>

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
    FLAG_HAS_Z80       = (1 << 0),
    FLAG_MOUSE_SUPPORT = (1 << 1),
    FLAG_VIDEO_TIMING  = (1 << 2),
    FLAG_AQPLUS        = (1 << 3),
    FLAG_FORCE_TURBO   = (1 << 4),
};

class CoreAquariusPlus : public FpgaCore {
public:
    SemaphoreHandle_t mutex;
    KbHcEmu           kbHcEmu;
    uint8_t           videoTimingMode   = 0;
    bool              useT80            = false;
    bool              forceTurbo        = false;
    bool              bypassStartScreen = false;
    TimerHandle_t     bypassStartTimer  = nullptr;
    bool              bypassStartCancel = false;
    bool              warmReset         = false;
    uint8_t           keyMode           = 3;

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

        kbHcEmu.coreName         = getCoreInfo()->name;
        kbHcEmu.updateHandCtrl   = [this](uint8_t hctrl1, uint8_t hctrl2) { aqpUpdateHandCtrl(hctrl1, hctrl2); };
        kbHcEmu.updateKeybMatrix = [this](uint64_t val) { aqpUpdateKeybMatrix(val); };

        UartProtocol::instance()->setBaudrate(3579545);
        loadSettings();
        Keyboard::instance()->reset(true);
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
            keyChar('\r', false, 0);
    }

    void loadSettings() {
        auto coreInfo = getCoreInfo();

        if ((coreInfo->flags & FLAG_HAS_Z80) == 0) {
            useT80 = true;
        }

        kbHcEmu.loadSettings();

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

            if (coreInfo->flags & FLAG_HAS_Z80) {
                if (nvs_get_u8(h, "useT80", &val8) == ESP_OK) {
                    useT80 = val8 != 0;
                }
            }
#endif

            if (nvs_get_u8(h, "bypassStart", &val8) == ESP_OK) {
                bypassStartScreen = val8 != 0;
            }
            if (nvs_get_u8(h, "forceTurbo", &val8) == ESP_OK) {
                forceTurbo = val8 != 0;
                aqpForceTurbo(forceTurbo);
            }
            nvs_close(h);
        }
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        if (coreInfo->flags & FLAG_VIDEO_TIMING)
            aqpSetVideoMode(videoTimingMode);
#endif
        resetCore();
    }

    void aqpWriteKeybBuffer(uint8_t ch) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_WRITE_KBBUF, ch};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void resetCore() override {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t resetCfg = 0;
        if (useT80)
            resetCfg |= 1;
        if (!warmReset)
            resetCfg |= 2;

        uint8_t cmd[] = {CMD_RESET, resetCfg};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);

        warmReset = true;

        {
            RecursiveMutexLock lock(mutex);
            bypassStartCancel = false;

            if (bypassStartScreen) {
                xTimerReset(bypassStartTimer, pdMS_TO_TICKS(CONFIG_BYPASS_START_TIME_MS));
            }
        }
    }

    void aqpForceTurbo(bool en) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cfg   = en ? 1 : 0;
        uint8_t cmd[] = {CMD_FORCE_TURBO, cfg};
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

    void aqpUpdateHandCtrl(uint8_t hctrl1, uint8_t hctrl2) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_SET_HCTRL, hctrl1, hctrl2};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpSetVideoMode(uint8_t mode) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_SET_VIDMODE, mode};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

#ifdef CONFIG_MACHINE_TYPE_AQPLUS
    void aqpAqcuireBus() {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_BUS_ACQUIRE};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpReleaseBus() {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_BUS_RELEASE};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    void aqpWriteMem(uint16_t addr, uint8_t data) {
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_MEM_WRITE, (uint8_t)(addr & 0xFF), (uint8_t)(addr >> 8), data};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    uint8_t aqpReadMem(uint16_t addr) {
        auto               fpga = FPGA::instance();
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
        auto               fpga = FPGA::instance();
        RecursiveMutexLock lock(fpga->getMutex());
        fpga->spiSel(true);
        uint8_t cmd[] = {CMD_IO_WRITE, (uint8_t)(addr & 0xFF), (uint8_t)(addr >> 8), data};
        fpga->spiTx(cmd, sizeof(cmd));
        fpga->spiSel(false);
    }

    uint8_t aqpReadIO(uint16_t addr) {
        auto               fpga = FPGA::instance();
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

    bool keyScancode(uint8_t modifiers, unsigned scanCode, bool keyDown) override {
        RecursiveMutexLock lock(mutex);
        if (kbHcEmu.keyScancode(modifiers, scanCode, keyDown))
            return true;

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
        if ((keyMode & 4) == 0 && isRepeat)
            return;
        bypassStartCancel = true;
        aqpWriteKeybBuffer(ch);
    }

    void mouseReport(int dx, int dy, uint8_t buttonMask, int dWheel, bool absPos) override {
        RecursiveMutexLock lock(mutex);

        if (absPos) {
            if (dx < 0 || dy < 0)
                return;

            dy -= 32;
            if (!videoTimingMode)
                dx -= 32;

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

    void cmdReset() {
        keyMode = 3;
    }

    void cmdKeyMode(uint8_t mode) {
        // DBGF("KEYMODE(mode=0x%02X)", mode);
        keyMode = mode;

        auto up = UartProtocol::instance();
        up->txStart();
        up->txWrite(0);
    }

    int uartCommand(uint8_t cmd, const uint8_t *buf, size_t len) override {
        RecursiveMutexLock lock(mutex);
        switch (cmd) {
            case ESPCMD_RESET: {
                cmdReset();
                return 1;
            }
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
            case ESPCMD_KEYMODE: {
                if (len == 1) {
                    cmdKeyMode(buf[0]);
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
        auto               fpga = FPGA::instance();
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
        auto               fpga = FPGA::instance();
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
        auto coreInfo = getCoreInfo();
        {
            auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Reset CPU (CTRL-ESC)");
            item.onEnter = [this]() {
                resetCore();
            };
        }
        menu.items.emplace_back(MenuItemType::separator);
        kbHcEmu.addMainMenuItems(menu);
        menu.items.emplace_back(MenuItemType::separator);
#ifdef CONFIG_MACHINE_TYPE_AQPLUS
        if (coreInfo->flags & FLAG_AQPLUS) {
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
        if (coreInfo->flags & FLAG_FORCE_TURBO) {
            auto &item  = menu.items.emplace_back(MenuItemType::onOff, "Force turbo mode");
            item.setter = [this](int newVal) {
                forceTurbo = (newVal != 0);
                aqpForceTurbo(forceTurbo);

                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_u8(h, "forceTurbo", forceTurbo ? 1 : 0) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            };
            item.getter = [this]() { return forceTurbo ? 1 : 0; };
        }
        if (coreInfo->flags & FLAG_MOUSE_SUPPORT) {
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
        if (coreInfo->flags & FLAG_AQPLUS) {
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
        if (coreInfo->flags & FLAG_HAS_Z80) {
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
        if (coreInfo->flags & FLAG_VIDEO_TIMING) {
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
};

std::shared_ptr<FpgaCore> newCoreAquariusPlus() {
    return std::make_shared<CoreAquariusPlus>();
}
