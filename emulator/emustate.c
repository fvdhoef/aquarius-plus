#include "emustate.h"
#include <stdlib.h>

struct emulation_state emustate = {
    .expander_enabled = true,
    .keyb_matrix      = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .handctrl1        = 0xFF,
    .handctrl2        = 0xFF,
    .ramexp_enabled   = true,
};

void emustate_init(void) {
    for (unsigned i = 0; i < sizeof(emustate.screenram); i++) {
        emustate.screenram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emustate.colorram); i++) {
        emustate.colorram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emustate.basicram); i++) {
        emustate.basicram[i] = rand();
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
