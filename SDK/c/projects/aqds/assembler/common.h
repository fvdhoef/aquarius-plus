#ifndef _COMMON_H
#define _COMMON_H

#include "aqplus.h"
#include <stdbool.h>

#ifndef __SDCC
#include <unistd.h>
#endif

void error(const char *str);
void syntax_error(void);

void    skip_whitespace(void);
uint8_t to_lower(uint8_t ch);

extern char    *cur_p;
extern uint16_t cur_scope; // Current variable scope (used for local variables starting with a '.')
extern uint8_t  cur_pass;  // Current assembler pass (0-based)

#endif
