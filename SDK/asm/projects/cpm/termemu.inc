;-----------------------------------------------------------------------------
; termemu.inc
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
TEXTBASE        equ $3000 + 80
COLUMNS         equ 80
ROWS            equ 24

_text_addr:     defw TEXTBASE
_column:        defb 0
_row:           defb 0
_saved_color:   defb $8F
_oldstack:      defw 0
                defs 16
_tmpstack:
_in_escape:     defb 0

_loadcur_y:     defb 0
_loadcur_x:     defb 0

;-----------------------------------------------------------------------------
; Hide cursor
;-----------------------------------------------------------------------------
_hide_cursor:
    ; Restore saved color at cursor position
    ld      a,$E1
    out     (IO_VCTRL),a

    ld      hl,(_text_addr)
    ld      a,(_saved_color)
    ld      (hl),a

    ld      a,$61
    out     (IO_VCTRL),a
    ret

;-----------------------------------------------------------------------------
; Show cursor
;-----------------------------------------------------------------------------
_show_cursor:
    ; Save color at cursor position
    ld      a,$E1
    out     (IO_VCTRL),a

    ld      hl,(_text_addr)
    ld      a,(hl)
    ld      (_saved_color),a

    ; Set color at cursor position
    ld      a,$F9
    ld      (hl),a

    ld      a,$61
    out     (IO_VCTRL),a
    ret

;-----------------------------------------------------------------------------
; Bell
;-----------------------------------------------------------------------------
_bell:
    ld      bc,70
.1: ld      a,1
    out     (IO_CASSETTE),a
    call    .delay
    xor     a
    out     (IO_CASSETTE),a
    call    .delay
    dec     bc
    ld      a,b
    or      a,c
    jr      nz,.1
    ret

.delay:
    ld      a,192
.l: dec     a
    jr      nz,.l
    ret

;-----------------------------------------------------------------------------
; Move cursor up
;-----------------------------------------------------------------------------
_line_up:
    ; Cursor to previous line
    ld      a,(_row)
    or      a
    ret     z           ; Already on first line?
    dec     a
    ld      (_row),a
    ld      hl,(_text_addr)
    ld      bc,COLUMNS
    ccf
    sbc     hl,bc
    ld      (_text_addr),hl
    ret

;-----------------------------------------------------------------------------
; Move cursor forward
;-----------------------------------------------------------------------------
_fwdspace:
    inc     hl                  ; Increment text address
    ld      (_text_addr),hl
    ld      a,(_column)         ; Increment column
    inc     a
    cp      COLUMNS             ; End of line?
    jr      z,.next_line
    ld      (_column),a         ; Update column
    ret

.next_line:
    xor     a                   ; Column = 0
    ld      (_column),a
    ld      a,(_row)            ; Increment row
    inc     a
    cp      ROWS                ; Scroll?
    jr      z,.next_line_scroll
    ld      (_row),a            ; Update column
    ret

.next_line_scroll:
    call    _scroll             ; Scroll screen
    ld      a,ROWS-1            ; Row = ROWS-1
    ld      (_row),a
    ld      hl,TEXTBASE + COLUMNS * (ROWS-1)    ; Text addr to lower left corner
    ld      (_text_addr),hl
    ret

;-----------------------------------------------------------------------------
; Clear screen
;-----------------------------------------------------------------------------
_clrscr:
    ld      hl,TEXTBASE
    ld      bc,COLUMNS*ROWS
    ld      a,' '
    call    memset
    ret

;-----------------------------------------------------------------------------
; Start escape sequence
;-----------------------------------------------------------------------------
_start_escape:
    ld      a,1
    ld      (_in_escape),a
    ret

;-----------------------------------------------------------------------------
; Escape sequence handling
;-----------------------------------------------------------------------------
_escape_sequence:
    ld      a,(_in_escape)
    cp      1
    jr      z,.first
    cp      2
    jr      z,.second
    cp      3
    jr      z,.third
.exit_escape:
    xor     a
.1: ld      (_in_escape),a
    ret
.next:
    ld      a,(_in_escape)
    inc     a
    jr      .1

.first:
    ld      a,c
    cp      '='
    jr      nz,.exit_escape
    jr      .next
.second:
    ld      a,c
    sub     a,' '
    ld      (_loadcur_y),a
    jr      .next
.third:
    ld      a,c
    sub     a,' '
    ld      (_loadcur_x),a
    call    _set_cursor
    jr      .exit_escape

;-----------------------------------------------------------------------------
; Set cursor
;-----------------------------------------------------------------------------
_set_cursor:
    ; Check Y < ROWS
    ld      a,(_loadcur_y)
    cp      ROWS
    jr      c,.y_ok
    ld      a,ROWS-1
.y_ok:
    ld      (_row),a

    ; Check X < COLUMNS
    ld      a,(_loadcur_x)
    cp      COLUMNS
    jr      c,.x_ok
    ld      a,COLUMNS-1
.x_ok:
    ld      (_column),a

    ; _text_addr = TEXTBASE + _row * COLUMNS + _column
    assert COLUMNS = 80
    ld      h,0             ; Row * COLUMNS (assumes 80) (row * 16 + row * 64)
    ld      a,(_row)
    ld      l,a
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    ld      d,h
    ld      e,l
    add     hl,hl
    add     hl,hl
    add     hl,de

    ld      d,0             ; += _column
    ld      a,(_column)
    ld      e,a
    add     hl,de
    ld      de,TEXTBASE
    add     hl,de
    ld      (_text_addr),hl

    ret

;-----------------------------------------------------------------------------
; Cursor to upper left corner
;-----------------------------------------------------------------------------
_home_cursor:
    xor     a
    ld      (_loadcur_y),a
    ld      (_loadcur_x),a
    jp      _set_cursor

;-----------------------------------------------------------------------------
; Backspace
;-----------------------------------------------------------------------------
_backspace:
    ; Backspace
    ld      a,(_column)
    or      a                   ; At first column?
    jr      z,.2
    dec     a                   ; Decrement column
.3: ld      (_column),a         ; Update column
    ld      hl,(_text_addr)     ; Decrement text address
    dec     hl
    ld      (_text_addr),hl
    ret
.2: ld      a,(_row)
    or      a                   ; At first row?
    ret     z
    dec     a                   ; Decrement row
    ld      (_row),a
    ld      a,COLUMNS-1         ; Update column to COLUMNS-1
    jr      .3
    ret

;-----------------------------------------------------------------------------
; Carriage return
;-----------------------------------------------------------------------------
_cr:
    ; Cursor back to start of line
    ld      hl,(_text_addr)
    ld      a,(_column)
    ld      c,a
    xor     a
    ld      (_column),a
    ld      b,a
    sbc     hl,bc
    ld      (_text_addr),hl
    ret

;-----------------------------------------------------------------------------
; Line feed
;-----------------------------------------------------------------------------
_lf:
    ; Cursor to next line
    ld      a,(_row)
    cp      ROWS-1
    jp      z,_scroll
    inc     a
    ld      (_row),a
    ld      hl,(_text_addr)
    ld      bc,COLUMNS
    add     hl,bc
    ld      (_text_addr),hl
    ret

;-----------------------------------------------------------------------------
; Scroll
;-----------------------------------------------------------------------------
_scroll:
    ld      de,TEXTBASE
    ld      hl,TEXTBASE + COLUMNS
    ld      bc,COLUMNS * (ROWS-1)
    ldir
    ld      hl,TEXTBASE + COLUMNS * (ROWS-1)
    ld      bc,COLUMNS
    ld      a,' '
    call    memset
    ret

;-----------------------------------------------------------------------------
; Handle control character
;-----------------------------------------------------------------------------
_control:
    cp      a,7             ; BEL (Bell)?
    jp      z,_bell
    cp      a,8             ; BS (Backspace)?
    jp      z,_backspace
    cp      a,10            ; LF (Line Feed)?
    jp      z,_lf
    cp      a,11            ; VT (Upline)?
    jp      z,_line_up
    cp      a,12            ; FF (Forward space)?
    jp      z,_fwdspace
    cp      a,13            ; CR (Carriage Return)?
    jp      z,_cr
    cp      a,26            ; SUB (Clear Screen)?
    jp      z,_clrscr
    cp      a,27            ; ESC (Initiate Load Cursor)?
    jp      z,_start_escape
    cp      a,30            ; RS (Home Cursor)?
    jp      z,_home_cursor
    ret

;-----------------------------------------------------------------------------
; Handle the character in C
;-----------------------------------------------------------------------------
_handle_char:
    ld      a,(_in_escape)          ; In escape sequence?
    or      a
    jp      nz,_escape_sequence
    ld      a,c                     ; Control character?
    cp      32
    jr      c,_control

    ; Write to text address
    ld      hl,(_text_addr)
    ld      (hl),c
    jp      _fwdspace

;-----------------------------------------------------------------------------
; CONOUT - Console output
;
; Write the character in C to the screen.
;-----------------------------------------------------------------------------
termemu_out:
    ; We need to set up a temporary stack pointer here, since the current
    ; stack pointer can be in the bottom 16KB of memory and we are switching
    ; that out in this routine.

    ld      hl,0            ; Save stack pointer
    add     hl,sp
    ld      (_oldstack),hl
    ld      sp,_tmpstack    ; Set temp stack pointer

    in      a,(IO_BANK0)    ; Save bank 0
    push    a
    ld      a,$C0           ; Set bank 0 to screen memory
    out     (IO_BANK0),a

    call    _hide_cursor    ; Hide cursor
    call    _handle_char    ; Handle the character
    call    _show_cursor    ; Show cursor

    pop     a               ; Restore bank 0
    out     (IO_BANK0),a
    ld      hl,(_oldstack)  ; Restore stack point
    ld      sp,hl
    ret
