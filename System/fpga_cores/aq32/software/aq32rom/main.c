#include "lib.h"
#include "console.h"

// called from start.S
void main(void) {
    REGS->VCTRL = VCTRL_80COLUMNS | VCTRL_REMAP_BORDER_CH | VCTRL_TEXT_EN;
    console_init();

    int a = 0;
    while (1) {
        printf("Hello world! %d\n", a++);
    }
    // for (int i = 0; i < 80 * 25; i++) {
    //     TRAM[i] = ' ' | 0x0600;
    // } // 6 8 9

    // volatile uint8_t *p = (volatile uint8_t *)TRAM;

    // int i      = 0;
    // TRAM[2047] = ' ' | 0x0900;

    // p[i] = 'B';
    // p += 2;
    // p[i] = 'l';
    // p += 2;
    // p[i] = 'a';
    // p += 2;
    // p[i] = 'a';
    // p += 2;
    // p[i] = 't';
    // p += 2;

    while (1);
}
