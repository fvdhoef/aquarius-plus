#include "file_io.h"
#include "esp.h"
#include <string.h>

int8_t open(const char *path, uint8_t flags) {
    esp_cmd(ESPCMD_OPEN);
    esp_send_byte(flags);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);
    return (int8_t)esp_get_byte();
}
