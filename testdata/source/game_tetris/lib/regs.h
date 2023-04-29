#ifndef _REGS_H
#define _REGS_H

// Make code completion happy.
#ifdef __SDCC
#    define IOREG(ioaddr, name) \
        static __sfr __at ioaddr name
#else
#    define IOREG(ioaddr, name) \
        static unsigned char name
#endif

IOREG(0xE0, IO_VCTRL);
IOREG(0xE1, IO_VSCRX_L);
IOREG(0xE2, IO_VSCRX_H);
IOREG(0xE3, IO_VSCRY);
IOREG(0xE4, IO_VSPRSEL);
IOREG(0xE5, IO_VSPRX_L);
IOREG(0xE6, IO_VSPRX_H);
IOREG(0xE7, IO_VSPRY);
IOREG(0xE8, IO_VSPRIDX);
IOREG(0xE9, IO_VSPRATTR);
IOREG(0xEA, IO_VPALSEL);
IOREG(0xEB, IO_VPALDATA);
IOREG(0xEC, IO_VLINE);
IOREG(0xED, IO_VIRQLINE);
IOREG(0xEE, IO_IRQMASK);
IOREG(0xEF, IO_IRQSTAT);
IOREG(0xF0, IO_BANK0);
IOREG(0xF1, IO_BANK1);
IOREG(0xF2, IO_BANK2);
IOREG(0xF3, IO_BANK3);
IOREG(0xF4, IO_ESPCTRL);
IOREG(0xF5, IO_ESPDATA);
IOREG(0xF6, IO_PSG1DATA);
IOREG(0xF7, IO_PSG1ADDR);
IOREG(0xF8, IO_PSG2DATA);
IOREG(0xF9, IO_PSG2ADDR);
IOREG(0xFB, IO_SYSCTRL);
IOREG(0xFC, IO_CASSETTE);
IOREG(0xFD, IO_CPM);
IOREG(0xFD, IO_VSYNC);
IOREG(0xFE, IO_PRINTER);
IOREG(0xFF, IO_SCRAMBLE);
IOREG(0xFF, IO_KEYBOARD);

enum {
    ESPCMD_RESET    = 0x01, // Reset ESP
    ESPCMD_OPEN     = 0x10, // Open / create file
    ESPCMD_CLOSE    = 0x11, // Close open file
    ESPCMD_READ     = 0x12, // Read from file
    ESPCMD_WRITE    = 0x13, // Write to file
    ESPCMD_SEEK     = 0x14, // Move read/write pointer
    ESPCMD_TELL     = 0x15, // Get current read/write
    ESPCMD_OPENDIR  = 0x16, // Open directory
    ESPCMD_CLOSEDIR = 0x17, // Close open directory
    ESPCMD_READDIR  = 0x18, // Read from directory
    ESPCMD_DELETE   = 0x19, // Remove file or directory
    ESPCMD_RENAME   = 0x1A, // Rename / move file or directory
    ESPCMD_MKDIR    = 0x1B, // Create directory
    ESPCMD_CHDIR    = 0x1C, // Change directory
    ESPCMD_STAT     = 0x1D, // Get file status
    ESPCMD_GETCWD   = 0x1E, // Get current working directory
    ESPCMD_CLOSEALL = 0x1F, // Close any open file/directory descriptor
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

enum {
    VCTRL_TEXT_EN   = (1 << 0),
    VCTRL_MODE_OFF  = (0 << 1),
    VCTRL_MODE_TILE = (1 << 1),
    VCTRL_MODE_BM   = (2 << 1),
    VCTRL_SPR_EN    = (1 << 3),
    VCTRL_TEXT_PRIO = (1 << 4),
};

#endif
