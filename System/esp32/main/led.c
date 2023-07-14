#include "led.h"

static TimerHandle_t th;
static volatile bool stop      = false;
static volatile bool led_level = false;

static void timercb(TimerHandle_t xTimer) {
    led_level = !led_level;

    if (stop) {
        led_level = true;
        xTimerStop(th, portMAX_DELAY);
    }
    gpio_set_level(IOPIN_LED, led_level);
}

void led_flash_start(void) {
    if (!th) {
        th = xTimerCreate("led", pdMS_TO_TICKS(20), 1, NULL, timercb);
    }
    if (stop) {
        stop = false;
    }
    if (!xTimerIsTimerActive(th)) {
        led_level = false;
        gpio_set_level(IOPIN_LED, led_level);
        xTimerReset(th, portMAX_DELAY);
    }
}

void led_flash_stop(void) {
    stop = true;
}
