#include "WiFi.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_sntp.h>
#include <esp_tls.h>

static const char *TAG = "wifi";

WiFi::WiFi() {
    status = EWiFiStatus::disconnected;
}

WiFi &WiFi::instance() {
    static WiFi obj;
    return obj;
}

void WiFi::init() {
    mutex = xSemaphoreCreateRecursiveMutex();
    RecursiveMutexLock lock(mutex);

    // Initialize CA store
    extern const uint8_t certificate_start[] asm("_binary_letsencrypt_root_certificate_pem_start");
    extern const uint8_t certificate_end[] asm("_binary_letsencrypt_root_certificate_pem_end");
    ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store(certificate_start, certificate_end - certificate_start));

    // Initialize TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());

    // Register our event handler for Wi-Fi, IP and Provisioning related events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_eventHandler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_eventHandler, this));

    // Initialize Wi-Fi including netif with default config
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Start Wi-Fi station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Start SNTP client for time sync
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    // esp_sntp_set_time_sync_notification_cb();
    esp_sntp_init();
}

EWiFiStatus WiFi::getStatus() {
    RecursiveMutexLock lock(mutex);
    return status;
}

std::string WiFi::getStatusStr() {
    RecursiveMutexLock lock(mutex);
    switch (status) {
        default:
        case EWiFiStatus::disconnected: return "Disconnected";
        case EWiFiStatus::disconnectedNoConfig: return "Disconnected (no config)";
        case EWiFiStatus::connecting: return "Connecting";
        case EWiFiStatus::connected: return "Connected";
        case EWiFiStatus::authError: return "Auth error";
        case EWiFiStatus::apNotFound: return "AP not found";
    }
}

void WiFi::doConnect() {
    ESP_LOGI(TAG, "Connecting.");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK) {
        status = EWiFiStatus::connecting;
    } else if (err == ESP_ERR_WIFI_SSID) {
        status = EWiFiStatus::disconnectedNoConfig;
    } else {
        status = EWiFiStatus::disconnected;
    }
}

void WiFi::_eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    static_cast<WiFi *>(arg)->eventHandler(eventBase, eventId, eventData);
}

void WiFi::eventHandler(esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    RecursiveMutexLock lock(mutex);

    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        doConnect();

    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
        status                   = EWiFiStatus::connected;

        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));

    } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        status = EWiFiStatus::disconnected;
        ESP_LOGI(TAG, "Disconnected.");

        wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)eventData;
        switch (disconnected->reason) {
            case WIFI_REASON_AUTH_EXPIRE:
            case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            case WIFI_REASON_AUTH_FAIL:
            case WIFI_REASON_HANDSHAKE_TIMEOUT:
            case WIFI_REASON_MIC_FAILURE: status = EWiFiStatus::authError; break;
            case WIFI_REASON_NO_AP_FOUND: status = EWiFiStatus::apNotFound; break;
            default: doConnect(); break;
        }
    }
}
