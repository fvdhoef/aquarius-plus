#include <stdint.h>
#include "regs.h"

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

static inline void esp_send_bytes(const void *buf, uint16_t length) {
    const uint8_t *p = buf;
    while (length--) {
        esp_send_byte(*(p++));
    }
}

static inline void esp_get_bytes(void *buf, uint16_t length) {
    uint8_t *p = buf;
    while (length--) {
        *(p++) = esp_get_byte();
    }
}
