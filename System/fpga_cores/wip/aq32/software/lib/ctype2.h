#pragma once

#include <stdint.h>

static inline bool is_octal(uint8_t ch) { return (ch >= '0' && ch <= '7'); }
static inline bool is_decimal(uint8_t ch) { return (ch >= '0' && ch <= '9'); }
static inline bool is_hexadecimal(uint8_t ch) { return is_decimal(ch) || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'); }
static inline bool is_upper(uint8_t ch) { return (ch >= 'A' && ch <= 'Z'); }
static inline bool is_lower(uint8_t ch) { return (ch >= 'a' && ch <= 'z'); }
static inline bool is_alpha(uint8_t ch) { return is_upper(ch) || is_lower(ch); }
static inline char to_upper(char ch) { return is_lower(ch) ? (ch - 'a' + 'A') : ch; }
static inline char to_lower(char ch) { return is_upper(ch) ? (ch - 'A' + 'a') : ch; }
static inline bool is_typechar(char ch) { return (ch == '%' || ch == '&' || ch == '!' || ch == '#' || ch == '$'); }
static inline bool is_cntrl(uint8_t ch) { return (ch < 32 || (ch >= 127 && ch < 160)); }
