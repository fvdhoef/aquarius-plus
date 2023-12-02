#include "expr.h"
#include "tokenizer.h"

#define MAX_NODES (64)

static struct expr_node nodes[MAX_NODES];
static int              node_idx = 0;

static struct expr_node *_parse_expression(void);

static struct expr_node *alloc_node(uint8_t op, struct expr_node *left, struct expr_node *right) {
    if (node_idx >= MAX_NODES)
        error("Expression too complex");

    struct expr_node *result = &nodes[node_idx++];
    result->op               = op;
    result->left_node        = left;
    result->right_node       = right;
    return result;
}

static void reset_nodes(void) {
    node_idx = 0;
}

static struct expr_node *parse_primary_expr(void) {
    struct expr_node *node = NULL;

    uint8_t token = get_token();
    if (token == TOK_CONSTANT) {
        node           = alloc_node(TOK_CONSTANT, NULL, NULL);
        node->left_val = tok_value;
        ack_token();
    } else if (token == '(') {
        ack_token();
        node = _parse_expression();
        if (get_token() != ')')
            syntax_error();
        ack_token();

    } else {
        syntax_error();
    }
    return node;
}

static struct expr_node *parse_unary_expr(void) {
    uint8_t token = get_token();
    if (token == '-' || token == '+' || token == '~') {
        ack_token();
        return alloc_node(token, parse_unary_expr(), NULL);
    }
    return parse_primary_expr();
}

static struct expr_node *parse_mult_expr(void) {
    struct expr_node *result = parse_unary_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '*' || token == '/' || token == '%') {
            ack_token();
            result = alloc_node(token, result, parse_unary_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_add_expr(void) {
    struct expr_node *result = parse_mult_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '+' || token == '-') {
            ack_token();
            result = alloc_node(token, result, parse_mult_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_shift_expr(void) {
    struct expr_node *result = parse_add_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == TOK_OP_SHL || token == TOK_OP_SHR) {
            ack_token();
            result = alloc_node(token, result, parse_add_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_rel_expr(void) {
    struct expr_node *result = parse_shift_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == TOK_OP_LE || token == TOK_OP_GE || token == '<' || token == '>') {
            ack_token();
            result = alloc_node(token, result, parse_shift_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_eq_expr(void) {
    struct expr_node *result = parse_rel_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == TOK_OP_EQ || token == TOK_OP_NE) {
            ack_token();
            result = alloc_node(token, result, parse_rel_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_and_expr(void) {
    struct expr_node *result = parse_eq_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '&') {
            ack_token();
            result = alloc_node(token, result, parse_eq_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_xor_expr(void) {
    struct expr_node *result = parse_and_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '^') {
            ack_token();
            result = alloc_node(token, result, parse_and_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_or_expr(void) {
    struct expr_node *result = parse_xor_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '|') {
            ack_token();
            result = alloc_node(token, result, parse_xor_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_logical_and_expr(void) {
    struct expr_node *result = parse_or_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == TOK_OP_AND) {
            ack_token();
            result = alloc_node(token, result, parse_or_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_logical_or_expr(void) {
    struct expr_node *result = parse_logical_and_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == TOK_OP_OR) {
            ack_token();
            result = alloc_node(token, result, parse_logical_and_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *_parse_expression(void) {
    return parse_logical_or_expr();
}

struct expr_node *parse_expression(void) {
    reset_nodes();

    struct expr_node *node = NULL;
    node                   = _parse_expression();

    return node;
}
