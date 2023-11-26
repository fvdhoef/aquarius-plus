#ifndef _FILE_IO_H
#define _FILE_IO_H

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

int8_t  esp_open(const char *path, uint8_t flags);
int8_t  esp_close(int8_t fd);
int16_t esp_read(int8_t fd, void *buf, uint16_t length);
int16_t esp_write(int8_t fd, const void *buf, uint16_t length);

int8_t  esp_seek(int8_t fd, uint32_t offset);
int32_t esp_tell(int8_t fd);
int8_t  esp_opendir(const char *path);
int8_t  esp_opendirext(const char *path, uint8_t flags, uint16_t skip_cnt);
int8_t  esp_closedir(int8_t dd);
int8_t  esp_readdir(int8_t dd, struct esp_stat *st, char *fn, uint8_t fn_buflen);
int8_t  esp_delete(const char *path);
int8_t  esp_rename(const char *path_old, const char *path_new);
int8_t  esp_mkdir(const char *path);
int8_t  esp_chdir(const char *path);
int8_t  esp_stat(const char *path, struct esp_stat *st);
int8_t  esp_getcwd(char *cwd, uint8_t cwd_buflen);
int8_t  esp_closeall(void);

bool load_binary(const char *path, void *addr, uint16_t max_length);

#endif
