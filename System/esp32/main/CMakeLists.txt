idf_component_register(
    SRCS
        "main.cpp"
        "FPGA.cpp"
        "usbhost.cpp"
        "aq_keyb.cpp"
        "aq_keyb_press_key.cpp"
        "uart_protocol.cpp"

        "VFS/VFS.cpp"
        "VFS/SDCardVFS.cpp"
        "VFS/EspVFS.cpp"

        "EspSettingsConsole.cpp"

        "FileServer.cpp"
        "led.cpp"
        "hid.cpp"
        "connectivity/ca_store.cpp"
        "connectivity/wifi.cpp"
        "connectivity/ble.cpp"

        "connectivity/nimble-cpp/NimBLE2904.cpp"
        "connectivity/nimble-cpp/NimBLEAddress.cpp"
        "connectivity/nimble-cpp/NimBLEAdvertisedDevice.cpp"
        "connectivity/nimble-cpp/NimBLEAdvertising.cpp"
        "connectivity/nimble-cpp/NimBLEBeacon.cpp"
        "connectivity/nimble-cpp/NimBLECharacteristic.cpp"
        "connectivity/nimble-cpp/NimBLEClient.cpp"
        "connectivity/nimble-cpp/NimBLEDescriptor.cpp"
        "connectivity/nimble-cpp/NimBLEDevice.cpp"
        "connectivity/nimble-cpp/NimBLEEddystoneTLM.cpp"
        "connectivity/nimble-cpp/NimBLEEddystoneURL.cpp"
        "connectivity/nimble-cpp/NimBLEExtAdvertising.cpp"
        "connectivity/nimble-cpp/NimBLEHIDDevice.cpp"
        "connectivity/nimble-cpp/NimBLERemoteCharacteristic.cpp"
        "connectivity/nimble-cpp/NimBLERemoteDescriptor.cpp"
        "connectivity/nimble-cpp/NimBLERemoteService.cpp"
        "connectivity/nimble-cpp/NimBLEScan.cpp"
        "connectivity/nimble-cpp/NimBLESecurity.cpp"
        "connectivity/nimble-cpp/NimBLEServer.cpp"
        "connectivity/nimble-cpp/NimBLEService.cpp"
        "connectivity/nimble-cpp/NimBLEUUID.cpp"
        "connectivity/nimble-cpp/NimBLEUtils.cpp"


    INCLUDE_DIRS
        "."
        "VFS"
        "connectivity"
        "connectivity/nimble-cpp"

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
        # "esp-nimble-cpp"

    EMBED_FILES
        "../../fpga/top.bit"
        "../../rom_src/settings/build/settings.caq"

    EMBED_TXTFILES
        "connectivity/letsencrypt_root_certificate.pem"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
