#pragma once

#include "common.h"
#include "z80.h"
#include "ay8910.h"

#define AUDIO_LEVEL (16000)

struct emulation_state {
    Z80Context    z80context;       // Z80 emulation core state
    int           line_hcycles;     // Half-cycles for this line
    int           sample_hcycles;   // Half-cycles for this sample
    int           linenr;           // Current display line
    int16_t       audio_out;        // Audio level of internal sound output (also used for cassette interface)
    bool          cpm_remap;        // Remap memory for CP/M
    uint8_t       scramble_value;   // Value to XOR (scramble) the external data bus with
    uint8_t       keyb_matrix[8];   // Keyboard matrix (8 x 6bits)
    bool          expander_enabled; // Mini-expander enabled?
    uint8_t       ay_addr;          // Mini-expander - AY-3-8910: Selected address to access via data register
    uint8_t       handctrl1;        // Mini-expander - Hand controller 1 state (connected to port 1 of AY-3-8910)
    uint8_t       handctrl2;        // Mini-expander - Hand controller 2 state (connected to port 1 of AY-3-8910)
    bool          ramexp_enabled;   // RAM expansion enabled?
    uint8_t       rom[0x3000];      // 0x0000-0x2FFF: System ROM (12KB)
    uint8_t       ram[0x1000];      // 0x3000-0x3FFF: System RAM
    uint8_t       ramexp[0x8000];   // 0x4000-0xBFFF: RAM expansion
    uint8_t       gamerom[0x4000];  // 0xC000-0xFFFF: Cartridge
    struct ay8910 ay_state;         // AY-3-8910 emulation state
};

extern struct emulation_state emustate;
