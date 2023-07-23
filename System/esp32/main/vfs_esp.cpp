#include "vfs_esp.h"
#include "wifi.h"
#include <esp_ota_ops.h>
#include <esp_app_format.h>
#include <esp_wifi.h>
#include "sdcard.h"
#include "led.h"

extern const uint8_t settings_caq_start[] asm("_binary_settings_caq_start");
extern const uint8_t settings_caq_end[] asm("_binary_settings_caq_end");
static const char   *fn_settings = "settings.caq";
static const char   *fn_com      = "com";

static void console_task(void *pvParameters);

static StreamBufferHandle_t tx_buffer   = nullptr;
static StreamBufferHandle_t rx_buffer   = nullptr;
static volatile bool        new_session = true;

EspVFS::EspVFS() {
    dir_idx     = 0;
    file_offset = 0;
    file_idx    = 0;
}

EspVFS &EspVFS::instance() {
    static EspVFS vfs;
    return vfs;
}

void EspVFS::init(void) {
    tx_buffer = xStreamBufferCreate(256, 1);
    rx_buffer = xStreamBufferCreate(256, 1);
    xTaskCreate(console_task, "console", 8192, NULL, 1, NULL);
}

int EspVFS::open(uint8_t flags, const char *path) {
    // Skip leading slashes
    while (*path == '/')
        path++;

    printf("esp_open(%u, \"%s\")\n", flags, path);

    if (strcasecmp(path, fn_com) == 0) {
        new_session = true;

        // Flush any data still in TX buffer
        for (int i = 0; i < 256; i++) {
            uint8_t data;
            if (xStreamBufferReceive(tx_buffer, &data, 1, 0) == 0)
                break;
        }
        {
            uint8_t data = 0;
            xStreamBufferSend(rx_buffer, &data, 1, portMAX_DELAY);
        }
        return 1;
    }
    if (strcasecmp(path, fn_settings) == 0) {
        file_idx = 0;
    } else {
        file_idx = -1;
    }

    file_offset = 0;
    return file_idx < 0 ? ERR_NOT_FOUND : 0;
}

int EspVFS::read(int fd, uint16_t size, void *buf) {
    if (fd == 0) {
        int filesize  = settings_caq_end - settings_caq_start;
        int remaining = filesize - file_offset;

        if (size > remaining) {
            size = remaining;
        }
        memcpy(buf, settings_caq_start + file_offset, size);
        file_offset += size;
        return size;

    } else if (fd == 1) {
        int result = xStreamBufferReceive(tx_buffer, buf, size, 0);
        return result;

    } else {
        return ERR_OTHER;
    }
}

int EspVFS::write(int fd, uint16_t size, const void *buf) {
    if (fd == 1) {
        return xStreamBufferSend(rx_buffer, buf, size, 0);
    } else {
        return ERR_OTHER;
    }
}

int EspVFS::close(int fd) {
    return 0;
}

DirEnumCtx EspVFS::direnum(const char *path) {
    auto result = std::make_shared<std::vector<DirEnumEntry>>();
    result->emplace_back(fn_settings, settings_caq_end - settings_caq_start, 0, 0, 0);
    return result;
}

int EspVFS::stat(const char *path, struct stat *st) {
    if (strcasecmp(path, "") == 0) {
        memset(st, 0, sizeof(*st));
        st->st_mode = S_IFDIR;
        return 0;
    }
    return ERR_OTHER;
}

static void cprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

static void cprintf(const char *fmt, ...) {
    char    tmp[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    xStreamBufferSend(tx_buffer, tmp, strlen(tmp), portMAX_DELAY);
}

static void cputc(char ch) {
    xStreamBufferSend(tx_buffer, &ch, 1, portMAX_DELAY);
}

static char cgetc(void) {
    uint8_t val;
    xStreamBufferReceive(rx_buffer, &val, 1, portMAX_DELAY);
    return val;
}

static void creadline(char *buf, size_t max_len, bool is_password) {
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

static void show_help(void) {
    cprintf("help    |this help\n");
    cprintf("wifi    |show WiFi status\n");
    cprintf("wifi set|set WiFi network\n");
    cprintf("date    |show current time/date\n");
    cprintf("update  |system update from SD card\n");
    cprintf("ctrl-c  |exit to BASIC\n");
}

static void wifi_status(void) {
    bool connected = wifi_is_connected();
    cprintf("WiFi status: %s\n", wifi_get_status_str());

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    cprintf("MAC     :%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (connected) {
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
}

#define MAX_SCAN_AP (20)

static void wifi_set(void) {
    cprintf("Scanning networks\n");

    uint16_t         ap_count = MAX_SCAN_AP;
    wifi_ap_record_t ap_info[MAX_SCAN_AP];
    unsigned         idxs[MAX_SCAN_AP];
    unsigned         idxs_count = 0;

    esp_err_t result = esp_wifi_scan_start(NULL, true);
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
}

static void show_date(void) {
    time_t    now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S (%Z)", &timeinfo);
    cprintf("%s\n", strftime_buf);
}

#define UPDATEFILE_NAME "aquarius-plus.bin"

static void system_update(void) {
    const int              app_desc_offset  = sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t);
    size_t                 update_size      = 0;
    const esp_partition_t *update_partition = nullptr;
    esp_err_t              err;

    esp_ota_handle_t ota_handle  = 0;
    FILE            *f           = NULL;
    const size_t     tmpbuf_size = 65536;
    void            *tmpbuf      = malloc(tmpbuf_size);
    bool             success     = false;
    char             str[4];

    if (tmpbuf == NULL) {
        cprintf("Out of memory\n");
        goto done;
    }

    led_flash_start();
    if ((f = fopen(MOUNT_POINT "/" UPDATEFILE_NAME, "rb")) == NULL) {
        cprintf("File not found (%s)\n", UPDATEFILE_NAME);
        goto done;
    }
    led_flash_stop();

    led_flash_start();
    fseek(f, 0, SEEK_END);
    update_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    led_flash_stop();
    cprintf("Update file size: %u\n", update_size);

    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(esp_ota_get_running_partition(), &running_app_info) != ESP_OK) {
        goto done;
    }

    esp_app_desc_t app_info;

    led_flash_start();
    fseek(f, app_desc_offset, SEEK_SET);
    if (fread(&app_info, sizeof(app_info), 1, f) != 1) {
        goto done;
    }
    led_flash_stop();
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

    if ((update_partition = esp_ota_get_next_update_partition(NULL)) == NULL) {
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
    led_flash_start();
    fseek(f, 0, SEEK_SET);
    led_flash_stop();

    while (1) {
        led_flash_start();
        size_t size = fread(tmpbuf, 1, tmpbuf_size, f);
        led_flash_stop();
        if (size == 0)
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
    fclose(f);
    led_flash_stop();

    if (!success)
        cprintf("Error during update, aborting.\n");
    if (ota_handle)
        esp_ota_abort(ota_handle);
}

static void console_task(void *pvParameters) {
    while (1) {
        if (new_session) {
            new_session = false;

            cprintf("\n");
            const esp_partition_t *running = esp_ota_get_running_partition();
            esp_app_desc_t         running_app_info;
            if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                cprintf("Name:%s\nVersion:%s\nCompile date:%s\nCompile time:%s\n", running_app_info.project_name, running_app_info.version, running_app_info.date, running_app_info.time);
            }
            cprintf("\nType HELP for more info\n");
        }

        char line[34];
        cprintf("ESP>");
        creadline(line, sizeof(line), false);
        if (strcasecmp(line, "help") == 0)
            show_help();
        else if (strcasecmp(line, "wifi set") == 0)
            wifi_set();
        else if (strcasecmp(line, "wifi") == 0)
            wifi_status();
        else if (strcasecmp(line, "date") == 0)
            show_date();
        else if (strcasecmp(line, "update") == 0)
            system_update();
        else if (line[0])
            cprintf("Unknown command\n");

        // if (line[0] != '\0') {
        //     cprintf("You said: %s\n", line);
        // }
    }
}
