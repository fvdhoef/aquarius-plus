#include "disp.h"
#include <aqplus.h>

#define TRAM ((uint8_t *)0x3000)
#define CRAM ((uint8_t *)0x3400)

static uint8_t col_bar    = 0x06;
static uint8_t col_text   = 0x8F;
static uint8_t col_cursor = 0x07;

static bool     cursor_show = true;
static int8_t   cursor_row  = 0;
static int8_t   cursor_col  = 0;
static uint8_t *cursor_tp   = TRAM;
static uint8_t *cursor_cp   = CRAM;
static uint8_t  cursor_saved_col;
static bool     update_p = true;

void disp_set_bar_color(uint8_t col) {
    col_bar = col;
}
void disp_set_text_color(uint8_t col) {
    col_text = col;
}
void disp_set_cursor_color(uint8_t col) {
    col_cursor = col;
}

static void save_col_under_cursor(void) {
    uint8_t col      = *cursor_cp;
    cursor_saved_col = col;

    if (cursor_show) {
        *cursor_cp = col_cursor;
    }
}

static void restore_col_under_cursor(void) {
    *cursor_cp = cursor_saved_col;
}

void disp_cursor_show(void) {
    save_col_under_cursor();
    cursor_show = true;
    restore_col_under_cursor();
}
void disp_cursor_hide(void) {
    save_col_under_cursor();
    cursor_show = false;
    restore_col_under_cursor();
}

void disp_clear(void) {
    // Switch to 40-column mode
    IO_VCTRL = VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;

    for (int i = 0; i < 40; i++) {
        TRAM[i] = ' ';
        CRAM[i] = col_bar;
    }

    for (int i = 40; i < 1024; i++) {
        TRAM[i] = ' ';
        CRAM[i] = col_text;
    }

    cursor_col       = 0;
    cursor_row       = 1;
    cursor_tp        = TRAM + 40;
    cursor_cp        = CRAM + 40;
    cursor_saved_col = col_text;

    save_col_under_cursor();
}

static void disp_scroll_up(void) {
    memmove(TRAM + 40, TRAM + 80, 23 * 40);
    memset(TRAM + (24 * 40), ' ', 40);
}

static void check_cursor(void) {
    // if (!update_p && cursor_col >= 0 && cursor_col < 40 && cursor_row >= 0 && cursor_row < 25)
    //     return;

    if (cursor_col < 0) {
        cursor_col = 39;
        cursor_row--;
        update_p = true;
    }
    if (cursor_col >= 40) {
        cursor_col = 0;
        cursor_row++;
        update_p = true;
    }
    if (cursor_row < 0) {
        cursor_row = 0;
        update_p   = true;
    }
    while (cursor_row >= 25) {
        disp_scroll_up();
        cursor_row--;
        update_p = true;
    }

    if (update_p) {
        cursor_tp = &TRAM[cursor_row * 40 + cursor_col];
        cursor_cp = &CRAM[cursor_row * 40 + cursor_col];
        update_p  = false;
    }
}

static void _disp_putchar(char c) {
    if (c == '\r') {
        cursor_col = 0;
        update_p   = true;
    } else if (c == '\n') {
        cursor_row++;
        update_p = true;
    } else if (c == '\b') {
        cursor_col--;
        cursor_tp--;
        cursor_cp--;
    } else {
        cursor_col++;
        *(cursor_tp++) = c;
        cursor_cp++;
    }
    check_cursor();
}

void disp_putchar(char ch) {
    restore_col_under_cursor();
    _disp_putchar(ch);
    save_col_under_cursor();
}

void disp_puts(const char *str) {
    restore_col_under_cursor();

    char ch;
    while (ch = *(str++)) {
        _disp_putchar(ch);
    }

    save_col_under_cursor();
}

void disp_set_cursor(int8_t row, int8_t col) {
    restore_col_under_cursor();
    cursor_row = row;
    cursor_col = col;
    update_p   = true;
    check_cursor();
    save_col_under_cursor();
}
