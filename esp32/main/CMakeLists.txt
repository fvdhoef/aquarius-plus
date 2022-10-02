idf_component_register(
    SRCS
        "main.c"
        "fpga.c"
        "usbhost.c"
        "sdcard.c"
        "aq_keyb.c"

    INCLUDE_DIRS
        "."

    EMBED_FILES
        "../../fpga/top.bit"
)
