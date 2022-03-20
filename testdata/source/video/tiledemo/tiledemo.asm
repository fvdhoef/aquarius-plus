
    include "entry.asm"
    include "../../../../rom_src/aqplus_rom/regs.inc"
    include "file.asm"

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
scroll_x:   dw 0
ball_x:     dw 0
ball_y:     db 0
ball_dx:    dw 1
ball_dy:    db 2

;-----------------------------------------------------------------------------
; Setup sprites
;-----------------------------------------------------------------------------
setup_sprites:
    ld      a, 0
    out     (IO_VSPRSEL), a
    ld      a, (128+227) & 0xFF
    out     (IO_VSPRIDX), a
    ld      a, $80 | ((128+227) >> 8)
    out     (IO_VSPRATTR), a

    ld      a, 1
    out     (IO_VSPRSEL), a
    ld      a, (128+228) & 0xFF
    out     (IO_VSPRIDX), a
    ld      a, $80 | ((128+228) >> 8)
    out     (IO_VSPRATTR), a

    ld      a, 2
    out     (IO_VSPRSEL), a
    ld      a, (128+243) & 0xFF
    out     (IO_VSPRIDX), a
    ld      a, $80 | ((128+243) >> 8)
    out     (IO_VSPRATTR), a

    ld      a, 3
    out     (IO_VSPRSEL), a
    ld      a, (128+244) & 0xFF
    out     (IO_VSPRIDX), a
    ld      a, $80 | ((128+244) >> 8)
    out     (IO_VSPRATTR), a

    ret

;-----------------------------------------------------------------------------
; update_sprites
;-----------------------------------------------------------------------------
update_sprites:
    ld      a, 0
    out     (IO_VSPRSEL), a
    ld      a, (ball_x)
    out     (IO_VSPRX_L), a
    ld      a, (ball_x+1)
    out     (IO_VSPRX_H), a
    ld      a, (ball_y)
    out     (IO_VSPRY), a

    ld      a, 1
    out     (IO_VSPRSEL), a
    ld      a,8
    out     (IO_VSPRX_L), a

    ld      a, (ball_x)
    add     a, 8
    out     (IO_VSPRX_L), a
    ld      a, (ball_x+1)
    adc     a, 8
    out     (IO_VSPRX_H), a
    ld      a, (ball_y)
    out     (IO_VSPRY), a

    ld      a, 2
    out     (IO_VSPRSEL), a
    ld      a, (ball_x)
    out     (IO_VSPRX_L), a
    ld      a, (ball_x+1)
    out     (IO_VSPRX_H), a
    ld      a, (ball_y)
    add     a, 8
    out     (IO_VSPRY), a

    ld      a, 3
    out     (IO_VSPRSEL), a
    ld      a, (ball_x)
    add     a, 8
    out     (IO_VSPRX_L), a
    ld      a, (ball_x+1)
    adc     a, 8
    out     (IO_VSPRX_H), a
    ld      a, (ball_y)
    add     a, 8
    out     (IO_VSPRY), a

    ret

;-----------------------------------------------------------------------------
; update_ball
;-----------------------------------------------------------------------------
update_ball:
    ; ball_x += ball_dx
    ld      hl, (ball_dx)
    ex      de, hl
    ld      hl, (ball_x)
    add     hl, de
    ld      (ball_x), hl

    ; if (ball_x >= 320-16) ball_dx = -ball_dx
    ld      bc, 320-16
    or      a
    sbc     hl, bc
    jr      c, .hdone
    ld      hl, 0
    sbc     hl, de
    ld      (ball_dx), hl
.hdone:

    ; ball_y += ball_dy
    ld      a, (ball_dy)
    ld      b, a
    ld      a, (ball_y)
    add     a, b

    ; if (ball_y >= 200-16) ball_dy = -ball_dy
    ld      (ball_y), a
    cp      a, 200-16
    jr      c, .vdone
    ld      a, b
    neg
    ld      (ball_dy), a
.vdone:

    ret

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

    ; Setup sprites
    call    setup_sprites
    call    update_sprites

    ; Set video mode
    ld      a, VCTRL_MODE_TILE | VCTRL_SPR_EN
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

    ; Move ball
    call    update_ball
    call    update_sprites

    jp      .game_loop

    pop     hl
    ret

filename: db "tiledata.bin",0

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

