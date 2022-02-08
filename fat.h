// FAT emulation layer for CH376

#pragma once

#include "common.h"

int fat_init(const char *basepath);
int fat_open(const char *name);
int fat_close(void);
int fat_read(void *buf, size_t size);
int fat_delete(const char *name);
