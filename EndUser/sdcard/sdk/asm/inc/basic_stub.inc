; This file contains the header needed for a valid executable to be run from BASIC
; It contains a small BASIC program which jumps to the entry point of the program

    org $38E1

    ; Header and BASIC stub
    defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    "AQPLUS"
    defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00

    push    hl      ; The HL register is used by BASIC and needs to be preserved
    call    main
    pop     hl
    ret
