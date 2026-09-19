#ifndef _EXPR_H
#define _EXPR_H

#include "common.h"

struct symbol;
struct string;

struct expr_node {
    uint8_t op;
    union {
        struct expr_node *left_node;
        struct symbol    *sym;
        struct string    *str;
        int16_t           val;
    };
    struct expr_node *right_node;

    uint8_t symtype;
    uint8_t typespec;
};

struct expr_node *parse_expression(void);

#endif
