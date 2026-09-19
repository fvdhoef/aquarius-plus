#pragma once

#include "common.h"

#define ARRAY_MAX_DIMENSIONS         8
#define ARRAY_DEFAULT_DIMENSION_SIZE 11

int  bytecode_get_line_for_offset(const uint8_t *buf, size_t buf_size, uint16_t offset);
void bytecode_dump(void);
void bytecode_run(const uint8_t *p_buf, size_t bc_size, size_t vars_sz);

enum {
    OPEN_MODE_INPUT = 0,
    OPEN_MODE_OUTPUT,
    OPEN_MODE_RANDOM,
    OPEN_MODE_APPEND,
};

enum {
    BC_UNDEFINED = 0,
    BC_END,
    BC_LINE_TAG,

    BC_DUP,
    BC_SWAP,
    BC_DROP,

    BC_PUSH_CONST_UNSPECIFIED_PARAM,
    BC_PUSH_CONST_INT,
    BC_PUSH_CONST_LONG,
    BC_PUSH_CONST_SINGLE,
    BC_PUSH_CONST_DOUBLE,
    BC_PUSH_CONST_STRING,

    BC_PUSH_VAR_INT,
    BC_PUSH_VAR_LONG,
    BC_PUSH_VAR_SINGLE,
    BC_PUSH_VAR_DOUBLE,
    BC_PUSH_VAR_STRING,

    BC_STORE_VAR_INT,
    BC_STORE_VAR_LONG,
    BC_STORE_VAR_SINGLE,
    BC_STORE_VAR_DOUBLE,
    BC_STORE_VAR_STRING,

    BC_PUSH_ARRAY_INT,
    BC_PUSH_ARRAY_LONG,
    BC_PUSH_ARRAY_SINGLE,
    BC_PUSH_ARRAY_DOUBLE,
    BC_PUSH_ARRAY_STRING,

    BC_STORE_ARRAY_INT,
    BC_STORE_ARRAY_LONG,
    BC_STORE_ARRAY_SINGLE,
    BC_STORE_ARRAY_DOUBLE,
    BC_STORE_ARRAY_STRING,

    BC_DIM_ARRAY_INT,
    BC_DIM_ARRAY_LONG,
    BC_DIM_ARRAY_SINGLE,
    BC_DIM_ARRAY_DOUBLE,
    BC_DIM_ARRAY_STRING,

    BC_FREE_ARRAY,

    BC_JMP,
    BC_JMP_NZ,
    BC_JMP_Z,
    BC_JSR,
    BC_RETURN,

    BC_DATA,
    BC_DATA_READ,
    BC_DATA_RESTORE,

    // Statement tokens (tokenizer.h)
    BC_STMT_CLEAR,
    BC_STMT_TIMER,

    // Math operators
    BC_OP_POW,
    BC_OP_NEGATE,
    BC_OP_MULT,
    BC_OP_DIV,
    BC_OP_INTDIV,
    BC_OP_MOD,
    BC_OP_ADD,
    BC_OP_SUB,
    BC_OP_EQ,
    BC_OP_NE,
    BC_OP_LT,
    BC_OP_LE,
    BC_OP_GT,
    BC_OP_GE,
    BC_OP_NOT,
    BC_OP_AND,
    BC_OP_OR,
    BC_OP_XOR,
    BC_OP_EQV,
    BC_OP_IMP,
    BC_OP_INC,
    BC_OP_LE_GE, // Used in for loop with step, takes 3 params: step/var/end

    BC_FUNC_ABS,
    BC_FUNC_ATN,
    BC_FUNC_TAN,
    BC_FUNC_COS,
    BC_FUNC_SIN,
    BC_FUNC_SQR,
    BC_FUNC_EXP,
    BC_FUNC_FIX,
    BC_FUNC_SGN,
    BC_FUNC_LOG,
    BC_FUNC_INT,
    BC_FUNC_RND,

    BC_STMT_RANDOMIZE,

    // Type conversion
    BC_FUNC_CINT,
    BC_FUNC_CLNG,
    BC_FUNC_CSNG,
    BC_FUNC_CDBL,

    // String handling
    BC_FUNC_LEN,
    BC_FUNC_LEFTs,
    BC_FUNC_RIGHTs,
    BC_FUNC_MIDs,
    BC_FUNC_ASC,
    BC_FUNC_INSTR,
    BC_FUNC_VAL,
    BC_FUNC_STRINGs,
    BC_FUNC_SPACEs,
    BC_FUNC_STRs,
    BC_FUNC_CHRs,
    BC_FUNC_HEXs,
    BC_FUNC_OCTs,
    BC_FUNC_LTRIMs,
    BC_FUNC_RTRIMs,
    BC_FUNC_LCASEs,
    BC_FUNC_UCASEs,

    // Console I/O
    BC_FUNC_INKEYs,
    BC_FUNC_CSRLIN,
    BC_FUNC_POS,

    BC_STMT_CLS,
    BC_STMT_COLOR,
    BC_STMT_INPUT,
    BC_STMT_LINE_INPUT,
    BC_STMT_LOCATE,
    BC_STMT_WIDTH,

    BC_PRINT_VAL,
    BC_PRINT_SPC,
    BC_PRINT_TAB,
    BC_PRINT_NEXT_FIELD,
    BC_PRINT_NEWLINE,
    BC_SET_FILE,
    BC_UNSET_FILE,

    // Error handling
    BC_FUNC_ERL,
    BC_FUNC_ERR,

    BC_STMT_ERROR,
    BC_STMT_RESUME,

    // File I/O
    BC_FUNC_MKIs,
    BC_FUNC_MKLs,
    BC_FUNC_MKSs,
    BC_FUNC_MKDs,
    BC_FUNC_CVI,
    BC_FUNC_CVL,
    BC_FUNC_CVS,
    BC_FUNC_CVD,

    BC_FILE_OPEN,
    BC_FILE_CLOSE,
    BC_FILE_CLOSE_ALL,
    BC_FILE_READ,
    BC_FILE_WRITE,
    BC_FILE_SEEK,
    BC_FILE_TELL,
    BC_FILE_SIZE,
    BC_FILE_READLINE,
    BC_FILE_INPUTs,
    BC_FILE_EOF,

    BC_STMT_KILL,
    BC_STMT_CHDIR,
    BC_STMT_MKDIR,
    BC_STMT_RMDIR,

    BC_FUNC_TIMER,
};
