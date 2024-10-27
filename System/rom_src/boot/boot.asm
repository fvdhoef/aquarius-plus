;-----------------------------------------------------------------------------
; Aquarius+ boot code
;-----------------------------------------------------------------------------
; By Frank van den Hoef
;-----------------------------------------------------------------------------

PAGE_CART           equ 19
PAGE_CHARROM        equ 21

PAGE_MAINRAM0       equ 56
PAGE_MAINRAM1       equ 57
PAGE_MAINRAM2       equ 58
PAGE_MAINRAM3       equ 59
PAGE_SYSROM0        equ 60
PAGE_SYSROM1        equ 61
PAGE_SYSROM2        equ 62
PAGE_CART_NONSCRAM  equ 63

BOOTSTUB_ADDR       equ $3880

    include "regs.inc"

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
    org     $C000
    jp      _start                ; $C000 - Normal boot - checks for harware cartridge
    jp      _softcart             ; $C003 - Checks for cart image in bank [A]

_softcart:
    ld      (bank3_page),a
    jr      _boot

_start:
    ld      a,PAGE_CART
_boot
    ld      sp,$0
    push    af

    ; Disable video
    xor     a
    out     (IO_VCTRL), a

    ; Load character ROM
    ld      a,PAGE_CHARROM
    out     (IO_BANK2),a
    ld      hl,fn_default_chr
    call    esp_open
    ld      hl,$8000
    ld      de,2048
    call    esp_read_bytes
    call    esp_close

    ; Init palettes
    ld      b,0
    ld      hl,default_palette
    call    .set_palette
    ld      hl,default_palette
    call    .set_palette
    ld      hl,default_palette
    call    .set_palette
    ld      hl,default_palette
    call    .set_palette

    ; Clear video RAM $3000-$37FF
    in      a,(IO_BANK0)
    or      a,BANK_READONLY | BANK_OVERLAY
    out     (IO_BANK0),a
    xor     a
    ld      hl,$3000
    ld      (hl),a
    ld      de,$3001
    ld      bc,$800-1
    ldir

    ; Init video mode
    ld      a, 1
    out     (IO_VCTRL), a

    ; Check for cartridge
    ld      a,PAGE_CART_NONSCRAM
    out     (IO_BANK1),a
    pop     af
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
    jp      nz,no_cart  ; ROM not found
    ex      de,hl       ; Encryption Key Address into HL
    ld      b,$0C
.2: add     a,(hl)
    inc     hl
    add     a,b
    dec     b
    jr      nz,.2
    xor     (hl)
    jp      z,.descramble_done

    ; Descramble ROM with XOR value in A
    ld      b,a
    ld      a,PAGE_CART_NONSCRAM
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

.descramble_done:

    ld      a,(bank3_page)
    or      BANK_READONLY
    ld      (bank3_page),a
    
    ; Load S2 ROM for highest compatibility with cartridge games
    ld      hl,fn_sysrom_s2_bin
    call    load_sysrom
    jp      start_sysrom

.set_palette:
    ld      c,IO_VPALSEL
    ld      d,32
.palloop:
    out     (c),b
    ld      a,(hl)
    out     (IO_VPALDATA),a
    inc     hl
    inc     b
    dec     d
    jr      nz,.palloop
    ret

;-----------------------------------------------------------------------------
; No cartridge
;-----------------------------------------------------------------------------
no_cart:
    ; Try to load system ROM from SD card
    ld      hl,fn_sysrom_sdcard_bin
    call    load_sysrom
    jr      z,.done

    ; SD card load failed, fall back to plusBASIC ROM
    ld      hl,fn_sysrom_pb_bin
    call    load_sysrom
.done:
    jp      start_sysrom


default_palette:
    dw $111,$F11,$1F1,$FF1,$22E,$F1F,$3CC,$FFF
    dw $CCC,$3BB,$C2C,$419,$FF7,$2D4,$B22,$333

fn_default_chr:        defb "esp:default.chr",0
fn_sysrom_s2_bin:      defb "esp:sysrom_s2.bin",0
fn_sysrom_pb_bin:      defb "esp:sysrom.bin",0
fn_sysrom_sdcard_bin:  defb "sysrom.bin",0

cart_crtsig:
    defb "+7$$3,",0
cart_fixup:
    defb 65,81,80,76,85,156,83,176,70,108,73,100,88,168,131,112

bank3_page:
    defb PAGE_CART

;-----------------------------------------------------------------------------
; start_sysrom
;-----------------------------------------------------------------------------
start_sysrom:
    ; Copy stub to bank 0 and jump to it
    ld      de,BOOTSTUB_ADDR
    ld      hl,bootstub
    ld      bc,bootstub_end-bootstub
    ldir

    ; Disable turbo mode
    ld      a,0
    out     (IO_SYSCTRL),a

    ld      a,(bank3_page)
    jp      BOOTSTUB_ADDR

;-----------------------------------------------------------------------------
; load_sysrom
;-----------------------------------------------------------------------------
load_sysrom:
    push    hl

    ; Initialize memory banks
    ld      a,PAGE_SYSROM0
    out     (IO_BANK0),a
    ld      a,PAGE_SYSROM1
    out     (IO_BANK1),a
    ld      a,PAGE_SYSROM2
    out     (IO_BANK2),a

    ; Clear area from $2000-$3800 with $FF
    ld      a,$FF
    ld      hl,$2000
    ld      (hl),a
    ld      de,$2001
    ld      bc,$1800-1
    ldir

    ; Read file
    pop     hl
    call    esp_open
    ret     nz
    ld      hl,$0
    ld      de,$3800
    call    esp_read_bytes
    ld      hl,$4000
    ld      de,$0800
    call    esp_read_bytes
    ld      hl,$4000
    ld      de,$8000
    call    esp_read_bytes
    call    esp_close

    ; Remap banks
    ld      a,PAGE_SYSROM0 | BANK_READONLY | BANK_OVERLAY
    out     (IO_BANK0),a
    ld      a,PAGE_MAINRAM1
    out     (IO_BANK1),a
    ld      a,PAGE_MAINRAM2
    out     (IO_BANK2),a

    xor     a
    ret

;-----------------------------------------------------------------------------
; bootstub - used to switch bank 3 and jump to sysrom entry
;-----------------------------------------------------------------------------
bootstub:
    phase   BOOTSTUB_ADDR
    out     (IO_BANK3),a
    jp      0
    dephase
bootstub_end:

;-----------------------------------------------------------------------------
; esp functions
;-----------------------------------------------------------------------------
    include "esp.asm"

    end
