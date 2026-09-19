#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int readline(char *buf, size_t buf_size);
int readline_no_newline(char *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif
