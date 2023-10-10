#include "file_io.h"
#include "esp.h"
#include <string.h>

int8_t rename(const char *path_old, const char *path_new) {
    esp_cmd(ESPCMD_RENAME);
    uint16_t len = strlen(path_old) + 1;
    esp_send_bytes(path_old, len);
    len = strlen(path_new) + 1;
    esp_send_bytes(path_new, len);
    return (int8_t)esp_get_byte();
}
