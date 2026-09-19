#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void console_init(void);
bool console_set_width(int width);

void console_clear_screen(void);
void console_show_cursor(bool show);

void console_set_cursor_row(int val);
int  console_get_cursor_row(void);
int  console_get_num_rows(void);

void console_set_cursor_column(int val);
int  console_get_cursor_column(void);
int  console_get_num_columns(void);

void console_set_foreground_color(int val);
void console_set_background_color(int val);

void console_putc(char ch);
void console_puts(const char *s);

uint8_t console_getc(void);

void console_flush_input(void);
void _console_handle_key(int key);

#ifdef __cplusplus
}
#endif
