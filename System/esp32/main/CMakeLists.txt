idf_component_register(
    SRCS
        "main.cpp"
        "fpga.cpp"
        "usbhost.cpp"
        "sdcard.cpp"
        "aq_keyb.cpp"
        "aq_keyb_press_key.cpp"
        "uart_protocol.cpp"
        "direnum.cpp"
        "vfs.cpp"
        "vfs_esp.cpp"
        "fileserver.cpp"
        "led.cpp"
        "hid.cpp"
        "connectivity/ca_store.cpp"
        "connectivity/wifi.cpp"
        "connectivity/ble.cpp"

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
        "esp-nimble-cpp"

    EMBED_FILES
        "../../fpga/top.bit"
        "../../rom_src/settings/build/settings.caq"

    EMBED_TXTFILES
        "connectivity/letsencrypt_root_certificate.pem"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
