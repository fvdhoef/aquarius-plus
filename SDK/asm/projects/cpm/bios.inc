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
HOME:       jp  _home
SELDSK:     jp  _seldsk
SETTRK:     jp  _settrk
SETSEC:     jp  _setsec
SETDMA:     jp  _setdma
READ:       jp  _read
WRITE:      jp  _write
PRSTAT:     jp  _prstat
SECTRN:     jp  _sectran

;-----------------------------------------------------------------------------
; Variables used by BIOS
;-----------------------------------------------------------------------------
_dma_addr:  defw 0
_keyval:    defb 0
_text_addr: defw $3000
_text_col:  defb 0
_text_row:  defb 0

_diskno:    defb 0
_track:     defb 0
_sector:    defb 0

;-----------------------------------------------------------------------------
; Disk parameter header
;-----------------------------------------------------------------------------
_dph_disk0:
    defw    _trans  ; XLT
    defw    0,0,0   ; Scratch area
    defw    _dirbuf ; Directory operation buffer
    defw    _dpb    ; Disk parameter block
    defw    _chk00  ; 
    defw    _all00

_trans:
    defb     1,  7, 13, 19  ;SECTORS  1,  2,  3,  4
    defb    25,  5, 11, 17  ;SECTORS  5,  6,  7,  6
    defb    23,  3,  9, 15  ;SECTORS  9, 10, 11, 12
    defb    21,  2,  8, 14  ;SECTORS 13, 14, 15, 16
    defb    20, 26,  6, 12  ;SECTORS 17, 18, 19, 20
    defb    18, 24,  4, 10  ;SECTORS 21, 22, 23, 24
    defb    16, 22          ;SECTORS 25, 26

;-----------------------------------------------------------------------------
; Disk parameter block
;-----------------------------------------------------------------------------
_dpb:
    defw    26      ; SPT: total number of sectors per track. 
    defb    3       ; BSH: data allocation block shift factor, determined by the data block allocation size.
    defb    7       ; BLM: data allocation block mask ((2^BSH)-1).
    defb    0       ; EXM: extent mask, determined by the data block allocation size and the number of disk blocks.
    defw    242     ; DSM: determines the total storage capacity of the disk drive.
    defw    63      ; DRM: determines the total number of directory entries that can be stored on this drive.
    defb    192     ; AL0: reserved directory blocks
    defb    0       ; AL1: reserved directory blocks
    defw    16      ; CKS: size of the directory check vector
    defw    2       ; OFF: number of reserved tracks at the beginning of the (logical) disk

_dirbuf:    defs    128     ; Scratch directory area
_chk00:     defs    16      ; Check vector 0
_all00:     defs    31      ; Allocation vector 0

;-----------------------------------------------------------------------------
; Initialization values for low storage area ($0000-$0007)
;-----------------------------------------------------------------------------
__low_storage_values:
    jp      WBOOT   ; Jump to BIOS
    defb    0       ; IO byte
    defb    0       ; DSK byte (current default drive)
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

    ; Open disk image
    ld      a,ESPCMD_OPEN
    call    esp_cmd
    ld      a,FO_RDWR           ; Flags (read-only)
    call    esp_send_byte
    ld      hl,_filename
    call    _esp_send_string
    call    esp_get_byte

    ret

_filename: defb "cpm2-1.dsk",0

; Send string in HL
_esp_send_string:
    ld      a,(hl)
    call    esp_send_byte
    or      a
    ret     z
    inc     hl
    jr      _esp_send_string

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
    xor     a
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


_home:
    ld      c,0
    call    SETTRK
    ret

_seldsk:
    ld      hl,0            ; error return code
    ld      a,c
    ld      (_diskno),a
    cp      1  ;4           ; must be between 0-3
    ret     nc
    ld      hl, _dph_disk0
    ret

_settrk:
    ld      a,c
    ld      (_track),a
    ret

_setsec:
    ld      a,c
    ld      (_sector),a
    ret

_sectran:
    ex      de,hl
    add     hl,bc
    ld      l,(hl)
    ld      h,0
    ret

_read:
    call    _seek_disk
    xor     a
    ld      hl,(_dma_addr)
    ld      de,128
    call    _esp_read_bytes
    xor     a
    ret

_write:
    call    _seek_disk
    xor     a
    ld      hl,(_dma_addr)
    ld      de,128
    call    _esp_write_bytes
    xor     a
    ret

_seek_disk:
    ld      d,0
    ld      a,(_track)
    ld      e,a
    ld      bc,26
    call    _mul16
    ld      d,0
    ld      a,(_sector)
    dec     a
    ld      e,a
    add     hl,de
    ex      de,hl
    xor     a
    jp      _seek

; This routine performs the operation DEHL=BC*DE
_mul16:
    ld hl,0
    ld a,16
.1: add hl,hl
    rl e
    rl d
    jp nc,.2
    add hl,bc
    jp nc,.2
    inc de      ; This instruction (with the jump) is like an "ADC DE,0"
.2: dec a
    jp nz,.1
    ret


;-----------------------------------------------------------------------------
; Seek to record DE (file descriptor in A)
;-----------------------------------------------------------------------------
_seek:
    ld      b,a
    ld      a,ESPCMD_SEEK
    call    esp_cmd
    ld      a,b
    call    esp_send_byte   ; File descriptor

    ; a = (de << 7) & 0xFF
    ld      a,e
    rrca
    and     $80
    call    esp_send_byte

    ; a = (de >> 1) & 0xFF
    ld      a,d
    rra                     ; Shift lowest bit in B into carry
    ld      a,e
    rra
    call    esp_send_byte

    ; a = (de >> 9) & 0xFF
    ld      a,d
    rra
    and     $7F
    call    esp_send_byte

    xor     a
    call    esp_send_byte

    call    esp_get_byte
    ret

;-----------------------------------------------------------------------------
; Read bytes
; Input:  A: file descriptor
;         HL: destination address
;         DE: number of bytes to read
; Output: HL: next address (start address if no bytes read)
;         DE: number of bytes actually read
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
_esp_read_bytes:
    ld      b,a
    ld      a,ESPCMD_READ
    call    esp_cmd
    ld      a,b

    ; Send file descriptor
    call    esp_send_byte

    ; Send read size
    ld      a,e
    call    esp_send_byte
    ld      a,d
    call    esp_send_byte

    ; Get result
    call    esp_get_byte
    or      a
    ret     nz

    ; Get number of bytes actual read
    call    esp_get_byte
    ld      e,a
    call    esp_get_byte
    ld      d,a

    push    de

.loop:
    ; Done reading? (DE=0)
    ld      a,d
    or      a,e
    jr      z,.done

    call    esp_get_byte
    ld      (hl),a
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de

    xor     a
    ret

;-----------------------------------------------------------------------------
; Send bytes
; Input:  HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;-----------------------------------------------------------------------------
_esp_send_bytes:
    push    de

.loop:
    ; Done sending? (DE=0)
    ld      a, d
    or      a, e
    jr      z, .done

    ld      a, (hl)
    call    esp_send_byte
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de
    ret

;-----------------------------------------------------------------------------
; Write bytes
; Input:  A: file descriptor
;         HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
_esp_write_bytes:
    ld      b,a
    ld      a, ESPCMD_WRITE
    call    esp_cmd
    ld      a,b

    ; Send file descriptor
    call    esp_send_byte

    ; Send write size
    ld      a, e
    call    esp_send_byte
    ld      a, d
    call    esp_send_byte

    ; Send bytes
    call    _esp_send_bytes

    ; Get result
    call    esp_get_byte
    or      a
    ret     nz

    ; Get number of bytes actual written
    call    esp_get_byte
    ld      e, a
    call    esp_get_byte
    ld      d, a

    xor     a
    ret
