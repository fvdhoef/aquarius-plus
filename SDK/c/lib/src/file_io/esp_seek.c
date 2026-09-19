#include "file_io.h"
#include "esp.h"

int8_t esp_seek(int8_t fd, uint32_t offset) {
    esp_cmd(ESPCMD_SEEK);
    esp_send_byte(fd);
    esp_send_bytes(&offset, sizeof(offset));
    return (int8_t)esp_get_byte();
}
