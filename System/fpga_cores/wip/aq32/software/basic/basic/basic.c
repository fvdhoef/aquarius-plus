#include "basic.h"
#include "bytecode/bytecode.h"
#include "parser/parser.h"
#include "parser/reloc.h"
#include <setjmp.h>
#include "console.h"
#include "common/buffers.h"
#include "bytecode/bytecode_internal.h"

struct err_str {
    uint8_t     err;
    const char *str;
};

static const struct err_str err_str[] = {
    {1, "NEXT without FOR"},
    {2, "Syntax error"},
    {3, "RETURN without GOSUB"},
    {4, "Out of DATA"},
    {5, "Illegal function call"},
    {6, "Overflow"},
    {7, "Out of memory"},
    {8, "Label not defined"},
    {9, "Subscript out of range"},
    {10, "Duplicate definition"},
    {11, "Division by zero"},
    {12, "Illegal in direct mode"},
    {13, "Type mismatch"},
    {14, "Out of string space"},
    {15, "String too long"}, // Only in GW-BASIC
    {16, "Formula too complex"},
    {17, "Cannot continue"},
    {18, "Function not defined"},
    {19, "No RESUME"},
    {20, "RESUME without error"},
    {21, "Block IF without END IF"}, // 21 new
    {22, "Missing operand"},         // Only in GW-BASIC
    {23, "Line buffer overflow"},    // Only in GW-BASIC
    {24, "Device timeout"},
    {25, "Device fault"},
    {26, "FOR without NEXT"},
    {27, "Out of paper"},
    // 28
    {29, "WHILE without WEND"},
    {30, "WEND without WHILE"},
    // 31,32
    {33, "Duplicate label"}, // Only in Q-BASIC
    // 34
    {35, "Subprogram not defined"}, // Only in Q-BASIC
    // 36
    {37, "Argument-count mismatch"}, // Only in Q-BASIC
    {38, "Array not defined"},       // Only in Q-BASIC
    // 39
    {40, "Variable required"}, // Only in Q-BASIC
    // 41-49
    {50, "FIELD overflow"},
    {51, "Internal error"},
    {52, "Bad file name or number"},
    {53, "File not found"},
    {54, "Bad file mode"},
    {55, "File already open"},
    {56, "FIELD statement active"}, // Only in Q-BASIC
    {57, "Device I/O error"},
    {58, "File already exists"},
    {59, "Bad record length"}, // Only in Q-BASIC
    // 60
    {61, "Disk full"},
    {62, "Input past end of file"},
    {63, "Bad record number"},
    {64, "Bad filename"},
    // 65
    {66, "Direct statement in file"}, // Only in GW-BASIC
    {67, "Too many files"},
    {68, "Device unavailable"},
    {69, "Communication-buffer overflow"},
    {70, "Permission denied"},
    {71, "Disk not ready"},
    {72, "Disk-media error"},
    {73, "Feature unavailable"},
    {74, "Rename across disks"},
    {75, "Path/File access error"},
    {76, "Path not found"},
};

static int     cur_error;
static jmp_buf jb_error;
int            err_line;

void _basic_error(int err) {
    cur_error = err;
    longjmp(jb_error, 1);
}

int basic_compile(struct editbuf *eb) {
    cur_error = 0;
    if (setjmp(jb_error) == 0) {
        basic_parse(eb);
    } else {
        return cur_error;
    }
    return 0;
}

int basic_run(void) {
    bc_file_close_all();

    cur_error = 0;
    if (setjmp(jb_error) == 0) {
        console_init();
        bytecode_run(buf_bytecode, buf_bytecode_end - buf_bytecode, vars_total_size);
    }

    bc_file_close_all();
    return cur_error;
}

int basic_get_error_line(void) {
    return err_line;
}

const char *basic_get_error_str(int err) {
    static char result[64];

    const char *str = NULL;
    for (int i = 0; i < (int)(sizeof(err_str) / sizeof(err_str[0])); i++) {
        if (err_str[i].err == err) {
            str = err_str[i].str;
            break;
        }
    }
    if (str == NULL) {
        snprintf(result, sizeof(result), "Error %d in line %d", err, err_line + 1);
    } else {
        snprintf(result, sizeof(result), "%s in line %d", str, err_line + 1);
    }
    return result;
}
