#include "expr.h"
#include "tokenizer.h"

#define MAX_NODES (64)

static struct expr_node nodes[MAX_NODES];
static int              node_idx = 0;

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
    } else {
        syntax_error();
    }
    return node;
}

struct expr_node *parse_expression(void) {
    reset_nodes();

    struct expr_node *node = NULL;
    node                   = parse_primary_expr();

    return node;
}
