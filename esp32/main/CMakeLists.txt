idf_component_register(
    SRCS
        "main.c"
        "fpga.c"
        "usbhost.c"
        "sdcard.c"

    INCLUDE_DIRS
        "."

    EMBED_FILES
        "../../fpga/top.bit"
)
