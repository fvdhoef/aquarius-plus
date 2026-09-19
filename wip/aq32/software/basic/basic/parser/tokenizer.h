#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_STRLEN 254

struct editbuf;
void tokenizer_init(struct editbuf *_eb);
int  tokenizer_get_cur_line(void);

uint8_t get_token(void);
void    ack_token(void);

struct tokenizer_state {
    const uint8_t *p_line;
    const uint8_t *p_line_end;
    const uint8_t *p_cur;
    int            cur_line;
};

void tokenizer_save_state(struct tokenizer_state *st);
void tokenizer_restore_state(const struct tokenizer_state *st);

union tokval_number {
    int32_t val_long;
    float   val_single;
    double  val_double;
};

extern union tokval_number tokval_num;
extern char                tokval_str[MAX_STRLEN + 3];
extern int                 tokval_strlen;

enum {
    TOK_END_OF_FILE = 0x00,
    TOK_EOL,
    TOK_LINENR,
    TOK_LABEL,
    TOK_IDENTIFIER,
    TOK_CONST_INT,
    TOK_CONST_LONG,
    TOK_CONST_SINGLE,
    TOK_CONST_DOUBLE,
    TOK_CONST_STR,

    TOK_LINE_TAG,
    TOK_TARGET,
    TOK_VAR_INT16,
    TOK_VAR_INT32,
    TOK_VAR_FLT32,
    TOK_VAR_FLT64,
    TOK_VAR_STR,

    // Helper keywords
    TOK_COLON,
    TOK_COMMA,
    TOK_ELSE,
    TOK_ELSEIF,
    TOK_ENDIF,
    TOK_FN,
    TOK_HASH,
    TOK_LET,
    TOK_LPAREN,
    TOK_NEXT,
    TOK_OFF,
    TOK_REM,
    TOK_RPAREN,
    TOK_SEMICOLON,
    TOK_SPC, // SPC(
    TOK_STEP,
    TOK_TAB, // TAB(
    TOK_THEN,
    TOK_TO,
    TOK_USING,
    TOK_WEND,

    // Operators (ordered by precedence)
    TOK_POW, // '^'
    TOK_OP_FIRST = TOK_POW,
    TOK_MULT,   // '*'
    TOK_DIV,    // '/'
    TOK_INTDIV, // '\'
    TOK_MOD,    // MOD
    TOK_PLUS,   // '+'
    TOK_MINUS,  // '-'
    TOK_EQ,     // '='
    TOK_NE,     // '<>'
    TOK_LT,     // '<'
    TOK_LE,     // '<='
    TOK_GT,     // '>'
    TOK_GE,     // '>='
    TOK_NOT,
    TOK_AND,
    TOK_OR,
    TOK_XOR,
    TOK_EQV,
    TOK_IMP,
    TOK_OP_LAST = TOK_IMP,

    // Statements
    TOK_CHDIR,
    TOK_STMT_FIRST = TOK_CHDIR,
    TOK_CLEAR,
    TOK_CLOSE,
    TOK_CLS,
    TOK_COLOR,
    TOK_DATA,
    TOK_DEFDBL,
    TOK_DEFINT,
    TOK_DEFLNG,
    TOK_DEFSNG,
    TOK_DEFSTR,
    TOK_DIM,
    TOK_END,
    TOK_ERASE,
    TOK_ERROR,
    TOK_FOR,
    TOK_GOSUB,
    TOK_GOTO,
    TOK_IF,
    TOK_INPUT,
    TOK_KILL,
    TOK_LINE_INPUT,
    TOK_LINE,
    TOK_LOCATE,
    TOK_MKDIR,
    TOK_ON,
    TOK_OPEN,
    TOK_PRINT,
    TOK_RANDOMIZE,
    TOK_READ,
    TOK_RESTORE,
    TOK_RESUME,
    TOK_RETURN,
    TOK_RMDIR,
    TOK_SWAP,
    TOK_WHILE,
    TOK_WIDTH,
    TOK_WRITE,
    TOK_STMT_LAST = TOK_WRITE,

    // Functions
    TOK_ABS,
    TOK_FUNC_FIRST = TOK_ABS,
    TOK_ASC,
    TOK_ATN,
    TOK_CDBL,
    TOK_CHRs, // CHR$
    TOK_CINT,
    TOK_CLNG,
    TOK_COS,
    TOK_CSNG,
    TOK_CSRLIN,
    TOK_CVD,
    TOK_CVI,
    TOK_CVL,
    TOK_CVS,
    TOK_EOF,
    TOK_ERL,
    TOK_ERR,
    TOK_EXP,
    TOK_FIX,
    TOK_HEXs,   // HEX$
    TOK_INKEYs, // INKEY$
    TOK_INPUTs, // INPUT$
    TOK_INSTR,
    TOK_INT,
    TOK_LCASEs, // LCASE$
    TOK_LEFTs,  // LEFT$
    TOK_LEN,
    TOK_LOF,
    TOK_LOG,
    TOK_LTRIMs, // LTRIM$
    TOK_MIDs,   // MID$
    TOK_MKDs,   // MKD$
    TOK_MKIs,   // MKI$
    TOK_MKLs,   // MKL$
    TOK_MKSs,   // MKS$
    TOK_OCTs,   // OCT$
    TOK_POS,
    TOK_RIGHTs, // RIGHT$
    TOK_RND,
    TOK_RTRIMs, // RTRIM$
    TOK_SEEK,
    TOK_SGN,
    TOK_SIN,
    TOK_SPACEs, // SPACE$
    TOK_SQR,
    TOK_STRINGs, // STRING$
    TOK_STRs,    // STR$
    TOK_TAN,
    TOK_TIMER,
    TOK_UCASEs, // UCASE$
    TOK_VAL,
    TOK_FUNC_LAST = TOK_VAL,
};
