#pragma once

#include "common.h"

void fpga_init(void);

void    fpga_reset_req(void);
void    fpga_update_keyb_matrix(uint8_t *keyb_matrix);
void    fpga_bus_acquire(void);
void    fpga_bus_release(void);
void    fpga_mem_write(uint16_t addr, uint8_t data);
uint8_t fpga_mem_read(uint16_t addr);
void    fpga_io_write(uint16_t addr, uint8_t data);
uint8_t fpga_io_read(uint16_t addr);
