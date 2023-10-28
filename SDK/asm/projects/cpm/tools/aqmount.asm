;-----------------------------------------------------------------------------
; aqmount.asm
;-----------------------------------------------------------------------------
    include "regs.inc"

    org     $100
    jp      _entry

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
_disk:      defb 0
_path:      defs 128

_stat:
    defw 0,0
_stat_attr:
    defb 0
_stat_size:
    defd 0

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
_entry:
    ld      hl,$81
    call    _skip_spaces
    ld      a,(hl)
    or      a
    jp      z,.done

    ; Get disk number
    call    _to_upper
    cp      'A'
    jp      c,_invalid_argument
    cp      'D'+1
    jp      nc,_invalid_argument
    sub     a,'A'
    ld      (_disk),a

    ; Get path
    inc     hl
    ld      a,(hl)
    or      a
    jr      z,.unmount
    cp      ' '
    jr      nz,_invalid_argument
    call    _skip_spaces

    ld      de,_path
    cp      '/'
    jr      z,.append_path

    ; Get current directory
    ld      a,ESPCMD_GETCWD
    call    esp_cmd
    call    esp_get_byte
.1: call    esp_get_byte
    or      a
    jr      z,.2
    ld      (de),a
    inc     de
    jr      .1
.2:

    ; Append argument to path
.append_path:
    ld      a,(hl)
    ld      (de),a
    inc     hl
    inc     de
    or      a
    jr      nz,.append_path

    ; Check if path is valid
    ld      a,ESPCMD_STAT
    call    esp_cmd
    ld      hl,_path
    call    esp_send_str
    call    esp_get_byte
    or      a
    jr      nz,_invalid_argument
    ld      hl,_stat
    ld      de,9
    call    esp_get_bytes

    ; Check if not directory
    ld      a,(_stat_attr)
    and     1
    jr      nz,_invalid_argument

    ; Check file size
    ld      a,(_stat_size+0)
    cp      $00
    jp      nz,_invalid_image
    ld      a,(_stat_size+1)
    cp      $E9
    jp      nz,_invalid_image
    ld      a,(_stat_size+2)
    cp      $03
    jp      nz,_invalid_image
    ld      a,(_stat_size+3)
    cp      $00
    jp      nz,_invalid_image

    ; Set disk
.set_disk
    ld      hl,_path
    ld      a,(_disk)
    ld      c,a
    call    _setdisk

.done:
    call    _print_disks
    ret

.unmount:
    xor     a
    ld      (_path),a
    jr      .set_disk

;-----------------------------------------------------------------------------
; _invalid_argument
;-----------------------------------------------------------------------------
_invalid_argument:
    ld      hl,.str
    call    _puts
    ret

.str:   defb "Error: Invalid argument.",13,10,"Syntax: AQMOUNT <DISK> <Image>",13,10,"Example: AQMOUNT C /MYDISK.DSK",13,10,0

;-----------------------------------------------------------------------------
; _invalid_image
;-----------------------------------------------------------------------------
_invalid_image:
    ld      hl,.str
    call    _puts
    ret

.str:   defb "Error: Invalid disk image.",13,10,"Disk image should be 256256 byte in size.",13,10,0

;-----------------------------------------------------------------------------
; _to_upper
;-----------------------------------------------------------------------------
_to_upper:
    cp      a,'a'
    ret     nc
    cp      a,'z'+1
    ret     c
    sub     a,'A'-'a'
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
; _print_disks
;-----------------------------------------------------------------------------
_print_disks:
    ld      a,0
    ld      (_disk),a
.1: call    _print_disk
    ld      a,(_disk)
    inc     a
    cp      4
    ret     nc
    ld      (_disk),a
    jr      .1

;-----------------------------------------------------------------------------
; _print_disk
;-----------------------------------------------------------------------------
_print_disk:
    ld      a,(_disk)
    add     a,'A'
    call    _putchar
    ld      a,':'
    call    _putchar

    ld      a,(_disk)
    ld      c,a
    call    _getdisk
    call    _puts

    ld      a,13
    call    _putchar
    ld      a,10
    call    _putchar

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
; _putchar
;-----------------------------------------------------------------------------
_putchar:
    ld      e,a
    ld      c,2
    jp      5

;-----------------------------------------------------------------------------
; _getdisk
;-----------------------------------------------------------------------------
_getdisk:
    ld      hl,(1)
    ld      de,3*17
    add     hl,de
    jp      (hl)

;-----------------------------------------------------------------------------
; _setdisk
;-----------------------------------------------------------------------------
_setdisk:
    ld      ix,(1)
    ld      de,3*16
    add     ix,de
    jp      (ix)

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
