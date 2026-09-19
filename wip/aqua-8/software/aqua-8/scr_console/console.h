#pragma once

#include "common.h"

void    console_putc(char ch);
void    console_puts(const char *str);
void    console_putline(const char *str);
void    console_printf(const char *fmt, ...) __attribute__((__format__(printf, 1, 2)));
uint8_t console_getc(void);
void    console_perform(void);
