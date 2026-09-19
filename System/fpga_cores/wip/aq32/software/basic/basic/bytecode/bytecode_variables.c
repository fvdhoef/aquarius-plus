#include "bytecode_internal.h"

enum {
    ARRAY_TYPE_INT,
    ARRAY_TYPE_LONG,
    ARRAY_TYPE_SINGLE,
    ARRAY_TYPE_DOUBLE,
    ARRAY_TYPE_STRING,
};

static uint8_t at_sizes[] = {
    [ARRAY_TYPE_INT]    = sizeof(int16_t),
    [ARRAY_TYPE_LONG]   = sizeof(int32_t),
    [ARRAY_TYPE_SINGLE] = sizeof(float),
    [ARRAY_TYPE_DOUBLE] = sizeof(double),
    [ARRAY_TYPE_STRING] = sizeof(uint8_t *),
};

// Simple types
static void _push_var_int(uint8_t *p_var) { bc_stack_push_long((int16_t)read_u16(p_var)); }
static void _push_var_long(uint8_t *p_var) { bc_stack_push_long((int32_t)read_u32(p_var)); }
static void _push_var_single(uint8_t *p_var) {
    uint32_t u32 = read_u32(p_var);
    bc_stack_push_single(*(float *)&u32);
}
static void _push_var_double(uint8_t *p_var) {
    uint64_t u64 = read_u64(p_var);
    bc_stack_push_double(*(double *)&u64);
}

void _store_var_int(uint8_t *p_var) {
    stkval_t *val = bc_stack_pop();
    bc_to_long_round(val);
    if (val->val_long < INT16_MIN || val->val_long >= INT16_MAX)
        _basic_error(ERR_OVERFLOW);
    write_u16(p_var, val->val_long);
}
void _store_var_long(uint8_t *p_var) {
    stkval_t *val = bc_stack_pop();
    bc_to_long_round(val);
    write_u32(p_var, val->val_long);
}
void _store_var_single(uint8_t *p_var) {
    stkval_t *val = bc_stack_pop();
    bc_to_single(val);
    write_u32(p_var, val->val_long);
}
void _store_var_double(uint8_t *p_var) {
    stkval_t *val = bc_stack_pop();
    bc_to_double(val);
    write_u64(p_var, val->val_longlong);
}

// String type, stored as:
// u16        length
// u8[length] str

static void _push_var_string(uint8_t *p_var) {
    uint8_t *p_str;
    memcpy(&p_str, p_var, sizeof(uint8_t *));

    stkval_t *stk       = bc_stack_push();
    stk->type           = VT_STR;
    stk->val_str.flags  = STR_FLAGS_TYPE_VAR;
    stk->val_str.p      = (p_str == NULL) ? NULL : p_str + 2;
    stk->val_str.length = (p_str == NULL) ? 0 : read_u16(p_str);
}

void store_var_string(uint8_t *p_var, const char *str, unsigned len) {
    uint8_t *p_str;
    memcpy(&p_str, p_var, sizeof(uint8_t *));

    p_str = buf_realloc(p_str, 2 + len);
    if (p_str == NULL)
        _basic_error(ERR_OUT_OF_MEM);
    memcpy(p_var, &p_str, sizeof(uint8_t *));

    write_u16(p_str, len);
    memcpy(p_str + 2, str, len);
}

static void _store_var_string(uint8_t *p_var) {
    stkval_t *stk = bc_stack_pop_str();
    store_var_string(p_var, (const char *)stk->val_str.p, stk->val_str.length);
    bc_free_temp_val(stk);
}

//////////////////////////////////////////////////////////////////////////////

// Array stored as:
// u8                   element_type
// u8                   num_dimensions
// u16[num_dimensions]  dimension_size
// u8[...]              array_data

static void _allocate_array(uint8_t *p_var, uint8_t element_type, uint8_t num_dimensions, const int *dimension_sizes) {
    uint8_t *p_arr;
    memcpy(&p_arr, p_var, sizeof(uint8_t *));

    if (p_arr != NULL)
        _basic_error(ERR_DUPLICATE_DEFINITION);

    unsigned total_elements = 1;
    for (int i = 0; i < num_dimensions; i++) {
        int sz = (dimension_sizes != NULL) ? dimension_sizes[i] : ARRAY_DEFAULT_DIMENSION_SIZE;
        if (sz < 1 || sz > 65535)
            _basic_error(ERR_SUBSCRIPT_OUT_OF_RANGE);
        total_elements *= sz;
    }

    unsigned array_data_size = total_elements * at_sizes[element_type];
    p_arr                    = buf_calloc(1 + 1 + sizeof(uint16_t) * num_dimensions + array_data_size);
    if (p_arr == NULL)
        _basic_error(ERR_OUT_OF_MEM);
    memcpy(p_var, &p_arr, sizeof(uint8_t *));

    p_arr[0] = element_type;
    p_arr[1] = num_dimensions;
    for (int i = 0; i < num_dimensions; i++)
        write_u16(p_arr + 2 + i * 2, (dimension_sizes != NULL) ? dimension_sizes[i] : ARRAY_DEFAULT_DIMENSION_SIZE);
}

static void *_get_array_val_p(unsigned element_type) {
    uint8_t  num_dimensions = bc_get_u8();
    uint8_t *p_var          = &bc_state.p_vars[bc_get_u16()];
    uint8_t *p_arr;
    memcpy(&p_arr, p_var, sizeof(uint8_t *));
    if (p_arr == NULL) {
        _allocate_array(p_var, element_type, num_dimensions, NULL);
        memcpy(&p_arr, p_var, sizeof(uint8_t *));
    }
    if (p_arr[0] != element_type)
        _basic_error(ERR_TYPE_MISMATCH);
    if (p_arr[1] != num_dimensions)
        _basic_error(ERR_SUBSCRIPT_OUT_OF_RANGE);

    unsigned element_sz = at_sizes[element_type];

    // Get dimension sizes from array record
    unsigned dimension_sizes[ARRAY_MAX_DIMENSIONS];
    for (int i = 0; i < num_dimensions; i++)
        dimension_sizes[i] = read_u16(p_arr + 1 + 1 + i * sizeof(uint16_t));

    // Get indices from stack
    unsigned multiplier = 1;
    unsigned offset     = 0;

    for (int i = num_dimensions - 1; i >= 0; i--) {
        int idx = bc_stack_pop_long();
        if (idx < 0 || (unsigned)idx >= dimension_sizes[i])
            _basic_error(ERR_SUBSCRIPT_OUT_OF_RANGE);

        offset += idx * multiplier;
        multiplier *= dimension_sizes[i];
    }

    uint8_t *p_data = p_arr + 1 + 1 + num_dimensions * sizeof(uint16_t) + (offset * element_sz);
    return p_data;
}

static void _dim_array(unsigned element_type) {
    uint8_t num_dimensions = bc_get_u8();
    int     dimension_sizes[ARRAY_MAX_DIMENSIONS];
    for (int i = num_dimensions - 1; i >= 0; i--)
        dimension_sizes[i] = bc_stack_pop_long() + 1;

    _allocate_array(&bc_state.p_vars[bc_get_u16()], element_type, num_dimensions, dimension_sizes);
}

static void _free_array(void) {
    uint8_t *p_var = &bc_state.p_vars[bc_get_u16()];
    uint8_t *p_arr;
    memcpy(&p_arr, p_var, sizeof(uint8_t *));
    memset(p_var, 0, sizeof(*p_var));

    if (p_arr == NULL)
        return;

    if (p_arr[0] == ARRAY_TYPE_STRING) {
        // String array needs freeing of strings as well

        // Calculate number of elements
        unsigned num_elements = 1;
        for (int i = 0; i < p_arr[1]; i++)
            num_elements *= read_u16(p_arr + 2 + i * sizeof(uint16_t));

        uint8_t *p_data = p_arr + 2 + p_arr[1] * sizeof(uint16_t);

        for (unsigned i = 0; i < num_elements; i++) {
            uint8_t *p_str;
            memcpy(&p_str, p_data + i * sizeof(uint8_t *), sizeof(uint8_t *));
            if (p_str != NULL) {
                buf_free(p_str);
            }
        }
    }

    buf_free(p_arr);
}

void bc_push_var_int(void) { _push_var_int(&bc_state.p_vars[bc_get_u16()]); }
void bc_push_var_long(void) { _push_var_long(&bc_state.p_vars[bc_get_u16()]); }
void bc_push_var_single(void) { _push_var_single(&bc_state.p_vars[bc_get_u16()]); }
void bc_push_var_double(void) { _push_var_double(&bc_state.p_vars[bc_get_u16()]); }
void bc_push_var_string(void) { _push_var_string(&bc_state.p_vars[bc_get_u16()]); }
void bc_store_var_int(void) { _store_var_int(&bc_state.p_vars[bc_get_u16()]); }
void bc_store_var_long(void) { _store_var_long(&bc_state.p_vars[bc_get_u16()]); }
void bc_store_var_single(void) { _store_var_single(&bc_state.p_vars[bc_get_u16()]); }
void bc_store_var_double(void) { _store_var_double(&bc_state.p_vars[bc_get_u16()]); }
void bc_store_var_string(void) { _store_var_string(&bc_state.p_vars[bc_get_u16()]); }
void bc_push_array_int(void) { _push_var_int(_get_array_val_p(ARRAY_TYPE_INT)); }
void bc_push_array_long(void) { _push_var_long(_get_array_val_p(ARRAY_TYPE_LONG)); }
void bc_push_array_single(void) { _push_var_single(_get_array_val_p(ARRAY_TYPE_SINGLE)); }
void bc_push_array_double(void) { _push_var_double(_get_array_val_p(ARRAY_TYPE_DOUBLE)); }
void bc_push_array_string(void) { _push_var_string(_get_array_val_p(ARRAY_TYPE_STRING)); }
void bc_store_array_int(void) { _store_var_int(_get_array_val_p(ARRAY_TYPE_INT)); }
void bc_store_array_long(void) { _store_var_long(_get_array_val_p(ARRAY_TYPE_LONG)); }
void bc_store_array_single(void) { _store_var_single(_get_array_val_p(ARRAY_TYPE_SINGLE)); }
void bc_store_array_double(void) { _store_var_double(_get_array_val_p(ARRAY_TYPE_DOUBLE)); }
void bc_store_array_string(void) { _store_var_string(_get_array_val_p(ARRAY_TYPE_STRING)); }
void bc_dim_array_int(void) { _dim_array(ARRAY_TYPE_INT); }
void bc_dim_array_long(void) { _dim_array(ARRAY_TYPE_LONG); }
void bc_dim_array_single(void) { _dim_array(ARRAY_TYPE_SINGLE); }
void bc_dim_array_double(void) { _dim_array(ARRAY_TYPE_DOUBLE); }
void bc_dim_array_string(void) { _dim_array(ARRAY_TYPE_STRING); }
void bc_free_array(void) { _free_array(); }
