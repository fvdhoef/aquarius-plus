#pragma once

#include "Common.h"
#include "Video.h"
#include "riscv.h"
#include "Video.h"
#include "DebugInterface.h"

#define RAM_BASE        0
#define RAM_SIZE        (512 * 1024)
#define ROM_BASE        0xFF000000
#define ROM_SIZE        (8 * 1024 * 1024)
#define TEXTRAM_BASE    0xFFFF0000
#define TEXTRAM_SIZE    (8192)
#define TLB_BASE        0xFFFFA000
#define TLB_SIZE        0x1000
#define VIDEO_BASE      0xFFFF9400
#define VIDEO_SIZE      0x100
#define UART1_BASE      0xFFFF9000
#define UART1_SIZE      0x100
#define CURSOR_RAM_BASE 0xFFFFB000
#define CURSOR_RAM_SIZE 0x800

struct EmuState;
extern EmuState emuState;

struct EmuState {
    EmuState();
    void reset();

    // Memory
    alignas(4) uint8_t ram[RAM_SIZE];
    alignas(4) uint8_t rom[ROM_SIZE];
    uint32_t tlb[TLB_SIZE / 4];

    Video video;

    // CPU
    riscv cpu;
    int   ms_ticks = 0;
    int   vs_ticks = 0;

    // Debugger
    DebugInterface dbgIf;

    bool enableDebugger    = false;
    bool enableBreakpoints = false;

    struct Breakpoint {
        uint32_t    addr = 0;
        std::string name;
        bool        enabled = false;
    };

    std::vector<Breakpoint> breakpoints;
    int64_t                 tmpBreakpoint = -1;
    int64_t                 lastBpAddress = -1;
    int                     haltAfterRet  = -1;

    enum class WatchType {
        Hex8 = 0,
        DecU8,
        DecS8,
        Hex16,
        DecU16,
        DecS16,
        Hex32,
        DecU32,
        DecS32,
    };
    struct Watch {
        uint32_t    addr = 0;
        std::string name;
        WatchType   type;
    };
    std::vector<Watch> watches;

    enum EmuMode {
        Em_Halted,
        Em_Step,
        Em_Running,
    };
    EmuMode emuMode = Em_Running;

    bool    loadRom(const std::string &path);
    bool    memWrite(uint32_t paddr, uint32_t val, uint32_t mask);
    int64_t memRead(uint32_t paddr);
    void    emulateStep();
    void    emulate();
    void    keybScanCode(unsigned scancode, bool key_down);
};
