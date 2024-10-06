idf_component_register(
    SRCS
        "main.cpp"
        "FPGA.cpp"
        "WiFi.cpp"
        "Bluetooth.cpp"
        "Keyboard.cpp"
        "FileServer.cpp"
        "XzDecompress.cpp"
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

    EMBED_FILES
        "assets/ovl_font.chr"
        "../../rom_src/romfs.bin"
        "assets/aqp_top.bit.xzh"
        "assets/root_certificates.xzh"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
