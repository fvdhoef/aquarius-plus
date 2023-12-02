#include "expr.h"
#include "tokenizer.h"

#define MAX_NODES (64)

static struct expr_node nodes[MAX_NODES];
static int              node_idx = 0;

static struct expr_node *alloc_node(void) {
    if (node_idx >= MAX_NODES)
        error("Expression too complex");

    struct expr_node *result = &nodes[node_idx++];
    return result;
}

static void reset_nodes(void) {
    node_idx = 0;
}

struct expr_node *parse_expression(void) {
    reset_nodes();
    int token = get_token();
    printf("token: %d\n", token);
    ack_token();

    return NULL;
}
