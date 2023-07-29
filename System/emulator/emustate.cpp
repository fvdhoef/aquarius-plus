#include "emustate.h"
#include <stdlib.h>

emulation_state emustate;

static const uint16_t defaultPalette[] = {
    0x111, 0xF11, 0x1F1, 0xFF1, 0x22E, 0xF1F, 0x3CC, 0xFFF,
    0xCCC, 0x3BB, 0xC2C, 0x419, 0xFF7, 0x2D4, 0xB22, 0x333};

void emustate_init(void) {
    memset(emustate.keyb_matrix, 0xFF, sizeof(emustate.keyb_matrix));
    emustate.handctrl1 = 0xFF;
    emustate.handctrl2 = 0xFF;
    memcpy(emustate.video_palette, defaultPalette, sizeof(defaultPalette));
    emustate.video_ctrl = 1;

    for (unsigned i = 0; i < sizeof(emustate.screenram); i++) {
        emustate.screenram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emustate.colorram); i++) {
        emustate.colorram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emustate.flashrom); i++) {
        emustate.flashrom[i] = 0xFF;
    }
    for (unsigned i = 0; i < sizeof(emustate.mainram); i++) {
        emustate.mainram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emustate.gamerom); i++) {
        emustate.gamerom[i] = 0xFF;
    }
    for (unsigned i = 0; i < sizeof(emustate.videoram); i++) {
        emustate.videoram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emustate.charram); i++) {
        emustate.charram[i] = rand();
    }
}
