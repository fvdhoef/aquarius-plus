#pragma once

#include <stdint.h>
#include <stdbool.h>

enum {
    DE_FLAG_ALWAYS_DIRS = 0x01, // Always return directories even if they don't match the wildcard
    DE_FLAG_HIDDEN      = 0x02, // Return hidden files (with system/hidden attribute or starting with a dot)
    DE_FLAG_DOTDOT      = 0x04, // Include a '..' entry if this is not the root directory
    DE_FLAG_MODE83      = 0x08, // Return entries in 8.3 mode
};

enum {
    DE_ATTR_DIR = (1 << 0),
};

struct esp_stat {
    uint16_t date;
    uint16_t time;
    uint8_t  attr;
    uint32_t size;
};

int esp_open(const char *path, uint8_t flags);
int esp_close(int fd);
int esp_read(int fd, void *buf, uint16_t length);
int esp_readline(int fd, void *buf, uint16_t max_length);
int esp_write(int fd, const void *buf, uint16_t length);
int esp_seek(int fd, uint32_t offset);
int esp_tell(int fd);
int esp_opendir(const char *path);
int esp_opendirext(const char *path, uint8_t flags, uint16_t skip_cnt);
int esp_closedir(int dd);
int esp_readdir(int dd, struct esp_stat *st, char *fn, uint8_t fn_buflen);
int esp_delete(const char *path);
int esp_rename(const char *path_old, const char *path_new);
int esp_mkdir(const char *path);
int esp_chdir(const char *path);
int esp_stat(const char *path, struct esp_stat *st);
int esp_getcwd(char *cwd, uint8_t cwd_buflen);
int esp_closeall(void);

bool load_binary(const char *path, void *addr, uint16_t max_length);
