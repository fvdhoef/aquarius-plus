#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include <sys/stat.h>

#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_event.h>
#include <esp_timer.h>
#include <esp_mac.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

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

// trim from left
static inline std::string ltrim(const std::string &s, const char *t = " \t\n\r\f\v") {
    std::string result = s;
    result.erase(0, result.find_first_not_of(t));
    return result;
}

// trim from right
static inline std::string rtrim(const std::string &s, const char *t = " \t\n\r\f\v") {
    std::string result = s;
    result.erase(result.find_last_not_of(t) + 1);
    return result;
}

// trim from left & right
static inline std::string trim(const std::string &s, const char *t = " \t\n\r\f\v") {
    return ltrim(rtrim(s, t), t);
}

void splitPath(const std::string &path, std::vector<std::string> &result);
bool startsWith(const std::string &s1, const std::string &s2, bool caseSensitive = false);
bool createPath(const std::string &path);
