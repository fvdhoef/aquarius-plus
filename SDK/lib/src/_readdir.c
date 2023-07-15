#include "file_io.h"
#include "esp.h"
#include <stddef.h>

int8_t readdir(int8_t dd, struct stat *st, char *fn, uint8_t fn_buflen) {
    if (fn == NULL || fn_buflen < 1)
        return ERR_PARAM;

    esp_cmd(ESPCMD_READDIR);
    esp_send_byte(dd);

    int8_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }
    esp_get_bytes(st, sizeof(*st));

    while (1) {
        uint8_t val = esp_get_byte();
        if (val == 0)
            break;

        if (fn_buflen > 1) {
            *(fn++) = val;
            fn_buflen--;
        }
    }
    *fn = '\0';

    return result;
}
