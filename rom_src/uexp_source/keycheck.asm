;======================================================================
;  KEY_CHECK: See if a key has been pressed and return ASCII value
;======================================================================
; differences from Stock Aquarius KEYCHK:-
; - does not expand control codes to BASIC keywords
; - does not use IX, HL', DE',BC'
; - CTRL-O = '~'
;
; note: DEBOUNCE is still a constant. If there is a large delay between
;       keyscans you can reduce debounce time with the following code:-
;
;       LD   A,(SCANCNT)
;       CP   6                 ; starting key-up debounce?
;       JR   NZ,.check_key
;       LD   A,DEBOUNCE-x      ; x = number of scans to go
;       LD   (SCANCNT),A       ; adjust debounce count
;  .check_key:
;       call Key_Check         ; Get ASCII of last key pressed
;
;  in:
; out: A = ASCII code of key pressed (0 = no key)
;
DEBOUNCE = 70   ; number of key-up scans before believing key is up

Key_Check:
        push    hl
        push    bc
        ld      bc,$00ff        ; Scan all columns at once
        in      a,(c)           ; Read the results
        cpl                     ; invert - (a key down now gives 1)
        and     $3f             ; check all rows
        ld      hl,LASTKEY      ; HL = &LASTKEY (scan code of last key pressed)
        jr      z,.nokeys
        ld      b,$7f           ; 01111111 - scanning column 8
        in      a,(c)
        cpl                     ; invert bits
        and     $0f             ; check lower 4 bits
        jr      nz,.keydown     ; if any keys in column 8 pressed then do KEYDOWN
.scncols:
        ld      b,$bf           ; 10111111 - start with column 7
.keycolumn:
        in      a,(c)
        cpl                     ; invert bits
        and     $3f             ; is any key down?
        jr      nz,.keydown     ; yes,
        rrc     b               ; no, try next column
        jr      c,.keycolumn    ; until all columns scanned

; key up debouncer
.nokeys:                        ; no keys are down.
        inc     hl              ; HL = &SCANCNT, counts how many times the same
        ld      a,DEBOUNCE      ;                code has been scanned in a row.
        cp      (hl)            ; compare scan count to debounce value
        jr      c,.nokey        ; if scanned more than DEBOUNCE times then done
        jr      z,.keyup        ; if scanned DEBOUNCE times then do KEY UP
        inc     (hl)            ; else increment SCANCNT
        jr      .nokey

; HL = &LASTKEY
; B  = bit pattern of column being scanned.
; A  = row bits
; KROWCNT converts the BIT number of the row and column into
; actual numbers. So if bit 7 was set, a would hold 7.
; the column is multiplied by 6 so it can be added to the row number
; to give a unique scan code for each key.
; There are 8 columns of 6 keys giving a total of 48 keys.
;
.keydown:
       ld      c,0             ; C = column count
.krowcnt:
       inc     c               ; column count + 1
       rra
       jr      nc,.krowcnt     ; Count how many rotations to get the bit into Carry.
       ld      a,c             ; A = number of bit which was set
.kcolcnt:
       rr      b
       jr      nc,.krowcol     ; jump when 0 bit gets to CARRY
       add     a,6             ; add 6 for each rotate, to give the column number.
       jr      .kcolcnt
; A = (column*6)+row
.krowcol:
       cp      (hl)            ; is scancode same as last time?
       ld      (hl),a          ; (LASTKEY) = scancode
       inc     hl              ; HL = &SCANCOUNT
       jr      nz,.newkey      ; no,
       ld      a,4             ; yes, has it been down for 4 scans? (debounce)
       cp      (hl)
       jr      c,.scan6        ; if more than 4 counts then we are already handling it
       jr      z,.kdecode      ; if key has been down for exactly 4 scans then decode it
       inc     (hl)            ; otherwise increment SCANCOUNT
       jr      .nokey          ; exit with no key
.scan6:
       ld      (hl),6          ; SCANCOUNT = 6
       jr      .nokey          ; exit with no key

; The same key has now been down for 4 scans.
; so it's time to find out what it is.
;  in: HL = &SCANCOUNT
.kdecode:
       inc     (hl)           ; increment the scan count
       ld      bc,$7fff       ; read column 8 ($7f = 01111111)
       in      a,(c)
       ld      hl,CTLTBL-1    ; point to start of CTRL key lookup table
       bit     5,a            ; CTRL key down?
       jr      z,.klookup     ; yes,
       ld      hl,SHFTBL-1    ; point to start of SHIFT key lookup table
       bit     4,a            ; SHIFT key down?
       jr      z,.klookup     ; yes,
       ld      hl,KEYTBL-1    ; else point to start of normal key lookup table.
.klookup:
       ld      b,0
       ld      a,(LASTKEY)    ; get scancode
       ld      c,a
       add     hl,bc          ; offset into table
       ld      a,(hl)         ; A = ASCII key
       or      a
       jr      .exit          ; return nz with ASCII key in A
.keyup:
       inc     (hl)           ; increment SCANCNT
       dec     hl             ; HL = &LASTKEY
.newkey:
       ld      (hl),0         ; set SCANCNT/LASTKEY to 0
.nokey:
       xor     a              ; return z, A = no key
.exit:
       pop     bc
       pop     hl
       ret

;-------------------------------------------------------
;                     KEY TABLES
;-------------------------------------------------------
; note: minimum offset is 1 not 0.

;Vanilla key table - no shift or control keys pressed:
KEYTBL:
    db    '='
    db    $08    ; backspace
    db    ':'
    db    CR
    db    ";.-/0pl,9okmnj8i7uhb6ygvcf5t"
    db    "4rdx3esz a2w1q"

; SHIFT key table
SHFTBL:
    db    "+\*",CR
    db    "@>_^?PL<)OKMNJ(I'UHB&YGVCF%T$"
    db    "RDX#ESZ A"
    db    $22    ; '"'
    db    "W!Q"

; CTL key table
;      code  key#  symbol   ctrl-name      operation
;       ---  ----  ------   ---------   ----------------
CTLTBL:
    db  $82  ; 1     =
    db  $1c  ; 2     <-        BS       Backspace
    db  $c1  ; 3     :
    db  $0d  ; 4     CR        CR       Carriage Return
    db  $94  ; 5     ;
    db  $c4  ; 6     .
    db  $81  ; 7     -
    db  $1e  ; 8     /       CTRL-/
    db  $30  ; 9     0         0        '0' (zero)
    db  $10  ;10     p       CTRL-P
    db  $ca  ;11     l
    db  $c3  ;12     ,
    db  $92  ;13     9
    db  '~'  ;14     o       CTRL-O
    db  $9d  ;15     k
    db  $0d  ;16     m       CTRL-M
    db  $c8  ;17     n
    db  $9c  ;18     j
    db  $8d  ;19     8
    db  $09  ;20     i       CTRL-I     TAB
    db  $8c  ;21     7
    db  $15  ;22     u       CTRL-U     abandon line
    db  $08  ;23     h       CTRL-H
    db  $c9  ;24     b
    db  $90  ;25     6
    db  $19  ;26     y       CTRL-Y
    db  $07  ;27     g       CTRL-G     ring the bell
    db  $c7  ;28     v
    db  $03  ;29     c       CTRL-C     break
    db  $83  ;30     f
    db  $88  ;31     5
    db  $84  ;32     t
    db  $a5  ;33     4
    db  $12  ;34     r       CTRL-R
    db  $86  ;35     d
    db  $18  ;36     x       CTRL-X     undo line
    db  $8a  ;37     3
    db  $85  ;38     e
    db  $13  ;39     s       CTRL-S     pause (wait for key press)
    db  $9a  ;40     z
    db  $c6  ;41    " "
    db  $9b  ;42     a
    db  $97  ;43     2
    db  $8e  ;44     w
    db  $89  ;45     1
    db  $11  ;46     q       CTRL-Q
KEYTBL_END