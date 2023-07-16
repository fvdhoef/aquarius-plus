#include "file_io.h"
#include "esp.h"

int8_t closeall(void) {
    esp_cmd(ESPCMD_CLOSEALL);
    return (int8_t)esp_get_byte();
}
