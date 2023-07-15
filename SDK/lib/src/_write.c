#include "file_io.h"
#include "esp.h"

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
