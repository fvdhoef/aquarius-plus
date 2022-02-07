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

struct ay_ym_param {
    float r_up;
    float r_down;
    float res[16]; // Effective output resistance for different DAC values
};

static const struct ay_ym_param ay8910_param = {
    .r_up   = 800000,
    .r_down = 8000000,
    .res    = {
        15950, 15350, 15090, 14760,
        14275, 13620, 12890, 11370,
        10600, 8590, 7190, 5985,
        4820, 3945, 3017, 2345},
};

static void build_3d_table(struct ay8910 *ay, float r_load, bool normalize, float factor, bool zero_is_off) {
    float min = 10.0;
    float max = 0.0;

    for (int e = 0; e < 8; e++) {
        for (int j1 = 0; j1 < 16; j1++) {
            for (int j2 = 0; j2 < 16; j2++) {
                for (int j3 = 0; j3 < 16; j3++) {
                    float n;
                    if (zero_is_off) {
                        n = (j1 != 0 || (e & 0x01)) ? 1.0f : 0.0f;
                        n += (j2 != 0 || (e & 0x02)) ? 1.0f : 0.0f;
                        n += (j3 != 0 || (e & 0x04)) ? 1.0f : 0.0f;
                    } else {
                        n = 3.0f;
                    }

                    float rt = n / ay8910_param.r_up + 3.0f / ay8910_param.r_down + 1.0f / r_load;
                    float rw = n / ay8910_param.r_up;

                    rw += 1.0f / ay8910_param.res[j1];
                    rt += 1.0f / ay8910_param.res[j1];
                    rw += 1.0f / ay8910_param.res[j2];
                    rt += 1.0f / ay8910_param.res[j2];
                    rw += 1.0f / ay8910_param.res[j3];
                    rt += 1.0f / ay8910_param.res[j3];

                    int indx = (e << 12) | (j3 << 8) | (j2 << 4) | j1;

                    ay->output_amplitude_table[indx] = rw / rt;
                    if (ay->output_amplitude_table[indx] < min)
                        min = ay->output_amplitude_table[indx];
                    if (ay->output_amplitude_table[indx] > max)
                        max = ay->output_amplitude_table[indx];
                }
            }
        }
    }

    if (normalize) {
        for (unsigned j = 0; j < sizeof(ay->output_amplitude_table) / sizeof(ay->output_amplitude_table[0]); j++) {
            ay->output_amplitude_table[j] = ((ay->output_amplitude_table[j] - min) / (max - min)) * factor;
        }
    }
}

void ay8910_init(struct ay8910 *ay) {
    // The previous implementation added all three channels up instead of averaging them.
    // The factor of 3 will force the same levels if normalizing is used.
    const float res_load = 1000;
    build_3d_table(ay, res_load, true, 3, true);

    ay8910_reset(ay);
}

void ay8910_reset(struct ay8910 *ay) {
    ay->rng = 1;

    // Reset tone generator values
    for (int ch = 0; ch < 3; ch++) {
        ay->tone[ch].period     = 0;
        ay->tone[ch].volume     = 0;
        ay->tone[ch].count      = 0;
        ay->tone[ch].duty_cycle = 0;
        ay->tone[ch].output     = 0;
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
        case AY_AVOL: ay->tone[0].volume = ay->regs[AY_AVOL]; break;
        case AY_BVOL: ay->tone[1].volume = ay->regs[AY_BVOL]; break;
        case AY_CVOL: ay->tone[2].volume = ay->regs[AY_CVOL]; break;
        case AY_EACOARSE:
        case AY_EAFINE: ay->envelope.period = (ay->regs[AY_EACOARSE] << 8) | ay->regs[AY_EAFINE]; break;
        case AY_ENABLE: break;
        case AY_EASHAPE: {
            uint8_t       shape = ay->regs[AY_EASHAPE];
            const uint8_t mask  = 0x0F;
            ay->envelope.attack = (shape & 0x04) ? mask : 0x00;
            if ((shape & 0x08) == 0) {
                // if Continue = 0, map the shape to the equivalent one which has Continue = 1
                ay->envelope.hold      = true;
                ay->envelope.alternate = ay->envelope.attack != 0;
            } else {
                ay->envelope.hold      = (shape & 0x01) != 0;
                ay->envelope.alternate = (shape & 0x02) != 0;
            }
            ay->envelope.step    = mask;
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

float ay8910_render(struct ay8910 *ay) {
    for (int ch = 0; ch < 3; ch++) {
        struct tone *tone   = &ay->tone[ch];
        int          period = tone->period < 1 ? 1 : tone->period;
        tone->count += 1;
        while (tone->count >= period) {
            tone->duty_cycle = (tone->duty_cycle - 1) & 0x1F;
            tone->output     = tone->duty_cycle & 1;
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

    // Determine output amplitude
    unsigned table_idx = 0;
    for (int ch = 0; ch < 3; ch++) {
        unsigned mask   = 0;
        unsigned volume = 0;

        if (((ay->tone[ch].volume >> 4) & 1) != 0) {
            // Amplitude controlled by envelope generator
            mask   = (1 << (ch + 12));
            volume = ay->envelope.volume;

        } else {
            // Amplitude controlled by amplitude register
            volume = ay->tone[ch].volume & 0x0F;
        }
        table_idx |= mask | (ay->value[ch] ? volume << (ch * 4) : 0);
    }
    return ay->output_amplitude_table[table_idx];
}
