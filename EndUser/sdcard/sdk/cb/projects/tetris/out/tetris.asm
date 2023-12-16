; #include "/sdk/cb/inc/common.cb"
; #asm
    include "/sdk/asm/inc/basic_stub.inc"
main:
    jp      _main
; #endasm

; ioport IO_VCTRL    = 0xE0;
; ioport IO_VSCRX_L  = 0xE1;
; ioport IO_VSCRX_H  = 0xE2;
; ioport IO_VSCRY    = 0xE3;
; ioport IO_VSPRSEL  = 0xE4;
; ioport IO_VSPRX_L  = 0xE5;
; ioport IO_VSPRX_H  = 0xE6;
; ioport IO_VSPRY    = 0xE7;
; ioport IO_VSPRIDX  = 0xE8;
; ioport IO_VSPRATTR = 0xE9;
; ioport IO_VPALSEL  = 0xEA;
; ioport IO_VPALDATA = 0xEB;
; ioport IO_VLINE    = 0xEC;
; ioport IO_PCMDAC   = 0xEC;
; ioport IO_VIRQLINE = 0xED;
; ioport IO_IRQMASK  = 0xEE;
; ioport IO_IRQSTAT  = 0xEF;
; ioport IO_BANK0    = 0xF0;
; ioport IO_BANK1    = 0xF1;
; ioport IO_BANK2    = 0xF2;
; ioport IO_BANK3    = 0xF3;
; ioport IO_ESPCTRL  = 0xF4;
; ioport IO_ESPDATA  = 0xF5;
; ioport IO_PSG1DATA = 0xF6;
; ioport IO_PSG1ADDR = 0xF7;
; ioport IO_PSG2DATA = 0xF8;
; ioport IO_PSG2ADDR = 0xF9;
; ioport IO_KEYBUF   = 0xFA;
; ioport IO_SYSCTRL  = 0xFB;
; ioport IO_CASSETTE = 0xFC;
; ioport IO_CPM      = 0xFD;
; ioport IO_VSYNC    = 0xFD;
; ioport IO_PRINTER  = 0xFE;
; ioport IO_SCRAMBLE = 0xFF;
; ioport IO_KEYBOARD = 0xFF;

; ioport IO_KEYBOARD_ALL  = 0x00FF;
; ioport IO_KEYBOARD_COL7 = 0x7FFF;
; ioport IO_KEYBOARD_COL6 = 0xBFFF;
; ioport IO_KEYBOARD_COL5 = 0xDFFF;
; ioport IO_KEYBOARD_COL4 = 0xEFFF;
; ioport IO_KEYBOARD_COL3 = 0xF7FF;
; ioport IO_KEYBOARD_COL2 = 0xFBFF;
; ioport IO_KEYBOARD_COL1 = 0xFDFF;
; ioport IO_KEYBOARD_COL0 = 0xFEFF;

; const char VCTRL_TEXT_EN   = (1 << 0);
; const char VCTRL_MODE_TILE = (1 << 1);
; const char VCTRL_SPR_EN    = (1 << 3);


; // Aquarius keyboard scancodes
; const char KEY_EQUALS    = 0;  // = +
; const char KEY_BACKSPACE = 1;  // BS Backslash
; const char KEY_COLON     = 2;  // : *
; const char KEY_RETURN    = 3;  // Return
; const char KEY_SEMICOLON = 4;  // ; @
; const char KEY_PERIOD    = 5;  // . >
; const char KEY_INSERT    = 6;  // Insert
; const char KEY_DELETE    = 7;  // Delete
; const char KEY_MINUS     = 8;  // - _
; const char KEY_SLASH     = 9;  // / ^
; const char KEY_0         = 10; // 0 ?
; const char KEY_P         = 11; // P
; const char KEY_L         = 12; // L
; const char KEY_COMMA     = 13; // ; <
; const char KEY_UP        = 14; // Cursor up
; const char KEY_RIGHT     = 15; // Cursor right
; const char KEY_9         = 16; // 9 )
; const char KEY_O         = 17; // O
; const char KEY_K         = 18; // K
; const char KEY_M         = 19; // M
; const char KEY_N         = 20; // N
; const char KEY_J         = 21; // J
; const char KEY_LEFT      = 22; // Cursor left
; const char KEY_DOWN      = 23; // Cursor down
; const char KEY_8         = 24; // 8 (
; const char KEY_I         = 25; // I
; const char KEY_7         = 26; // 7 '
; const char KEY_U         = 27; // U
; const char KEY_H         = 28; // H
; const char KEY_B         = 29; // B
; const char KEY_HOME      = 30; // Home
; const char KEY_END       = 31; // End
; const char KEY_6         = 32; // 6 &
; const char KEY_Y         = 33; // Y
; const char KEY_G         = 34; // G
; const char KEY_V         = 35; // V
; const char KEY_C         = 36; // C
; const char KEY_F         = 37; // F
; const char KEY_PGUP      = 38; // Page Up
; const char KEY_PGDN      = 39; // Page Down
; const char KEY_5         = 40; // 5 %
; const char KEY_T         = 41; // T
; const char KEY_4         = 42; // 4 $
; const char KEY_R         = 43; // R
; const char KEY_D         = 44; // D
; const char KEY_X         = 45; // X
; const char KEY_PAUSE     = 46; // Pause/break
; const char KEY_PRTSCR    = 47; // PrtScr/SysRq
; const char KEY_3         = 48; // 3 #
; const char KEY_E         = 49; // E
; const char KEY_S         = 50; // S
; const char KEY_Z         = 51; // Z
; const char KEY_SPACE     = 52; // Space
; const char KEY_A         = 53; // A
; const char KEY_MENU      = 54; // Menu key
; const char KEY_TAB       = 55; // Tab
; const char KEY_2         = 56; // 2 "
; const char KEY_W         = 57; // W
; const char KEY_1         = 58; // 1 !
; const char KEY_Q         = 59; // Q
; const char KEY_SHIFT     = 60; // Shift
; const char KEY_CTRL      = 61; // Ctrl
; const char KEY_ALT       = 62; // Alt
; const char KEY_GUI       = 63; // Gui


; // Playfield dimensions
; const int PLAYFIELD_W  = 10;
; const int PLAYFIELD_H  = 20;
; const int PLAYFIELD_YT = 2;
; const int PLAYFIELD_YB = (PLAYFIELD_YT + PLAYFIELD_H - 1);
; const int PLAYFIELD_XL = 11;
; const int PLAYFIELD_XR = (PLAYFIELD_XL + PLAYFIELD_W - 1);

; const int MARKER_TILE      = 1;
; const int DELAY_LEFT_RIGHT = 5;
; const int DELAY_DOWN       = 3;

; // Tetrominoes enum
; const int TM_I     = 0;
; const int TM_J     = 1;
; const int TM_L     = 2;
; const int TM_O     = 3;
; const int TM_S     = 4;
; const int TM_T     = 5;
; const int TM_Z     = 6;
; const int TM_TOTAL = 7;

; // Tile data
; extern char tile_palette[];
; extern char tile_data[];
; extern char tile_data_end[];

; // VRAM pointers
; const int  *vram_tilemap  = 0xC000;
; const char *vram_tiledata = 0xE000;

; // Game statistics
; int  score;
_score:
    defw 0
; char level;
_level:
    defb 0
; int  lines;
_lines:
    defw 0
; char gameover;
_gameover:
    defb 0
; char quit;
_quit:
    defb 0

; // Background animation variables
; char *bgtiles_dst;
_bgtiles_dst:
    defw 0
; char *bgtiles_src;
_bgtiles_src:
    defw 0
; char *bgtiles_src2;
_bgtiles_src2:
    defw 0
; char  bgtiles_idx = 0;
_bgtiles_idx:
    defb 0
; char  bgdelay     = 0;
_bgdelay:
    defb 0

; // Current tetromino variables
; char cur_tetromino;
_cur_tetromino:
    defb 0
; char cur_rot;
_cur_rot:
    defb 0
; char cur_posx;
_cur_posx:
    defb 0
; char cur_posy;
_cur_posy:
    defb 0

; // Current background tile
; char bg_tile = 13;
_bg_tile:
    defb 13

; // Pressed keys array
; char pressed_keys[8];
_pressed_keys:
    defs 8

; // Next tetromino (shown in preview area)
; char next_tetromino;
_next_tetromino:
    defb 0

; // Random selection for next tetromino
; char tetromino_random = 0;
_tetromino_random:
    defb 0

; // Temporary string variables for drawing stats
; char tmpstr[16];
_tmpstr:
    defs 16
; char tmpstr2[16];
_tmpstr2:
    defs 16

; // Level speed curve (number of frames delay per gravity drop)
; char speed_curve[21] = {
_speed_curve:
;     52, 48, 44, 40, 36, 32, 27, 21, 16, 10,
    defb 52
    defb 48
    defb 44
    defb 40
    defb 36
    defb 32
    defb 27
    defb 21
    defb 16
    defb 10
;     9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2};
    defb 9
    defb 8
    defb 7
    defb 6
    defb 5
    defb 5
    defb 4
    defb 4
    defb 3
    defb 3
    defb 2

; // clang-format off
; int gameover_playfield[20 * 10] = {
_gameover_playfield:
;     0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
;     0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
;     0x09, 0x4A, 0x4B, 0x4C, 0x4C, 0x4C, 0x4C, 0x24B, 0x24A, 0x09,
    defw 9
    defw 74
    defw 75
    defw 76
    defw 76
    defw 76
    defw 76
    defw 587
    defw 586
    defw 9
;     0x09, 0x6A, 0x6B, 0x6C, 0x6C, 0x6C, 0x6C, 0x26B, 0x26A, 0x09,
    defw 9
    defw 106
    defw 107
    defw 108
    defw 108
    defw 108
    defw 108
    defw 619
    defw 618
    defw 9
;     0x09, 0x6D, 0x6E, 0x0F + 'G', 0x0F + 'A', 0x0F + 'M', 0x0F + 'E', 0x26E, 0x26D, 0x09,
    defw 9
    defw 109
    defw 110
    defw 86
    defw 80
    defw 92
    defw 84
    defw 622
    defw 621
    defw 9
;     0x09, 0x6D, 0x6E, 0x01, 0x01, 0x01, 0x01, 0x26E, 0x26D, 0x09,
    defw 9
    defw 109
    defw 110
    defw 1
    defw 1
    defw 1
    defw 1
    defw 622
    defw 621
    defw 9
;     0x09, 0x6D, 0x6E, 0x0F + 'O', 0x0F + 'V', 0x0F + 'E', 0x0F + 'R', 0x26E, 0x26D, 0x09,
    defw 9
    defw 109
    defw 110
    defw 94
    defw 101
    defw 84
    defw 97
    defw 622
    defw 621
    defw 9
;     0x09, 0x46A, 0x46B, 0x46C, 0x46C, 0x46C, 0x46C, 0x66B, 0x66A, 0x09,
    defw 9
    defw 1130
    defw 1131
    defw 1132
    defw 1132
    defw 1132
    defw 1132
    defw 1643
    defw 1642
    defw 9
;     0x09, 0x44A, 0x44B, 0x44C, 0x44C, 0x44C, 0x44C, 0x64B, 0x64A, 0x09,
    defw 9
    defw 1098
    defw 1099
    defw 1100
    defw 1100
    defw 1100
    defw 1100
    defw 1611
    defw 1610
    defw 9
;     0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
;     0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
    defw 9
;     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
;     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
;     0x01, 0x01, 0x0F + 'P', 0x0F + 'L', 0x0F + 'E', 0x0F + 'A', 0x0F + 'S', 0x0F + 'E', 0x01, 0x01,
    defw 1
    defw 1
    defw 95
    defw 91
    defw 84
    defw 80
    defw 98
    defw 84
    defw 1
    defw 1
;     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
;     0x01, 0x01, 0x01, 0x0F + 'T', 0x0F + 'R', 0x0F + 'Y', 0x01, 0x01, 0x01, 0x01,
    defw 1
    defw 1
    defw 1
    defw 99
    defw 97
    defw 104
    defw 1
    defw 1
    defw 1
    defw 1
;     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
;     0x01, 0x01, 0x0F + 'A', 0x0F + 'G', 0x0F + 'A', 0x0F + 'I', 0x0F + 'N', 0x4F, 0x01, 0x01,
    defw 1
    defw 1
    defw 80
    defw 86
    defw 80
    defw 88
    defw 93
    defw 79
    defw 1
    defw 1
;     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
;     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
    defw 1
; };
    defw 1
; // clang-format on

; // Keyboard bitmap
; const int KBM_LEFT       = (1 << 0);
; const int KBM_RIGHT      = (1 << 1);
; const int KBM_UP         = (1 << 2);
; const int KBM_DOWN       = (1 << 3);
; const int KBM_ROTATE_CCW = (1 << 4);
; const int KBM_ROTATE_CW  = (1 << 5);

; char kb_pressing_prev[8];
_kb_pressing_prev:
    defs 8
; char kb_pressing_keys[8];
_kb_pressing_keys:
    defs 8

; // clang-format off
; // Tetromino I shape in 4 rotations
; char tetromino_i[4 * 16] = {
_tetromino_i:
;     // Rotation 0
;      0,  0,  0,  0,
    defb 0
    defb 0
    defb 0
    defb 0
;     16, 17, 17, 18,
    defb 16
    defb 17
    defb 17
    defb 18
;      0,  0,  0,  0,
    defb 0
    defb 0
    defb 0
    defb 0
;      0,  0,  0,  0,
    defb 0
    defb 0
    defb 0
    defb 0
;     // Rotation 1
;      0,  0, 19,  0,
    defb 0
    defb 0
    defb 19
    defb 0
;      0,  0, 20,  0,
    defb 0
    defb 0
    defb 20
    defb 0
;      0,  0, 20,  0,
    defb 0
    defb 0
    defb 20
    defb 0
;      0,  0, 21,  0,
    defb 0
    defb 0
    defb 21
    defb 0
;     // Rotation 2
;      0,  0,  0,  0,
    defb 0
    defb 0
    defb 0
    defb 0
;      0,  0,  0,  0,
    defb 0
    defb 0
    defb 0
    defb 0
;     16, 17, 17, 18,
    defb 16
    defb 17
    defb 17
    defb 18
;      0,  0,  0,  0,
    defb 0
    defb 0
    defb 0
    defb 0
;     // Rotation 3
;      0, 19,  0,  0,
    defb 0
    defb 19
    defb 0
    defb 0
;      0, 20,  0,  0,
    defb 0
    defb 20
    defb 0
    defb 0
;      0, 20,  0,  0,
    defb 0
    defb 20
    defb 0
    defb 0
;      0, 21,  0,  0
    defb 0
    defb 21
    defb 0
; };
    defb 0

; // Tetromino J shape in 4 rotations
; char tetromino_j[4 * 9] = {
_tetromino_j:
;     // Rotation 0
;     25,  0,  0,
    defb 25
    defb 0
    defb 0
;     25, 25, 25,
    defb 25
    defb 25
    defb 25
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     // Rotation 1
;      0, 25, 25,
    defb 0
    defb 25
    defb 25
;      0, 25,  0,
    defb 0
    defb 25
    defb 0
;      0, 25,  0,
    defb 0
    defb 25
    defb 0
;     // Rotation 2
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     25, 25, 25,
    defb 25
    defb 25
    defb 25
;      0,  0, 25,
    defb 0
    defb 0
    defb 25
;     // Rotation 3
;      0, 25,  0,
    defb 0
    defb 25
    defb 0
;      0, 25,  0,
    defb 0
    defb 25
    defb 0
;     25, 25,  0
    defb 25
    defb 25
; };
    defb 0

; // Tetromino L shape in 4 rotations
; char tetromino_l[4 * 9] = {
_tetromino_l:
;     // Rotation 0
;      0,  0, 26,
    defb 0
    defb 0
    defb 26
;     26, 26, 26,
    defb 26
    defb 26
    defb 26
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     // Rotation 1
;      0, 26,  0,
    defb 0
    defb 26
    defb 0
;      0, 26,  0,
    defb 0
    defb 26
    defb 0
;      0, 26, 26,
    defb 0
    defb 26
    defb 26
;     // Rotation 2
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     26, 26, 26,
    defb 26
    defb 26
    defb 26
;     26,  0,  0,
    defb 26
    defb 0
    defb 0
;     // Rotation 3
;     26, 26,  0,
    defb 26
    defb 26
    defb 0
;      0, 26,  0,
    defb 0
    defb 26
    defb 0
;      0, 26,  0
    defb 0
    defb 26
; };
    defb 0

; // Tetromino O shape (just 1 rotation)
; char tetromino_o[4] = {
_tetromino_o:
;     27, 27,
    defb 27
    defb 27
;     27, 27
    defb 27
; };
    defb 27

; // Tetromino S shape in 4 rotations
; char tetromino_s[4 * 9] = {
_tetromino_s:
;     // Rotation 0
;      0, 24, 24,
    defb 0
    defb 24
    defb 24
;     24, 24,  0,
    defb 24
    defb 24
    defb 0
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     // Rotation 1
;      0, 24,  0,
    defb 0
    defb 24
    defb 0
;      0, 24, 24,
    defb 0
    defb 24
    defb 24
;      0,  0, 24,
    defb 0
    defb 0
    defb 24
;     // Rotation 2
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;      0, 24, 24,
    defb 0
    defb 24
    defb 24
;     24, 24,  0,
    defb 24
    defb 24
    defb 0
;     // Rotation 3
;     24,  0,  0,
    defb 24
    defb 0
    defb 0
;     24, 24,  0,
    defb 24
    defb 24
    defb 0
;      0, 24,  0
    defb 0
    defb 24
; };
    defb 0

; // Tetromino T shape in 4 rotations
; char tetromino_t[4 * 9] = {
_tetromino_t:
;     // Rotation 0
;      0, 22,  0,
    defb 0
    defb 22
    defb 0
;     22, 22, 22,
    defb 22
    defb 22
    defb 22
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     // Rotation 1
;      0, 22,  0,
    defb 0
    defb 22
    defb 0
;      0, 22, 22,
    defb 0
    defb 22
    defb 22
;      0, 22,  0,
    defb 0
    defb 22
    defb 0
;     // Rotation 2
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     22, 22, 22,
    defb 22
    defb 22
    defb 22
;      0, 22,  0,
    defb 0
    defb 22
    defb 0
;     // Rotation 3
;      0, 22,  0,
    defb 0
    defb 22
    defb 0
;     22, 22,  0,
    defb 22
    defb 22
    defb 0
;      0, 22,  0
    defb 0
    defb 22
; };
    defb 0

; // Tetromino Z shape in 4 rotations
; char tetromino_z[4 * 9] = {
_tetromino_z:
;     // Rotation 0
;     23, 23,  0,
    defb 23
    defb 23
    defb 0
;      0, 23, 23,
    defb 0
    defb 23
    defb 23
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     // Rotation 1
;      0,  0, 23,
    defb 0
    defb 0
    defb 23
;      0, 23, 23,
    defb 0
    defb 23
    defb 23
;      0, 23,  0,
    defb 0
    defb 23
    defb 0
;     // Rotation 2
;      0,  0,  0,
    defb 0
    defb 0
    defb 0
;     23, 23,  0,
    defb 23
    defb 23
    defb 0
;      0, 23, 23,
    defb 0
    defb 23
    defb 23
;     // Rotation 3
;      0, 23,  0,
    defb 0
    defb 23
    defb 0
;     23, 23,  0,
    defb 23
    defb 23
    defb 0
;     23,  0,  0
    defb 23
    defb 0
; };
    defb 0

; // Offsets used for rotation
; int wall_kick_offsets_jlstz_x[8 * 5] = {
_wall_kick_offsets_jlstz_x:
;     0, +1, +1, 0, +1, // CCW:0->L
    defw 0
    defw 1
    defw 1
    defw 0
    defw 1
;     0, -1, -1, 0, -1, // CW :0->R
    defw 0
    defw -1
    defw -1
    defw 0
    defw -1
;     0, +1, +1, 0, +1, // CCW:R->0
    defw 0
    defw 1
    defw 1
    defw 0
    defw 1
;     0, +1, +1, 0, +1, // CW :R->2
    defw 0
    defw 1
    defw 1
    defw 0
    defw 1
;     0, -1, -1, 0, -1, // CCW:2->R
    defw 0
    defw -1
    defw -1
    defw 0
    defw -1
;     0, +1, +1, 0, +1, // CW :2->L
    defw 0
    defw 1
    defw 1
    defw 0
    defw 1
;     0, -1, -1, 0, -1, // CCW:L->2
    defw 0
    defw -1
    defw -1
    defw 0
    defw -1
;     0, -1, -1, 0, -1  // CW :L->0
    defw 0
    defw -1
    defw -1
    defw 0
; };
    defw -1
; int wall_kick_offsets_jlstz_y[8 * 5] = {
_wall_kick_offsets_jlstz_y:
;     0, 0, +1, -2, -2, // CCW:0->L
    defw 0
    defw 0
    defw 1
    defw -2
    defw -2
;     0, 0, +1, -2, -2, // CW :0->R
    defw 0
    defw 0
    defw 1
    defw -2
    defw -2
;     0, 0, -1, +2, +2, // CCW:R->0
    defw 0
    defw 0
    defw -1
    defw 2
    defw 2
;     0, 0, -1, +2, +2, // CW :R->2
    defw 0
    defw 0
    defw -1
    defw 2
    defw 2
;     0, 0, +1, -2, -2, // CCW:2->R
    defw 0
    defw 0
    defw 1
    defw -2
    defw -2
;     0, 0, +1, -2, -2, // CW :2->L
    defw 0
    defw 0
    defw 1
    defw -2
    defw -2
;     0, 0, -1, +2, +2, // CCW:L->2
    defw 0
    defw 0
    defw -1
    defw 2
    defw 2
;     0, 0, -1, +2, +2  // CW :L->0
    defw 0
    defw 0
    defw -1
    defw 2
; };
    defw 2

; // Offsets used for rotation
; int wall_kick_offsets_i_x[8 * 5] = {
_wall_kick_offsets_i_x:
;     0, -1, +2, -1, +2, // CCW:0->L
    defw 0
    defw -1
    defw 2
    defw -1
    defw 2
;     0, -2, +1, -2, +1, // CW :0->R
    defw 0
    defw -2
    defw 1
    defw -2
    defw 1
;     0, +2, -1, +2, -1, // CCW:R->0
    defw 0
    defw 2
    defw -1
    defw 2
    defw -1
;     0, -1, +2, -1, +2, // CW :R->2
    defw 0
    defw -1
    defw 2
    defw -1
    defw 2
;     0, +1, -2, +1, -2, // CCW:2->R
    defw 0
    defw 1
    defw -2
    defw 1
    defw -2
;     0, +2, -1, +2, -1, // CW :2->L
    defw 0
    defw 2
    defw -1
    defw 2
    defw -1
;     0, -2, +1, -2, +1, // CCW:L->2
    defw 0
    defw -2
    defw 1
    defw -2
    defw 1
;     0, +1, -2, +1, -2  // CW :L->0
    defw 0
    defw 1
    defw -2
    defw 1
; };
    defw -2
; int wall_kick_offsets_i_y[8 * 5] = {
_wall_kick_offsets_i_y:
;     0, 0, 0, +2, -1, // CCW:0->L
    defw 0
    defw 0
    defw 0
    defw 2
    defw -1
;     0, 0, 0, -1, +2, // CW :0->R
    defw 0
    defw 0
    defw 0
    defw -1
    defw 2
;     0, 0, 0, +1, -2, // CCW:R->0
    defw 0
    defw 0
    defw 0
    defw 1
    defw -2
;     0, 0, 0, +2, -1, // CW :R->2
    defw 0
    defw 0
    defw 0
    defw 2
    defw -1
;     0, 0, 0, -2, +1, // CCW:2->R
    defw 0
    defw 0
    defw 0
    defw -2
    defw 1
;     0, 0, 0, +1, -2, // CW :2->L
    defw 0
    defw 0
    defw 0
    defw 1
    defw -2
;     0, 0, 0, -1, +2, // CCW:L->2
    defw 0
    defw 0
    defw 0
    defw -1
    defw 2
;     0, 0, 0, -2, +1  // CW :L->0
    defw 0
    defw 0
    defw 0
    defw -2
; };
    defw 1

; // clang-format on

; kb_scan() {
_kb_scan:
    push    ix
    ld      ix,0
    add     ix,sp
;     char i = 0;
    ld      hl,0
    push    hl
;     while (i < 8) {
.l0:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l1
;         kb_pressing_prev[i] = kb_pressing_keys[i];
    ld      hl,_kb_pressing_keys
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    push    hl
    ld      hl,_kb_pressing_prev
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;         i                   = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l0
.l1:
;     kb_pressing_keys[0] = ~IO_KEYBOARD_COL0;
    ld      b,$FE
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[1] = ~IO_KEYBOARD_COL1;
    ld      b,$FD
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    inc     hl
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[2] = ~IO_KEYBOARD_COL2;
    ld      b,$FB
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[3] = ~IO_KEYBOARD_COL3;
    ld      b,$F7
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[4] = ~IO_KEYBOARD_COL4;
    ld      b,$EF
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[5] = ~IO_KEYBOARD_COL5;
    ld      b,$DF
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,5
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[6] = ~IO_KEYBOARD_COL6;
    ld      b,$BF
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,6
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     kb_pressing_keys[7] = ~IO_KEYBOARD_COL7;
    ld      b,$7F
    ld      c,$FF
    in      a,(c)
    ld      h,0
    ld      l,a
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; kb_pressing(char scancode) {
_kb_pressing:
    push    ix
    ld      ix,0
    add     ix,sp
;     return (kb_pressing_keys[scancode / 8] & (1 << (scancode & 7))) != 0;
    ld      hl,_kb_pressing_keys
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    call    __divsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    push    hl
    ld      hl,1
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ex      de,hl
    pop     hl
    call    __shl
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jr      z,.l2
    ld      hl,1
.l2:
    jp      .return
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; kb_pressed(char scancode) {
_kb_pressed:
    push    ix
    ld      ix,0
    add     ix,sp
;     return ((~kb_pressing_prev[scancode / 8] & kb_pressing_keys[scancode / 8]) & (1 << (scancode & 7))) != 0;
    ld      hl,_kb_pressing_prev
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    call    __divsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      hl,_kb_pressing_keys
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    call    __divsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    push    hl
    ld      hl,1
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ex      de,hl
    pop     hl
    call    __shl
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jr      z,.l3
    ld      hl,1
.l3:
    jp      .return
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; video_wait_line(char linenr) {
_video_wait_line:
    push    ix
    ld      ix,0
    add     ix,sp
;     IO_VIRQLINE = linenr;
    ld      h,0
    ld      l,(ix+4)
    ld      b,$00
    ld      c,$ED
    ld      a,l
    out     (c),a
;     IO_IRQSTAT  = 2;
    ld      hl,2
    ld      b,$00
    ld      c,$EF
    ld      a,l
    out     (c),a
;     while ((IO_IRQSTAT & 2) == 0) {
.l4:
    ld      b,$00
    ld      c,$EF
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l6
    ld      hl,0
    jr      .l7
.l6:
    inc     hl
.l7:
    ld      a,h
    or      l
    jp      z,.l5
;     }
    jp      .l4
.l5:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Set tile 'tile_idx' at position i,j
; set_tile(char i, char j, char tile_idx) {
_set_tile:
    push    ix
    ld      ix,0
    add     ix,sp
;     vram_tilemap[(j << 6) | i] = 0x1100 | tile_idx;
    ld      hl,4352
    push    hl
    ld      h,0
    ld      l,(ix+8)
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    push    hl
    ld      hl,-16384
    push    hl
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      hl,6
    ex      de,hl
    pop     hl
    call    __shl
    push    hl
    ld      h,0
    ld      l,(ix+4)
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    pop     de
    ld      (hl),e
    inc     hl
    ld      (hl),d
    ex      de,hl
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; set_tile2(char i, char j, int val) {
_set_tile2:
    push    ix
    ld      ix,0
    add     ix,sp
;     vram_tilemap[(j << 6) | i] = 0x1100 | val;
    ld      hl,4352
    push    hl
    ld      l,(ix+8)
    ld      h,(ix+9)
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    push    hl
    ld      hl,-16384
    push    hl
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      hl,6
    ex      de,hl
    pop     hl
    call    __shl
    push    hl
    ld      h,0
    ld      l,(ix+4)
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    pop     de
    ld      (hl),e
    inc     hl
    ld      (hl),d
    ex      de,hl
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Get tile at position i,j
; get_tile(char i, char j) {
_get_tile:
    push    ix
    ld      ix,0
    add     ix,sp
;     if (j < PLAYFIELD_YT && (i >= PLAYFIELD_XL && i <= PLAYFIELD_XR))
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jr      z,.l8
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,11
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jr      z,.l10
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __les
    ld      a,h
    or      l
.l10:
    ld      hl,0
    jr      z,.l11
    inc     l
.l11:
    ld      a,h
    or      l
.l8:
    ld      hl,0
    jr      z,.l9
    inc     l
.l9:
    ld      a,h
    or      l
    jp      z,.l12
;         return bg_tile;
    ld      a,(_bg_tile)
    ld      h,0
    ld      l,a
    jp      .return

;     return vram_tilemap[(j << 6) | i] & 0xFF;
.l12:
    ld      hl,-16384
    push    hl
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      hl,6
    ex      de,hl
    pop     hl
    call    __shl
    push    hl
    ld      h,0
    ld      l,(ix+4)
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      e,(hl)
    inc     hl
    ld      d,(hl)
    ex      de,hl
    push    hl
    ld      hl,255
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    jp      .return
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // This function will draw the given tetromino at coordinate i,j. If check
; // is set, instead of drawing the function will return if the tetromino
; // can be drawn without intersecting with existing blocks
; // (returns 0 on collision).
; draw_tetromino(char i, char j, char tetromino, char rot, char check, char lock) {
_draw_tetromino:
    push    ix
    ld      ix,0
    add     ix,sp
;     rot      = rot & 3;
    ld      h,0
    ld      l,(ix+10)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      (ix+10),l
;     char *p  = 0;
    ld      hl,0
    push    hl
;     char  sz = 0;
    ld      hl,0
    push    hl

;     if (tetromino == TM_O) {
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l13
    ld      hl,0
    jr      .l14
.l13:
    inc     hl
.l14:
    ld      a,h
    or      l
    jp      z,.l15
;         sz = 2;
    ld      hl,2
    ld      (ix+-4),l
;         p  = tetromino_o;
    ld      hl,_tetromino_o
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_J) {
    jp      .l16
.l15:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l17
    ld      hl,0
    jr      .l18
.l17:
    inc     hl
.l18:
    ld      a,h
    or      l
    jp      z,.l19
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_j + 9 * rot;
    ld      hl,_tetromino_j
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_L) {
    jp      .l20
.l19:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l21
    ld      hl,0
    jr      .l22
.l21:
    inc     hl
.l22:
    ld      a,h
    or      l
    jp      z,.l23
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_l + 9 * rot;
    ld      hl,_tetromino_l
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_S) {
    jp      .l24
.l23:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l25
    ld      hl,0
    jr      .l26
.l25:
    inc     hl
.l26:
    ld      a,h
    or      l
    jp      z,.l27
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_s + 9 * rot;
    ld      hl,_tetromino_s
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_T) {
    jp      .l28
.l27:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,5
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l29
    ld      hl,0
    jr      .l30
.l29:
    inc     hl
.l30:
    ld      a,h
    or      l
    jp      z,.l31
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_t + 9 * rot;
    ld      hl,_tetromino_t
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_Z) {
    jp      .l32
.l31:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,6
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l33
    ld      hl,0
    jr      .l34
.l33:
    inc     hl
.l34:
    ld      a,h
    or      l
    jp      z,.l35
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_z + 9 * rot;
    ld      hl,_tetromino_z
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_I) {
    jp      .l36
.l35:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l37
    ld      hl,0
    jr      .l38
.l37:
    inc     hl
.l38:
    ld      a,h
    or      l
    jp      z,.l39
;         sz = 4;
    ld      hl,4
    ld      (ix+-4),l
;         p  = tetromino_i + 16 * rot;
    ld      hl,_tetromino_i
    push    hl
    ld      hl,16
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     }

;     if (sz == 3 || sz == 4) {
.l39:
.l36:
.l32:
.l28:
.l24:
.l20:
.l16:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l42
    ld      hl,0
    jr      .l43
.l42:
    inc     hl
.l43:
    ld      a,h
    or      l
    jr      nz,.l40
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l44
    ld      hl,0
    jr      .l45
.l44:
    inc     hl
.l45:
    ld      a,h
    or      l
.l40:
    ld      hl,1
    jr      nz,.l41
    dec     l
.l41:
    ld      a,h
    or      l
    jp      z,.l46
;         i = i - 1;
    ld      h,0
    ld      l,(ix+4)
    dec     hl
    ld      (ix+4),l
;         j = j - 1;
    ld      h,0
    ld      l,(ix+6)
    dec     hl
    ld      (ix+6),l
;     }

;     if (check) {
.l46:
    ld      h,0
    ld      l,(ix+12)
    ld      a,h
    or      l
    jp      z,.l47
;         char y = 0;
    ld      hl,0
    push    hl
;         char x;
    push    af

;         while (y < sz) {
.l48:
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l49
;             x = 0;
    ld      hl,0
    ld      (ix+-8),l
;             while (x < sz) {
.l50:
    ld      h,0
    ld      l,(ix+-8)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l51
;                 char val = *p;
    ld      l,(ix+-2)
    ld      h,(ix+-1)
    ld      a,(hl)
    ld      l,a
    ld      h,0
    push    hl
;                 p        = p + 1;
    ld      l,(ix+-2)
    ld      h,(ix+-1)
    inc     hl
    ld      (ix+-2),l
    ld      (ix+-1),h
;                 if (val && get_tile(i + x, j + y) != bg_tile)
    ld      h,0
    ld      l,(ix+-10)
    ld      a,h
    or      l
    jr      z,.l52
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      h,0
    ld      l,(ix+-6)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      h,0
    ld      l,(ix+-8)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _get_tile
    pop     af
    pop     af
    push    hl
    ld      a,(_bg_tile)
    ld      h,0
    ld      l,a
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jr      z,.l54
    ld      hl,1
.l54:
    ld      a,h
    or      l
.l52:
    ld      hl,0
    jr      z,.l53
    inc     l
.l53:
    ld      a,h
    or      l
    jp      z,.l55
;                     return 0;
    ld      hl,0
    jp      .return
;                 x = x + 1;
.l55:
    ld      h,0
    ld      l,(ix+-8)
    inc     hl
    ld      (ix+-8),l
;             }
    pop     af
    jp      .l50
.l51:
;             y = y + 1;
    ld      h,0
    ld      l,(ix+-6)
    inc     hl
    ld      (ix+-6),l
;         }
    jp      .l48
.l49:
;         return 1;
    ld      hl,1
    jp      .return

;     } else {
    pop     af
    pop     af
    jp      .l56
.l47:
;         char y = 0;
    ld      hl,0
    push    hl
;         char x;
    push    af

;         while (y < sz) {
.l57:
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l58
;             x = 0;
    ld      hl,0
    ld      (ix+-8),l
;             while (x < sz) {
.l59:
    ld      h,0
    ld      l,(ix+-8)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l60
;                 char val = *p;
    ld      l,(ix+-2)
    ld      h,(ix+-1)
    ld      a,(hl)
    ld      l,a
    ld      h,0
    push    hl
;                 p        = p + 1;
    ld      l,(ix+-2)
    ld      h,(ix+-1)
    inc     hl
    ld      (ix+-2),l
    ld      (ix+-1),h
;                 if (val) {
    ld      h,0
    ld      l,(ix+-10)
    ld      a,h
    or      l
    jp      z,.l61
;                     char jy = j + y;
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      h,0
    ld      l,(ix+-6)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
;                     if (jy < PLAYFIELD_YT) {
    ld      h,0
    ld      l,(ix+-12)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l62
;                         return 0;
    ld      hl,0
    jp      .return
;                     }
;                     if (lock)
.l62:
    ld      h,0
    ld      l,(ix+14)
    ld      a,h
    or      l
    jp      z,.l63
;                         val = MARKER_TILE;
    ld      hl,1
    ld      (ix+-10),l
;                     set_tile(i + x, jy, val);
.l63:
    ld      h,0
    ld      l,(ix+-10)
    push    hl
    ld      h,0
    ld      l,(ix+-12)
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      h,0
    ld      l,(ix+-8)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;                 }
    pop     af
;                 x = x + 1;
.l61:
    ld      h,0
    ld      l,(ix+-8)
    inc     hl
    ld      (ix+-8),l
;             }
    pop     af
    jp      .l59
.l60:
;             y = y + 1;
    ld      h,0
    ld      l,(ix+-6)
    inc     hl
    ld      (ix+-6),l
;         }
    jp      .l57
.l58:
;         return 1;
    ld      hl,1
    jp      .return
;     }
    pop     af
    pop     af
.l56:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Update sprites used to display moving tetromino
; set_tetromino_sprites(char x, char y, char tetromino, char rot) {
_set_tetromino_sprites:
    push    ix
    ld      ix,0
    add     ix,sp
;     rot      = rot & 3;
    ld      h,0
    ld      l,(ix+10)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      (ix+10),l
;     char *p  = 0;
    ld      hl,0
    push    hl
;     char  sz = 0;
    ld      hl,0
    push    hl

;     if (tetromino == TM_O) {
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l64
    ld      hl,0
    jr      .l65
.l64:
    inc     hl
.l65:
    ld      a,h
    or      l
    jp      z,.l66
;         sz = 2;
    ld      hl,2
    ld      (ix+-4),l
;         p  = tetromino_o;
    ld      hl,_tetromino_o
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_J) {
    jp      .l67
.l66:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l68
    ld      hl,0
    jr      .l69
.l68:
    inc     hl
.l69:
    ld      a,h
    or      l
    jp      z,.l70
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_j + 9 * rot;
    ld      hl,_tetromino_j
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_L) {
    jp      .l71
.l70:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l72
    ld      hl,0
    jr      .l73
.l72:
    inc     hl
.l73:
    ld      a,h
    or      l
    jp      z,.l74
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_l + 9 * rot;
    ld      hl,_tetromino_l
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_S) {
    jp      .l75
.l74:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l76
    ld      hl,0
    jr      .l77
.l76:
    inc     hl
.l77:
    ld      a,h
    or      l
    jp      z,.l78
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_s + 9 * rot;
    ld      hl,_tetromino_s
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_T) {
    jp      .l79
.l78:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,5
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l80
    ld      hl,0
    jr      .l81
.l80:
    inc     hl
.l81:
    ld      a,h
    or      l
    jp      z,.l82
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_t + 9 * rot;
    ld      hl,_tetromino_t
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_Z) {
    jp      .l83
.l82:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,6
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l84
    ld      hl,0
    jr      .l85
.l84:
    inc     hl
.l85:
    ld      a,h
    or      l
    jp      z,.l86
;         sz = 3;
    ld      hl,3
    ld      (ix+-4),l
;         p  = tetromino_z + 9 * rot;
    ld      hl,_tetromino_z
    push    hl
    ld      hl,9
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     } else if (tetromino == TM_I) {
    jp      .l87
.l86:
    ld      h,0
    ld      l,(ix+8)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l88
    ld      hl,0
    jr      .l89
.l88:
    inc     hl
.l89:
    ld      a,h
    or      l
    jp      z,.l90
;         sz = 4;
    ld      hl,4
    ld      (ix+-4),l
;         p  = tetromino_i + 16 * rot;
    ld      hl,_tetromino_i
    push    hl
    ld      hl,16
    push    hl
    ld      h,0
    ld      l,(ix+10)
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-2),l
    ld      (ix+-1),h
;     }

;     if (sz == 3 || sz == 4) {
.l90:
.l87:
.l83:
.l79:
.l75:
.l71:
.l67:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l93
    ld      hl,0
    jr      .l94
.l93:
    inc     hl
.l94:
    ld      a,h
    or      l
    jr      nz,.l91
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l95
    ld      hl,0
    jr      .l96
.l95:
    inc     hl
.l96:
    ld      a,h
    or      l
.l91:
    ld      hl,1
    jr      nz,.l92
    dec     l
.l92:
    ld      a,h
    or      l
    jp      z,.l97
;         x = x - 8;
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    ld      (ix+4),l
;         y = y - 8;
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    ld      (ix+6),l
;     }

;     IO_VSPRSEL = 0;
.l97:
    ld      hl,0
    ld      b,$00
    ld      c,$E4
    ld      a,l
    out     (c),a

;     char j = 0;
    ld      hl,0
    push    hl
;     char i;
    push    af
;     char val;
    push    af
;     int  sx;
    push    af

;     while (j < sz) {
.l98:
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l99
;         i = 0;
    ld      hl,0
    ld      (ix+-8),l
;         while (i < sz) {
.l100:
    ld      h,0
    ld      l,(ix+-8)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l101
;             val = *p;
    ld      l,(ix+-2)
    ld      h,(ix+-1)
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ld      (ix+-10),l
;             p   = p + 1;
    ld      l,(ix+-2)
    ld      h,(ix+-1)
    inc     hl
    ld      (ix+-2),l
    ld      (ix+-1),h
;             if (val) {
    ld      h,0
    ld      l,(ix+-10)
    ld      a,h
    or      l
    jp      z,.l102
;                 sx          = x + (i << 3);
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      h,0
    ld      l,(ix+-8)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    call    __shl
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-12),l
    ld      (ix+-11),h
;                 IO_VSPRX_L  = sx & 0xFF;
    ld      l,(ix+-12)
    ld      h,(ix+-11)
    push    hl
    ld      hl,255
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      b,$00
    ld      c,$E5
    ld      a,l
    out     (c),a
;                 IO_VSPRX_H  = sx >> 8;
    ld      l,(ix+-12)
    ld      h,(ix+-11)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    call    __shr
    ld      b,$00
    ld      c,$E6
    ld      a,l
    out     (c),a
;                 IO_VSPRY    = y + (j << 3);
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    call    __shl
    ex      de,hl
    pop     hl
    add     hl,de
    ld      b,$00
    ld      c,$E7
    ld      a,l
    out     (c),a
;                 IO_VSPRIDX  = val;
    ld      h,0
    ld      l,(ix+-10)
    ld      b,$00
    ld      c,$E8
    ld      a,l
    out     (c),a
;                 IO_VSPRATTR = 0x91;
    ld      hl,145
    ld      b,$00
    ld      c,$E9
    ld      a,l
    out     (c),a
;                 IO_VSPRSEL  = IO_VSPRSEL + 1;
    ld      b,$00
    ld      c,$E4
    in      a,(c)
    ld      h,0
    ld      l,a
    inc     hl
    ld      b,$00
    ld      c,$E4
    ld      a,l
    out     (c),a
;             }
;             i = i + 1;
.l102:
    ld      h,0
    ld      l,(ix+-8)
    inc     hl
    ld      (ix+-8),l
;         }
    jp      .l100
.l101:
;         j = j + 1;
    ld      h,0
    ld      l,(ix+-6)
    inc     hl
    ld      (ix+-6),l
;     }
    jp      .l98
.l99:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Draw string 'str' at i,j
; draw_text(char i, char j, char *str) {
_draw_text:
    push    ix
    ld      ix,0
    add     ix,sp
;     while (*str) {
.l103:
    ld      l,(ix+8)
    ld      h,(ix+9)
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ld      a,h
    or      l
    jp      z,.l104
;         char val = *str;
    ld      l,(ix+8)
    ld      h,(ix+9)
    ld      a,(hl)
    ld      l,a
    ld      h,0
    push    hl
;         str      = str + 1;
    ld      l,(ix+8)
    ld      h,(ix+9)
    inc     hl
    ld      (ix+8),l
    ld      (ix+9),h
;         char idx = 1;
    ld      hl,1
    push    hl

;         if (val >= '0' && val <= '9') {
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,48
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jr      z,.l105
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,57
    ex      de,hl
    pop     hl
    call    __les
    ld      a,h
    or      l
.l105:
    ld      hl,0
    jr      z,.l106
    inc     l
.l106:
    ld      a,h
    or      l
    jp      z,.l107
;             idx = 64 + (val - '0');
    ld      hl,64
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,48
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-4),l
;         } else if (val >= 'A' && val <= 'Z') {
    jp      .l108
.l107:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,65
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jr      z,.l109
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,90
    ex      de,hl
    pop     hl
    call    __les
    ld      a,h
    or      l
.l109:
    ld      hl,0
    jr      z,.l110
    inc     l
.l110:
    ld      a,h
    or      l
    jp      z,.l111
;             idx = 80 + (val - 'A');
    ld      hl,80
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,65
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-4),l
;         } else if (val >= 'a' && val <= 'z') {
    jp      .l112
.l111:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,97
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jr      z,.l113
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,122
    ex      de,hl
    pop     hl
    call    __les
    ld      a,h
    or      l
.l113:
    ld      hl,0
    jr      z,.l114
    inc     l
.l114:
    ld      a,h
    or      l
    jp      z,.l115
;             idx = 80 + (val - 'a');
    ld      hl,80
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,97
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (ix+-4),l
;         }
;         set_tile(i, j, idx);
.l115:
.l112:
.l108:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      h,0
    ld      l,(ix+6)
    push    hl
    ld      h,0
    ld      l,(ix+4)
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;         i = i + 1;
    ld      h,0
    ld      l,(ix+4)
    inc     hl
    ld      (ix+4),l
;     }
    pop     af
    pop     af
    jp      .l103
.l104:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Draw static part of screen. Only drawn at start
; draw_static_screen() {
_draw_static_screen:
    push    ix
    ld      ix,0
    add     ix,sp
;     char i;
    push    af
;     char j;
    push    af

;     // Draw playfield borders
;     j = 0;
    ld      hl,0
    ld      (ix+-4),l
;     while (j < PLAYFIELD_H) {
.l116:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l117
;         set_tile(PLAYFIELD_XL - 1, j + PLAYFIELD_YT, 32);
    ld      hl,32
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      hl,10
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;         set_tile(PLAYFIELD_XR + 1, j + PLAYFIELD_YT, 32);
    ld      hl,32
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      hl,21
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;         j = j + 1;
    ld      h,0
    ld      l,(ix+-4)
    inc     hl
    ld      (ix+-4),l
;     }
    jp      .l116
.l117:
;     i = 0;
    ld      hl,0
    ld      (ix+-2),l
;     while (i < PLAYFIELD_W + 2) {
.l118:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,12
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l119
;         set_tile(i + PLAYFIELD_W, PLAYFIELD_YB + 1, 33);
    ld      hl,33
    push    hl
    ld      hl,22
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;         i = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l118
.l119:

;     // Draw playfield content
;     j = 0;
    ld      hl,0
    ld      (ix+-4),l
;     while (j < PLAYFIELD_H) {
.l120:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l121
;         i = 0;
    ld      hl,0
    ld      (ix+-2),l
;         while (i < PLAYFIELD_W) {
.l122:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l123
;             set_tile(i + PLAYFIELD_XL, j + PLAYFIELD_YT, bg_tile);
    ld      a,(_bg_tile)
    ld      h,0
    ld      l,a
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,11
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;             i = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;         }
    jp      .l122
.l123:
;         j = j + 1;
    ld      h,0
    ld      l,(ix+-4)
    inc     hl
    ld      (ix+-4),l
;     }
    jp      .l120
.l121:

;     // Draw tetromino preview borders
;     set_tile(26, 4, 33);
    ld      hl,33
    push    hl
    ld      hl,4
    push    hl
    ld      hl,26
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(27, 4, 34);
    ld      hl,34
    push    hl
    ld      hl,4
    push    hl
    ld      hl,27
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(28, 4, 35);
    ld      hl,35
    push    hl
    ld      hl,4
    push    hl
    ld      hl,28
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(29, 4, 35);
    ld      hl,35
    push    hl
    ld      hl,4
    push    hl
    ld      hl,29
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(30, 4, 36);
    ld      hl,36
    push    hl
    ld      hl,4
    push    hl
    ld      hl,30
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(31, 4, 33);
    ld      hl,33
    push    hl
    ld      hl,4
    push    hl
    ld      hl,31
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af

;     set_tile(26, 5, 40);
    ld      hl,40
    push    hl
    ld      hl,5
    push    hl
    ld      hl,26
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(26, 6, 41);
    ld      hl,41
    push    hl
    ld      hl,6
    push    hl
    ld      hl,26
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(26, 7, 41);
    ld      hl,41
    push    hl
    ld      hl,7
    push    hl
    ld      hl,26
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(26, 8, 42);
    ld      hl,42
    push    hl
    ld      hl,8
    push    hl
    ld      hl,26
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af

;     set_tile(31, 5, 43);
    ld      hl,43
    push    hl
    ld      hl,5
    push    hl
    ld      hl,31
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(31, 6, 44);
    ld      hl,44
    push    hl
    ld      hl,6
    push    hl
    ld      hl,31
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(31, 7, 44);
    ld      hl,44
    push    hl
    ld      hl,7
    push    hl
    ld      hl,31
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(31, 8, 45);
    ld      hl,45
    push    hl
    ld      hl,8
    push    hl
    ld      hl,31
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af

;     set_tile(26, 9, 33);
    ld      hl,33
    push    hl
    ld      hl,9
    push    hl
    ld      hl,26
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(27, 9, 37);
    ld      hl,37
    push    hl
    ld      hl,9
    push    hl
    ld      hl,27
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(28, 9, 38);
    ld      hl,38
    push    hl
    ld      hl,9
    push    hl
    ld      hl,28
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(29, 9, 38);
    ld      hl,38
    push    hl
    ld      hl,9
    push    hl
    ld      hl,29
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(30, 9, 39);
    ld      hl,39
    push    hl
    ld      hl,9
    push    hl
    ld      hl,30
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;     set_tile(31, 9, 33);
    ld      hl,33
    push    hl
    ld      hl,9
    push    hl
    ld      hl,31
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af

;     // Draw texts
;     draw_text(24, 2, " MARATHON ");
    ld      hl,__str1
    push    hl
    ld      hl,2
    push    hl
    ld      hl,24
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
;     draw_text(25, 11, "  SCORE ");
    ld      hl,__str2
    push    hl
    ld      hl,11
    push    hl
    ld      hl,25
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
;     draw_text(25, 14, "  LEVEL ");
    ld      hl,__str3
    push    hl
    ld      hl,14
    push    hl
    ld      hl,25
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
;     draw_text(25, 17, "  LINES ");
    ld      hl,__str4
    push    hl
    ld      hl,17
    push    hl
    ld      hl,25
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
; }
.return:
    ld      sp,ix
    pop     ix
    ret
__str1:
    defb 32,77,65,82,65,84,72,79,78,32,0
__str2:
    defb 32,32,83,67,79,82,69,32,0
__str3:
    defb 32,32,76,69,86,69,76,32,0
__str4:
    defb 32,32,76,73,78,69,83,32,0

; // Draw score/level/lines stats
; int  old_score = 0xFFFF;
_old_score:
    defw -1
; char old_level = 0xFF;
_old_level:
    defb 255
; int  old_lines = 0xFFFF;
_old_lines:
    defw -1

; draw_stats() {
_draw_stats:
    push    ix
    ld      ix,0
    add     ix,sp
;     tmpstr[0] = ' ';
    ld      hl,32
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     tmpstr[1] = ' ';
    ld      hl,32
    push    hl
    ld      hl,_tmpstr
    inc     hl
    pop     de
    ld      (hl),e
    ex      de,hl
;     tmpstr[7] = ' ';
    ld      hl,32
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;     tmpstr[8] = 0;
    ld      hl,0
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl

;     if (old_score != score) {
    ld      hl,(_old_score)
    push    hl
    ld      hl,(_score)
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jr      z,.l124
    ld      hl,1
.l124:
    ld      a,h
    or      l
    jp      z,.l125
;         old_score = score;
    ld      hl,(_score)
    ld      (_old_score),hl
;         itoa(tmpstr + 2, score);
    ld      hl,(_score)
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _itoa
    pop     af
    pop     af
;         tmpstr[7] = ' ';
    ld      hl,32
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;         draw_text(25, 12, tmpstr);
    ld      hl,_tmpstr
    push    hl
    ld      hl,12
    push    hl
    ld      hl,25
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
;     }

;     if (old_level != level) {
.l125:
    ld      a,(_old_level)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jr      z,.l126
    ld      hl,1
.l126:
    ld      a,h
    or      l
    jp      z,.l127
;         old_level = level;
    ld      a,(_level)
    ld      h,0
    ld      l,a
    ld      a,l
    ld      (_old_level),a
;         itoa(tmpstr + 2, level);
    ld      a,(_level)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _itoa
    pop     af
    pop     af
;         tmpstr[7] = ' ';
    ld      hl,32
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;         draw_text(25, 15, tmpstr);
    ld      hl,_tmpstr
    push    hl
    ld      hl,15
    push    hl
    ld      hl,25
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
;     }

;     if (old_lines != lines) {
.l127:
    ld      hl,(_old_lines)
    push    hl
    ld      hl,(_lines)
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jr      z,.l128
    ld      hl,1
.l128:
    ld      a,h
    or      l
    jp      z,.l129
;         old_lines = lines;
    ld      hl,(_lines)
    ld      (_old_lines),hl
;         itoa(tmpstr + 2, lines);
    ld      hl,(_lines)
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _itoa
    pop     af
    pop     af
;         tmpstr[7] = ' ';
    ld      hl,32
    push    hl
    ld      hl,_tmpstr
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    add     hl,de
    pop     de
    ld      (hl),e
    ex      de,hl
;         draw_text(25, 18, tmpstr);
    ld      hl,_tmpstr
    push    hl
    ld      hl,18
    push    hl
    ld      hl,25
    push    hl
    call    _draw_text
    pop     af
    pop     af
    pop     af
;     }
; }
.l129:
.return:
    ld      sp,ix
    pop     ix
    ret

; // Draw tetromino preview content
; draw_preview() {
_draw_preview:
    push    ix
    ld      ix,0
    add     ix,sp
;     // Draw tetromino preview content
;     char j;
    push    af
;     char i;
    push    af
;     j = 0;
    ld      hl,0
    ld      (ix+-2),l
;     while (j < 4) {
.l130:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l131
;         i = 0;
    ld      hl,0
    ld      (ix+-4),l
;         while (i < 4) {
.l132:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l133
;             set_tile(i + 27, j + 5, 1);
    ld      hl,1
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,5
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,27
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;             i = i + 1;
    ld      h,0
    ld      l,(ix+-4)
    inc     hl
    ld      (ix+-4),l
;         }
    jp      .l132
.l133:
;         j = j + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l130
.l131:

;     char x = 28;
    ld      hl,28
    push    hl
;     char y = 7;
    ld      hl,7
    push    hl
;     if (next_tetromino == TM_O) {
    ld      a,(_next_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l134
    ld      hl,0
    jr      .l135
.l134:
    inc     hl
.l135:
    ld      a,h
    or      l
    jp      z,.l136
;         y = 6;
    ld      hl,6
    ld      (ix+-8),l
;     } else if (next_tetromino == TM_I) {
    jp      .l137
.l136:
    ld      a,(_next_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l138
    ld      hl,0
    jr      .l139
.l138:
    inc     hl
.l139:
    ld      a,h
    or      l
    jp      z,.l140
;         x = 28;
    ld      hl,28
    ld      (ix+-6),l
;         y = 6;
    ld      hl,6
    ld      (ix+-8),l
;     }
;     draw_tetromino(x, y, next_tetromino, 0, 0, 0);
.l140:
.l137:
    ld      hl,0
    push    hl
    ld      hl,0
    push    hl
    ld      hl,0
    push    hl
    ld      a,(_next_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      h,0
    ld      l,(ix+-8)
    push    hl
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; get_joystick() {
_get_joystick:
    push    ix
    ld      ix,0
    add     ix,sp
;     char old_addr = IO_PSG1ADDR;
    ld      b,$00
    ld      c,$F7
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl
;     IO_PSG1ADDR   = 14;
    ld      hl,14
    ld      b,$00
    ld      c,$F7
    ld      a,l
    out     (c),a
;     char joyval   = IO_PSG1DATA;
    ld      b,$00
    ld      c,$F6
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl
;     IO_PSG1ADDR   = old_addr;
    ld      h,0
    ld      l,(ix+-2)
    ld      b,$00
    ld      c,$F7
    ld      a,l
    out     (c),a
;     return joyval;
    ld      h,0
    ld      l,(ix+-4)
    jp      .return
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Compose keys bitmap with only the keys used by the game
; getkeys() {
_getkeys:
    push    ix
    ld      ix,0
    add     ix,sp
;     char result = 0;
    ld      hl,0
    push    hl
;     if (kb_pressing(KEY_A) || kb_pressing(KEY_LEFT))
    ld      hl,53
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
    jr      nz,.l141
    ld      hl,22
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
.l141:
    ld      hl,1
    jr      nz,.l142
    dec     l
.l142:
    ld      a,h
    or      l
    jp      z,.l143
;         result = result | KBM_LEFT;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (kb_pressing(KEY_D) || kb_pressing(KEY_RIGHT))
.l143:
    ld      hl,44
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
    jr      nz,.l144
    ld      hl,15
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
.l144:
    ld      hl,1
    jr      nz,.l145
    dec     l
.l145:
    ld      a,h
    or      l
    jp      z,.l146
;         result = result | KBM_RIGHT;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (kb_pressing(KEY_W) || kb_pressing(KEY_UP))
.l146:
    ld      hl,57
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
    jr      nz,.l147
    ld      hl,14
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
.l147:
    ld      hl,1
    jr      nz,.l148
    dec     l
.l148:
    ld      a,h
    or      l
    jp      z,.l149
;         result = result | KBM_UP;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (kb_pressing(KEY_S) || kb_pressing(KEY_DOWN))
.l149:
    ld      hl,50
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
    jr      nz,.l150
    ld      hl,23
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
.l150:
    ld      hl,1
    jr      nz,.l151
    dec     l
.l151:
    ld      a,h
    or      l
    jp      z,.l152
;         result = result | KBM_DOWN;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (kb_pressing(KEY_N) || kb_pressing(KEY_Z))
.l152:
    ld      hl,20
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
    jr      nz,.l153
    ld      hl,51
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
.l153:
    ld      hl,1
    jr      nz,.l154
    dec     l
.l154:
    ld      a,h
    or      l
    jp      z,.l155
;         result = result | KBM_ROTATE_CCW;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,16
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (kb_pressing(KEY_M) || kb_pressing(KEY_X))
.l155:
    ld      hl,19
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
    jr      nz,.l156
    ld      hl,45
    push    hl
    call    _kb_pressing
    pop     af
    ld      a,h
    or      l
.l156:
    ld      hl,1
    jr      nz,.l157
    dec     l
.l157:
    ld      a,h
    or      l
    jp      z,.l158
;         result = result | KBM_ROTATE_CW;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,32
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l

;     char joyval = ~get_joystick();
.l158:
    call    _get_joystick
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
;     if (joyval & (1 << 0))
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l159
;         result = result | KBM_DOWN;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (joyval & (1 << 1))
.l159:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l160
;         result = result | KBM_RIGHT;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (joyval & (1 << 2))
.l160:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l161
;         result = result | KBM_UP;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (joyval & (1 << 3))
.l161:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l162
;         result = result | KBM_LEFT;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (joyval & (1 << 6))
.l162:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,64
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l163
;         result = result | KBM_ROTATE_CW;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,32
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l
;     if (joyval & (1 << 7))
.l163:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,128
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l164
;         result = result | KBM_ROTATE_CCW;
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,16
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-2),l

;     return result;
.l164:
    ld      h,0
    ld      l,(ix+-2)
    jp      .return
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Move tetromino left
; move_left() {
_move_left:
    push    ix
    ld      ix,0
    add     ix,sp
;     if (draw_tetromino(cur_posx - 1, cur_posy, cur_tetromino, cur_rot, 1, 0)) {
    ld      hl,0
    push    hl
    ld      hl,1
    push    hl
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    dec     hl
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    ld      a,h
    or      l
    jp      z,.l165
;         cur_posx = cur_posx - 1;
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    dec     hl
    ld      a,l
    ld      (_cur_posx),a
;     }
; }
.l165:
.return:
    ld      sp,ix
    pop     ix
    ret

; // Move tetromino right
; move_right() {
_move_right:
    push    ix
    ld      ix,0
    add     ix,sp
;     if (draw_tetromino(cur_posx + 1, cur_posy, cur_tetromino, cur_rot, 1, 0)) {
    ld      hl,0
    push    hl
    ld      hl,1
    push    hl
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    inc     hl
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    ld      a,h
    or      l
    jp      z,.l166
;         cur_posx = cur_posx + 1;
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    inc     hl
    ld      a,l
    ld      (_cur_posx),a
;     }
; }
.l166:
.return:
    ld      sp,ix
    pop     ix
    ret

; // Move tetromino down
; move_down() {
_move_down:
    push    ix
    ld      ix,0
    add     ix,sp
;     if (draw_tetromino(cur_posx, cur_posy + 1, cur_tetromino, cur_rot, 1, 0)) {
    ld      hl,0
    push    hl
    ld      hl,1
    push    hl
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    inc     hl
    push    hl
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    ld      a,h
    or      l
    jp      z,.l167
;         cur_posy = cur_posy + 1;
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    inc     hl
    ld      a,l
    ld      (_cur_posy),a
;         return 1;
    ld      hl,1
    jp      .return
;     } else {
    jp      .l168
.l167:
;         return 0;
    ld      hl,0
    jp      .return
;     }
.l168:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Rotate tetromino either clockwise (cw=true) or counter-clockwise (cw=false)
; rotate(char cw) {
_rotate:
    push    ix
    ld      ix,0
    add     ix,sp
;     char new_rot = cur_rot;
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
;     if (cw)
    ld      h,0
    ld      l,(ix+4)
    ld      a,h
    or      l
    jp      z,.l169
;         new_rot = (new_rot + 1) & 3;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      (ix+-2),l
;     else
    jp      .l170
.l169:
;         new_rot = (new_rot - 1) & 3;
    ld      h,0
    ld      l,(ix+-2)
    dec     hl
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      (ix+-2),l
.l170:

;     char idx = (cur_rot << 1);
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    call    __shl
    push    hl
;     if (cw)
    ld      h,0
    ld      l,(ix+4)
    ld      a,h
    or      l
    jp      z,.l171
;         idx = idx | 1;
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      (ix+-4),l

;     int *wall_kick_offsets_x;
.l171:
    push    af
;     int *wall_kick_offsets_y;
    push    af
;     int  offset = 5 * idx;
    ld      hl,5
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    call    __multsi
    push    hl
;     if (cur_tetromino == TM_I) {
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l172
    ld      hl,0
    jr      .l173
.l172:
    inc     hl
.l173:
    ld      a,h
    or      l
    jp      z,.l174
;         wall_kick_offsets_x = wall_kick_offsets_i_x + offset;
    ld      hl,_wall_kick_offsets_i_x
    push    hl
    ld      l,(ix+-10)
    ld      h,(ix+-9)
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      (ix+-6),l
    ld      (ix+-5),h
;         wall_kick_offsets_y = wall_kick_offsets_i_y + offset;
    ld      hl,_wall_kick_offsets_i_y
    push    hl
    ld      l,(ix+-10)
    ld      h,(ix+-9)
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      (ix+-8),l
    ld      (ix+-7),h
;     } else {
    jp      .l175
.l174:
;         wall_kick_offsets_x = wall_kick_offsets_jlstz_x + offset;
    ld      hl,_wall_kick_offsets_jlstz_x
    push    hl
    ld      l,(ix+-10)
    ld      h,(ix+-9)
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      (ix+-6),l
    ld      (ix+-5),h
;         wall_kick_offsets_y = wall_kick_offsets_jlstz_y + offset;
    ld      hl,_wall_kick_offsets_jlstz_y
    push    hl
    ld      l,(ix+-10)
    ld      h,(ix+-9)
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      (ix+-8),l
    ld      (ix+-7),h
;     }
.l175:
;     char max_i = 4;
    ld      hl,4
    push    hl
;     if (cur_tetromino == TM_O)
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l176
    ld      hl,0
    jr      .l177
.l176:
    inc     hl
.l177:
    ld      a,h
    or      l
    jp      z,.l178
;         max_i = 0;
    ld      hl,0
    ld      (ix+-12),l

;     char i = 0;
.l178:
    ld      hl,0
    push    hl
;     while (i <= max_i) {
.l179:
    ld      h,0
    ld      l,(ix+-14)
    push    hl
    ld      h,0
    ld      l,(ix+-12)
    ex      de,hl
    pop     hl
    call    __les
    ld      a,h
    or      l
    jp      z,.l180
;         char new_posx = cur_posx + wall_kick_offsets_x[i];
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    push    hl
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    push    hl
    ld      h,0
    ld      l,(ix+-14)
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      e,(hl)
    inc     hl
    ld      d,(hl)
    ex      de,hl
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
;         char new_posy = cur_posy + wall_kick_offsets_y[i];
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    push    hl
    ld      l,(ix+-8)
    ld      h,(ix+-7)
    push    hl
    ld      h,0
    ld      l,(ix+-14)
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      e,(hl)
    inc     hl
    ld      d,(hl)
    ex      de,hl
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
;         if (draw_tetromino(new_posx, new_posy, cur_tetromino, new_rot, 1, 0)) {
    ld      hl,0
    push    hl
    ld      hl,1
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      h,0
    ld      l,(ix+-18)
    push    hl
    ld      h,0
    ld      l,(ix+-16)
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    ld      a,h
    or      l
    jp      z,.l181
;             cur_rot  = new_rot;
    ld      h,0
    ld      l,(ix+-2)
    ld      a,l
    ld      (_cur_rot),a
;             cur_posx = new_posx;
    ld      h,0
    ld      l,(ix+-16)
    ld      a,l
    ld      (_cur_posx),a
;             cur_posy = new_posy;
    ld      h,0
    ld      l,(ix+-18)
    ld      a,l
    ld      (_cur_posy),a
;             break;
    jp      .l180
;         }
;         i = i + 1;
.l181:
    ld      h,0
    ld      l,(ix+-14)
    inc     hl
    ld      (ix+-14),l
;     }
    pop     af
    pop     af
    jp      .l179
.l180:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Processing that needs to be done at the start of the frame
; frame() {
_frame:
    push    ix
    ld      ix,0
    add     ix,sp
;     // IO_VPALSEL  = 0;
;     // IO_VPALDATA = 0;

;     // Wait for end of frame (line 216)
;     video_wait_line(216);
    ld      hl,216
    push    hl
    call    _video_wait_line
    pop     af

;     // IO_VPALSEL  = 0;
;     // IO_VPALDATA = 0xFF;

;     // Scan keys
;     kb_scan();
    call    _kb_scan

;     // Update screen during non-visible part
;     if (bgdelay == 0) {
    ld      a,(_bgdelay)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l182
    ld      hl,0
    jr      .l183
.l182:
    inc     hl
.l183:
    ld      a,h
    or      l
    jp      z,.l184
;         // Animate background by updating 2 tile patterns
;         memcpy(bgtiles_dst, bgtiles_src2, 64);
    ld      hl,64
    push    hl
    ld      hl,(_bgtiles_src2)
    push    hl
    ld      hl,(_bgtiles_dst)
    push    hl
    call    _memcpy
    pop     af
    pop     af
    pop     af
;     }
;     set_tetromino_sprites(cur_posx << 3, cur_posy << 3, cur_tetromino, cur_rot);
.l184:
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    call    __shl
    push    hl
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    call    __shl
    push    hl
    call    _set_tetromino_sprites
    pop     af
    pop     af
    pop     af
    pop     af

;     // Animate background
;     if (bgdelay == 0) {
    ld      a,(_bgdelay)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l185
    ld      hl,0
    jr      .l186
.l185:
    inc     hl
.l186:
    ld      a,h
    or      l
    jp      z,.l187
;         bgdelay = 2;
    ld      hl,2
    ld      a,l
    ld      (_bgdelay),a

;         if (bgtiles_idx == 7) {
    ld      a,(_bgtiles_idx)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l188
    ld      hl,0
    jr      .l189
.l188:
    inc     hl
.l189:
    ld      a,h
    or      l
    jp      z,.l190
;             bgtiles_idx  = 0;
    ld      hl,0
    ld      a,l
    ld      (_bgtiles_idx),a
;             bgtiles_src2 = bgtiles_src;
    ld      hl,(_bgtiles_src)
    ld      (_bgtiles_src2),hl
;         } else {
    jp      .l191
.l190:
;             bgtiles_idx  = bgtiles_idx + 1;
    ld      a,(_bgtiles_idx)
    ld      h,0
    ld      l,a
    inc     hl
    ld      a,l
    ld      (_bgtiles_idx),a
;             bgtiles_src2 = bgtiles_src2 + 64;
    ld      hl,(_bgtiles_src2)
    push    hl
    ld      hl,64
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (_bgtiles_src2),hl
;         }
.l191:
;     } else {
    jp      .l192
.l187:
;         bgdelay = bgdelay - 1;
    ld      a,(_bgdelay)
    ld      h,0
    ld      l,a
    dec     hl
    ld      a,l
    ld      (_bgdelay),a
;     }
.l192:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Switch to next tetromino piece
; next_piece() {
_next_piece:
    push    ix
    ld      ix,0
    add     ix,sp
;     cur_posx       = (PLAYFIELD_XL + PLAYFIELD_XR) / 2;
    ld      hl,15
    ld      a,l
    ld      (_cur_posx),a
;     cur_posy       = PLAYFIELD_YT - 1;
    ld      hl,1
    ld      a,l
    ld      (_cur_posy),a
;     cur_rot        = 0;
    ld      hl,0
    ld      a,l
    ld      (_cur_rot),a
;     cur_tetromino  = next_tetromino;
    ld      a,(_next_tetromino)
    ld      h,0
    ld      l,a
    ld      a,l
    ld      (_cur_tetromino),a
;     next_tetromino = tetromino_random;
    ld      a,(_tetromino_random)
    ld      h,0
    ld      l,a
    ld      a,l
    ld      (_next_tetromino),a
;     draw_preview();
    call    _draw_preview
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Check if given line is full
; check_line(char line) {
_check_line:
    push    ix
    ld      ix,0
    add     ix,sp
;     char i = 0;
    ld      hl,0
    push    hl
;     while (i < PLAYFIELD_W) {
.l193:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l194
;         if (get_tile(i + PLAYFIELD_XL, line + PLAYFIELD_YT) == bg_tile)
    ld      h,0
    ld      l,(ix+4)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,11
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _get_tile
    pop     af
    pop     af
    push    hl
    ld      a,(_bg_tile)
    ld      h,0
    ld      l,a
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l195
    ld      hl,0
    jr      .l196
.l195:
    inc     hl
.l196:
    ld      a,h
    or      l
    jp      z,.l197
;             return 0;
    ld      hl,0
    jp      .return
;         i = i + 1;
.l197:
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l193
.l194:
;     return 1;
    ld      hl,1
    jp      .return
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // Check for full lines
; check_lines() {
_check_lines:
    push    ix
    ld      ix,0
    add     ix,sp
;     char lines_full = 0;
    ld      hl,0
    push    hl
;     int  j;
    push    af
;     int  i;
    push    af

;     j = 0;
    ld      hl,0
    ld      (ix+-4),l
    ld      (ix+-3),h
;     while (j < PLAYFIELD_H) {
.l198:
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l199
;         if (check_line(j)) {
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    push    hl
    call    _check_line
    pop     af
    ld      a,h
    or      l
    jp      z,.l200
;             i = 0;
    ld      hl,0
    ld      (ix+-6),l
    ld      (ix+-5),h
;             while (i < PLAYFIELD_W) {
.l201:
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l202
;                 set_tile(i + PLAYFIELD_XL, j + PLAYFIELD_YT, MARKER_TILE);
    ld      hl,1
    push    hl
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    push    hl
    ld      hl,11
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;                 i = i + 1;
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    inc     hl
    ld      (ix+-6),l
    ld      (ix+-5),h
;             }
    jp      .l201
.l202:
;             lines_full = lines_full + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;         }
;         j = j + 1;
.l200:
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    inc     hl
    ld      (ix+-4),l
    ld      (ix+-3),h
;     }
    jp      .l198
.l199:

;     if (lines_full == 0)
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l203
    ld      hl,0
    jr      .l204
.l203:
    inc     hl
.l204:
    ld      a,h
    or      l
    jp      z,.l205
;         return;
    jp      .return

;     // Small delay
;     i = 0;
.l205:
    ld      hl,0
    ld      (ix+-6),l
    ld      (ix+-5),h
;     while (i < 5) {
.l206:
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    push    hl
    ld      hl,5
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l207
;         frame();
    call    _frame
;         i = i + 1;
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    inc     hl
    ld      (ix+-6),l
    ld      (ix+-5),h
;     }
    jp      .l206
.l207:

;     // Remove all marked lines
;     int line = PLAYFIELD_H - 1;
    ld      hl,19
    push    hl
;     while (line >= 0) {
.l208:
    ld      l,(ix+-8)
    ld      h,(ix+-7)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jp      z,.l209
;         if (get_tile(PLAYFIELD_XL, line + PLAYFIELD_YT) == MARKER_TILE) {
    ld      l,(ix+-8)
    ld      h,(ix+-7)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      hl,11
    push    hl
    call    _get_tile
    pop     af
    pop     af
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l210
    ld      hl,0
    jr      .l211
.l210:
    inc     hl
.l211:
    ld      a,h
    or      l
    jp      z,.l212
;             // Remove line by copying all lines above it one line down
;             j = line;
    ld      l,(ix+-8)
    ld      h,(ix+-7)
    ld      (ix+-4),l
    ld      (ix+-3),h
;             while (j >= 0) {
.l213:
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jp      z,.l214
;                 i = 0;
    ld      hl,0
    ld      (ix+-6),l
    ld      (ix+-5),h
;                 while (i < PLAYFIELD_W) {
.l215:
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l216
;                     set_tile(PLAYFIELD_XL + i, PLAYFIELD_YT + j, get_tile(PLAYFIELD_XL + i, PLAYFIELD_YT + j - 1));
    ld      hl,2
    push    hl
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    ex      de,hl
    pop     hl
    add     hl,de
    dec     hl
    push    hl
    ld      hl,11
    push    hl
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _get_tile
    pop     af
    pop     af
    push    hl
    ld      hl,2
    push    hl
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      hl,11
    push    hl
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;                     i = i + 1;
    ld      l,(ix+-6)
    ld      h,(ix+-5)
    inc     hl
    ld      (ix+-6),l
    ld      (ix+-5),h
;                 }
    jp      .l215
.l216:
;                 j = j - 1;
    ld      l,(ix+-4)
    ld      h,(ix+-3)
    dec     hl
    ld      (ix+-4),l
    ld      (ix+-3),h
;             }
    jp      .l213
.l214:
;             lines = lines + 1;
    ld      hl,(_lines)
    inc     hl
    ld      (_lines),hl
;             if (lines % 10 == 0) {
    ld      hl,(_lines)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __modsi
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l217
    ld      hl,0
    jr      .l218
.l217:
    inc     hl
.l218:
    ld      a,h
    or      l
    jp      z,.l219
;                 level = level + 1;
    ld      a,(_level)
    ld      h,0
    ld      l,a
    inc     hl
    ld      a,l
    ld      (_level),a
;                 if (level > 20)
    ld      a,(_level)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __gts
    ld      a,h
    or      l
    jp      z,.l220
;                     level = 20;
    ld      hl,20
    ld      a,l
    ld      (_level),a
;             }
.l220:
;             frame();
.l219:
    call    _frame

;         } else {
    jp      .l221
.l212:
;             line = line - 1;
    ld      l,(ix+-8)
    ld      h,(ix+-7)
    dec     hl
    ld      (ix+-8),l
    ld      (ix+-7),h
;         }
.l221:
;     }
    jp      .l208
.l209:

;     if (lines_full == 1)
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l222
    ld      hl,0
    jr      .l223
.l222:
    inc     hl
.l223:
    ld      a,h
    or      l
    jp      z,.l224
;         score = score + 40 * (level + 1);
    ld      hl,(_score)
    push    hl
    ld      hl,40
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    inc     hl
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (_score),hl
;     else if (lines_full == 2)
    jp      .l225
.l224:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l226
    ld      hl,0
    jr      .l227
.l226:
    inc     hl
.l227:
    ld      a,h
    or      l
    jp      z,.l228
;         score = score + 100 * (level + 1);
    ld      hl,(_score)
    push    hl
    ld      hl,100
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    inc     hl
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (_score),hl
;     else if (lines_full == 3)
    jp      .l229
.l228:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,3
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l230
    ld      hl,0
    jr      .l231
.l230:
    inc     hl
.l231:
    ld      a,h
    or      l
    jp      z,.l232
;         score = score + 300 * (level + 1);
    ld      hl,(_score)
    push    hl
    ld      hl,300
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    inc     hl
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (_score),hl
;     else if (lines_full == 4)
    jp      .l233
.l232:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,4
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l234
    ld      hl,0
    jr      .l235
.l234:
    inc     hl
.l235:
    ld      a,h
    or      l
    jp      z,.l236
;         score = score + 1200 * (level + 1);
    ld      hl,(_score)
    push    hl
    ld      hl,1200
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    inc     hl
    ex      de,hl
    pop     hl
    call    __multsi
    ex      de,hl
    pop     hl
    add     hl,de
    ld      (_score),hl

;     if (score > 30000)
.l236:
.l233:
.l229:
.l225:
    ld      hl,(_score)
    push    hl
    ld      hl,30000
    ex      de,hl
    pop     hl
    call    __gts
    ld      a,h
    or      l
    jp      z,.l237
;         score = 30000;
    ld      hl,30000
    ld      (_score),hl
;     // if (score > 9999999)
;     //     score = 9999999;
; }
.l237:
.return:
    ld      sp,ix
    pop     ix
    ret

; // Lock current tetromino in place and switch to the next one
; lock_piece() {
_lock_piece:
    push    ix
    ld      ix,0
    add     ix,sp
;     // Lock in place
;     if (!draw_tetromino(cur_posx, cur_posy, cur_tetromino, cur_rot, 0, 1)) {
    ld      hl,1
    push    hl
    ld      hl,0
    push    hl
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    ld      a,h
    or      l
    ld      hl,0
    jr      nz,.l238
    inc     l
.l238:
    ld      a,h
    or      l
    jp      z,.l239
;         gameover = 1;
    ld      hl,1
    ld      a,l
    ld      (_gameover),a
;     }

;     // Hide sprites
;     IO_VCTRL = IO_VCTRL & ~VCTRL_SPR_EN;
.l239:
    ld      b,$00
    ld      c,$E0
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,-9
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      b,$00
    ld      c,$E0
    ld      a,l
    out     (c),a

;     // Small delay
;     char i = 0;
    ld      hl,0
    push    hl
;     while (i < 5) {
.l240:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,5
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l241
;         frame();
    call    _frame
;         i = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l240
.l241:
;     draw_tetromino(cur_posx, cur_posy, cur_tetromino, cur_rot, 0, 0);
    ld      hl,0
    push    hl
    ld      hl,0
    push    hl
    ld      a,(_cur_rot)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_tetromino)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posy)
    ld      h,0
    ld      l,a
    push    hl
    ld      a,(_cur_posx)
    ld      h,0
    ld      l,a
    push    hl
    call    _draw_tetromino
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af
    pop     af

;     // Switch to next piece
;     next_piece();
    call    _next_piece

;     // Show sprites
;     IO_VCTRL = IO_VCTRL | VCTRL_SPR_EN;
    ld      b,$00
    ld      c,$E0
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    ld      b,$00
    ld      c,$E0
    ld      a,l
    out     (c),a

;     // Check for full lines
;     check_lines();
    call    _check_lines
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; play_marathon() {
_play_marathon:
    push    ix
    ld      ix,0
    add     ix,sp
;     char prev_keys = 0;
    ld      hl,0
    push    hl

;     score    = 0;
    ld      hl,0
    ld      (_score),hl
;     level    = 0;
    ld      hl,0
    ld      a,l
    ld      (_level),a
;     lines    = 0;
    ld      hl,0
    ld      (_lines),hl
;     gameover = 0;
    ld      hl,0
    ld      a,l
    ld      (_gameover),a

;     char gravity_delay = speed_curve[level];
    ld      hl,_speed_curve
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    push    hl
;     char left_delay    = 0;
    ld      hl,0
    push    hl
;     char right_delay   = 0;
    ld      hl,0
    push    hl
;     char down_delay    = 0;
    ld      hl,0
    push    hl

;     cur_tetromino = tetromino_random;
    ld      a,(_tetromino_random)
    ld      h,0
    ld      l,a
    ld      a,l
    ld      (_cur_tetromino),a

;     // Initial screen drawing
;     draw_static_screen();
    call    _draw_static_screen
;     draw_preview();
    call    _draw_preview

;     // Switch video mode to tile mode
;     IO_VCTRL = VCTRL_SPR_EN | VCTRL_MODE_TILE;
    ld      hl,10
    ld      b,$00
    ld      c,$E0
    ld      a,l
    out     (c),a
;     next_piece();
    call    _next_piece

;     char wait_down_release = 0;
    ld      hl,0
    push    hl

;     while (!gameover) {
.l242:
    ld      a,(_gameover)
    ld      h,0
    ld      l,a
    ld      a,h
    or      l
    ld      hl,0
    jr      nz,.l244
    inc     l
.l244:
    ld      a,h
    or      l
    jp      z,.l243
;         frame();
    call    _frame

;         // Move tetromino down when gravity delay expires
;         if (gravity_delay == 0) {
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l245
    ld      hl,0
    jr      .l246
.l245:
    inc     hl
.l246:
    ld      a,h
    or      l
    jp      z,.l247
;             gravity_delay = speed_curve[level];
    ld      hl,_speed_curve
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ld      (ix+-4),l
;             if (!move_down()) {
    call    _move_down
    ld      a,h
    or      l
    ld      hl,0
    jr      nz,.l248
    inc     l
.l248:
    ld      a,h
    or      l
    jp      z,.l249
;                 lock_piece();
    call    _lock_piece
;             }
;         } else {
.l249:
    jp      .l250
.l247:
;             gravity_delay = gravity_delay - 1;
    ld      h,0
    ld      l,(ix+-4)
    dec     hl
    ld      (ix+-4),l
;         }
.l250:

;         // Update 'random' tetromino index
;         tetromino_random = tetromino_random + 1;
    ld      a,(_tetromino_random)
    ld      h,0
    ld      l,a
    inc     hl
    ld      a,l
    ld      (_tetromino_random),a
;         if (tetromino_random >= TM_TOTAL)
    ld      a,(_tetromino_random)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jp      z,.l251
;             tetromino_random = 0;
    ld      hl,0
    ld      a,l
    ld      (_tetromino_random),a

;         // Handle keyboard interaction
;         char keys    = getkeys();
.l251:
    call    _getkeys
    push    hl
;         char newkeys = ~prev_keys & keys;
    ld      h,0
    ld      l,(ix+-2)
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      h,0
    ld      l,(ix+-14)
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    push    hl

;         if (keys & KBM_LEFT) {
    ld      h,0
    ld      l,(ix+-14)
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l252
;             if (left_delay == 0) {
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l253
    ld      hl,0
    jr      .l254
.l253:
    inc     hl
.l254:
    ld      a,h
    or      l
    jp      z,.l255
;                 left_delay = DELAY_LEFT_RIGHT;
    ld      hl,5
    ld      (ix+-6),l
;                 move_left();
    call    _move_left
;             } else {
    jp      .l256
.l255:
;                 left_delay = left_delay - 1;
    ld      h,0
    ld      l,(ix+-6)
    dec     hl
    ld      (ix+-6),l
;             }
.l256:
;         } else {
    jp      .l257
.l252:
;             left_delay = 0;
    ld      hl,0
    ld      (ix+-6),l
;         }
.l257:
;         if (keys & KBM_RIGHT) {
    ld      h,0
    ld      l,(ix+-14)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l258
;             if (right_delay == 0) {
    ld      h,0
    ld      l,(ix+-8)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l259
    ld      hl,0
    jr      .l260
.l259:
    inc     hl
.l260:
    ld      a,h
    or      l
    jp      z,.l261
;                 right_delay = DELAY_LEFT_RIGHT;
    ld      hl,5
    ld      (ix+-8),l
;                 move_right();
    call    _move_right
;             } else {
    jp      .l262
.l261:
;                 right_delay = right_delay - 1;
    ld      h,0
    ld      l,(ix+-8)
    dec     hl
    ld      (ix+-8),l
;             }
.l262:
;         } else {
    jp      .l263
.l258:
;             right_delay = 0;
    ld      hl,0
    ld      (ix+-8),l
;         }
.l263:
;         if (keys & KBM_DOWN) {
    ld      h,0
    ld      l,(ix+-14)
    push    hl
    ld      hl,8
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l264
;             if (!wait_down_release) {
    ld      h,0
    ld      l,(ix+-12)
    ld      a,h
    or      l
    ld      hl,0
    jr      nz,.l265
    inc     l
.l265:
    ld      a,h
    or      l
    jp      z,.l266
;                 if (down_delay == 0) {
    ld      h,0
    ld      l,(ix+-10)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    jp      z,.l267
    ld      hl,0
    jr      .l268
.l267:
    inc     hl
.l268:
    ld      a,h
    or      l
    jp      z,.l269
;                     down_delay = 3;
    ld      hl,3
    ld      (ix+-10),l
;                     if (move_down()) {
    call    _move_down
    ld      a,h
    or      l
    jp      z,.l270
;                         gravity_delay = speed_curve[level];
    ld      hl,_speed_curve
    push    hl
    ld      a,(_level)
    ld      h,0
    ld      l,a
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ld      (ix+-4),l
;                         score         = score + 1;
    ld      hl,(_score)
    inc     hl
    ld      (_score),hl
;                     } else {
    jp      .l271
.l270:
;                         lock_piece();
    call    _lock_piece
;                         wait_down_release = 1;
    ld      hl,1
    ld      (ix+-12),l
;                     }
.l271:
;                 } else {
    jp      .l272
.l269:
;                     down_delay = down_delay - 1;
    ld      h,0
    ld      l,(ix+-10)
    dec     hl
    ld      (ix+-10),l
;                 }
.l272:
;             }
;         } else {
.l266:
    jp      .l273
.l264:
;             wait_down_release = 0;
    ld      hl,0
    ld      (ix+-12),l
;             down_delay        = 0;
    ld      hl,0
    ld      (ix+-10),l
;         }
.l273:
;         if (newkeys & KBM_ROTATE_CCW) {
    ld      h,0
    ld      l,(ix+-16)
    push    hl
    ld      hl,16
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l274
;             rotate(0);
    ld      hl,0
    push    hl
    call    _rotate
    pop     af
;         }
;         if (newkeys & KBM_ROTATE_CW) {
.l274:
    ld      h,0
    ld      l,(ix+-16)
    push    hl
    ld      hl,32
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l275
;             rotate(1);
    ld      hl,1
    push    hl
    call    _rotate
    pop     af
;         }

;         draw_stats();
.l275:
    call    _draw_stats

;         // Keep track of keys pressed during this round
;         prev_keys = keys;
    ld      h,0
    ld      l,(ix+-14)
    ld      (ix+-2),l
;     }
    pop     af
    pop     af
    jp      .l242
.l243:

;     // Disable sprites
;     IO_VCTRL = VCTRL_MODE_TILE;
    ld      hl,2
    ld      b,$00
    ld      c,$E0
    ld      a,l
    out     (c),a

;     // Animation

;     // Draw playfield content
;     int j = PLAYFIELD_H - 1;
    ld      hl,19
    push    hl
;     int i;
    push    af
;     while (j >= 0) {
.l276:
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    push    hl
    ld      hl,0
    ex      de,hl
    pop     hl
    call    __ges
    ld      a,h
    or      l
    jp      z,.l277
;         i = 0;
    ld      hl,0
    ld      (ix+-16),l
    ld      (ix+-15),h
;         while (i < PLAYFIELD_W) {
.l278:
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l279
;             set_tile(i + PLAYFIELD_XL, j + PLAYFIELD_YT, 31);
    ld      hl,31
    push    hl
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    push    hl
    ld      hl,11
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;             i = i + 1;
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    inc     hl
    ld      (ix+-16),l
    ld      (ix+-15),h
;         }
    jp      .l278
.l279:
;         frame();
    call    _frame
;         frame();
    call    _frame
;         j = j - 1;
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    dec     hl
    ld      (ix+-14),l
    ld      (ix+-13),h
;     }
    jp      .l276
.l277:
;     j = 0;
    ld      hl,0
    ld      (ix+-14),l
    ld      (ix+-13),h
;     while (j < PLAYFIELD_H) {
.l280:
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l281
;         i = 0;
    ld      hl,0
    ld      (ix+-16),l
    ld      (ix+-15),h
;         while (i < PLAYFIELD_W) {
.l282:
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l283
;             set_tile2(i + PLAYFIELD_XL, j + PLAYFIELD_YT, gameover_playfield[j * 10 + i] | 0x1100);
    ld      hl,_gameover_playfield
    push    hl
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    push    hl
    ld      hl,10
    ex      de,hl
    pop     hl
    call    __multsi
    push    hl
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    ex      de,hl
    pop     hl
    add     hl,de
    ex      de,hl
    pop     hl
    add     hl,de
    add     hl,de
    ld      e,(hl)
    inc     hl
    ld      d,(hl)
    ex      de,hl
    push    hl
    ld      hl,4352
    ex      de,hl
    pop     hl
    ld      a,h
    or      d
    ld      h,a
    ld      a,l
    or      e
    ld      l,a
    push    hl
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    push    hl
    ld      hl,2
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    push    hl
    ld      hl,11
    ex      de,hl
    pop     hl
    add     hl,de
    push    hl
    call    _set_tile2
    pop     af
    pop     af
    pop     af
;             i = i + 1;
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    inc     hl
    ld      (ix+-16),l
    ld      (ix+-15),h
;         }
    jp      .l282
.l283:
;         frame();
    call    _frame
;         frame();
    call    _frame
;         j = j + 1;
    ld      l,(ix+-14)
    ld      h,(ix+-13)
    inc     hl
    ld      (ix+-14),l
    ld      (ix+-13),h
;     }
    jp      .l280
.l281:

;     char blink = 0;
    ld      hl,0
    push    hl
;     char wait  = 1;
    ld      hl,1
    push    hl
;     char idx;
    push    af
;     while (wait) {
.l284:
    ld      h,0
    ld      l,(ix+-20)
    ld      a,h
    or      l
    jp      z,.l285
;         // Short blink delay
;         i = 0;
    ld      hl,0
    ld      (ix+-16),l
    ld      (ix+-15),h
;         while (i < 20) {
.l286:
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    push    hl
    ld      hl,20
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l287
;             frame();
    call    _frame

;             // Quit game on CTRL-C (or ESCAPE)
;             if (kb_pressed(KEY_C) && kb_pressed(KEY_CTRL)) {
    ld      hl,36
    push    hl
    call    _kb_pressed
    pop     af
    ld      a,h
    or      l
    jr      z,.l288
    ld      hl,61
    push    hl
    call    _kb_pressed
    pop     af
    ld      a,h
    or      l
.l288:
    ld      hl,0
    jr      z,.l289
    inc     l
.l289:
    ld      a,h
    or      l
    jp      z,.l290
;                 quit = 1;
    ld      hl,1
    ld      a,l
    ld      (_quit),a
;                 wait = 0;
    ld      hl,0
    ld      (ix+-20),l
;             }

;             // Start a new game when one of the game keys is pressed
;             char keys = getkeys();
.l290:
    call    _getkeys
    push    hl
;             if (~prev_keys & keys)
    ld      h,0
    ld      l,(ix+-2)
    ld      a,h
    xor     $FF
    ld      h,a
    ld      a,l
    xor     $FF
    ld      l,a
    push    hl
    ld      h,0
    ld      l,(ix+-24)
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l291
;                 wait = 0;
    ld      hl,0
    ld      (ix+-20),l
;             prev_keys = keys;
.l291:
    ld      h,0
    ld      l,(ix+-24)
    ld      (ix+-2),l

;             i = i + 1;
    ld      l,(ix+-16)
    ld      h,(ix+-15)
    inc     hl
    ld      (ix+-16),l
    ld      (ix+-15),h
;         }
    pop     af
    jp      .l286
.l287:

;         if (blink)
    ld      h,0
    ld      l,(ix+-18)
    ld      a,h
    or      l
    jp      z,.l292
;             idx = 0x4F;
    ld      hl,79
    ld      (ix+-22),l
;         else
    jp      .l293
.l292:
;             idx = 0x01;
    ld      hl,1
    ld      (ix+-22),l
.l293:
;         set_tile2(7 + PLAYFIELD_XL, 17 + PLAYFIELD_YT, idx);
    ld      h,0
    ld      l,(ix+-22)
    push    hl
    ld      hl,19
    push    hl
    ld      hl,18
    push    hl
    call    _set_tile2
    pop     af
    pop     af
    pop     af
;         blink = !blink;
    ld      h,0
    ld      l,(ix+-18)
    ld      a,h
    or      l
    ld      hl,0
    jr      nz,.l294
    inc     l
.l294:
    ld      (ix+-18),l
;     }
    jp      .l284
.l285:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; init() {
_init:
    push    ix
    ld      ix,0
    add     ix,sp
;     // Enable turbo mode
;     IO_SYSCTRL = 4;
    ld      hl,4
    ld      b,$00
    ld      c,$FB
    ld      a,l
    out     (c),a

;     // Map video RAM to 0xC000
;     IO_BANK3 = 20;
    ld      hl,20
    ld      b,$00
    ld      c,$F3
    ld      a,l
    out     (c),a

;     // Set tile data palette
;     char i = 0;
    ld      hl,0
    push    hl
;     while (i < 32) {
.l295:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,32
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l296
;         IO_VPALSEL  = 32 + i;
    ld      hl,32
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    ex      de,hl
    pop     hl
    add     hl,de
    ld      b,$00
    ld      c,$EA
    ld      a,l
    out     (c),a
;         IO_VPALDATA = tile_palette[i];
    ld      hl,_tile_palette
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    ex      de,hl
    pop     hl
    add     hl,de
    ld      a,(hl)
    ld      l,a
    ld      h,0
    ld      b,$00
    ld      c,$EB
    ld      a,l
    out     (c),a
;         i           = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l295
.l296:
;     // Copy tile data into VRAM
;     memcpy(vram_tiledata, tile_data, tile_data_end - tile_data);
    ld      hl,_tile_data_end
    push    hl
    ld      hl,_tile_data
    ex      de,hl
    pop     hl
    or      a
    sbc     hl,de
    push    hl
    ld      hl,_tile_data
    push    hl
    ld      hl,-8192
    push    hl
    call    _memcpy
    pop     af
    pop     af
    pop     af

;     // Init video
;     IO_VSCRX_L = 0;
    ld      hl,0
    ld      b,$00
    ld      c,$E1
    ld      a,l
    out     (c),a
;     IO_VSCRX_H = 0;
    ld      hl,0
    ld      b,$00
    ld      c,$E2
    ld      a,l
    out     (c),a
;     IO_VSCRY   = 0;
    ld      hl,0
    ld      b,$00
    ld      c,$E3
    ld      a,l
    out     (c),a

;     // Disable all sprites
;     i = 0;
    ld      hl,0
    ld      (ix+-2),l
;     while (i < 64) {
.l297:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,64
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l298
;         IO_VSPRSEL  = i;
    ld      h,0
    ld      l,(ix+-2)
    ld      b,$00
    ld      c,$E4
    ld      a,l
    out     (c),a
;         IO_VSPRATTR = 0;
    ld      hl,0
    ld      b,$00
    ld      c,$E9
    ld      a,l
    out     (c),a
;         i           = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;     }
    jp      .l297
.l298:

;     bgtiles_dst      = vram_tiledata + 32 * 46;
    ld      hl,-6720
    ld      (_bgtiles_dst),hl
;     bgtiles_src      = vram_tiledata + 32 * 48;
    ld      hl,-6656
    ld      (_bgtiles_src),hl
;     bgtiles_src2     = bgtiles_src;
    ld      hl,(_bgtiles_src)
    ld      (_bgtiles_src2),hl
;     tetromino_random = IO_VLINE % 7;
    ld      b,$00
    ld      c,$EC
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl
    ld      hl,7
    ex      de,hl
    pop     hl
    call    __modsi
    ld      a,l
    ld      (_tetromino_random),a

;     // Draw background
;     char j = 0;
    ld      hl,0
    push    hl
;     while (j < 25) {
.l299:
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      hl,25
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l300
;         i = 0;
    ld      hl,0
    ld      (ix+-2),l
;         while (i < 40) {
.l301:
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      hl,40
    ex      de,hl
    pop     hl
    call    __lts
    ld      a,h
    or      l
    jp      z,.l302
;             char idx;
    push    af
;             if ((i ^ j) & 1) {
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    ex      de,hl
    pop     hl
    ld      a,h
    xor     d
    ld      h,a
    ld      a,l
    xor     e
    ld      l,a
    push    hl
    ld      hl,1
    ex      de,hl
    pop     hl
    ld      a,h
    and     d
    ld      h,a
    ld      a,l
    and     e
    ld      l,a
    ld      a,h
    or      l
    jp      z,.l303
;                 idx = 47;
    ld      hl,47
    ld      (ix+-6),l
;             } else {
    jp      .l304
.l303:
;                 idx = 46;
    ld      hl,46
    ld      (ix+-6),l
;             }
.l304:
;             set_tile(i, j, idx);
    ld      h,0
    ld      l,(ix+-6)
    push    hl
    ld      h,0
    ld      l,(ix+-4)
    push    hl
    ld      h,0
    ld      l,(ix+-2)
    push    hl
    call    _set_tile
    pop     af
    pop     af
    pop     af
;             i = i + 1;
    ld      h,0
    ld      l,(ix+-2)
    inc     hl
    ld      (ix+-2),l
;         }
    pop     af
    jp      .l301
.l302:
;         j = j + 1;
    ld      h,0
    ld      l,(ix+-4)
    inc     hl
    ld      (ix+-4),l
;     }
    jp      .l299
.l300:
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; main() {
_main:
    push    ix
    ld      ix,0
    add     ix,sp
;     char iobank3_old = IO_BANK3;
    ld      b,$00
    ld      c,$F3
    in      a,(c)
    ld      h,0
    ld      l,a
    push    hl

;     init();
    call    _init
;     while (!quit)
.l305:
    ld      a,(_quit)
    ld      h,0
    ld      l,a
    ld      a,h
    or      l
    ld      hl,0
    jr      nz,.l307
    inc     l
.l307:
    ld      a,h
    or      l
    jp      z,.l306
;         play_marathon();
    call    _play_marathon
    jp      .l305
.l306:

;     IO_BANK3 = iobank3_old;
    ld      h,0
    ld      l,(ix+-2)
    ld      b,$00
    ld      c,$F3
    ld      a,l
    out     (c),a
;     IO_VCTRL = VCTRL_TEXT_EN;
    ld      hl,1
    ld      b,$00
    ld      c,$E0
    ld      a,l
    out     (c),a
; }
.return:
    ld      sp,ix
    pop     ix
    ret

; // clang-format off
; #asm
    include "../tiledata.asm"
    include "/sdk/cb/lib/memcpy.asm"
    include "/sdk/cb/lib/itoa.asm"
; #endasm
;     // clang-format on
    
; --- Support functions ---
__multsi:
    ld      c,l
    ld      b,h
    xor     a
    ld      l,a
    or      b
    ld      b,16
    jr      nz,.l309
    ld      b,8
    ld      a,c
.l308:
    add     hl,hl
.l309:
    rl      c
    rla
    jr      nc,.l310
    add     hl,de
.l310:
    djnz    .l308
    ret
__divu16:
    ld      a,e
    and     $80
    or      d
    jr      nz,.l313
    ld      b,16
    adc     hl,hl
.l311:
    rla
    sub     e
    jr      nc,.l312
    add     a,e
.l312:
    ccf
    adc     hl,hl
    djnz    .l311
    ld      e,a
    ex      de,hl
    ret
.l313:
    ld      b,9
    ld      a,l
    ld      l,h
    ld      h,0
    rr      l
.l314:
    adc     hl,hl
    sbc     hl,de
    jr      nc,.l315
    add     hl,de
.l315:
    ccf
    rla
    djnz    .l314
    rl      b
    ld      d,b
    ld      e,a
    ret
__divsi:
    ld      a,h
    xor     d
    rla
    ld      a,h
    push    af
    rla
    jr      nc,.l316
    sub     a
    sub     l
    ld      l,a
    sbc     a,a
    sub     h
    ld      h,a
.l316:
    bit     7,d
    jr      z,.l317
    sub     a,a
    sub     a,e
    ld      e,a
    sbc     a,a
    sub     a,d
    ld      d,a
.l317:
    call    __divu16
    pop     af
    jr      nc,.l318
    ld      b,a
    sub     a
    sub     e
    ld      e,a
    sbc     a,a
    sub     d
    ld      d,a
    ld      a,b
.l318:
    ex      de,hl
    ret
__modsi:
    call    __divsi
    ex      de,hl
    rla
    ret     nc
    ex      de,hl
    sub     a
    sub     e
    ld      e,a
    sbc     a,a
    sub     d
    ld      d,a
    ex      de,hl
    ret
__shl:
    ld      b,e
    inc     b
    jr      .l320
.l319:
    add     hl,hl
.l320:
    djnz    .l319
    ret
__shr:
    ld      b, e
    inc     b
    jr      .l322
.l321:
    sra     h
    rr      l
.l322:
    djnz    .l321
    ret
__lts:
    ld      a,l
    sub     e
    ld      a,h
    sbc     a,d
    jp      po,.l323
    xor     $80
.l323:
    rlca
    and     1
    ld      l,a
    ld      h,0
    ret
__les:
    ld      a,e
    sub     l
    ld      a,d
    sbc     a,h
    jp      po,.l324
    xor     $80
.l324:
    rlca
    and     1
    xor     1
    ld      l,a
    ld      h,0
    ret
__gts:
    ld      a,e
    sub     l
    ld      a,d
    sbc     a,h
    jp      po,.l325
    xor     $80
.l325:
    rlca
    and     1
    ld      l,a
    ld      h,0
    ret
__ges:
    ld      a,l
    sub     e
    ld      a,h
    sbc     a,d
    jp      po,.l326
    xor     $80
.l326:
    rlca
    and     1
    xor     1
    ld      l,a
    ld      h,0
    ret
