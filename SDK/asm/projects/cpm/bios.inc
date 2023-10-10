    include "regs.inc"

;-----------------------------------------------------------------------------
; BIOS jump table
;-----------------------------------------------------------------------------
BOOT:   jp  _boot
WBOOT:  jp  _wboot
CONST:  jp  _const
CONIN:  jp  _conin
CONOUT: jp  _conout
LIST:   jp  _list
PUNCH:  jp  _punch
READER: jp  _reader
HOME:   jp  _home
SELDSK: jp  _seldsk
SETTRK: jp  _settrk
SETSEC: jp  _setsec
SETDMA: jp  _setdma
READ:   jp  _read
WRITE:  jp  _write
PRSTAT: jp  _prstat
SECTRN: jp  _sectrn

_dma_addr:  dw 0
_keyval:    db 0

;-----------------------------------------------------------------------------
; _memset
;-----------------------------------------------------------------------------
_memset:
    inc     c
    inc     b
    jr      .start
.repeat:
    ld      (hl), a
    inc     hl
.start:
    dec     c
    jr      nz, .repeat
    dec     b
    jr      nz, .repeat
    ret

;-----------------------------------------------------------------------------
; Initialization values for low storage area ($0000-$0007)
;-----------------------------------------------------------------------------
__low_storage_values:
    jp      $F200   ; Jump to BIOS
    db      0       ; IO byte
    db      0       ; DSK byte (current default drive)
    jp      $E400   ; Jump to BDOS
__low_storage_values_end:

;-----------------------------------------------------------------------------
; BOOT - Cold start routine
;-----------------------------------------------------------------------------
_boot:
    ; Setup a stack
    ld      sp, $0

    ; Clear user memory
    ld      hl, $0
    ld      bc, $DC00
    ld      a, 0
    call    _memset

    ; Setup low storage area
    ld  de, $0
    ld  hl, __low_storage_values
    ld  bc, __low_storage_values_end - __low_storage_values
    ldir

    ; Jump to CCP
    jp      $DC00

.loop:
    jr      .loop

;-----------------------------------------------------------------------------
; WBOOT - Warm boot - reload command processor
;
; Reloads the command processor and (on some systems) the BDOS as well.
; How it does this is implementation-dependent; it may use the reserved tracks
; of a floppy disc or extra memory.
;-----------------------------------------------------------------------------
_wboot:
    jr _wboot

;-----------------------------------------------------------------------------
; CONST - Console status
;
; Returns its status in A; 0 if no character is ready, $FF if one is.
;-----------------------------------------------------------------------------
_const:
    ; First check _keyval buffer
    ld      a, (_keyval)
    or      a
    jr      nz, .haschar

    ; Check KEYBUF IO port
    in      a, (IO_KEYBUF)
    or      a
    ret     z   ; No key

    ; Save value in _keyval buffer
    ld      (_keyval), a

.haschar:
    ld      a, $FF
    ret

;-----------------------------------------------------------------------------
; CONIN - Console input
;
; Wait until the keyboard is ready to provide a character, and return it in A.
;-----------------------------------------------------------------------------
_conin:
    jr      const
    or      a
    jr      z, _conin
    ld      a, (_keyval)
    ret

;-----------------------------------------------------------------------------
; CONOUT - Console output
;
; Write the character in C to the screen.
;-----------------------------------------------------------------------------
_conout:
    jr _conout

;-----------------------------------------------------------------------------
; LIST - Printer output
;
; Write the character in C to the printer. If the printer isn't ready,
; wait until it is.
;-----------------------------------------------------------------------------
_list:       jr _list

;-----------------------------------------------------------------------------
; PUNCH - Paper tape punch output
;
; Write the character in C to the "paper tape punch" - or whatever the current
; auxiliary device is. If the device isn't ready, wait until it is.
;-----------------------------------------------------------------------------
_punch:      jr _punch

;-----------------------------------------------------------------------------
; READER - Paper tape reader input
;
; Read a character from the "paper tape reader" - or whatever the current
; auxiliary device is. If the device isn't ready, wait until it is. The
; character will be returned in A. If this device isn't implemented, return
; character 26 (^Z).
;-----------------------------------------------------------------------------
_reader:     jr _reader

;-----------------------------------------------------------------------------
; HOME - Move disc head to track 0
;
; Move the current drive to track 0
;-----------------------------------------------------------------------------
_home:       jr _home

;-----------------------------------------------------------------------------
; SELDSK - Select disc drive
;
; Select the disc drive in register C (0=A:, 1=B: ...).
; Called with E=0 or $FFFF.
;
; If bit 0 of E is 0, then the disc is logged in as if new;
; if the format has to be determined from the boot sector, for example,
; this will be done.
;
; If bit 0 if E is 1, then the disc has been logged in before.
; The disc is not accessed; the DPH address (or zero) is returned immediately.
;
; SELDSK returns the address of a Disc Parameter Header in HL.
; The exact format of a DPH varies between CP/M versions.
;
; If the disc could not be selected it returns HL=0.
;-----------------------------------------------------------------------------
_seldsk:
    ld      hl, 0
    ret

;-----------------------------------------------------------------------------
; SETTRK - Set track number
;
; Set the track in BC (0-based).
;-----------------------------------------------------------------------------
_settrk:
    jr _settrk

;-----------------------------------------------------------------------------
; SETSEC - Set sector number
;
; Set the sector in BC. Under CP/M 1 and 2 a sector is 128 bytes.
;-----------------------------------------------------------------------------
_setsec:
    jr _setsec

;-----------------------------------------------------------------------------
; SETDMA - Set DMA address
;
; The next disc operation will read its data from (or write its data to)
; the address given in BC.
;-----------------------------------------------------------------------------
_setdma:
    ld (_dma_addr), bc
    ret

;-----------------------------------------------------------------------------
; READ - Read a sector
;
; Read the currently set track and sector at the current DMA address.
; Returns A=0 for OK, 1 for unrecoverable error, $FF if media changed.
;-----------------------------------------------------------------------------
_read:
    jr _read

;-----------------------------------------------------------------------------
; WRITE - Write a sector
;
; Write the currently set track and sector. C contains a deblocking code:
; C=0 - Write can be deferred
; C=1 - Write must be immediate
; C=2 - Write can be deferred, no pre-read is necessary.
; Returns A=0 for OK, 1 for unrecoverable error, 2 if disc is readonly,
; $FF if media changed.
;-----------------------------------------------------------------------------
_write:
    jr _write

;-----------------------------------------------------------------------------
; LISTST - Status of list device
;
; Return status of current printer device.
; Returns A=0 (not ready) or A=$FF (ready).
;-----------------------------------------------------------------------------
_prstat:
    ld      a, 0
    ret

;-----------------------------------------------------------------------------
; SECTRAN - Sector translation for skewing
;
; Translate sector numbers to take account of skewing.
; On entry, BC=logical sector number (zero based) and DE=address of
; translation table. On exit, HL contains physical sector number. On a system
; with hardware skewing, this would normally ignore DE and return either
; BC or BC+1.
;-----------------------------------------------------------------------------
_sectrn:
    push    hl
    pop     bc
    ret
