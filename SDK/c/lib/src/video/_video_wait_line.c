#include <video.h>
#include <regs.h>

void video_wait_line(uint8_t linenr) {
    IO_VIRQLINE = linenr;
    IO_IRQSTAT  = 2;
    while ((IO_IRQSTAT & 2) == 0) {
    }
}