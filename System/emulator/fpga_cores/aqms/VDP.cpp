#include "VDP.h"

// TODO:
// - Sprite zooming
// - Japanese Y's bug (something with nametable mirroring?)
// - Maybe generate sprite line one line before to correctly(?) generate overflow bit
//

#define VDP_DEBUG 0

// vdpReg0:
// 7 VScroll inhibit
// 6 HScroll inhibit
// 5 Left col blank
// 4 IE1 - H-Line interrupt
// 3 Sprite shift bit
// 2 1
// 1 1
// 0 0

// vdpReg1:
// 7 1
// 6 Screen enable
// 5 IE - V-blank interrupt
// 4 0
// 3 0
// 2 0
// 1 16-pix sprite height
// 0 sprite mag bit

// vdpReg2: base address of screen map
// vdpReg3/4: always 0xFF
// vdpReg5: base address of sprite attribute table
// vdpReg6: base address of sprite patterns
// vdpReg7: border color index
// vdpReg8: horizontal scroll value
// vdpReg9: vertical scroll value
// vdpReg10: raster line interrupt

VDP::VDP() {
}

void VDP::reset() {
    line        = 0;
    lineCounter = 0xFF;

    bfToggle        = false;
    vsyncIrqPending = false;
    lineIrqPending  = false;
    sprOverflow     = false;
    sprCollision    = false;

    vdpCodeReg = 0;
    vdpAddr    = 0;
    readBuffer = 0;

    reg0VscrollInhibit = false;
    reg0HscrollInhibit = false;
    reg0LeftColBlank   = false;
    reg0LineIrqEn      = false;
    reg0SpriteShiftBit = false;
    reg1ScreenEn       = false;
    reg1VblankIrqEn    = false;
    reg1SprH16         = false;
    reg1SprMag         = false;
    reg2ScrMapBase     = 0;
    reg5SprAttrBase    = 0;
    reg6SprPatBase     = 0;
    reg7BorderColIdx   = 0;
    reg8HScroll        = 0;
    reg9VScroll        = 0;
    reg10RasterIrqLine = 0;

    reg9VScrollLatched = 0;
}

uint8_t VDP::readControlPort() {
    uint8_t result = 0;
    if (vsyncIrqPending)
        result |= 0x80;
    if (sprOverflow)
        result |= 0x40;
    if (sprCollision)
        result |= 0x20;

    bfToggle        = false;
    vsyncIrqPending = false;
    lineIrqPending  = false;
    sprOverflow     = false;
    sprCollision    = false;
    return result;
}

void VDP::writeControlPort(uint8_t data) {
    if (!bfToggle) {
        vdpAddr = (vdpAddr & 0x3F00) | data;
    } else {
        vdpCodeReg = data >> 6;
        vdpAddr    = ((data & 0x3F) << 8) | (vdpAddr & 0xFF);

        if (vdpCodeReg == 0) {
            // Read VDP ram
            readBuffer = vram[vdpAddr];
            vdpAddr    = (vdpAddr + 1) & 0x3FFF;

        } else if (vdpCodeReg == 2) {
            // Write VDP register
            uint8_t reg = (vdpAddr >> 8) & 0xF;
            uint8_t val = vdpAddr & 0xFF;

            switch (reg) {
                case 0: {
                    reg0VscrollInhibit = (val & 0x80) != 0;
                    reg0HscrollInhibit = (val & 0x40) != 0;
                    reg0LeftColBlank   = (val & 0x20) != 0;
                    reg0LineIrqEn      = (val & 0x10) != 0;
                    reg0SpriteShiftBit = (val & 0x08) != 0;
                    break;
                }
                case 1: {
                    reg1ScreenEn    = (val & 0x40) != 0;
                    reg1VblankIrqEn = (val & 0x20) != 0;
                    reg1SprH16      = (val & 0x02) != 0;
                    reg1SprMag      = (val & 0x01) != 0;
                    break;
                }
                case 2: reg2ScrMapBase = val; break;
                case 5: reg5SprAttrBase = val; break;
                case 6: reg6SprPatBase = val; break;
                case 7: reg7BorderColIdx = val & 0xF; break;
                case 8: reg8HScroll = val; break;
                case 9: reg9VScroll = val; break;
                case 10: reg10RasterIrqLine = val; break;
            }
        }
    }
    bfToggle = !bfToggle;
}

uint8_t VDP::readDataPort() {
    uint8_t result = readBuffer;
    readBuffer     = vram[vdpAddr];

    bfToggle = false;
    vdpAddr  = (vdpAddr + 1) & 0x3FFF;
    return result;
}

void VDP::writeDataPort(uint8_t data) {
    bfToggle = false;

    readBuffer = data;
    if (vdpCodeReg == 3) {
        uint8_t r = (data >> 0) & 3;
        uint8_t g = (data >> 2) & 3;
        uint8_t b = (data >> 4) & 3;

        r = (r << 6) | (r << 4) | (r << 2) | r;
        g = (g << 6) | (g << 4) | (g << 2) | g;
        b = (b << 6) | (b << 4) | (b << 2) | b;

        cram[vdpAddr & 31] = (b << 16) | (g << 8) | (r << 0);

    } else {
        vram[vdpAddr] = data;
        //        cout << "vram[" << toHex(vdpAddr) << "] = " << toHex(data) << endl;
    }

    vdpAddr = (vdpAddr + 1) & 0x3FFF;
}

uint8_t VDP::regVCounterRead() {
    // ntsc: 0x00 - 0xDA, 0xD5 - 0xFF
    return (line <= 0xDA) ? line : (line - (0xDA - 0xD5 + 1));
}

uint8_t VDP::regHCounterRead() {
    // H-counter port, not implemented
    return 0xFF;
}

bool VDP::isIrqPending() {
    return ((lineIrqPending && reg0LineIrqEn) || (vsyncIrqPending && reg1VblankIrqEn));
}

void VDP::renderBackground(uint32_t *lineBuf, uint8_t *pixelInFrontOfSpriteBuf) {
    uint8_t offsetX = (reg0HscrollInhibit && line < 16) ? 0 : reg8HScroll;
    int     offsetY = line + reg9VScrollLatched; // y-scroll

    uint16_t mapBase = (reg2ScrMapBase & 0x0E) << 9;
    uint16_t addr    = mapBase + 32 * ((offsetY / 8) % 28);

    // Render tilemap
    for (int i = 0; i < 32; i++) {
        uint16_t entryAddr = (addr + i) * 2;
        uint16_t entry     = vram[entryAddr + 0] | (vram[entryAddr + 1] << 8);

        uint16_t tileDataAddr         = (entry & 0x1FF) * 32;
        bool     flipHorizontal       = (entry & 0x200) != 0;
        bool     flipVertical         = (entry & 0x400) != 0;
        bool     useSpritePalette     = (entry & 0x800) != 0;
        bool     tileInFrontOfSprites = (entry & 0x1000) != 0;

        int y = flipVertical ? 7 - (offsetY & 7) : (offsetY & 7);

        uint32_t tileData =
            (vram[tileDataAddr + y * 4 + 0] << 0) |
            (vram[tileDataAddr + y * 4 + 1] << 8) |
            (vram[tileDataAddr + y * 4 + 2] << 16) |
            (vram[tileDataAddr + y * 4 + 3] << 24);

        for (int k = 0; k < 8; k++) {
            uint8_t colorIdx = 0;
            if (!flipHorizontal) {
                colorIdx =
                    ((tileData & (1 << ((7 - k) + 0))) ? 1 : 0) |
                    ((tileData & (1 << ((7 - k) + 8))) ? 2 : 0) |
                    ((tileData & (1 << ((7 - k) + 16))) ? 4 : 0) |
                    ((tileData & (1 << ((7 - k) + 24))) ? 8 : 0);
            } else {
                colorIdx =
                    ((tileData & (1 << (k + 0))) ? 1 : 0) |
                    ((tileData & (1 << (k + 8))) ? 2 : 0) |
                    ((tileData & (1 << (k + 16))) ? 4 : 0) |
                    ((tileData & (1 << (k + 24))) ? 8 : 0);
            }
            uint32_t color = cram[useSpritePalette ? (16 + colorIdx) : colorIdx];

            // background color index 0 is always behind sprites
            pixelInFrontOfSpriteBuf[offsetX] = colorIdx == 0 ? 0 : tileInFrontOfSprites;

            if (reg0LeftColBlank && offsetX < 8) {
                color                            = cram[16 + reg7BorderColIdx];
                pixelInFrontOfSpriteBuf[offsetX] = true;
            }
            lineBuf[offsetX++] = color;
        }
    }
}

void VDP::renderSprites(uint32_t *lineBuf, const uint8_t *pixelInFrontOfSpriteBuf) {
    uint8_t spritePixelDrawn[256];
    memset(spritePixelDrawn, 0, sizeof(spritePixelDrawn));

    int line2 = line - 1;

    uint16_t spritePatternBase   = ((reg6SprPatBase & 0x04) == 0) ? 0x0000 : 0x2000;
    uint16_t spriteAttributeBase = (reg5SprAttrBase & 0x7E) << 7;
    uint8_t  spritesDrawnCount   = 0;
    uint8_t  spritesToDraw[8];

    for (int i = 0; i < 64; i++) {
        int vpos = vram[spriteAttributeBase + i];
        if (vpos == 0xD0) {
            // terminator
            break;
        }
        if (vpos >= 240) {
            vpos -= 256;
        }

        int vpos2 = vpos + (reg1SprH16 ? 16 : 8) - 1;
        if (line2 < vpos || line2 > vpos2) {
            continue;
        }

        if (spritesDrawnCount >= 8) {
            sprOverflow = true;
            break;
        }
        spritesToDraw[spritesDrawnCount++] = i;
    }

    for (int i = spritesDrawnCount - 1; i >= 0; i--) {
        int spriteIdx = spritesToDraw[i];

        int vpos = vram[spriteAttributeBase + spriteIdx];
        if (vpos >= 240) {
            vpos -= 256;
        }

        uint8_t hpos     = vram[spriteAttributeBase + 0x80 + spriteIdx * 2 + 0];
        uint8_t charcode = vram[spriteAttributeBase + 0x80 + spriteIdx * 2 + 1];

        if (reg1SprH16) {
            charcode &= ~1;
        }

        uint8_t charline = line2 - vpos;
        if (charline >= 8) {
            charcode |= 1;
        }
        charline &= 7;

        uint16_t tileDataAddr = spritePatternBase | (charcode * 32);

        uint32_t tileData =
            (vram[tileDataAddr + charline * 4 + 0] << 0) |
            (vram[tileDataAddr + charline * 4 + 1] << 8) |
            (vram[tileDataAddr + charline * 4 + 2] << 16) |
            (vram[tileDataAddr + charline * 4 + 3] << 24);

        int hpos2 = (int)hpos + 8;
        if (hpos2 > 256) {
            hpos2 = 256;
        }

        for (int x = hpos, k = 0; x < hpos2; x++, k++) {
            uint8_t colorIdx =
                ((tileData & (1 << ((7 - k) + 0))) ? 1 : 0) |
                ((tileData & (1 << ((7 - k) + 8))) ? 2 : 0) |
                ((tileData & (1 << ((7 - k) + 16))) ? 4 : 0) |
                ((tileData & (1 << ((7 - k) + 24))) ? 8 : 0);

            if (colorIdx == 0) {
                continue;
            }

            uint32_t color = cram[16 + colorIdx];

            if ((reg0LeftColBlank && x < 8) || pixelInFrontOfSpriteBuf[x]) {
                continue;
            }

            if (spritePixelDrawn[x]) {
                // two sprites touched
                sprCollision = true;
            }

            lineBuf[x]          = color;
            spritePixelDrawn[x] = 1;
        }

        if (reg0SpriteShiftBit) {
            // printf("Sprite shift\n");
        }
    }
}

bool VDP::renderLine() {
    if (line == 0) {
        reg9VScrollLatched = reg9VScroll;
    }

    if (line < 192) {
        uint32_t *fbp = &framebuffer[line * 256];
        if (!reg1ScreenEn) {
            memset(fbp, 0, 256 * sizeof(uint32_t));
        } else {
            uint8_t pixelInFrontOfSprites[256];
            renderBackground(fbp, pixelInFrontOfSprites);
            renderSprites(fbp, pixelInFrontOfSprites);
        }
    }

    if (line == 193) {
        vsyncIrqPending = true;
    }

    if (line < 193) {
        lineCounter--;
        if (lineCounter == 0xFF) {
            lineCounter = reg10RasterIrqLine;
        }
    } else {
        lineCounter = reg10RasterIrqLine;
    }

    if (line == 261) {
        line = 0;
        return true;
    }

    line++;
    return false;
}
