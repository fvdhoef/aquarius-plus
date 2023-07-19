idf_component_register(
    SRCS
        "main.c"
        "fpga.c"
        "usbhost.c"
        "sdcard.c"
        "aq_keyb.c"
        "uart_protocol.c"
        "direnum.c"
        "vfs.c"
        "vfs_esp.c"
        "fileserver.c"
        "tcpserver.c"
        "led.c"
        "connectivity/ca_store.c"
        "connectivity/wifi.c"
        "connectivity/bluetooth.c"

    INCLUDE_DIRS
        "."
        "connectivity"

    REQUIRES
        "driver"
        "log"
        "esp_event"
        "esp_timer"
        "esp_rom"
        "esp-tls"
        "nvs_flash"
        "esp_wifi"
        "esp_http_client"
        "esp_http_server"
        "json"
        "app_update"
        "usb"
        "fatfs"
        "bt"
        "nimble_central_utils"

    EMBED_FILES
        "../../fpga/top.bit"
        "../../rom_src/settings/build/settings.caq"

    EMBED_TXTFILES
        "connectivity/letsencrypt_root_certificate.pem"
)
