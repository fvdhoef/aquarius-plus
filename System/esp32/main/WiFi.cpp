#include "WiFi.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_sntp.h>
#include <esp_tls.h>

static const char *TAG = "WiFi";

#define MAX_SCAN_AP (20)

#define MAX_KNOWN_APS (5)
struct KnownAP {
    char ssid[33];
    char password[64];
};

class WiFiInt : public WiFi {

    bool enabled   = false;
    bool connected = false;
    bool scanning  = false;

    bool joining     = false;
    bool sntpStarted = false;

    KnownAP                       knownAPs[MAX_KNOWN_APS];
    esp_netif_t                  *netIf;
    struct NetworkInfo            netInfo;
    SemaphoreHandle_t             mutex;
    std::vector<wifi_ap_record_t> scanResults;

public:
    WiFiInt() {
        mutex = xSemaphoreCreateRecursiveMutex();
    }

    void init() override {
        RecursiveMutexLock lock(mutex);

        // Initialize CA store
        extern const uint8_t certificatesStart[] asm("_binary_root_certificates_start");
        extern const uint8_t certificatesEnd[] asm("_binary_root_certificates_end");
        ESP_ERROR_CHECK(esp_tls_init_global_ca_store());

        auto ret = esp_tls_set_global_ca_store(certificatesStart, certificatesEnd - certificatesStart);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_tls_set_global_ca_store failed: %d", ret);
        }

        // Initialize TCP/IP
        ESP_ERROR_CHECK(esp_netif_init());

        // Register our event handler for Wi-Fi, IP and Provisioning related events
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_eventHandler, this));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_eventHandler, this));

        // Initialize Wi-Fi including netif with default config
        netIf = esp_netif_create_default_wifi_sta();

        // Set hostname
        nvs_handle_t h;
        if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
            char   hostname[32];
            size_t len = sizeof(hostname);
            if (nvs_get_str(h, "hostname", hostname, &len) == ESP_OK) {
                esp_netif_set_hostname(netIf, hostname);
            }
            nvs_close(h);
        }

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

        // Restore config from NVS
        loadEnabled();
        loadKnownAPs();

        // Start WiFi if previously enabled
        if (enabled) {
            esp_wifi_start();
        }
    }

    static void _eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
        static_cast<WiFiInt *>(arg)->eventHandler(eventBase, eventId, eventData);
    }

    void _join(const std::string &ssid, const std::string &password) {
        RecursiveMutexLock lock(mutex);
        printf("_join ssid '%s'\n", ssid.c_str());

        joining = true;
        disconnect();

        wifi_config_t wifiConfig = {
            .sta = {
                .scan_method = WIFI_ALL_CHANNEL_SCAN,
                .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
                .threshold   = {.rssi = -127},
                .pmf_cfg     = {.capable = true},
            },
        };
        snprintf((char *)wifiConfig.sta.ssid, sizeof(wifiConfig.sta.ssid), "%s", ssid.c_str());
        snprintf((char *)wifiConfig.sta.password, sizeof(wifiConfig.sta.password), "%s", password.c_str());

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
        ESP_ERROR_CHECK(esp_wifi_connect());

        joining = false;
    }

    void startScan() {
        RecursiveMutexLock lock(mutex);
        if (!enabled || scanning)
            return;

        if (esp_wifi_scan_start(nullptr, false) == ESP_OK) {
            ESP_LOGI(TAG, "Started scan");
            scanning = true;
        }
    }

    void updateScanResults() {
        RecursiveMutexLock lock(mutex);

        uint16_t numAP;
        if (esp_wifi_scan_get_ap_num(&numAP) != ESP_OK)
            return;

        std::vector<wifi_ap_record_t> apInfo;
        apInfo.resize(numAP);
        auto result = esp_wifi_scan_get_ap_records(&numAP, apInfo.data());
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error getting scan results: %s", esp_err_to_name(result));
            return;
        }

        // Remove doubles if present
        scanResults.clear();
        for (auto &info1 : apInfo) {
            bool isNew = true;

            for (auto &info2 : scanResults) {
                if (strcmp((const char *)info1.ssid, (const char *)info2.ssid) == 0) {
                    isNew = false;
                    break;
                }
            }

            if (isNew)
                scanResults.push_back(info1);
        }
    }

    void disconnect() {
        RecursiveMutexLock lock(mutex);
        if (connected) {
            esp_wifi_disconnect();
            connected = false;
        }
    }

    void updateNetInfo() {
        RecursiveMutexLock lock(mutex);

        netInfo.connected = connected;

        wifi_ap_record_t war;
        netInfo.currentNetwork.clear();
        if (connected && esp_wifi_sta_get_ap_info(&war) == ESP_OK) {
            setWiFiApInfo(netInfo.currentNetwork, war);
        }

        esp_read_mac(netInfo.mac, ESP_MAC_WIFI_STA);

        if (connected) {
            char str[32];

            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info) == ESP_OK) {
                snprintf(str, sizeof(str), IPSTR, IP2STR(&ip_info.ip));
                netInfo.ip = str;
                snprintf(str, sizeof(str), IPSTR, IP2STR(&ip_info.netmask));
                netInfo.netmask = str;
                snprintf(str, sizeof(str), IPSTR, IP2STR(&ip_info.gw));
                netInfo.gateway = str;
            }

            esp_netif_dns_info_t dns_info;
            if (esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
                if (dns_info.ip.u_addr.ip4.addr) {
                    snprintf(str, sizeof(str), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
                    netInfo.dns1 = str;
                }
            }
            if (esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
                if (dns_info.ip.u_addr.ip4.addr) {
                    snprintf(str, sizeof(str), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
                    netInfo.dns2 = str;
                }
            }
            if (esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_FALLBACK, &dns_info) == ESP_OK) {
                if (dns_info.ip.u_addr.ip4.addr) {
                    snprintf(str, sizeof(str), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
                    netInfo.dns3 = str;
                }
            }
        }

        // First add the list of known networks
        netInfo.knownNetworks.clear();
        for (int i = 0; i < MAX_KNOWN_APS; i++) {
            if (knownAPs[i].ssid[0] == '\0') {
                // End of list reached
                break;
            }

            // Skip currently connected network
            if (strcmp(netInfo.currentNetwork.ssid.c_str(), knownAPs[i].ssid) == 0)
                continue;

            auto &item = netInfo.knownNetworks.emplace_back();
            item.ssid  = knownAPs[i].ssid;
        }

        // Fill other networks lists and update known networks info
        netInfo.otherNetworks.clear();

        for (auto &result : scanResults) {
            // Skip currently connected network
            if (strcmp(netInfo.currentNetwork.ssid.c_str(), (const char *)result.ssid) == 0)
                continue;

            // Check if this is a known AP
            bool known = false;
            for (auto &info : netInfo.knownNetworks) {
                if (strcmp(info.ssid.c_str(), (const char *)result.ssid) == 0) {
                    // Update with additional info
                    known = true;
                    setWiFiApInfo(info, result);
                    break;
                }
            }
            if (known)
                continue;

            setWiFiApInfo(netInfo.otherNetworks.emplace_back(), result);
        }
    }

    void autoConnect(const std::vector<wifi_ap_record_t> &scanResults) {
        if (connected)
            return;

        for (auto &result : scanResults) {
            for (int i = 0; i < MAX_KNOWN_APS; i++) {
                if (knownAPs[i].ssid[0] == 0)
                    break;

                if (strcmp(knownAPs[i].ssid, (const char *)result.ssid) == 0) {
                    // Found known network, connect to it
                    _join(knownAPs[i].ssid, knownAPs[i].password);
                    return;
                }
            }
        }
    }

    void eventHandler(esp_event_base_t eventBase, int32_t eventId, void *eventData) {
        if (eventBase == WIFI_EVENT) {
            switch (eventId) {
                case WIFI_EVENT_STA_START: {
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
                    startScan();
                    break;
                }

                case WIFI_EVENT_STA_STOP: {
                    RecursiveMutexLock lock(mutex);
                    scanning  = false;
                    connected = false;
                    scanResults.clear();
                    updateNetInfo();
                    break;
                }

                case WIFI_EVENT_SCAN_DONE: {
                    ESP_LOGI(TAG, "WIFI_EVENT_SCAN_DONE");

                    RecursiveMutexLock lock(mutex);
                    scanning = false;
                    updateScanResults();
                    updateNetInfo();

                    // Auto-connect if not connected yet
                    autoConnect(scanResults);

                    break;
                }

                case WIFI_EVENT_STA_CONNECTED: {
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
                    RecursiveMutexLock lock(mutex);
                    connected = true;
                    updateNetInfo();
                    break;
                }
                case WIFI_EVENT_STA_DISCONNECTED: {
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
                    RecursiveMutexLock lock(mutex);
                    connected = false;

                    if (!joining)
                        startScan();
                    break;
                }
                default: ESP_LOGI(TAG, "WIFI event: %d", (int)eventId); break;
            }

        } else if (eventBase == IP_EVENT) {
            switch (eventId) {
                case IP_EVENT_STA_GOT_IP: {
                    ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
                    updateNetInfo();

                    if (!sntpStarted) {
                        // Start SNTP client for time sync
                        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
                        esp_sntp_setservername(0, "pool.ntp.org");
                        // esp_sntp_set_time_sync_notification_cb();
                        esp_sntp_init();
                        sntpStarted = true;
                    }

                    break;
                }

                default: ESP_LOGI(TAG, "IP event: %d", (int)eventId); break;
            }
        }
    }

    bool getEnabled() {
        RecursiveMutexLock lock(mutex);

        return enabled;
    }

    void setEnabled(bool enable) {
        RecursiveMutexLock lock(mutex);

        if (enable == enabled)
            return;
        enabled = enable;

        saveEnabled();

        if (enabled) {
            esp_wifi_start();
        } else {
            esp_wifi_stop();
        }
    }

    void loadEnabled() {
        RecursiveMutexLock lock(mutex);
        enabled = false;

        nvs_handle_t h;
        if (nvs_open("wifi_settings", NVS_READONLY, &h) == ESP_OK) {
            uint8_t val;
            if (nvs_get_u8(h, "enabled", &val) == ESP_OK) {
                enabled = val != 0;
            }
            nvs_close(h);
        }
    }

    void saveEnabled() {
        RecursiveMutexLock lock(mutex);

        nvs_handle_t h;
        if (nvs_open("wifi_settings", NVS_READWRITE, &h) == ESP_OK) {
            if (nvs_set_u8(h, "enabled", enabled ? 1 : 0) == ESP_OK) {
                nvs_commit(h);
            }
            nvs_close(h);
        }
    }

    std::string getHostName() override {
        RecursiveMutexLock lock(mutex);

        const char *hostname;
        if (esp_netif_get_hostname(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &hostname) == ESP_OK) {
            return hostname;
        }
        return "";
    }

    void setHostName(const std::string &name) override {
    }

    void loadKnownAPs() {
        RecursiveMutexLock lock(mutex);
        memset(knownAPs, 0, sizeof(knownAPs));

        nvs_handle_t h;
        if (nvs_open("wifi_settings", NVS_READONLY, &h) == ESP_OK) {
            size_t len = sizeof(knownAPs);
            if (nvs_get_blob(h, "known_aps", knownAPs, &len) != ESP_OK || len != sizeof(knownAPs)) {
                memset(knownAPs, 0, sizeof(knownAPs));
            }
            nvs_close(h);
        }
    }

    void saveKnownAPs() {
        RecursiveMutexLock lock(mutex);

        nvs_handle_t h;
        if (nvs_open("wifi_settings", NVS_READWRITE, &h) == ESP_OK) {
            if (nvs_set_blob(h, "known_aps", knownAPs, sizeof(knownAPs)) == ESP_OK) {
                nvs_commit(h);
            }
            nvs_close(h);
        }
    }

    void setWiFiApInfo(WiFiApInfo &wai, wifi_ap_record_t &war) {
        wai.ssid = (const char *)war.ssid;
        wai.rssi = war.rssi;

        switch (war.authmode) {
            case WIFI_AUTH_OPEN:
                wai.authMode = 'O';
                break;

            case WIFI_AUTH_WEP:
            case WIFI_AUTH_WPA_PSK:
            case WIFI_AUTH_OWE:
                wai.authMode = '1';
                break;

            case WIFI_AUTH_WPA2_PSK:
            case WIFI_AUTH_WPA_WPA2_PSK:
            case WIFI_AUTH_ENTERPRISE:
            case WIFI_AUTH_WAPI_PSK:
                wai.authMode = '2';
                break;

            case WIFI_AUTH_WPA3_PSK:
            case WIFI_AUTH_WPA2_WPA3_PSK:
            case WIFI_AUTH_WPA3_ENT_192:
            case WIFI_AUTH_WPA3_EXT_PSK:
            case WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE:
            case WIFI_AUTH_DPP:
                wai.authMode = '3';
                break;

            default: wai.authMode = '?'; break;
        }
    }

    void updateInfo() {
        RecursiveMutexLock lock(mutex);
        startScan();
        updateNetInfo();
    }

    NetworkInfo getNetworkInfo() override {
        RecursiveMutexLock lock(mutex);
        return netInfo;
    }

    void joinNetwork(const std::string &ssid, const std::string &password) override {
        RecursiveMutexLock lock(mutex);
        printf("joinNetwork ssid '%s' password '%s'\n", ssid.c_str(), password.c_str());

        if (ssid.size() > sizeof(knownAPs[0].ssid) - 1 || password.size() > sizeof(knownAPs[0].password) - 1) {
            ESP_LOGE(TAG, "joinNetwork - Provided ssid/password too long");
            return;
        }

        // Check if this is a already known AP
        int freeIdx = -1;
        for (int i = 0; i < MAX_KNOWN_APS; i++) {
            if (knownAPs[i].ssid[0] == '\0') {
                // End of list reached
                freeIdx = i;
                break;
            }

            if (strcmp(knownAPs[i].ssid, ssid.c_str()) == 0) {
                // Join known network, ignore password
                _join(knownAPs[i].ssid, knownAPs[i].password);
                return;
            }
        }

        if (freeIdx < 0) {
            // Overwrite last entry in list
            freeIdx = MAX_KNOWN_APS - 1;
        }

        strcpy(knownAPs[freeIdx].ssid, ssid.c_str());
        strcpy(knownAPs[freeIdx].password, password.c_str());
        saveKnownAPs();

        _join(ssid, password);

        updateNetInfo();
    }

    void forgetNetwork(const std::string &ssid) override {
        RecursiveMutexLock lock(mutex);
        if (connected && netInfo.currentNetwork.ssid == ssid) {
            disconnect();
        }

        int idx = -1;
        for (int i = 0; i < MAX_KNOWN_APS; i++) {
            if (strcmp(knownAPs[i].ssid, ssid.c_str()) == 0) {
                idx = i;
                break;
            }
        }
        if (idx < 0) {
            // Network not found
            return;
        }

        while (idx < MAX_KNOWN_APS - 1) {
            knownAPs[idx] = knownAPs[idx + 1];
            idx++;
        }
        memset(&knownAPs[idx], 0, sizeof(knownAPs[idx]));
        saveKnownAPs();

        updateNetInfo();
    }
};

WiFi *getWiFi() {
    static WiFiInt obj;
    return &obj;
}
