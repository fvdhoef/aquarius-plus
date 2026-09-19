#include "Aq32Pcm.h"

Aq32Pcm::Aq32Pcm() {
    reset();
}

void Aq32Pcm::reset() {
    fifoIrqThreshold = 0;
    rate             = 0;
    phaseAccum       = 0;
    data.clear();
    lastData = 0;
}

void Aq32Pcm::render(int16_t results[2]) {
    phaseAccum += rate * 571;
    bool nextSample = (phaseAccum >> 20) != 0;
    phaseAccum &= 0xFFFFF;

    if (nextSample && !data.empty()) {
        lastData = data.front();
        data.pop_front();
    }

    results[0] = data.empty() ? 0 : (lastData & 0xFFFF);
    results[1] = data.empty() ? 0 : ((lastData >> 16) & 0xFFFF);
}
