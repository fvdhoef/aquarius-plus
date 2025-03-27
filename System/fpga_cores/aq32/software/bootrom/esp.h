#pragma once

#include "regs.h"

static inline void esp_send_byte(uint8_t val) {
    while (REGS->ESP_CTRL & 2) {
    }
    REGS->ESP_DATA = val;
}

static inline uint8_t esp_get_byte(void) {
    while ((REGS->ESP_CTRL & 1) == 0) {
    }
    return REGS->ESP_DATA;
}

static inline void esp_cmd(uint8_t cmd) {
    while (REGS->ESP_CTRL & 1) {
        (void)REGS->ESP_DATA;
    }

    while (REGS->ESP_CTRL & 2) {
    }
    REGS->ESP_DATA = 0x100;
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
