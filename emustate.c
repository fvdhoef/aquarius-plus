#include "emustate.h"

struct emulation_state emustate = {
    .audio_out        = AUDIO_LEVEL,
    .expander_enabled = true,
    .keyb_matrix      = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .handctrl1        = 0xFF,
    .handctrl2        = 0xFF,
    .ramexp_enabled   = true,
};