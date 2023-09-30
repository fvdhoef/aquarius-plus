#pragma once

#include "Common.h"
#include <map>

enum class KeyLayout {
    US = 0,
    UK = 1,
    Count,
};

void        setKeyLayout(KeyLayout layout);
KeyLayout   getKeyLayout();
std::string getKeyLayoutName(KeyLayout layout);

enum {
    NUM_LOCK    = (1 << 0),
    CAPS_LOCK   = (1 << 1),
    SCROLL_LOCK = (1 << 2),
};

class KeyboardLayout {
public:
    enum {
        LedNumLock    = (1 << 0),
        LedCapsLock   = (1 << 1),
        LedScrollLock = (1 << 2),
    };
    enum {
        ModLCtrl  = (1 << 0),
        ModLShift = (1 << 1),
        ModLAlt   = (1 << 2),
        ModLGui   = (1 << 3),
        ModRCtrl  = (1 << 4),
        ModRShift = (1 << 5),
        ModRAlt   = (1 << 6),
        ModRGui   = (1 << 7),
    };

    void processScancode(unsigned scanCode, bool keyDown);

    uint8_t layoutUS(unsigned scanCode);
    uint8_t layoutUK(unsigned scanCode);
    uint8_t compose(uint8_t first, uint8_t second);

    uint8_t modifiers    = 0;
    uint8_t leds         = 0;
    uint8_t composeFirst = 0;
};

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
    bool scrollLockOn() {
        return (ledStatus != 0xFF) && (ledStatus & SCROLL_LOCK);
    }
    void setScrollLock(bool value) {
        if (ledStatus == 0xFF)
            ledStatus = 0;
        ledStatus = (ledStatus & ~SCROLL_LOCK) | (value ? SCROLL_LOCK : 0);
    }

private:
#ifndef EMULATOR
    SemaphoreHandle_t mutex;
#endif

    KeyboardLayout kbLayout;

    std::map<unsigned, uint64_t> keyMaskMap;

    bool     dontSend         = false;
    unsigned handCtrl1Pressed = 0;
    uint64_t prevMatrix       = 0;
    uint64_t keybMatrix       = 0;
    uint8_t  prevHandCtrl1    = 0xFF;
    uint8_t  prevHandCtrl2    = 0xFF;
    uint8_t  handCtrl1        = 0xFF;
    uint8_t  handCtrl2        = 0xFF;
    uint8_t  ledStatus        = 0xFF;

    bool handController(unsigned scanCode, bool keyDown);

    static const uint8_t scanCodeLut[];
};
