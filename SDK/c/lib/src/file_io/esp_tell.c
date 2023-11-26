#include "file_io.h"
#include "esp.h"

int32_t esp_tell(int8_t fd) {
    esp_cmd(ESPCMD_TELL);
    esp_send_byte(fd);

    int32_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }

    esp_get_bytes(&result, sizeof(result));
    return result;
}
