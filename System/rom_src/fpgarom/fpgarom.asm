;-----------------------------------------------------------------------------
; Aquarius+ FPGA ROM
;-----------------------------------------------------------------------------
; By Frank van den Hoef
;-----------------------------------------------------------------------------

    include "regs.inc"

    org     0
    ld      sp,$38A0

    ; Enable turbo mode
    ld      a,4
    out     (IO_SYSCTRL),a

    ; Initialize bank 3 to page 36
    ld      a,36
    out     (IO_BANK3),a

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
