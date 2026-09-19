#include "file_io.h"
#include "esp.h"

int8_t esp_closedir(int8_t dd) {
    esp_cmd(ESPCMD_CLOSEDIR);
    esp_send_byte(dd);
    return (int8_t)esp_get_byte();
}
