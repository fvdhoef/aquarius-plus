    .area   _CODE
_putchar::
    push    ix

    ; 16-bit argument passed in hl
    ld      a, l

    ; Convert LF into CRLF
    cp      #'\n'
    jr      nz, _no_lf
    ld      a, #'\r'
    call    0x1D94
    ld      a, #'\n'
_no_lf:

    ; Print character
    call    0x1D94

    ; Return 0
    ld      de, #0

    pop     ix
    ret
