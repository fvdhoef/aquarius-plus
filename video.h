#pragma once

#include "common.h"
#include "SDL.h"

#define VIDEO_WIDTH (352)
#define VIDEO_HEIGHT (224)

const uint8_t *video_get_fb(void);
void           video_draw_line(int line);
void           draw_screen(void);