#include "console.h"

static int cursor_column = 0;

void console_init(void) {
}
bool console_set_width(int width) {
    if (width != 40 && width != 80)
        return false;
    return true;
}
void console_clear_screen(void) {}
void console_show_cursor(bool) {}
int  console_get_cursor_row(void) { return 0; }
int  console_get_cursor_column(void) { return cursor_column; }
int  console_get_num_columns(void) { return 80; }
int  console_get_num_rows(void) { return 25; }

void console_set_cursor_row(int) {}
void console_set_cursor_column(int) {}
void console_set_foreground_color(int) {}
void console_set_background_color(int) {}

void console_putc(char ch) {
    putchar(ch);
    if (ch == '\n')
        cursor_column = 0;
    else {
        cursor_column++;
        if (cursor_column == 80) {
            cursor_column = 0;
            putchar('\n');
        }
    }
}

void console_puts(const char *s) {
    while (*s)
        console_putc(*(s++));
}

uint8_t console_getc(void) {
    int ch = getchar();
    return ch < 0 ? 0 : ch;
}
