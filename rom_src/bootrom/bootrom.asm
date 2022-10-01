    include "../aqplus_rom/regs.inc"

    org     $0

    ld      sp, $38A0
loop:
    ld      hl, $3001
    inc     (hl)

    call    delay
    jr      loop

delay:
    ld      hl, 0
    dec     hl

    xor     b
.2: xor     a
.1: dec     a
    jr      nz, .1
    dec     b
    jr      nz, .2

    ret


; message:

;     ld      hl, .str
; .1: ld      a, (hl)
;     or      a
;     jr      z, .done
;     out     (IO_ESPDATA), a
;     inc     hl
;     jr      .1
; .done:

;     jr      message

; .hang:
;     jr      .hang

; .str:
;     .db "Hello world!",13,10,0

    end
