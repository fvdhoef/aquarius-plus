#pragma once

#include "common.h"

void fpga_init(void);

void fpga_reset_req(void);
void fpga_update_keyb_matrix(uint8_t *buf);
