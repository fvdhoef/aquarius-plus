// Based on MAME AY-3-8910 code, which has following license and copyright:
// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "ay8910.h"
#include <string.h>
#include <stdbool.h>

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

struct tone {
    uint32_t period;
    uint8_t  volume;
    int32_t  count;
    uint8_t  duty_cycle;
    uint8_t  output;
};

struct envelope {
    uint32_t period;
    uint32_t count;
    int8_t   step;
    uint8_t  volume;
    bool     hold;
    bool     alternate;
    uint8_t  attack;
    bool     holding;
};

struct ay8910 {
    uint8_t         regs[16];                                 // Registers
    struct tone     tone[3];                                  // Tone generator state
    struct envelope envelope;                                 // Envelope generator state
    uint8_t         prescale_noise;                           // Noise prescaler
    uint8_t         noise_cnt;                                // Noise period counter
    uint32_t        rng;                                      // RNG LFSR state
    uint8_t         value[3];                                 // Current channel value (either 0 or 1)
    float           output_amplitude_table[8 * 16 * 16 * 16]; // Table containing all possible output amplitudes
};

static struct ay8910 ay_state = {0};

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

static void build_3d_table(float r_load, bool normalize, float factor, bool zero_is_off) {
    float min = 10.0;
    float max = 0.0;

    for (int e = 0; e < 8; e++) {
        for (int j1 = 0; j1 < 16; j1++) {
            for (int j2 = 0; j2 < 16; j2++) {
                for (int j3 = 0; j3 < 16; j3++) {
                    float n;
                    if (zero_is_off) {
                        n = (j1 != 0 || (e & 0x01)) ? 1 : 0;
                        n += (j2 != 0 || (e & 0x02)) ? 1 : 0;
                        n += (j3 != 0 || (e & 0x04)) ? 1 : 0;
                    } else {
                        n = 3.0;
                    }

                    float rt = n / ay8910_param.r_up + 3.0 / ay8910_param.r_down + 1.0 / r_load;
                    float rw = n / ay8910_param.r_up;

                    rw += 1.0 / ay8910_param.res[j1];
                    rt += 1.0 / ay8910_param.res[j1];
                    rw += 1.0 / ay8910_param.res[j2];
                    rt += 1.0 / ay8910_param.res[j2];
                    rw += 1.0 / ay8910_param.res[j3];
                    rt += 1.0 / ay8910_param.res[j3];

                    int indx = (e << 12) | (j3 << 8) | (j2 << 4) | j1;

                    ay_state.output_amplitude_table[indx] = rw / rt;
                    if (ay_state.output_amplitude_table[indx] < min)
                        min = ay_state.output_amplitude_table[indx];
                    if (ay_state.output_amplitude_table[indx] > max)
                        max = ay_state.output_amplitude_table[indx];
                }
            }
        }
    }

    if (normalize) {
        for (unsigned j = 0; j < sizeof(ay_state.output_amplitude_table) / sizeof(ay_state.output_amplitude_table[0]); j++) {
            ay_state.output_amplitude_table[j] = ((ay_state.output_amplitude_table[j] - min) / (max - min)) * factor;
        }
    }
}

void ay8910_init(void) {
    // The previous implementation added all three channels up instead of averaging them.
    // The factor of 3 will force the same levels if normalizing is used.
    const float res_load = 1000;
    build_3d_table(res_load, true, 3, true);

    ay8910_reset();
}

void ay8910_reset(void) {
    ay_state.rng = 1;

    // Reset tone generator values
    for (int ch = 0; ch < 3; ch++) {
        ay_state.tone[ch].period     = 0;
        ay_state.tone[ch].volume     = 0;
        ay_state.tone[ch].count      = 0;
        ay_state.tone[ch].duty_cycle = 0;
        ay_state.tone[ch].output     = 0;
    }

    // Reset envelope generator values
    ay_state.envelope.period    = 0;
    ay_state.envelope.count     = 0;
    ay_state.envelope.step      = 0;
    ay_state.envelope.volume    = 0;
    ay_state.envelope.hold      = false;
    ay_state.envelope.alternate = false;
    ay_state.envelope.attack    = 0;
    ay_state.envelope.holding   = false;

    ay_state.noise_cnt      = 0;
    ay_state.prescale_noise = 0;
    for (int i = 0; i < AY_PORTA; i++)
        ay8910_write_reg(i, 0);
}

void ay8910_write_reg(uint8_t r, uint8_t v) {
    if (r > 15)
        return;
    ay_state.regs[r] = v;

    switch (r) {
        case AY_AFINE:
        case AY_ACOARSE: ay_state.tone[0].period = ((ay_state.regs[AY_ACOARSE] & 0xF) << 8) | ay_state.regs[AY_AFINE]; break;
        case AY_BFINE:
        case AY_BCOARSE: ay_state.tone[1].period = ((ay_state.regs[AY_BCOARSE] & 0xF) << 8) | ay_state.regs[AY_BFINE]; break;
        case AY_CFINE:
        case AY_CCOARSE: ay_state.tone[2].period = ((ay_state.regs[AY_CCOARSE] & 0xF) << 8) | ay_state.regs[AY_CFINE]; break;
        case AY_NOISEPER: break;
        case AY_AVOL: ay_state.tone[0].volume = ay_state.regs[AY_AVOL]; break;
        case AY_BVOL: ay_state.tone[1].volume = ay_state.regs[AY_BVOL]; break;
        case AY_CVOL: ay_state.tone[2].volume = ay_state.regs[AY_CVOL]; break;
        case AY_EACOARSE:
        case AY_EAFINE: ay_state.envelope.period = (ay_state.regs[AY_EACOARSE] << 8) | ay_state.regs[AY_EAFINE]; break;
        case AY_ENABLE: break;
        case AY_EASHAPE: {
            uint8_t       shape      = ay_state.regs[AY_EASHAPE];
            const uint8_t mask       = 0x0F;
            ay_state.envelope.attack = (shape & 0x04) ? mask : 0x00;
            if ((shape & 0x08) == 0) {
                // if Continue = 0, map the shape to the equivalent one which has Continue = 1
                ay_state.envelope.hold      = true;
                ay_state.envelope.alternate = ay_state.envelope.attack != 0;
            } else {
                ay_state.envelope.hold      = (shape & 0x01) != 0;
                ay_state.envelope.alternate = (shape & 0x02) != 0;
            }
            ay_state.envelope.step    = mask;
            ay_state.envelope.holding = false;
            ay_state.envelope.volume  = (ay_state.envelope.step ^ ay_state.envelope.attack);
            break;
        }
        case AY_PORTA: break;
        case AY_PORTB: break;
    }
}

uint8_t ay8910_read_reg(uint8_t r) {
    if (r > 15)
        return 0xFF;

    const uint8_t mask[0x10] = {0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0x1F, 0xFF, 0x1F, 0x1F, 0x1F, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF};
    return ay_state.regs[r] & mask[r];
}

float ay8910_render(void) {
    for (int ch = 0; ch < 3; ch++) {
        struct tone *tone   = &ay_state.tone[ch];
        int          period = tone->period < 1 ? 1 : tone->period;
        tone->count += 1;
        while (tone->count >= period) {
            tone->duty_cycle = (tone->duty_cycle - 1) & 0x1F;
            tone->output     = tone->duty_cycle & 1;
            tone->count -= period;
        }
    }

    if (++ay_state.noise_cnt >= (ay_state.regs[AY_NOISEPER] & 0x1F)) {
        // Toggle the prescaler output. Noise is no different to channels.
        ay_state.noise_cnt = 0;
        ay_state.prescale_noise ^= 1;

        if (!ay_state.prescale_noise) {
            // The Random Number Generator of the 8910 is a 17-bit LFSR.
            // The input to the shift register is bit0 XOR bit3
            // (bit0 is the output).
            ay_state.rng = (ay_state.rng >> 1) | (((ay_state.rng & 1) ^ ((ay_state.rng >> 3) & 1)) << 16);
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

        ay_state.value[ch] =
            (ay_state.tone[ch].output | ((ay_state.regs[AY_ENABLE] >> ch) & 1)) &
            ((ay_state.rng & 1) | ((ay_state.regs[AY_ENABLE] >> (3 + ch)) & 1));
    }

    // Update envelope
    if (!ay_state.envelope.holding) {
        if ((++ay_state.envelope.count) >= ay_state.envelope.period * 2) {
            ay_state.envelope.count = 0;
            ay_state.envelope.step--;

            // Check envelope current position
            if (ay_state.envelope.step < 0) {
                if (ay_state.envelope.hold) {
                    if (ay_state.envelope.alternate)
                        ay_state.envelope.attack ^= 0x0F;
                    ay_state.envelope.holding = true;
                    ay_state.envelope.step    = 0;

                } else {
                    // If envelope.count has looped an odd number of times (usually 1),
                    // invert the output.
                    if (ay_state.envelope.alternate && (ay_state.envelope.step & (0x0F + 1)))
                        ay_state.envelope.attack ^= 0x0F;

                    ay_state.envelope.step &= 0x0F;
                }
            }
        }
    }
    ay_state.envelope.volume = ay_state.envelope.step ^ ay_state.envelope.attack;

    // Determine output amplitude
    unsigned table_idx = 0;
    for (int ch = 0; ch < 3; ch++) {
        unsigned mask   = 0;
        unsigned volume = 0;

        if (((ay_state.tone[ch].volume >> 4) & 1) != 0) {
            // Amplitude controlled by envelope generator
            mask   = (1 << (ch + 12));
            volume = ay_state.envelope.volume;

        } else {
            // Amplitude controlled by amplitude register
            volume = ay_state.tone[ch].volume & 0x0F;
        }
        table_idx |= mask | (ay_state.value[ch] ? volume << (ch * 4) : 0);
    }
    return ay_state.output_amplitude_table[table_idx];
}
