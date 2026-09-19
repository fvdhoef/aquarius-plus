#include "expr.h"
#include "tokenizer.h"
#include "symbols.h"

#define MAX_NODES (64)

static struct expr_node nodes[MAX_NODES];
static int              node_idx = 0;

static struct expr_node *_parse_expression(void);
static struct expr_node *parse_cast_expr(void);

static struct expr_node *alloc_node(uint8_t op, struct expr_node *left, struct expr_node *right, uint8_t symtype, uint8_t typespec) {
    if (node_idx >= MAX_NODES)
        error("Expression too complex");

    struct expr_node *result = &nodes[node_idx++];
    result->op               = op;
    result->left_node        = left;
    result->right_node       = right;
    result->symtype          = symtype;
    result->typespec         = typespec;

    return result;
}

static void reset_nodes(void) {
    node_idx = 0;
}

static struct expr_node *parse_primary_expr(void) {
    struct expr_node *node = NULL;

    uint8_t token = get_token();
    if (token == TOK_CONSTANT) {
        node      = alloc_node(TOK_CONSTANT, NULL, NULL, SYM_SYMTYPE_VALUE, SYM_TYPESPEC_INT);
        node->val = tok_value;
        ack_token();

    } else if (token == TOK_IDENTIFIER) {
        struct symbol *sym = symbol_find(tok_strval, 0, true);
        ack_token();

        if (!sym) {
            if (get_token() == '(') {
                // Function not defined. Implicitly define it.
                sym          = symbol_add(tok_strval, 0);
                sym->symtype = SYM_SYMTYPE_FUNC;
                sym->storage = SYM_STORAGE_STATIC;
            } else {
                error_sym_not_found(tok_strval);
            }
        }

        if (sym->storage == SYM_STORAGE_CONSTANT) {
            node      = alloc_node(TOK_CONSTANT, NULL, NULL, sym->symtype, sym->typespec);
            node->val = sym->value;

        } else {
            node      = alloc_node(TOK_IDENTIFIER, NULL, NULL, sym->symtype, sym->typespec);
            node->sym = sym;
        }

    } else if (token == TOK_STRING_LITERAL) {
        ack_token();
        node      = alloc_node(TOK_STRING_LITERAL, NULL, NULL, 0, 0);
        node->str = strings_add(tok_strval, 0);

    } else if (token == '(') {
        ack_token();
        node = _parse_expression();
        if (get_token() != ')')
            error_syntax();
        ack_token();

    } else {
        error_syntax();
    }
    return node;
}

static struct expr_node *parse_postfix_expr(void) {
    struct expr_node *result = parse_primary_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '(') {
            ack_token();
            result = alloc_node(TOK_FUNC_CALL, result, NULL, SYM_SYMTYPE_VALUE, SYM_TYPESPEC_INT);

            if (get_token() == ')') {
                ack_token();
            } else {
                struct expr_node **list_next = &result->right_node;
                while (1) {
                    struct expr_node *expr = _parse_expression();

                    *list_next = alloc_node(TOK_FUNC_ARG, expr, NULL, expr->symtype, expr->typespec);
                    list_next  = &(*list_next)->right_node;

                    if (get_token() == ')') {
                        ack_token();
                        break;
                    }
                    expect_tok_ack(',');
                }
            }

        } else if (token == '[') {
            ack_token();
            if (result->symtype != SYM_SYMTYPE_ARRAY && result->symtype != SYM_SYMTYPE_PTR)
                error("Expected pointer type");

            struct expr_node *expr = _parse_expression();

            // Transform: ident[expr] -> *(ident + expr)
            result = alloc_node('+', result, expr, result->symtype, result->typespec);
            if (result->symtype == SYM_SYMTYPE_ARRAY)
                result->symtype = SYM_SYMTYPE_PTR;
            result = alloc_node(TOK_DEREF, result, NULL, result->symtype, result->typespec);

            expect_tok_ack(']');

        } else {
            break;
        }
    }
    return result;
}

static struct expr_node *parse_unary_expr(void) {
    uint8_t token = get_token();
    if (token == '-') {
        ack_token();
        struct expr_node *expr = parse_cast_expr();
        return alloc_node(TOK_OP_NEG, expr, NULL, expr->symtype, expr->typespec);
    }
    if (token == '+') {
        ack_token();
        return parse_cast_expr();
    }
    if (token == '~') {
        ack_token();
        struct expr_node *expr = parse_cast_expr();
        return alloc_node('~', expr, NULL, expr->symtype, expr->typespec);
    }
    if (token == '!') {
        ack_token();
        struct expr_node *expr = parse_cast_expr();
        return alloc_node('!', expr, NULL, expr->symtype, expr->typespec);
    }
    if (token == '*') {
        ack_token();
        struct expr_node *expr = parse_cast_expr();
        return alloc_node(TOK_DEREF, expr, NULL, SYM_SYMTYPE_VALUE, expr->typespec);
    }
    if (token == '&') {
        ack_token();
        struct expr_node *expr = parse_cast_expr();
        return alloc_node(TOK_ADDR_OF, expr, NULL, SYM_SYMTYPE_PTR, expr->typespec);
    }
    return parse_postfix_expr();
}

static struct expr_node *parse_cast_expr(void) {
    uint8_t token = get_token();
    if (token == '(') {
        ack_token();
        token = get_token();
        if (token == TOK_CHAR || token == TOK_INT) {
            uint8_t tok_type = token;

            ack_token();
            expect_tok_ack('*');
            expect_tok_ack(')');

            struct expr_node *expr = parse_unary_expr();
            expr->symtype          = SYM_SYMTYPE_PTR;
            expr->typespec         = (tok_type == TOK_CHAR) ? SYM_TYPESPEC_CHAR : SYM_TYPESPEC_INT;
            return expr;

        } else {
            struct expr_node *expr = _parse_expression();
            if (get_token() != ')')
                error_syntax();
            ack_token();
            return expr;
        }
    }
    return parse_unary_expr();
}

static struct expr_node *parse_mult_expr(void) {
    struct expr_node *result = parse_cast_expr();
    while (1) {
        uint8_t token = get_token();
        if (token == '*' || token == '/' || token == '%') {
            ack_token();
            result = alloc_node(token, result, parse_cast_expr(), result->symtype, result->typespec);
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
            result = alloc_node(token, result, parse_mult_expr(), result->symtype, result->typespec);
            if (result->symtype == SYM_SYMTYPE_ARRAY)
                result->symtype = SYM_SYMTYPE_PTR;
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
            result = alloc_node(token, result, parse_add_expr(), result->symtype, result->typespec);
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
            result = alloc_node(token, result, parse_shift_expr(), SYM_SYMTYPE_VALUE, SYM_TYPESPEC_INT);
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
            result = alloc_node(token, result, parse_rel_expr(), SYM_SYMTYPE_VALUE, SYM_TYPESPEC_INT);
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
            result = alloc_node(token, result, parse_eq_expr(), result->symtype, result->typespec);
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
            result = alloc_node(token, result, parse_and_expr(), result->symtype, result->typespec);
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
            result = alloc_node(token, result, parse_xor_expr(), result->symtype, result->typespec);
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
            result = alloc_node(token, result, parse_or_expr(), SYM_SYMTYPE_VALUE, SYM_TYPESPEC_INT);
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
            result = alloc_node(token, result, parse_logical_and_expr(), SYM_SYMTYPE_VALUE, SYM_TYPESPEC_INT);
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
            struct expr_node *expr = parse_assign_expr();

            result = alloc_node(token, result, expr, expr->symtype, expr->typespec);
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
    if (node->op == TOK_CONSTANT || node->op == TOK_IDENTIFIER || node->op == TOK_STRING_LITERAL)
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

    } else if (node->op == '!' && node->left_node->op == TOK_CONSTANT) {
        node->op  = TOK_CONSTANT;
        node->val = !node->left_node->val;

    } else if (
        node->left_node && node->left_node->op == TOK_CONSTANT &&
        node->right_node && node->right_node->op == TOK_CONSTANT) {

        int16_t lv = node->left_node->val;
        int16_t rv = node->right_node->val;

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

#ifdef DEBUG_OUTPUT
static void dump_expr(struct expr_node *node, int depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
    if (node->op == TOK_CONSTANT) {
        printf("- val: %d\n", node->val);
    } else if (node->op == TOK_IDENTIFIER) {
        printf("- identifier: %s\n", node->sym->name);
    } else if (node->op == TOK_STRING_LITERAL) {
        printf("- string literal %d: %s\n", node->str->idx, node->str->buf);
    } else {
        if (node->op > ' ') {
            printf("- '%c' -> symtype:%u typespec:%u\n", node->op, node->symtype, node->typespec);
        } else {
            printf("- %d -> symtype:%u typespec:%u\n", node->op, node->symtype, node->typespec);
        }
        if (node->left_node)
            dump_expr(node->left_node, depth + 2);
        if (node->right_node)
            dump_expr(node->right_node, depth + 2);
    }
}
#endif

struct expr_node *parse_expression(void) {
    reset_nodes();

    struct expr_node *node = NULL;
    node                   = _parse_expression();
    simplify_expr(node);

#ifdef DEBUG_OUTPUT
    dump_expr(node, 0);
#endif

    return node;
}
