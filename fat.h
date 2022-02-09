// FAT emulation layer for CH376

#pragma once

#include "common.h"

struct fat_dirent {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t filesize;
};

enum {
    ERR_INVALID_NAME = -1,
    ERR_EOF          = -2,
    OPEN_IS_DIR      = 1,
};

int fat_init(const char *basepath);
int fat_open(const char *name);
int fat_close(void);
int fat_read(void *buf, size_t size);
int fat_delete(const char *name);
