    .area   CODE
_getchar::
    push    ix
    call    0x1E7E
    pop     ix
    ld      d, #0
    ld      e, a
    ret
