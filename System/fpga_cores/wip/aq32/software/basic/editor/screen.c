#include "screen.h"

volatile uint16_t *p_scr = TRAM->text;
uint16_t           color;

void scr_draw_border(int y, int x, int w, int h, uint8_t border_color, unsigned flags, const char *title) {
    w -= 2;
    h -= 2;

    bool shadow = (flags & BORDER_FLAG_NO_SHADOW) == 0;

    // Draw top border
    scr_locate(y, x);
    scr_setcolor(border_color);
    scr_putchar(16);
    if (title) {
        int title_w = strlen(title);
        int w2      = (w - (title_w + 2)) / 2;
        scr_fillchar(25, w2);

        if (flags & BORDER_FLAG_TITLE_INVERSE)
            color = ((color & 0xF000) >> 4) | ((color & 0x0F00) << 4);
        scr_putchar(' ');
        scr_puttext(title);
        scr_putchar(' ');

        scr_setcolor(border_color);

        w2 = w - (title_w + 2) - w2;
        scr_fillchar(25, w2);

    } else {
        for (int i = 0; i < w; i++)
            scr_putchar(25);
    }
    scr_putchar(18);
    y++;

    // Draw sides
    for (int j = 0; j < h; j++) {
        scr_setcolor(border_color);
        scr_locate(y, x);
        scr_putchar(26);

        scr_locate(y, x + 2 + w - 1);
        scr_putchar(26);

        if (shadow) {
            scr_setcolor(0);
            scr_putchar(' ');
            scr_putchar(' ');
        }

        y++;
    }

    if (flags & BORDER_FLAG_NO_BOTTOM)
        return;

    // Draw bottom border
    scr_locate(y, x);
    scr_setcolor(border_color);
    scr_putchar(22);
    for (int i = 0; i < w; i++)
        scr_putchar(25);
    scr_putchar(24);

    if (shadow) {
        scr_setcolor(0);
        scr_putchar(' ');
        scr_putchar(' ');
    }
    y++;

    if (shadow) {
        scr_locate(y, x + 2);
        scr_fillchar(' ', w + 2);
    }
}

void scr_draw_separator(int y, int x, int w, uint8_t color) {
    scr_setcolor(color);

    scr_locate(y, x);
    scr_putchar(19);
    scr_fillchar(25, w - 2);
    scr_putchar(21);
}

size_t strlen_accel(const char *s) {
    size_t w = 0;
    while (*s) {
        if (s[0] != '&')
            w++;
        s++;
    }
    return w;
}

void scr_puttext_centered(int w, const char *text, bool has_accel) {
    int text_len = has_accel ? strlen_accel(text) : strlen(text);
    int l        = (w - text_len) / 2;

    scr_fillchar(' ', l);
    if (has_accel)
        scr_puttext_accel(text, true);
    else
        scr_puttext(text);
    scr_fillchar(' ', w - l - text_len);
}

void scr_puttext_filled(int w, const char *text, bool has_accel, bool pad) {
    int text_len = 0;
    if (pad)
        scr_putchar(' ');

    if (text) {
        text_len = has_accel ? strlen_accel(text) : strlen(text);
        if (has_accel)
            scr_puttext_accel(text, true);
        else
            scr_puttext(text);
    }
    scr_fillchar(' ', w - (pad ? 1 : 0) - text_len);
}

void scr_status_msg(const char *s) {
    scr_setcolor(COLOR_STATUS);
    scr_locate(TEXT_ROWS - 1, 0);
    scr_puttext_filled(68, s, false, true);
}
