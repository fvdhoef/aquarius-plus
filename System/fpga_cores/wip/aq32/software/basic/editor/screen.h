#pragma once

#include "common.h"
#include "colors.h"

extern volatile uint16_t *p_scr;
extern uint16_t           color;

static inline void scr_locate(int row, int column) {
    p_scr = &TRAM->text[80 * row + column];
}

static inline void scr_putchar(uint8_t ch) {
    *(p_scr++) = color | ch;
}

static inline void scr_putcolor(uint8_t col) {
    ((uint8_t *)p_scr)[1] = col;
    p_scr++;
}

static inline void scr_fillchar(uint8_t ch, int count) {
    while (count > 0) {
        count--;
        *(p_scr++) = color | ch;
    }
}

static inline void scr_puttext(const char *p) {
    while (*p)
        scr_putchar(*(p++));
}

static inline void scr_putbuf(const uint8_t *p, int sz) {
    while (sz > 0) {
        sz--;
        scr_putchar(*(p++));
    }
}

static inline int scr_puttext_accel(const char *p, bool show_accel) {
    volatile uint16_t *p_old = p_scr;
    uint16_t           col   = color;

    while (*p) {
        color = col;
        if (p[0] == '&') {
            if (show_accel)
                color |= 0xF000;
            p++;
        }
        scr_putchar(*(p++));
    }
    return p_scr - p_old;
}

static inline void scr_setcolor(uint8_t col) {
    color = col << 8;
}

#define BORDER_FLAG_NO_SHADOW     (1 << 0)
#define BORDER_FLAG_NO_BOTTOM     (1 << 1)
#define BORDER_FLAG_TITLE_INVERSE (1 << 2)

void   scr_draw_border(int y, int x, int w, int h, uint8_t color, unsigned flags, const char *title);
void   scr_draw_separator(int y, int x, int w, uint8_t color);
size_t strlen_accel(const char *s);
void   scr_puttext_centered(int w, const char *text, bool has_accel);
void   scr_puttext_filled(int w, const char *text, bool has_accel, bool pad);

static inline void scr_center_text(int y, int x, int w, const char *text, bool has_accel) {
    scr_locate(y, x);
    scr_puttext_centered(w, text, has_accel);
}

void scr_status_msg(const char *s);
