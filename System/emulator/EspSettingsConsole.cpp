// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#include "EspSettingsConsole.h"
#ifndef EMULATOR
#    include <esp_ota_ops.h>
#    include <esp_app_format.h>
#    include <esp_wifi.h>
#    include "WiFi.h"
#    include "SDCardVFS.h"
#    include <esp_ota_ops.h>
#    include <esp_https_ota.h>
#    include <nvs_flash.h>

static const char *TAG = "settings";

#    define MAX_SCAN_AP (20)
#    define UPDATEFILE_NAME "aquarius-plus.bin"
#endif

EspSettingsConsole::EspSettingsConsole() {
}

EspSettingsConsole &EspSettingsConsole::instance() {
    static EspSettingsConsole obj;
    return obj;
}

void EspSettingsConsole::init() {
#ifndef EMULATOR
    tx_buffer = xStreamBufferCreate(256, 1);
    rx_buffer = xStreamBufferCreate(256, 1);
    xTaskCreate(_consoleTask, "console", 8192, this, 1, nullptr);
#else
    std::thread([&]() { consoleTask(); }).detach();
#endif
}

void EspSettingsConsole::newSession() {
    new_session = true;

    // Flush any data still in TX buffer
#ifndef EMULATOR
    for (int i = 0; i < 256; i++) {
        uint8_t data;
        if (xStreamBufferReceive(tx_buffer, &data, 1, 0) == 0)
            break;
    }
    {
        uint8_t data = 0;
        xStreamBufferSend(rx_buffer, &data, 1, portMAX_DELAY);
    }
#else
    while (!tx_buffer.empty())
        tx_buffer.pop();
    rx_buffer.push(0);
#endif
}

int EspSettingsConsole::recv(void *buf, size_t size) {
#ifndef EMULATOR
    return xStreamBufferReceive(tx_buffer, buf, size, 0);
#else
    int      count = 0;
    uint8_t *p     = (uint8_t *)buf;
    while (count < (int)size) {
        if (tx_buffer.empty())
            break;
        p[count++] = tx_buffer.pop();
    }
    return count;
#endif
}

int EspSettingsConsole::send(const void *buf, size_t size) {
#ifndef EMULATOR
    return xStreamBufferSend(rx_buffer, buf, size, 0);
#else
    const uint8_t *p = (const uint8_t *)buf;
    for (int i = 0; i < (int)size; i++) {
        rx_buffer.push(p[i]);
    }
    return (int)size;
#endif
}

#ifndef EMULATOR
void EspSettingsConsole::_consoleTask(void *obj) {
    static_cast<EspSettingsConsole *>(obj)->consoleTask();
    vTaskDelete(nullptr);
}
#endif

void EspSettingsConsole::consoleTask() {
    while (1) {
        if (new_session) {
            new_session = false;

#ifndef EMULATOR
            cprintf("\n");
            const esp_partition_t *running = esp_ota_get_running_partition();
            esp_app_desc_t         running_app_info;
            if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                cprintf("Name:%s\nVersion:%s\nCompile date:%s\nCompile time:%s\n", running_app_info.project_name, running_app_info.version, running_app_info.date, running_app_info.time);
            }
#else
            extern const char *versionStr;
            cprintf("Name:%s\nVersion:%s\nCompile date:%s\nCompile time:%s\n", "aquarius-plus-emulator", versionStr, __DATE__, __TIME__);
#endif
            cprintf("\nType HELP for more info\n");
        }

        char line[34];
        cprintf("ESP>");
        creadline(line, sizeof(line), false);
        if (strcasecmp(line, "help") == 0)
            showHelp();
        else if (strcasecmp(line, "wifi set") == 0)
            wifiSet();
        else if (strcasecmp(line, "wifi") == 0)
            wifiStatus();
        else if (strcasecmp(line, "date") == 0)
            showDate();
        else if (strcasecmp(line, "tz") == 0)
            timeZoneShow();
        else if (strcasecmp(line, "tz set") == 0)
            timeZoneSet();
        else if (strcasecmp(line, "update") == 0)
            systemUpdate();
        else if (strcasecmp(line, "updategh") == 0)
            systemUpdateGitHub();
        else if (line[0])
            cprintf("Unknown command\n");

        // if (line[0] != '\0') {
        //     cprintf("You said: %s\n", line);
        // }
    }
}

void EspSettingsConsole::cprintf(const char *fmt, ...) {
    char    tmp[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);

#ifndef EMULATOR
    xStreamBufferSend(tx_buffer, tmp, strlen(tmp), portMAX_DELAY);
#else
    for (int i = 0; i < (int)strlen(tmp); i++) {
        cputc(tmp[i]);
    }
#endif
}

void EspSettingsConsole::cputc(char ch) {
#ifndef EMULATOR
    xStreamBufferSend(tx_buffer, &ch, 1, portMAX_DELAY);
#else
    tx_buffer.push(ch);
#endif
}

char EspSettingsConsole::cgetc() {
#ifndef EMULATOR
    uint8_t val;
    xStreamBufferReceive(rx_buffer, &val, 1, portMAX_DELAY);
    return val;
#else
    return rx_buffer.pop();
#endif
}

void EspSettingsConsole::creadline(char *buf, size_t max_len, bool is_password) {
    char *p = buf;
    *p      = '\0';

    while (1) {
        char ch = cgetc();
        if (ch == 0) {
            *buf = '\0';
            return;
        }
        if (ch == '\b') {
            if (p > buf) {
                cputc('\b');
                cputc(' ');
                cputc('\b');
                p--;
                *p = '\0';
            }
            continue;
        }
        if (ch == '\n' || ch == '\r') {
            cputc('\r');
            cputc('\n');
            break;
        }
        if (ch == ' ' && p == buf) {
            continue;
        }
        if (p < buf + max_len - 1) {
            cputc(is_password ? '*' : ch);
            *(p++) = ch;
            *p     = '\0';
        }
    }
}

void EspSettingsConsole::showHelp() {
    cprintf("help    |this help\n");
    cprintf("wifi    |show WiFi status\n");
    cprintf("wifi set|set WiFi network\n");
    cprintf("date    |show current time/date\n");
    cprintf("tz      |show current time zone\n");
    cprintf("tz set  |set time zone\n");
    cprintf("update  |system update from SD card\n");
    cprintf("updategh|system update from GitHub\n");
    cprintf("ctrl-c  |exit to BASIC\n");
}

void EspSettingsConsole::wifiStatus() {
#ifndef EMULATOR
    WiFi &wifi       = WiFi::instance();
    auto  wifiStatus = wifi.getStatus();
    cprintf("WiFi status: %s\n", wifi.getStatusStr().c_str());

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    cprintf("MAC     :%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (wifiStatus == EWiFiStatus::connected) {
        wifi_ap_record_t war;
        if (esp_wifi_sta_get_ap_info(&war) == ESP_OK) {
            cprintf("SSID    :%s\n", war.ssid);
            cprintf("Channel :%u\n", war.primary);
            cprintf("RSSI    :%d dBm\n", war.rssi);
        }

        const char *hostname;
        if (esp_netif_get_hostname(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &hostname) == ESP_OK) {
            if (hostname && *hostname)
                cprintf("Hostname:%s\n", hostname);
        }
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info) == ESP_OK) {
            cprintf("IP      :" IPSTR "\n", IP2STR(&ip_info.ip));
            cprintf("Netmask :" IPSTR "\n", IP2STR(&ip_info.netmask));
            cprintf("Gateway :" IPSTR "\n", IP2STR(&ip_info.gw));
        }

        esp_netif_dns_info_t dns_info;
        if (esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
            cprintf("DNS     :" IPSTR "\n", IP2STR(&dns_info.ip.u_addr.ip4));
        }
        if (esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
            if (dns_info.ip.u_addr.ip4.addr)
                cprintf("DNS     :" IPSTR "\n", IP2STR(&dns_info.ip.u_addr.ip4));
        }
        if (esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_FALLBACK, &dns_info) == ESP_OK) {
            if (dns_info.ip.u_addr.ip4.addr)
                cprintf("DNS     :" IPSTR "\n", IP2STR(&dns_info.ip.u_addr.ip4));
        }
    }
#else
    cprintf("Not available on emulator\n");
#endif
}

void EspSettingsConsole::wifiSet() {
#ifndef EMULATOR
    cprintf("Scanning networks\n");

    uint16_t         ap_count = MAX_SCAN_AP;
    wifi_ap_record_t ap_info[MAX_SCAN_AP];
    unsigned         idxs[MAX_SCAN_AP];
    unsigned         idxs_count = 0;

    esp_err_t result = esp_wifi_scan_start(nullptr, true);
    if (result == ESP_OK) {
        memset(ap_info, 0, sizeof(ap_info));
        result = esp_wifi_scan_get_ap_records(&ap_count, ap_info);
    }
    if (result != ESP_OK) {
        ESP_LOGE("wifi", "Error during scan: %s", esp_err_to_name(result));
        cprintf("Error during scan!\n");
        return;
    }
    if (ap_count == 0) {
        cprintf("No APs found!\n");
        return;
    }

    // Remove doubles using an indirect index table
    for (int i = 0; i < ap_count; i++) {
        bool is_new = true;

        for (int j = 0; j < idxs_count; j++) {
            if (strcmp((const char *)ap_info[i].ssid, (const char *)ap_info[idxs[j]].ssid) == 0) {
                is_new = false;
                break;
            }
        }

        if (is_new) {
            idxs[idxs_count++] = i;
        }
    }

    for (int i = 0; i < idxs_count; i++) {
        unsigned ap_idx = idxs[i];
        cprintf("%d) %-23s (RSSI:%d)\n", i, ap_info[ap_idx].ssid, ap_info[ap_idx].rssi);
    }

    cprintf("Select network:");

    char str[4];
    creadline(str, sizeof(str), false);
    if (new_session)
        return;

    char    *endp;
    unsigned idx = strtoul(str, &endp, 10);
    if (*str == '\0' || *endp != '\0' || idx > idxs_count) {
        cprintf("Invalid entry, aborting.\n");
        return;
    }
    idx = idxs[idx];

    cprintf("Selected:%s\n", ap_info[idx].ssid);
    char password[65] = {0};
    if (ap_info[idx].authmode != WIFI_AUTH_OPEN) {
        cprintf("Password:");
        creadline(password, sizeof(password), true);
        if (new_session)
            return;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold   = {.rssi = -127},
            .pmf_cfg     = {.capable = true},
        },
    };
    snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), (const char *)ap_info[idx].ssid);
    snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), password);

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
#else
    cprintf("Not available on emulator\n");
#endif
}

void EspSettingsConsole::showDate() {
    time_t now;
    time(&now);
    struct tm timeinfo = *localtime(&now);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S (%Z)", &timeinfo);
    cprintf("%s\n", strftime_buf);
}

void EspSettingsConsole::systemUpdate() {
#ifndef EMULATOR
    const int              app_desc_offset  = sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t);
    size_t                 update_size      = 0;
    const esp_partition_t *update_partition = nullptr;
    esp_err_t              err;

    esp_ota_handle_t ota_handle  = 0;
    int              fd          = -1;
    const size_t     tmpbuf_size = 65536;
    void            *tmpbuf      = malloc(tmpbuf_size);
    bool             success     = false;
    char             str[4];
    auto            &vfs = SDCardVFS::instance();

    if (tmpbuf == nullptr) {
        cprintf("Out of memory\n");
        goto done;
    }

    struct stat st;
    if (vfs.stat(UPDATEFILE_NAME, &st) < 0) {
        cprintf("File not found (%s)\n", UPDATEFILE_NAME);
        goto done;
    }

    fd = vfs.open(FO_RDONLY, UPDATEFILE_NAME);
    if (fd < 0) {
        cprintf("File not found (%s)\n", UPDATEFILE_NAME);
        goto done;
    }

    update_size = st.st_size;
    cprintf("Update file size: %u\n", update_size);

    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(esp_ota_get_running_partition(), &running_app_info) != ESP_OK) {
        goto done;
    }

    esp_app_desc_t app_info;

    if (vfs.seek(fd, app_desc_offset) < 0)
        goto done;
    if (vfs.read(fd, sizeof(app_info), &app_info) != sizeof(app_info))
        goto done;

    if (app_info.magic_word != ESP_APP_DESC_MAGIC_WORD) {
        ESP_LOGE("update", "Incorrect app descriptor magic");
        cprintf("Invalid update file\n");
        goto done;
    }

    if (strcmp(running_app_info.project_name, app_info.project_name) != 0) {
        ESP_LOGE("update", "Project name does not match");
        cprintf("Invalid update file\n");
        goto done;
    }

    cprintf("Running:%s\n", running_app_info.version);
    cprintf("Update :%s\nUpdate :%s %s\n", app_info.version, app_info.date, app_info.time);
    cprintf("Do you want to continue?\nType yes to confirm\n");
    creadline(str, sizeof(str), false);
    if (strcmp(str, "yes") != 0) {
        cprintf("Aborting update\n");
        success = true;
        goto done;
    }

    if ((update_partition = esp_ota_get_next_update_partition(nullptr)) == nullptr) {
        cprintf("Error: can't find update partition\n");
        return;
    }

    cprintf("Initiating update.\n");
    err = esp_ota_begin(update_partition, update_size, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE("update", "esp_ota_begin: %s", esp_err_to_name(err));
        goto done;
    }

    cprintf("Writing:");
    vfs.seek(fd, 0);

    while (1) {
        int size = vfs.read(fd, tmpbuf_size, tmpbuf);
        if (size <= 0)
            break;

        cprintf(".");
        if ((err = esp_ota_write(ota_handle, tmpbuf, size)) != ESP_OK) {
            ESP_LOGE("update", "esp_ota_write: %s", esp_err_to_name(err));
            goto done;
        }
    }

    err        = esp_ota_end(ota_handle);
    ota_handle = 0;
    if (err != ESP_OK) {
        ESP_LOGE("update", "esp_ota_end: %s", esp_err_to_name(err));
        goto done;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE("update", "esp_ota_set_boot_partition: %s", esp_err_to_name(err));
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            cprintf("Invalid update\n");
        }
        goto done;
    }

    success = true;
    cprintf("Done.\nPress enter to reboot.\n");
    creadline(str, sizeof(str), false);
    esp_restart();

done:
    if (fd >= 0)
        vfs.close(fd);

    if (!success)
        cprintf("Error during update, aborting.\n");
    if (ota_handle)
        esp_ota_abort(ota_handle);
#else
    cprintf("Not available on emulator\n");
#endif
}

void EspSettingsConsole::systemUpdateGitHub() {
#ifndef EMULATOR
    cprintf("Enter firmware version (GitHub tag, e.g. V0.7):");
    char str[32];
    creadline(str, sizeof(str), false);
    if (new_session)
        return;

    auto url = std::string("https://github.com/fvdhoef/aquarius-plus/releases/download/") + str + "/aquarius-plus.bin";

    esp_http_client_config_t http_config = {
        .url                 = url.c_str(),
        .timeout_ms          = 5000,
        .buffer_size         = 64 * 1024,
        .buffer_size_tx      = 10 * 1024,
        .use_global_ca_store = true,
        .keep_alive_enable   = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_err_t              err;
    esp_https_ota_handle_t https_ota_handle = NULL;
    int                    lastPercentage   = -1;

    cprintf("Updating from: %s\n", url.c_str());

    err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        cprintf("Error starting OTA update\n");
        goto ota_end;
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        cprintf("Error getting image header\n");
        goto ota_end;
    }

    // Validate image header
    {
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_app_desc_t         running_app_info;
        if (esp_ota_get_partition_description(running, &running_app_info) != ESP_OK) {
            cprintf("Error!\n");
            goto ota_end;
        }
        if (strcmp(app_desc.project_name, running_app_info.project_name) != 0) {
            cprintf("Incompatible update, refusing update.\n");
            goto ota_end;
        }
        if (memcmp(app_desc.version, running_app_info.version, sizeof(app_desc.version)) == 0 &&
            memcmp(app_desc.time, running_app_info.time, sizeof(app_desc.time)) == 0 &&
            memcmp(app_desc.date, running_app_info.date, sizeof(app_desc.date)) == 0) {
            cprintf("Already running this firmware version. Aborting update.\n");
            goto ota_end;
        }
        cprintf("Running firmware version: %s\n", running_app_info.version);
        cprintf("New firmware version: %s\n", app_desc.version);
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        // esp_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
        // data read so far.
        auto progressSize = esp_https_ota_get_image_len_read(https_ota_handle);
        auto totalSize    = esp_https_ota_get_image_size(https_ota_handle);
        auto percentage   = (progressSize * 100) / totalSize;
        if (lastPercentage != percentage) {
            lastPercentage = percentage;
            cprintf("Updating: %d%%\n", percentage);
        }
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        cprintf("Data incomplete, aborting.");

    } else {
        esp_err_t ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if (err == ESP_OK && ota_finish_err == ESP_OK) {
            cprintf("Update successful.\nPress enter to reboot.\n");
            creadline(str, sizeof(str), false);
            esp_restart();

        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                cprintf("Image validation failed, image is corrupted.");
            }
            cprintf("Upgrade failed (0x%x)", ota_finish_err);
            return;
        }
    }
    return;

ota_end:
    if (https_ota_handle != NULL) {
        esp_https_ota_abort(https_ota_handle);
    }
    cprintf("Upgrade failed.\n");
    return;
#else
    cprintf("Not available on emulator\n");
#endif
}

bool EspSettingsConsole::creadUint(unsigned *value) {
    char str[4];
    creadline(str, sizeof(str), false);
    if (new_session)
        return false;
    char *endp;
    *value = strtoul(str, &endp, 10);
    if (*str == '\0' || *endp != '\0')
        return false;
    return true;
}

void EspSettingsConsole::timeZoneSet() {
    // cprintf(" 0:Enter custom timezone string\n");
    cprintf(" 1:North/South America\n");
    cprintf(" 2:Europe\n");
    cprintf(" 3:Africa\n");
    cprintf(" 4:Asia\n");
    cprintf(" 5:Australia/New Zealand\n");
    cprintf("Make a selection:");

    unsigned value;
    if (!creadUint(&value) || value > 5) {
        if (new_session)
            return;
        cprintf("Invalid entry, aborting.\n");
        return;
    }

    const char *tz = nullptr;

    switch (value) {
        case 1: {
            cprintf(" 1:SST        Samoa\n");
            cprintf(" 2:HST        Hawaii\n");
            cprintf(" 3:HST/HDT    Hawaii\n");
            cprintf(" 4:AKST/AKDT  Alaska\n");
            cprintf(" 5:PST/PDT    Pacific\n");
            cprintf(" 6:MST        Mountain\n");
            cprintf(" 7:MST/MDT    Mountain\n");
            cprintf(" 8:CST        Central\n");
            cprintf(" 9:CST/CDT    Central\n");
            cprintf("10:EST        Eastern\n");
            cprintf("11:EST/EDT    Eastern\n");
            cprintf("12:AST        Atlantic\n");
            cprintf("13:AST/ADT    Atlantic\n");
            cprintf("14:NST/NDT    Newfoundland\n");
            cprintf("Make a selection:");

            if (creadUint(&value)) {
                switch (value) {
                    case 1: tz = "SST11"; break;
                    case 2: tz = "HST10"; break;
                    case 3: tz = "HST10HDT,M3.2.0,M11.1.0"; break;
                    case 4: tz = "AKST9AKDT,M3.2.0,M11.1.0"; break;
                    case 5: tz = "PST8PDT,M3.2.0,M11.1.0"; break;
                    case 6: tz = "MST7"; break;
                    case 7: tz = "MST7MDT,M3.2.0,M11.1.0"; break;
                    case 8: tz = "CST6"; break;
                    case 9: tz = "CST6CDT,M3.2.0,M11.1.0"; break;
                    case 10: tz = "EST5"; break;
                    case 11: tz = "EST5EDT,M3.2.0,M11.1.0"; break;
                    case 12: tz = "AST4"; break;
                    case 13: tz = "AST4ADT,M3.2.0,M11.1.0"; break;
                    case 14: tz = "NST3:30NDT,M3.2.0,M11.1.0"; break;
                }
            }
            break;
        }

        case 2: {
            cprintf("1:GMT       Greenwich Mean\n");
            cprintf("2:GMT/BST   UK\n");
            cprintf("3:GMT/IST   Ireland\n");
            cprintf("4:WET/WEST  Portugal\n");
            cprintf("5:CET/CEST  Most of Western Europe\n");
            cprintf("6:EET       Eastern Europe\n");
            cprintf("7:EET/EEST  Eastern Europe\n");
            cprintf("8:MSK       Western Russia\n");
            cprintf("Make a selection:");

            if (creadUint(&value)) {
                switch (value) {
                    case 1: tz = "GMT0"; break;
                    case 2: tz = "GMT0BST,M3.5.0/1,M10.5.0"; break;
                    case 3: tz = "GMT0IST,M3.5.0/1,M10.5.0"; break;
                    case 4: tz = "WET0WEST,M3.5.0/1,M10.5.0"; break;
                    case 5: tz = "CET-1CEST,M3.5.0,M10.5.0/3"; break;
                    case 6: tz = "EET-2"; break;
                    case 7: tz = "EET-2EEST,M3.5.0/3,M10.5.0/4"; break;
                    case 8: tz = "MSK-3"; break;
                }
            }
            break;
        }

        case 3: {
            cprintf("1:GMT   Greenwich Mean\n");
            cprintf("2:WAT   West Africa\n");
            cprintf("3:SAST  South Africa\n");
            cprintf("4:CAST  Central Africa\n");
            cprintf("5:EAT   East Africa\n");
            cprintf("Make a selection:");

            if (creadUint(&value)) {
                switch (value) {
                    case 1: tz = "GMT0"; break;
                    case 2: tz = "WAT-1"; break;
                    case 3: tz = "SAST-2"; break;
                    case 4: tz = "CAT-2"; break;
                    case 5: tz = "EAT-3"; break;
                }
            }
            break;
        }

        case 4: {
            cprintf(" 1:PKT  Pakistan\n");
            cprintf(" 2:IST  India\n");
            cprintf(" 3:WIB  Western Indonesia\n");
            cprintf(" 4:WITA Central Indonesia\n");
            cprintf(" 5:WIT  Eastern Indonesia\n");
            cprintf(" 6:CST  China\n");
            cprintf(" 7:HKT  Hong Kong\n");
            cprintf(" 8:PST  Philippines\n");
            cprintf(" 9:JST  Japan\n");
            cprintf("10:KST  Korea\n");
            cprintf("Make a selection:");

            if (creadUint(&value)) {
                switch (value) {
                    case 1: tz = "PKT-5"; break;
                    case 2: tz = "IST-5:30"; break;
                    case 3: tz = "WIB-7"; break;
                    case 4: tz = "WITA-8"; break;
                    case 5: tz = "WIT-9"; break;
                    case 6: tz = "CST-8"; break;
                    case 7: tz = "HKT-8"; break;
                    case 8: tz = "PST-8"; break;
                    case 9: tz = "JST-9"; break;
                    case 10: tz = "KST-9"; break;
                }
            }
            break;
        }

        case 5: {
            cprintf(" 1:AWST         Western Australia\n");
            cprintf(" 2:ACST         Central Australia\n");
            cprintf(" 3:ACST/ACDT    Central Australia\n");
            cprintf(" 4:AEST         Eastern Australia\n");
            cprintf(" 5:AEST/AEDT    Eastern Australia\n");
            cprintf(" 6:NZST/NZDT    New Zealand\n");
            cprintf(" 7:CHAST/CHADT  Chatham Island\n");
            cprintf("Make a selection:");

            if (creadUint(&value)) {
                switch (value) {
                    case 1: tz = "AWST-8"; break;
                    case 2: tz = "ACST-9:30"; break;
                    case 3: tz = "ACST-9:30ACDT,M10.1.0,M4.1.0/3"; break;
                    case 4: tz = "AEST-10"; break;
                    case 5: tz = "AEST-10AEDT,M10.1.0,M4.1.0/3"; break;
                    case 6: tz = "NZST-12NZDT,M9.5.0,M4.1.0/3"; break;
                    case 7: tz = "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45"; break;
                }
            }
            break;
        }
    }

    if (new_session)
        return;
    if (tz == nullptr) {
        cprintf("Invalid entry, aborting.\n");
        return;
    }

#if EMULATOR
    cur_tz = tz;
#endif

#ifndef _WIN32
    setenv("TZ", tz, 1);
#endif

#ifndef EMULATOR
    // Save timezone to flash
    {
        nvs_handle_t h;
        if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
            if (nvs_set_str(h, "tz", tz) == ESP_OK) {
                nvs_commit(h);
            }
            nvs_close(h);
        }
    }
#endif

    cprintf("Setting time zone done.\n");
}

void EspSettingsConsole::timeZoneShow() {
#ifndef EMULATOR
    const char *cur_tz;
#endif

#ifndef _WIN32
    cur_tz = getenv("TZ");
    if (!cur_tz)
        cur_tz = "";
#endif

    const char *p = cur_tz;
    if (p[0] == 0) {
        cprintf("No timezone set.\n");
        return;
    }

    std::string tzName;
    std::string tzDstName;

    // Extract timezone
    while (isalpha(*p))
        tzName.push_back(*(p++));

    // Skip GMT offset
    while (isdigit(*p) || *p == ':' || *p == '-')
        p++;

    // Extract DST timezone if available
    while (isalpha(*p))
        tzDstName.push_back(*(p++));

    if (!tzDstName.empty()) {
        cprintf("Timezone:%s/%s\n", tzName.c_str(), tzDstName.c_str());
    } else {
        cprintf("Timezone:%s\n", tzName.c_str());
    }
}
