#include "bytecode_internal.h"
#include <errno.h>
#include <sys/stat.h>

#define MAX_OPEN (10)

struct open_file {
    FILE    *f;
    unsigned column;
};

static struct open_file files[MAX_OPEN];

int file_io_cur_file;

void bc_func_mki_s(void) {
    int32_t val = bc_stack_pop_long();
    if (val < INT16_MIN || val > INT16_MAX)
        _basic_error(ERR_OVERFLOW);

    int16_t   val16 = val;
    stkval_t *stk   = bc_stack_push_temp_str(2);
    memcpy(stk->val_str.p, &val16, 2);
}

void bc_func_mkl_s(void) {
    int32_t   val = bc_stack_pop_long();
    stkval_t *stk = bc_stack_push_temp_str(4);
    memcpy(stk->val_str.p, &val, 4);
}

void bc_func_mks_s(void) {
    stkval_t *stk = bc_stack_pop_num();
    bc_to_single(stk);
    float val = stk->val_single;

    stk = bc_stack_push_temp_str(4);
    memcpy(stk->val_str.p, &val, 4);
}

void bc_func_mkd_s(void) {
    stkval_t *stk = bc_stack_pop_num();
    bc_to_double(stk);
    double val = stk->val_double;

    stk = bc_stack_push_temp_str(8);
    memcpy(stk->val_str.p, &val, 8);
}

void bc_func_cvi(void) {
    stkval_t stk = *bc_stack_pop_str();
    if (stk.val_str.length < 2)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    int16_t val;
    memcpy(&val, stk.val_str.p, 2);
    bc_free_temp_val(&stk);
    bc_stack_push_long(val);
}

void bc_func_cvl(void) {
    stkval_t stk = *bc_stack_pop_str();
    if (stk.val_str.length < 4)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    int32_t val;
    memcpy(&val, stk.val_str.p, 4);
    bc_free_temp_val(&stk);
    bc_stack_push_long(val);
}

void bc_func_cvs(void) {
    stkval_t stk = *bc_stack_pop_str();
    if (stk.val_str.length < 4)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    float val;
    memcpy(&val, stk.val_str.p, 4);
    bc_free_temp_val(&stk);
    bc_stack_push_single(val);
}

void bc_func_cvd(void) {
    stkval_t stk = *bc_stack_pop_str();
    if (stk.val_str.length < 8)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    double val;
    memcpy(&val, stk.val_str.p, 8);
    bc_free_temp_val(&stk);
    bc_stack_push_double(val);
}

static void check_errno_result(int result) {
    if (result >= 0)
        return;

    int baserr = ERR_INTERNAL_ERROR;

    switch (errno) {
        case ENOENT: baserr = ERR_FILE_NOT_FOUND; break;
        case ENFILE: baserr = ERR_TOO_MANY_FILES; break;
        case EINVAL: baserr = ERR_ILLEGAL_FUNC_CALL; break;
        case EEXIST: baserr = ERR_FILE_ALREADY_EXISTS; break;
        case EIO: baserr = ERR_DEVICE_IO_ERROR; break;
        case ENODEV: baserr = ERR_DEVICE_UNAVAILABLE; break;
        case ENOTEMPTY: baserr = ERR_PATH_FILE_ACCESS_ERROR; break;
        case EROFS: baserr = ERR_PERMISSION_DENIED; break;
    }
    _basic_error(baserr);
}

static void get_str(char *tmp, unsigned sizeof_tmp_size) {
    stkval_t stk = *bc_stack_pop_str();
    if (stk.val_str.length == 0 || stk.val_str.length + 1U > sizeof_tmp_size)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    memcpy(tmp, stk.val_str.p, stk.val_str.length);
    tmp[stk.val_str.length] = 0;
    bc_free_temp_val(&stk);
}

void bc_func_open(void) {
    char path[256];
    int  mode = bc_stack_pop_long();
    get_str(path, sizeof(path));

    int idx = -1;
    for (int i = 0; i < MAX_OPEN; i++) {
        if (files[i].f == NULL) {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        _basic_error(ERR_TOO_MANY_FILES);

    const char *mode_str;
    switch (mode) {
        case OPEN_MODE_INPUT: mode_str = "rb"; break;
        case OPEN_MODE_OUTPUT: mode_str = "wb"; break;
        case OPEN_MODE_RANDOM: mode_str = "w+b"; break;
        case OPEN_MODE_APPEND: mode_str = "ab"; break;
        default: _basic_error(ERR_INTERNAL_ERROR);
    }

    if ((files[idx].f = fopen(path, mode_str)) == NULL)
        check_errno_result(-1);
    files[idx].column = 0;

    bc_stack_push_long(idx);
}

void bc_file_close_all(void) {
    for (int i = 0; i < MAX_OPEN; i++) {
        if (files[i].f != NULL) {
            fclose(files[i].f);
            files[i].f = NULL;
        }
    }
}

void bc_file_close(void) {
    int fn = bc_stack_pop_long();
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    fclose(files[fn].f);
    files[fn].f = NULL;
}

void file_io_read(int fn, void *buf, size_t len) {
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    if (fread(buf, len, 1, files[fn].f) != 1) {
        check_errno_result(-1);
    }
}

void file_io_write(int fn, const void *buf, size_t len) {
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    if (fwrite(buf, len, 1, files[fn].f) != 1) {
        check_errno_result(-1);
    }

    if (len == 1 && ((const char *)buf)[0] == '\n') {
        files[fn].column = 0;
    } else {
        files[fn].column += len;
    }
}

unsigned file_io_get_column(int fn) {
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);
    return files[fn].column;
}

void bc_file_read(void) {
    uint8_t  var_type = bc_get_u8();
    uint8_t *p_var    = &bc_state.p_vars[bc_get_u16()];

    unsigned len;
    switch (var_type) {
        case '%': len = sizeof(uint16_t); break;
        case '&': len = sizeof(uint32_t); break;
        case '!': len = sizeof(float); break;
        case '#': len = sizeof(double); break;
        case '$': {
            uint8_t *p_str;
            memcpy(&p_str, p_var, sizeof(uint8_t *));
            if (!p_str)
                return;

            len   = read_u16(p_str);
            p_var = p_str + 2;
            break;
        }
        default: _basic_error(ERR_INTERNAL_ERROR);
    }
    file_io_read(file_io_cur_file, p_var, len);
}

void file_io_readline(int fn, char *buf, size_t len) {
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    if (fgets(buf, len, files[fn].f) == NULL) {
        if (feof(files[fn].f))
            _basic_error(ERR_INPUT_PAST_END_OF_FILE);

        check_errno_result(-1);
    }
}

void bc_file_readline(void) {
    uint8_t *p_var = &bc_state.p_vars[bc_get_u16()];

    int  fn = bc_stack_pop_long();
    char tmp[256];
    file_io_readline(fn, tmp, sizeof(tmp));

    unsigned len = strlen(tmp);
    while (len > 0 && (tmp[len - 1] == '\r' || tmp[len - 1] == '\n'))
        len--;

    store_var_string(p_var, tmp, len);
}

void bc_file_write(void) {
    uint8_t  var_type = bc_get_u8();
    uint8_t *p_var    = &bc_state.p_vars[bc_get_u16()];

    unsigned len;
    switch (var_type) {
        case '%': len = sizeof(uint16_t); break;
        case '&': len = sizeof(uint32_t); break;
        case '!': len = sizeof(float); break;
        case '#': len = sizeof(double); break;
        case '$': {
            uint8_t *p_str;
            memcpy(&p_str, p_var, sizeof(uint8_t *));
            if (!p_str)
                return;

            len   = read_u16(p_str);
            p_var = p_str + 2;
            break;
        }
        default: _basic_error(ERR_INTERNAL_ERROR);
    }
    file_io_write(file_io_cur_file, p_var, len);
}

void bc_file_seek(void) {
    int fn = file_io_cur_file;
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    int offset = bc_stack_pop_long();
    if (offset < 1)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    check_errno_result(fseek(files[fn].f, offset - 1, SEEK_SET));
}

void bc_file_tell(void) {
    int fn = bc_stack_pop_long();
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    long result = ftell(files[fn].f);
    check_errno_result((int)result);
    bc_stack_push_long(result + 1);
}

void bc_file_size(void) {
    int fn = bc_stack_pop_long();
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    long cur_pos = ftell(files[fn].f);
    fseek(files[fn].f, 0, SEEK_END);
    long file_size = ftell(files[fn].f);
    fseek(files[fn].f, cur_pos, SEEK_SET);
    check_errno_result((int)file_size);
    bc_stack_push_long(file_size);
}

void bc_file_eof(void) {
    int fn = bc_stack_pop_long();
    if (fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    bool is_eof = false;
    int  c      = fgetc(files[fn].f);
    if (c == EOF) {
        is_eof = true;
    } else {
        ungetc(c, files[fn].f);
    }
    bc_stack_push_bool(is_eof);
}

void bc_file_inputs_s(void) {
    int fn = bc_stack_pop_long();
    int n  = bc_stack_pop_long();

    if (n < 0 || fn < 0 || fn >= MAX_OPEN || files[fn].f == NULL)
        _basic_error(ERR_ILLEGAL_FUNC_CALL);

    stkval_t *stk = bc_stack_push_temp_str(n);
    file_io_read(fn, stk->val_str.p, stk->val_str.length);
}

void bc_stmt_kill(void) {
    char tmp[256];
    get_str(tmp, sizeof(tmp));
    check_errno_result(unlink(tmp));
}

void bc_stmt_chdir(void) {
    char tmp[256];
    get_str(tmp, sizeof(tmp));
    check_errno_result(chdir(tmp));
}

void bc_stmt_mkdir(void) {
    char tmp[256];
    get_str(tmp, sizeof(tmp));
    check_errno_result(mkdir(tmp, 0777));
}

void bc_stmt_rmdir(void) {
    char tmp[256];
    get_str(tmp, sizeof(tmp));
    check_errno_result(rmdir(tmp));
}
