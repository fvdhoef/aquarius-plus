#ifndef _PT3PLAY_H
#define _PT3PLAY_H

#include <stdint.h>

struct ayregs {
    uint16_t tone_a;
    uint16_t tone_b;
    uint16_t tone_c;
    uint8_t  noise_period;
    uint8_t  mixer;
    uint8_t  ampl_a;
    uint8_t  ampl_b;
    uint8_t  ampl_c;
    uint16_t env_period;
    uint8_t  env_shape_cycle;
};

extern struct ayregs pt3play_ayregs;

void    pt3play_init(void *pt3);
uint8_t pt3play_play(void); // Non-zero return value at end-of-song (loop point)
void    pt3play_mute(void);

#endif
