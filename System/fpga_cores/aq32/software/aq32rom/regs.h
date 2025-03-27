#pragma once

#include <stdint.h>

struct regs {
    volatile uint32_t ESP_CTRL;
    volatile uint32_t ESP_DATA;
    volatile uint32_t VCTRL;
    volatile uint32_t VSCRX;
    volatile uint32_t VSCRY;
    volatile uint32_t VLINE;
    volatile uint32_t VIRQLINE;
};

#define TRAM     ((uint16_t *)0xFF000000)
#define CHRAM    ((uint8_t *)0xFF100000)
#define IO_VIDEO ((uint8_t *)0xFF300000)
#define PALETTE  ((uint16_t *)0xFF400000)
#define REGS     ((struct regs *)0xFF500000)

enum {
    ESPCMD_RESET       = 0x01, // Reset ESP
    ESPCMD_VERSION     = 0x02, // Get version string
    ESPCMD_GETDATETIME = 0x03, // Get current date/time
    ESPCMD_KEYMODE     = 0x08, // Set keyboard buffer mode
    ESPCMD_GETMOUSE    = 0x0C, // Get mouse state
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

#define VCTRL_TEXT_EN         (1 << 0)
#define VCTRL_GFXMODE_OFF     (0 << 1)
#define VCTRL_GFXMODE_TILE    (1 << 1)
#define VCTRL_GFXMODE_BM1BPP  (2 << 1)
#define VCTRL_GFXMODE_BM4BPP  (3 << 1)
#define VCTRL_SPR_EN          (1 << 3)
#define VCTRL_TEXT_PRIO       (1 << 4)
#define VCTRL_REMAP_BORDER_CH (1 << 5)
#define VCTRL_80COLUMNS       (1 << 6)
#define VCTRL_TEXTPAGE2       (1 << 7)
