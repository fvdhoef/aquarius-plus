#pragma once

#include "Common.h"

struct CoreInfo {
    uint8_t coreType;
    uint8_t flags;
    uint8_t versionMajor;
    uint8_t versionMinor;
    char    name[17];
};

class FPGA {
public:
    virtual void init() = 0;

    // FPGA configuration
    virtual bool loadBitstream(const void *data, size_t length) = 0;
    virtual bool getCoreInfo(CoreInfo *info)                    = 0;

#ifdef CONFIG_MACHINE_TYPE_MORPHBOOK
    // FPGA core interface
    virtual uint64_t getKeys()                              = 0;
    virtual void     setVolume(uint16_t volume, bool spkEn) = 0;
#endif

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
