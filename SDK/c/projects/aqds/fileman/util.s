    .globl _scr_putchar

    .area   _CODE

;-----------------------------------------------------------------------------
; Print 5-digit number digit in HL
;-----------------------------------------------------------------------------
_print_5digits::
    ld      d,#1
    ld      bc,#-10000
    call    .num1
    jr      _print

;-----------------------------------------------------------------------------
; Print 4-digit number digit in HL
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
    call    _scr_putchar
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
    call    _scr_putchar
    pop     bc
    pop     hl
    ret

;-----------------------------------------------------------------------------
; Print 2-digit number digit in A
;-----------------------------------------------------------------------------
_print_2digits::
    cp      #100
    jr      c,.l0
    sub     a,#100
    jr      _print_2digits
.l0:
    ld      c,#0
.l1:
    inc     c
    sub     a,#10
    jr      nc,.l1
    add     a,#10
    push    af

    ld      a,c
    add     #'0'-1
    call    _scr_putchar
    pop     af
    add     #'0'
    call    _scr_putchar
    ret
