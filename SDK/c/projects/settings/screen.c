#include "screen.h"

#define TEXT_RAM ((uint8_t *)0x3000)

uint8_t *text_p;
uint8_t  text_x;
uint8_t  text_y;
uint8_t  text_color;
uint8_t  text_ch;

void scr_init(void) {
    IO_VCTRL           = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    *(TEXT_RAM + 2047) = 0;
}

void scr_putchar(uint8_t ch) {
    IO_VCTRL    = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    *text_p     = (ch);
    IO_VCTRL    = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    *(text_p++) = text_color;

    if (text_x == 79) {
        text_x = 0;
        text_y++;
    } else {
        text_x++;
    }
}

void scr_puts(const char *s) {
    while (*s) {
        scr_putchar(*(s++));
    }
}

bool scr_puts_clip(const char *s, uint8_t max_len) {
    while (*s && max_len) {
        scr_putchar(*(s++));
        max_len--;
    }
    return *s != NULL;
}

void scr_puts_pad(const char *s, uint8_t len) {
    while (*s && len) {
        scr_putchar(*(s++));
        len--;
    }
    while (len) {
        scr_putchar(' ');
        len--;
    }
}

void scr_pad_line(void) {
    while (text_x) {
        scr_putchar(' ');
    }
}

void scr_skip(void) {
    text_p += 40;
    text_x += 40;
}

void scr_to_xy(uint8_t x, uint8_t y) {
    text_x = x;
    text_y = y;
    text_p = TEXT_RAM + y * 80 + x;
}

void scr_set_color(uint8_t x, uint8_t y, uint8_t width, uint8_t color) {
    scr_to_xy(x, y);
    IO_VCTRL = VCTRL_TEXTPAGE2 | VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    while (width--) {
        *(text_p++) = color;
    }
}
