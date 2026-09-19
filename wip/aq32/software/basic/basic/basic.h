#pragma once

#include "editor/editbuf.h"

enum {
    ERR_SYNTAX_ERROR           = 2,
    ERR_RETURN_WITHOUT_GOSUB   = 3,
    ERR_OUT_OF_DATA            = 4,
    ERR_ILLEGAL_FUNC_CALL      = 5,
    ERR_OVERFLOW               = 6,
    ERR_OUT_OF_MEM             = 7,
    ERR_LABEL_NOT_DEFINED      = 8,
    ERR_SUBSCRIPT_OUT_OF_RANGE = 9,
    ERR_DUPLICATE_DEFINITION   = 10,
    ERR_DIV_BY_ZERO            = 11,
    ERR_TYPE_MISMATCH          = 13,
    ERR_FORMULA_TOO_COMPLEX    = 16,
    ERR_BLOCK_IF_WITHOUT_ENDIF = 21,
    ERR_FOR_WITHOUT_NEXT       = 26,
    ERR_WHILE_WITHOUT_WEND     = 29,
    ERR_DUPLICATE_LABEL        = 33,
    ERR_INTERNAL_ERROR         = 51,
    ERR_FILE_NOT_FOUND         = 53,
    ERR_DEVICE_IO_ERROR        = 57,
    ERR_FILE_ALREADY_EXISTS    = 58,
    ERR_INPUT_PAST_END_OF_FILE = 62,
    ERR_TOO_MANY_FILES         = 67,
    ERR_DEVICE_UNAVAILABLE     = 68,
    ERR_PERMISSION_DENIED      = 70,
    ERR_UNHANDLED              = 73,
    ERR_PATH_FILE_ACCESS_ERROR = 75,
};

int         basic_compile(struct editbuf *eb);
int         basic_run(void);
int         basic_get_error_line(void);
const char *basic_get_error_str(int err);

extern int err_line;

noreturn void _basic_error(int err);
