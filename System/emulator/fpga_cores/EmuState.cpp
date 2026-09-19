#include "EmuState.h"
#include "FpgaCore.h"

static std::shared_ptr<EmuState> curEmuState;

std::shared_ptr<EmuState> EmuState::get() {
    return curEmuState;
}

void EmuState::loadCore(const std::string &_name) {
    if (curEmuState)
        curEmuState->unloading();
    curEmuState = nullptr;

    std::string name;

    auto colonPos = _name.find(':', 0);
    if (colonPos == std::string::npos) {
        name = _name;
    } else {
        name = _name.substr(colonPos + 1);
    }

    // Normalize passed name
    {
        std::vector<std::string> result;
        splitPath(name, result);
        if (result.empty())
            return;

        name = result.back();
        for (auto &ch : name) {
            ch = tolower(ch);
        }
    }

    if (name == "aqplus.core") {
        curEmuState = newAqpEmuState();
    } else if (name == "aqms.core") {
        curEmuState = newAqmsEmuState();
    } else if (name == "aq32.core") {
        curEmuState = newAq32EmuState();
    } else if (name == "aqua-8.core") {
        curEmuState = newAqua8EmuState();
    }
}

void EmuState::renderOverlay(void *pixels, int pitch) {
    int w, h;
    getVideoSize(w, h);
    if (w < 640 || h < 400)
        return;

    int xOffset = (w - 640) / 2;
    int yOffset = (h == 480) ? 32 : ((h - 400) / 2);

    for (int j = 0; j < 400; j++) {
        for (int i = 0; i < 640; i++) {
            int      row     = (j / 2) / 8;
            int      column  = (i / 2) / 8;
            unsigned addr    = (row * 40 + column) & 0x3FF;
            uint16_t chCol   = ovlText[addr];
            uint8_t  ch      = chCol & 0xFF;
            uint8_t  color   = chCol >> 8;
            uint8_t  charBm  = ovlFont[ch * 8 + ((j / 2) & 7)];
            uint8_t  colIdx  = (charBm & (1 << (7 - ((i / 2) & 7)))) ? (color >> 4) : (color & 0xF);
            uint16_t col4444 = ovlPalette[colIdx];

            if ((col4444 >> 12) >= 8) {
                ((uint32_t *)((uintptr_t)pixels + (j + yOffset) * pitch))[i + xOffset] = col12_to_col32(col4444);
            }
        }
    }
}

void EmuState::spiSel(bool enable) {
    if (spiSelected == enable)
        return;
    spiSelected = enable;
    txBuf.clear();
}

void EmuState::spiTx(const void *data, size_t length) {
    if (!spiSelected)
        return;

    auto p = static_cast<const uint8_t *>(data);
    txBuf.insert(txBuf.end(), p, p + length);
    if (txBuf.empty())
        return;

    switch (txBuf[0]) {
        case CMD_RESET: {
            if (txBuf.size() == 1 + 1) {
                if (txBuf[1] & 2) {
                    reset(true);
                } else {
                    reset(false);
                }
            }
            break;
        }

        case CMD_GET_SYSINFO: {
            if (txBuf.size() == 2) {
                rxQueue.push(coreType);
                rxQueue.push(coreFlags);
                rxQueue.push(coreVersionMajor);
                rxQueue.push(coreVersionMinor);
            }
            break;
        }
        case CMD_GET_NAME1: {
            if (txBuf.size() == 2) {
                rxQueuePushData(coreName, 8);
            }
            break;
        }
        case CMD_GET_NAME2: {
            if (txBuf.size() == 2) {
                rxQueuePushData(coreName + 8, 8);
            }
            break;
        }

        case CMD_OVL_TEXT: {
            if (txBuf.size() == 1 + 2048) {
                memcpy(ovlText, &txBuf[1], 2048);
            }
            break;
        }
        case CMD_OVL_PALETTE: {
            if (txBuf.size() == 1 + 32) {
                memcpy(ovlPalette, &txBuf[1], 32);
            }
            break;
        }
        case CMD_OVL_FONT: {
            if (txBuf.size() == 1 + 2048) {
                memcpy(ovlFont, &txBuf[1], 2048);
            }
            break;
        }
    }
}

void EmuState::spiRx(void *buf, size_t length) {
    if (!spiSelected)
        return;

    uint8_t *p = static_cast<uint8_t *>(buf);
    for (unsigned i = 0; i < length; i++) {
        uint8_t val = 0;
        if (!rxQueue.empty()) {
            val = rxQueue.front();
            rxQueue.pop();
        }
        *(p++) = val;
    }
}

void EmuState::rxQueuePushData(const void *_p, size_t size) {
    auto p = static_cast<const uint8_t *>(_p);
    while (size--) {
        rxQueue.push(*(p++));
    }
}

void esp_restart() {
    void loadStartupCore();
    loadStartupCore();
}
