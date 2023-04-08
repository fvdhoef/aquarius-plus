idf_component_register(
    SRCS
        "main.c"
        "fpga.c"
        "usbhost.c"
        "sdcard.c"
        "aq_keyb.c"
        "flash.c"
        "screen.c"
        "uart_protocol.c"
        "direnum.c"
        "vfs.c"
        "esp_vfs.c"
        "connectivity/ca_store.c"
        "connectivity/wifi.c"

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
        "json"
        "app_update"
        "usb"
        "fatfs"

    EMBED_FILES
        "../../fpga/top.bit"
        "../../assets/aquarius_s2.rom"
        "../../emulator/aquarius.rom"

    EMBED_TXTFILES
        "connectivity/letsencrypt_root_certificate.pem"
)
