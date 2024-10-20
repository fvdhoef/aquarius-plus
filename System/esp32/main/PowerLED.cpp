#include "PowerLED.h"

class PowerLEDInt : public PowerLED {
public:
    SemaphoreHandle_t mutex;
    TimerHandle_t     th;
    volatile bool     stop;
    volatile bool     ledLevel;

    PowerLEDInt() {
        th       = nullptr;
        stop     = false;
        ledLevel = false;
    }

    void init() override {
        mutex = xSemaphoreCreateRecursiveMutex();
        RecursiveMutexLock lock(mutex);

        // Configure power LED IO
        gpio_config_t io_conf = {.pin_bit_mask = (1ULL << IOPIN_LED), .mode = GPIO_MODE_OUTPUT};
        gpio_config(&io_conf);

        // Enable LED
        gpio_set_level(IOPIN_LED, 1);
    }

    static void _timerCb(TimerHandle_t xTimer) {
        static_cast<PowerLEDInt *>(pvTimerGetTimerID(xTimer))->timerCb();
    }

    void timerCb() {
        RecursiveMutexLock lock(mutex);
        ledLevel = !ledLevel;

        if (stop) {
            ledLevel = true;
            xTimerStop(th, portMAX_DELAY);
        }
        gpio_set_level(IOPIN_LED, ledLevel);
    }

    void flashStart() override {
        RecursiveMutexLock lock(mutex);
        if (!th) {
            th = xTimerCreate("PowerLED", pdMS_TO_TICKS(20), 1, this, _timerCb);
        }
        if (stop) {
            stop = false;
        }
        if (!xTimerIsTimerActive(th)) {
            ledLevel = false;
            gpio_set_level(IOPIN_LED, ledLevel);
            xTimerReset(th, portMAX_DELAY);
        }
    }

    void flashStop() override {
        RecursiveMutexLock lock(mutex);
        stop = true;
    }
};

PowerLED *getPowerLED() {
    static PowerLEDInt obj;
    return &obj;
}
