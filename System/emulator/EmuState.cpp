#include "EmuState.h"
#include <stdlib.h>

EmuState emuState;

static const uint16_t defaultPalette[] = {
    0x111, 0xF11, 0x1F1, 0xFF1, 0x22E, 0xF1F, 0x3CC, 0xFFF,
    0xCCC, 0x3BB, 0xC2C, 0x419, 0xFF7, 0x2D4, 0xB22, 0x333};

void emustate_init(void) {
    memset(emuState.keybMatrix, 0xFF, sizeof(emuState.keybMatrix));
    emuState.handCtrl1 = 0xFF;
    emuState.handCtrl2 = 0xFF;
    memcpy(emuState.videoPalette, defaultPalette, sizeof(defaultPalette));
    emuState.videoCtrl = 1;

    for (unsigned i = 0; i < sizeof(emuState.screenRam); i++) {
        emuState.screenRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.colorRam); i++) {
        emuState.colorRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.flashRom); i++) {
        emuState.flashRom[i] = 0xFF;
    }
    for (unsigned i = 0; i < sizeof(emuState.mainRam); i++) {
        emuState.mainRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.gameRom); i++) {
        emuState.gameRom[i] = 0xFF;
    }
    for (unsigned i = 0; i < sizeof(emuState.videoRam); i++) {
        emuState.videoRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.charRam); i++) {
        emuState.charRam[i] = rand();
    }
}
