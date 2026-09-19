#include "SN76489.h"
#include <stdio.h>

SN76489::SN76489() {
    reset();
}

void SN76489::reset() {
    latchedCh = 0;
    for (int i = 0; i < 4; i++) {
        chFreqDiv[i] = 0;
        chAtten[i]   = 15;
    }

    noiseFb         = false;
    noiseUseCh3Freq = false;

    noiseLfsr = 0x4000;
    for (int i = 0; i < 4; i++) {
        chCnt[i] = 0;
        chVal[i] = false;
    }
    clkDiv = 0;
}

void SN76489::write(uint8_t data) {
    if (data & 0x80) {
        latchedCh = (data >> 5) & 3;

        unsigned ch = (data >> 5) & 3;

        if ((data & 0x90) == 0x90) {
            chAtten[ch] = data & 0xF;
        } else if ((data & 0x90) == 0x80) {
            if (ch < 3) {
                chFreqDiv[ch] = (chFreqDiv[ch] & 0x3F0) | (data & 0xF);
            } else {
                noiseFb         = (data & 4) != 0;
                noiseUseCh3Freq = false;

                switch (data & 3) {
                    case 0: chFreqDiv[ch] = 0x10; break;
                    case 1: chFreqDiv[ch] = 0x20; break;
                    case 2: chFreqDiv[ch] = 0x40; break;
                    default: noiseUseCh3Freq = true; break;
                }
            }
        }

    } else {
        if (latchedCh < 3) {
            chFreqDiv[latchedCh] = ((data & 0x3F) << 4) | (chFreqDiv[latchedCh] & 0xF);
        }
    }
}

uint16_t SN76489::render() {
    uint16_t result = 0;
    for (int i = 0; i < 4; i++) {
        if (chCnt[i] == 0) {
            if (i < 3) {
                chCnt[i] = chFreqDiv[i];
                chVal[i] = !chVal[i];
            } else {
                bool bit0 = (noiseLfsr & 1) != 0;

                chCnt[i]  = noiseUseCh3Freq ? chFreqDiv[2] : chFreqDiv[i];
                chVal[i]  = bit0;
                noiseLfsr = (noiseLfsr >> 1) ^ (bit0 ? (noiseFb ? 0xF037 : 0x8000) : 0);
            }
        } else {
            chCnt[i]--;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (chVal[i]) {
            switch (chAtten[i]) {
                case 0: result += 1023; break;
                case 1: result += 813; break;
                case 2: result += 646; break;
                case 3: result += 513; break;
                case 4: result += 407; break;
                case 5: result += 323; break;
                case 6: result += 257; break;
                case 7: result += 205; break;
                case 8: result += 162; break;
                case 9: result += 128; break;
                case 10: result += 102; break;
                case 11: result += 81; break;
                case 12: result += 64; break;
                case 13: result += 51; break;
                case 14: result += 40; break;
                case 15: result += 0; break;
            }
        }
    }
    return result;
}
