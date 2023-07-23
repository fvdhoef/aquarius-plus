#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_event.h>
#include <esp_timer.h>
#include <esp_mac.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#include <driver/gpio.h>

#include "iopins.h"

class RecursiveMutexLock {
public:
    RecursiveMutexLock(SemaphoreHandle_t _mutex)
        : mutex(_mutex) {
        xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    }
    ~RecursiveMutexLock() {
        xSemaphoreGiveRecursive(mutex);
    }

private:
    SemaphoreHandle_t mutex;
};
