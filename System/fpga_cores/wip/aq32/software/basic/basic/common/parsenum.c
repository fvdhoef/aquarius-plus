#include "parsenum.h"
#include "basic.h"
#include "ctype2.h"

int copy_num_to_buf(const uint8_t **ps, const uint8_t *ps_end, char *buf, size_t buf_size, char *type) {
    const uint8_t *ps_cur = *ps;
    char          *pd     = buf;
    char          *pd_end = buf + buf_size;
    int            base   = 0;

    // Read constant into tmp buf
    bool is_float = false;
    {
        if (ps_cur < ps_end && ps_cur[0] == '-')
            *(pd++) = *(ps_cur++);
        if (ps_cur[0] == '&') {
            ps_cur++;

            if (to_upper(ps_cur[0]) == 'B') {
                base = 2;
                ps_cur++;
                while (ps_cur < ps_end && pd < pd_end && ps_cur[0] >= '0' && ps_cur[0] <= '1')
                    *(pd++) = *(ps_cur++);

            } else if (to_upper(ps_cur[0]) == 'O') {
                base = 8;
                ps_cur++;
                while (ps_cur < ps_end && pd < pd_end && is_octal(ps_cur[0]))
                    *(pd++) = *(ps_cur++);

            } else if (to_upper(ps_cur[0]) == 'H') {
                base = 16;
                ps_cur++;
                while (ps_cur < ps_end && pd < pd_end && is_hexadecimal(ps_cur[0]))
                    *(pd++) = *(ps_cur++);

            } else {
                _basic_error(ERR_SYNTAX_ERROR);
            }

        } else {
            while (ps_cur < ps_end && pd < pd_end && is_decimal(ps_cur[0]))
                *(pd++) = *(ps_cur++);

            if (ps_cur < ps_end && pd < pd_end && ps_cur[0] == '.') {
                is_float = true;
                *(pd++)  = *(ps_cur++);

                while (ps_cur < ps_end && pd < pd_end && is_decimal(ps_cur[0]))
                    *(pd++) = *(ps_cur++);
            }

            if (ps_cur < ps_end && pd < pd_end && (ps_cur[0] == 'e' || ps_cur[0] == 'E')) {
                is_float = true;
                *(pd++)  = *(ps_cur++);

                if (ps_cur < ps_end && pd < pd_end && (ps_cur[0] == '-' || ps_cur[0] == '+'))
                    *(pd++) = *(ps_cur++);

                while (ps_cur < ps_end && pd < pd_end && is_decimal(ps_cur[0]))
                    *(pd++) = *(ps_cur++);
            }
        }

        if (pd >= pd_end)
            _basic_error(ERR_OUT_OF_MEM);

        *pd = 0;
    }

    if (pd == buf || (pd == buf + 1 && buf[0] == '-')) {
        // String empty or only contains '-', not a number.
        return -1;
    }

    if (type) {
        *type = 0;
        if (ps_cur < ps_end && is_typechar(ps_cur[0]))
            *type = *(ps_cur++);

        if (*type == '!' || *type == '#')
            is_float = true;

        if (is_float && (*type == '%' || *type == '&' || base > 0))
            _basic_error(ERR_TYPE_MISMATCH);
    }
    if (base == 0 && !is_float)
        base = 10;

    *ps = ps_cur;
    return base;
}
