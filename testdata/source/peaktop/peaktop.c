#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "regs.h"
#include "file_io.h"

unsigned long ticks = 0;

static uint8_t get_joystick(void) {
    IO_PSG1ADDR = 14;
    return IO_PSG1DATA;
}

static void clear_screen() {
    uint8_t *tram = (uint8_t *)0x3000;
    uint8_t *cram = (uint8_t *)0x3400;

    // Clear screen
    for (int i = 0; i < 1000; i++) {
        tram[i] = ' ';
        cram[i] = 0x06;
    }
    printf("ticks = %u\n", the_ticks());
}

unsigned long the_ticks() {
    return ticks;
}

static inline void wait_vsync(void) {
    ticks++;
    // while (IO_VLINE < 216) {
    // }

    // Wait for line 216
    IO_VIRQLINE = 216;
    IO_IRQSTAT  = 2;
    while ((IO_IRQSTAT & 2) == 0) {
    }
}

bool init(void) {
    // Load in CHR and COL data
    int8_t fd = open("pt_ss.scr", FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    read(fd, (void *)0x3000, 0x3800);
    close(fd);

    return true;
}

int main(void) {
    if (!init()) {
        return 1;
    }

    // Main Loop
    while (1) {
        wait_vsync();
        uint8_t joyval = ~get_joystick();
        if (joyval & (1 << 0)) {
            clear_screen();
            return 0;
        }
        if (joyval & (1 << 1)) {
            clear_screen();
            return 0;
        }
        if (joyval & (1 << 2)) {
            clear_screen();
            return 0;
        }
        if (joyval & (1 << 3)) {
            clear_screen();
            return 0;
        }
    }
}
