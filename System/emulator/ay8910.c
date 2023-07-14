// Based on MAME AY-3-8910 code, which has following license and copyright:
// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "ay8910.h"
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

static const uint16_t dac_levels[16] = {0, 6, 9, 13, 19, 27, 39, 56, 80, 116, 166, 239, 344, 495, 712, 1024};

void ay8910_reset(struct ay8910 *ay) {
    ay->rng = 1;

    // Reset tone generator values
    for (int ch = 0; ch < 3; ch++) {
        ay->tone[ch].period = 0;
        ay->tone[ch].volume = 0;
        ay->tone[ch].count  = 0;
        ay->tone[ch].output = 0;
    }

    // Reset envelope generator values
    ay->envelope.period    = 0;
    ay->envelope.count     = 0;
    ay->envelope.step      = 0;
    ay->envelope.volume    = 0;
    ay->envelope.hold      = false;
    ay->envelope.alternate = false;
    ay->envelope.attack    = 0;
    ay->envelope.holding   = false;

    ay->noise_cnt      = 0;
    ay->prescale_noise = 0;
    for (int i = 0; i < AY_PORTA; i++)
        ay8910_write_reg(ay, i, 0);
}

void ay8910_write_reg(struct ay8910 *ay, uint8_t r, uint8_t v) {
    if (r > 15)
        return;
    ay->regs[r] = v;

    switch (r) {
        case AY_AFINE:
        case AY_ACOARSE: ay->tone[0].period = ((ay->regs[AY_ACOARSE] & 0xF) << 8) | ay->regs[AY_AFINE]; break;
        case AY_BFINE:
        case AY_BCOARSE: ay->tone[1].period = ((ay->regs[AY_BCOARSE] & 0xF) << 8) | ay->regs[AY_BFINE]; break;
        case AY_CFINE:
        case AY_CCOARSE: ay->tone[2].period = ((ay->regs[AY_CCOARSE] & 0xF) << 8) | ay->regs[AY_CFINE]; break;
        case AY_NOISEPER: break;
        case AY_ENABLE: break;
        case AY_AVOL: ay->tone[0].volume = ay->regs[AY_AVOL]; break;
        case AY_BVOL: ay->tone[1].volume = ay->regs[AY_BVOL]; break;
        case AY_CVOL: ay->tone[2].volume = ay->regs[AY_CVOL]; break;
        case AY_EAFINE:
        case AY_EACOARSE: ay->envelope.period = (ay->regs[AY_EACOARSE] << 8) | ay->regs[AY_EAFINE]; break;
        case AY_EASHAPE: {
            uint8_t       shape = ay->regs[AY_EASHAPE];
            ay->envelope.attack = (shape & 0x04) ? 0x0F : 0x00;
            if ((shape & 0x08) == 0) {
                // if Continue = 0, map the shape to the equivalent one which has Continue = 1
                ay->envelope.hold      = true;
                ay->envelope.alternate = ay->envelope.attack != 0;
            } else {
                ay->envelope.hold      = (shape & 0x01) != 0;
                ay->envelope.alternate = (shape & 0x02) != 0;
            }
            ay->envelope.step    = 0x0F;
            ay->envelope.holding = false;
            ay->envelope.volume  = (ay->envelope.step ^ ay->envelope.attack);
            break;
        }
        case AY_PORTA: break;
        case AY_PORTB: break;
    }
}

uint8_t ay8910_read_reg(struct ay8910 *ay, uint8_t r) {
    if (r > 15)
        return 0xFF;

    const uint8_t mask[0x10] = {0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0x1F, 0xFF, 0x1F, 0x1F, 0x1F, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF};
    return ay->regs[r] & mask[r];
}

void ay8910_render(struct ay8910 *ay, uint16_t abc[3]) {
    for (int ch = 0; ch < 3; ch++) {
        struct tone *tone   = &ay->tone[ch];
        uint16_t     period = tone->period < 1 ? 1 : tone->period;
        tone->count++;
        while (tone->count >= period) {
            tone->output ^= 1;
            tone->count -= period;
        }
    }

    if (++ay->noise_cnt >= (ay->regs[AY_NOISEPER] & 0x1F)) {
        // Toggle the prescaler output. Noise is no different to channels.
        ay->noise_cnt = 0;
        ay->prescale_noise ^= 1;

        if (!ay->prescale_noise) {
            // The Random Number Generator of the 8910 is a 17-bit LFSR.
            // The input to the shift register is bit0 XOR bit3
            // (bit0 is the output).
            ay->rng = (ay->rng >> 1) | (((ay->rng & 1) ^ ((ay->rng >> 3) & 1)) << 16);
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

        ay->value[ch] =
            (ay->tone[ch].output | ((ay->regs[AY_ENABLE] >> ch) & 1)) &
            ((ay->rng & 1) | ((ay->regs[AY_ENABLE] >> (3 + ch)) & 1));
    }

    // Update envelope
    if (!ay->envelope.holding) {
        if ((++ay->envelope.count) >= ay->envelope.period * 2) {
            ay->envelope.count = 0;
            ay->envelope.step--;

            // Check envelope current position
            if (ay->envelope.step < 0) {
                if (ay->envelope.hold) {
                    if (ay->envelope.alternate)
                        ay->envelope.attack ^= 0x0F;
                    ay->envelope.holding = true;
                    ay->envelope.step    = 0;

                } else {
                    // If envelope.count has looped an odd number of times (usually 1),
                    // invert the output.
                    if (ay->envelope.alternate && (ay->envelope.step & (0x0F + 1)))
                        ay->envelope.attack ^= 0x0F;

                    ay->envelope.step &= 0x0F;
                }
            }
        }
    }
    ay->envelope.volume = ay->envelope.step ^ ay->envelope.attack;

    for (int ch = 0; ch < 3; ch++) {
        unsigned volume;
        if (((ay->tone[ch].volume >> 4) & 1) != 0) {
            // Amplitude controlled by envelope generator
            volume = ay->envelope.volume & 0xF;
        } else {
            // Amplitude controlled by amplitude register
            volume = ay->tone[ch].volume & 0xF;
        }
        abc[ch] = dac_levels[ay->value[ch] ? volume : 0];
    }
}
