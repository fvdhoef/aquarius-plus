;-----------------------------------------------------------------------------
; Aquarius+ boot code
;-----------------------------------------------------------------------------
; By Frank van den Hoef
;-----------------------------------------------------------------------------

BOOTSTUB_ADDR   equ $3880

    include "regs.inc"

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
    org     $C000
    ld      sp,$0

    ; Load character ROM
    ld      a,21
    out     (IO_BANK2),a
    ld      hl,fn_default_chr
    call    esp_open
    ld      hl,$8000
    ld      de,2048
    call    esp_read_bytes
    call    esp_close

    ; Init palette 0
    ld      hl,default_palette
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

    ; Check for cartridge
    ld      a,35            ; page 35 holds descrambled ROM
    out     (IO_BANK1),a
    ld      a,19            ; page 19 is cartridge ROM
    out     (IO_BANK2),a

    ld      de,$A010+1
    ld      hl,cart_crtsig-1
.1: dec     de
    dec     de
    inc     hl
    ld      a,(de)
    rrca
    rrca
    add     a,e
    cp      (hl)
    jr      z,.1
    ld      a,(hl)
    or      a
    jr      nz,no_cart  ; ROM not found, start Basic
    ex      de,hl       ; Encryption Key Address into HL
    ld      b,$0C
.2: add     a,(hl)
    inc     hl
    add     a,b
    dec     b
    jr      nz,.2
    xor     (hl)
    jp      z,descramble_done

    ; Descramble ROM with XOR value in A
    ld      b,a
    ld      a,35
    ld      (bank3_page),a

    ld      hl,$8000
    ld      de,$4000
.3: ld      a,b
    xor     (hl)
    ld      (de),a
    inc     hl
    inc     de
    ld      a,d
    cp      $80
    jr      nz,.3

    ; Fixup cartridge header
    ld      de,$6000
    ld      hl,cart_fixup
    ld      bc,16
    ldir

descramble_done:
no_cart:

    ; Initialize memory banks
    ld      a,32
    out     (IO_BANK0),a
    ld      a,33
    out     (IO_BANK1),a
    ld      a,34
    out     (IO_BANK2),a

    ; Load system ROM
    ld      hl,fn_sysrom_s2_bin
    call    esp_open
    ld      hl,$0
    ld      de,$C000
    call    esp_read_bytes
    call    esp_close

    ; Disable turbo mode
    ld      a,0
    out     (IO_SYSCTRL),a

    ; Remap bank 0
    ld      a,32 | BANK_READONLY | BANK_OVERLAY
    out     (IO_BANK0),a

    ; Copy stub to bank 0 and jump to it
    ld      de,BOOTSTUB_ADDR
    ld      hl,bootstub
    ld      bc,bootstub_end-bootstub
    ldir

    ld      a,(bank3_page)
    jp      BOOTSTUB_ADDR

default_palette:
    dw $111,$F11,$1F1,$FF1,$22E,$F1F,$3CC,$FFF
    dw $CCC,$3BB,$C2C,$419,$FF7,$2D4,$B22,$333

fn_default_chr:    defb "esp:default.chr",0
fn_sysrom_s2_bin:  defb "esp:sysrom_s2.bin",0

bootstub:
    phase   BOOTSTUB_ADDR
    out     (IO_BANK3),a
    jp      0
    dephase
bootstub_end:

cart_crtsig:
    defb "+7$$3,",0
cart_fixup:
    defb 65,81,80,76,85,156,83,176,70,108,73,100,88,168,131,112
bank3_page:
    defb 19

    include "esp.asm"
    end
