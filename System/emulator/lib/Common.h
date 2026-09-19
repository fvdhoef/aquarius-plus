#pragma once

#if _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdalign.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <fstream>
#include <map>
#include <queue>
#include <deque>
#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <pwd.h>
#include <uuid/uuid.h>
#endif

#if _WIN32
#include "getopt.h"
#include <direct.h>
#include <io.h>
#define strdup      _strdup
#define unlink      _unlink
#define rmdir       _rmdir
#define lseek       _lseek
#define mkdir       _mkdir
#define strncasecmp _strnicmp
#define strcasecmp  _stricmp
#else
#include <unistd.h>
#include <pwd.h>
#endif

#define CPU_FREQ (3579545)

static inline void stripTrailingSlashes(std::string &path) {
    // Leave a slash at the start of the string intact
    while (path.size() > 2 && (path.back() == '/' || path.back() == '\\'))
        path.pop_back();
}

#ifndef _WIN32
static inline std::string fmtstr(const char *fmt, ...) __attribute__((__format__(__printf__, 1, 2)));
#endif

static inline std::string fmtstr(const char *fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    int length = vsnprintf(nullptr, 0, fmt, vl);
    va_end(vl);

    std::string result;
    result.reserve(length + 1);
    result.resize(length);

    va_start(vl, fmt);
    vsnprintf(&result[0], length + 1, fmt, vl);
    va_end(vl);

    return result;
}

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

#define CONFIG_MACHINE_TYPE_AQPLUS

#include "FreeRtosMock.h"

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

void esp_restart();

#define CONFIG_BYPASS_START_TIME_MS 3000
