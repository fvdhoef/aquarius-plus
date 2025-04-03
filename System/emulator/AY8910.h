#pragma once

#include <stdint.h>
#include <stdbool.h>

class AY8910 {
public:
    AY8910();
    void    reset();
    void    writeReg(uint8_t r, uint8_t v);
    uint8_t readReg(uint8_t r);
    void    render(uint16_t abc[3]);

private:
    struct ToneGenerator {
        uint16_t period;
        uint8_t  volume;
        int32_t  count;
        uint8_t  output;
    };

    struct Envelope {
        uint32_t period;
        uint32_t count;
        int8_t   step;
        uint8_t  volume;
        bool     hold;
        bool     alternate;
        uint8_t  attack;
        bool     holding;
    };

    uint8_t       regs[16];      // Registers
    ToneGenerator toneGen[3];    // Tone generator state
    Envelope      envelope;      // Envelope generator state
    uint8_t       prescaleNoise; // Noise prescaler
    uint8_t       noiseCnt;      // Noise period counter
    uint32_t      rng;           // RNG LFSR state
    uint8_t       value[3];      // Current channel value (either 0 or 1)

    friend class UIInt;
};
