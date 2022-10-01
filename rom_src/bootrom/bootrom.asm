    include "../aqplus_rom/regs.inc"

    org     $0

    ld      hl, $3001
loop:
    inc     (hl)
    jr      loop

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
