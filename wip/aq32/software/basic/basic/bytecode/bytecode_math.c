#include "bytecode_internal.h"

void bc_op_pow(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_promote_types_flt(val_l, val_r);
    switch (val_l->type) {
        case VT_SINGLE: bc_stack_push_single(powf(val_l->val_single, val_r->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(pow(val_l->val_double, val_r->val_double)); break;
    }
}

void bc_op_negate(void) {
    stkval_t *val = bc_stack_pop_num();
    switch (val->type) {
        case VT_LONG: bc_stack_push_long(-val->val_long); break;
        case VT_SINGLE: bc_stack_push_single(-val->val_single); break;
        case VT_DOUBLE: bc_stack_push_double(-val->val_double); break;
    }
}

void bc_op_mult(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_long(val_l->val_long * val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_single(val_l->val_single * val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_double(val_l->val_double * val_r->val_double); break;
    }
}

void bc_op_div(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_promote_types_flt(val_l, val_r);
    switch (val_l->type) {
        case VT_SINGLE: bc_stack_push_single(val_l->val_single / val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_double(val_l->val_double / val_r->val_double); break;
    }
}

void bc_op_intdiv(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    if (val_r->val_long == 0)
        _basic_error(ERR_DIV_BY_ZERO);

    bc_stack_push_long(val_l->val_long / val_r->val_long);
}

void bc_op_mod(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    if (val_r->val_long == 0)
        _basic_error(ERR_DIV_BY_ZERO);

    bc_stack_push_long(val_l->val_long % val_r->val_long);
}

void bc_op_add(void) {
    stkval_t val_r = *bc_stack_pop();
    stkval_t val_l = *bc_stack_pop();
    if (val_l.type == VT_STR && val_r.type == VT_STR) {
        stkval_t *stk = bc_stack_push_temp_str(val_l.val_str.length + val_r.val_str.length);
        memcpy(stk->val_str.p, val_l.val_str.p, val_l.val_str.length);
        memcpy(stk->val_str.p + val_l.val_str.length, val_r.val_str.p, val_r.val_str.length);
        bc_free_temp_val(&val_l);
        bc_free_temp_val(&val_r);
        return;
    }

    bc_promote_types(&val_l, &val_r);

    switch (val_l.type) {
        case VT_LONG: bc_stack_push_long(val_l.val_long + val_r.val_long); break;
        case VT_SINGLE: bc_stack_push_single(val_l.val_single + val_r.val_single); break;
        case VT_DOUBLE: bc_stack_push_double(val_l.val_double + val_r.val_double); break;
    }
}

void bc_op_sub(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_long(val_l->val_long - val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_single(val_l->val_single - val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_double(val_l->val_double - val_r->val_double); break;
    }
}

void bc_op_eq(void) {
    stkval_t *val_r = bc_stack_pop();
    stkval_t *val_l = bc_stack_pop();
    if (val_l->type == VT_STR && val_r->type == VT_STR) {
        bool result =
            val_l->val_str.length == val_r->val_str.length &&
            memcmp(val_l->val_str.p, val_r->val_str.p, val_l->val_str.length) == 0;

        bc_free_temp_val(val_l);
        bc_free_temp_val(val_r);
        bc_stack_push_bool(result);
        return;
    }
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_bool(val_l->val_long == val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_bool(val_l->val_single == val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_bool(val_l->val_double == val_r->val_double); break;
    }
}

void bc_op_ne(void) {
    stkval_t *val_r = bc_stack_pop();
    stkval_t *val_l = bc_stack_pop();
    if (val_l->type == VT_STR && val_r->type == VT_STR) {
        bool result =
            val_l->val_str.length != val_r->val_str.length ||
            memcmp(val_l->val_str.p, val_r->val_str.p, val_l->val_str.length) != 0;

        bc_free_temp_val(val_l);
        bc_free_temp_val(val_r);
        bc_stack_push_bool(result);
        return;
    }
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_bool(val_l->val_long != val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_bool(val_l->val_single != val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_bool(val_l->val_double != val_r->val_double); break;
    }
}

void bc_op_lt(void) {
    stkval_t *val_r = bc_stack_pop();
    stkval_t *val_l = bc_stack_pop();
    if (val_l->type == VT_STR && val_r->type == VT_STR) {
        bool result = false;

        int n   = min(val_l->val_str.length, val_r->val_str.length);
        int res = memcmp(val_l->val_str.p, val_r->val_str.p, n);
        if (res < 0 || (res == 0 && val_l->val_str.length < val_r->val_str.length))
            result = true;

        bc_free_temp_val(val_l);
        bc_free_temp_val(val_r);
        bc_stack_push_bool(result);
        return;
    }
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_bool(val_l->val_long < val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_bool(val_l->val_single < val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_bool(val_l->val_double < val_r->val_double); break;
    }
}

void bc_op_le(void) {
    stkval_t *val_r = bc_stack_pop();
    stkval_t *val_l = bc_stack_pop();
    if (val_l->type == VT_STR && val_r->type == VT_STR) {
        bool result = false;

        int n   = min(val_l->val_str.length, val_r->val_str.length);
        int res = memcmp(val_l->val_str.p, val_r->val_str.p, n);
        if (res < 0 || (res == 0 && val_l->val_str.length <= val_r->val_str.length))
            result = true;

        bc_free_temp_val(val_l);
        bc_free_temp_val(val_r);
        bc_stack_push_bool(result);
        return;
    }
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_bool(val_l->val_long <= val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_bool(val_l->val_single <= val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_bool(val_l->val_double <= val_r->val_double); break;
    }
}

void bc_op_gt(void) {
    stkval_t *val_r = bc_stack_pop();
    stkval_t *val_l = bc_stack_pop();
    if (val_l->type == VT_STR && val_r->type == VT_STR) {
        bool result = false;

        int n   = min(val_l->val_str.length, val_r->val_str.length);
        int res = memcmp(val_l->val_str.p, val_r->val_str.p, n);
        if (res > 0 || (res == 0 && val_l->val_str.length > val_r->val_str.length))
            result = true;

        bc_free_temp_val(val_l);
        bc_free_temp_val(val_r);
        bc_stack_push_bool(result);
        return;
    }
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_bool(val_l->val_long > val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_bool(val_l->val_single > val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_bool(val_l->val_double > val_r->val_double); break;
    }
}

void bc_op_ge(void) {
    stkval_t *val_r = bc_stack_pop();
    stkval_t *val_l = bc_stack_pop();
    if (val_l->type == VT_STR && val_r->type == VT_STR) {
        bool result = false;

        int n   = min(val_l->val_str.length, val_r->val_str.length);
        int res = memcmp(val_l->val_str.p, val_r->val_str.p, n);
        if (res > 0 || (res == 0 && val_l->val_str.length >= val_r->val_str.length))
            result = true;

        bc_free_temp_val(val_l);
        bc_free_temp_val(val_r);
        bc_stack_push_bool(result);
        return;
    }
    bc_promote_types(val_l, val_r);

    switch (val_l->type) {
        case VT_LONG: bc_stack_push_bool(val_l->val_long >= val_r->val_long); break;
        case VT_SINGLE: bc_stack_push_bool(val_l->val_single >= val_r->val_single); break;
        case VT_DOUBLE: bc_stack_push_bool(val_l->val_double >= val_r->val_double); break;
    }
}

void bc_op_not(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_long_round(val);
    bc_stack_push_long(-(val->val_long + 1));
}

void bc_op_and(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    bc_stack_push_long(val_l->val_long & val_r->val_long);
}

void bc_op_or(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    bc_stack_push_long(val_l->val_long | val_r->val_long);
}

void bc_op_xor(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    bc_stack_push_long(val_l->val_long ^ val_r->val_long);
}

void bc_op_eqv(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    bc_stack_push_long(~(val_l->val_long ^ val_r->val_long));
}

void bc_op_imp(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_to_long_round(val_l);
    bc_to_long_round(val_r);
    bc_stack_push_long(~(val_l->val_long & ~val_r->val_long));
}

void bc_op_inc(void) {
    stkval_t *val = bc_stack_pop_num();
    switch (val->type) {
        case VT_LONG: bc_stack_push_long(val->val_long + 1); break;
        case VT_SINGLE: bc_stack_push_single(val->val_single + 1.0f); break;
        case VT_DOUBLE: bc_stack_push_double(val->val_double + 1.0); break;
    }
}

void bc_op_le_ge(void) {
    stkval_t *val_r = bc_stack_pop_num();
    stkval_t *val_l = bc_stack_pop_num();
    bc_promote_types(val_l, val_r);

    bool is_neg = false;

    stkval_t *val_step = bc_stack_pop_num();
    switch (val_step->type) {
        case VT_LONG: is_neg = val_step->val_long < 0; break;
        case VT_SINGLE: is_neg = val_step->val_single < 0.0f; break;
        case VT_DOUBLE: is_neg = val_step->val_double < 0.0; break;
    }

    if (is_neg) {
        switch (val_l->type) {
            case VT_LONG: bc_stack_push_bool(val_l->val_long >= val_r->val_long); break;
            case VT_SINGLE: bc_stack_push_bool(val_l->val_single >= val_r->val_single); break;
            case VT_DOUBLE: bc_stack_push_bool(val_l->val_double >= val_r->val_double); break;
        }
    } else {
        switch (val_l->type) {
            case VT_LONG: bc_stack_push_bool(val_l->val_long <= val_r->val_long); break;
            case VT_SINGLE: bc_stack_push_bool(val_l->val_single <= val_r->val_single); break;
            case VT_DOUBLE: bc_stack_push_bool(val_l->val_double <= val_r->val_double); break;
        }
    }
}

void bc_func_abs(void) {
    stkval_t *val = bc_stack_pop_num();
    switch (val->type) {
        case VT_LONG: bc_stack_push_long(val->val_long < 0 ? -val->val_long : val->val_long); break;
        case VT_SINGLE: bc_stack_push_single(val->val_single < 0 ? -val->val_single : val->val_single); break;
        case VT_DOUBLE: bc_stack_push_double(val->val_double < 0 ? -val->val_double : val->val_double); break;
    }
}

void bc_func_atn(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE: bc_stack_push_single(atanf(val->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(atan(val->val_double)); break;
    }
}

void bc_func_tan(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE: bc_stack_push_single(tanf(val->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(tan(val->val_double)); break;
    }
}

void bc_func_cos(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE: bc_stack_push_single(cosf(val->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(cos(val->val_double)); break;
    }
}

void bc_func_sin(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE: bc_stack_push_single(sinf(val->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(sin(val->val_double)); break;
    }
}

void bc_func_sqr(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE:
            if (val->val_single < 0)
                _basic_error(ERR_ILLEGAL_FUNC_CALL);
            bc_stack_push_single(sqrtf(val->val_single));
            break;
        case VT_DOUBLE:
            if (val->val_double < 0)
                _basic_error(ERR_ILLEGAL_FUNC_CALL);
            bc_stack_push_double(sqrt(val->val_double));
            break;
    }
}

void bc_func_exp(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE: bc_stack_push_single(expf(val->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(exp(val->val_double)); break;
    }
}

void bc_func_fix(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE: bc_stack_push_single(truncf(val->val_single)); break;
        case VT_DOUBLE: bc_stack_push_double(trunc(val->val_double)); break;
    }
}

void bc_func_sgn(void) {
    stkval_t *val = bc_stack_pop_num();

    int result = 0;
    switch (val->type) {
        case VT_LONG:
            if (val->val_long < 0)
                result = -1;
            else if (val->val_long > 0)
                result = 1;
            break;

        case VT_SINGLE:
            if (val->val_single < 0)
                result = -1;
            else if (val->val_single > 0)
                result = 1;
            break;
        case VT_DOUBLE:
            if (val->val_double < 0)
                result = -1;
            else if (val->val_double > 0)
                result = 1;
            break;
    }
    bc_stack_push_long(result);
}

void bc_func_log(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);
    switch (val->type) {
        case VT_SINGLE:
            if (val->val_single < 0)
                _basic_error(ERR_ILLEGAL_FUNC_CALL);
            bc_stack_push_single(logf(val->val_single));
            break;
        case VT_DOUBLE:
            if (val->val_double < 0)
                _basic_error(ERR_ILLEGAL_FUNC_CALL);
            bc_stack_push_double(log(val->val_double));
            break;
    }
}

void bc_func_int(void) {
    stkval_t *val = bc_stack_pop_num();

    int result = 0;
    switch (val->type) {
        case VT_LONG: result = val->val_long; break;
        case VT_SINGLE: result = (int)floorf(val->val_single); break;
        case VT_DOUBLE: result = (int)floor(val->val_double); break;
    }
    bc_stack_push_long(result);
}

/* From newlib:
   Pseudo-random generator based on Minimal Standard by
   Lewis, Goodman, and Miller in 1969.

   I[j+1] = a*I[j] (mod m)

   where a = 16807
         m = 2147483647

   Using Schrage's algorithm, a*I[j] (mod m) can be rewritten as:

     a*(I[j] mod q) - r*{I[j]/q}      if >= 0
     a*(I[j] mod q) - r*{I[j]/q} + m  otherwise

   where: {} denotes integer division
          q = {m/a} = 127773
          r = m (mod a) = 2836

   note that the seed value of 0 cannot be used in the calculation as
   it results in 0 itself
*/
static int myrand_r(uint32_t *seed) {
    int s = *seed;
    if (s < 0)
        s += INT32_MAX;
    if (s == 0)
        s = 0x12345987;

    int k = s / 127773;
    s     = 16807 * (s - k * 127773) - 2836 * k;
    if (s < 0)
        s += INT32_MAX;

    *seed = (uint32_t)s;
    return (int)(s & INT32_MAX);
}

static uint32_t cur_seed = 0;

void bc_func_rnd(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_single(val);

    int result;

    if (val->val_single < 0) {
        uint32_t seed = val->val_long;
        result        = myrand_r(&seed);

    } else if (val->val_single == 0) {
        uint32_t seed = cur_seed;
        result        = myrand_r(&seed);

    } else { // if (val->val_single > 0) {
        result = myrand_r(&cur_seed);
    }

    bc_stack_push_single((float)result / (float)INT32_MAX);
}

void bc_stmt_randomize(void) {
    stkval_t *val = bc_stack_pop_num();
    bc_to_long_round(val);

    cur_seed = val->val_long;
}
