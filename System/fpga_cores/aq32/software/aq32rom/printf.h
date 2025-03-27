#pragma once

#include "common.h"

int printf(const char *format, ...) __attribute__((format(printf, 1, 2)));
int vprintf(const char *format, va_list ap);
int printf_handler(void (*char_out)(void *, char), void *context, const char *format, va_list ap);
