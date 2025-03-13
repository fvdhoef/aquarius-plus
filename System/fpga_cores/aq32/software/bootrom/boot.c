#include <stdint.h>

#define TRAM     ((uint16_t *)0xFF000000)
#define CHRAM    ((uint8_t *)0xFF100000)
#define IO_VIDEO ((uint8_t *)0xFF300000)
#define PALETTE  ((uint16_t *)0xFF400000)

struct regs {
    volatile uint32_t ESP_CTRL;
    volatile uint32_t ESP_DATA;
    volatile uint32_t VCTRL;
    volatile uint32_t VSCRX;
    volatile uint32_t VSCRY;
    volatile uint32_t VLINE;
    volatile uint32_t VIRQLINE;
};

#define REGS ((struct regs *)0xFF500000)

// called from start.S
void boot(void) {
    // CHRAM[0] = 0xFF;
    TRAM[0] = 'A' | 0x0300;
    TRAM[1] = 'B' | 0x0300;

    REGS->VCTRL = 1;

    while (1);
}
