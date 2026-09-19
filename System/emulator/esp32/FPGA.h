#pragma once

#include "Common.h"

enum {
    // Aq+ command
    CMD_RESET           = 0x01,
    CMD_FORCE_TURBO     = 0x02,
    CMD_SET_KEYB_MATRIX = 0x10,
    CMD_SET_HCTRL       = 0x11,
    CMD_WRITE_KBBUF     = 0x12,
    CMD_WRITE_KBBUF16   = 0x13,
    CMD_WRITE_GAMEPAD1  = 0x14,
    CMD_WRITE_GAMEPAD2  = 0x15,
    CMD_BUS_ACQUIRE     = 0x20,
    CMD_BUS_RELEASE     = 0x21,
    CMD_MEM_WRITE       = 0x22,
    CMD_MEM_READ        = 0x23,
    CMD_IO_WRITE        = 0x24,
    CMD_IO_READ         = 0x25,
    CMD_ROM_WRITE       = 0x30,
    CMD_SET_VIDMODE     = 0x40,

    // General commands
    CMD_GET_KEYS    = 0xF1, // MorphBook specific
    CMD_SET_VOLUME  = 0xF3, // MorphBook specific
    CMD_OVL_TEXT    = 0xF4,
    CMD_OVL_FONT    = 0xF5,
    CMD_OVL_PALETTE = 0xF6,
    CMD_GET_STATUS  = 0xF7,
    CMD_GET_SYSINFO = 0xF8,
    CMD_GET_NAME1   = 0xF9,
    CMD_GET_NAME2   = 0xFA,
};

struct CoreInfo {
    uint8_t coreType;
    uint8_t flags;
    uint8_t versionMajor;
    uint8_t versionMinor;
    char    name[17];
};

class FPGA {
public:
    static FPGA *instance();

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
