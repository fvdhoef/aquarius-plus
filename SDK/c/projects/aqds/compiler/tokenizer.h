#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include "common.h"

enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_CONSTANT,
    TOK_STRING_LITERAL,

    TOK_BREAK,    // break
    TOK_CHAR,     // char
    TOK_CONTINUE, // continue
    TOK_ELSE,     // else
    TOK_GOTO,     // goto
    TOK_IF,       // if
    TOK_INT,      // int
    TOK_RETURN,   // return
    TOK_WHILE,    // while
    TOK_OP_EQ,    // ==
    TOK_OP_NE,    // !=
    TOK_OP_GE,    // >=
    TOK_OP_LE,    // <=
    TOK_OP_SHL,   // <<
    TOK_OP_SHR,   // >>
};

uint8_t get_token(void);
void    ack_token(void);

extern int  tok_value;
extern char tok_strval[256];
extern char linebuf[256];

#endif
