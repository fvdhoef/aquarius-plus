#pragma once

#include "common.h"
#include "z80.h"
#include "ay8910.h"

#define AUDIO_LEVEL (16000)

struct emulation_state {
    Z80Context z80context;       // Z80 emulation core state
    int        line_hcycles;     // Half-cycles for this line
    int        sample_hcycles;   // Half-cycles for this sample
    uint8_t    keyb_matrix[8];   // Keyboard matrix (8 x 6bits)
    bool       expander_enabled; // Mini-expander enabled?
    uint8_t    handctrl1;        // Mini-expander - Hand controller 1 state (connected to port 1 of AY-3-8910)
    uint8_t    handctrl2;        // Mini-expander - Hand controller 2 state (connected to port 1 of AY-3-8910)
    bool       ramexp_enabled;   // RAM expansion enabled?

    // IO space
    uint8_t       video_ctrl;               // $E0   : Video control register
    uint16_t      video_scrx;               // $E1/E2: Tile map horizontal scroll register
    uint8_t       video_scry;               // $E3   : Tile map horizontal scroll register
    uint16_t      video_sprx[64];           // $E4/E5: Sprite X-position
    uint8_t       video_spry[64];           // $E6   : Sprite Y-position
    uint16_t      video_spridx[64];         // $E7/E8: Sprite tile index
    uint8_t       video_sprattr[64];        // $E8   : Sprite attributes
    uint8_t       video_palette_text[16];   // $E9   : Text palette
    uint8_t       video_palette_tile[16];   // $EA   : Tile/bitmap palette
    uint8_t       video_palette_sprite[16]; // $EB   : Sprite palette
    uint16_t      video_line;               // $EC   : Current line number
    uint8_t       video_irqline;            // $ED   : Line number at which to generate IRQ
    uint8_t       irqmask;                  // $EE   : Interrupt mask register
    uint8_t       irqstatus;                // $EF   : Interrupt status register
    uint8_t       bankregs[4];              // $F0-F3: Banking registers
    struct ay8910 ay_state;                 // $F6/F7: AY-3-8910 emulation state
    uint8_t       ay_addr;                  // $F7   : AY-3-8910: Selected address to access via data register
    struct ay8910 ay2_state;                // $F8/F9: AY-3-8910 emulation state
    uint8_t       ay2_addr;                 // $F9   : AY-3-8910: Selected address to access via data register
    bool          sysctrl_disable_ext;      // $FB<0>: Disable access to extended registers
    bool          sysctrl_ay_both;          // $FB<1>: Send first PSG to both channels
    bool          sysctrl_ay_disable;       // $FB<2>: Disable AY PSGs
    bool          sound_output;             // $FC<1>: Cassette/Sound output
    bool          cpm_remap;                // $FD<1>: Remap memory for CP/M
    uint8_t       extbus_scramble;          // $FF   : External bus scramble (XOR) value

    // Memory space
    uint8_t textram[1024];        // $3000-33FF: Text RAM
    uint8_t colorram[1024];       // $3400-37FF: Color RAM for text mode
    uint8_t basicram[2048];       // $3800-3FFF: BASIC RAM on a stock Aquarius
    uint8_t flashrom[256 * 1024]; // Flash memory
    uint8_t mainram[512 * 1024];  // Main RAM
    uint8_t gamerom[16 * 1024];   // Cartridge ROM
    uint8_t videoram[16 * 1024];  // Video RAM
    uint8_t charram[2048];        // Character RAM
};

extern struct emulation_state emustate;
