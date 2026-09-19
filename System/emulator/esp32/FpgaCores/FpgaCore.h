#pragma once

#include "Common.h"
#include "DisplayOverlay/Menu.h"
#include "FPGA.h"

struct GamePadData {
    int8_t   lx, ly;
    int8_t   rx, ry;
    uint8_t  lt, rt;
    uint16_t buttons;
};

static_assert(sizeof(GamePadData) == 8);

enum class FpgaCoreType {
    AquariusPlus = 1,
    Aquarius32   = 2,
};

class FpgaCore {
public:
    virtual void resetCore() {}
    virtual bool keyScancode(uint8_t modifiers, unsigned scanCode, bool keyDown) { return false; }
    virtual void keyChar(uint8_t ch, bool isRepeat, uint8_t modifiers) {}
    virtual void mouseReport(int dx, int dy, uint8_t buttonMask, int dWheel, bool absPos = false) {}
    virtual void gamepadReport(unsigned idx, const GamePadData &data) {}
    virtual int  uartCommand(uint8_t cmd, const uint8_t *buf, size_t len) { return -1; }
    virtual void addMainMenuItems(Menu &menu)                    = 0;
    virtual bool getGamePadData(unsigned idx, GamePadData &data) = 0;

    static std::shared_ptr<FpgaCore> load(const void *data = nullptr, size_t length = 0);
    static std::shared_ptr<FpgaCore> loadAqPlus();
    static std::shared_ptr<FpgaCore> loadCore(const char *path);
    static void                      unload();
    static std::shared_ptr<FpgaCore> get();
    static const CoreInfo           *getCoreInfo();

private:
    static CoreInfo coreInfo;
};

// Not to be directly called
std::shared_ptr<FpgaCore> newCoreAquariusPlus();
std::shared_ptr<FpgaCore> newCoreAquarius32();
