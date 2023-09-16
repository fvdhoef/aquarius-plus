#pragma once

#include "Common.h"

#define WIFI_MIN_RSSI (-100)
#define WIFI_MAX_RSSI (-55)

enum class EWiFiStatus {
    disconnected,
    disconnectedNoConfig,
    connecting,
    connected,
    authError,
    apNotFound,
};

class WiFi {
    WiFi();

public:
    static WiFi &instance();

    void        init();
    std::string getStatusStr();
    EWiFiStatus getStatus();

private:
    SemaphoreHandle_t mutex;

    void        doConnect();
    static void _eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData);
    void        eventHandler(esp_event_base_t eventBase, int32_t eventId, void *eventData);

    EWiFiStatus status;

    bool sntpStarted = false;
};
