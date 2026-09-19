    include "regs.inc"

	org $38E1

    ; Header and BASIC stub
	defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    "AQPLUS"
	defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp      main

main:
    ld      a,ESPCMD_LOADFPGA
    call    esp_cmd
    ld      hl,.str
    call    esp_send_str
    call    esp_get_byte
.1: jr      .1

.str: defb "aqms.core",0

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
