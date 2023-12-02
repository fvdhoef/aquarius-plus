#ifndef _EXPR_H
#define _EXPR_H

#include "common.h"

struct expr_node {
    uint8_t op;
    union {
        struct expr_node *left_node;
        int16_t           left_val;
    };
    union {
        struct expr_node *right_node;
        int16_t           right_val;
    };
};

struct expr_node *parse_expression(void);

#endif
