#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "regs.h"
#include "file_io.h"

static inline void wait_vsync(void) {
    while (IO_VSYNC & 1) {
    }
    while ((IO_VSYNC & 1) == 0) {
    }
}

struct ball {
    uint16_t x;
    uint8_t  y;
    uint16_t dx;
    uint8_t  dy;
};

struct ball balls[16];

static inline void sprite_init(uint8_t sprite_idx, uint16_t attr_idx) {
    IO_VSPRSEL  = sprite_idx;
    IO_VSPRIDX  = attr_idx & 0xFF;
    IO_VSPRATTR = attr_idx >> 8;
}

static inline void sprite_move(uint8_t sprite_idx, uint16_t x, uint8_t y) {
    IO_VSPRSEL = sprite_idx;
    IO_VSPRX_L = x & 0xFF;
    IO_VSPRX_H = x >> 8;
    IO_VSPRY   = y;
}

void setup_ball_sprites(uint8_t ball_idx) {
    uint8_t base = ball_idx * 4;
    sprite_init(base + 0, 0x8000 | (128 + 227));
    sprite_init(base + 1, 0x8000 | (128 + 228));
    sprite_init(base + 2, 0x8000 | (128 + 243));
    sprite_init(base + 3, 0x8000 | (128 + 244));
}

void update_ball_sprites(uint8_t ball_idx) {
    uint8_t      base  = ball_idx * 4;
    struct ball *ballp = &balls[ball_idx];

    sprite_move(base + 0, ballp->x, ballp->y);
    sprite_move(base + 1, ballp->x + 8, ballp->y);
    sprite_move(base + 2, ballp->x, ballp->y + 8);
    sprite_move(base + 3, ballp->x + 8, ballp->y + 8);
}

int main(void) {
    puts("Tiledemo\n");

    // Map video RAM to $C000
    IO_BANK3 = 20;

    // Load in tile data
    printf("Loading tile data...");
    if (load_binary("tiledata.bin", (void *)0xC000, 0x4000)) {
        printf("done\n");
    } else {
        printf("error\n");
        return 1;
    }

    // Set palette
    for (uint8_t i = 0; i < 32; i++) {
        IO_VPALSEL  = i;
        IO_VPALDATA = *(uint8_t *)(0xF000 + i);
    }

    // Init balls
    for (uint8_t i = 0; i < 16; i++) {
        struct ball *ballp = &balls[i];

        ballp->x  = rand() % (320 - 16);
        ballp->y  = rand() % (200 - 16);
        ballp->dx = rand() % 5 - 2;
        ballp->dy = rand() % 5 - 2;

        setup_ball_sprites(i);
        update_ball_sprites(i);
    }

    // Set video mode
    IO_VCTRL = VCTRL_MODE_TILE | VCTRL_SPR_EN;

    static uint16_t scroll_x = 0;
    while (1) {
        wait_vsync();

        // Scroll horizontally
        scroll_x++;
        IO_VSCRX_L = scroll_x & 0xFF;
        IO_VSCRX_H = scroll_x >> 8;

        // Move balls
        for (uint8_t i = 0; i < 16; i++) {
            struct ball *ballp = &balls[i];

            ballp->x += ballp->dx;
            if (ballp->x >= 320 - 16)
                ballp->dx = -ballp->dx;
            ballp->y += ballp->dy;
            if (ballp->y >= 200 - 16)
                ballp->dy = -ballp->dy;

            update_ball_sprites(i);
        }
    }

    // return 0;
}
