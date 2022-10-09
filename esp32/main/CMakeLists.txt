idf_component_register(
    SRCS
        "main.c"
        "fpga.c"
        "usbhost.c"
        "sdcard.c"
        "aq_keyb.c"
        "flash.c"
        "screen.c"

    INCLUDE_DIRS
        "."

    EMBED_FILES
        "../../fpga/top.bit"
        "../../assets/aquarius_s2.rom"
        "../../emulator/aquarius.rom"
)
