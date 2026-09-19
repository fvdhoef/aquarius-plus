#include "esp.h"
#include "common.h"
#include <errno.h>

#define XFER_MAX 0xF000

int esp_chdir(const char *path) {
    esp_cmd(ESPCMD_CHDIR);
    esp_send_str(path);
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
    esp_send_str(path);
    return (int8_t)esp_get_byte();
}

int esp_getcwd(char *cwd, size_t cwd_buflen) {
    if (cwd == NULL || cwd_buflen < 1)
        return ERR_PARAM;

    esp_cmd(ESPCMD_GETCWD);

    int result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

    char *p = cwd;
    while (1) {
        uint8_t val = esp_get_byte();
        if (val == 0)
            break;

        if (cwd_buflen > 1) {
            *(p++) = val;
            cwd_buflen--;
        }
    }
    // if (p == cwd) {
    //     *(p++) = '/';
    // }
    *p = '\0';

    return result;
}

int esp_mkdir(const char *path) {
    esp_cmd(ESPCMD_MKDIR);
    esp_send_str(path);
    return (int8_t)esp_get_byte();
}

int esp_open(const char *path, uint8_t flags) {
    esp_cmd(ESPCMD_OPEN);
    esp_send_byte(flags);
    esp_send_str(path);
    return (int8_t)esp_get_byte();
}

int esp_opendir(const char *path) {
    esp_cmd(ESPCMD_OPENDIR);
    esp_send_str(path);
    return (int8_t)esp_get_byte();
}

int esp_opendirext(const char *path, uint8_t flags, uint16_t skip_cnt) {
    esp_cmd(ESPCMD_OPENDIREXT);
    esp_send_byte(flags);
    esp_send_byte(skip_cnt & 0xFF);
    esp_send_byte(skip_cnt >> 8);
    esp_send_str(path);
    return (int8_t)esp_get_byte();
}

int esp_read(int fd, void *buf, size_t count) {
    uint8_t *p         = buf;
    unsigned remaining = count;

    while (remaining > 0) {
        uint16_t length;
        if (remaining > XFER_MAX) {
            length = XFER_MAX;
        } else {
            length = remaining;
        }

        esp_cmd(ESPCMD_READ);
        esp_send_byte(fd);
        esp_send_byte(length & 0xFF);
        esp_send_byte(length >> 8);
        int result = (int8_t)esp_get_byte();
        if (result < 0)
            return result;

        uint16_t rx_len = esp_get_byte();
        rx_len |= esp_get_byte() << 8;
        while (rx_len--)
            *(p++) = esp_get_byte();

        if (length != rx_len)
            break;
        remaining -= length;
    }
    return p - (uint8_t *)buf;
}

int esp_readdir(int dd, struct esp_stat *st, char *fn, size_t fn_buflen) {
    if (fn == NULL || fn_buflen < 1)
        return ERR_PARAM;

    esp_cmd(ESPCMD_READDIR);
    esp_send_byte(dd);

    int result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

    esp_get_bytes(&st->date, sizeof(st->date));
    esp_get_bytes(&st->time, sizeof(st->time));
    esp_get_bytes(&st->attr, sizeof(st->attr));
    esp_get_bytes(&st->size, sizeof(st->size));

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
    int result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

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
    esp_send_str(path_old);
    esp_send_str(path_new);
    return (int8_t)esp_get_byte();
}

int esp_seek(int fd, uint32_t offset) {
    esp_cmd(ESPCMD_SEEK);
    esp_send_byte(fd);
    esp_send_bytes(&offset, sizeof(offset));
    return (int8_t)esp_get_byte();
}

int esp_lseek(int fd, int offset, int whence) {
    esp_cmd(ESPCMD_LSEEK);
    esp_send_byte(fd);
    esp_send_byte(offset & 0xFF);
    esp_send_byte((offset >> 8) & 0xFF);
    esp_send_byte((offset >> 16) & 0xFF);
    esp_send_byte((offset >> 24) & 0xFF);
    esp_send_byte(whence);
    int32_t result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

    esp_get_bytes(&result, sizeof(result));
    return result;
}

int esp_stat(const char *path, struct esp_stat *st) {
    esp_cmd(ESPCMD_STAT);
    esp_send_str(path);

    int result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

    esp_get_bytes(&st->date, sizeof(st->date));
    esp_get_bytes(&st->time, sizeof(st->time));
    esp_get_bytes(&st->attr, sizeof(st->attr));
    esp_get_bytes(&st->size, sizeof(st->size));
    return result;
}

int esp_tell(int fd) {
    esp_cmd(ESPCMD_TELL);
    esp_send_byte(fd);

    int32_t result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

    esp_get_bytes(&result, sizeof(result));
    return result;
}

int esp_write(int fd, const void *buf, size_t count) {
    const uint8_t *p = buf;

    unsigned remaining = count;

    while (remaining > 0) {
        uint16_t length;
        if (remaining > XFER_MAX) {
            length = XFER_MAX;
        } else {
            length = remaining;
        }

        esp_cmd(ESPCMD_WRITE);
        esp_send_byte(fd);
        esp_send_byte(length & 0xFF);
        esp_send_byte(length >> 8);
        uint16_t tx_len = length;
        while (tx_len--)
            esp_send_byte(*(p++));

        int result = (int8_t)esp_get_byte();
        if (result < 0)
            return result;

        uint16_t written = esp_get_byte();
        if (length != written)
            break;
        remaining -= length;
    }

    return p - (uint8_t *)buf;
}

int esp_set_errno(int esp_err) {
    switch (esp_err) {
        case ERR_NOT_FOUND: errno = ENOENT; break;     // File / directory not found
        case ERR_TOO_MANY_OPEN: errno = ENFILE; break; // Too many open files / directories
        case ERR_PARAM: errno = EINVAL; break;         // Invalid parameter
        case ERR_EXISTS: errno = EEXIST; break;        // File already exists
        case ERR_OTHER: errno = EIO; break;            // Other error
        case ERR_NO_DISK: errno = ENODEV; break;       // No disk
        case ERR_NOT_EMPTY: errno = ENOTEMPTY; break;  // Not empty
        case ERR_WRITE_PROTECT: errno = EROFS; break;  // Write protected SD-card
        case ERR_EOF:                                  // End of file / directory
        default: errno = EIO; break;
    }
    return -1;
}

int esp_get_midi_data(void *buf, size_t buf_size) {
    uint8_t *p      = buf;
    uint16_t length = buf_size;
    if (buf_size > 0xFFFF)
        length = 0xFFFF;

    esp_cmd(ESPCMD_GETMIDIDATA);
    esp_send_byte(length & 0xFF);
    esp_send_byte(length >> 8);
    int result = (int8_t)esp_get_byte();
    if (result < 0)
        return result;

    uint16_t rx_len = esp_get_byte();
    rx_len |= esp_get_byte() << 8;
    while (rx_len--)
        *(p++) = esp_get_byte();

    return p - (uint8_t *)buf;
}
