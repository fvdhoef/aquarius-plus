#ifndef _FILE_IO_H
#define _FILE_IO_H

#include <stdint.h>
#include <stdbool.h>

struct stat {
    uint16_t date;
    uint16_t time;
    uint8_t  attr;
    uint32_t size;
};

int8_t  open(const char *path, uint8_t flags);
int8_t  close(int8_t fd);
int16_t read(int8_t fd, void *buf, uint16_t length);
int16_t write(int8_t fd, const void *buf, uint16_t length);

int8_t  seek(int8_t fd, uint32_t offset);
int32_t tell(int8_t fd);
int8_t  opendir(const char *path);
int8_t  closedir(int8_t dd);
int8_t  readdir(int8_t dd, struct stat *st, char *fn, uint8_t fn_buflen);
int8_t delete(const char *path);
int8_t rename(const char *path_old, const char *path_new);
int8_t mkdir(const char *path);
int8_t chdir(const char *path);
int8_t stat(const char *path, struct stat *st);
int8_t getcwd(char *cwd, uint8_t cwd_buflen);
int8_t closeall(void);

bool load_binary(const char *path, void *addr, uint16_t max_length);

#endif
