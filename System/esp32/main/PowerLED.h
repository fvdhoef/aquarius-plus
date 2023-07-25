#pragma once

#include "Common.h"

class PowerLED {
    PowerLED();

public:
    static PowerLED &instance();

    void init();
    void flashStart();
    void flashStop();

private:
    static void _timerCb(TimerHandle_t xTimer);
    void        timerCb();

    SemaphoreHandle_t mutex;
    TimerHandle_t     th;
    volatile bool     stop;
    volatile bool     ledLevel;
};
