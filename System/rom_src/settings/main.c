#include <aqplus.h>
#include "disp.h"

static uint8_t buf[32];

int main(void) {
    int8_t fd = esp_open("esp:com", FO_RDWR);
    if (fd < 0) {
        printf("Error opening communication channel\n");
        return 1;
    }

    disp_clear();
    {
        const char *str = "Aquarius+ settings";
        disp_set_cursor(0, (40 - strlen(str)) / 2);
        disp_puts(str);
    }
    disp_set_cursor(1, 0);

    while (1) {
        uint8_t ch = getchar();
        if (ch) {
            esp_write(fd, &ch, 1);
        }

        int16_t size = esp_read(fd, buf, sizeof(buf));
        if (size >= 0) {
            for (int16_t i = 0; i < size; i++) {
                ch = buf[i];
                if (ch == 3)
                    goto cleanup;
                if (ch == '\r')
                    continue;
                if (ch == '\n')
                    disp_putchar('\r');
                disp_putchar(buf[i]);
            }
        }
    }

cleanup:
    esp_close(fd);

    // Clear screen
    putchar(11);

    // Returning a value to exit the main loop
    return 0;
}
