#pragma once

#include <stdint.h>

void    ay8910_init(void);
void    ay8910_reset(void);
void    ay8910_write_reg(uint8_t r, uint8_t v);
uint8_t ay8910_read_reg(uint8_t r);
float   ay8910_render(void);
