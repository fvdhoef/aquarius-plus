#include "regs.h"
#include "esp.h"
#include "file_io.h"

// called from start.S
void boot(void) {
    // CHRAM[0] = 0xFF;
    int i     = 0;
    TRAM[i++] = 'B' | 0x0300;
    TRAM[i++] = 'l' | 0x0300;
    TRAM[i++] = 'a' | 0x0300;
    TRAM[i++] = 'a' | 0x0300;
    TRAM[i++] = 't' | 0x0300;

    while (1);
}
