;-------------------------------------------------
;          Lowercase->Uppercase
;-------------------------------------------------
; in-out; A = char
;
UpperCase:
       CP  'a'     ; >='a'?
       RET  C
       CP   'z'+1  ; <='z'?
       RET  NC
       SUB  $20    ; a-z -> A-Z
       RET

;--------------------------------------------------
;                 String Copy
;--------------------------------------------------
;
;  in: HL-> string (null-terminated)
;      DE-> dest
;
; out: HL-> past end of string
;      DE-> at end of dest
;
; NOTE: does NOT null-terminate the destination string!
;       Do it with LD (DE),A after calling strcpy.
;
_strcpy_loop:
       LD    (DE),A
       INC   DE
strcpy:
       LD    A,(HL)
       INC   HL
       OR    A
       JR    NZ,_strcpy_loop
       RET

;------------------------------------------------
;               String Length
;------------------------------------------------
;
;  in: HL-> string (null-terminated)
;
; out: A = number of characters in string
;
strlen:
       PUSH  DE
       LD    D,H
       LD    E,L
       XOR   A
       DEC   HL
_strlen_loop:
       INC   HL
       CP    (HL)
       JR    NZ,_strlen_loop
       SBC   HL,DE
       LD    A,L
       EX    DE,HL
       POP   DE
       RET


;----------------------------------------------
;           String Concatenate
;----------------------------------------------
;
; in: hl = string being added to (must have sufficient space at end!)
;     de = string to add
;
strcat:
    xor  a
_strcat_find_end:
    cp   (hl)               ; end of string?
    jr   z,_strcat_append
    inc  hl                 ; no, continue looking for it
    jr   _strcat_find_end
_strcat_append:             ; yes, append string
    ld   a,(de)
    inc  de
    ld   (hl),a
    inc  hl
    or   a
    jr   nz,_strcat_append
    ret


;-------------------------------------------
;            Compare Strings
;-------------------------------------------
;  in: hl = string 1 (null terminated)
;      de = string 2 (null terminated)
;
; out:   Z  = strings equal
;        NZ = not equal
;
strcmp:
    ld   a,(de)          ; get char from string 2
    inc  de
    cp  (hl)             ; compare to char in string 1
    inc  hl
    ret  nz              ; return NZ if not equal
    or   a
    jr   nz,strcmp       ; loop until end of strings
    ret                  ; return Z


;-----------------------------------------
;        Compare Memory Blocks
;-----------------------------------------
;  in: de = memory block 1
;      hl = memory block 2
;       b = length (1-256 bytes)
;
; out: de = end of block 1 +1
;      hl = end of block 2 +1
;       z = compare match
;       c = block 1 < block 2
;
cmp_mem:
    ld   a,(de)
    cp   (hl)
    jr   nz,.skip
    inc  hl
    inc  de
    djnz cmp_mem
    ret
.skip:
    inc  hl
    inc  de
    djnz .skip
    ret


;--------------------------------------------------------------------
;            get next character, skipping spaces
;--------------------------------------------------------------------
;  in:    HL = text pointer
;
; out: NZ, A = next non-space char, HL = address of char in text
;      Z,  A = 0, HL = end of text
;
get_next:                       ; starting at next location
    inc     hl
get_arg:                        ; starting at current location
    ld      a,(hl)
    or      a
    ret     z                   ; return Z if NULL
    cp      ' '
    ret     nz                  ; return NZ if not SPACE
    jr      get_next

;-----------------------------------------------------------------
;          check for argument in current statement
;-----------------------------------------------------------------
;  in: HL = text pointer
;
; out: NZ = argument present
;       Z = end of statement
;
chkarg:
    push hl                   ; save BASIC text pointer
_chkarg_next_char:
    ld   a,(hl)               ; get char
    inc  hl
    cp   ' '                  ; skip spaces
    jr   z,_chkarg_next_char
    cp   ':'                  ; Z if end of statement
    jr   z,_chkarg_done       ; return Z if end of statement
    or   a                    ; Z if end of line
_chkarg_done:
    pop  hl                   ; restore BASIC text pointer
    ret


;-----------------------------------------------------------------
;               Print Null-terminated String
;-----------------------------------------------------------------
;  in: HL = text ending with NULL
;
prtstr:
   ld   a,(hl)
   inc  hl
   or   a
   ret  z
   call PRNCHR
   jr   prtstr


