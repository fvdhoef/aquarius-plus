idf_component_register(
    SRCS
        "main.cpp"
        "Common.cpp"
        "FPGA.cpp"
        "WiFi.cpp"
        "Bluetooth.cpp"
        "Keyboard.cpp"
        "FileServer.cpp"
        "PowerLED.cpp"
        "MidiData.cpp"
        "xz.c"

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
        "USB/USBInterfaceMIDI.cpp"

        "fatfs/ff.c"
        "fatfs/ffsystem.c"
        "fatfs/ffunicode.c"

        "UartProtocol.cpp"
        "VFS/VFS.cpp"
        "VFS/EspVFS.cpp"
        "VFS/SDCardVFS.cpp"
        "VFS/HttpVFS.cpp"
        "VFS/TcpVFS.cpp"

        "FpgaCores/FpgaCore.cpp"
        "FpgaCores/KbHcEmu.cpp"
        "FpgaCores/AquariusPlus/CoreAquariusPlus.cpp"
        "FpgaCores/Aquarius32/CoreAquarius32.cpp"

    INCLUDE_DIRS
        "."
        "HID"
        "VFS"
        "USB"
        "fatfs"
        "FpgaCores"

    EMBED_FILES
        "assets/ovl_font.chr"
        "assets/romfs.bin"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
