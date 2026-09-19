;-----------------------------------------------------------------------------
; aqdir.asm
;-----------------------------------------------------------------------------
    include "regs.inc"

    org     $100

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
_entry:
    ; Get argument, skip spaces
    ld      hl,$81
    call    _skip_spaces

    ; Argument given?
    ld      a,(hl)
    or      a
    jr      z,.show_path

    ; Change directory
    ld      a,ESPCMD_CHDIR
    call    esp_cmd
    call    esp_send_str
    call    esp_get_byte

    ; Show new path
.show_path:
    ld      a,ESPCMD_GETCWD
    call    esp_cmd
    call    esp_get_byte
.3: call    esp_get_byte
    or      a
    jr      z,.done
    call    _putchar
    jr      .3

.done:
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
