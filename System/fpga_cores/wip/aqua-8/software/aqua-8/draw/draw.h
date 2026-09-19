#pragma once

#include "common.h"

static inline void swapint(int *a, int *b) {
    int c = *a;
    *a    = *b;
    *b    = c;
}

static inline void wait_frame(void) {
    while ((csr_read_clear(mip, (1 << 16)) & (1 << 16)) == 0);
}

typedef struct {
    int x0, y0;
    int x1, y1;
} rect_t;

static inline bool rect_contains(const rect_t *r, int x, int y) {
    return x >= r->x0 && x <= r->x1 && y >= r->y0 && y <= r->y1;
}

static inline void rect_shrink(rect_t *r, int amount) {
    r->x0 += amount;
    r->y0 += amount;
    r->x1 -= amount;
    r->y1 -= amount;
}

void palette_init(void);
void remap_reset(void);
void clear_screen(unsigned color);

void draw_sprite(unsigned spr, int x, int y);
void draw_game_sprite(unsigned spr, int x, int y);

void draw_pixel(int x, int y, unsigned color);
int  draw_char(int x, int y, uint8_t ch, unsigned color);
int  draw_char_altfont(int x, int y, uint8_t ch, unsigned color);
int  draw_text(const char *str, int x, int y, unsigned color, bool use_altfont);
void draw_hline(int x, int y, int w, unsigned col);
void draw_vline(int x, int y, int h, unsigned col);
void draw_rect(const rect_t *r, unsigned col);
void fill_rect(const rect_t *r, unsigned col);
void draw_icon(int x, int y, unsigned icon, unsigned color);
