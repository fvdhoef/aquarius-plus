#pragma once

#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

// KEYBUF
// | Bit | Description                  |
// | --: | ---------------------------- |
// |  31 | Empty(1)                     |
// |  14 | Scancode(1) / Character(0)   |
// |  13 | Scancode key up(0) / down(1) |
// |  12 | Repeated                     |
// |  11 | Modifier: Gui                |
// |  10 | Modifier: Alt                |
// |   9 | Modifier: Shift              |
// |   8 | Modifier: Ctrl               |
// | 7:0 | Character / Scancode         |

#define KEY_IS_SCANCODE (1 << 14)
#define KEY_KEYDOWN     (1 << 13)
#define KEY_IS_REPEATED (1 << 12)
#define KEY_MOD_GUI     (1 << 11)
#define KEY_MOD_ALT     (1 << 10)
#define KEY_MOD_SHIFT   (1 << 9)
#define KEY_MOD_CTRL    (1 << 8)

#define KEY_MODIFIERS (KEY_MOD_GUI | KEY_MOD_ALT | KEY_MOD_SHIFT | KEY_MOD_CTRL)
#define KEY_CODE_MASK (0xFF)

struct regs_video {
    volatile uint16_t PALETTE[16];
    union {
        volatile uint32_t POSX1616;
        struct {
            volatile uint16_t _pad1;
            volatile uint16_t POSX16;
        };
    };
    union {
        volatile uint32_t POSY1616;
        struct {
            volatile uint16_t _pad2;
            volatile uint16_t POSY16;
        };
    };
    volatile uint32_t COLOR;
    volatile uint32_t REMAP_T;
    volatile uint8_t  REMAP[16];
    union {
        volatile uint32_t CLIPRECT;
        struct {
            volatile uint8_t CLIPX1;
            volatile uint8_t CLIPX2;
            volatile uint8_t CLIPY1;
            volatile uint8_t CLIPY2;
        };
    };
    volatile uint32_t FLAGS;
    volatile uint32_t WR1BPP;
    volatile uint32_t WR4BPP;
    volatile uint32_t PAGE;
};

#define ESP_STATUS (*(volatile uint32_t *)0x2000)
#define ESP_DATA   (*(volatile uint32_t *)0x2004)
#define KEYBUF     (*(volatile int32_t *)0x2010)
#define HANDCTRL   (*(volatile uint16_t *)0x2014)
#define KEYS       (*(volatile uint64_t *)0x2018)
#define GAMEPAD1   (*(volatile uint64_t *)0x2020)
#define GAMEPAD2   (*(volatile uint64_t *)0x2028)

#define TRAM  ((struct regs_tram *)0x06000)
#define CHRAM ((volatile uint8_t *)0x05000)

#define VIDEO    ((struct regs_video *)0x20000)
#define VRAM     ((volatile uint32_t *)0x28000)
#define VRAM4BIT ((volatile uint8_t *)0x30000)

enum {
    ESPCMD_RESET       = 0x01, // Reset ESP
    ESPCMD_VERSION     = 0x02, // Get version string
    ESPCMD_GETDATETIME = 0x03, // Get current date/time
    ESPCMD_KEYMODE     = 0x08, // Set keyboard buffer mode
    ESPCMD_GETMOUSE    = 0x0C, // Get mouse state
    ESPCMD_GETGAMECTRL = 0x0E, // Get game controller state
    ESPCMD_GETMIDIDATA = 0x0F, // Get mouse state
    ESPCMD_OPEN        = 0x10, // Open / create file
    ESPCMD_CLOSE       = 0x11, // Close open file
    ESPCMD_READ        = 0x12, // Read from file
    ESPCMD_WRITE       = 0x13, // Write to file
    ESPCMD_SEEK        = 0x14, // Move read/write pointer
    ESPCMD_TELL        = 0x15, // Get current read/write
    ESPCMD_OPENDIR     = 0x16, // Open directory
    ESPCMD_CLOSEDIR    = 0x17, // Close open directory
    ESPCMD_READDIR     = 0x18, // Read from directory
    ESPCMD_DELETE      = 0x19, // Remove file or directory
    ESPCMD_RENAME      = 0x1A, // Rename / move file or directory
    ESPCMD_MKDIR       = 0x1B, // Create directory
    ESPCMD_CHDIR       = 0x1C, // Change directory
    ESPCMD_STAT        = 0x1D, // Get file status
    ESPCMD_GETCWD      = 0x1E, // Get current working directory
    ESPCMD_CLOSEALL    = 0x1F, // Close any open file/directory descriptor
    ESPCMD_OPENDIR83   = 0x20, // Open directory in 8.3 filename mode
    ESPCMD_READLINE    = 0x21, // Read line from file
    ESPCMD_OPENDIREXT  = 0x22, // Open directory with extended options
    ESPCMD_LSEEK       = 0x23, // Seek in file with offset and whence
    ESPCMD_LOADFPGA    = 0x40, // Load FPGA bitstream
};

enum {
    ERR_NOT_FOUND     = -1, // File / directory not found
    ERR_TOO_MANY_OPEN = -2, // Too many open files / directories
    ERR_PARAM         = -3, // Invalid parameter
    ERR_EOF           = -4, // End of file / directory
    ERR_EXISTS        = -5, // File already exists
    ERR_OTHER         = -6, // Other error
    ERR_NO_DISK       = -7, // No disk
    ERR_NOT_EMPTY     = -8, // Not empty
    ERR_WRITE_PROTECT = -9, // Write protected SD-card
};

enum {
    FO_RDONLY  = 0x00, // Open for reading only
    FO_WRONLY  = 0x01, // Open for writing only
    FO_RDWR    = 0x02, // Open for reading and writing
    FO_ACCMODE = 0x03, // Mask for above modes

    FO_APPEND = 0x04, // Append mode
    FO_CREATE = 0x08, // Create if non-existant
    FO_TRUNC  = 0x10, // Truncate to zero length
    FO_EXCL   = 0x20, // Error if already exists
};

#define ESP_STATUS_RXNE       (1 << 0)
#define ESP_STATUS_TXF        (1 << 1)
#define ESP_DATA_START_OF_MSG (1 << 8)

#ifdef __cplusplus
}
#endif
