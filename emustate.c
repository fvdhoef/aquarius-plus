#include "emustate.h"

struct emulation_state emustate = {
    .expander_enabled = true,
    .keyb_matrix      = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .handctrl1        = 0xFF,
    .handctrl2        = 0xFF,
    .ramexp_enabled   = true,
    .bankregs         = {
        (1 << 7) | (1 << 6) | 16,
        (0 << 7) | (0 << 6) | 32,
        (0 << 7) | (0 << 6) | 33,
        (1 << 7) | (0 << 6) | 3,
    },
};
