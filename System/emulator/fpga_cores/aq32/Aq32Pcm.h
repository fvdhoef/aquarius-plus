#pragma once

#include "Common.h"

class Aq32Pcm {
public:
    Aq32Pcm();
    void reset();
    void render(int16_t results[2]);

    uint16_t fifoIrqThreshold = 0;
    uint16_t rate             = 0;
    uint32_t phaseAccum       = 0;

    bool hasIrq() { return data.size() < fifoIrqThreshold; };

    std::deque<uint32_t> data;
    uint32_t             lastData = 0;
};
