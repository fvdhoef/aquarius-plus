#include "file_io.h"
#include "esp.h"
#include <string.h>

int8_t esp_stat(const char *path, struct esp_stat *st) {
    esp_cmd(ESPCMD_STAT);
    uint16_t len = strlen(path) + 1;
    esp_send_bytes(path, len);

    int8_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }
    esp_get_bytes(st, sizeof(*st));
    return result;
}
