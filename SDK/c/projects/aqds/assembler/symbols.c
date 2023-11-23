#include "symbols.h"

void symbol_add(const char *str, size_t len, uint16_t value) {
    if (len == 0)
        len = strlen(str);

    printf("[Symbol %.*s = $%04x]\n", (int)len, str, value);
}

uint16_t symbol_get(const char *str, size_t len) {
    error("Symbol not found");
    return 0;
}
