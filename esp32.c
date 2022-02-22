#include "esp32.h"

enum {
    ESP_OPEN     = 0x10, //  Open / create file
    ESP_CLOSE    = 0x11, //  Close open file
    ESP_READ     = 0x12, //  Read from file
    ESP_WRITE    = 0x13, //  Write to file
    ESP_SEEK     = 0x14, //  Move read/write pointer
    ESP_TELL     = 0x15, //  Get current read/write
    ESP_OPENDIR  = 0x16, //  Open directory
    ESP_CLOSEDIR = 0x17, //  Close open directory
    ESP_READDIR  = 0x18, //  Read from directory
    ESP_UNLINK   = 0x19, //  Remove file or directory
    ESP_RENAME   = 0x1A, //  Rename / move file or directory
    ESP_MKDIR    = 0x1B, //  Create directory
    ESP_CHDIR    = 0x1C, //  Change directory
    ESP_STAT     = 0x1D, //  Get file status
};

void esp32_init(const char *basepath) {
    (void)basepath;
}

void esp32_write_ctrl(uint8_t data) {
    printf("esp32_write_ctrl: %02X\n", data);
}

void esp32_write_data(uint8_t data) {
    printf("esp32_write_data: %02X\n", data);
}

uint8_t esp32_read_ctrl(void) {
    printf("esp32_read_ctrl\n");
    return 0;
}

uint8_t esp32_read_data(void) {
    printf("esp32_read_data\n");
    return 0;
}
