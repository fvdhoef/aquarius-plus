#ifndef _ARGMATCH_H
#define _ARGMATCH_H

#include "common.h"

enum {
    OT_NONE,
    OT_8BIT,
    OT_16BIT,
    OT_D         = (1 << 5),
    OT_PREFIX_DD = (1 << 6),
    OT_PREFIX_FD = (2 << 6),
};
extern uint8_t  cur_outtype;
extern uint16_t arg_value;
extern uint8_t  d_value;

bool match_argtype(char *arg, uint8_t arg_type);

#endif
