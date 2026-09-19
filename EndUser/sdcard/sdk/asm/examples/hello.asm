    include "../inc/basic_stub.inc"

; Entry points in BASIC ROM
STROUT      equ $0E9D   ; Print null-terminated string in HL

; Our main function
main:
    ld      hl,.str
    call    STROUT
    ret

.str:   defb "Hello world",13,10,0
