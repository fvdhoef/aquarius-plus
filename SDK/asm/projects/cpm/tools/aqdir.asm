;-----------------------------------------------------------------------------
; aqdir.asm
;-----------------------------------------------------------------------------
    include "regs.inc"

    org     $100
    jp      _entry

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
_dir_dd:    defb 0

_dirent:
_dirent_date: defw 0
_dirent_time: defw 0
_dirent_attr: defb 0
_dirent_size: defd 0
_dirent_name: defs 8
_dirent_ext:  defs 3
_dirent_end:

_count: defb 0

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
_entry:
    ld      a, ESPCMD_OPENDIR83
    call    esp_cmd

    ; Get argument, skip spaces
    ld      hl,$81
    call    _skip_spaces
    call    esp_send_str
    call    esp_get_byte
    ld      (_dir_dd),a

    call    _dir_listing

    ld      a, ESPCMD_CLOSEDIR
    call    esp_cmd
    ld      a,(_dir_dd)
    call    esp_send_byte
    call    esp_get_byte
    ret

;-----------------------------------------------------------------------------
; _skip_spaces
;-----------------------------------------------------------------------------
_skip_spaces:
    ld      a,(hl)
    or      a
    ret     z
    cp      ' '
    ret     nz
    inc     hl
    jr      _skip_spaces

;-----------------------------------------------------------------------------
; _dir_listing
;-----------------------------------------------------------------------------
_dir_listing:
    xor     a
    ld      (_count),a

.1: call    _read_dirent
    ret     nz

    ; Print 'S' at start of line
    ld      a,(_count)
    or      a
    jr      nz, .6
    ld      a,'S'
    call    _putchar

    ; Print ':' separator
.6: ld      a,':'
    call    _putchar

    ; Print '<' if dir, otherwise ' '
    ld      a,(_dirent_attr)
    bit     0,a
    jr      z,.2
    ld      a,'<'
    call    _putchar
    jr      .3
.2: ld      a,' '
    call    _putchar
.3:
    ; Print filename and extension
    ld      c,8
    ld      hl,_dirent_name
    call    _put_bytes
    ld      a,' '
    call    _putchar
    ld      c,3
    ld      hl,_dirent_ext
    call    _put_bytes

    ; Print '>' if dir, otherwise ' '
    ld      a,(_dirent_attr)
    bit     0,a
    jr      z,.4
    ld      a,'>'
    call    _putchar
    jr      .5
.4: ld      a,' '
    call    _putchar
.5:
    ; Newline every 5 columns
    ld      a,(_count)
    inc     a
    ld      (_count),a
    cp      5
    jr      nz,.1
    xor     a
    ld      (_count),a

    ; Print newline
    ld      a,13
    call    _putchar
    ld      a,10
    call    _putchar
    jr      .1

;-----------------------------------------------------------------------------
; _put_bytes
;-----------------------------------------------------------------------------
_put_bytes:
    ld      a,c
    or      a
    ret     z

    ld      a,(hl)
    push    bc
    push    hl
    call    _putchar
    pop     hl
    pop     bc
    inc     hl
    dec     c
    jr      _put_bytes

;-----------------------------------------------------------------------------
; _putchar
;-----------------------------------------------------------------------------
_putchar:
    ld      e,a
    ld      c,2
    jp      5

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
; Send string in HL to ESP
;-----------------------------------------------------------------------------
esp_send_str:
    ld      a,(hl)
    call    esp_send_byte
    or      a
    ret     z
    inc     hl
    jr      esp_send_str

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

;-----------------------------------------------------------------------------
; _read_dirent
;-----------------------------------------------------------------------------
_read_dirent:
    ld      a,ESPCMD_READDIR
    call    esp_cmd
    ld      a,(_dir_dd)
    call    esp_send_byte
    call    esp_get_byte
    or      a
    ret     nz

    ; Read date/time/attribute
    ld      hl,_dirent
    ld      de,9
    call    esp_get_bytes

    ld      hl,_dirent_name

    ; Read filename
    ld      c,8
.1: call    esp_get_byte
    cp      '.'
    jr      z,.pad2         ; Dot
    or      a
    jr      z,.pad1         ; Zero-byte
    ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.1

    call    esp_get_byte    ; Dot
    or      a
    jr      nz,.ext

.pad1:
    inc     c
    inc     c
    inc     c
    jr      .pad3

    ; Pad filename with spaces
.pad2:
    ld      a,' '
.2: ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.2

    ; Read extension
.ext:
    ld      c,3
.3: call    esp_get_byte
    or      a
    jr      z,.pad3         ; Zero-byte
    ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.3
    ret

.pad3:
    ; Pad extension with spaces
    ld      a,' '
.4: ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.4
    ret
