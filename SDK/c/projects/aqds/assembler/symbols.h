#ifndef _SYMBOLS_H
#define _SYMBOLS_H

#include "common.h"

void     symbol_add(const char *str, size_t len, uint16_t value);
uint16_t symbol_get(const char *str, size_t len, bool allow_undefined);

#endif
