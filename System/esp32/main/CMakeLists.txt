idf_component_register(
    SRCS
        "main.cpp"
        "FPGA.cpp"
        "MemDump.cpp"

        "USB/USBHost.cpp"
        "USB/USBDevice.cpp"
        "USB/USBInterface.cpp"
        "USB/USBInterfaceHID.cpp"

        "fatfs/ff.c"
        "fatfs/ffsystem.c"
        "fatfs/ffunicode.c"

        "HID/HIDReportDescriptor.cpp"
        "HID/HIDReportHandler.cpp"
        "HID/HIDReportHandlerKeyboard.cpp"
        "HID/HIDReportHandlerMouse.cpp"
        "HID/HIDReportHandlerGamepad.cpp"

        # Files shared between emulator and ESP32
        "../../emulator/Shared/AqKeyboard.cpp"
        "../../emulator/Shared/AqUartProtocol.cpp"
        "../../emulator/Shared/EspSettingsConsole.cpp"
        "../../emulator/Shared/VFS/VFS.cpp"
        "../../emulator/Shared/VFS/EspVFS.cpp"
        "../../emulator/Shared/VFS/SDCardVFS.cpp"
        "../../emulator/Shared/VFS/HttpVFS.cpp"
        "../../emulator/Shared/VFS/TcpVFS.cpp"

        "FileServer.cpp"
        "PowerLED.cpp"
        "WiFi.cpp"
        "BLE.cpp"

        "NimBLE_Cpp/NimBLEAddress.cpp"
        "NimBLE_Cpp/NimBLEAdvertisedDevice.cpp"
        "NimBLE_Cpp/NimBLEClient.cpp"
        "NimBLE_Cpp/NimBLEDevice.cpp"
        "NimBLE_Cpp/NimBLERemoteCharacteristic.cpp"
        "NimBLE_Cpp/NimBLERemoteDescriptor.cpp"
        "NimBLE_Cpp/NimBLERemoteService.cpp"
        "NimBLE_Cpp/NimBLEScan.cpp"
        "NimBLE_Cpp/NimBLEUUID.cpp"
        "NimBLE_Cpp/NimBLEUtils.cpp"

    INCLUDE_DIRS
        "."
        "../../emulator/Shared"
        "../../emulator/Shared/VFS"
        "fatfs"
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
