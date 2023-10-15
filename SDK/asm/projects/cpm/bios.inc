    include "regs.inc"

;-----------------------------------------------------------------------------
; BIOS jump table
;-----------------------------------------------------------------------------
BOOT:       jp  _boot
WBOOT:      jp  _wboot
CONST:      jp  _const
CONIN:      jp  _conin
CONOUT:     jp  _conout
LIST:       jp  _list
PUNCH:      jp  _punch
READER:     jp  _reader
; HOME:       jp  _home
; SELDSK:     jp  _seldsk
; SETTRK:     jp  _settrk
; SETSEC:     jp  _setsec
SETDMA:     jp  _setdma
; READ:       jp  _read
; WRITE:      jp  _write
; PRSTAT:     jp  _prstat
; SECTRN:     jp  _sectrn

;-----------------------------------------------------------------------------
; Variables used by BIOS
;-----------------------------------------------------------------------------
_dma_addr:  dw 0
_keyval:    db 0
_text_addr: dw $3000
_text_col:  db 0
_text_row:  db 0

;-----------------------------------------------------------------------------
; Initialization values for low storage area ($0000-$0007)
;-----------------------------------------------------------------------------
__low_storage_values:
    jp      WBOOT   ; Jump to BIOS
    db      0       ; IO byte
    db      0       ; DSK byte (current default drive)
    jp      FBASE   ; Jump to BDOS
__low_storage_values_end:

;-----------------------------------------------------------------------------
; BOOT - Cold start routine
;-----------------------------------------------------------------------------
_boot:
    ; Set video mode to text with remapped border character
    ld      a, $21
    out     (IO_VCTRL), a

    ; Setup a stack
    ld      sp, $0

    ; Clear user memory
    ld      hl, $0
    ld      bc, ccp
    ld      a, 0
    call    memset

    ; Setup low storage area
    ld  de, $0
    ld  hl, __low_storage_values
    ld  bc, __low_storage_values_end - __low_storage_values
    ldir

    ; Default dma address is $80
    ld      bc, $80
    call    _setdma

    ;
    ld      bc, .bootstr
    call    PRTMESG

    jr      _wboot

.bootstr: defb "CP/M 2.2 for Aquarius+",13,10,"$"


;-----------------------------------------------------------------------------
; WBOOT - Warm boot - reload command processor
;
; Reloads the command processor and (on some systems) the BDOS as well.
; How it does this is implementation-dependent; it may use the reserved tracks
; of a floppy disc or extra memory.
;-----------------------------------------------------------------------------
_wboot:
    ; Close any open descriptor
    ld      a, ESPCMD_CLOSEALL
    call    esp_cmd
    call    esp_get_byte

    ; Jump to CCP
    ld      c, 0
    jp      ccp

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
    call    _const
    or      a
    jr      z, _conin
    ld      a, (_keyval)
    ld      b, a
    xor     a
    ld      (_keyval), a
    ld      a, b
    ret

;-----------------------------------------------------------------------------
; CONOUT - Console output
;
; Write the character in C to the screen.
;-----------------------------------------------------------------------------
_conout:
    ; Save bank 0 and set to screen memory
    in      a, (IO_BANK0)
    push    a
    ld      a, $C0
    out     (IO_BANK0), a

    ; Write to text address
    ld      hl, (_text_addr)
    ld      a, c
    cp      a, 13       ; CR?
    jr      z, .cr
    cp      a, 10
    jr      z, .lf      ; LF?
    cp      a, 8
    jr      z, .bs      ; BS?
    ld      (hl), a

    ; Increment text address
    inc     hl
    ld      (_text_addr), hl

    ld      a, (_text_col)
    inc     a
    cp      40
    jr      z, .next_line
    ld      (_text_col), a

.done:
    ; Restore bank 0
    pop     a
    out     (IO_BANK0), a

    ret

.next_line:
    xor     a
    ld      (_text_col), a
    ld      a, (_text_row)
    inc     a
    cp      25
    jr      z, .next_line_scroll
    ld      (_text_row), a
    jr      .done

.next_line_scroll:
    call    .do_scroll
    ld      a, 24
    ld      (_text_row), a
    ld      hl, $3000 + 40 * 24
    ld      (_text_addr), hl
    jr      .done

.cr:
    ; Cursor back to start of line
    ld      hl, (_text_addr)
    ld      a, (_text_col)
    ld      c, a
    xor     a
    ld      (_text_col), a
    ld      b, a
    sbc     hl, bc
    ld      (_text_addr), hl
    jr      .done

.lf:
    ; Cursor to next line
    ld      a, (_text_row)
    cp      24
    jr      z, .lf_scroll
    inc     a
    ld      (_text_row), a

    ld      hl, (_text_addr)
    ld      bc, 40
    or      a   ; Clear carry
    adc     hl, bc
    ld      (_text_addr), hl
    jr      .done

.lf_scroll:
    call    .do_scroll
    jr      .done

.do_scroll:
    ld      de, $3000
    ld      hl, $3000 + 40
    ld      bc, 40 * 24
    ldir
    ld      hl, $3000 + 40 * 24
    ld      bc, 40
    ld      a, ' '
    call    memset
    ret

.bs:
    ; Backspace
    ld      a, (_text_col)
    or      a                   ; At first column?
    jr      z, .bs2
    dec     a                   ; Decrement column
.bs3:
    ld      (_text_col), a      ; Update column

    ld      hl, (_text_addr)    ; Decrement text address
    dec     hl
    ld      (_text_addr), hl
.bs_done:
    jp      .done

.bs2:
    ld      a, (_text_row)
    or      a                   ; At first row?
    jr      z, .bs_done
    dec     a                   ; Decrement row
    ld      (_text_row), a
    ld      a, 39               ; Update column to 39
    jr      .bs3



;-----------------------------------------------------------------------------
; LIST - Printer output
;
; Write the character in C to the printer. If the printer isn't ready,
; wait until it is.
;-----------------------------------------------------------------------------
_list:
    ret

;-----------------------------------------------------------------------------
; LISTST - Status of list device
;
; Return status of current printer device.
; Returns A=0 (not ready) or A=$FF (ready).
;-----------------------------------------------------------------------------
_prstat:
    ld      a, $FF
    ret

;-----------------------------------------------------------------------------
; PUNCH - Paper tape punch output
;
; Write the character in C to the "paper tape punch" - or whatever the current
; auxiliary device is. If the device isn't ready, wait until it is.
;-----------------------------------------------------------------------------
_punch:
    ret

;-----------------------------------------------------------------------------
; READER - Paper tape reader input
;
; Read a character from the "paper tape reader" - or whatever the current
; auxiliary device is. If the device isn't ready, wait until it is. The
; character will be returned in A. If this device isn't implemented, return
; character 26 (^Z).
;-----------------------------------------------------------------------------
_reader:
    ld      a, 26
    ret

;-----------------------------------------------------------------------------
; HOME - Move disc head to track 0
;
; Move the current drive to track 0
;-----------------------------------------------------------------------------
; _home:
;     xor     a
;     ld      (_cur_track + 0), a
;     ld      (_cur_track + 1), a
;     ret

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
; _seldsk:
;     ld      hl, _dph
;     ret

;-----------------------------------------------------------------------------
; SETTRK - Set track number
;
; Set the track in BC (0-based).
;-----------------------------------------------------------------------------
; _settrk:
;     ld      (_cur_track), bc
;     ret

;-----------------------------------------------------------------------------
; SETSEC - Set sector number
;
; Set the sector in BC. Under CP/M 1 and 2 a sector is 128 bytes.
;-----------------------------------------------------------------------------
; _setsec:
;     ld      (_cur_sector), bc
;     ret

;-----------------------------------------------------------------------------
; SETDMA - Set DMA address
;
; The next disc operation will read its data from (or write its data to)
; the address given in BC.
;-----------------------------------------------------------------------------
_setdma:
    ld      (_dma_addr), bc
    ret

;-----------------------------------------------------------------------------
; READ - Read a sector
;
; Read the currently set track and sector at the current DMA address.
; Returns A=0 for OK, 1 for unrecoverable error, $FF if media changed.
;-----------------------------------------------------------------------------
; _read:
;     ; TODO: READ

;     ld      a, 1
;     ; xor     a
;     ret
;     ; jr      _read

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
; _write:
;     jr      _write

;-----------------------------------------------------------------------------
; SECTRAN - Sector translation for skewing
;
; Translate sector numbers to take account of skewing.
; On entry, BC=logical sector number (zero based) and DE=address of
; translation table. On exit, HL contains physical sector number. On a system
; with hardware skewing, this would normally ignore DE and return either
; BC or BC+1.
;-----------------------------------------------------------------------------
; _sectrn:
;     push    hl
;     pop     bc
;     ret





;-----------------------------------------------------------------------------
; memset
;-----------------------------------------------------------------------------
memset:
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
; Issue command to ESP
;-----------------------------------------------------------------------------
esp_cmd:
    push    a

    ; Drain RX FIFO
.drain:
    in      a, (IO_ESPCTRL)
    and     a, 1
    jr      z, .done
    in      a, (IO_ESPDATA)
    jr      .drain
.done:

    ; Issue start of command
    ld      a, $80
    out     (IO_ESPCTRL), a

    ; Issue command
    pop     a
    jp      esp_send_byte

;-----------------------------------------------------------------------------
; Wait for data from ESP
;-----------------------------------------------------------------------------
esp_get_byte:
.wait:
    in      a, (IO_ESPCTRL)
    and     a, 1
    jr      z, .wait
    in      a, (IO_ESPDATA)
    ret

;-----------------------------------------------------------------------------
; Write data to ESP
;-----------------------------------------------------------------------------
esp_send_byte:
    push    a

.wait:
    in      a, (IO_ESPCTRL)
    and     a, 2
    jr      nz, .wait

    pop     a
    out     (IO_ESPDATA), a
    ret

;-----------------------------------------------------------------------------
; Get bytes
; Input:  HL: destination address
;         DE: number of bytes to read
;-----------------------------------------------------------------------------
esp_get_bytes:
.loop:
    ; Done reading? (DE=0)
    ld      a, d
    or      a, e
    ret     z

    call    esp_get_byte
    ld      (hl), a
    inc     hl
    dec     de
    jr      .loop
