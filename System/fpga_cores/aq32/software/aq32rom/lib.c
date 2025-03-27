#include "lib.h"

static inline bool has_null_byte(uint32_t val) { return ((val - 0x01010101) & ~val & 0x80808080); }
static inline bool is_aligned32(const void *p) { return ((uintptr_t)p & 3) == 0; }

size_t strlen(const char *str) {
    const char *src8 = str;

    while (!is_aligned32(src8)) {
        if (!*src8)
            return src8 - str;
        src8++;
    }

    const uint32_t *src32 = (const uint32_t *)src8;
    while (!has_null_byte(*src32))
        src32++;

    src8 = (const char *)src32;

    while (*src8)
        src8++;

    return src8 - str;
}
