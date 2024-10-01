idf_component_register(
    SRCS
        "main.cpp"
        "FPGA.cpp"
        "WiFi.cpp"
        "Bluetooth.cpp"
        "Keyboard.cpp"
        "FileServer.cpp"
        "PowerLED.cpp"

        "DisplayOverlay/DisplayOverlay.cpp"
        "DisplayOverlay/Menu.cpp"
        "DisplayOverlay/Menus.cpp"
        "DisplayOverlay/GitHubUpdateMenu.cpp"
        "DisplayOverlay/SdCardUpdateMenu.cpp"

        "HID/HIDReportDescriptor.cpp"
        "HID/HIDReportHandler.cpp"
        "HID/HIDReportHandlerKeyboard.cpp"
        "HID/HIDReportHandlerMouse.cpp"
        "HID/HIDReportHandlerGamepad.cpp"

        "USB/USBHost.cpp"
        "USB/USBDevice.cpp"
        "USB/USBInterface.cpp"
        "USB/USBInterfaceHID.cpp"

        "fatfs/ff.c"
        "fatfs/ffsystem.c"
        "fatfs/ffunicode.c"

        "UartProtocol.cpp"
        "VFS/EspVFS.cpp"
        "VFS/SDCardVFS.cpp"
        "VFS/HttpVFS.cpp"
        "VFS/TcpVFS.cpp"

        "FpgaCores/FpgaCore.cpp"
        "FpgaCores/AquariusPlus/CoreAquariusPlus.cpp"

    INCLUDE_DIRS
        "."
        "HID"
        "VFS"
        "USB"
        "fatfs"
        "FpgaCores"
        "FpgaCores/AquariusPlus"

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
        "bt"

    EMBED_FILES
        "../../fpga/aqp_top.bit"
        "assets/ovl_font.chr"
        "../../rom_src/romfs.bin"

    EMBED_TXTFILES
        "assets/root_certificates"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
