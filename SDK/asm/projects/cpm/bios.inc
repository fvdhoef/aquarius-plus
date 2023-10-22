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
HOME:       jp  HOME
SELDSK:     jp  SELDSK
SETTRK:     jp  SETTRK
SETSEC:     jp  SETSEC
SETDMA:     jp  _setdma
READ:       jp  READ
WRITE:      jp  WRITE
PRSTAT:     jp  PRSTAT
SECTRN:     jp  SECTRN

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
; Initialize
;-----------------------------------------------------------------------------
_init:
    ; Set video mode to 80-columns mode text with remapped border character
    ld      a,$61
    out     (IO_VCTRL),a

    ; Setup low storage area
    ld      de,$0
    ld      hl,__low_storage_values
    ld      bc,__low_storage_values_end - __low_storage_values
    ldir

    ; Default dma address is $80
    ld      bc,$80
    call    _setdma

    ; Close any open descriptor
    ld      a,ESPCMD_CLOSEALL
    call    esp_cmd
    call    esp_get_byte

    ; Enable keyboard repeat
    ld      a,ESPCMD_KEYMODE
    call    esp_cmd
    ld      a,7
    call    esp_send_byte
    call    esp_get_byte

    ret

;-----------------------------------------------------------------------------
; Copy CP/M to page 36 so it can be reloaded by _load_cpm.
;-----------------------------------------------------------------------------
_save_cpm:
    ld      a,36
    out     (IO_BANK0),a
    ld      de,$0
    ld      hl,ccp
    ld      bc,bios-ccp
    ldir
    ld      a,32
    out     (IO_BANK0),a
    ret

;-----------------------------------------------------------------------------
; Load CP/M from page 36
;-----------------------------------------------------------------------------
_load_cpm:
    ld      a,36
    out     (IO_BANK0),a
    ld      de,ccp
    ld      hl,0
    ld      bc,bios-ccp
    ldir
    ld      a,32
    out     (IO_BANK0),a
    ret

;-----------------------------------------------------------------------------
; BOOT - Cold start routine
;-----------------------------------------------------------------------------
_boot:
    ld      sp,$0           ; Setup a stack
    call    _save_cpm       ; Save CP/M for later reloading by warm boot

    ; Clear user memory
    ld      hl,$0
    ld      bc,ccp
    ld      a,0
    call    memset

    call    _init           ; Initialize
    ld      bc,.bootstr     ; Boot message
    call    PRTMESG
    jr      _go_ccp

.bootstr: defb "CP/M 2.2 for Aquarius+",13,10,"$"

;-----------------------------------------------------------------------------
; WBOOT - Warm boot - reload command processor
;
; Reloads the command processor and (on some systems) the BDOS as well.
; How it does this is implementation-dependent; it may use the reserved tracks
; of a floppy disc or extra memory.
;-----------------------------------------------------------------------------
_wboot:
    ld      sp,$0           ; Setup a stack
    call    _load_cpm       ; Load CP/M
    call    _init           ; Initialize

_go_ccp:
    ; Jump to CCP
    ld      c,0
    jp      CBASE+3     ; Clear input buffer before starting CCP

;-----------------------------------------------------------------------------
; CONST - Console status
;
; Returns its status in A; 0 if no character is ready, $FF if one is.
;-----------------------------------------------------------------------------
_const:
    ; First check _keyval buffer
    ld      a,(_keyval)
    or      a
    jr      nz,.haschar

    ; Check KEYBUF IO port
    in      a,(IO_KEYBUF)
    or      a
    ret     z   ; No key

    ; Save value in _keyval buffer
    ld      (_keyval),a

.haschar:
    ld      a,$FF
    ret

;-----------------------------------------------------------------------------
; CONIN - Console input
;
; Wait until the keyboard is ready to provide a character, and return it in A.
;-----------------------------------------------------------------------------
_conin:
    call    _const
    or      a
    jr      z,_conin
    ld      a,(_keyval)
    ld      b,a
    xor     a
    ld      (_keyval),a
    ld      a,b
    ret

_saved_col: .defb $8F

;-----------------------------------------------------------------------------
; Hide cursor
;-----------------------------------------------------------------------------
_hide_cursor:
    ; Restore saved color at cursor position
    ld      a,$E1
    out     (IO_VCTRL),a

    ld      hl,(_text_addr)
    ld      a,(_saved_col)
    ld      (hl),a

    ld      a,$61
    out     (IO_VCTRL),a
    ret

;-----------------------------------------------------------------------------
; Show cursor
;-----------------------------------------------------------------------------
_show_cursor:
    ; Save color at cursor position
    ld      a,$E1
    out     (IO_VCTRL),a

    ld      hl,(_text_addr)
    ld      a,(hl)
    ld      (_saved_col),a

    ; Set color at cursor position
    ld      a,$F9
    ld      (hl),a

    ld      a,$61
    out     (IO_VCTRL),a
    ret

_oldstack: defw 0
    defs 16
_tmpstack:

;-----------------------------------------------------------------------------
; CONOUT - Console output
;
; Write the character in C to the screen.
;-----------------------------------------------------------------------------
_conout:
    ; We need to set up a temporary stack pointer here, since the current
    ; stack pointer can be in the bottom 16KB of memory and we are switching
    ; that out in this routine.

    ; Save stack pointer
    ld      hl,0
    add     hl,sp
    ld      (_oldstack),hl
    ; Set temp stack pointer
    ld      sp,_tmpstack

    ; Save bank 0 and set to screen memory
    in      a,(IO_BANK0)
    push    a
    ld      a,$C0
    out     (IO_BANK0),a

    ; Hide cursor
    call    _hide_cursor

    ; Write to text address
    ld      hl,(_text_addr)
    ld      a,c
    cp      a,13       ; CR?
    jr      z,.cr
    cp      a,10
    jr      z,.lf      ; LF?
    cp      a,8
    jp      z,.bs      ; BS?
    ld      (hl),a

    ; Increment text address
    inc     hl
    ld      (_text_addr),hl

    ld      a,(_text_col)
    inc     a
    cp      80
    jr      z,.next_line
    ld      (_text_col),a

.done:
    call    _show_cursor

    ; Restore bank 0
    pop     a
    out     (IO_BANK0),a

    ; Restore stack point
    ld      hl,(_oldstack)
    ld      sp,hl
    ret

.next_line:
    xor     a
    ld      (_text_col),a
    ld      a,(_text_row)
    inc     a
    cp      25
    jr      z,.next_line_scroll
    ld      (_text_row),a
    jr      .done

.next_line_scroll:
    call    .do_scroll
    ld      a,24
    ld      (_text_row),a
    ld      hl,$3000 + 80 * 24
    ld      (_text_addr),hl
    jr      .done

.cr:
    ; Cursor back to start of line
    ld      hl,(_text_addr)
    ld      a,(_text_col)
    ld      c,a
    xor     a
    ld      (_text_col),a
    ld      b,a
    sbc     hl,bc
    ld      (_text_addr),hl
    jr      .done

.lf:
    ; Cursor to next line
    ld      a,(_text_row)
    cp      24
    jr      z,.lf_scroll
    inc     a
    ld      (_text_row),a

    ld      hl,(_text_addr)
    ld      bc,80
    or      a   ; Clear carry
    adc     hl,bc
    ld      (_text_addr),hl
    jr      .done

.lf_scroll:
    call    .do_scroll
    jr      .done

.do_scroll:
    ld      de,$3000
    ld      hl,$3000 + 80
    ld      bc,80 * 24
    ldir
    ld      hl,$3000 + 80 * 24
    ld      bc,80
    ld      a,' '
    call    memset
    ret

.bs:
    ; Backspace
    ld      a,(_text_col)
    or      a                   ; At first column?
    jr      z,.bs2
    dec     a                   ; Decrement column
.bs3:
    ld      (_text_col),a       ; Update column

    ld      hl,(_text_addr)     ; Decrement text address
    dec     hl
    ld      (_text_addr),hl
.bs_done:
    jp      .done

.bs2:
    ld      a,(_text_row)
    or      a                   ; At first row?
    jr      z,.bs_done
    dec     a                   ; Decrement row
    ld      (_text_row),a
    ld      a,79                ; Update column to 79
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
    ld      a,$FF
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
    ld      a,26
    ret

;-----------------------------------------------------------------------------
; SETDMA - Set DMA address
;
; The next disc operation will read its data from (or write its data to)
; the address given in BC.
;-----------------------------------------------------------------------------
_setdma:
    ld      (_dma_addr),bc
    ret

;-----------------------------------------------------------------------------
; memset
;-----------------------------------------------------------------------------
memset:
    inc     c
    inc     b
    jr      .start
.repeat:
    ld      (hl),a
    inc     hl
.start:
    dec     c
    jr      nz,.repeat
    dec     b
    jr      nz,.repeat
    ret

;-----------------------------------------------------------------------------
; Issue command to ESP
;-----------------------------------------------------------------------------
esp_cmd:
    push    a

    ; Drain RX FIFO
.drain:
    in      a,(IO_ESPCTRL)
    and     a,1
    jr      z,.done
    in      a,(IO_ESPDATA)
    jr      .drain
.done:

    ; Issue start of command
    ld      a,$80
    out     (IO_ESPCTRL),a

    ; Issue command
    pop     a
    jp      esp_send_byte

;-----------------------------------------------------------------------------
; Wait for data from ESP
;-----------------------------------------------------------------------------
esp_get_byte:
.wait:
    in      a,(IO_ESPCTRL)
    and     a,1
    jr      z,.wait
    in      a,(IO_ESPDATA)
    ret

;-----------------------------------------------------------------------------
; Write data to ESP
;-----------------------------------------------------------------------------
esp_send_byte:
    push    a

.wait:
    in      a,(IO_ESPCTRL)
    and     a,2
    jr      nz,.wait

    pop     a
    out     (IO_ESPDATA),a
    ret

;-----------------------------------------------------------------------------
; Get bytes
; Input:  HL: destination address
;         DE: number of bytes to read
;-----------------------------------------------------------------------------
esp_get_bytes:
.loop:
    ; Done reading? (DE=0)
    ld      a,d
    or      a,e
    ret     z

    call    esp_get_byte
    ld      (hl),a
    inc     hl
    dec     de
    jr      .loop
