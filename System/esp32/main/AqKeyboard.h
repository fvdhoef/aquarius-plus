#pragma once

#include "common.h"

class AqKeyboard {
    AqKeyboard();

public:
    static AqKeyboard &instance();

    void init();
    void handleScancode(unsigned scancode, bool keydown);
    void pressKey(unsigned ch);
    void updateMatrix();

private:
    SemaphoreHandle_t mutex;

    uint8_t keybMatrix[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t handCtrl1     = 0xFF;
    uint8_t handCtrl2     = 0xFF;
    uint8_t ledStatus     = 0;

    void keyDown(int key);
    void keyUp(int key);
    void keyDown(int key, bool shift);
    void handController(unsigned scancode, bool keydown);

    static const uint8_t scanCodeLut[];
};