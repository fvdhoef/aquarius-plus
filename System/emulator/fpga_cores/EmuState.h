#pragma once

#include "Common.h"

#define ERF_RENDER_SCREEN    (1 << 0)
#define ERF_NEW_AUDIO_SAMPLE (1 << 1)

class EmuState {
public:
    static std::shared_ptr<EmuState> get();
    static void                      loadCore(const std::string &name);

    virtual void reset(bool cold = false)                             = 0;
    virtual void emulateFrame(int16_t *audioBuf, unsigned numSamples) = 0;
    virtual void getVideoSize(int &w, int &h)                         = 0;
    virtual void getPixels(void *pixels, int pitch)                   = 0;

    virtual void spiSel(bool enable);
    virtual void spiTx(const void *data, size_t length);
    virtual void spiRx(void *buf, size_t length);

    virtual void fileMenu() {}
    virtual void pasteText(const std::string &str) {}
    virtual bool pasteIsDone() { return true; }

    virtual bool getDebuggerEnabled() { return enableDebugger; };
    virtual void setDebuggerEnabled(bool en) { enableDebugger = en; };
    virtual void dbgMenu()    = 0;
    virtual void dbgWindows() = 0;

    virtual void unloading() {}

protected:
    // Core info
    uint8_t coreType         = 0;
    uint8_t coreFlags        = 0;
    uint8_t coreVersionMajor = 0;
    uint8_t coreVersionMinor = 0;
    char    coreName[17];

    // Debugging
    bool enableDebugger = false;

    // SPI interface
    bool                 spiSelected = false;
    std::vector<uint8_t> txBuf;
    std::queue<uint8_t>  rxQueue;

    void rxQueuePushData(const void *_p, size_t size);

    // Overlay
    uint8_t  ovlFont[2048];
    uint16_t ovlPalette[32];
    uint16_t ovlText[1024];

    void renderOverlay(void *pixels, int pitch);
};

std::shared_ptr<EmuState> newAqpEmuState();
std::shared_ptr<EmuState> newAqmsEmuState();
std::shared_ptr<EmuState> newAq32EmuState();
std::shared_ptr<EmuState> newAqua8EmuState();

static inline uint32_t col12_to_col32(uint16_t col444) {
    unsigned r4 = (col444 >> 8) & 0xF;
    unsigned g4 = (col444 >> 4) & 0xF;
    unsigned b4 = (col444 >> 0) & 0xF;

    unsigned r8 = (r4 << 4) | r4;
    unsigned g8 = (g4 << 4) | g4;
    unsigned b8 = (b4 << 4) | b4;

    return (0xFF << 24) | (b8 << 16) | (g8 << 8) | (r8);
}
