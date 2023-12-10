#ifndef _SYMBOLS_H
#define _SYMBOLS_H

#include "common.h"

// #define SYMTYPE_DEFINE   (0 << 0) // Not stored in memory
// #define SYMTYPE_FUNC     (1 << 0) // Function
// #define SYMTYPE_VAR_CHAR (2 << 0) // Character variable
// #define SYMTYPE_VAR_INT  (3 << 0) // Integer variable
// #define SYMTYPE_MASK     (3 << 0)
// #define SYMTYPE_GLOBAL   (1 << 5) // Global variable
// #define SYMTYPE_PTR      (1 << 6) // Pointer
// #define SYMTYPE_ARRAY    (1 << 7) // Array

enum {
    SYM_SYMTYPE_VAR,
    SYM_SYMTYPE_ARRAY,
    SYM_SYMTYPE_PTR,
    SYM_SYMTYPE_FUNC,
};

enum {
    SYM_TYPESPEC_CHAR,
    SYM_TYPESPEC_INT,
};

enum {
    SYM_STORAGE_STATIC,   // Global variable referred to by label name
    SYM_STORAGE_STACK,    // Variable stored on stack, value indicates offset
    SYM_STORAGE_CONSTANT, // Constant value stored in value
};

struct symbol {
    uint8_t scope_idx; // Scope index
    uint8_t symtype;   // Symbol type
    uint8_t typespec;  // Type specifier
    uint8_t storage;   // Storage class
    int16_t value;     // Value
    uint8_t name_len;  // Length of name excluding zero-termination
    char    name[];    // Zero-terminated name
};

struct symbol *symbol_find(const char *name, uint8_t name_len, bool allow_undefined);
struct symbol *symbol_add(const char *name, uint8_t name_len);
void           symbol_push_scope(void);
void           symbol_pop_scope(void);
void           symbol_dump(struct symbol *symbol);
void           symbols_dump(void);

#endif
