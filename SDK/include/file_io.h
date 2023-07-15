#ifndef _FILE_IO_H
#define _FILE_IO_H

#include <stdint.h>
#include <stdbool.h>

int8_t  open(const char *path, uint8_t flags);
int16_t read(int8_t fd, void *buf, uint16_t length);
int16_t write(int8_t fd, const void *buf, uint16_t length);
int8_t  close(int8_t fd);

bool load_binary(const char *path, void *addr, uint16_t max_length);

#endif
