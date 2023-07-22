#pragma once

#include "common.h"

enum {
    IO_BANK0 = 0xF0,
    IO_BANK1 = 0xF1,
    IO_BANK2 = 0xF2,
    IO_BANK3 = 0xF3,
};

void fpga_init(void);

void    fpga_reset_req(void);
void    fpga_update_keyb_matrix(uint8_t *keyb_matrix);
void    fpga_update_handctrl(uint8_t hctrl1, uint8_t hctrl2, TickType_t ticks_to_wait = portMAX_DELAY);
void    fpga_bus_acquire(void);
void    fpga_bus_release(void);
void    fpga_mem_write(uint16_t addr, uint8_t data);
uint8_t fpga_mem_read(uint16_t addr);
void    fpga_io_write(uint16_t addr, uint8_t data);
uint8_t fpga_io_read(uint16_t addr);

void fpga_save_banks(void);
void fpga_restore_banks(void);
void fpga_set_bank(unsigned bank, uint8_t val);
