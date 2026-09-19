#pragma once

#include <stdint.h>
#include <stdbool.h>

class SN76489 {
public:
    SN76489();
    void     reset();
    void     write(uint8_t data);
    uint16_t render();

private:
    uint8_t  latchedCh;
    uint16_t chFreqDiv[4];
    uint8_t  chAtten[4];
    bool     noiseFb;
    bool     noiseUseCh3Freq;

    uint16_t noiseLfsr;
    uint16_t chCnt[4];
    bool     chVal[4];
    uint8_t  clkDiv;
};
