#include "wifi.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_sntp.h>

static const char *TAG = "wifi";

static volatile bool connected  = false;
static const char   *status_str = "Disconnected";

static void do_connect(void) {
    ESP_LOGI(TAG, "Connecting.");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK) {
        status_str = "Connecting";
    } else if (err == ESP_ERR_WIFI_SSID) {
        status_str = "Disconnected (no config)";
    } else {
        status_str = "Disconnected";
    }
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        connected = false;
        do_connect();

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        status_str               = "Connected";

        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        connected = true;

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connected = false;
        ESP_LOGI(TAG, "Disconnected.");

        wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;
        switch (disconnected->reason) {
            case WIFI_REASON_AUTH_EXPIRE:
            case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            case WIFI_REASON_AUTH_FAIL:
            case WIFI_REASON_HANDSHAKE_TIMEOUT:
            case WIFI_REASON_MIC_FAILURE: status_str = "Auth error"; break;
            case WIFI_REASON_NO_AP_FOUND: status_str = "AP not found"; break;
            default: do_connect(); break;
        }
    }
}

void wifi_init(void) {
    // Initialize TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());

    // Register our event handler for Wi-Fi, IP and Provisioning related events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // Initialize Wi-Fi including netif with default config
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Start Wi-Fi station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    // sntp_set_time_sync_notification_cb();
    sntp_init();
}

bool wifi_is_connected(void) {
    return connected;
}

const char *wifi_get_status_str(void) {
    return status_str;
}
