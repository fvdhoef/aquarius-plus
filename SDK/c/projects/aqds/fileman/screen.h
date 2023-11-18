#pragma once

#include <aqplus.h>

extern uint8_t *text_p;
extern uint8_t  text_x;
extern uint8_t  text_y;
extern uint8_t  text_color;
extern uint8_t  text_ch;

void print_2digits(uint8_t val);
void print_4digits(uint16_t val);
void print_5digits(uint16_t val);

void scr_putchar(uint8_t ch);
void scr_puts(const char *s);
bool scr_puts_clip(const char *s, uint8_t max_len);
void scr_puts_pad(const char *s, uint8_t len);
void scr_pad_line(void);
void scr_skip(void);
void scr_to_xy(uint8_t x, uint8_t y);
void scr_set_color(uint8_t x, uint8_t y, uint8_t width, uint8_t color);
