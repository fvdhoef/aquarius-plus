#include "EmuState.h"
#include <time.h>

EmuState emuState;

EmuState::EmuState() {
    for (unsigned i = 0; i < ROM_SIZE; i++) {
        rom[i] = 0xFF;
    }

    reset();
}

void EmuState::reset() {
    // Fill memories with random bytes
    srand(time(0));
    for (unsigned i = 0; i < RAM_SIZE; i++) {
        ram[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(video.textram) / 2; i++) {
        video.textram[i] = 0;
    }
    for (unsigned i = 0; i < sizeof(video.cursor_ram) / 2; i++) {
        video.cursor_ram[i] = 0;
    }
    for (unsigned i = 0; i < TLB_SIZE / 4; i++) {
        tlb[i] = 0;
    }

    ms_ticks = 0;
    vs_ticks = 0;

    memset(&cpu, 0, sizeof(cpu));
    cpu.pc     = 0xFF100000;
    cpu.m_mode = true;
    cpu.mcycle = 0;

    for (int i = 1; i < 32; i++) {
        cpu.regs[i] = 0xBEEF0000 | i;
    }

    video.init();
}

bool EmuState::loadRom(const std::string &path) {
    auto ifs = std::ifstream(path, std::ifstream::binary);
    if (!ifs.good()) {
        return false;
    }

    ifs.read((char *)(rom + 0x100000), sizeof(rom) - 0x100000);
    return true;
}

bool EmuState::memWrite(uint32_t paddr, uint32_t val, uint32_t mask) {
    if (/*paddr >= RAM_BASE &&*/ paddr < RAM_BASE + RAM_SIZE) {
        uint32_t *p = (uint32_t *)((uintptr_t)ram + ((paddr & ~3) - RAM_BASE));
        *p          = (*p & ~mask) | (val & mask);
        return true;
    }
    if (paddr >= TEXTRAM_BASE && paddr < TEXTRAM_BASE + TEXTRAM_SIZE) {
        video.textram[(paddr - TEXTRAM_BASE) >> 1] = val & 0xFFFF;
        return true;
    }
    if (paddr >= CURSOR_RAM_BASE && paddr < CURSOR_RAM_BASE + CURSOR_RAM_SIZE) {
        video.cursor_ram[(paddr - CURSOR_RAM_BASE) >> 1] = val & 0xFFFF;
        return true;
    }
    if (paddr >= TLB_BASE && paddr < TLB_BASE + TLB_SIZE) {
        tlb[((paddr - TLB_BASE) / 4) ^ (cpu.masid << 4)] = val;
        return true;
    }
    if (paddr >= VIDEO_BASE && paddr < VIDEO_BASE + VIDEO_SIZE) {
        video.writeReg((paddr >> 2) & 7, val);
        return true;
    }
    if (paddr >= UART1_BASE && paddr < UART1_BASE + UART1_SIZE) {
        // Print uart values to stdout
        if (((paddr >> 2) & 1) == 0)
            putchar(val & 0xFF);
        return true;
    }
    return false;
}

int64_t EmuState::memRead(uint32_t paddr) {
    if (/*paddr >= RAM_BASE &&*/ paddr < RAM_BASE + RAM_SIZE) {
        return *(uint32_t *)((uintptr_t)ram + ((paddr & ~3) - RAM_BASE));
    }
    if (paddr >= ROM_BASE && paddr < ROM_BASE + ROM_SIZE) {
        return *(uint32_t *)((uintptr_t)rom + ((paddr & ~3) - ROM_BASE));
    }
    if (paddr >= TEXTRAM_BASE && paddr < TEXTRAM_BASE + TEXTRAM_SIZE) {
        uint16_t val = video.textram[(paddr - TEXTRAM_BASE) >> 1];
        return (uint32_t)((val << 16) | (val << 0));
    }
    if (paddr >= CURSOR_RAM_BASE && paddr < CURSOR_RAM_BASE + CURSOR_RAM_SIZE) {
        uint16_t val = video.cursor_ram[(paddr - CURSOR_RAM_BASE) >> 1];
        return (uint32_t)((val << 16) | (val << 0));
    }
    if (paddr >= TLB_BASE && paddr < TLB_BASE + TLB_SIZE) {
        return tlb[((paddr - TLB_BASE) / 4) ^ (cpu.masid << 4)];
    }
    if (paddr >= VIDEO_BASE && paddr < VIDEO_BASE + VIDEO_SIZE) {
        return video.readReg((paddr >> 2) & 7);
    }
    if (paddr >= UART1_BASE && paddr < UART1_BASE + UART1_SIZE) {
        return 0;
    }
    return -1;
}

void EmuState::emulateStep() {
    cpu.emulate(1);
    if (++ms_ticks == 24000) {
        ms_ticks = 0;
    }
    // if (++vs_ticks == 200000) {
    //     vs_ticks = 0;
    //     // V-Blank interrupt
    //     // cpu.mip |= MIP_VBLANK;
    // }
}

void EmuState::emulate() {
    dbgIf.process();

    if (emuState.emuMode == EmuState::Em_Running) {
        // Emulate for 1/60s (@24MIPS).
        for (int i = 0; i < 24000000 / 60; i++) {
            if (emuState.enableBreakpoints && emuState.enableDebugger && lastBpAddress != emuState.cpu.pc) {
                for (auto &bp : emuState.breakpoints) {
                    if (bp.enabled && bp.addr == emuState.cpu.pc) {
                        lastBpAddress    = emuState.cpu.pc;
                        emuState.emuMode = EmuState::Em_Halted;
                        dbgIf.breakpointHit();
                        return;
                    }
                }
            }

            emulateStep();
            lastBpAddress = -1;
        }

    } else if (emuState.emuMode == EmuState::Em_Step) {
        emuState.emuMode = EmuState::Em_Halted;
        emulateStep();
        lastBpAddress = -1;
    }
}

void EmuState::keybScanCode(unsigned scancode, bool key_down) {
    // printf("scancode: %3u  down: %u\n", scancode, key_down);
}
