#pragma once

#include "Common.h"
#include "FpgaCore.h"
#include "DisplayOverlay/DisplayOverlay.h"
#include "DisplayOverlay/KeyboardHandCtrlMappingMenu.h"
#include "DisplayOverlay/GamepadHandCtrlMappingMenu.h"
#include "DisplayOverlay/GamepadKeyboardMappingMenu.h"

class KbHcEmu {
public:
    std::string                                                coreName;
    std::function<void(uint8_t hctrl1, uint8_t hctrl2)>        updateHandCtrl;
    std::function<void(uint64_t val)>                          updateKeybMatrix;
    std::function<void(unsigned idx, const GamePadData &data)> updateGamePad;

    KbHcEmu();
    void loadSettings();
    bool keyScancode(uint8_t modifiers, unsigned scanCode, bool keyDown);
    void gamepadReport(unsigned idx, const GamePadData &data);
    bool getGamePadData(unsigned idx, GamePadData &data);
    void cmdGetGameCtrl(uint8_t idx);
    void addMainMenuItems(Menu &menu);

private:
    GamePadData  gamePads[2];
    uint64_t     prevMatrix           = 0;
    uint64_t     keybMatrix           = 0;
    bool         gamepadNavigation    = false;
    unsigned     keybHandCtrl1Pressed = 0;
    uint8_t      keybHandCtrl1        = 0xFF;
    uint8_t      gamePadHandCtrl[2]   = {0xFF, 0xFF};
    Kb2HcMapping kb2hcSettings; // Keyboard to hand controller mapping
    Gp2HcMapping gp2hcSettings; // Gamepad to hand controller mapping
    Gp2KbMapping gp2kbSettings; // Gamepad to keyboard mapping

    bool        handControllerEmulate(unsigned scanCode, bool keyDown);
    void        gameCtrlUpdated();
    std::string getPresetPath(std::string presetType);
    void        savePreset(Menu &menu, std::string presetType, const void *buf, size_t size);
    void        loadPreset(std::string presetType, void *buf, size_t size);
};
