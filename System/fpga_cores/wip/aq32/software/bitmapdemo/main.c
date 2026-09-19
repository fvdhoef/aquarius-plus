#include "common.h"
#include "esp.h"
#include "csr.h"
#include "console.h"

#include "keen_data.h"

static void wait_frame(void) {
    while ((csr_read_clear(mip, (1 << 16)) & (1 << 16)) == 0);
}

#define WRAP

int main(void) {
    memcpy((void *)VRAM4BPP, keen_data, keen_data_len);

    console_set_width(40);
    console_show_cursor(false);
    console_set_background_color(0);
    console_set_foreground_color(9);
    console_clear_screen();

    GFX->CTRL =
        GFX_CTRL_TEXT_PRIO | GFX_CTRL_TEXT_EN |
        GFX_CTRL_GFX_EN |
#ifdef WRAP
        GFX_CTRL_BM_WRAP |
#endif
        0;

    while (1) {
        wait_frame();

        int scrx = GFX->SCRX1;
        int scry = GFX->SCRY1;
        {
            uint8_t joyval = ~(HANDCTRL & 0xFF);
            if (joyval & (1 << 0)) {
                scry++;
            }
            if (joyval & (1 << 1)) {
                scrx++;
            }
            if (joyval & (1 << 2)) {
                scry--;
            }
            if (joyval & (1 << 3)) {
                scrx--;
            }
        }

#ifdef WRAP
        if (scrx < 0)
            scrx += 320;
        if (scrx >= 320)
            scrx -= 320;

        if (scry < 0)
            scry += 200;
        if (scry >= 200)
            scry -= 200;
#endif

        GFX->SCRX1 = scrx;
        GFX->SCRY1 = scry;

        console_set_cursor_row(0);
        console_set_cursor_column(0);
        printf("%3u %3u", (unsigned)GFX->SCRX1, (unsigned)GFX->SCRY1);
    }

    return 0;
}
