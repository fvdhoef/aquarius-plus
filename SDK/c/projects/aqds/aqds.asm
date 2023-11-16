;-----------------------------------------------------------------------------
; aqds.asm
;-----------------------------------------------------------------------------
    include "regs.inc"

;-----------------------------------------------------------------------------
; Header and BASIC stub
;-----------------------------------------------------------------------------
    org     $38E1
    defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    "AQPLUS"
    defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp      main

;-----------------------------------------------------------------------------
; main
;-----------------------------------------------------------------------------
main:
    ; Disable interrupts
    di

    ; Enable turbo mode
    ld      a,$4
    out     (IO_SYSCTRL),a

    jp      _go_fileman


;-----------------------------------------------------------------------------
; _go_fileman
;-----------------------------------------------------------------------------
_go_fileman:
    ; Relocate binary
    ld      a,60
    out     (IO_BANK3),a
    ld      hl,_fileman_start
    ld      de,$C100
    ld      bc,_fileman_end - _fileman_start
    ldir

    ; Start stub
    ld      a,63
    out     (IO_BANK3),a
    ld      hl,_jmp_app_stub_start
    ld      de,$C000
    ld      bc,_jmp_app_stub_end - _jmp_app_stub_start
    ldir
    jp      _jmp_app

;-----------------------------------------------------------------------------
; _go_editor
;-----------------------------------------------------------------------------
_go_editor:
    ; Relocate binary
    ld      a,60
    out     (IO_BANK3),a
    ld      hl,_editor_start
    ld      de,$C100
    ld      bc,_editor_end - _editor_start
    ldir

    ; Set filename
    ld      hl,.filename
    ld      de,$C080
.1: ld      a,(hl)
    ld      (de),a
    inc     hl
    inc     de
    or      a
    jr      nz,.1

    ; Start stub
    ld      a,63
    out     (IO_BANK3),a
    ld      hl,_jmp_app_stub_start
    ld      de,$C000
    ld      bc,_jmp_app_stub_end - _jmp_app_stub_start
    ldir
    jp      _jmp_app

.filename:  defb "Blaat2.txt",0



_jmp_app_stub_start:
    phase   $C000

_jmp_app:
    ld      a,60 | $40      ; Enable overlay
    out     (IO_BANK0),a
    ld      a,61
    out     (IO_BANK1),a
    ld      a,62
    out     (IO_BANK2),a
    ld      sp,$3000
    jp      $100

    dephase
_jmp_app_stub_end:

_editor_start:
    incbin  "editor/build/editor.bin"
_editor_end:

_fileman_start:
    incbin  "fileman/build/fileman.bin"
_fileman_end:
