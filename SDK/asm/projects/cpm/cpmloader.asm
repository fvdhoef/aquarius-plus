    include "regs.inc"

	org $38E1

    ; Header and BASIC stub
	db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db "AQPLUS"
	db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp main

cpm_start:
    incbin "zout/cpm22.cim"
cpm_end:

main:
    ; Map RAM in bank 3
    ld  a, 35
    out (IO_BANK3), a

    ; Clear bank 3
    ld      hl, $C000
    ld      bc, $4000
    ld      a, 0
    call    _memset

    ; Copy CP/M to bank 3
    ld  de, $DC00
    ld  hl, cpm_start
    ld  bc, cpm_end - cpm_start
    ldir

    ; Map RAM in bank 0
    ld  a, 32
    out (IO_BANK0), a

    ; Jump to CP/M BIOS
    jp  $F200

_memset:
    inc     c
    inc     b
    jr      .start
.repeat:
    ld      (hl), a
    inc     hl
.start:
    dec     c
    jr      nz, .repeat
    dec     b
    jr      nz, .repeat
    ret
