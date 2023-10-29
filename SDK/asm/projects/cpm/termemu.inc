;-----------------------------------------------------------------------------
; termemu.inc
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
_text_addr: defw $3000
_text_col:  defb 0
_text_row:  defb 0
_saved_col: defb $8F
_oldstack:  defw 0
            defs 16
_tmpstack:

;-----------------------------------------------------------------------------
; Hide cursor
;-----------------------------------------------------------------------------
_hide_cursor:
    ; Restore saved color at cursor position
    ld      a,$E1
    out     (IO_VCTRL),a

    ld      hl,(_text_addr)
    ld      a,(_saved_col)
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
    ld      (_saved_col),a

    ; Set color at cursor position
    ld      a,$F9
    ld      (hl),a

    ld      a,$61
    out     (IO_VCTRL),a
    ret

;-----------------------------------------------------------------------------
; CONOUT - Console output
;
; Write the character in C to the screen.
;-----------------------------------------------------------------------------
termemu_out:
    ; We need to set up a temporary stack pointer here, since the current
    ; stack pointer can be in the bottom 16KB of memory and we are switching
    ; that out in this routine.

    ; Save stack pointer
    ld      hl,0
    add     hl,sp
    ld      (_oldstack),hl
    ; Set temp stack pointer
    ld      sp,_tmpstack

    ; Save bank 0 and set to screen memory
    in      a,(IO_BANK0)
    push    a
    ld      a,$C0
    out     (IO_BANK0),a

    ; Hide cursor
    call    _hide_cursor

    ; Write to text address
    ld      hl,(_text_addr)
    ld      a,c
    cp      a,13       ; CR?
    jr      z,.cr
    cp      a,10
    jr      z,.lf      ; LF?
    cp      a,8
    jp      z,.bs      ; BS?
    ld      (hl),a

    ; Increment text address
    inc     hl
    ld      (_text_addr),hl

    ld      a,(_text_col)
    inc     a
    cp      80
    jr      z,.next_line
    ld      (_text_col),a

.done:
    call    _show_cursor

    ; Restore bank 0
    pop     a
    out     (IO_BANK0),a

    ; Restore stack point
    ld      hl,(_oldstack)
    ld      sp,hl
    ret

.next_line:
    xor     a
    ld      (_text_col),a
    ld      a,(_text_row)
    inc     a
    cp      25
    jr      z,.next_line_scroll
    ld      (_text_row),a
    jr      .done

.next_line_scroll:
    call    .do_scroll
    ld      a,24
    ld      (_text_row),a
    ld      hl,$3000 + 80 * 24
    ld      (_text_addr),hl
    jr      .done

.cr:
    ; Cursor back to start of line
    ld      hl,(_text_addr)
    ld      a,(_text_col)
    ld      c,a
    xor     a
    ld      (_text_col),a
    ld      b,a
    sbc     hl,bc
    ld      (_text_addr),hl
    jr      .done

.lf:
    ; Cursor to next line
    ld      a,(_text_row)
    cp      24
    jr      z,.lf_scroll
    inc     a
    ld      (_text_row),a

    ld      hl,(_text_addr)
    ld      bc,80
    or      a   ; Clear carry
    adc     hl,bc
    ld      (_text_addr),hl
    jr      .done

.lf_scroll:
    call    .do_scroll
    jr      .done

.do_scroll:
    ld      de,$3000
    ld      hl,$3000 + 80
    ld      bc,80 * 24
    ldir
    ld      hl,$3000 + 80 * 24
    ld      bc,80
    ld      a,' '
    call    memset
    ret

.bs:
    ; Backspace
    ld      a,(_text_col)
    or      a                   ; At first column?
    jr      z,.bs2
    dec     a                   ; Decrement column
.bs3:
    ld      (_text_col),a       ; Update column

    ld      hl,(_text_addr)     ; Decrement text address
    dec     hl
    ld      (_text_addr),hl
.bs_done:
    jp      .done

.bs2:
    ld      a,(_text_row)
    or      a                   ; At first row?
    jr      z,.bs_done
    dec     a                   ; Decrement row
    ld      (_text_row),a
    ld      a,79                ; Update column to 79
    jr      .bs3
