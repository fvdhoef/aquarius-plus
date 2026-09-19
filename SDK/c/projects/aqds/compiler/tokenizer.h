#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include "common.h"

enum {
    TOK_EOF,
    TOK_EOL, // only returned if crossline_disable == true
    TOK_IDENTIFIER,
    TOK_CONSTANT,
    TOK_STRING_LITERAL,
    TOK_FUNC_CALL,
    TOK_FUNC_ARG,
    TOK_DEREF,
    TOK_ADDR_OF,

    TOK_BREAK,    // break
    TOK_CHAR,     // char
    TOK_CONST,    // const
    TOK_CONTINUE, // continue
    TOK_ELSE,     // else
    TOK_EXTERN,   // extern
    TOK_FOR,      // for
    TOK_IF,       // if
    TOK_INT,      // int
    TOK_IOPORT,   // ioport
    TOK_RETURN,   // return
    TOK_WHILE,    // while
    TOK_OP_EQ,    // ==
    TOK_OP_NE,    // !=
    TOK_OP_GE,    // >=
    TOK_OP_LE,    // <=
    TOK_OP_SHL,   // <<
    TOK_OP_SHR,   // >>
    TOK_OP_AND,   // &&
    TOK_OP_OR,    // ||
    TOK_OP_NEG,   // -
};

uint8_t get_token(void);
void    ack_token(void);

extern int  tok_value;
extern char tok_strval[256];

#endif
