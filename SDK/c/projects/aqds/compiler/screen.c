#include "screen.h"

#define TEXT_RAM ((uint8_t *)0xF000)

void scr_init(void) {
    IO_VCTRL = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    memset(TEXT_RAM, 0x74, 2047);
    *(TEXT_RAM + 2047) = 0;

    IO_VCTRL = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    memset(TEXT_RAM, ' ', 2048);
}

int putchar(int ch) {
    static uint8_t text_x;
    static uint8_t text_y;
    uint8_t       *text_p = TEXT_RAM + text_y * 80 + text_x;

    if (ch == '\n') {
        text_x = 0;
        text_y++;
    } else {
        *(text_p++) = (ch);
        text_x++;
        if (text_x == 80) {
            text_x = 0;
            text_y++;
        }
    }
    if (text_y == 25) {
        text_y = 24;
        memmove(TEXT_RAM, TEXT_RAM + 80, 24 * 80);
        memset(TEXT_RAM + 24 * 80, ' ', 80);
    }
    return 1;
}
