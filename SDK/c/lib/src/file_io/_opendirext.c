#include "file_io.h"
#include "esp.h"
#include <string.h>

int8_t opendirext(const char *path, uint8_t flags, uint16_t skip_cnt) {
    esp_cmd(ESPCMD_OPENDIREXT);
    esp_send_byte(flags);
    esp_send_byte(skip_cnt & 0xFF);
    esp_send_byte(skip_cnt >> 8);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}
