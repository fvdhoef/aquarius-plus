#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "regs.h"
#include "file_io.h"

// #define PERFMON

static inline void wait_vsync(void) {
    // while (IO_VLINE < 216) {
    // }

    // Wait for line 216
    IO_VIRQLINE = 216;
    IO_IRQSTAT  = 2;
    while ((IO_IRQSTAT & 2) == 0) {
    }

    // while (IO_VSYNC & 1) {
    // }
    // while ((IO_VSYNC & 1) == 0) {
    // }
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

static inline void setup_ball_sprites(uint8_t ball_idx) {
    uint8_t base = ball_idx * 4;
    sprite_init(base++, 0x8000 | (128 + 227));
    sprite_init(base++, 0x8000 | (128 + 228));
    sprite_init(base++, 0x8000 | (128 + 243));
    sprite_init(base, 0x8000 | (128 + 244));
}

static inline void update_ball_sprites(uint8_t ball_idx) {
    struct ball *ballp = &balls[ball_idx];
    uint16_t     x     = ballp->x;
    uint16_t     x8    = ballp->x + 8;
    uint8_t      y     = ballp->y;
    uint8_t      y8    = ballp->y + 8;

    uint8_t base = ball_idx * 4;
    sprite_move(base++, x, y);
    sprite_move(base++, x8, y);
    sprite_move(base++, x, y8);
    sprite_move(base, x8, y8);
}

static inline void sonic_sprite(uint8_t frame, int x, int y) {
    uint8_t  base   = 32;
    uint16_t spridx = 128 + 256 + (uint16_t)frame * 16;
    for (uint8_t j = 0; j < 2; j++) {
        int tx = x;
        for (uint8_t i = 0; i < 3; i++) {
            sprite_init(base, 0x8800 | spridx);
            sprite_move(base, tx, y);
            base++;
            spridx += 2;
            tx += 8;
        }
        y += 16;
    }
}

static uint8_t get_joystick(void) {
    IO_PSG1ADDR = 14;
    return IO_PSG1DATA;
}

bool init(void) {
    // Map video RAM to $C000
    IO_BANK3 = 20;

    uint8_t palette[32];

    // Load in tile data
    printf("Loading tile data...\n");
    int8_t fd = open("tiledata.bin", FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    read(fd, (void *)0xC000, 0x4000);
    read(fd, palette, 32);
    close(fd);

    // Set palette
    for (uint8_t i = 0; i < 32; i++) {
        IO_VPALSEL  = i;
        IO_VPALDATA = palette[i];
    }
    return true;
}

int main(void) {
    puts("Tiledemo\n");
    if (!init()) {
        printf("error\n");
        return 1;
    }

    static uint8_t i;

    // Init balls
    for (i = 0; i < 8; i++) {
        struct ball *ballp = &balls[i];

        ballp->x  = rand() % (320 - 16);
        ballp->y  = rand() % (200 - 16);
        ballp->dx = rand() % 5 - 2;
        ballp->dy = rand() % 5 - 2;
        if (ballp->dx == 0)
            ballp->dx = 1;
        if (ballp->dy == 0)
            ballp->dy = 1;

        setup_ball_sprites(i);
        update_ball_sprites(i);
    }

    {
        uint8_t *tram = (uint8_t *)0x3000;
        uint8_t *cram = (uint8_t *)0x3400;

        // Clear screen
        for (int i = 0; i < 1000; i++) {
            tram[i] = ' ';
            cram[i] = 0x90;
        }

        // Put text at top of screen
        const char *str = "  Score:xxxxx    Lives:x    Time:xxx";
        const char *ps  = str;
        uint8_t    *pd  = tram + 1;
        while (*ps) {
            *(pd++) = *(ps++);
        }
    }

    // Manually patch the tilemap; give the palm tree priority
    {
        uint16_t *tilemap = (uint16_t *)0xC000;
        for (int j = 0; j < 5; j++) {
            for (int i = 0; i < 5; i++) {
                tilemap[64 * (8 + j) + (7 + i)] |= 0x4000;
            }
        }
        for (int j = 0; j < 5; j++) {
            tilemap[64 * (13 + j) + 9] |= 0x4000;
        }
    }

    // Set video mode
    IO_VCTRL = VCTRL_MODE_TILE | VCTRL_SPR_EN | VCTRL_TEXT_EN | VCTRL_TEXT_PRIO;

#ifdef PERFMON
    IO_VPALSEL     = 0;
    uint8_t col0_l = IO_VPALDATA;
    IO_VPALSEL     = 1;
    uint8_t col0_h = IO_VPALDATA;
#endif

    static uint16_t scroll_x   = 0;
    static uint8_t  anim_frame = 0;
    static uint8_t  anim_delay = 0;

    static int sonic_x;
    static int sonic_y;

    sonic_x = 160 - 12;
    sonic_y = 90;

    while (1) {
        wait_vsync();

#ifdef PERFMON
        // Background: blue
        IO_VPALSEL  = 0;
        IO_VPALDATA = col0_l;
        IO_VPALSEL  = 1;
        IO_VPALDATA = col0_h;
#endif

        // Scroll horizontally
        scroll_x++;
        IO_VSCRX_L = scroll_x & 0xFF;
        IO_VSCRX_H = scroll_x >> 8;

        sonic_sprite(anim_frame, sonic_x, sonic_y);

        anim_delay++;
        if (anim_delay >= 5) {
            anim_delay = 0;
            anim_frame++;
            if (anim_frame >= 6) {
                anim_frame = 0;
            }
        }

        // Move balls
        for (i = 0; i < 8; i++) {
            struct ball *ballp = &balls[i];

            ballp->x += ballp->dx;
            if (ballp->x >= 320 - 16)
                ballp->dx = -ballp->dx;
            ballp->y += ballp->dy;
            if (ballp->y >= 200 - 16)
                ballp->dy = -ballp->dy;

            update_ball_sprites(i);
        }

        {
            uint8_t joyval = ~get_joystick();
            if (joyval & (1 << 0))
                sonic_y++;
            if (joyval & (1 << 1))
                sonic_x++;
            if (joyval & (1 << 2))
                sonic_y--;
            if (joyval & (1 << 3))
                sonic_x--;
        }

#ifdef PERFMON
        // Background: black
        IO_VPALSEL  = 0;
        IO_VPALDATA = 0;
        IO_VPALSEL  = 1;
        IO_VPALDATA = 0;
#endif
    }

    // return 0;
}
