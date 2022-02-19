#include "video.h"
#include "emustate.h"

extern unsigned char charrom_bin[2048]; // Character ROM contents

// Original Aquarius palette
// static const uint32_t palette[16] = {
//     0x101010, 0xf71010, 0x10f710, 0xf7ef10,
//     0x2121de, 0xf710f7, 0x31c6c6, 0xf7f7f7,
//     0xc6c6c6, 0x29adad, 0xc621c6, 0x42108c,
//     0xf7f773, 0x21ce42, 0xad2121, 0x313131};

// 2-Bit Aquarius palette
// optimized for a better color range.
// static const uint32_t palette[16] = {
//     0x000000, 0xff0000, 0x55ff55, 0xffaa00,
//     0x0000ff, 0xff00ff, 0x55aaff, 0xffffff,
//     0xaaaaaa, 0x0055aa, 0xaa00aa, 0x000055,
//     0xffff55, 0x55aa55, 0xaa0000, 0x555555};

// 2-Bit Aquarius palette
// optimized to match STOCK color.
static const uint32_t palette[16] = {
    0x000000, 0xff0000, 0x55ff55, 0xffff00,
    0x0000ff, 0xff00ff, 0x00ffff, 0xffffff,
    0xaaaaaa, 0x00aaaa, 0xaa00aa, 0x0000aa,
    0xffffaa, 0x55aa55, 0xaa0000, 0x555555};

static inline void draw_char(void *pixels, int pitch, uint8_t ch, uint8_t color, int row, int column) {
    uint32_t       fgcol = palette[color >> 4];
    uint32_t       bgcol = palette[color & 0xF];
    const uint8_t *ps    = &charrom_bin[ch * 8];

    for (int j = 0; j < 8; j++) {
        uint32_t *pd = &((uint32_t *)((uintptr_t)pixels + (row * 8 + j) * pitch))[column * 8];
        for (int i = 0; i < 8; i++) {
            pd[i] = (ps[j] & (1 << (7 - i))) ? fgcol : bgcol;
        }
    }
}

static void draw_screen(void *pixels, int pitch) {
    uint8_t border_ch    = emustate.ram[0];
    uint8_t border_color = emustate.ram[0x400];

    for (int row = 0; row < 28; row++) {
        for (int column = 0; column < 44; column++) {
            if (row >= 2 && row < 26 && column >= 2 && column < 42)
                continue;

            draw_char(pixels, pitch, border_ch, border_color, row, column);
        }
    }

    for (int row = 0; row < 24; row++) {
        for (int column = 0; column < 40; column++) {
            uint8_t ch    = emustate.ram[(row + 1) * 40 + column];
            uint8_t color = emustate.ram[0x400 + (row + 1) * 40 + column];
            draw_char(pixels, pitch, ch, color, row + 2, column + 2);
        }
    }
}

void render_screen(SDL_Renderer *renderer) {
    static SDL_Texture *texture = NULL;
    if (texture == NULL) {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 352, 224);
    }

    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    draw_screen(pixels, pitch);
    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);

    // Determine target size
    {
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        // Retain aspect ratio
        int w1 = w / 352 * 352;
        int h1 = w1 * 224 / 352;
        int h2 = h / 224 * 224;
        int w2 = h2 * 352 / 224;

        int sw, sh;
        if (w1 == 0 || h1 == 0) {
            sw = w;
            sh = h;
        } else if (w1 <= w && h1 <= h) {
            sw = w1;
            sh = h1;
        } else {
            sw = w2;
            sh = h2;
        }

        SDL_Rect dst;
        dst.w = (int)sw;
        dst.h = (int)sh;
        dst.x = (w - dst.w) / 2;
        dst.y = (h - dst.h) / 2;
        SDL_RenderCopy(renderer, texture, NULL, &dst);
    }
    SDL_RenderPresent(renderer);
}
