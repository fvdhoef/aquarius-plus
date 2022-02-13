;====================================================================
; Mattel Aquarius:   Enhanced Line Editing Routines
;====================================================================
;
; changes:-
; 2015-11-13 PRINTSTR after CTRL-C to ensure that cursor goes to next line.
;            Could not type into last char position in buffer - fixed.
; 2015-11-14 Cursor now restores original charactcr color when removed.
; 2016-02-06 Return to BABASIC immediate mode loop.
; 2017-04-18 CTRL-R = retype previous line entered
; 2017-04-29 bugfix: EDIT retreiving another line if line not found
; 2017-05-06 using equates LINBUF and LINBUFLEN
;            retype clears old line before recalling history buffer
;
;---------------------------------
;      Edit a BASIC Line
;---------------------------------
; EDIT <line number>
;
ST_EDIT:
    call  STRTOVAL        ; DE = line number
    ld    a,d
    or    e
    jr    nz,.prtline
    ld    e,MO_ERR        ; if no line number then MO error
    JP    DO_ERROR
.prtline:
    ex    de,hl           ; HL = line number
    push  hl
    call  PRINTINT        ; Print line number (also puts number string in $38ea)
    pop   de
    call  FINDLIN         ; find line in BASIC program
    push  af              ; push flags (c = found line in BASIC program)
    ld    de,LINBUF       ; DE = buffer
    ld    hl,$38ea        ; HL = floating point decimal number (line number)
    call  getinteger      ; copy decimal number string to edit buffer
    pop   af              ; pop flags
    push  de              ; push buffer pointer
    jr    nc,.gotline     ; if no line found then start with empty line
    ld    hl,4            ; skip line number, next line pointer
    add   hl,bc           ; HL = address of text in BASIC line
.getline:
    ld    a,(hl)          ; get next byte in BASIC line
    or    a
    jr    z,.gotline      ; if byte = 0 then end of line
    call  expand_token    ; copy char to buffer, expanding tokens
    inc   hl              ; next byte
    jr    .getline
.gotline:
    xor   a
    ld    (de),a          ; terminate string in buffer
    pop   hl              ; pop buffer pointer into HL
    ld    a,l
    sub   low(LINBUF)
    cpl
    inc   a
    add   LINBUFLEN       ; A = length of buffer - length of line number
    ld    b,a
.editline:
    call  EDITLINE        ; edit string in buffer
.done:
    inc   sp
    inc   sp              ; clean up stack
    jp    ENTERLINE       ; back to BABASIC immediate mode ($041d)

;----------------------------------
; Copy Decimal Number String
;----------------------------------
; copies digits until non-numeric
; character found.
;
;  in: HL = source string
;      DE = dest
;
; out: HL = first non-numeric char
;      DE = end of number string in dest (null-terminated)
;
getinteger:
    ld    a,(hl)
    inc   hl
    cp    '0'
    jr    c,.done
    cp    '9'+1
    jr    nc,.done
    ld    (de),a
    inc   de
    jr    getinteger
.done:
    xor   a
    ld    (de),a     ; null-terminate destination string
    ret


;-----------------------------
;   Expand token to string
;-----------------------------
; in: A  = char or token
;     DE = dest string address
;
expand_token:
    push  hl
    bit   7,a           ; if byte is less than $80 then just copy it
    jr    z,.exp_char
    cp    $95           ; if token for PRINT then expand to '?'
    jr    z,.exp_print
    ld    hl,$0245      ; DE = system BASIC keyword table
    sub   $7f           ; keyword table number number 1~xx
    cp    $d3-$7f
    jr    c,.exp_count  ; if > $D3 then it must be a BABASIC keyword, so...
    ld    hl,TBLCMDS    ;    DE = BABASIC keyword table
    sub   $d3-$7f       ;    BABASIC keyword table entry number 1~xx
.exp_count:
    ld    c,a           ; C = keyword counter
.exp_find:
    ld    a,(hl)        ; get char of current keyword in table
    inc   hl
    or    a
    jp    p,.exp_find   ; loop back until end of keyword
    dec   c             ; Word counter - 1
    jr    nz,.exp_find  ; Keep looping until we get to the correct keyword
.exp_token:
    and   $7f           ; remove marker bit from 1st character
    ld    (de),a        ; copy character of keyword
    inc   de
    ld    a,(hl)        ; get next char of keyword
    inc   hl
    bit   7,a
    jr    z,.exp_token  ; loop until end of keyword
    jr    .exp_end
.exp_print:
    ld    a,'?'         ; show '?' shortcut for PRINT
.exp_char:
    ld    (de),a
    inc   de
.exp_end:
    pop   hl
    ret

; editing keys
; cursor left  = $0f     ; CTRL-O
; cursor right = $10     ; CTRL-P
; delete       = $1e     ; CTRL-/

;--------------------------------------------------
;        Edit Line of Text in a Buffer
;--------------------------------------------------
;  in: HL = buffer address
;       B = buffer size
; out: Carry set if pressed CTRL-C
;      buffer holds text entered by user
;
EDITLINE:
    ld    a,(CURHOLD)
    ld    de,(CURRAM)     ; hide system cursor
    ld    (de),a
    call  _showstr         ; show buffer on screen
    push  hl
    ld    d,-1
.strlen:
    ld    a,(hl)
    inc   hl
    inc   d               ; D = number of chars in string
    or    a
    jr    nz,.strlen
    pop   hl
    ld    e,0             ; E = cursor position in string
.waitkey:
    call  _show_cursor
    call  _clr_key_wait   ; wait for keypress
    call  _hide_cursor
    ld    c,a             ; C = key
    cp    $1c             ; <DELETE>?
    jr    z,.delete       ; yes,
    cp    $03             ; CTRL-C?
    jp    z,.quit         ; yes, quit with Carry set
    cp    $0d             ; CR?
    jp    z,.retn         ; yes, return with typed line in buffer
    cp    $08             ; BACKSPACE?
    jp    z,.backspace    ; yes, do backspace
    cp    $10             ; CURSOR LEFT?
    jp    z,.left         ; yes, do it
    cp    $1e             ; CURSOR RIGHT?
    jp    z,.right        ; yes, do it
    cp    $12             ; RETYP?
    jp    z,.retype       ; yes, do it
    cp    $0f
    jr    nz,.other
    ld    c,'~'           ; CTRL-O = '~'
    jr    .ascii
.other:
    cp    $20             ; some other ctrl code?
    jr    c,.waitkey      ; yes, ignore other control codes
; insert key into line
.ascii:
    ld    a,d             ; A = string length (not including null terminator)
    inc   a               ; add 1 for NULL
    cp    b               ; compare to buffer length
    jr    nc,.waitkey     ; if buffer full then don't insert
    jr    z,.insert       ; if already at end of buffer then don't open gap
; open gap in line to insert key into
    push  hl
    push  de
    push  bc
    ld    b,0
    ld    a,d             ; A = string length
    sub   e               ; subtract cursor position in string
    jr    c,.gapped       ; must not be less than zero!
    ld    c,a             ; C = distance from cursor to end of string
    add   hl,bc           ; HL = end of string
    ld    d,h
    ld    e,l
    inc   de              ; DE = end of string + 1
    inc   c               ; BC = number of chars to move
    lddr                  ; stretch right side of string to make room for key
.gapped:
    pop   bc
    pop   de
    pop   hl
; insert key into gap
.insert:
    ld    (hl),c          ; store character in buffer
    call  _showstr        ; update screen text
    call  _cursor_right
    inc   hl              ; cursor address in buffer + 1
    inc   e               ; cursor position in buffer + 1
    ld    a,d
    inc   a
    cp    b               ; if not at end of buffer
    jr    nc,.waitkey
    inc   d               ;    then end of string + 1
    jr    .waitkey

; pressed <RTN>, clean up and return with HL = buffer-1
.retn:
    call  PRINTSTR        ; move screen cursor to end of string
    call  PRNCRLF         ; print CR+LF
    xor   a               ; Carry clear = line edited
    ret

; BACKSPACE
.backspace:
    dec   e
    inc   e
    jr    z,.waitkey      ; if already at start of buffer then done
    dec   hl
    dec   e               ; move cursor left
    call  _cursor_left
; DELETE
.delete:
    ld    a,d             ; a = number of chars in string
    sub   e               ; subract cursor position in string
    jp    z,.waitkey      ; if no bytes to move then done
    ld    c,a             ; C = number of bytes to move
    dec   d               ; number of chars -1
    push  bc
    push  de
    push  hl
    ld    d,h             ; DE = address of char at cursor
    ld    e,l
    inc   hl              ; HL = address of next char
    ld    b,0             ; BC = number of bytes to move
    ldir                  ; pull right side of string left over cursor
    dec   de              ; de = new end of string (NULL)
    pop   hl              ; hl = cursor address in string
    ld    a,' '
    ld   (de),a           ; add SPACE to rub out previous last char
    call  _showstr        ; print right side of string
    xor   a
    ld    (de),a          ; remove SPACE from end of string
    pop   de
    pop   bc
    jp    .waitkey

; cursor left
.left:
    xor   a
    cp    e
    jp    z,.waitkey      ; if already at start then stay there
    call  _cursor_left
    dec   hl
    dec   e
    jp    .waitkey

; cursor right
.right:
    ld    a,e
    cp    d
    jp    nc,.waitkey       ; limit movement to inside string
    call  _cursor_right
    inc   e
    inc   hl
    jp    .waitkey


; CTRL-R = retype
.retype:
    ld    a,(SYSFLAGS)    ; if CTRL-R inactive then ignore it
    BIT   SF_RETYP,a
    JP    z,.waitkey
    inc   e
.rt_home:
    dec   e
    jr    z,.gethistory
    dec   hl
    call  _cursor_left    ; cursor left to start of buffer
    jr    .rt_home
.gethistory:
    push  hl              ; push buffer address
.clearline:
    ld    a,(hl)
    or    a
    jr    z,.line_cleared
    ld   (hl),' '         ; spaces up to end of string
    inc   hl
    jr    .clearline
.line_cleared:
    pop   hl
    call  _showstr        ; show spaces over previous string
    push  hl
    ld    c,-1
    ld    de,ReTypBuf     ; DE = history buffer
.rt_copy:
    ld    a,(de)          ; get char from history buffer
    inc   de
    ld    (hl),a          ; copy char to line buffer
    inc   hl
    inc   c               ; C = number of chars copied
    or    a
    jr    nz,.rt_copy     ; copy until done NULL
    pop   hl              ; pop buffer address
    call  _showstr        ; show string
    ld    d,c             ; D = end of string in buffer
    ld    e,0             ; E = start of buffer
.rt_end:
    jp    .waitkey


; CTRL-C
.quit:
    call  PRINTSTR        ; move screen cursor to end of string
    call  PRNCRLF         ; CR+LF
    scf                   ; set Carry flag = edit aborted
    ret


;--------------------------------------------------------------------
;         Clear Keyboard Buffer and Wait for Key
;--------------------------------------------------------------------
;
; out: A = key
;
_clr_key_wait:
    xor     a
    ld      (LSTASCI),a ; clear last key pressed
_key_wait:
    call    $1e7e       ; get last key pressed
    jr      z,_key_wait ; loop until key pressed
    push af
    ld   a,$FF          ; speaker ON
    out  ($fc),a
.click_wait:
    push af
    pop  af             ; delay 6 cycles * 256
    dec  a
    jr   nz,.click_wait
    out  ($fc),a        ; speaker OFF
    pop  af
    ret


;--------------------------------------------------------------------
;         Print String to screen without moving cursor
;--------------------------------------------------------------------
;  in: HL = string (null-terminated)
;
_showstr:
    push  hl
    push  de
    push  bc
    ld    de,(CURRAM)   ; DE = cursor address in character RAM
    ld    a,(CURCOL)
    ld    c,a           ; C = cursor column
    jr    .prt_next     ; start printing
.prt_loop:
    ld    a,c
    cp    38            ; if at column 38 then...
    jr    nz,.prt_char
    ld    c,0           ;    C = column 0
    inc   de
    inc   de            ;    skip over border to next line
    push  hl
    ld    hl,$33e7
    rst   $20           ;    compare cursor address to end of screen
    pop   hl
    jr    nc,.prt_char  ;    if end of screen then...
    push  hl
    push  bc
    call  SCROLLUP      ;       scroll screen up
    ld    bc,-40
    ld    hl,(CURRAM)
    add   hl,bc         ;       move cursor up 1 line
    ld    (CURRAM),hl
    pop   bc
    pop   hl
    ld    de,$33c1      ;       set address to start of bottom line
.prt_char:
    ld    a,(hl)
    ld    (de),a        ; put character into screen RAM
    inc   hl            ; next char in string
    inc   de            ; next screen address
    inc   c             ; next column
.prt_next:
    ld    a,(hl)        ; get next character from string
    or    a             ; NULL?
    jr    nz,.prt_loop  ; no, print it
    pop   bc
    pop   de
    pop   hl
    ret

;--------------------------------------------------------------------
; Move cursor to the previous character position on screen. If at
; column 0 then go up to colomn 37 on the previous line.
;
; Only updates CURCOL and CURRAM (no affect on cursor display).
; Cursor should be turned off to avoid leaving colored blocks behind!
;
; NOTE: does not test for wrap at start of screen, so don't call
; this function if cursor is at position 0,0!
;
_cursor_left:
    push  hl
    ld    hl,CURCOL
    ld    a,(hl)
    or    a
    jp    nz,.column
    ld    (hl),38      ; if at column 0 then goto column 38
.column:
    dec   (hl)         ; column - 1
    ld    hl,(CURRAM)
    or    a
    jr    nz,.ram
    dec   hl           ; if going up 1 line then skip over border
    dec   hl
.ram:
    dec   hl           ; character RAM address - 1
    ld    (CURRAM),hl
    pop   hl
    ret

;--------------------------------------------------------------------
; Move cursor location to the next character position. If at end of
; screen then scroll up and put cursor on the 1st column of the last
; line.
;
; Only updates CURCOL and CURRAM (no affect on cursor display).
; Cursor should be turned off to avoid leaving colored blocks behind!
;
_cursor_right:
    push  hl
    ld    hl,CURCOL
    ld    a,(HL)
    inc   a           ; cursor column + 1
    cp    38
    jr    c,.column   ; if end of line then
    ld    a,0         ;   set column to start of (next) line
.column:
    ld    (hl),a      ; update cursor column
    ld    hl,(CURRAM)
    inc   hl          ; cursor address + 1
    jr    c,.ram      ; if past end of line then
    inc   hl
    inc   hl          ;    skip over border to start of next line
    push  de
    ld    de,$33e8
    rst   $20         ;    compare cursor address to end of screen
    jr    c,.scr      ;    if past end of screen then
    push  bc
    call  SCROLLUP    ;       scroll up
    pop   bc
    ld    hl,$33c1    ;       set cursor address to start of bottom line
.scr:
    pop   de
.ram:
    ld    (CURRAM),hl ; update cursor address
    pop   hl
    ret

;--------------------------------------------------------------------
;                      Show Colored Cursor
;--------------------------------------------------------------------
; Uses color atrribute RAM to show a colored block at the cursor
; location. With appropriate colors the character 'under' the cursor
; shows through, and does not have to be restored after the cursor is
; removed.
;
_show_cursor:
    push  af
    push  hl
    ld    hl,(CURRAM)
    set   2,h             ; HL = cursor address in color RAM
    ld    a,(hl)          ; get original character color
    ld    (RUBSW),a       ; save it
    and   $0f
    cp    $04
    ld    a,$74           ; A = cursor forground/background color
    jr    nz,.show        ; if background same as cursor color
    ld    a,$47           ;     then use a different color
.show:
    ld    (hl),a          ; character color = cursor color
    pop   hl
    pop   af
    ret

;--------------------------------------------------------------------
;                     Remove Colored Cursor
;--------------------------------------------------------------------
; Use to restore normal character color when moving cursor to another
; location
_hide_cursor:
    push  af
    push  hl
    ld    hl,(CURRAM)
    set   2,h             ; HL = character position in color RAM
    ld    a,(RUBSW)
    ld    (hl),a          ; restore original character color
    pop   hl
    pop   af
    ret


