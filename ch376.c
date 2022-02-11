#include "ch376.h"
#include "direnum.h"
#include "fat.h"

enum {
    CMD_GET_IC_VER    = 0x01, // Get the chip and firmware versions
    CMD_SET_BAUDRATE  = 0x02,
    CMD_ENTER_SLEEP   = 0x03,
    CMD_SET_USB_SPEED = 0x04,
    CMD_RESET_ALL     = 0x05,
    CMD_CHECK_EXIST   = 0x06,
    CMD_SET_SDO_INT   = 0x0B,
    CMD_GET_FILE_SIZE = 0x0C,
    CMD_SET_FILE_SIZE = 0x0D,
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
    USB_INT_SUCCESS    = 0x14,
    USB_INT_DISK_READ  = 0x1D,
    USB_INT_DISK_WRITE = 0x1E,
    ERR_OPEN_DIR       = 0x41,
    ERR_MISS_FILE      = 0x42,
};

static uint8_t result_buf[256];

static uint8_t rdbuf[256];
static uint8_t rdbuf_idx = 0;
static uint8_t cur_cmd   = 0;

static uint8_t wrbuf[256];
static uint8_t wrbuf_idx = 0;

static uint8_t  intstatus          = 0;
static uint16_t rdlength_remaining = 0;
static uint16_t wrlength_remaining = 0;

static char *basepath;
static char  cur_filename[16];

void ch376_init(const char *path) {
    basepath = strdup(path);

    fat_init(basepath);
    // fat_open("aqubasic.rom");
}

static void read_file(void) {
    if (rdlength_remaining == 0) {
        intstatus     = USB_INT_SUCCESS;
        result_buf[0] = 0;
    } else {
        unsigned bytes_to_read = rdlength_remaining;
        if (bytes_to_read > 255)
            bytes_to_read = 255;

        int bytes_read = fat_read(result_buf + 1, bytes_to_read);
        result_buf[0]  = bytes_read;

        if (bytes_read == 0) {
            rdlength_remaining = 0;
            intstatus          = USB_INT_SUCCESS;
        } else {
            intstatus = USB_INT_DISK_READ;
            rdlength_remaining -= bytes_read;
        }
    }
}

static void read_dir(void) {
    int bytes_read = fat_read(result_buf + 1, 32);
    if (bytes_read == 32) {
        result_buf[0] = 32;
        intstatus     = USB_INT_DISK_READ;
    } else {
        intstatus = ERR_MISS_FILE;
    }
}

void ch376_write_cmd(uint8_t cmd) {
    if (basepath == NULL)
        return;

    cur_cmd   = cmd;
    wrbuf_idx = 0;

    memset(rdbuf, 0, sizeof(rdbuf));
    rdbuf_idx = 0;

    switch (cmd) {
        case CMD_CHECK_EXIST:
        case CMD_SET_USB_MODE:
        case CMD_SET_FILE_NAME:
        case CMD_FILE_CLOSE:
        case CMD_BYTE_READ:
        case CMD_BYTE_WRITE:
        case CMD_SET_FILE_SIZE: break;

        case CMD_DISK_MOUNT: {
            // printf("- Mount disk\n");
            intstatus = USB_INT_SUCCESS;
            break;
        }
        case CMD_GET_STATUS: {
            // printf("CMD_GET_STATUS\n");
            rdbuf[0] = intstatus;
            break;
        }

        case CMD_FILE_OPEN: {
            printf("- File open: %s\n", cur_filename);
            int result = fat_open(cur_filename);
            if (result == 0) {
                intstatus = USB_INT_DISK_READ;
            } else if (result == OPEN_IS_DIR) {
                intstatus = ERR_OPEN_DIR;
            } else if (result == OPEN_ENUM_DIR) {
                read_dir();
            } else if (result == OPEN_IS_FILE) {
                intstatus = USB_INT_SUCCESS;
            } else if (result == ERR_INVALID_NAME) {
                intstatus = ERR_MISS_FILE;
            } else {
                intstatus = CMD_RET_ABORT;
            }
            break;
        }

        case CMD_FILE_CREATE: {
            printf("- File create: %s\n", cur_filename);
            int result = fat_create(cur_filename);
            if (result == 0) {
                intstatus = USB_INT_SUCCESS;
            } else {
                intstatus = CMD_RET_ABORT;
            }
            break;
        }

        case CMD_RD_USB_DATA0: {
            memcpy(rdbuf, result_buf, sizeof(rdbuf));
            // printf("- USB Read Data0: %u bytes in buffer\n", rdbuf[0]);
            break;
        }

        case CMD_FILE_ENUM_GO:
            // printf("- File enum go\n");
            read_dir();
            break;

        case CMD_BYTE_RD_GO: {
            // printf("- Byte read continue (%u bytes remaining)\n", rdlength_remaining);
            read_file();
            break;
        }

        case CMD_BYTE_WR_GO:
        case CMD_WR_REQ_DATA: {
            // printf("- Write data (wrlength_remaining: %u)\n", wrlength_remaining);
            rdbuf[0] = wrlength_remaining > 64 ? 64 : wrlength_remaining;
            if (wrlength_remaining == 0)
                intstatus = USB_INT_SUCCESS;

            break;
        }

        default: {
            printf("CH376 CMD WR: %02X\n", cmd);
            break;
        }
    }
}

void ch376_write_data(uint8_t data) {
    if (basepath == NULL)
        return;

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
                strncpy(cur_filename, (const char *)wrbuf, sizeof(cur_filename) - 1);
                cur_filename[sizeof(cur_filename) - 1] = 0;
            }
            break;
        }

        case CMD_FILE_CLOSE: {
            printf("- Close file\n");
            fat_close();
            break;
        }

        case CMD_BYTE_READ: {
            if (wrbuf_idx == 2) {
                rdlength_remaining = wrbuf[0] | (wrbuf[1] << 8);
                // printf("- Read %u bytes\n", rdlength_remaining);
                read_file();
            }
            break;
        }

        case CMD_BYTE_WRITE: {
            if (wrbuf_idx == 2) {
                wrlength_remaining = wrbuf[0] | (wrbuf[1] << 8);
                // printf("- Write %u bytes\n", wrlength_remaining);

                if (wrlength_remaining == 0) {
                    intstatus = USB_INT_SUCCESS;
                } else {
                    intstatus = USB_INT_DISK_WRITE;
                }
                // read_file();
            }
            break;
        }

        case CMD_BYTE_LOCATE: {
            if (wrbuf_idx == 4) {
                unsigned offset = wrbuf[0] | (wrbuf[1] << 8) | (wrbuf[2] << 16) | (wrbuf[3] << 24);
                printf("- Seek: %u\n", offset);
                fat_seek(offset);
            }
            break;
        }

        case CMD_BYTE_WR_GO:
        case CMD_WR_REQ_DATA: {
            // printf("- Write data: %02X\n", data);
            fat_write(&data, 1);
            if (wrlength_remaining > 0)
                wrlength_remaining--;
            break;
        }

        case CMD_SET_FILE_SIZE: {
            if (wrbuf_idx == 5) {
                // This command isn't really supported, but we just truncate the file.
                fat_truncate();
            }
            break;
        }

        default:
            printf("> CH376 DATA WR: %02X\n", data);
            break;
    }
}

uint8_t ch376_read_data(void) {
    if (basepath == NULL)
        return 0xFF;

    uint8_t result = rdbuf[rdbuf_idx++];
    // printf("> CH376 DATA RD: %02X\n", result);
    return result;
}

uint8_t ch376_read_status(void) {
    if (basepath == NULL)
        return 0xFF;

    uint8_t result = 0;
    // printf("> CH376 STATUS RD: %02X\n", result);
    return result;
}
