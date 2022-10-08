#include "fpga.h"

static uint8_t saved_screen_ram[2048];

void screen_save(void) {
    fpga_set_bank(0, 0xC0);

    for (int i = 0; i < 2048; i++)
        saved_screen_ram[i] = fpga_mem_read(0x3000 + i);
}

void screen_restore(void) {
    fpga_set_bank(0, 0xC0);

    for (int i = 0; i < 2048; i++)
        fpga_mem_write(0x3000 + i, saved_screen_ram[i]);
}

void screen_show_msg(const char *str) {
    fpga_set_bank(0, 0xC0);

    unsigned addr = 0x3000 + 40;

    const char *p = str;

    while (*p != '\0') {
        fpga_mem_write(addr, *p);
        fpga_mem_write(addr + 1024, 0x70);
        p++;
        addr++;
    }

    while ((addr - 0x3000) % 40 != 0) {
        fpga_mem_write(addr, ' ');
        fpga_mem_write(addr + 1024, 0x70);
        addr++;
    }
}
