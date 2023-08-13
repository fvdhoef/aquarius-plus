#pragma once

#include "Common.h"

class MemDump {
public:
    static void dumpCartridge();
    static void dumpScreen();
    static void dumpMemory();

private:
    static void saveMsgRam();
    static void restoreMsgRam();
    static void showMessage(const char *fmt, ...);

    static uint8_t savedRam3000[40];
    static uint8_t savedRam3400[40];

    static int screenshotIdx;
};
