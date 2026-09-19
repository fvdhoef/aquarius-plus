#include "common.h"
#include "esp.h"

static const uint16_t palette[16] = {
    0x000, 0x00A, 0x0A0, 0x0AA, 0xA00, 0xA0A, 0xA50, 0xAAA,
    0x555, 0x55F, 0x5F5, 0x5FF, 0xF55, 0xF5F, 0xFF5, 0xFFF};

void reinit_video(void) {
#ifndef PCDEV
    GFX->CTRL = GFX_CTRL_TEXT_MODE80 | GFX_CTRL_TEXT_EN;

    for (int i = 0; i < 64; i++)
        PALETTE[i] = palette[i & 15];

    int fd = esp_open("esp:latin1b.chr", 0);
    if (fd >= 0) {
        esp_read(fd, (void *)CHRAM, 2048);
        esp_close(fd);
    }
#endif
}

void hexdump(const void *buf, int length) {
    int            idx = 0;
    const uint8_t *p   = (const uint8_t *)buf;

    while (length > 0) {
        int len = length;
        if (len > 16) {
            len = 16;
        }

        printf("%08x  ", idx);

        for (int i = 0; i < 16; i++) {
            if (i < len) {
                printf("%02x ", p[i]);
            } else {
                printf("   ");
            }
            if (i == 7) {
                printf(" ");
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
