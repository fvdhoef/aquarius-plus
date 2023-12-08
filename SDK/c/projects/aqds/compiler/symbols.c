#include "symbols.h"

#define MAX_SCOPE_DEPTH 8

#ifndef __SDCC
static uint8_t fake_heap[0x8000];
#endif

static struct symbol *scope[MAX_SCOPE_DEPTH];
static uint8_t        scope_idx;
static struct symbol *sym_first;
static struct symbol *sym_last;

#ifndef __SDCC
void *getheap(void) {
    return fake_heap;
}
static uint8_t *getheap_end(void) {
    return fake_heap + sizeof(fake_heap);
}
#else
static uint8_t *getheap_end(void) {
    return (uint8_t *)0xF000;
}
#endif

void symbol_push_scope(void) {
    if (scope_idx >= MAX_SCOPE_DEPTH) {
        error("Scope nesting too deep!");
    }
    scope[scope_idx] = sym_last;
    scope_idx++;
}

void symbol_pop_scope(void) {
    if (scope_idx == 0) {
        error("No scope to pop!");
    }
    scope_idx--;
    sym_last = scope[scope_idx];
}

struct symbol *symbol_get(const char *name, uint8_t name_len, bool allow_undefined) {
    if (sym_last) {
        if (name_len == 0)
            name_len = strlen(name);

        struct symbol *cur = sym_last;
        while (1) {
            if (cur->name_len == name_len && memcmp(name, cur->name, name_len) == 0)
                return cur;

            if (cur == sym_first)
                break;

            cur = cur->prev;
        }
    }
    if (!allow_undefined)
        error("Symbol not found");
    return NULL;
}

struct symbol *symbol_add(uint8_t type, const char *name, uint8_t name_len) {
    if (name_len == 0)
        name_len = strlen(name);

    struct symbol *sym = symbol_get(name, name_len, true);
    if (sym && sym->scope_idx == scope_idx) {
        error("Symbol already defined!");
    }

    if (sym_last == NULL) {
        sym = getheap();
    } else {
        sym = (struct symbol *)((uint8_t *)(sym_last + 1) + sym_last->name_len + 1);
    }

    unsigned reclen = sizeof(struct symbol) + name_len + 1;
    if ((uint8_t *)sym + reclen > getheap_end())
        error("Out of memory!");

    if (sym_first == NULL)
        sym_first = sym;

    sym->prev      = sym_last;
    sym->scope_idx = scope_idx;
    sym->type      = type;
    sym->value     = 0;
    sym->name_len  = name_len;
    memcpy(sym->name, name, name_len);
    sym->name[sym->name_len] = 0;

    sym_last = sym;

    return sym;
}

void symbol_dump(struct symbol *symbol) {
    printf("- '%s' $%02x value:%u\n", symbol->name, symbol->type, symbol->value);
}

void symbol_dump_all(void) {
    if (!sym_first)
        return;

    printf("--- Symbols ---\n");
    struct symbol *cur = sym_first;
    while (1) {
        symbol_dump(cur);
        if (cur == sym_last)
            break;

        cur = (struct symbol *)((uint8_t *)(cur + 1) + cur->name_len + 1);
    }
}
