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

#ifdef __SDCC
#    define IOREG16(ioaddr, name) \
        static __sfr __banked __at ioaddr name
#else
#    define IOREG16(ioaddr, name) \
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
IOREG(0xEC, IO_PCMDAC);
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
IOREG(0xFA, IO_KEYBUF);
IOREG(0xFB, IO_SYSCTRL);
IOREG(0xFC, IO_CASSETTE);
IOREG(0xFD, IO_CPM);
IOREG(0xFD, IO_VSYNC);
IOREG(0xFE, IO_PRINTER);
IOREG(0xFF, IO_SCRAMBLE);
IOREG(0xFF, IO_KEYBOARD);

IOREG16(0x00FF, IO_KEYBOARD_ALL);
IOREG16(0x7FFF, IO_KEYBOARD_COL7);
IOREG16(0xBFFF, IO_KEYBOARD_COL6);
IOREG16(0xDFFF, IO_KEYBOARD_COL5);
IOREG16(0xEFFF, IO_KEYBOARD_COL4);
IOREG16(0xF7FF, IO_KEYBOARD_COL3);
IOREG16(0xFBFF, IO_KEYBOARD_COL2);
IOREG16(0xFDFF, IO_KEYBOARD_COL1);
IOREG16(0xFEFF, IO_KEYBOARD_COL0);

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

enum {
    VCTRL_TEXT_EN         = (1U << 0), // Text enable
    VCTRL_MODE_OFF        = (0U << 1), // Graphics mode: Disabled
    VCTRL_MODE_TILE       = (1U << 1), // Graphics mode: 64x32 tilemap
    VCTRL_MODE_BM1BPP     = (2U << 1), // Graphics mode: 320x200 bitmapped (1bpp - fg/bg color selectable per 8x8 pixels)
    VCTRL_MODE_BM4BPP     = (3U << 1), // Graphics mode: 160x200 bitmapped (4bpp)
    VCTRL_SPR_EN          = (1U << 3), // Sprites enable
    VCTRL_TEXT_PRIO       = (1U << 4), // Text priority
    VCTRL_REMAP_BORDER_CH = (1U << 5), // Remap border character
    VCTRL_80COLUMNS       = (1U << 6), // 80-columns mode
    VCTRL_TEXTPAGE2       = (1U << 7), // 2nd text page
};

// Aquarius keyboard scancodes
enum {
    KEY_EQUALS    = 0,  // = +
    KEY_BACKSPACE = 1,  // BS Backslash
    KEY_COLON     = 2,  // : *
    KEY_RETURN    = 3,  // Return
    KEY_SEMICOLON = 4,  // ; @
    KEY_PERIOD    = 5,  // . >
    KEY_INSERT    = 6,  // Insert
    KEY_DELETE    = 7,  // Delete
    KEY_MINUS     = 8,  // - _
    KEY_SLASH     = 9,  // / ^
    KEY_0         = 10, // 0 ?
    KEY_P         = 11, // P
    KEY_L         = 12, // L
    KEY_COMMA     = 13, // , <
    KEY_UP        = 14, // Cursor up
    KEY_RIGHT     = 15, // Cursor right
    KEY_9         = 16, // 9 )
    KEY_O         = 17, // O
    KEY_K         = 18, // K
    KEY_M         = 19, // M
    KEY_N         = 20, // N
    KEY_J         = 21, // J
    KEY_LEFT      = 22, // Cursor left
    KEY_DOWN      = 23, // Cursor down
    KEY_8         = 24, // 8 (
    KEY_I         = 25, // I
    KEY_7         = 26, // 7 '
    KEY_U         = 27, // U
    KEY_H         = 28, // H
    KEY_B         = 29, // B
    KEY_HOME      = 30, // Home
    KEY_END       = 31, // End
    KEY_6         = 32, // 6 &
    KEY_Y         = 33, // Y
    KEY_G         = 34, // G
    KEY_V         = 35, // V
    KEY_C         = 36, // C
    KEY_F         = 37, // F
    KEY_PGUP      = 38, // Page Up
    KEY_PGDN      = 39, // Page Down
    KEY_5         = 40, // 5 %
    KEY_T         = 41, // T
    KEY_4         = 42, // 4 $
    KEY_R         = 43, // R
    KEY_D         = 44, // D
    KEY_X         = 45, // X
    KEY_PAUSE     = 46, // Pause/break
    KEY_PRTSCR    = 47, // PrtScr/SysRq
    KEY_3         = 48, // 3 #
    KEY_E         = 49, // E
    KEY_S         = 50, // S
    KEY_Z         = 51, // Z
    KEY_SPACE     = 52, // Space
    KEY_A         = 53, // A
    KEY_MENU      = 54, // Menu key
    KEY_TAB       = 55, // Tab
    KEY_2         = 56, // 2 "
    KEY_W         = 57, // W
    KEY_1         = 58, // 1 !
    KEY_Q         = 59, // Q
    KEY_SHIFT     = 60, // Shift
    KEY_CTRL      = 61, // Ctrl
    KEY_ALT       = 62, // Alt
    KEY_GUI       = 63, // Gui
};

#endif
