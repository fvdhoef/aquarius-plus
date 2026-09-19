#include "bytecode.h"
#include "common/buffers.h"

static const char *bc_names[] = {
    [BC_END]                          = "END",
    [BC_LINE_TAG]                     = "LINE_TAG",
    [BC_DUP]                          = "DUP",
    [BC_SWAP]                         = "SWAP",
    [BC_DROP]                         = "DROP",
    [BC_PUSH_CONST_UNSPECIFIED_PARAM] = "PUSH_CONST_UNSPECIFIED_PARAM",
    [BC_OP_INC]                       = "OP_INC",
    [BC_OP_LE_GE]                     = "OP_LE_GE",
    [BC_DATA]                         = "DATA",
    [BC_DATA_READ]                    = "DATA_READ",
    [BC_DATA_RESTORE]                 = "DATA_RESTORE",
    [BC_PUSH_CONST_INT]               = "PUSH_CONST_INT",
    [BC_PUSH_CONST_LONG]              = "PUSH_CONST_LONG",
    [BC_PUSH_CONST_SINGLE]            = "PUSH_CONST_SINGLE",
    [BC_PUSH_CONST_DOUBLE]            = "PUSH_CONST_DOUBLE",
    [BC_PUSH_CONST_STRING]            = "PUSH_CONST_STRING",
    [BC_PUSH_VAR_INT]                 = "PUSH_VAR_INT",
    [BC_PUSH_VAR_LONG]                = "PUSH_VAR_LONG",
    [BC_PUSH_VAR_SINGLE]              = "PUSH_VAR_SINGLE",
    [BC_PUSH_VAR_DOUBLE]              = "PUSH_VAR_DOUBLE",
    [BC_PUSH_VAR_STRING]              = "PUSH_VAR_STRING",
    [BC_STORE_VAR_INT]                = "STORE_VAR_INT",
    [BC_STORE_VAR_LONG]               = "STORE_VAR_LONG",
    [BC_STORE_VAR_SINGLE]             = "STORE_VAR_SINGLE",
    [BC_STORE_VAR_DOUBLE]             = "STORE_VAR_DOUBLE",
    [BC_STORE_VAR_STRING]             = "STORE_VAR_STRING",
    [BC_PUSH_ARRAY_INT]               = "PUSH_ARRAY_INT",
    [BC_PUSH_ARRAY_LONG]              = "PUSH_ARRAY_LONG",
    [BC_PUSH_ARRAY_SINGLE]            = "PUSH_ARRAY_SINGLE",
    [BC_PUSH_ARRAY_DOUBLE]            = "PUSH_ARRAY_DOUBLE",
    [BC_PUSH_ARRAY_STRING]            = "PUSH_ARRAY_STRING",
    [BC_STORE_ARRAY_INT]              = "STORE_ARRAY_INT",
    [BC_STORE_ARRAY_LONG]             = "STORE_ARRAY_LONG",
    [BC_STORE_ARRAY_SINGLE]           = "STORE_ARRAY_SINGLE",
    [BC_STORE_ARRAY_DOUBLE]           = "STORE_ARRAY_DOUBLE",
    [BC_STORE_ARRAY_STRING]           = "STORE_ARRAY_STRING",
    [BC_DIM_ARRAY_INT]                = "DIM_ARRAY_INT",
    [BC_DIM_ARRAY_LONG]               = "DIM_ARRAY_LONG",
    [BC_DIM_ARRAY_SINGLE]             = "DIM_ARRAY_SINGLE",
    [BC_DIM_ARRAY_DOUBLE]             = "DIM_ARRAY_DOUBLE",
    [BC_DIM_ARRAY_STRING]             = "DIM_ARRAY_STRING",
    [BC_FREE_ARRAY]                   = "FREE_ARRAY",
    [BC_JMP]                          = "JMP",
    [BC_JMP_NZ]                       = "JMP_NZ",
    [BC_JMP_Z]                        = "JMP_Z",
    [BC_JSR]                          = "JSR",
    [BC_RETURN]                       = "RETURN",
    [BC_PRINT_VAL]                    = "PRINT_VAL",
    [BC_PRINT_SPC]                    = "PRINT_SPC",
    [BC_PRINT_TAB]                    = "PRINT_TAB",
    [BC_PRINT_NEXT_FIELD]             = "PRINT_NEXT_FIELD",
    [BC_PRINT_NEWLINE]                = "PRINT_NEWLINE",
    [BC_SET_FILE]                     = "SET_FILE",
    [BC_UNSET_FILE]                   = "UNSET_FILE",
    [BC_OP_POW]                       = "OP_POW",
    [BC_OP_MULT]                      = "OP_MULT",
    [BC_OP_DIV]                       = "OP_DIV",
    [BC_OP_INTDIV]                    = "OP_INTDIV",
    [BC_OP_MOD]                       = "OP_MOD",
    [BC_OP_ADD]                       = "OP_ADD",
    [BC_OP_SUB]                       = "OP_SUB",
    [BC_OP_EQ]                        = "OP_EQ",
    [BC_OP_NE]                        = "OP_NE",
    [BC_OP_LT]                        = "OP_LT",
    [BC_OP_LE]                        = "OP_LE",
    [BC_OP_GT]                        = "OP_GT",
    [BC_OP_GE]                        = "OP_GE",
    [BC_OP_NOT]                       = "OP_NOT",
    [BC_OP_AND]                       = "OP_AND",
    [BC_OP_OR]                        = "OP_OR",
    [BC_OP_XOR]                       = "OP_XOR",
    [BC_OP_EQV]                       = "OP_EQV",
    [BC_OP_IMP]                       = "OP_IMP",
    [BC_OP_NEGATE]                    = "OP_NEGATE",
    [BC_FILE_CLOSE_ALL]               = "FILE_CLOSE_ALL",
    [BC_FILE_CLOSE]                   = "FILE_CLOSE",
    [BC_FILE_EOF]                     = "FILE_EOF",
    [BC_FILE_INPUTs]                  = "FILE_INPUT$",
    [BC_FILE_OPEN]                    = "FILE_OPEN",
    [BC_FILE_READ]                    = "FILE_READ",
    [BC_FILE_READLINE]                = "FILE_READLINE",
    [BC_FILE_SEEK]                    = "FILE_SEEK",
    [BC_FILE_SIZE]                    = "FILE_SIZE",
    [BC_FILE_TELL]                    = "FILE_TELL",
    [BC_FILE_WRITE]                   = "FILE_WRITE",
    [BC_FUNC_ABS]                     = "FUNC_ABS",
    [BC_FUNC_ASC]                     = "FUNC_ASC",
    [BC_FUNC_ATN]                     = "FUNC_ATN",
    [BC_FUNC_CDBL]                    = "FUNC_CDBL",
    [BC_FUNC_CHRs]                    = "FUNC_CHR$",
    [BC_FUNC_CINT]                    = "FUNC_CINT",
    [BC_FUNC_CLNG]                    = "FUNC_CLNG",
    [BC_FUNC_COS]                     = "FUNC_COS",
    [BC_FUNC_CSNG]                    = "FUNC_CSNG",
    [BC_FUNC_CSRLIN]                  = "FUNC_CSRLIN",
    [BC_FUNC_CVD]                     = "FUNC_CVD",
    [BC_FUNC_CVI]                     = "FUNC_CVI",
    [BC_FUNC_CVL]                     = "FUNC_CVL",
    [BC_FUNC_CVS]                     = "FUNC_CVS",
    [BC_FUNC_ERL]                     = "FUNC_ERL",
    [BC_FUNC_ERR]                     = "FUNC_ERR",
    [BC_FUNC_EXP]                     = "FUNC_EXP",
    [BC_FUNC_FIX]                     = "FUNC_FIX",
    [BC_FUNC_HEXs]                    = "FUNC_HEX$",
    [BC_FUNC_INKEYs]                  = "FUNC_INKEY$",
    [BC_FUNC_INSTR]                   = "FUNC_INSTR",
    [BC_FUNC_INT]                     = "FUNC_INT",
    [BC_FUNC_LCASEs]                  = "FUNC_LCASE$",
    [BC_FUNC_LEFTs]                   = "FUNC_LEFT$",
    [BC_FUNC_LEN]                     = "FUNC_LEN",
    [BC_FUNC_LOG]                     = "FUNC_LOG",
    [BC_FUNC_LTRIMs]                  = "FUNC_LTRIM$",
    [BC_FUNC_MIDs]                    = "FUNC_MID$",
    [BC_FUNC_MKDs]                    = "FUNC_MKD$",
    [BC_FUNC_MKIs]                    = "FUNC_MKI$",
    [BC_FUNC_MKLs]                    = "FUNC_MKL$",
    [BC_FUNC_MKSs]                    = "FUNC_MKS$",
    [BC_FUNC_OCTs]                    = "FUNC_OCT$",
    [BC_FUNC_POS]                     = "FUNC_POS",
    [BC_FUNC_RIGHTs]                  = "FUNC_RIGHT$",
    [BC_FUNC_RND]                     = "FUNC_RND",
    [BC_FUNC_RTRIMs]                  = "FUNC_RTRIM$",
    [BC_FUNC_SGN]                     = "FUNC_SGN",
    [BC_FUNC_SIN]                     = "FUNC_SIN",
    [BC_FUNC_SPACEs]                  = "FUNC_SPACE$",
    [BC_FUNC_SQR]                     = "FUNC_SQR",
    [BC_FUNC_STRINGs]                 = "FUNC_STRING$",
    [BC_FUNC_STRs]                    = "FUNC_STR$",
    [BC_FUNC_TAN]                     = "FUNC_TAN",
    [BC_FUNC_TIMER]                   = "FUNC_TIMER",
    [BC_FUNC_UCASEs]                  = "FUNC_UCASE$",
    [BC_FUNC_VAL]                     = "FUNC_VAL",
    [BC_STMT_CLEAR]                   = "STMT_CLEAR",
    [BC_STMT_CLS]                     = "STMT_CLS",
    [BC_STMT_COLOR]                   = "STMT_COLOR",
    [BC_STMT_ERROR]                   = "STMT_ERROR",
    [BC_STMT_INPUT]                   = "STMT_INPUT",
    [BC_STMT_KILL]                    = "STMT_KILL",
    [BC_STMT_LINE_INPUT]              = "STMT_LINE_INPUT",
    [BC_STMT_LOCATE]                  = "STMT_LOCATE",
    [BC_STMT_RANDOMIZE]               = "STMT_RANDOMIZE",
    [BC_STMT_RESUME]                  = "STMT_RESUME",
    [BC_STMT_TIMER]                   = "STMT_TIMER",
    [BC_STMT_WIDTH]                   = "STMT_WIDTH",
};

void bytecode_dump(void) {
    const uint8_t *p = buf_bytecode;

    while (p < buf_bytecode_end) {
        uint8_t bc = *(p++);
        if (bc >= sizeof(bc_names) / sizeof(bc_names[0])) {
            printf("Byte code out of bounds! %02x\n", bc);
            exit(1);
        }

        printf("%5d %s%s", (int)(p - 1 - buf_bytecode), bc == BC_LINE_TAG ? "- " : "  - ", bc_names[bc]);

        switch (bc) {
            case BC_PUSH_CONST_INT: {
                printf(": %d", (int16_t)read_u16(p));
                p += 2;
                break;
            }
            case BC_PUSH_CONST_LONG: {
                printf(": %d", (int)(int32_t)read_u32(p));
                p += 4;
                break;
            }
            case BC_PUSH_CONST_SINGLE: {
                uint32_t v = read_u32(p);
                p += 4;
                printf(": %g", (double)*(float *)&v);
                break;
            }
            case BC_PUSH_CONST_DOUBLE: {
                uint64_t v = read_u64(p);
                p += 8;
                printf(": %lg", *(double *)&v);
                break;
            }
            case BC_PUSH_CONST_STRING: {
                printf(": \"%.*s\"", p[0], p + 1);
                p += 1 + p[0];
                break;
            }

            case BC_FILE_READ: {
                printf(": type=%c offset=%u", p[0], read_u16(p + 1));
                p += 3;
                break;
            }

            case BC_DATA:
            case BC_JMP:
            case BC_JMP_NZ:
            case BC_JMP_Z:
            case BC_JSR:
            case BC_DATA_RESTORE:
            case BC_PUSH_VAR_INT:
            case BC_PUSH_VAR_LONG:
            case BC_PUSH_VAR_SINGLE:
            case BC_PUSH_VAR_DOUBLE:
            case BC_PUSH_VAR_STRING:
            case BC_STORE_VAR_INT:
            case BC_STORE_VAR_LONG:
            case BC_STORE_VAR_SINGLE:
            case BC_STORE_VAR_DOUBLE:
            case BC_STORE_VAR_STRING:
            case BC_FREE_ARRAY:
            case BC_FILE_READLINE: {
                printf(": %u", read_u16(p));
                p += 2;
                break;
            }

            case BC_PUSH_ARRAY_INT:
            case BC_PUSH_ARRAY_LONG:
            case BC_PUSH_ARRAY_SINGLE:
            case BC_PUSH_ARRAY_DOUBLE:
            case BC_PUSH_ARRAY_STRING:
            case BC_STORE_ARRAY_INT:
            case BC_STORE_ARRAY_LONG:
            case BC_STORE_ARRAY_SINGLE:
            case BC_STORE_ARRAY_DOUBLE:
            case BC_STORE_ARRAY_STRING:
            case BC_DIM_ARRAY_INT:
            case BC_DIM_ARRAY_LONG:
            case BC_DIM_ARRAY_SINGLE:
            case BC_DIM_ARRAY_DOUBLE:
            case BC_DIM_ARRAY_STRING: {
                printf("<%u>: %u", p[0], read_u16(p + 1));
                p += 3;
                break;
            }

            case BC_LINE_TAG: {
                printf(": %u", read_u16(p) + 1);
                p += 2;
                break;
            }

            case BC_STMT_INPUT: {
                printf(": \"%.*s\"", p[0], p + 1);
                p += 1 + p[0];

                printf(" flags=%u", p[0]);
                p++;

                while (p[0] != 0) {
                    printf(" var%c:%u", p[0], read_u16(p + 1));
                    p += 3;
                }
                p++;
                break;
            }

            case BC_STMT_LINE_INPUT: {
                printf(": \"%.*s\"", p[0], p + 1);
                p += 1 + p[0];

                printf(" flags=%u", p[0]);
                p++;

                printf(" var$:%u", read_u16(p));
                p += 2;

                break;
            }
        }

        printf("\n");
    }
}
