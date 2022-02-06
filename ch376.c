#include "ch376.h"

enum {
    GET_IC_VER    = 0x01, // Get the chip and firmware versions
    SET_BAUDRATE  = 0x02,
    ENTER_SLEEP   = 0x03,
    RESET_ALL     = 0x05,
    CHECK_EXIST   = 0x06,
    SET_SDO_INT   = 0x0B,
    GET_FILE_SIZE = 0x0C,
    SET_USB_MODE  = 0x15,
    GET_STATUS    = 0x22,
    RD_USB_DATA0  = 0x27,
    WR_HOST_DATA  = 0x2C,
    WR_REQ_DATA   = 0x2D,
    WR_OFS_DATA   = 0x2E,
    SET_FILE_NAME = 0x2F,
    DISK_CONNECT  = 0x30,
    DISK_MOUNT    = 0x31,
    FILE_OPEN     = 0x32,
    FILE_ENUM_GO  = 0x33,
    FILE_CREATE   = 0x34,
    FILE_ERASE    = 0x35,
    FILE_CLOSE    = 0x36,
    DIR_INFO_READ = 0x37,
    DIR_INFO_SAVE = 0x38,
    BYTE_LOCATE   = 0x39,
    BYTE_READ     = 0x3A,
    BYTE_RD_GO    = 0x3B,
    BYTE_WRITE    = 0x3C,
    BYTE_WR_GO    = 0x3D,
    DISK_CAPACITY = 0x3E,
    DISK_QUERY    = 0x3F,
    DIR_CREATE    = 0x40,
    SEC_LOCATE    = 0x4A,
    SEC_READ      = 0x4B,
    SEC_WRITE     = 0x4C,
    DISK_BOC_CMD  = 0x50,
    DISK_READ     = 0x54,
    DISK_RD_GO    = 0x55,
    DISK_WRITE    = 0x56,
    DISK_WR_GO    = 0x57,
};

enum {
    CMD_RET_SUCCESS = 0x51,
    CMD_RET_ABORT   = 0x5F,
};

enum {
    USB_INT_SUCCESS   = 0x14,
    USB_INT_DISK_READ = 0x1D,
};

static uint8_t rdbuf[256];
static uint8_t rdbuf_idx = 0;
static uint8_t cur_cmd   = 0;

static uint8_t wrbuf[256];
static uint8_t wrbuf_idx = 0;

static uint8_t intstatus = 0;

void ch376_write_cmd(uint8_t cmd) {
    printf("CH376 CMD WR: %02X\n", cmd);
    cur_cmd   = cmd;
    wrbuf_idx = 0;

    memset(rdbuf, 0, sizeof(rdbuf));
    rdbuf_idx = 0;

    switch (cmd) {
        case DISK_MOUNT: {
            printf("- Mount disk\n");
            intstatus = USB_INT_SUCCESS;
            break;
        }
        case GET_STATUS: {
            rdbuf[0] = intstatus;
            break;
        }

        case FILE_OPEN:
            printf("- File open\n");
            intstatus = USB_INT_DISK_READ;
            break;

        default: break;
    }
}

void ch376_write_data(uint8_t data) {
    printf("CH376 DATA WR: %02X\n", data);

    wrbuf[wrbuf_idx++] = data;

    switch (cur_cmd) {
        case CHECK_EXIST:
            rdbuf[0]  = ~data;
            rdbuf_idx = 0;
            break;

        case SET_USB_MODE:
            printf("- Set USB mode: %02X\n", data);
            rdbuf[0]  = CMD_RET_SUCCESS;
            rdbuf_idx = 0;
            break;

        case SET_FILE_NAME: {
            if (data == 0) {
                printf("- Set filename: %s\n", wrbuf);
            }
            break;
        }

        default: break;
    }
}

uint8_t ch376_read_data(void) {
    uint8_t result = rdbuf[rdbuf_idx++];
    printf("CH376 DATA RD: %02X\n", result);
    return result;
}

uint8_t ch376_read_status(void) {
    uint8_t result = 0;
    printf("CH376 STATUS RD: %02X\n", result);
    return result;
}
