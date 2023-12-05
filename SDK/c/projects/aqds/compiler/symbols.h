#ifndef _SYMBOLS_H
#define _SYMBOLS_H

#include "common.h"

#define SYMTYPE_DEFINE   (0 << 0) // Not stored in memory
#define SYMTYPE_FUNC     (1 << 0) // Function
#define SYMTYPE_VAR_CHAR (2 << 0) // Character variable
#define SYMTYPE_VAR_INT  (3 << 0) // Integer variable
#define SYMTYPE_GLOBAL   (1 << 5) // Global variable
#define SYMTYPE_PTR      (1 << 6) // Pointer
#define SYMTYPE_ARRAY    (1 << 7) // Array

struct symbol {
    struct symbol *next;
    uint8_t        type;
    uint16_t       value;
    uint8_t        name_len; // Excluding zero-termination
    char           name[];
};

struct symbol *symbol_get(const char *name, uint8_t name_len, bool allow_undefined);
struct symbol *symbol_add(uint8_t type, const char *name, uint8_t name_len);

void symbol_dump(struct symbol *symbol);
void symbol_dump_all(void);

#endif
