#include "EmuState.h"
#include <stdlib.h>
#include "AqUartProtocol.h"
#include "AqKeyboard.h"

EmuState emuState;

EmuState::EmuState() {
    memset(emuState.keybMatrix, 0xFF, sizeof(emuState.keybMatrix));
    emuState.handCtrl1 = 0xFF;
    emuState.handCtrl2 = 0xFF;

    static const uint16_t defaultPalette[] = {
        0x111, 0xF11, 0x1F1, 0xFF1, 0x22E, 0xF1F, 0x3CC, 0xFFF,
        0xCCC, 0x3BB, 0xC2C, 0x419, 0xFF7, 0x2D4, 0xB22, 0x333};
    memcpy(emuState.videoPalette + 0, defaultPalette, sizeof(defaultPalette));
    memcpy(emuState.videoPalette + 16, defaultPalette, sizeof(defaultPalette));
    memcpy(emuState.videoPalette + 32, defaultPalette, sizeof(defaultPalette));
    memcpy(emuState.videoPalette + 48, defaultPalette, sizeof(defaultPalette));

    memset(emuState.screenRam, 0, sizeof(emuState.screenRam));
    memset(emuState.colorRam, 0, sizeof(emuState.colorRam));
    memset(emuState.systemRom, 0xFF, sizeof(emuState.systemRom));
    for (unsigned i = 0; i < sizeof(emuState.mainRam); i++) {
        emuState.mainRam[i] = rand();
    }
    memset(emuState.videoRam, 0, sizeof(emuState.videoRam));
    memset(emuState.charRam, 0, sizeof(emuState.charRam));

    z80ctx.ioRead   = _ioRead;
    z80ctx.ioWrite  = _ioWrite;
    z80ctx.memRead  = _memRead;
    z80ctx.memWrite = _memWrite;
}

void EmuState::reset() {
    // Reset registers
    videoCtrl         = 0;
    videoScrX         = 0;
    videoScrY         = 0;
    videoSprSel       = 0;
    videoPalSel       = 0;
    videoLine         = 0;
    audioDAC          = 0;
    videoIrqLine      = 0;
    irqMask           = 0;
    irqStatus         = 0;
    bankRegs[0]       = 0xC0 | 0;
    bankRegs[1]       = 33;
    bankRegs[2]       = 34;
    bankRegs[3]       = 19;
    ay1Addr           = 0;
    ay2Addr           = 0;
    sysCtrlDisableExt = false;
    sysCtrlAyDisable  = false;
    sysCtrlTurbo      = false;
    soundOutput       = false;
    cpmRemap          = false;

    Z80RESET(&z80ctx);
    ay1.reset();
    ay2.reset();
    kbBufReset();

    emuMode = Em_Running;
}

int EmuState::cpuEmulate() {
    bool haltAfterThis = false;

    if (enableDebugger) {
        if (tmpBreakpoint == z80ctx.PC) {
            tmpBreakpoint = -1;
            emuMode       = EmuState::Em_Halted;
            return 0;
        }

        if (enableBreakpoints) {
            for (int i = 0; i < (int)breakpoints.size(); i++) {
                auto &bp = breakpoints[i];
                if (bp.enabled && bp.type == 0 && bp.onX && z80ctx.PC == bp.addr && bp.addr != lastBpAddress) {
                    emuMode       = EmuState::Em_Halted;
                    lastBp        = i;
                    lastBpAddress = bp.addr;
                    return 0;
                }
            }
        }

        if (haltAfterRet >= 0) {
            uint8_t opcode = emuState.memRead(emuState.z80ctx.PC);
            if (opcode == 0xCD ||          // CALL nn
                (opcode & 0xC7) == 0xC4) { // CALL c,nn

                haltAfterRet++;

            } else if (
                opcode == 0xC9 ||          // RET
                (opcode & 0xC7) == 0xC7) { // RET cc

                haltAfterRet--;

                if (haltAfterRet < 0) {
                    haltAfterThis = true;
                    haltAfterRet  = -1;
                }
            }
        }
    }
    lastBp = -1;

    // Generate interrupt if needed
    if ((irqStatus & irqMask) != 0) {
        Z80INT(&z80ctx, 0xFF);
    }

    z80ctx.tstates = 0;
    Z80Execute(&z80ctx);
    int delta = z80ctx.tstates * 2;

    if (enableDebugger) {
        if (traceEnable && (!z80ctx.halted || !prevHalted)) {
            cpuTrace.emplace_back();
            auto &entry = cpuTrace.back();
            entry.pc    = z80ctx.PC;
            entry.r1    = z80ctx.R1;
            entry.r2    = z80ctx.R2;

            z80ctx.tstates = 0;
            Z80Debug(&z80ctx, entry.bytes, entry.instrStr);

            while ((int)cpuTrace.size() > traceDepth) {
                cpuTrace.pop_front();
            }
        }
        prevHalted = z80ctx.halted;

        if (haltAfterThis || z80ctx.halted) {
            emuMode = EmuState::Em_Halted;
        }
    }
    return delta;
}

unsigned EmuState::emulate() {
    unsigned resultFlags = 0;

    int delta = cpuEmulate();
    if (emuState.sysCtrlTurbo) {
        if (emuMode != EmuState::Em_Halted)
            delta += cpuEmulate();
        delta /= 2;
    }

    int prevLineHalfCycles = lineHalfCycles;
    lineHalfCycles += delta;
    sampleHalfCycles += delta;

    // Handle VIRQLINE register
    if (prevLineHalfCycles < 320 && lineHalfCycles >= 320 && videoLine == videoIrqLine) {
        irqStatus |= (1 << 1);
    }

    if (lineHalfCycles >= HCYCLES_PER_LINE) {
        lineHalfCycles -= HCYCLES_PER_LINE;

        video.drawLine();

        videoLine++;
        if (videoLine == 200) {
            irqStatus |= (1 << 0);

        } else if (videoLine == 262) {
            resultFlags |= ERF_RENDER_SCREEN;
            videoLine = 0;
        }
    }
    keyboardTypeIn();

    // Render audio?
    if (sampleHalfCycles >= HCYCLES_PER_SAMPLE) {
        sampleHalfCycles -= HCYCLES_PER_SAMPLE;

        // Take average of 5 AY8910 samples to match sampling rate (16*5*44100 = 3.528MHz)
        audioLeft  = 0;
        audioRight = 0;

        for (int i = 0; i < 5; i++) {
            uint16_t abc[3];
            ay1.render(abc);
            audioLeft += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
            audioRight += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];

            ay2.render(abc);
            audioLeft += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
            audioRight += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];

            audioLeft += (audioDAC << 4);
            audioRight += (audioDAC << 4);
        }

        uint16_t beep = soundOutput ? 10000 : 0;

        audioLeft  = audioLeft + beep;
        audioRight = audioRight + beep;
        resultFlags |= ERF_NEW_AUDIO_SAMPLE;
    }

    if (delta)
        lastBpAddress = -1;

    return resultFlags;
}

bool EmuState::loadSystemROM(const std::string &path) {
    auto ifs = std::ifstream(path, std::ifstream::binary);
    if (!ifs.good()) {
        return false;
    }

    ifs.read((char *)systemRom, sizeof(systemRom));
    if ((emuState.systemRomSize = ifs.gcount()) < 8192) {
        fprintf(stderr, "Error during reading of system ROM image.\n");
        return false;
    }
    emuState.systemRomSize = (emuState.systemRomSize + 0x1FFF) & ~0x1FFF;
    return true;
}

bool EmuState::loadCartridgeROM(const std::string &path) {
    auto ifs = std::ifstream(path, std::ifstream::binary);
    if (!ifs.good()) {
        return false;
    }

    ifs.seekg(0, ifs.end);
    auto fileSize = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    if (fileSize == 8192) {
        ifs.read((char *)(cartRom + 8192), fileSize);
        // Mirror ROM to $C000
        memcpy(cartRom, cartRom + 8192, 8192);

    } else if (fileSize == 16384) {
        ifs.read((char *)cartRom, fileSize);

    } else {
        fprintf(stderr, "Invalid cartridge ROM file: %u, should be either exactly 8 or 16KB.\n", (unsigned)fileSize);
        return false;
    }
    ifs.close();

    cartridgeInserted = true;

    return true;
}

void EmuState::keyboardTypeIn() {
    if (emuState.kbBufCnt < 16 && !typeInStr.empty()) {
        char ch = typeInStr.front();
        typeInStr.erase(typeInStr.begin());
        AqKeyboard::instance().pressKey(ch);
    }
}

uint8_t EmuState::memRead(uint16_t addr, bool triggerBp) {
    if (emuState.enableBreakpoints && triggerBp) {
        for (int i = 0; i < (int)emuState.breakpoints.size(); i++) {
            auto &bp = emuState.breakpoints[i];
            if (bp.enabled && bp.onR && bp.type == 0 && addr == bp.addr && bp.addr != emuState.lastBpAddress) {
                emuState.emuMode       = EmuState::Em_Halted;
                emuState.lastBp        = i;
                emuState.lastBpAddress = bp.addr;
            }
        }
    }

    // Handle CPM remap bit
    if (emuState.cpmRemap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    // Get and decode banking register
    uint8_t  bankReg    = emuState.bankRegs[addr >> 14];
    unsigned page       = bankReg & 0x3F;
    bool     overlayRam = (bankReg & (1 << 6)) != 0;

    addr &= 0x3FFF;

    if (overlayRam && addr >= 0x3000) {
        if (addr >= 0x3800) {
            return emuState.mainRam[addr];
        }

        if (emuState.videoCtrl & VCTRL_80_COLUMNS) {
            if (emuState.videoCtrl & VCTRL_TRAM_PAGE) {
                return emuState.colorRam[addr & 0x7FF];
            } else {
                return emuState.screenRam[addr & 0x7FF];
            }

        } else {
            unsigned offset = (emuState.videoCtrl & VCTRL_TRAM_PAGE) ? 0x400 : 0;
            if (addr < 0x3400) {
                return emuState.screenRam[offset | (addr & 0x3FF)];
            } else {
                return emuState.colorRam[offset | (addr & 0x3FF)];
            }
        }
    }

    if (page < 16) {
        return emuState.systemRom[page * 0x4000 + addr];
    } else if (page == 19) {
        return emuState.cartridgeInserted ? emuState.cartRom[addr] : 0xFF;
    } else if (page == 20) {
        return emuState.videoRam[addr];
    } else if (page == 21) {
        if (addr < 0x800) {
            return emuState.charRam[addr];
        }
    } else if (page >= 32 && page < 64) {
        return emuState.mainRam[(page - 32) * 0x4000 + addr];
    }
    return 0xFF;
}

void EmuState::memWrite(uint16_t addr, uint8_t data, bool triggerBp) {
    if (emuState.enableBreakpoints && triggerBp) {
        for (int i = 0; i < (int)emuState.breakpoints.size(); i++) {
            auto &bp = emuState.breakpoints[i];
            if (bp.enabled && bp.onW && bp.type == 0 && addr == bp.addr && bp.addr != emuState.lastBpAddress) {
                emuState.emuMode       = EmuState::Em_Halted;
                emuState.lastBp        = i;
                emuState.lastBpAddress = bp.addr;
            }
        }
    }

    // Handle CPM remap bit
    if (emuState.cpmRemap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    // Get and decode banking register
    uint8_t  bankReg    = emuState.bankRegs[addr >> 14];
    unsigned page       = bankReg & 0x3F;
    bool     overlayRam = (bankReg & (1 << 6)) != 0;
    bool     readonly   = (bankReg & (1 << 7)) != 0;
    addr &= 0x3FFF;

    if (overlayRam && addr >= 0x3000) {
        if (addr >= 0x3800) {
            emuState.mainRam[addr] = data;
            return;
        }

        if (emuState.videoCtrl & VCTRL_80_COLUMNS) {
            if (emuState.videoCtrl & VCTRL_TRAM_PAGE) {
                emuState.colorRam[addr & 0x7FF] = data;
            } else {
                emuState.screenRam[addr & 0x7FF] = data;
            }

        } else {
            unsigned offset = (emuState.videoCtrl & VCTRL_TRAM_PAGE) ? 0x400 : 0;
            if (addr < 0x3400) {
                emuState.screenRam[offset | (addr & 0x3FF)] = data;
            } else {
                emuState.colorRam[offset | (addr & 0x3FF)] = data;
            }
        }
        return;
    }

    if (readonly) {
        return;
    }

    if (page < 16) {
        // System ROM is readonly
        return;
    } else if (page == 19) {
        // Game ROM is readonly
        return;
    } else if (page == 20) {
        emuState.videoRam[addr] = data;
    } else if (page == 21) {
        if (addr < 0x800) {
            emuState.charRam[addr] = data;
        }
    } else if (page >= 32 && page < 64) {
        emuState.mainRam[(page - 32) * 0x4000 + addr] = data;
    }
}

uint8_t EmuState::ioRead(uint16_t addr, bool triggerBp) {
    if (emuState.enableBreakpoints && triggerBp) {
        for (int i = 0; i < (int)emuState.breakpoints.size(); i++) {
            auto &bp = emuState.breakpoints[i];
            if (bp.enabled && bp.onR && ((bp.type == 1 && (addr & 0xFF) == (bp.addr & 0xFF)) || (bp.type == 2 && addr == bp.addr))) {
                emuState.emuMode = EmuState::Em_Halted;
                emuState.lastBp  = i;
            }
        }
    }

    if (!emuState.sysCtrlDisableExt) {
        switch (addr & 0xFF) {
            case 0xE0: return emuState.videoCtrl;
            case 0xE1: return emuState.videoScrX & 0xFF;
            case 0xE2: return emuState.videoScrX >> 8;
            case 0xE3: return emuState.videoScrY;
            case 0xE4: return emuState.videoSprSel;
            case 0xE5: return emuState.videoSprX[emuState.videoSprSel] & 0xFF;
            case 0xE6: return emuState.videoSprX[emuState.videoSprSel] >> 8;
            case 0xE7: return emuState.videoSprY[emuState.videoSprSel];
            case 0xE8: return emuState.videoSprIdx[emuState.videoSprSel] & 0xFF;
            case 0xE9: return (
                (emuState.videoSprAttr[emuState.videoSprSel] & 0xFE) |
                ((emuState.videoSprIdx[emuState.videoSprSel] >> 8) & 1));
            case 0xEA: return emuState.videoPalSel;
            case 0xEB: return (emuState.videoPalette[emuState.videoPalSel >> 1] >> ((emuState.videoPalSel & 1) * 8)) & 0xFF;
            case 0xEC: return emuState.videoLine < 255 ? emuState.videoLine : 255;
            case 0xED: return emuState.videoIrqLine;
            case 0xEE: return emuState.irqMask;
            case 0xEF: return emuState.irqStatus;
            case 0xF0: return emuState.bankRegs[0];
            case 0xF1: return emuState.bankRegs[1];
            case 0xF2: return emuState.bankRegs[2];
            case 0xF3: return emuState.bankRegs[3];
            case 0xF4: return AqUartProtocol::instance().readCtrl();
            case 0xF5: return AqUartProtocol::instance().readData();
        }
    }

    switch (addr & 0xFF) {
        case 0xF6:
        case 0xF7:
            if (emuState.sysCtrlAyDisable)
                return 0xFF;
            else {
                switch (emuState.ay1Addr) {
                    case 14: return emuState.handCtrl1;
                    case 15: return emuState.handCtrl2;
                    default: return emuState.ay1.readReg(emuState.ay1Addr);
                }
            }
            break;

        case 0xF8:
        case 0xF9:
            if (emuState.sysCtrlAyDisable || emuState.sysCtrlDisableExt)
                return 0xFF;
            else
                return emuState.ay2.readReg(emuState.ay2Addr);

        case 0xFA: return emuState.kbBufRead();
        case 0xFB: return (
            (emuState.sysCtrlTurbo ? (1 << 2) : 0) |
            (emuState.sysCtrlAyDisable ? (1 << 1) : 0) |
            (emuState.sysCtrlDisableExt ? (1 << 0) : 0));

        case 0xFC: /* printf("Cassette port input (%04x)\n", addr); */ return 0xFF;
        case 0xFD: return (emuState.videoLine >= 16 && emuState.videoLine <= 216) ? 1 : 0;
        case 0xFE: /* printf("Clear to send status (%04x)\n", addr); */ return 0xFF;
        case 0xFF: {
            // Keyboard matrix. Selected rows are passed in the upper 8 address lines.
            uint8_t rows = addr >> 8;

            // Wire-AND all selected rows.
            uint8_t result = 0xFF;
            for (int i = 0; i < 8; i++) {
                if ((rows & (1 << i)) == 0) {
                    result &= emuState.keybMatrix[i];
                }
            }
            return result;
        }
        default: break;
    }

    printf("ioRead(0x%02x)\n", addr & 0xFF);
    return 0xFF;
}

void EmuState::ioWrite(uint16_t addr, uint8_t data, bool triggerBp) {
    if (emuState.enableBreakpoints && triggerBp) {
        for (int i = 0; i < (int)emuState.breakpoints.size(); i++) {
            auto &bp = emuState.breakpoints[i];
            if (bp.enabled && bp.onW && ((bp.type == 1 && (addr & 0xFF) == (bp.addr & 0xFF)) || (bp.type == 2 && addr == bp.addr))) {
                emuState.emuMode = EmuState::Em_Halted;
                emuState.lastBp  = i;
            }
        }
    }

    if (!emuState.sysCtrlDisableExt) {
        switch (addr & 0xFF) {
            case 0xE0: emuState.videoCtrl = data; return;
            case 0xE1: emuState.videoScrX = (emuState.videoScrX & ~0xFF) | data; return;
            case 0xE2: emuState.videoScrX = (emuState.videoScrX & 0xFF) | ((data & 1) << 8); return;
            case 0xE3: emuState.videoScrY = data; return;
            case 0xE4: emuState.videoSprSel = data & 0x3F; return;
            case 0xE5: emuState.videoSprX[emuState.videoSprSel] = (emuState.videoSprX[emuState.videoSprSel] & ~0xFF) | data; return;
            case 0xE6: emuState.videoSprX[emuState.videoSprSel] = (emuState.videoSprX[emuState.videoSprSel] & 0xFF) | ((data & 1) << 8); return;
            case 0xE7: emuState.videoSprY[emuState.videoSprSel] = data; return;
            case 0xE8: emuState.videoSprIdx[emuState.videoSprSel] = (emuState.videoSprIdx[emuState.videoSprSel] & ~0xFF) | data; return;
            case 0xE9:
                emuState.videoSprAttr[emuState.videoSprSel] = data & 0xFE;
                emuState.videoSprIdx[emuState.videoSprSel]  = (emuState.videoSprIdx[emuState.videoSprSel] & 0xFF) | ((data & 1) << 8);
                return;
            case 0xEA: emuState.videoPalSel = data & 0x7F; return;
            case 0xEB:
                if ((emuState.videoPalSel & 1) == 0) {
                    emuState.videoPalette[emuState.videoPalSel >> 1] =
                        (emuState.videoPalette[emuState.videoPalSel >> 1] & 0xF00) | data;
                } else {
                    emuState.videoPalette[emuState.videoPalSel >> 1] =
                        ((data & 0xF) << 8) | (emuState.videoPalette[emuState.videoPalSel >> 1] & 0xFF);
                }
                return;
            case 0xEC: emuState.audioDAC = data; return;
            case 0xED: emuState.videoIrqLine = data; return;
            case 0xEE: emuState.irqMask = data & 3; return;
            case 0xEF: emuState.irqStatus &= ~data; return;
            case 0xF0: emuState.bankRegs[0] = data; return;
            case 0xF1: emuState.bankRegs[1] = data; return;
            case 0xF2: emuState.bankRegs[2] = data; return;
            case 0xF3: emuState.bankRegs[3] = data; return;
            case 0xF4: AqUartProtocol::instance().writeCtrl(data); return;
            case 0xF5: AqUartProtocol::instance().writeData(data); return;
            case 0xFA: emuState.kbBufReset(); return;
        }
    }

    switch (addr & 0xFF) {
        case 0xF6:
            if (!emuState.sysCtrlAyDisable && emuState.ay1Addr < 14)
                emuState.ay1.writeReg(emuState.ay1Addr, data);
            return;

        case 0xF7:
            if (!emuState.sysCtrlAyDisable)
                emuState.ay1Addr = data;
            return;

        case 0xF8:
            if (!(emuState.sysCtrlAyDisable || emuState.sysCtrlDisableExt) && emuState.ay2Addr < 14)
                emuState.ay2.writeReg(emuState.ay2Addr, data);
            return;

        case 0xF9:
            if (!(emuState.sysCtrlAyDisable || emuState.sysCtrlDisableExt))
                emuState.ay2Addr = data;
            return;

        case 0xFB:
            emuState.sysCtrlDisableExt = (data & (1 << 0)) != 0;
            emuState.sysCtrlAyDisable  = (data & (1 << 1)) != 0;
            emuState.sysCtrlTurbo      = (data & (1 << 2)) != 0;
            return;

        case 0xFC: emuState.soundOutput = (data & 1) != 0; break;
        case 0xFD: emuState.cpmRemap = (data & 1) != 0; break;
        case 0xFE: /* printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); */ break;
        case 0xFF: break;
        default: printf("ioWrite(0x%02x, 0x%02x)\n", addr & 0xFF, data); break;
    }
}

void EmuState::kbBufReset() {
    kbBufCnt   = 0;
    kbBufRdIdx = 0;
    kbBufWrIdx = 0;
}

void EmuState::kbBufWrite(uint8_t val) {
    if (kbBufCnt < sizeof(kbBuf)) {
        kbBufCnt++;
        kbBuf[kbBufWrIdx++] = val;
        if (kbBufWrIdx == sizeof(kbBuf))
            kbBufWrIdx = 0;
    }
}

uint8_t EmuState::kbBufRead() {
    uint8_t result = 0;
    if (kbBufCnt > 0) {
        result = kbBuf[kbBufRdIdx++];
        if (kbBufRdIdx == sizeof(kbBuf))
            kbBufRdIdx = 0;
        kbBufCnt--;
    }
    return result;
}
