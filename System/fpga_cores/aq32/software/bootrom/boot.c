#include <stdint.h>

#define TRAM     ((uint16_t *)0xFF000000)
#define CHRAM    ((uint8_t *)0xFF100000)
#define IO_VIDEO ((uint8_t *)0xFF300000)
#define PALETTE  ((uint16_t *)0xFF400000)

// called from start.S
void boot(void) {
    // CHRAM[0] = 0xFF;
    TRAM[0] = 'A' | 0x0300;

    IO_VIDEO[0] = 1;

    while (1);
}
