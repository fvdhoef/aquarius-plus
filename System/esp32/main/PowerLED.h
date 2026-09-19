#pragma once

#include "Common.h"

class PowerLED {
public:
    virtual void init()       = 0;
    virtual void flashStart() = 0;
    virtual void flashStop()  = 0;
};

PowerLED *getPowerLED();
