#include "symbols.h"

static struct symbol *first;
static struct symbol *last;

struct symbol *symbol_get(const char *name, uint8_t name_len, bool allow_undefined) {
    if (name_len == 0)
        name_len = strlen(name);

    struct symbol *cur = first;
    while (cur) {
        if (cur->name_len == name_len && memcmp(name, cur->name, name_len) == 0)
            return cur;
        cur = cur->next;
    }
    if (!allow_undefined)
        error("Symbol not found");
    return NULL;
}

struct symbol *symbol_add(uint8_t type, const char *name, uint8_t name_len) {
    if (name_len == 0)
        name_len = strlen(name);

    if (symbol_get(name, name_len, true))
        error("Symbol already defined!");

    struct symbol *result = malloc(sizeof(struct symbol) + name_len + 1);

    if (!first) {
        first = result;
    } else {
        last->next = result;
    }
    last = result;

    result->next     = NULL;
    result->type     = type;
    result->value    = 0;
    result->name_len = name_len;
    memcpy(result->name, name, name_len);
    result->name[name_len] = 0;

    return result;
}

void symbol_dump(struct symbol *symbol) {
    printf("- '%s' $%02x value:%u\n", symbol->name, symbol->type, symbol->value);
}

void symbol_dump_all(void) {
    printf("--- Symbols ---\n");
    struct symbol *cur = first;
    while (cur) {
        symbol_dump(cur);
        cur = cur->next;
    }
}
