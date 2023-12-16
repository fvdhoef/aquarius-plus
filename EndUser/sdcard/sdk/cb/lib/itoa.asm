;-----------------------------------------------------------------------------
; itoa(char *str, int val) - outputs 5 characters, no zero termination!
;-----------------------------------------------------------------------------
_itoa:
    push    ix
    ld      ix,0
    add     ix,sp

    ld      l,(ix+6)
    ld      h,(ix+7)
    call    .out_digits

    ld      sp,ix
    pop     ix
    ret

.putchar:
    push    hl
    ld      l,(ix+4)
    ld      h,(ix+5)
    ld      (hl),a
    inc     hl
    ld      (ix+4),l
    ld      (ix+5),h
    pop     hl
    ret

.out_digits:
    ld      d,1
    ld      bc,-10000
    call    .num1
    ld      bc,-1000
    call    .num1
    ld      bc,-100
    call    .num1
    ld      c,low(-10)
    call    .num1

    ld      d,0
    ld      c,low(-1)
.num1:
    ld      a,low(-1)
.num2:
    inc     a
    add     hl,bc
    jr      c,.num2
    sbc     hl,bc
    or      a
    jr      z,.zero

    ld      d,0
.normal:
    add     a,'0'
    ld      c,a
    call    .putchar
    ret

.zero:
    bit     0,d
    jr      z,.normal
    ld      a,' '
    ld      c,a
    call    .putchar
    ret
