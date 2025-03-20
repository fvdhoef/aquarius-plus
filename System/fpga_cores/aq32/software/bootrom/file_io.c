#include "file_io.h"
#include "esp.h"
#include <stddef.h>

static size_t strlen(const char *s) {
    size_t result = 0;
    while (*s) {
        s++;
        result++;
    }
    return result;
}

int esp_chdir(const char *path) {
    esp_cmd(ESPCMD_CHDIR);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int esp_closeall(void) {
    esp_cmd(ESPCMD_CLOSEALL);
    return (int8_t)esp_get_byte();
}

int esp_close(int fd) {
    esp_cmd(ESPCMD_CLOSE);
    esp_send_byte(fd);
    return (int8_t)esp_get_byte();
}

int esp_closedir(int dd) {
    esp_cmd(ESPCMD_CLOSEDIR);
    esp_send_byte(dd);
    return (int8_t)esp_get_byte();
}

int esp_delete(const char *path) {
    esp_cmd(ESPCMD_DELETE);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int esp_getcwd(char *cwd, uint8_t cwd_buflen) {
    if (cwd == NULL || cwd_buflen < 1)
        return ERR_PARAM;

    esp_cmd(ESPCMD_GETCWD);

    int result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }

    while (1) {
        uint8_t val = esp_get_byte();
        if (val == 0)
            break;

        if (cwd_buflen > 1) {
            *(cwd++) = val;
            cwd_buflen--;
        }
    }
    *cwd = '\0';

    return result;
}

int esp_mkdir(const char *path) {
    esp_cmd(ESPCMD_MKDIR);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int esp_open(const char *path, uint8_t flags) {
    esp_cmd(ESPCMD_OPEN);
    esp_send_byte(flags);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int esp_opendir(const char *path) {
    esp_cmd(ESPCMD_OPENDIR);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int esp_opendirext(const char *path, uint8_t flags, uint16_t skip_cnt) {
    esp_cmd(ESPCMD_OPENDIREXT);
    esp_send_byte(flags);
    esp_send_byte(skip_cnt & 0xFF);
    esp_send_byte(skip_cnt >> 8);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int esp_read(int fd, void *buf, uint16_t length) {
    esp_cmd(ESPCMD_READ);
    esp_send_byte(fd);
    esp_send_byte(length & 0xFF);
    esp_send_byte(length >> 8);
    int16_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }
    result = esp_get_byte();
    result |= esp_get_byte() << 8;

    uint16_t count = result;
    uint8_t *p     = buf;
    while (count--) {
        *(p++) = esp_get_byte();
    }
    return result;
}

int esp_readdir(int dd, struct esp_stat *st, char *fn, uint8_t fn_buflen) {
    if (fn == NULL || fn_buflen < 1)
        return ERR_PARAM;

    esp_cmd(ESPCMD_READDIR);
    esp_send_byte(dd);

    int8_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }
    esp_get_bytes(st, sizeof(*st));

    while (1) {
        uint8_t val = esp_get_byte();
        if (val == 0)
            break;

        if (fn_buflen > 1) {
            *(fn++) = val;
            fn_buflen--;
        }
    }
    *fn = '\0';

    return result;
}

int esp_readline(int fd, void *buf, uint16_t length) {
    esp_cmd(ESPCMD_READLINE);
    esp_send_byte(fd);
    esp_send_byte(length & 0xFF);
    esp_send_byte(length >> 8);
    int16_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }

    result     = 0;
    uint8_t *p = buf;
    while (1) {
        uint8_t val = esp_get_byte();
        *(p++)      = val;
        if (val == 0)
            break;
        result++;
    }
    return result;
}

int esp_rename(const char *path_old, const char *path_new) {
    esp_cmd(ESPCMD_RENAME);
    uint16_t len = strlen(path_old) + 1;
    esp_send_bytes(path_old, len);
    len = strlen(path_new) + 1;
    esp_send_bytes(path_new, len);
    return (int8_t)esp_get_byte();
}

int esp_seek(int fd, uint32_t offset) {
    esp_cmd(ESPCMD_SEEK);
    esp_send_byte(fd);
    esp_send_bytes(&offset, sizeof(offset));
    return (int8_t)esp_get_byte();
}

int esp_stat(const char *path, struct esp_stat *st) {
    esp_cmd(ESPCMD_STAT);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);

    int8_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }
    esp_get_bytes(st, sizeof(*st));
    return result;
}

int esp_tell(int fd) {
    esp_cmd(ESPCMD_TELL);
    esp_send_byte(fd);

    int32_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }

    esp_get_bytes(&result, sizeof(result));
    return result;
}

int esp_write(int fd, const void *buf, uint16_t length) {
    esp_cmd(ESPCMD_WRITE);
    esp_send_byte(fd);
    esp_send_byte(length & 0xFF);
    esp_send_byte(length >> 8);
    esp_send_bytes(buf, length);

    int16_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }
    result = esp_get_byte();
    result |= esp_get_byte() << 8;
    return result;
}

bool load_binary(const char *path, void *addr, uint16_t max_length) {
    int fd = esp_open(path, FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    esp_read(fd, addr, max_length);
    esp_close(fd);
    return true;
}
