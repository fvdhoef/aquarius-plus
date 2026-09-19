#include "file_io.h"
#include "esp.h"

int8_t esp_close(int8_t fd) {
    esp_cmd(ESPCMD_CLOSE);
    esp_send_byte(fd);
    return (int8_t)esp_get_byte();
}
