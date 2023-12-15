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
extern char             basename[32];
extern const char      *filename_cb;
extern char             tmpbuf[256];

void error(const char *fmt, ...);
void syntax_error(void);
void eof_error(void);
void exit_program(void);
void check_esp_result(int16_t result);
void determine_basename(const char *path);

void output_puts(const char *str, int len);

void push_file(const char *path);
bool pop_file(void);

void expect_tok_ack(uint8_t token);
void expect_tok(uint8_t token);

#endif
