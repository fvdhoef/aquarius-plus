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

    for (unsigned i = 0; i < sizeof(emuState.screenRam); i++) {
        emuState.screenRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.colorRam); i++) {
        emuState.colorRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.systemRom); i++) {
        emuState.systemRom[i] = 0xFF;
    }
    for (unsigned i = 0; i < sizeof(emuState.mainRam); i++) {
        emuState.mainRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.videoRam); i++) {
        emuState.videoRam[i] = rand();
    }
    for (unsigned i = 0; i < sizeof(emuState.charRam); i++) {
        emuState.charRam[i] = rand();
    }

    z80ctx.ioRead   = ioRead;
    z80ctx.ioWrite  = ioWrite;
    z80ctx.memRead  = memRead;
    z80ctx.memWrite = memWrite;
}

void EmuState::reset() {
    Z80RESET(&z80ctx);
    cpmRemap = false;
    ay1.reset();
    ay2.reset();
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
    if (typeInRelease > 0) {
        typeInRelease--;
        if (typeInRelease > 0)
            return;
        AqKeyboard::instance().pressKey(typeInChar, false);
        typeInDelay = 1;
        return;
    }

    if (typeInDelay > 0) {
        typeInDelay--;
        if (typeInDelay > 0)
            return;
    }

    if (typeInStr.size() == 0)
        return;

    char ch = typeInStr.front();
    typeInStr.erase(typeInStr.begin());
    if (ch == '\\') {
        if (typeInStr.size() == 0)
            return;

        ch = typeInStr.front();
        typeInStr.erase(typeInStr.begin());
        if (ch == 'n') {
            ch = '\n';
        }
    }
    typeInChar = ch;
    AqKeyboard::instance().pressKey(typeInChar, true);
    typeInRelease = ch == '\n' ? 10 : 1;
}

uint8_t EmuState::memRead(size_t param, uint16_t addr) {
    (void)param;

    // Handle CPM remap bit
    if (emuState.cpmRemap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    // Get and decode banking register
    uint8_t  bankreg     = emuState.bankRegs[addr >> 14];
    unsigned page        = bankreg & 0x3F;
    bool     overlay_ram = (bankreg & (1 << 6)) != 0;

    addr &= 0x3FFF;

    if (overlay_ram && addr >= 0x3000) {
        if (addr < 0x3400) {
            return emuState.screenRam[addr & 0x3FF];
        } else if (addr < 0x3800) {
            return emuState.colorRam[addr & 0x3FF];
        } else {
            return emuState.mainRam[addr];
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

void EmuState::memWrite(size_t param, uint16_t addr, uint8_t data) {
    (void)param;

    // Handle CPM remap bit
    if (emuState.cpmRemap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    // Get and decode banking register
    uint8_t  bankreg     = emuState.bankRegs[addr >> 14];
    unsigned page        = bankreg & 0x3F;
    bool     overlay_ram = (bankreg & (1 << 6)) != 0;
    bool     readonly    = (bankreg & (1 << 7)) != 0;
    addr &= 0x3FFF;

    if (overlay_ram && addr >= 0x3000) {
        if (addr < 0x3400) {
            emuState.screenRam[addr & 0x3FF] = data;
        } else if (addr < 0x3800) {
            emuState.colorRam[addr & 0x3FF] = data;
        } else {
            emuState.mainRam[addr] = data;
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

uint8_t EmuState::ioRead(size_t param, ushort addr) {
    (void)param;

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
                return emuState.ay2.readReg(emuState.ay1Addr);

        case 0xFB: return (
            (emuState.sysCtrlDisableExt ? (1 << 0) : 0) |
            (emuState.sysCtrlAyDisable ? (1 << 1) : 0));

        case 0xFC: /* printf("Cassette port input (%04x)\n", addr); */ return 0xFF;
        case 0xFD: return (emuState.videoLine >= 242) ? 0 : 1;
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

void EmuState::ioWrite(size_t param, uint16_t addr, uint8_t data) {
    (void)param;

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
            case 0xEC: return;
            case 0xED: emuState.videoIrqLine = data; return;
            case 0xEE: emuState.irqMask = data & 3; return;
            case 0xEF: emuState.irqStatus &= ~data; return;
            case 0xF0: emuState.bankRegs[0] = data; return;
            case 0xF1: emuState.bankRegs[1] = data; return;
            case 0xF2: emuState.bankRegs[2] = data; return;
            case 0xF3: emuState.bankRegs[3] = data; return;
            case 0xF4: AqUartProtocol::instance().writeCtrl(data); return;
            case 0xF5: AqUartProtocol::instance().writeData(data); return;
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
            return;

        case 0xFC: emuState.soundOutput = (data & 1) != 0; break;
        case 0xFD: emuState.cpmRemap = (data & 1) != 0; break;
        case 0xFE: /* printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); */ break;
        case 0xFF: break;
        default: printf("ioWrite(0x%02x, 0x%02x)\n", addr & 0xFF, data); break;
    }
}
