#include "ch376.h"
#include "direnum.h"
#include "fat.h"

enum {
    CMD_GET_IC_VER    = 0x01, // Get the chip and firmware versions
    CMD_SET_BAUDRATE  = 0x02,
    CMD_ENTER_SLEEP   = 0x03,
    CMD_RESET_ALL     = 0x05,
    CMD_CHECK_EXIST   = 0x06,
    CMD_SET_SDO_INT   = 0x0B,
    CMD_GET_FILE_SIZE = 0x0C,
    CMD_SET_USB_MODE  = 0x15,
    CMD_GET_STATUS    = 0x22,
    CMD_RD_USB_DATA0  = 0x27,
    CMD_WR_HOST_DATA  = 0x2C,
    CMD_WR_REQ_DATA   = 0x2D,
    CMD_WR_OFS_DATA   = 0x2E,
    CMD_SET_FILE_NAME = 0x2F,
    CMD_DISK_CONNECT  = 0x30,
    CMD_DISK_MOUNT    = 0x31,
    CMD_FILE_OPEN     = 0x32,
    CMD_FILE_ENUM_GO  = 0x33,
    CMD_FILE_CREATE   = 0x34,
    CMD_FILE_ERASE    = 0x35,
    CMD_FILE_CLOSE    = 0x36,
    CMD_DIR_INFO_READ = 0x37,
    CMD_DIR_INFO_SAVE = 0x38,
    CMD_BYTE_LOCATE   = 0x39,
    CMD_BYTE_READ     = 0x3A,
    CMD_BYTE_RD_GO    = 0x3B,
    CMD_BYTE_WRITE    = 0x3C,
    CMD_BYTE_WR_GO    = 0x3D,
    CMD_DISK_CAPACITY = 0x3E,
    CMD_DISK_QUERY    = 0x3F,
    CMD_DIR_CREATE    = 0x40,
    CMD_SEC_LOCATE    = 0x4A,
    CMD_SEC_READ      = 0x4B,
    CMD_SEC_WRITE     = 0x4C,
    CMD_DISK_BOC_CMD  = 0x50,
    CMD_DISK_READ     = 0x54,
    CMD_DISK_RD_GO    = 0x55,
    CMD_DISK_WRITE    = 0x56,
    CMD_DISK_WR_GO    = 0x57,
};

enum {
    CMD_RET_SUCCESS = 0x51,
    CMD_RET_ABORT   = 0x5F,
};

enum {
    USB_INT_SUCCESS   = 0x14,
    USB_INT_DISK_READ = 0x1D,

    ERR_OPEN_DIR  = 0x41,
    ERR_MISS_FILE = 0x42,
};

static uint8_t rdbuf[256];
static uint8_t rdbuf_idx = 0;
static uint8_t cur_cmd   = 0;

static uint8_t wrbuf[256];
static uint8_t wrbuf_idx = 0;

static uint8_t intstatus = 0;

struct fat_dir_info {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t filesize;
};

#define PATH_MAX (1024)

static char *basepath;

void ch376_init(const char *path) {
    basepath = strdup(path);

    fat_init(basepath);
}

void ch376_write_cmd(uint8_t cmd) {
    if (basepath == NULL)
        return;

    cur_cmd   = cmd;
    wrbuf_idx = 0;

    memset(rdbuf, 0, sizeof(rdbuf));
    rdbuf_idx = 0;

    switch (cmd) {
        case CMD_DISK_MOUNT: {
            printf("- Mount disk\n");
            intstatus = USB_INT_SUCCESS;
            break;
        }
        case CMD_GET_STATUS: {
            rdbuf[0] = intstatus;
            break;
        }

        case CMD_FILE_OPEN:
            printf("- File open\n");
            intstatus = USB_INT_DISK_READ;
            break;

        case CMD_RD_USB_DATA0: {
            printf("- USB Read Data0\n");
            rdbuf[0] = 32;

            struct fat_dir_info fdi;
            memset(&fdi, 0, sizeof(fdi));

            memcpy(fdi.name, "HELLO   PT3", 11);
            fdi.filesize = 0;
            fdi.attr     = 0x10;

            memcpy(rdbuf + 1, &fdi, sizeof(fdi));
            break;
        }

        case CMD_FILE_ENUM_GO:
            printf("- File enum go\n");
            intstatus = ERR_MISS_FILE;
            break;

        default: {
            printf("CH376 CMD WR: %02X\n", cmd);
            break;
        }
    }
}

void ch376_write_data(uint8_t data) {
    if (basepath == NULL)
        return;

    printf("> CH376 DATA WR: %02X\n", data);

    wrbuf[wrbuf_idx++] = data;

    switch (cur_cmd) {
        case CMD_CHECK_EXIST:
            rdbuf[0]  = ~data;
            rdbuf_idx = 0;
            break;

        case CMD_SET_USB_MODE:
            printf("- Set USB mode: %02X\n", data);
            rdbuf[0]  = CMD_RET_SUCCESS;
            rdbuf_idx = 0;
            break;

        case CMD_SET_FILE_NAME: {
            if (data == 0) {
                printf("- Set filename: %s\n", wrbuf);
            }
            break;
        }

        default: break;
    }
}

uint8_t ch376_read_data(void) {
    if (basepath == NULL)
        return 0xFF;

    uint8_t result = rdbuf[rdbuf_idx++];
    printf("> CH376 DATA RD: %02X\n", result);
    return result;
}

uint8_t ch376_read_status(void) {
    if (basepath == NULL)
        return 0xFF;

    uint8_t result = 0;
    printf("> CH376 STATUS RD: %02X\n", result);
    return result;
}
