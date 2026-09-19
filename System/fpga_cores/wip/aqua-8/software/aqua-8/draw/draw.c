#include "draw.h"
#include "state.h"

static const uint8_t font[760] = {
#include "font.inl"
};
static const uint8_t altfont[760] = {
#include "altfont.inl"
};
static const uint8_t icons[760] = {
#include "icons.inl"
};
static const uint32_t sprites[] = {
#include "sprites.inl"
};
static const uint16_t palette[16] = {
    0x000, 0x125, 0x725, 0x085, 0xA53, 0x555, 0xBBB, 0xFFE,
    0xF04, 0xFA0, 0xFF2, 0x0E5, 0x2AF, 0x879, 0xF7A, 0xFCA};

void palette_init(void) {
    for (int i = 0; i < 16; i++)
        VIDEO->PALETTE[i] = palette[i];
}

void remap_reset(void) {
    for (int i = 0; i < 16; i++)
        VIDEO->REMAP[i] = i;
    VIDEO->REMAP_T = 0x0;
}

void clear_screen(unsigned color) {
    VIDEO->COLOR = color;
    VIDEO->FLAGS = 2;
    for (unsigned j = 0; j < 161; j++) {
        VIDEO->POSX16 = 0;
        VIDEO->POSY16 = j;
        for (unsigned i = 0; i < 25; i++) {
            VIDEO->WR1BPP = 0xFF;
        }
    }
}

void draw_sprite(unsigned spr, int x, int y) {
    const uint32_t *p = &sprites[(spr >> 4) * 128 + (spr & 15)];

    VIDEO->REMAP_T = 1;
    VIDEO->FLAGS   = 1;
    VIDEO->POSX16  = x;
    VIDEO->POSY16  = y;
    VIDEO->WR4BPP  = p[0];
    VIDEO->WR4BPP  = p[16];
    VIDEO->WR4BPP  = p[32];
    VIDEO->WR4BPP  = p[48];
    VIDEO->WR4BPP  = p[64];
    VIDEO->WR4BPP  = p[80];
    VIDEO->WR4BPP  = p[96];
    VIDEO->WR4BPP  = p[112];
    VIDEO->REMAP_T = 0;
}

void draw_game_sprite(unsigned spr, int x, int y) {
    const uint32_t *p = &data_state.sprites[(spr >> 4) * 128 + (spr & 15)];

    VIDEO->REMAP_T = 1;
    VIDEO->FLAGS   = 1;
    VIDEO->POSX16  = x;
    VIDEO->POSY16  = y;
    VIDEO->WR4BPP  = p[0];
    VIDEO->WR4BPP  = p[16];
    VIDEO->WR4BPP  = p[32];
    VIDEO->WR4BPP  = p[48];
    VIDEO->WR4BPP  = p[64];
    VIDEO->WR4BPP  = p[80];
    VIDEO->WR4BPP  = p[96];
    VIDEO->WR4BPP  = p[112];
    VIDEO->REMAP_T = 0;
}

void draw_pixel(int x, int y, unsigned color) {
    VIDEO->POSX16 = x;
    VIDEO->POSY16 = y;
    VIDEO->COLOR  = color;
    VIDEO->WR1BPP = 1;
}

int draw_char(int x, int y, uint8_t ch, unsigned color) {
    if (ch <= 32 || ch > 127)
        return 6;
    ch -= 32;

    const uint8_t *p = &font[ch * 8];

    VIDEO->COLOR  = color;
    VIDEO->FLAGS  = 1;
    VIDEO->POSX16 = x;
    VIDEO->POSY16 = y;
    VIDEO->WR1BPP = p[0];
    VIDEO->WR1BPP = p[1];
    VIDEO->WR1BPP = p[2];
    VIDEO->WR1BPP = p[3];
    VIDEO->WR1BPP = p[4];
    VIDEO->WR1BPP = p[5];
    return 6;
}

int draw_char_altfont(int x, int y, uint8_t ch, unsigned color) {
    if (ch <= 32 || ch > 127)
        return 4;
    ch -= 32;

    const uint8_t *p = &altfont[ch * 8];

    VIDEO->COLOR  = color;
    VIDEO->FLAGS  = 1;
    VIDEO->POSX16 = x;
    VIDEO->POSY16 = y;
    VIDEO->WR1BPP = p[0];
    VIDEO->WR1BPP = p[1];
    VIDEO->WR1BPP = p[2];
    VIDEO->WR1BPP = p[3];
    VIDEO->WR1BPP = p[4];
    VIDEO->WR1BPP = p[5];
    return 4;
}

void draw_icon(int x, int y, unsigned icon, unsigned color) {
    if (icon >= 96)
        return;

    const uint8_t *p = &icons[icon * 8];

    VIDEO->COLOR  = color;
    VIDEO->FLAGS  = 1;
    VIDEO->POSX16 = x;
    VIDEO->POSY16 = y;
    VIDEO->WR1BPP = p[0];
    VIDEO->WR1BPP = p[1];
    VIDEO->WR1BPP = p[2];
    VIDEO->WR1BPP = p[3];
    VIDEO->WR1BPP = p[4];
    VIDEO->WR1BPP = p[5];
    VIDEO->WR1BPP = p[6];
    VIDEO->WR1BPP = p[7];
}

int draw_text(const char *str, int x, int y, unsigned color, bool use_altfont) {
    if (use_altfont) {
        while (*str) {
            x += draw_char_altfont(x, y, *(str++), color);
        }
    } else {
        while (*str) {
            x += draw_char(x, y, *(str++), color);
        }
    }
    return x;
}

void draw_hline(int x, int y, int w, unsigned col) {
    VIDEO->COLOR  = col;
    VIDEO->POSX16 = x;
    VIDEO->POSY16 = y;
    VIDEO->FLAGS  = 2;

    while (w > 8) {
        VIDEO->WR1BPP = 0xFF;
        w -= 8;
    }
    if (w > 0) {
        VIDEO->WR1BPP = ((1 << w) - 1);
    }
}

void draw_vline(int x, int y, int h, unsigned col) {
    VIDEO->COLOR  = col;
    VIDEO->POSX16 = x;
    VIDEO->POSY16 = y;
    VIDEO->FLAGS  = 1;

    while (h-- > 0)
        VIDEO->WR1BPP = 1;
}

void draw_rect(const rect_t *r, unsigned col) {
    int x0, y0, x1, y1;
    if (r->x0 < r->x1) {
        x0 = r->x0;
        x1 = r->x1;
    } else {
        x0 = r->x1;
        x1 = r->x0;
    }
    if (r->y0 < r->y1) {
        y0 = r->y0;
        y1 = r->y1;
    } else {
        y0 = r->y1;
        y1 = r->y0;
    }

    int w = (x1 - x0) + 1;
    int h = (y1 - y0) + 1;

    draw_hline(x0, y0, w, col);
    draw_hline(x0, y1, w, col);
    draw_vline(x0, y0, h, col);
    draw_vline(x1, y0, h, col);
}

void fill_rect(const rect_t *r, unsigned col) {
    int x0, y0, x1, y1;
    if (r->x0 < r->x1) {
        x0 = r->x0;
        x1 = r->x1;
    } else {
        x0 = r->x1;
        x1 = r->x0;
    }
    if (r->y0 < r->y1) {
        y0 = r->y0;
        y1 = r->y1;
    } else {
        y0 = r->y1;
        y1 = r->y0;
    }

    int w = (x1 - x0) + 1;
    int h = (y1 - y0) + 1;

    while (h-- > 0) {
        draw_hline(x0, y0, w, col);
        y0++;
    }
}
