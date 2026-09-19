#include "file_io.h"
#include "esp.h"

int16_t esp_readline(int8_t fd, void *buf, uint16_t length) {
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
