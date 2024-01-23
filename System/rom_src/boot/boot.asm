;-----------------------------------------------------------------------------
; Aquarius+ boot code
;-----------------------------------------------------------------------------
; By Frank van den Hoef
;-----------------------------------------------------------------------------

    include "regs.inc"

    org     $C000
    ld      sp,$0

    ; Load character ROM
    ld      a,21
    out     (IO_BANK2),a
    ld      hl,.fn_default_chr
    call    esp_open
    ld      hl,$8000
    ld      de,2048
    call    esp_read_bytes
    call    esp_close
    
    ; Init palette 0
    ld      hl,.default_palette
    ld      c,IO_VPALSEL
    ld      b,0
    ld      d,32
.palloop:
    out     (c),b
    ld      a,(hl)
    out     (IO_VPALDATA),a
    inc     hl
    inc     b
    dec     d
    jr      nz,.palloop

    ; Init video mode
    ld      a, 1
    out     (IO_VCTRL), a

    ; Initialize memory banks
    ld      a,32
    out     (IO_BANK0),a
    ld      a,33
    out     (IO_BANK1),a
    ld      a,34
    out     (IO_BANK2),a

    ; Load system ROM
    ld      hl,.fn_sysrom_s2_bin
    call    esp_open
    ld      hl,$0
    ld      de,$C000
    call    esp_read_bytes
    call    esp_close

    ; Disable turbo mode
    ld      a,0
    out     (IO_SYSCTRL),a

    ; Remap bank 0 and jump to it
    ld      a,32 | BANK_READONLY | BANK_OVERLAY
    out     (IO_BANK0),a
    jp      0

.default_palette:
    dw $111, $F11, $1F1, $FF1, $22E, $F1F, $3CC, $FFF
    dw $CCC, $3BB, $C2C, $419, $FF7, $2D4, $B22, $333

.fn_default_chr:
    db "esp:default.chr",0
.fn_sysrom_s2_bin:
    db "esp:sysrom_s2.bin",0

; boot_stub:
;     phase $3880
;     ld      a,32
;     out     (IO_BANK0),a

    include "esp.asm"
    end
