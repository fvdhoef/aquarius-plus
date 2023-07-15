#include "file_io.h"
#include "esp.h"
#include <stddef.h>

int8_t getcwd(char *cwd, uint8_t cwd_buflen) {
    if (cwd == NULL || cwd_buflen < 1)
        return ERR_PARAM;

    esp_cmd(ESPCMD_GETCWD);

    int8_t result = (int8_t)esp_get_byte();
    if (result < 0) {
        return result;
    }

    while (1) {
        uint8_t val = esp_get_byte();
        if (val == 0)
            break;

        if (cwd_buflen > 1) {
            *(cwd++) = val;
            cwd_buflen--;
        }
    }
    *cwd = '\0';

    return result;
}
