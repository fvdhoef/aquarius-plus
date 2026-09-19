#pragma once

#include <aqplus.h>

extern uint8_t term_flags;

void terminal_init(void);
void terminal_putchar(uint8_t ch);
void terminal_show_cursor(bool en);
