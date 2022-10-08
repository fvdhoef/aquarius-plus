#pragma once

#include "common.h"

enum {
    IO_BANK0 = 0xF0,
    IO_BANK1 = 0xF1,
    IO_BANK2 = 0xF2,
    IO_BANK3 = 0xF3,
};

void flash_prepare(void);
void flash_finish(void);

void flash_program(unsigned addr, uint8_t val);
void flash_erase_4kb_sector(unsigned addr);
