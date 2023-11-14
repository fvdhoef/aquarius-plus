    .globl __putchar

    .area   _CODE

;-----------------------------------------------------------------------------
; _print_5digits
;-----------------------------------------------------------------------------
_print_5digits::
    ld      d,#1
    ld      bc,#-10000
    call    .num1
    jr      _print

;-----------------------------------------------------------------------------
; _print_4digits
;-----------------------------------------------------------------------------
_print_4digits::
    ld      d,#1
_print:
    ld      bc,#-1000
    call    .num1
    ld      bc,#-100
    call    .num1
    ld      c,#-10
    call    .num1

    ld      d,#0
    ld      c,#-1
.num1:
    ld      a,#-1
.num2:
    inc     a
    add     hl,bc
    jr      c,.num2
    sbc     hl,bc
    or      a
    jr      z,.zero

    ld      d,#0
.normal:
    add     #'0'
    ld      c,a

    push    hl
    push    bc
    call    __putchar
    pop     bc
    pop     hl
    ret

.zero:
    bit     0,d
    jr      z,.normal
    ld      a,#' '
    ld      c,a

    push    hl
    push    bc
    call    __putchar
    pop     bc
    pop     hl
    ret
