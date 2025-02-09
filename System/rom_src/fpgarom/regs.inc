;-----------------------------------------------------------------------------
; regs.inc
;
; This file contains locations of hardware registers.
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; IO registers
;-----------------------------------------------------------------------------
IO_VCTRL            equ $E0
IO_VSCRX_L          equ $E1
IO_VSCRX_H          equ $E2
IO_VSCRY            equ $E3
IO_VSPRSEL          equ $E4
IO_VSPRX_L          equ $E5
IO_VSPRX_H          equ $E6
IO_VSPRY            equ $E7
IO_VSPRIDX          equ $E8
IO_VSPRATTR         equ $E9
IO_VPALSEL          equ $EA
IO_VPALDATA         equ $EB
IO_VLINE            equ $EC     ; Read-only
IO_PCMDAC           equ $EC     ; Write-only
IO_VIRQLINE         equ $ED
IO_IRQMASK          equ $EE
IO_IRQSTAT          equ $EF
IO_BANK0            equ $F0
IO_BANK1            equ $F1
IO_BANK2            equ $F2
IO_BANK3            equ $F3
IO_ESPCTRL          equ $F4
IO_ESPDATA          equ $F5
IO_PSG1DATA         equ $F6
IO_PSG1ADDR         equ $F7
IO_PSG2DATA         equ $F8
IO_PSG2ADDR         equ $F9
IO_KEYBUF           equ $FA
IO_SYSCTRL          equ $FB
IO_CASSETTE         equ $FC
IO_CPM              equ $FD     ; Write-only
IO_VSYNC            equ $FD     ; Read-only
IO_PRINTER          equ $FE
IO_SCRAMBLE         equ $FF     ; Write-only
IO_KEYBOARD         equ $FF     ; Read-only

VCTRL_TEXT_EN       equ (1<<0)
VCTRL_MODE_OFF      equ (0<<1)
VCTRL_MODE_TILE     equ (1<<1)
VCTRL_MODE_BM       equ (2<<1)
VCTRL_SPR_EN        equ (1<<3)
VCTRL_TEXT_PRIO     equ (1<<4)

;-----------------------------------------------------------------------------
; Other definitions
;-----------------------------------------------------------------------------
BANK0_BASE          equ $0000
BANK1_BASE          equ $4000
BANK2_BASE          equ $8000
BANK3_BASE          equ $C000

BANK_READONLY       equ $80     ; Bank readonly
BANK_OVERLAY        equ $40     ; RAM overlay

;-----------------------------------------------------------------------------
; ESP32 commands
;-----------------------------------------------------------------------------
ESPCMD_RESET        equ $01     ; Reset ESP
ESPCMD_VERSION      equ $02     ; Get version string
ESPCMD_GETDATETIME  equ $03     ; Get current date/time
ESPCMD_KEYMODE      equ $08     ; Set keyboard buffer mode
ESPCMD_GETMOUSE     equ $0C     ; Get mouse state
ESPCMD_OPEN         equ $10     ; Open / create file
ESPCMD_CLOSE        equ $11     ; Close open file
ESPCMD_READ         equ $12     ; Read from file
ESPCMD_WRITE        equ $13     ; Write to file
ESPCMD_SEEK         equ $14     ; Move read/write pointer
ESPCMD_TELL         equ $15     ; Get current read/write
ESPCMD_OPENDIR      equ $16     ; Open directory
ESPCMD_CLOSEDIR     equ $17     ; Close open directory
ESPCMD_READDIR      equ $18     ; Read from directory
ESPCMD_DELETE       equ $19     ; Remove file or directory
ESPCMD_RENAME       equ $1A     ; Rename / move file or directory
ESPCMD_MKDIR        equ $1B     ; Create directory
ESPCMD_CHDIR        equ $1C     ; Change directory
ESPCMD_STAT         equ $1D     ; Get file status
ESPCMD_GETCWD       equ $1E     ; Get current working directory
ESPCMD_CLOSEALL     equ $1F     ; Close any open file/directory descriptor
ESPCMD_OPENDIR83    equ $20     ; Open directory in 8.3 mode
ESPCMD_LOADFPGA     equ $40     ; Load FPGA bitstream

ERR_NOT_FOUND       equ -1      ; File / directory not found
ERR_TOO_MANY_OPEN   equ -2      ; Too many open files / directories
ERR_PARAM           equ -3      ; Invalid parameter
ERR_EOF             equ -4      ; End of file / directory
ERR_EXISTS          equ -5      ; File already exists
ERR_OTHER           equ -6      ; Other error
ERR_NO_DISK         equ -7      ; No disk
ERR_NOT_EMPTY       equ -8      ; Not empty
ERR_WRITE_PROTECT   equ -9      ; Write protected SD-card

FO_RDONLY           equ $00     ; Open for reading only
FO_WRONLY           equ $01     ; Open for writing only
FO_RDWR             equ $02     ; Open for reading and writing
FO_ACCMODE          equ $03     ; Mask for above modes
FO_APPEND           equ $04     ; Append mode
FO_CREATE           equ $08     ; Create if non-existant
FO_TRUNC            equ $10     ; Truncate to zero length
FO_EXCL             equ $20     ; Error if already exists
