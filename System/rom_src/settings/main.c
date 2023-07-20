#include <aqplus.h>

static uint8_t buf[32];

int main(void) {
    int8_t fd = open("esp:com", FO_RDWR);
    if (fd < 0) {
        printf("Error opening communication channel\n");
        return 0;
    }

    while (1) {
        uint8_t ch = getchar();
        if (ch == 3)
            break;

        if (ch) {
            write(fd, &ch, 1);
        }

        int16_t size = read(fd, buf, sizeof(buf));
        if (size >= 0) {
            for (int16_t i = 0; i < size; i++) {
                putchar(buf[i]);
            }
        }
    }

    close(fd);

    // Returning a value to exit the main loop
    return 0;
}
