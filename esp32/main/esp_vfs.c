#include "esp_vfs.h"
#include "wifi.h"
#include <freertos/stream_buffer.h>
#include <esp_ota_ops.h>
#include <esp_wifi.h>

extern const uint8_t terminal_caq_start[] asm("_binary_esp_terminal_caq_start");
extern const uint8_t terminal_caq_end[] asm("_binary_esp_terminal_caq_end");
static const char   *fn_terminal = "terminal.caq";
static const char   *fn_com      = "com";

static int dir_idx     = 0;
static int file_offset = 0;
static int file_idx    = 0;

static volatile bool new_session = true;

static StreamBufferHandle_t tx_buffer;
static StreamBufferHandle_t rx_buffer;

static int esp_open(uint8_t flags, const char *path) {
    if (strcasecmp(path + 4, fn_com) == 0) {
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
    if (strcasecmp(path + 4, fn_terminal) == 0) {
        file_idx = 0;
    } else {
        file_idx = -1;
    }

    file_offset = 0;
    return file_idx < 0 ? ERR_NOT_FOUND : 0;
}

static int esp_read(int fd, uint16_t size, void *buf) {
    if (fd == 0) {
        int filesize  = terminal_caq_end - terminal_caq_start;
        int remaining = filesize - file_offset;

        if (size > remaining) {
            size = remaining;
        }
        memcpy(buf, terminal_caq_start + file_offset, size);
        file_offset += size;
        return size;

    } else if (fd == 1) {
        int result = xStreamBufferReceive(tx_buffer, buf, size, 0);
        return result;

    } else {
        return ERR_OTHER;
    }
}

static int esp_write(int fd, uint16_t size, const void *buf) {
    if (fd == 1) {
        return xStreamBufferSend(rx_buffer, buf, size, 0);
    } else {
        return ERR_OTHER;
    }
}

static int esp_close(int fd) {
    return 0;
}

static int esp_opendir(const char *path) {
    printf("esp_opendir: %s\n", path);
    dir_idx = 0;
    return 0;
}

static int esp_closedir(int dd) {
    return 0;
}

static int esp_readdir(int dd, struct direnum_ent *de) {
    if (dir_idx++ == 0) {
        snprintf(de->filename, sizeof(de->filename), fn_terminal);
        de->attr = 0;
        de->size = terminal_caq_end - terminal_caq_start;
        de->t    = 0;
        return 0;
    }

    return ERR_EOF;
}

static int esp_stat(const char *path, struct stat *st) {
    if (strcasecmp(path, "esp:") == 0) {
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
    cprintf("help     | this help\n");
    cprintf("wifi     | show WiFi status\n");
    cprintf("wifi set | set WiFi network\n");
    cprintf("date     | show current time/date\n");
    cprintf("ctrl-c   | exit to BASIC\n");
}

static void wifi_status(void) {
    cprintf("WiFi status:\n");

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    cprintf("MAC     :%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    wifi_ap_record_t war;
    if (esp_wifi_sta_get_ap_info(&war) == ESP_OK) {
        cprintf("SSID    :%s\n", war.ssid);
        cprintf("Channel :%u\n", war.primary);
        cprintf("RSSI    :%d\n", war.rssi);
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

#define MAX_SCAN_AP (20)

static void wifi_set(void) {
    cprintf("Scanning networks\n");

    uint16_t         ap_count = MAX_SCAN_AP;
    wifi_ap_record_t ap_info[MAX_SCAN_AP];

    esp_err_t result = esp_wifi_scan_start(NULL, true);
    if (result == ESP_OK) {
        memset(ap_info, 0, sizeof(ap_info));
        result = esp_wifi_scan_get_ap_records(&ap_count, ap_info);
    }
    if (result != ESP_OK) {
        cprintf("Error during scan!\n");
    }

    for (int i = 0; i < ap_count; i++) {
        cprintf("%d) %s\n", i, ap_info[i].ssid);
    }

    cprintf("Select network:");

    char str[16];
    creadline(str, sizeof(str), false);
    if (new_session)
        return;

    char    *endp;
    unsigned idx = strtoul(str, &endp, 10);
    if (*endp != '\0' || idx > ap_count) {
        cprintf("Invalid entry, aborting.\n");
        return;
    }

    cprintf("Selected:%s\n", ap_info[idx].ssid);
    char password[65] = {0};
    if (ap_info[idx].authmode != WIFI_AUTH_OPEN) {
        cprintf("Password:");
        creadline(password, sizeof(password), true);
        if (new_session)
            return;
    }

    wifi_config_t wifi_config = {0};
    snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), (const char *)ap_info[idx].ssid);
    snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), password);

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void show_date(void) {
    time_t    now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S (%Z)", &timeinfo);
    cprintf("%s\n", strftime_buf);
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
        else if (line[0])
            cprintf("Unknown command\n");

        // if (line[0] != '\0') {
        //     cprintf("You said: %s\n", line);
        // }
    }
}

void esp_vfs_init(void) {
    tx_buffer = xStreamBufferCreate(256, 1);
    rx_buffer = xStreamBufferCreate(256, 1);

    xTaskCreate(console_task, "console", 8192, NULL, 12, NULL);
}

struct vfs esp_vfs = {
    .open  = esp_open,
    .close = esp_close,
    .read  = esp_read,
    .write = esp_write,

    .opendir  = esp_opendir,
    .closedir = esp_closedir,
    .readdir  = esp_readdir,
    .stat     = esp_stat,
};
