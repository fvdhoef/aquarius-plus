#ifndef _COMMON_H
#define _COMMON_H

#include "aqplus.h"
#include <stdbool.h>

#ifndef __SDCC
#include <unistd.h>
#endif

struct file_ctx {
    uint16_t linenr;
    int8_t   fd;
    char     path[29];
};

extern struct file_ctx *cur_file_ctx;
extern int8_t           fd_out;
extern char             basename[32];
extern const char      *filename_cb;

void error(const char *str);
void syntax_error(void);
void exit_program(void);
void check_esp_result(int16_t result);
void parse_file(const char *path);
void determine_basename(const char *path);

void push_file(const char *path);
void pop_file(void);

#endif
