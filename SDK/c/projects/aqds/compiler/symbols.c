#include "symbols.h"

#define MAX_SCOPE_DEPTH 8

#ifndef __SDCC
static uint8_t fake_heap[0x8000];
#endif

static struct symbol *scope[MAX_SCOPE_DEPTH];
static uint8_t        scope_idx;
static struct symbol *sym_start;

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
    scope[scope_idx] = sym_start;
    scope_idx++;
}

void symbol_pop_scope(void) {
    if (scope_idx == 0) {
        error("No scope to pop!");
    }
    scope_idx--;
    sym_start = scope[scope_idx];
}

struct symbol *symbol_find(const char *name, uint8_t name_len, bool allow_undefined) {
    if (sym_start) {
        if (name_len == 0)
            name_len = strlen(name);

        struct symbol *cur = sym_start;
        while ((uint8_t *)cur < getheap_end()) {
            if (cur->name_len == name_len && memcmp(name, cur->name, name_len) == 0)
                return cur;

            cur = (struct symbol *)((uint8_t *)(cur + 1) + cur->name_len + 1);
        }
    }
    if (!allow_undefined)
        error("Symbol not found");
    return NULL;
}

struct symbol *symbol_add(const char *name, uint8_t name_len) {
    if (name_len == 0)
        name_len = strlen(name);

    struct symbol *sym = symbol_find(name, name_len, true);
    if (sym && sym->scope_idx == scope_idx)
        error("Symbol already defined!");

    unsigned sym_size = sizeof(struct symbol) + name_len + 1;

    if (sym_start == NULL) {
        sym = (struct symbol *)(getheap_end() - sym_size);
    } else {
        sym = (struct symbol *)((uint8_t *)sym_start - sym_size);
    }
    if ((uint8_t *)sym < (uint8_t *)getheap())
        error("Out of memory!");

    sym->scope_idx = scope_idx;
    sym->symtype   = 0;
    sym->typespec  = 0;
    sym->storage   = 0;
    sym->value     = 0;
    sym->name_len  = name_len;
    memcpy(sym->name, name, name_len);
    sym->name[sym->name_len] = 0;

    sym_start = sym;

    return sym;
}

void symbol_dump(struct symbol *sym) {
    printf("- '%s' symtype:%u typespec:%u storage:%u value:%d\n", sym->name, sym->symtype, sym->typespec, sym->storage, sym->value);
}

void symbols_dump(void) {
    if (!sym_start)
        return;

    printf("--- Symbols ---\n");
    struct symbol *cur = sym_start;
    while ((uint8_t *)cur < getheap_end()) {
        symbol_dump(cur);
        cur = (struct symbol *)((uint8_t *)(cur + 1) + cur->name_len + 1);
    }
}
