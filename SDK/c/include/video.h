#ifndef _VIDEO_H
#define _VIDEO_H

#include <stdint.h>

void video_wait_line(uint8_t linenr);
#define video_wait_eof() video_wait_line(216)

void sprite_init(uint8_t sprite_idx, uint16_t attr_idx);
void sprite_move(uint8_t sprite_idx, uint16_t x, uint8_t y);

#endif
