#pragma once

#include <stdint.h>
#include <stdbool.h>

class AY8910 {
public:
    AY8910();
    void reset();

    void write(uint8_t addr, uint8_t data) {
        if (addr & 1) {
            regIdx = data & 0xF;
        } else {
            writeReg(regIdx, data);
        }
    }
    uint8_t read() { return readReg(regIdx); }

    void render(uint16_t abc[3]);

    void dbgDrawIoRegs();

    uint8_t portRdData[2] = {0xFF, 0xFF};

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

    uint8_t       regIdx = 0;
    uint8_t       regs[16];      // Registers
    ToneGenerator toneGen[3];    // Tone generator state
    Envelope      envelope;      // Envelope generator state
    uint8_t       prescaleNoise; // Noise prescaler
    uint8_t       noiseCnt;      // Noise period counter
    uint32_t      rng;           // RNG LFSR state
    uint8_t       value[3];      // Current channel value (either 0 or 1)

    void    writeReg(uint8_t r, uint8_t v);
    uint8_t readReg(uint8_t r);
};
