#include "regs.h"
#include "esp.h"
#include "file_io.h"

// called from start.S
void boot(void) {
    TRAM[0]     = 'A' | 0x0300;
    REGS->VCTRL = 1;

    // int err_addr = -1;

    // volatile uint32_t *p = (volatile uint32_t *)0x1000;
    // for (unsigned i = 0; i < 64; i++)
    //     p[i] = i;

    // for (unsigned i = 0; i < 64; i++) {
    //     if (p[i] != i) {
    //         err_addr = i;
    //         break;
    //     }
    // }

    // if (err_addr >= 0) {
    //     TRAM[0] = 'E' | 0x0300;
    //     TRAM[1] = ((((uintptr_t)err_addr >> 24) & 0xF) + '0') | 0x0300;
    //     TRAM[2] = ((((uintptr_t)err_addr >> 16) & 0xF) + '0') | 0x0300;
    //     TRAM[3] = ((((uintptr_t)err_addr >> 8) & 0xF) + '0') | 0x0300;
    //     TRAM[4] = ((((uintptr_t)err_addr >> 0) & 0xF) + '0') | 0x0300;

    // } else {
    //     TRAM[0] = 'S' | 0x0300;
    // }

    // // CHRAM[0] = 0xFF;
    // TRAM[1] = 'B' | 0x0300;

    // REGS->VCTRL = 1;

    esp_cmd(ESPCMD_RESET);
    TRAM[0] = 'B' | 0x0300;
    int fd  = esp_open("aq32.rom", FO_RDONLY);
    TRAM[0] = 'C' | 0x0300;

    if (fd >= 0) {
        esp_read(fd, (void *)0, 0x4000);
        esp_close(fd);
        __asm __volatile("jr zero");
    }

    TRAM[0] = 'D' | 0x0300;
    while (1);
}
