
    include "entry.asm"
    include "../../../../rom_src/aqplus_rom/regs.inc"
    include "file.asm"

;-----------------------------------------------------------------------------
; main
;-----------------------------------------------------------------------------
main:
    push    hl
    call    esp_close_all

    ; Map video RAM to $C000
    ld      a, 20
    out     (IO_BANK3), a

    ; Load in tile data
    ld      hl, filename
    call    esp_open
    ld      hl, $C000
    ld      de, $4000
    call    esp_read_bytes
    call    esp_close_all

    ; Set palette
    ld      hl, $F000
    ld      c, IO_VPALSEL
    ld      b, 0
    ld      d, 32
.palloop:
    out     (c), b
    ld      a, (hl)
    out     (IO_VPALDATA), a
    inc     hl
    inc     b
    dec     d
    jr      nz, .palloop

    ; Setup sprite
    ; ld      b, 0

    ; ld      c, IO_VSPRX_L
    ; out     (c), a
    ; ld      c, IO_VSPRX_H
    ; out     (c), a
    ; ld      c, IO_VSPRY
    ; out     (c), a
    ; ld      c, IO_VSPRIDX
    ; out     (c), a
    ; ld      c, IO_VSPRATTR
    ; out     (c), a


    ; Set video mode
    ld      a, 2
    out     (IO_VCTRL), a

.game_loop:
    call    wait_vsync

    ; Scroll horizontally
    ld      hl, scroll_x
    inc     (hl)
    jr      nz, .scroll_done
    ld      hl, scroll_x+1
    inc     (hl)
.scroll_done:

    ; Set scroll register
    ld      a, (scroll_x)
    out     (IO_VSCRX_L), a
    ld      a, (scroll_x+1)
    out     (IO_VSCRX_H), a

    jp      .game_loop

    pop     hl
    ret

filename: db "tiledata.bin",0
scroll_x: dw 0

;-----------------------------------------------------------------------------
; wait_vsync
;-----------------------------------------------------------------------------
wait_vsync:
.wait_high:
    in      a, (IO_VSYNC)
    and     1
    jr      z, .wait_high

.wait_low:
    in      a, (IO_VSYNC)
    and     1
    jr      nz, .wait_low
    ret

