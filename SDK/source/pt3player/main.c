#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <file_io.h>
#include <regs.h>
#include <pt3play.h>

extern const unsigned char ingarden_pt3[];

int main(void) {
    pt3play_init(ingarden_pt3);

    while (1) {
        // Wait for end of frame (line 216)
        IO_VIRQLINE = 216;
        IO_IRQSTAT  = 2;
        while ((IO_IRQSTAT & 2) == 0) {
        }

        if (pt3play_play())
            break;
    }
    pt3play_mute();

    return 0;
}
