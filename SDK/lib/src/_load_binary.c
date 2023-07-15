#include "file_io.h"
#include "esp.h"

bool load_binary(const char *path, void *addr, uint16_t max_length) {
    int8_t fd = open(path, FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    read(fd, addr, max_length);
    close(fd);
    return true;
}
