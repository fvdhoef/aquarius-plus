#ifndef _EXPR_H
#define _EXPR_H

#include "common.h"

struct expr_node {
    uint8_t op;
    union {
        struct expr_node *left_node;
        int16_t           val;
    };
    struct expr_node *right_node;
};

struct expr_node *parse_expression(void);

#endif
