;-----------------------------------------------------------------------------
; memcpy(char *dst, char *src, int len)
;-----------------------------------------------------------------------------
_memcpy:
    push    ix
    ld      ix,0
    add     ix,sp

    ; Get stack parameters
    ld      e,(ix+4)
    ld      d,(ix+5)
    ld      l,(ix+6)
    ld      h,(ix+7)
    ld      c,(ix+8)
    ld      b,(ix+9)

    ; Check if BC is zero
    ld      a,b
    or      c
    jr      z,.1
    ldir
.1:
    ld      sp,ix
    pop     ix
    ret
