#pragma once

#include <stddef.h>
#include "regs.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void esp_send_byte(uint8_t val) {
    while (ESP_STATUS & ESP_STATUS_TXF) {
    }
    ESP_DATA = val;
}

static inline uint8_t esp_get_byte(void) {
    while ((ESP_STATUS & ESP_STATUS_RXNE) == 0) {
    }
    return ESP_DATA;
}

static inline void esp_cmd(uint8_t cmd) {
    while (ESP_STATUS & ESP_STATUS_RXNE) {
        (void)ESP_DATA;
    }

    while (ESP_STATUS & ESP_STATUS_TXF) {
    }
    ESP_DATA = ESP_DATA_START_OF_MSG;
    esp_send_byte(cmd);
}

static inline void esp_send_bytes(const void *buf, unsigned length) {
    const uint8_t *p = (const uint8_t *)buf;
    while (length--) {
        esp_send_byte(*(p++));
    }
}

static inline void esp_send_str(const char *s) {
    while (1) {
        uint8_t ch = *(s++);
        esp_send_byte(ch);
        if (ch == 0)
            break;
    }
}

static inline void esp_get_bytes(void *buf, unsigned length) {
    uint8_t *p = (uint8_t *)buf;
    while (length--) {
        *(p++) = esp_get_byte();
    }
}

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
int esp_read(int fd, void *buf, size_t count);
int esp_readline(int fd, void *buf, uint16_t max_length);
int esp_write(int fd, const void *buf, size_t count);
int esp_seek(int fd, uint32_t offset);
int esp_lseek(int fd, int offset, int whence);
int esp_tell(int fd);
int esp_opendir(const char *path);
int esp_opendirext(const char *path, uint8_t flags, uint16_t skip_cnt);
int esp_closedir(int dd);
int esp_readdir(int dd, struct esp_stat *st, char *fn, size_t fn_buflen);
int esp_delete(const char *path);
int esp_rename(const char *path_old, const char *path_new);
int esp_mkdir(const char *path);
int esp_chdir(const char *path);
int esp_stat(const char *path, struct esp_stat *st);
int esp_getcwd(char *cwd, size_t cwd_buflen);
int esp_closeall(void);

int esp_set_errno(int esp_err);

int esp_get_midi_data(void *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif
