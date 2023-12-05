#include "expr.h"
#include "tokenizer.h"
#include "symbols.h"

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
        node      = alloc_node(TOK_CONSTANT, NULL, NULL);
        node->val = tok_value;
        ack_token();

    } else if (token == TOK_IDENTIFIER) {
        struct symbol *sym = symbol_get(tok_strval, 0, false);
        ack_token();

        if (sym->type == SYMTYPE_DEFINE) {
            node      = alloc_node(TOK_CONSTANT, NULL, NULL);
            node->val = sym->value;
        } else {
            node      = alloc_node(TOK_IDENTIFIER, NULL, NULL);
            node->sym = sym;
        }

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
    if (token == '-') {
        ack_token();
        return alloc_node(TOK_OP_NEG, parse_unary_expr(), NULL);
    }
    if (token == '+') {
        return parse_unary_expr();
    }
    if (token == '~') {
        ack_token();
        return alloc_node('~', parse_unary_expr(), NULL);
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

static struct expr_node *parse_assign_expr(void) {
    struct expr_node *result = parse_logical_or_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '=') {
            ack_token();
            result = alloc_node(token, result, parse_assign_expr());
        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *_parse_expression(void) {
    return parse_assign_expr();
}

static void simplify_expr(struct expr_node *node) {
    if (node->op == TOK_CONSTANT || node->op == TOK_IDENTIFIER)
        return;

    if (node->left_node && node->left_node->op != TOK_CONSTANT) {
        simplify_expr(node->left_node);
    }
    if (node->right_node && node->right_node->op != TOK_CONSTANT) {
        simplify_expr(node->right_node);
    }
    if (node->op == TOK_OP_NEG && node->left_node->op == TOK_CONSTANT) {
        node->op  = TOK_CONSTANT;
        node->val = -node->left_node->val;

    } else if (node->op == '~' && node->left_node->op == TOK_CONSTANT) {
        node->op  = TOK_CONSTANT;
        node->val = ~node->left_node->val;

    } else if (
        node->left_node && node->left_node->op == TOK_CONSTANT &&
        node->right_node && node->right_node->op == TOK_CONSTANT) {

        int lv = node->left_node->val;
        int rv = node->right_node->val;

        // clang-format off
        switch (node->op) {
            case '*':        node->op = TOK_CONSTANT; node->val = lv *  rv; break;
            case '/':        node->op = TOK_CONSTANT; node->val = lv /  rv; break;
            case '%':        node->op = TOK_CONSTANT; node->val = lv %  rv; break;
            case '+':        node->op = TOK_CONSTANT; node->val = lv +  rv; break;
            case '-':        node->op = TOK_CONSTANT; node->val = lv -  rv; break;
            case TOK_OP_SHL: node->op = TOK_CONSTANT; node->val = lv << rv; break;
            case TOK_OP_SHR: node->op = TOK_CONSTANT; node->val = lv >> rv; break;
            case TOK_OP_LE:  node->op = TOK_CONSTANT; node->val = lv <= rv ? 1 : 0; break;
            case TOK_OP_GE:  node->op = TOK_CONSTANT; node->val = lv >= rv ? 1 : 0; break;
            case '<':        node->op = TOK_CONSTANT; node->val = lv <  rv ? 1 : 0; break;
            case '>':        node->op = TOK_CONSTANT; node->val = lv >  rv ? 1 : 0; break;
            case TOK_OP_EQ:  node->op = TOK_CONSTANT; node->val = lv == rv ? 1 : 0; break;
            case TOK_OP_NE:  node->op = TOK_CONSTANT; node->val = lv != rv ? 1 : 0; break;
            case '&':        node->op = TOK_CONSTANT; node->val = lv &  rv; break;
            case '^':        node->op = TOK_CONSTANT; node->val = lv ^  rv; break;
            case '|':        node->op = TOK_CONSTANT; node->val = lv |  rv; break;
            case TOK_OP_AND: node->op = TOK_CONSTANT; node->val = lv && rv; break;
            case TOK_OP_OR:  node->op = TOK_CONSTANT; node->val = lv || rv; break;
        }
        // clang-format on
    }
}

static void dump_expr(struct expr_node *node, int depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
    if (node->op == TOK_CONSTANT) {
        printf("- val: %d\n", node->val);
    } else if (node->op == TOK_IDENTIFIER) {
        printf("- identifier: %s\n", node->sym->name);
    } else {
        if (node->op > ' ') {
            printf("- '%c'\n", node->op);
        } else {
            printf("- %d\n", node->op);
        }
        if (node->left_node)
            dump_expr(node->left_node, depth + 2);
        if (node->right_node)
            dump_expr(node->right_node, depth + 2);
    }
}

struct expr_node *parse_expression(void) {
    reset_nodes();

    struct expr_node *node = NULL;
    node                   = _parse_expression();
    simplify_expr(node);
    dump_expr(node, 0);

    return node;
}
