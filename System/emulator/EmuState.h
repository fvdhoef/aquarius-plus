#pragma once

#include "Common.h"
#include "z80.h"
#include "Video.h"
#include "AY8910.h"
#include <deque>

// 3579545 Hz -> 59659 cycles / frame
// 7159090 Hz -> 119318 cycles / frame

// 455x262=119210 -> 60.05 Hz
// 51.2us + 1.5us + 4.7us + 6.2us = 63.6 us
// 366 active pixels

#define HCYCLES_PER_LINE (455)
#define HCYCLES_PER_SAMPLE (162)

#define BANK_READONLY (1 << 7)
#define BANK_MAP_RAM (1 << 6)

enum EmulateResultFlags {
    ERF_RENDER_SCREEN    = (1 << 0),
    ERF_NEW_AUDIO_SAMPLE = (1 << 1),
};

struct EmuState {
    EmuState();
    void reset();
    bool loadSystemROM(const std::string &path);
    bool loadCartridgeROM(const std::string &path);
    void keyboardTypeIn();

    static uint8_t memRead(size_t param, uint16_t addr);
    static void    memWrite(size_t param, uint16_t addr, uint8_t data);
    static uint8_t ioRead(size_t param, uint16_t addr);
    static void    ioWrite(size_t param, uint16_t addr, uint8_t data);

    unsigned emulate();
    unsigned audioLeft  = 0;
    unsigned audioRight = 0;

    // Breakpoints
    bool enableBreakpoints = false;

    struct Breakpoint {
        uint16_t value   = 0;
        bool     enabled = false;
        int      type    = 0;
        bool     onR     = true;
        bool     onW     = true;
        bool     onX     = true;
    };

    std::vector<Breakpoint> breakpoints;
    int                     tmpBreakpoint = -1;
    int                     lastBpAddress = -1;
    int                     lastBp        = -1;
    int                     haltAfterRet  = -1;

    enum EmuMode {
        Em_Halted,
        Em_Step,
        Em_Running,
    };
    EmuMode emuMode = Em_Running;

    // Emulator state
    Z80Context z80ctx;                   // Z80 emulation core state
    Video      video;                    // Video
    int        lineHalfCycles    = 0;    // Half-cycles for this line
    int        sampleHalfCycles  = 0;    // Half-cycles for this sample
    uint8_t    keybMatrix[8]     = {0};  // Keyboard matrix (8 x 6bits)
    uint8_t    handCtrl1         = 0xFF; // Mini-expander - Hand controller 1 state (connected to port 1 of AY-3-8910)
    uint8_t    handCtrl2         = 0xFF; // Mini-expander - Hand controller 2 state (connected to port 2 of AY-3-8910)
    bool       cartridgeInserted = false;
    size_t     systemRomSize     = 0;

    // Keyboard buffer
    uint8_t kbBuf[16];
    uint8_t kbBufWrIdx = 0;
    uint8_t kbBufRdIdx = 0;
    uint8_t kbBufCnt   = 0;

    void    kbBufReset();
    void    kbBufWrite(uint8_t val);
    uint8_t kbBufRead();

    // CPU tracing
    struct Z80TraceEntry {
        Z80Regs  r1;
        Z80Regs  r2;
        uint16_t pc;
        char     bytes[32];
        char     instrStr[32];
    };
    std::deque<Z80TraceEntry> cpuTrace;
    bool                      traceEnable    = false;
    int                       traceDepth     = 128;
    bool                      prevHalted     = false;
    int                       emulationSpeed = 1;

    // Virtual typing from command-line argument
    std::string typeInStr;

    // Mouse state
    uint16_t mouseX           = 0;
    uint8_t  mouseY           = 0;
    uint8_t  mouseButtons     = 0;
    float    mouseHideTimeout = 0;

    // IO space
    uint8_t  videoCtrl        = 0;      // $E0   : Video control register
    uint16_t videoScrX        = 0;      // $E1/E2: Tile map horizontal scroll register
    uint8_t  videoScrY        = 0;      // $E3   : Tile map horizontal scroll register
    uint8_t  videoSprSel      = 0;      // $E4   : Sprite select
    uint16_t videoSprX[64]    = {0};    // $E5/E6: Sprite X-position
    uint8_t  videoSprY[64]    = {0};    // $E7   : Sprite Y-position
    uint16_t videoSprIdx[64]  = {0};    // $E8/E9: Sprite tile index
    uint8_t  videoSprAttr[64] = {0};    // $E9   : Sprite attributes
    uint8_t  videoPalSel      = 0;      // $EA   : Palette entry select
    uint16_t videoPalette[64] = {0};    // $EB   : Video palette
    uint16_t videoLine        = 0;      // $EC   : Current line number
    uint8_t  videoIrqLine     = 0;      // $ED   : Line number at which to generate IRQ
    uint8_t  irqMask          = 0;      // $EE   : Interrupt mask register
    uint8_t  irqStatus        = 0;      // $EF   : Interrupt status register
    uint8_t  bankRegs[4]      = {0};    // $F0-F3: Banking registers
    AY8910   ay1;                       // $F6/F7: AY-3-8910 emulation state
    uint8_t  ay1Addr = 0;               // $F7   : AY-3-8910: Selected address to access via data register
    AY8910   ay2;                       // $F8/F9: AY-3-8910 emulation state
    uint8_t  ay2Addr           = 0;     // $F9   : AY-3-8910: Selected address to access via data register
    bool     sysCtrlDisableExt = false; // $FB<0>: Disable access to extended registers
    bool     sysCtrlAyDisable  = false; // $FB<1>: Disable AY PSGs
    bool     sysCtrlTurbo      = false; // $FB<2>: Turbo mode
    bool     soundOutput       = false; // $FC<1>: Cassette/Sound output
    bool     cpmRemap          = false; // $FD<1>: Remap memory for CP/M

    // Memory space
    uint8_t screenRam[2048];       // $3000-33FF: Screen RAM for text mode
    uint8_t colorRam[2048];        // $3400-37FF: Color RAM for text mode
    uint8_t systemRom[256 * 1024]; // Flash memory
    uint8_t mainRam[512 * 1024];   // Main RAM
    uint8_t cartRom[16 * 1024];    // Cartridge ROM
    uint8_t videoRam[16 * 1024];   // Video RAM
    uint8_t charRam[2048];         // Character RAM
};

extern EmuState emuState;
