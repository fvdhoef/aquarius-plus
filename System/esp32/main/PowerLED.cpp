#include "PowerLED.h"

PowerLED::PowerLED() {
    th       = nullptr;
    stop     = false;
    ledLevel = false;
}

PowerLED &PowerLED::instance() {
    static PowerLED obj;
    return obj;
}

void PowerLED::init() {
    mutex = xSemaphoreCreateRecursiveMutex();
    RecursiveMutexLock lock(mutex);

    // Configure power LED IO
    gpio_config_t io_conf = {.pin_bit_mask = (1ULL << IOPIN_LED), .mode = GPIO_MODE_OUTPUT};
    gpio_config(&io_conf);

    // Enable LED
    gpio_set_level((gpio_num_t)IOPIN_LED, 1);
}

void PowerLED::_timerCb(TimerHandle_t xTimer) {
    static_cast<PowerLED *>(pvTimerGetTimerID(xTimer))->timerCb();
}

void PowerLED::timerCb() {
    RecursiveMutexLock lock(mutex);
    ledLevel = !ledLevel;

    if (stop) {
        ledLevel = true;
        xTimerStop(th, portMAX_DELAY);
    }
    gpio_set_level((gpio_num_t)IOPIN_LED, ledLevel);
}

void PowerLED::flashStart() {
    RecursiveMutexLock lock(mutex);
    if (!th) {
        th = xTimerCreate("PowerLED", pdMS_TO_TICKS(20), 1, this, _timerCb);
    }
    if (stop) {
        stop = false;
    }
    if (!xTimerIsTimerActive(th)) {
        ledLevel = false;
        gpio_set_level((gpio_num_t)IOPIN_LED, ledLevel);
        xTimerReset(th, portMAX_DELAY);
    }
}

void PowerLED::flashStop() {
    RecursiveMutexLock lock(mutex);
    stop = true;
}
