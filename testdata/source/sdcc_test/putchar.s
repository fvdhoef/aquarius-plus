    .area   CODE
_putchar::
    cp      #'\n'
    jr      nz, _no_lf

    ld      a, #'\r'
    call    0x1D94
    ld      a, #'\n'
_no_lf:
    call    0x1D94
    ret
