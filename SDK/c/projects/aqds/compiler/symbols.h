#ifndef _SYMBOLS_H
#define _SYMBOLS_H

#include "common.h"

enum {
    SYM_SYMTYPE_VAR,
    SYM_SYMTYPE_ARRAY,
    SYM_SYMTYPE_PTR,
    SYM_SYMTYPE_FUNC,
    SYM_SYMTYPE_VALUE,
};

enum {
    SYM_TYPESPEC_UNDEFINED,
    SYM_TYPESPEC_CHAR,
    SYM_TYPESPEC_INT,
};

enum {
    SYM_STORAGE_STATIC,   // Global variable referred to by label name
    SYM_STORAGE_STACK,    // Variable stored on stack, value indicates offset
    SYM_STORAGE_CONSTANT, // Constant value stored in value
    SYM_STORAGE_IOPORT,   // IO port
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

struct string {
    int16_t idx;
    uint8_t buf_len; // Length of name excluding zero-termination
    char    buf[];   // Zero-terminated name
};

struct symbol *symbol_find(const char *name, uint8_t name_len, bool allow_undefined);
struct symbol *symbol_add(const char *name, uint8_t name_len);
void           symbol_push_scope(void);
void           symbol_pop_scope(void);
void           symbol_dump(struct symbol *symbol);
void           symbols_dump(void);

void           strings_clear(void);
struct string *strings_add(const char *buf, uint8_t buf_len);
struct string *strings_first(void);
struct string *strings_last(void);

#endif
