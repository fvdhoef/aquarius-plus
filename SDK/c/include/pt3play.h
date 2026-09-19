#ifndef _PT3PLAY_H
#define _PT3PLAY_H

#include <stdint.h>
#include <stdbool.h>

struct ayregs {
    volatile uint16_t tone_a;
    volatile uint16_t tone_b;
    volatile uint16_t tone_c;
    volatile uint8_t  noise_period;
    volatile uint8_t  mixer;
    volatile uint8_t  ampl_a;
    volatile uint8_t  ampl_b;
    volatile uint8_t  ampl_c;
    volatile uint16_t env_period;
    volatile uint8_t  env_shape_cycle;
};

extern volatile struct ayregs pt3play_ayregs;

void pt3play_init(void *pt3);
bool pt3play_play(void); // Returns true at end-of-song (loop point)
void pt3play_mute(void);

#endif
