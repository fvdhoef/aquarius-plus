#include "file_io.h"
#include "esp.h"

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
