#include "common.h"
#include "esp.h"

void hexdump(const void *buf, int length) {
    int            idx = 0;
    const uint8_t *p   = (const uint8_t *)buf;

    while (length > 0) {
        int len = length;
        if (len > 8) {
            len = 8;
        }

        printf("%08x  ", idx);

        for (int i = 0; i < 8; i++) {
            if (i < len) {
                printf("%02x ", p[i]);
            } else {
                printf("   ");
            }
        }
        printf(" |");

        for (int i = 0; i < len; i++) {
            printf("%c", (p[i] >= 32 && p[i] <= 126) ? p[i] : '.');
        }
        printf("|\n");

        idx += len;
        length -= len;
        p += len;
    }
}
