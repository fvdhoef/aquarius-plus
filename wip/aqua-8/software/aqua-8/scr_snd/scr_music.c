#include "scr.h"

static void draw(void) {
    scr_common(5);
}

screen_t scr_music = {
    .draw = draw,
};
