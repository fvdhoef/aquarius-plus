// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#pragma once

#include "Common.h"

class AqKeyboard {
    AqKeyboard();

public:
    static AqKeyboard &instance();

    void init();
    void handleScancode(unsigned scanCode, bool keyDown);
#ifdef EMULATOR
    void pressKey(unsigned char ch, bool keyDown);
#else
    void pressKey(unsigned ch);
#endif
    void updateMatrix();

private:
#ifndef EMULATOR
    SemaphoreHandle_t mutex;
#endif

    uint8_t  pressedKeys[8]   = {0};
    uint16_t modifiers        = 0;
    bool     prevShiftPressed = false;
    unsigned handCtrl1Pressed = 0;
    uint8_t  prevMatrix[8]    = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t  keybMatrix[8]    = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t  prevHandCtrl1    = 0xFF;
    uint8_t  prevHandCtrl2    = 0xFF;
    uint8_t  handCtrl1        = 0xFF;
    uint8_t  handCtrl2        = 0xFF;
    uint8_t  ledStatus        = 0;
    bool     waitAllReleased  = false;

    void _keyDown(int key);
    void _keyUp(int key);
    void _keyDown(int key, bool shift);
    void handController(unsigned scanCode, bool keyDown);

    static const uint8_t scanCodeLut[];
};
