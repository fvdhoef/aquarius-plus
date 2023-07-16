#include "file_io.h"
#include "regs.h"
#include <string.h>

static inline void esp_send_byte(uint8_t val) {
    while (IO_ESPCTRL & 2) {
    }
    IO_ESPDATA = val;
}

static inline uint8_t esp_get_byte(void) {
    while ((IO_ESPCTRL & 1) == 0) {
    }
    return IO_ESPDATA;
}

static inline void esp_cmd(uint8_t cmd) {
    IO_ESPCTRL = 0x83;
    esp_send_byte(cmd);
}

static void esp_send_bytes(const void *buf, uint16_t length) {
    const uint8_t *p = buf;
    while (length--) {
        esp_send_byte(*(p++));
    }
}

int8_t open(const char *path, uint8_t flags) {
    esp_cmd(ESPCMD_OPEN);
    esp_send_byte(flags);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}

int16_t read(int8_t fd, void *buf, uint16_t length) {
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

int16_t write(int8_t fd, const void *buf, uint16_t length) {
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

int8_t close(int8_t fd) {
    esp_cmd(ESPCMD_CLOSE);
    esp_send_byte(fd);
    return (int8_t)esp_get_byte();
}

bool load_binary(const char *path, void *addr, uint16_t max_length) {
    int8_t fd = open(path, FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    read(fd, addr, max_length);
    close(fd);
    return true;
}
