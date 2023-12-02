#ifndef _COMMON_H
#define _COMMON_H

#include "aqplus.h"
#include <stdbool.h>

#ifndef __SDCC
#include <unistd.h>
#endif

void error(const char *str);
void syntax_error(void);

#endif
