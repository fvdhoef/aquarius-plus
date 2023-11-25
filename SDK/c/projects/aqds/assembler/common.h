#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void    error(char *str);
void    skip_whitespace(void);
uint8_t to_lower(uint8_t ch);

extern char    *cur_p;
extern uint16_t cur_scope;

#endif
