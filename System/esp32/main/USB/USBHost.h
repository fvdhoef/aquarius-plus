#pragma once

#include "Common.h"

class USBHost {
public:
    virtual void init()                        = 0;
    virtual void keyboardSetLeds(uint8_t leds) = 0;
};

USBHost *getUSBHost();
