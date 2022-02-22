#pragma once

#include "common.h"

void    esp32_init(const char *basepath);
void    esp32_write_ctrl(uint8_t data);
void    esp32_write_data(uint8_t data);
uint8_t esp32_read_ctrl(void);
uint8_t esp32_read_data(void);
