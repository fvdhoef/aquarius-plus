#pragma once

#include "common.h"

// return 0 for float, otherwise integer base (8: octal, 10: decimal, 16: hexadecimal)
int copy_num_to_buf(const uint8_t **ps, const uint8_t *ps_end, char *buf, size_t buf_size, char *type);
