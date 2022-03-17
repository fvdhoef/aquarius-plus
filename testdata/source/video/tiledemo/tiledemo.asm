
    include "entry.asm"
    include "../../../../rom_src/aqplus_rom/regs.inc"
    include "file.asm"

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
    ld      b, 0
    ld      c, IO_VPALTILE
    ld      d, 32
.palloop:
    ld      a, (hl)
    out     (c), a
    inc     hl
    inc     b
    dec     d
    jr      nz, .palloop

    ; Set video mode
    ld      a, 2
    out     (IO_VCTRL), a

loop:
    jp      loop

    pop     hl
    ret

filename: db "tiledata.bin",0
