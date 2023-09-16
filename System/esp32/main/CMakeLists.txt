idf_component_register(
    SRCS
        "main.cpp"
        "FPGA.cpp"
        "AqKeyboard.cpp"
        "AqUartProtocol.cpp"
        "MemDump.cpp"

        "USB/USBHost.cpp"
        "USB/USBDevice.cpp"
        "USB/USBInterface.cpp"
        "USB/USBInterfaceHID.cpp"

        "VFS/VFS.cpp"
        "VFS/SDCardVFS.cpp"
        "VFS/EspVFS.cpp"

        "VFS/fatfs/ff.c"
        "VFS/fatfs/ffsystem.c"
        "VFS/fatfs/ffunicode.c"

        "HID/HIDReportDescriptor.cpp"
        "HID/HIDReportHandler.cpp"
        "HID/HIDReportHandlerKeyboard.cpp"
        "HID/HIDReportHandlerMouse.cpp"
        "HID/HIDReportHandlerGamepad.cpp"

        "EspSettingsConsole.cpp"

        "FileServer.cpp"
        "PowerLED.cpp"
        "WiFi.cpp"
        "BLE.cpp"

        "NimBLE_Cpp/NimBLE2904.cpp"
        "NimBLE_Cpp/NimBLEAddress.cpp"
        "NimBLE_Cpp/NimBLEAdvertisedDevice.cpp"
        "NimBLE_Cpp/NimBLEAdvertising.cpp"
        "NimBLE_Cpp/NimBLEBeacon.cpp"
        "NimBLE_Cpp/NimBLECharacteristic.cpp"
        "NimBLE_Cpp/NimBLEClient.cpp"
        "NimBLE_Cpp/NimBLEDescriptor.cpp"
        "NimBLE_Cpp/NimBLEDevice.cpp"
        "NimBLE_Cpp/NimBLEEddystoneTLM.cpp"
        "NimBLE_Cpp/NimBLEEddystoneURL.cpp"
        "NimBLE_Cpp/NimBLEExtAdvertising.cpp"
        "NimBLE_Cpp/NimBLEHIDDevice.cpp"
        "NimBLE_Cpp/NimBLERemoteCharacteristic.cpp"
        "NimBLE_Cpp/NimBLERemoteDescriptor.cpp"
        "NimBLE_Cpp/NimBLERemoteService.cpp"
        "NimBLE_Cpp/NimBLEScan.cpp"
        "NimBLE_Cpp/NimBLESecurity.cpp"
        "NimBLE_Cpp/NimBLEServer.cpp"
        "NimBLE_Cpp/NimBLEService.cpp"
        "NimBLE_Cpp/NimBLEUUID.cpp"
        "NimBLE_Cpp/NimBLEUtils.cpp"


    INCLUDE_DIRS
        "."
        "VFS"
        "VFS/fatfs"
        "HID"
        "USB"
        "NimBLE_Cpp"

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
        "esp_https_ota"
        "json"
        "app_update"
        "usb"
        "sdmmc"
        # "fatfs"
        "bt"

    EMBED_FILES
        "../../fpga/top.bit"
        "../../rom_src/settings/build/settings.aqx"

    EMBED_TXTFILES
        "Assets/root_certificates"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
