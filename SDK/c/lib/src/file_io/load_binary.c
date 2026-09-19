#include "file_io.h"
#include "esp.h"

bool load_binary(const char *path, void *addr, uint16_t max_length) {
    int8_t fd = esp_open(path, FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    esp_read(fd, addr, max_length);
    esp_close(fd);
    return true;
}
