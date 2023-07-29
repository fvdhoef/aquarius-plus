// Based on MAME AY-3-8910 code, which has following license and copyright:
// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "AY8910.h"
#include <string.h>

enum {
    AY_AFINE    = 0x00,
    AY_ACOARSE  = 0x01,
    AY_BFINE    = 0x02,
    AY_BCOARSE  = 0x03,
    AY_CFINE    = 0x04,
    AY_CCOARSE  = 0x05,
    AY_NOISEPER = 0x06,
    AY_ENABLE   = 0x07,
    AY_AVOL     = 0x08,
    AY_BVOL     = 0x09,
    AY_CVOL     = 0x0A,
    AY_EAFINE   = 0x0B,
    AY_EACOARSE = 0x0C,
    AY_EASHAPE  = 0x0D,
    AY_PORTA    = 0x0E,
    AY_PORTB    = 0x0F,
};

static const uint16_t dacLevels[16] = {0, 6, 9, 13, 19, 27, 39, 56, 80, 116, 166, 239, 344, 495, 712, 1023};

AY8910::AY8910() {
    reset();
}

void AY8910::reset() {
    rng = 1;

    // Reset tone generator values
    for (int ch = 0; ch < 3; ch++) {
        toneGen[ch].period = 0;
        toneGen[ch].volume = 0;
        toneGen[ch].count  = 0;
        toneGen[ch].output = 0;
    }

    // Reset envelope generator values
    envelope.period    = 0;
    envelope.count     = 0;
    envelope.step      = 0;
    envelope.volume    = 0;
    envelope.hold      = false;
    envelope.alternate = false;
    envelope.attack    = 0;
    envelope.holding   = false;

    noiseCnt      = 0;
    prescaleNoise = 0;
    for (int i = 0; i < AY_PORTA; i++)
        writeReg(i, 0);
}

void AY8910::writeReg(uint8_t r, uint8_t v) {
    if (r > 15)
        return;
    regs[r] = v;

    switch (r) {
        case AY_AFINE:
        case AY_ACOARSE: toneGen[0].period = ((regs[AY_ACOARSE] & 0xF) << 8) | regs[AY_AFINE]; break;
        case AY_BFINE:
        case AY_BCOARSE: toneGen[1].period = ((regs[AY_BCOARSE] & 0xF) << 8) | regs[AY_BFINE]; break;
        case AY_CFINE:
        case AY_CCOARSE: toneGen[2].period = ((regs[AY_CCOARSE] & 0xF) << 8) | regs[AY_CFINE]; break;
        case AY_NOISEPER: break;
        case AY_ENABLE: break;
        case AY_AVOL: toneGen[0].volume = regs[AY_AVOL]; break;
        case AY_BVOL: toneGen[1].volume = regs[AY_BVOL]; break;
        case AY_CVOL: toneGen[2].volume = regs[AY_CVOL]; break;
        case AY_EAFINE:
        case AY_EACOARSE: envelope.period = (regs[AY_EACOARSE] << 8) | regs[AY_EAFINE]; break;
        case AY_EASHAPE: {
            uint8_t shape   = regs[AY_EASHAPE];
            envelope.attack = (shape & 0x04) ? 0x0F : 0x00;
            if ((shape & 0x08) == 0) {
                // if Continue = 0, map the shape to the equivalent one which has Continue = 1
                envelope.hold      = true;
                envelope.alternate = envelope.attack != 0;
            } else {
                envelope.hold      = (shape & 0x01) != 0;
                envelope.alternate = (shape & 0x02) != 0;
            }
            envelope.step    = 0x0F;
            envelope.holding = false;
            envelope.volume  = (envelope.step ^ envelope.attack);
            break;
        }
        case AY_PORTA: break;
        case AY_PORTB: break;
    }
}

uint8_t AY8910::readReg(uint8_t r) {
    if (r > 15)
        return 0xFF;

    static const uint8_t mask[0x10] = {0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0x1F, 0xFF, 0x1F, 0x1F, 0x1F, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF};
    return regs[r] & mask[r];
}

void AY8910::render(uint16_t abc[3]) {
    for (int ch = 0; ch < 3; ch++) {
        auto     tone   = &toneGen[ch];
        uint16_t period = tone->period < 1 ? 1 : tone->period;
        tone->count++;
        while (tone->count >= period) {
            tone->output ^= 1;
            tone->count -= period;
        }
    }

    if (++noiseCnt >= (regs[AY_NOISEPER] & 0x1F)) {
        // Toggle the prescaler output. Noise is no different to channels.
        noiseCnt = 0;
        prescaleNoise ^= 1;

        if (!prescaleNoise) {
            // The Random Number Generator of the 8910 is a 17-bit LFSR.
            // The input to the shift register is bit0 XOR bit3
            // (bit0 is the output).
            rng = (rng >> 1) | (((rng & 1) ^ ((rng >> 3) & 1)) << 16);
        }
    }

    // Update channel values
    for (int ch = 0; ch < 3; ch++) {
        // The 8910 has three outputs, each output is the mix of one of the three
        // tone generators and of the (single) noise generator. The two are mixed
        // BEFORE going into the DAC. The formula to mix each channel is:
        // (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable).
        // Note that this means that if both tone and noise are disabled, the
        // output is 1, not 0, and can be modulated changing the volume.

        value[ch] =
            (toneGen[ch].output | ((regs[AY_ENABLE] >> ch) & 1)) &
            ((rng & 1) | ((regs[AY_ENABLE] >> (3 + ch)) & 1));
    }

    // Update envelope
    if (!envelope.holding) {
        if ((++envelope.count) >= envelope.period * 2) {
            envelope.count = 0;
            envelope.step--;

            // Check envelope current position
            if (envelope.step < 0) {
                if (envelope.hold) {
                    if (envelope.alternate)
                        envelope.attack ^= 0x0F;
                    envelope.holding = true;
                    envelope.step    = 0;

                } else {
                    // If envelope.count has looped an odd number of times (usually 1),
                    // invert the output.
                    if (envelope.alternate && (envelope.step & (0x0F + 1)))
                        envelope.attack ^= 0x0F;

                    envelope.step &= 0x0F;
                }
            }
        }
    }
    envelope.volume = envelope.step ^ envelope.attack;

    for (int ch = 0; ch < 3; ch++) {
        unsigned volume;
        if (((toneGen[ch].volume >> 4) & 1) != 0) {
            // Amplitude controlled by envelope generator
            volume = envelope.volume & 0xF;
        } else {
            // Amplitude controlled by amplitude register
            volume = toneGen[ch].volume & 0xF;
        }
        abc[ch] = dacLevels[value[ch] ? volume : 0];
    }
}
