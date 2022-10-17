#pragma once

#include "common.h"

void flash_prepare(void);
void flash_finish(void);

void flash_program(unsigned addr, uint8_t val);
void flash_erase_4kb_sector(unsigned addr);

bool verify_sysrom(void);
void flash_sysrom(void);
