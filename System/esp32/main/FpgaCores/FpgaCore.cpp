#include "FpgaCore.h"
#include "FPGA.h"

#include "AquariusPlus/CoreAquariusPlus.h"

static const char *TAG = "FpgaCore";

static std::shared_ptr<FpgaCore> currentCore;

std::shared_ptr<FpgaCore> getFpgaCore() {
    return currentCore;
}

void unloadFpgaCore() {
    currentCore = nullptr;
}

std::shared_ptr<FpgaCore> loadFpgaCore(FpgaCoreType type, const void *data, size_t length) {
    unloadFpgaCore();

    if (type == FpgaCoreType::AquariusPlus) {
        currentCore = newCoreAquariusPlus();
    }
    if (!currentCore) {
        ESP_LOGE(TAG, "Error creating core handler");
        return nullptr;
    }
    if (!currentCore->loadBitstream(data, length)) {
        unloadFpgaCore();
        return nullptr;
    }
    return currentCore;
}
