;-----------------------------------------------------------------------------
; aqcopy.asm
;-----------------------------------------------------------------------------
    include "regs.inc"

    org     $100
    jp      _entry

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
_fcb:   defs 36     ; CPM file
_fd:    defb 0      ; ESP file

_idx:   defb 0
_tmp:   defb 0

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
_entry:
    ld      a,($5C)
    cp      5
    jp      z,_copy_to_cpm
    cp      4
    jr      nc,_invalid_argument
    ld      a,($5D)
    cp      ' '
    jp      nz,_copy_from_cpm
    jr      _invalid_argument

;-----------------------------------------------------------------------------
; _invalid_argument
;-----------------------------------------------------------------------------
_invalid_argument:
    ld      hl,.str
    call    _puts
    jp      0

.str:
    defb "Error: Invalid argument.",13,10
    defb "Syntax: AQCOPY <Source>",13,10
    defb "Source: If E:Name, copy from SD card to current CP/M disk, otherwise copy to SD card",13,10
    defb 0

;-----------------------------------------------------------------------------
; _not_found
;-----------------------------------------------------------------------------
_not_found:
    ld      hl,.str
    call    _puts
    jp      0

.str:
    defb "Error: File not found.",13,10,0

;-----------------------------------------------------------------------------
; _err_create
;-----------------------------------------------------------------------------
_err_create:
    ld      hl,.str
    call    _puts
    jp      0

.str:
    defb "Error: Creating file failed.",13,10,0

;-----------------------------------------------------------------------------
; _search_idx
;-----------------------------------------------------------------------------
_search_idx:
    xor     a
    ld      (_tmp),a

    ; Search first file
    ld      de,$5C
    call    _search_first
    cp      $FF
    jp      z,_not_found
    jr      .check

    ; Search next file
.next:
    ld      de,$5C
    call    _search_next
    cp      $FF
    jr      z,.end_of_dir
.check:
    ld      c,a

    ld      a,(_idx)
    ld      e,a
    ld      a,(_tmp)
    cp      e
    jr      z,.found
    inc     a
    ld      (_tmp),a
    jr      .next

.end_of_dir:
    jp      0

.found:
    ; Increment idx for next round
    ld      a,e
    inc     a
    ld      (_idx),a

    ; Get directory index
    ld      a,c

    ; Get entry in DMA buffer
    add     a
    add     a
    add     a
    add     a
    add     a
    add     $81
    ld      h,0
    ld      l,a

    ; Clear FCB
    push    hl
    ld      hl,_fcb
    ld      bc,36
    xor     a
    call    _memset
    pop     hl

    ; Copy name into FCB
    ld      a,($5C)
    ld      de,_fcb
    ld      (de),a
    inc     de
    ld      bc,11
    ldir

    ret

;-----------------------------------------------------------------------------
; _copy_from_cpm
;-----------------------------------------------------------------------------
_copy_from_cpm:
    call    _search_idx
    call    _copy_file_from_cpm
    jr      _copy_from_cpm

;-----------------------------------------------------------------------------
; _copy_file_from_cpm
;-----------------------------------------------------------------------------
_copy_file_from_cpm:
    ; Print entry
    ld      hl,_fcb+1
    ld      c,11
    call    _put_bytes

    ; Open file
    ld      de,_fcb
    call    _open_file
    cp      $FF
    jp      z,_not_found

    ; Open file on SD card
    ld      a,ESPCMD_OPEN
    call    esp_cmd
    ld      a,$19
    call    esp_send_byte
    ld      hl,_fcb+1
    call    _send_filename
    call    esp_get_byte
    or      a
    jp      m,_err_create
    ld      (_fd),a

    ; Read file on CPM
.1: ld      de,_fcb
    call    _read_seq
    or      a
    jr      nz,.done

    ; Write to file on SD card
    ld      hl,$80
    ld      de,$80
    call    esp_write_bytes
    jr      .1

.done:
    ; Close file
    ld      a,ESPCMD_CLOSE
    call    esp_cmd
    ld      a,(_fd)
    call    esp_send_byte
    call    esp_get_byte

    ; Newline
    ld      a,13
    call    _putchar
    ld      a,10
    call    _putchar
    ret

;-----------------------------------------------------------------------------
; _copy_to_cpm
;-----------------------------------------------------------------------------
_copy_to_cpm:
    ret

;-----------------------------------------------------------------------------
; _puts
;-----------------------------------------------------------------------------
_puts:
    ld      a,(hl)
    or      a
    ret     z
    push    hl
    call    _putchar
    pop     hl
    inc     hl
    jr      _puts

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
; _getdisk
;-----------------------------------------------------------------------------
_getdisk:
    ld      hl,(1)
    ld      de,3*17
    add     hl,de
    jp      (hl)

;-----------------------------------------------------------------------------
; _memset
;-----------------------------------------------------------------------------
_memset:
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
; _putchar
;-----------------------------------------------------------------------------
_putchar:
    ld      e,a
    ld      c,2
    jp      5

;-----------------------------------------------------------------------------
; _open_file
;-----------------------------------------------------------------------------
_open_file:
    ld      c,15
    jp      5

;-----------------------------------------------------------------------------
; _search_first
;-----------------------------------------------------------------------------
_search_first:
    ld      c,17
    jp      5

;-----------------------------------------------------------------------------
; _search_next
;-----------------------------------------------------------------------------
_search_next:
    ld      c,18
    jp      5

;-----------------------------------------------------------------------------
; _read_seq
;-----------------------------------------------------------------------------
_read_seq:
    ld      c,20
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
; Send bytes
; Input:  HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;-----------------------------------------------------------------------------
esp_send_bytes:
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
; Input:  HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
esp_write_bytes:
    ld      a, ESPCMD_WRITE
    call    esp_cmd

    ; Send file descriptor
    ld      a,(_fd)
    call    esp_send_byte

    ; Send write size
    ld      a, e
    call    esp_send_byte
    ld      a, d
    call    esp_send_byte

    ; Send bytes
    call    esp_send_bytes

    ; Get result
    call    esp_get_byte
    jp      m,_err_create

    ; Get number of bytes actual written
    call    esp_get_byte
    ld      e, a
    call    esp_get_byte
    ld      d, a

    ret

;-----------------------------------------------------------------------------
; Send filename in hl
;-----------------------------------------------------------------------------
_send_filename:
    push    hl

    ; Send filename
    ld      c,8
.1: ld      a,(hl)
    cp      a,' '
    jr      z,.ext
    call    esp_send_byte
    inc     hl
    dec     c
    jr      nz,.1

    ; Send extension
.ext:
    ld      a,'.'
    call    esp_send_byte
    pop     hl
    ld      bc,8
    add     hl,bc
    ld      c,3
.2: ld      a,(hl)
    cp      a,' '
    jr      z,.zterm
    call    esp_send_byte
    inc     hl
    dec     c
    jr      nz,.2

    ; Send zero-termination
.zterm:
    xor     a
    call    esp_send_byte
    ret
