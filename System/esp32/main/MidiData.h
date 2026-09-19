#pragma once

#include "Common.h"

class MidiData {
public:
    static MidiData *instance();

    virtual bool     getData(uint8_t buf[4])       = 0;
    virtual unsigned getDataCount()                = 0;
    virtual void     addData(const uint8_t buf[4]) = 0;
};
