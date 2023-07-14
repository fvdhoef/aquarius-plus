#pragma once

#include "common.h"

void    ch376_init(const char *basepath);
void    ch376_write_data(uint8_t data);
void    ch376_write_cmd(uint8_t cmd);
uint8_t ch376_read_data(void);
uint8_t ch376_read_status(void);
