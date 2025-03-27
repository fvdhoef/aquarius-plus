#include "regs.h"

static void esp_send_byte(uint8_t val) {
    while (REGS->ESP_CTRL & 2) {
    }
    REGS->ESP_DATA = val;
}

static uint8_t esp_get_byte(void) {
    while ((REGS->ESP_CTRL & 1) == 0) {
    }
    return REGS->ESP_DATA;
}

static void esp_cmd(uint8_t cmd) {
    while (REGS->ESP_CTRL & 1) {
        (void)REGS->ESP_DATA;
    }

    while (REGS->ESP_CTRL & 2) {
    }
    REGS->ESP_DATA = 0x100;
    esp_send_byte(cmd);
}

static int esp_open(const char *path, uint8_t flags) {
    esp_cmd(ESPCMD_OPEN);
    esp_send_byte(flags);

    const uint8_t *p = (const uint8_t *)path;
    while (1) {
        uint8_t val = *(p++);
        esp_send_byte(val);
        if (val == 0)
            break;
    }
    return (int8_t)esp_get_byte();
}

static int esp_read(int fd, void *buf, uint16_t length) {
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

static int esp_close(int fd) {
    esp_cmd(ESPCMD_CLOSE);
    esp_send_byte(fd);
    return (int8_t)esp_get_byte();
}

// called from start.S
void boot(void) {
    esp_cmd(ESPCMD_RESET);
    int fd = esp_open("aq32.rom", FO_RDONLY);
    if (fd >= 0) {
        uint8_t *addr = (uint8_t *)0x80000000;
        while (1) {
            int count = esp_read(fd, addr, 0x8000);
            if (count <= 0)
                break;
            addr += count;
        }
        esp_close(fd);
        ((void (*)())0x80000000)();
    }
    while (1);
}
