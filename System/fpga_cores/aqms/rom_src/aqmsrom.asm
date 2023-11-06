IO_VDPDATA  equ 0xBE
IO_VDPCTRL  equ 0xBF
IO_JOY1     equ 0xC0

cursor_x    equ $C000
cursor_y    equ $C001

    org 0

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
_entry:
    jp      _main

;-----------------------------------------------------------------------------
; Main program
;-----------------------------------------------------------------------------
_main:
    di                      ; Disable interrupts
    ld      sp,$E000        ; Set stack pointer

    ; Clear RAM
    ld      hl,$C000
    ld      de,$C001
    ld      bc,$1FFF
    ld      (hl),$00
    ldir

    ; Init system
    call    _init

    call    _clear_screen

    ld      a,2
    ld      (cursor_y),a
    ld      a,10
    ld      (cursor_x),a

    call    _setaddr
    ld      hl,.str
    call    _puts

.1: jr      .1

.str:  defb "Hello world!",10,"Blaat",0

;-----------------------------------------------------------------------------
; Initialization
;-----------------------------------------------------------------------------
_init:
    ; Read VDP control
    in      a,(IO_VDPCTRL)

    ; Load VDP settings
    ld      c,IO_VDPCTRL
    ld      b,.vdp_settings_end - .vdp_settings
    ld      hl,.vdp_settings
    otir

    ; Load palette
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$C0
    out     (IO_VDPCTRL),a
    ld      c,IO_VDPDATA
    ld      b,.palette_end - .palette
    ld      hl,.palette
    otir
    ld      b,.palette_end - .palette
    ld      hl,.palette
    otir

    ; Load patterns
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$40
    out     (IO_VDPCTRL),a

    ld      d,(font_end-font_start)/32
    ld      c,IO_VDPDATA
    ld      hl,font_start
.l: ld      b,32
    otir
    dec     d
    jr      nz,.l

    ret

.vdp_settings:
    defb    $06,$80, $C0,$81, $FF,$82, $FF,$83, $FF,$84, $FF,$85, $FF,$86, $00,$87, $00,$88, $00,$89
.vdp_settings_end:

.palette:
    defb    $00,$03,$0C,$0F,$30,$33,$3C,$3F,$2A,$28,$33,$21,$1F,$1C,$02,$15
.palette_end:

    include "font.asm"

;-----------------------------------------------------------------------------
; Clear screen
;-----------------------------------------------------------------------------
_clear_screen:
    ; Map is located at $3800

    ; vaddr = $3800
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$40|$38
    out     (IO_VDPCTRL),a

    ; write 28x32x2 zeroes
    xor     a
    ld      c,28
.1: ld      b,32
.2: out     (IO_VDPDATA)
    out     (IO_VDPDATA)
    dec     b
    jr      nz,.2
    dec     c
    jr      nz,.1

    ret

;-----------------------------------------------------------------------------
; Set addr
;-----------------------------------------------------------------------------
_setaddr:
    ; cursor_y * 64
    ld      h,0
    ld      a,(cursor_y)
    ld      l,a
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl

    ; += cursor_x * 2
    ld      a,(cursor_x)
    add     a,a
    add     a,l
    out     (IO_VDPCTRL),a

    ld      a,h
    add     a,$40|$38
    out     (IO_VDPCTRL),a
    ret

;-----------------------------------------------------------------------------
; Put char
;-----------------------------------------------------------------------------
_putchar:
    cp      10
    jp      z,_newline
    sub     a,32
    cp      a,95
    jr      c,.1
    xor     a
.1: out     (IO_VDPDATA)
    xor     a
    out     (IO_VDPDATA)

    ; Increment cursor X
    ld      a,(cursor_x)
    inc     a
    ld      (cursor_x),a
    cp      32
    ret     c
    xor     a
    ld      (cursor_x),a

    ; Increment cursor Y
    ld      a,(cursor_y)
    inc     a
    ld      (cursor_y),a
    ; cp      32
    ; ret     c
    ; xor     a
    ; ld      (cursor_y),a

    ret

;-----------------------------------------------------------------------------
; New line
;-----------------------------------------------------------------------------
_newline:
    push    hl
    xor     a
    ld      (cursor_x),a
    ld      a,(cursor_y)
    inc     a
    ld      (cursor_y),a
    call    _setaddr
    pop     hl
    ret

;-----------------------------------------------------------------------------
; Put string
;-----------------------------------------------------------------------------
_puts:
    ld      a,(hl)
    or      a
    ret     z
    call    _putchar
    inc     hl
    jr      _puts
