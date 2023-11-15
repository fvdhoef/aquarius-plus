    include "regs.inc"

	org $38E1

    ; Header and BASIC stub
	defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    "AQPLUS"
	defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp      main

_editor_start:
    incbin  "editor/build/editor.bin"
_editor_end:

main:
    ; Disable interrupts
    di

    ; Enable turbo mode
    ld      a,$4
    out     (IO_SYSCTRL),a

    ; Relocate editor
    ld      a,60
    out     (IO_BANK3),a
    ld      hl,_editor_start
    ld      de,$C100
    ld      bc,_editor_end - _editor_start
    ldir

    ; Start stub
    ld      a,63
    out     (IO_BANK3),a
    ld      hl,_stub_start
    ld      de,$C000
    ld      bc,_stub_end - _stub_start
    ldir
    jp      $C000

.1: jr      .1

_stub_start:
    phase   $C000

    ld      a,60 | $40      ; Enable overlay
    out     (IO_BANK0),a
    ld      a,61
    out     (IO_BANK1),a
    ld      a,62
    out     (IO_BANK2),a
    ld      sp,$3000

    jp      $100

    dephase
_stub_end:
