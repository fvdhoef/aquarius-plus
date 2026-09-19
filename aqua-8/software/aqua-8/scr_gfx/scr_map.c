#include "scr.h"

static void draw(void) {
    scr_common(5);

    int x, y;

    // Sprite overview
    {
        x = 200 - 128 - 3;
        y = 8;
        draw_rect(&(rect_t){x, y, x + 1 + 128, y + 1 + 128}, 0);
        fill_rect(&(rect_t){x + 1, y + 1, x + 128, y + 128}, 0);
        x += 1;
        y += 1;

        for (int i = 0; i < 256; i++) {
            int row = i / 16;
            int col = i % 16;

            draw_game_sprite(i, x + col * 8, y + row * 8);
        }
    }
}

screen_t scr_map = {
    .draw = draw,
};
