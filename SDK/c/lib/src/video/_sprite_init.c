#include <video.h>
#include <regs.h>

void sprite_init(uint8_t sprite_idx, uint16_t attr_idx) {
    IO_VSPRSEL  = sprite_idx;
    IO_VSPRIDX  = attr_idx & 0xFF;
    IO_VSPRATTR = attr_idx >> 8;
}
