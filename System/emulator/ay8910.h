#pragma once

#include <stdint.h>
#include <stdbool.h>

struct tone {
    uint16_t period;
    uint8_t  volume;
    int32_t  count;
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

void    ay8910_reset(struct ay8910 *ay);
void    ay8910_write_reg(struct ay8910 *ay, uint8_t r, uint8_t v);
uint8_t ay8910_read_reg(struct ay8910 *ay, uint8_t r);
void    ay8910_render(struct ay8910 *ay, uint16_t abc[3]);
