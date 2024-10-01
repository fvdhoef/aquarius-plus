#pragma once

#include "Common.h"

enum {
    IO_BANK0 = 0xF0,
    IO_BANK1 = 0xF1,
    IO_BANK2 = 0xF2,
    IO_BANK3 = 0xF3,
};

class FPGA {
public:
    virtual void init() = 0;

    // FPGA configuration
    virtual bool loadBitstream(const void *data, size_t length) = 0;

    // Display overlay
    virtual void setOverlayText(const uint16_t buf[1024])  = 0;
    virtual void setOverlayFont(const uint8_t buf[2048])   = 0;
    virtual void setOverlayPalette(const uint16_t buf[16]) = 0;

    // To be used by core specific handlers
    virtual SemaphoreHandle_t getMutex()                             = 0;
    virtual void              spiSel(bool enable)                    = 0;
    virtual void              spiTx(const void *data, size_t length) = 0;
    virtual void              spiRx(void *buf, size_t length)        = 0;
};

FPGA *getFPGA();
