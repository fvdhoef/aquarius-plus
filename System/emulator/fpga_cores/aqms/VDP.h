#pragma once

#include "Common.h"

class VDP {
public:
    VDP();

    void reset();

    bool renderLine();

    // Registers
    uint8_t readControlPort();
    void    writeControlPort(uint8_t data);

    uint8_t readDataPort();
    void    writeDataPort(uint8_t data);

    uint8_t regVCounterRead();
    uint8_t regHCounterRead();

    const uint32_t *getFramebuffer() { return framebuffer; }

    // private:
    void renderBackground(uint32_t *lineBuf, uint8_t *pixelInFrontOfSpriteBuf);
    void renderSprites(uint32_t *lineBuf, const uint8_t *pixelInFrontOfSpriteBuf);

    bool isIrqPending();

    uint32_t framebuffer[256 * 192];

    uint8_t  vram[0x4000];
    uint32_t cram[32] = {0};

    uint16_t line        = 0;
    uint8_t  lineCounter = 0xFF;

    // uint8_t vdpReg[11] = {0x06, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0xFF};
    // uint8_t vdpReg9latched;

    bool bfToggle        = false;
    bool vsyncIrqPending = false;
    bool lineIrqPending  = false;
    bool sprOverflow     = false;
    bool sprCollision    = false;

    uint8_t  vdpCodeReg = 0;
    uint16_t vdpAddr    = 0;
    uint8_t  readBuffer = 0;

    bool    reg0VscrollInhibit = false; // 1: Disable vertical scrolling for columns 24-31
    bool    reg0HscrollInhibit = false; // 1: Disable horizontal scrolling for rows 0-1
    bool    reg0LeftColBlank   = false; // 1: Mask column 0 with overscan color from register #7
    bool    reg0LineIrqEn      = false; // 1: Line interrupt enable
    bool    reg0SpriteShiftBit = false; // 1: Shift sprites left by 8 pixels
    bool    reg1ScreenEn       = false; // 1: Display visible, 0: display blanked.
    bool    reg1VblankIrqEn    = false; // 1: Frame interrupt enable.
    bool    reg1SprH16         = false; // Sprites are 1:8x16, 0:8x8
    bool    reg1SprMag         = false; // Sprite pixels are doubled in size
    uint8_t reg2ScrMapBase     = 0;     // [3:1] = bit [13:11] of Name Table Base Address
    uint8_t reg5SprAttrBase    = 0;     // [6:1] = bit [13:8] of Sprite Attribute Table Base Address
    uint8_t reg6SprPatBase     = 0;     // [2] = bit [13] of Sprite Pattern Generator Base Address
    uint8_t reg7BorderColIdx   = 0;     // [3:0] = Overscan/Backdrop Color
    uint8_t reg8HScroll        = 0;     // Background X Scroll
    uint8_t reg9VScroll        = 0;     // Background Y Scroll
    uint8_t reg10RasterIrqLine = 0;     // Line counter

    uint8_t reg9VScrollLatched = 0;
};
