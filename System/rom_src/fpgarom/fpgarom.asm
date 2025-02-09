;-----------------------------------------------------------------------------
; Aquarius+ FPGA ROM
;-----------------------------------------------------------------------------
; By Frank van den Hoef
;-----------------------------------------------------------------------------

PAGE_BOOTBIN equ 51

    include "regs.inc"

    org     0

    ; Initialize bank 3 and initialize stack
    ld      a,PAGE_BOOTBIN
    out     (IO_BANK3),a
    ld      sp,$0

    ; Enable turbo mode
    ld      a,6
    out     (IO_SYSCTRL),a

    ; Initialize ESP
    ld      a,ESPCMD_RESET
    call    esp_cmd

    ; Open file
    ld      hl,.filename
    call    esp_open

    ; Read max $3000 bytes to $C000
    ld      hl,$C000
    ld      de,$3000
    call    esp_read_bytes

    ; Close file
    call    esp_close

    ; Jump to entry point
    jp      $C000

.err:
    jp      .err

.filename:
    db "esp:boot.bin",0

    include "esp.asm"
    end
