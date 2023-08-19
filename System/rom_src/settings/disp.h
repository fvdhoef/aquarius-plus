#ifndef _DISP_H
#define _DISP_H

#include <stdint.h>

void disp_set_bar_color(uint8_t col);
void disp_set_text_color(uint8_t col);
void disp_set_cursor_color(uint8_t col);
void disp_cursor_show(void);
void disp_cursor_hide(void);

void disp_clear(void);
void disp_putchar(char ch);
void disp_puts(const char *str);
void disp_set_cursor(int8_t row, int8_t col);

#endif
