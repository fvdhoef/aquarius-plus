;=============================================================
;                     Windowed Text
;=============================================================
;
; - Window is colored and (optionally) has a border and
;   title. Text and border have their own foreground &
;   background colors.
;
; - Custom text rendering via WinPrtChr. Does not use any
;   system variables or routines. Text printing optimized
;   for speed! (many times faster than system routines).
;   To get the fastet possible speed, most housekeeping
;   is off-loaded to the user.
;
; - WinPrtChr does no control code parsing, renders all
;   characters direct to screen.
;
; - Text is NOT clipped or wrapped at window edges.
;
; - NewLine function does explicit CR/LF. Clears to end
;   of line, then moves cursor to beginning of next line.
;   Scrolls up if at bottom of window.
;
; - Backspace function moves cursor left and deletes char.
;
; - Cursor is kept hidden.
;
; - Uses alternate registers HL' and DE' to keep track of
;   cursor position. These registers must be preserved!
;   Watch out for system functions such as KEYWAIT, which
;   uses alternate registers. If alternate registers are
;   currupted you should call OpenWindow or WinSetCursor
;   to reset them.
;
; changes:
; 2017-05-17 bugfix: NewLine blanked several lines if
;            cursor x was beyond right side of window.
; 2017-06-12 V1.0  bumped to release version
;
;---------------------------------------------------------
;
; OpenWindow - display window, intitialize cursor
;    in: IX  = window
;   out: HL' = cursor address in screen RAM
;        DE' = x,y position in window
;
; ClearWindow - clear text area and set cursor to 0,0
;               relative to window
;     in: IX = window
;
; ShowTitle - Show Title in top bar
;     in: IX = window
;
; DrawBorder - draw border around window
;     in: IX = window
;          A = color   foreground*16+background
;
; ColorWindow - set text color inside window
;     in: IX = window
;          A = color   foreground*16+background
;
; WinPrtChr - print character in window
;     in: A = char
;
; WinPrtChrs - print n chars in window
;         HL = character string
;          A = number of chars
;
; WinPrtStr - print string in window
;         HL = string
;
; WinPrtMsg - print inline string in window
;      in: string in code after CALL WinPrtMsg
;
; WinSetCursor - set cursor position in window
;      in: IX = window
;           D = x
;           E = y
;
; CursorAddr - calculate cursor address from position in window
;   in: IX = window
;       DE = x,y relative to window
;  out: HL = address in screen RAM
;
; TextAddr - calculate screen address from text x,y position
;      in: D = screen x, E = screen y
;     out: HL = address in screen RAM
;
; ScrollWindow - scroll window text up 1 line
;      in: IX = window
;
; NewLine - clear to end of line, start next line
;      in: IX = window
;
; ClearToEnd - print spaces to end of line
;      in: IX = window
;
; BackSpace - move cursor left and clear text under it
;
; WinCatDisk - Minimalist directory listing (shows all filenames)
;
;  out: A = number of files listed
;       z = no files listed
;       c = disk error
;
;------------------------------------------------------
; window structure
; STRUCTURE window,0
;      BYTE win_flags    ; window attributes
;      BYTE win_color    ; text color (foreground*16+background)
;      BYTE win_bcolor   ; border color
;      BYTE win_x        ; x position (column)
;      BYTE win_y        ; y position (line)
;      BYTE win_w        ; width of interior
;      BYTE win_h        ; height of interior
;      WORD win_title    ; pointer to title string
; ENDSTRUCT window
;
; window flag bits
;WA_BORDER = 0           ; window has a border
;WA_TITLE  = 1           ; window has a title
;WA_CENTER = 2           ; title is centered
;WA_SCROLL = 3           ; scroll enabled
;
;--------------------------------------------------------
;             Open a "window" on screen
;--------------------------------------------------------
;
;  in: IX-> window structure
; out: text area = window interior
;      cursor set to window top/left (0,0)
;      HL' & DE' store cursor address and x,y position.
;
; - subsequent calls to PrtChr, PrtMsg etc. render text
;   inside the window. Window must be opened first or the
;   cursor location will not be defined and may write to
;   memory outside the screen!
;
; - No clipping, no layers. Does not save area under window.
;   To 'close' a window you must open another window over the
;   top of it.
;
; - IX points to window definition, which is constant data
;   that may be stored in ROM.
;
OpenWindow:
       PUSH  HL
       PUSH  DE
       CALL  DrawBorder         ; draw border around window
       CALL  ShowTitle          ; show title in top border
       LD    DE,0
       CALL  WinSetCursor       ; set cursor to window top/left
       CALL  ClearWindow        ; clear text area inside window
       LD    A,(IX+win_color)
       CALL  ColorWindow        ; set text colors inside window
       POP   DE
       POP   HL
       RET

ShowTitleBar:
;----------------------------------------------------
;              Show Window Title
; in IX-> window
ShowTitle:
       BIT   WA_TITLE,(IX+win_flags)  ; if no title then quit
       RET   Z
       BIT   WA_BORDER,(IX+win_flags) ; has a border?
       JR    NZ,_tb_string      ; if yes then don't render title bar
       LD    D,(IX+win_x)
       LD    E,(IX+win_y)
       DEC   E
       CALL  WinTextAddr        ; HL = char address of window top/left
       LD    B,(IX+win_w)
       LD    C,(IX+win_bcolor)
       LD    A," "
       CALL  hline              ; render title bar
_tb_string:
       LD    L,(IX+win_Title)
       LD    H,(IX+win_Title+1) ; HL = title string
       LD    A,L
       OR    H                  ; quit if no title
       RET   Z
       PUSH  HL                 ; save string address
       LD    A,1                ; A = title text position
       BIT   WA_CENTER,(IX+win_flags)
       JR    Z,_tb_title        ; if centered...
       CALL  strlen
       LD    B,A                ;    B = title string length
       LD    A,(IX+win_w)
       SUB   B
       SRA   A                  ;    A = (window width - title length) / 2
_tb_title:
       ADD   (IX+win_x)
       LD    D,A                ; x = window x + title offset
       LD    E,(IX+win_y)
       DEC   E                  ; y = title bar
       CALL  WinTextAddr
       POP   DE
       EX    DE,HL              ; HL-> title string, DE-> title bar
       JP    strcpy             ; copy title string to bar


;------------------------------------------------
;            Draw Window Border
;------------------------------------------------
;
; in: IX = window structure
;      A = color
;
DrawBorder:
       BIT   WA_BORDER,(IX+win_flags)
       RET   Z
       LD    D,(IX+win_x)
       LD    E,(IX+win_y)
       CALL  WinTextAddr        ; HL = char address of window top/left
       LD    DE,-41
       ADD   HL,DE              ; HL = border address in color RAM
       PUSH  HL                 ; save top/left address
       LD    C,(IX+win_bcolor)  ; C = border color
       LD    A,222
       CALL  ShowGlyph          ; draw top/left corner
       INC   HL
       LD    B,(IX+win_w)
       LD    A,172
       CALL  hline              ; top bar
       LD    A,206
       CALL  ShowGlyph          ; top right corner
       LD    DE,40
       ADD   HL,DE
       LD    B,(IX+win_h)
       LD    A,214
       CALL  vline              ; right side
       LD    A,223
       CALL  ShowGlyph          ; bottom/right corner
       POP   HL                 ; restore top/left address
       ADD   HL,DE              ; down 1 line
       LD    B,(IX+win_h)
       LD    A,214
       CALL  vline              ; left side
       LD    A,207
       CALL  ShowGlyph          ; bottom/left corner
       INC   HL
       LD    B,(IX+win_w)
       LD    A,172
       JP    hline              ; bottom bar

;-----------------------------
;   Show colored character
;-----------------------------
; HL-> screen char RAM
;  A = char
;  C = color
ShowGlyph:
       LD   (HL),A
       SET  2,H
       LD   (HL),C
       RES  2,H
       RET


;----------------------------------------------------------------
;                 Clear Text Area in Window
;----------------------------------------------------------------
;
; in: IX = window
;
ClearWindow:
       EXX
       LD    D,(ix+win_x)
       LD    E,(ix+win_y)
       CALL  WinTextAddr       ; HL' = char address
       LD    C,(ix+win_h)
       LD    DE,40
       PUSH  HL
       LD    A," "             ; A = SPACE
_clrw_line:
       LD    B,(ix+win_w)
       PUSH  HL
_clrw_char:
       LD    (HL),A            ; poke SPACE into screen
       INC   HL
       DJNZ  _clrw_char        ; 1 line of SPACEs
       POP   HL
       ADD   HL,DE
       DEC   C                 ; next line
       JR    NZ,_clrw_line
       POP   HL                ; HL' = char address
       LD    DE,0              ; DE' = window x,y
       EXX
       RET


;--------------------------------------------------
;         Color Text Area in Window
;--------------------------------------------------
;
; in: IX = window
;      A = color
;
; uses: BC,DE
;
ColorWindow:
       LD    D,(ix+win_x)
       LD    E,(ix+win_y)
       CALL  WinTextAddr
       SET   2,H               ; HL = address in color RAM
       LD    DE,40             ; DE = increment to next line
       LD    C,(IX+win_h)      ; C = number of lines
.line:
       PUSH  HL                ; push start of line addr
       LD    B,(ix+win_w)      ; B = window width
.char:
       LD    (HL),A            ; set char color
       INC   HL                ; next char
       DJNZ  .char
       POP   HL                ; pop start of line addr
       ADD   HL,DE             ; advance to next line
       DEC   C
       JR    NZ,.line          ; next line
       RET


;------------------------------------------------
;            Draw Horizontal Line
;------------------------------------------------
;
; in: HL = char address in char RAM
;      A = char
;      B = width
;      C = color
;
; out: HL = next char address
;       B = 0
;
hline: PUSH  DE
       LD    D,H
       LD    E,L
       set   2,H      ; HL = color RAM
hl_draw:
       LD    (DE),A   ; poke char
       LD    (HL),C   ; poke color
       INC   DE
       INC   HL
       DJNZ  hl_draw
       EX    DE,HL
       POP   DE
       RET

;------------------------------------------------
;            Draw Vertical line
;------------------------------------------------
;  in: HL = char address in char RAM
;       A = character
;       B = height
;       C = color
;
; out: HL = next line
;       B = 0
;
vline: PUSH  DE
       LD    DE,40        ; 40 chars per line
.line:
       LD    (HL),A       ; poke char
       SET   2,H          ; to color RAM
       LD    (HL),C       ; poke color
       res   2,H          ; back to char RAM
       ADD   HL,DE        ; down to next line
       DJNZ  .line
       POP   DE
       RET

;-----------------------------------------------
;    calculate char address in screen RAM
;-----------------------------------------------
;
;  in: D = screen x, E = screen y
;
; out: HL = screen address
;
WinTextAddr:
       push  af
       ld    a,e                ; A = y
       add   a,a                ;    *2
       add   a,a                ;    *4
       add   a,e                ;    *5
       add   a,a                ;   *10
       ld    h,0
       add   a
       rl    h                  ;   *20
       add   a
       rl    h                  ;   *40
       add   d                  ;   + x
       ld    l,a
       ld    a,h
       adc   $30                ; HL = $3000 + y*40 + x
       ld    h,a
       pop   af
       RET


;-----------------------------------------
;  Place cursor at top left of window
;
CursorHome:
        ld      de,0

;-------------------------------------------
;     position cursor at x,y in window
;-------------------------------------------
; Sets up alternate registers for cursor
; address, x and y coordinates.
;
; NOTE: this function must have been called before
;       printing text into window with PRTSTR!
;
; in:    D = x (relative to window top/left)
;        E = y             ''
;
; out: HL' = cursor address
;       D' = x
;       E' = y
;
WinSetCursor:
        push    de
        exx
        pop     de                  ; DE' = x,y in window
        call    CursorAddr          ; HL' = screen address
        exx
        ret

;---------------------------------------------------------------
;      calculate screen address,x,y from window x,y
;---------------------------------------------------------------
;
; in: IX = window
;     DE = x,y relative to window eg. 0,0 = window top/left
;
;out: HL = address in screen char RAM
;
CursorAddr:
        push    de
        ld      a,(ix+win_x)
        add     d                   ; D = screen x
        ld      d,a
        ld      a,(ix+win_y)
        add     e                   ; E = screen y
        ld      e,a
        call    WinTextAddr         ; HL = screen address
        pop     de
        ret

;------------------------------------------------------------
;        print spaces to start of next line in window
;------------------------------------------------------------
;
;  in: IX = window
;
NewLine:
       push  af
       ld    a,(ix+win_w)
       exx                  ; now using alternate registers
       ld    b,d            ; B = window cursor x
       ld    d,a            ; D = window width
       sub   b              ; calc distance to end of line in window
       jr    z,_nl_down
       jr    c,_nl_down     ; if at or past end of line then no fill required
       ld    b,a
       ld    a," "
_nl_spaces:
       ld    (hl),a         ; fill with spaces until end of line
       inc   hl
       djnz  _nl_spaces
_nl_down:
       ld    a,e            ; A = window cursor y
       inc   a              ; A + 1 = next line
       cp    (ix+win_h)     ; below last line in window?
       jr    c,_nl_setcursor
       call  ScrollWindow   ; yes, scroll window text up 1 line
       dec   a              ;      back to last line
_nl_setcursor:
       ld    e,a            ; y = next line
       ld    d,0            ; x = beginning of line
       call  CursorAddr     ; update x, y, screen address in DE', HL'
       exx                  ; now using base registers
       pop   af
       ret

;------------------------------------------------------------
;      print spaces to end of current line
;------------------------------------------------------------
;
;  in: IX = window
;
ClearToEnd:
       push  af
       ld    a,(ix+win_w)
       exx                  ; now using alternate registers
       sub   d              ; calc distance to end of line
       jr    c,.done        ; if already past end then done
       jr    z,.done
       ld    b,a
       jr    .fill
.next:
       inc   d              ; next x
       inc   hl             ; next address
.fill:
       ld    a," "
       ld    (hl),a         ; fill with spaces until end of line
       djnz  .next
.done:
       exx                  ; now using base registers
       pop   af
       ret

;-------------------------------------------------------------
;             Scroll window text up 1 line
;-------------------------------------------------------------
;
;  in: IX = window
;
ScrollWindow:
       PUSH  AF
       PUSH  BC
       PUSH  DE
       PUSH  HL
       LD    D,(IX+win_x)
       LD    E,(IX+win_y)
       CALL  WinTextAddr      ; HL = char address of window
       EX    DE,HL            ; DE = current line in char RAM
       LD    A,(IX+win_h)
       DEC   A                ; scrolling number of lines -1
       JR    Z,_sw_blankline
       LD    B,A              ; B = number of lines to scroll
_sw_line:
       PUSH  BC               ; save number of lines
       LD    HL,40
       ADD   HL,DE            ; HL = next line in char RAM
       PUSH  HL               ; save next line
       LD    C,(IX+win_w)
       LD    B,0              ; BC = chars per line
       PUSH  BC
       PUSH  DE
       PUSH  HL
       LDIR                   ; copy next line chars over current line
       POP   DE
       POP   HL
       POP   BC
       SET   2,D              ; DE = current line in color RAM
       SET   2,H              ; HL = next line in color RAM
       LDIR                   ; copy next line colors over current line
       POP   DE               ; DE = next line
       POP   BC               ; B = number of lines to go
       DJNZ  _sw_line         ; next line
       EX    DE,HL            ; HL = bottom line
_sw_blankline:
       LD    A,(IX+win_w)     ; A = window width
       LD    C,(IX+win_color) ; C = text color
       PUSH  HL
       LD    B,A
_sw_blankchrs:
       LD    (HL)," "         ; clear bottom line
       INC   HL
       DJNZ  _sw_blankchrs
       POP   HL
       SET   2,H              ; HL =  color RAM
       LD    B,A
_sw_blankcolors:
       LD    (HL),C
       INC   HL
       DJNZ  _sw_blankcolors  ; set bottom line color
       POP   HL
       POP   DE
       POP   BC
       POP   AF
       RET


;-------------------------------------------------------------
;                 print inline message
;-------------------------------------------------------------
;
; in: null-terminated string in code after CALL
;
WinPrtMsg:
       EX     (SP),HL    ; get return address,save HL
.pm_loop:
       LD     A,(HL)     ; get inline text char
       INC    HL         ; point to next char
       AND    A
       JR     Z,.pm_done ; if null then done
       CP     CR
       JR     NZ,.pm_ascii
       CALL   NewLine    ; if CR then new line
       JR     .pm_loop
.pm_ascii:
       CALL   WinPrtChr  ; print char
       JR     .pm_loop   ; next char
.pm_done:
       EX     (SP),HL    ; update return address, restore HL
       RET


;------------------------------------------------------------
;                Print String at HL
;------------------------------------------------------------
; accepts the following control codes:-
;          CR = carriage return $0D
;
; in: HL = null-terminated string
;
WinPrtStr:
       LD    A,(HL)
       OR    A
       RET   Z
       INC   HL
       CP    CR
       JR    Z,.cr
       CALL  WinPrtChr
       JR    WinPrtStr
.cr:   CALL  NewLine
       JR    WinPrtStr


;------------------------------------------------------------
;              Print Characters at HL
;------------------------------------------------------------
; in: HL = string
;      A = number of chars to print
;
WinPrtChrs:
       PUSH  BC
       LD    B,A
.nextchr:
       LD    A,(HL)
       INC   HL
       CALL  WinPrtChr
       DJNZ  .nextchr
       POP   BC
       RET

;------------------------------------------------------------
;               Show Character on Screen
;------------------------------------------------------------
;
;  in: A = char to print
;
; NOTE: - shows _all_ characters, control codes not parsed!
;       - does not wrap at end of line!
;       - does not scroll if reached bottom of screen!
WinPrtChr:
       EXX
       LD    (HL),A      ; poke char into screen memory
       INC   HL          ; HL' = next screen address
       INC   D           ; D' = next screen x
       EXX
       RET


; back-space in window
BackSpace:
       EXX
       LD    (HL)," "    ; poke SPACE into screen memory
       DEC   HL          ; HL' = previous screen address
       DEC   D           ; D' = previous screen x
       EXX
       RET


;--------------------------------------------------------------------
;                      Catalog Disk
;--------------------------------------------------------------------
;
; Minimalist directory listing (shows all filenames)
;
;  out: A = number of files listed
;       z = no files listed
;       c = disk error
;
WinCatDisk:
    call    usb__ready              ; mount disk
    JR      NZ,.cat_fail
    ld      hl,.cat_star
    CALL    usb__open_read          ; open directory
    CP      CH376_INT_DISK_READ
    JR      Z,.cat_list
.cat_fail:
    XOR     A                       ; z, no files
    SCF                             ; c, disk error
    RET
.cat_list:
    LD      E,0                     ; E = number of files listed
.cat_loop:
    LD      A,CH376_CMD_RD_USB_DATA
    OUT     (CH376_CONTROL_PORT),A  ; command: read USB data (directory entry)
    IN      A,(CH376_DATA_PORT)     ; A = number of bytes in CH376 buffer
    OR      A                       ; if bytes = 0 then read next entry
    JP      Z,.cat_next
    LD      HL,-16
    ADD     HL,SP                   ; allocate 16 bytes on stack
    LD      SP,HL
    LD      B,12                    ; B = 11 bytes filename, 1 byte file attributes
    LD      C,CH376_DATA_PORT
    INIR                            ; get filename, attributes
    LD      B,16
.absorb_bytes:
    IN      A,(CH376_DATA_PORT)     ; absorb bytes until filesize
    DJNZ    .absorb_bytes
    LD      B,4                     ; B = 4 bytes file size
.read_size:
    INIR                            ; get file size
    LD      BC,-5
    ADD     HL,BC                   ; HL = &attributes
    LD      A,(HL)                  ;  A = attributes
    LD      BC,-11                  ; HL = &filename
    ADD     HL,BC
    LD      C,A                     ; C = attributes
    BIT     ATTR_B_DIRECTORY,C
    JR      Z,.cat_name
    ld      a,'<'                   ; if directory then print '<'
    CALL    WinPrtChr
.cat_name:
    LD      B,8                     ; 8 chars in file name
.cat_name_loop:
    LD      A,(HL)
    INC     HL
    CALL    WinPrtChr               ; print name char
    DJNZ    .cat_name_loop
    BIT     ATTR_B_DIRECTORY,C
    JR      NZ,.cat_extn
    LD      A,' '                   ; if file then print ' ' between name & extn
    CALL    WinPrtChr
.cat_extn:
    LD      B,2
.cat_extn_loop:
    LD      A,(HL)
    INC     HL
    CALL    WinPrtChr               ; print 1st & 2nd extn chars
    DJNZ    .cat_extn_loop
    LD      A,(HL)
    BIT     ATTR_B_DIRECTORY,C
    JR      Z,.cat_extn_last        ; if not directory then print last extn char
    CP      ' '
    JR      NZ,.cat_extn_last
    LD      A,'>'                   ; if space then print '>'
.cat_extn_last:
    CALL    WinPrtChr               ; print 3rd extn char
.cat_extn_end:
    EXX
    LD      A,D                     ; get window cursor x
    EXX
    ADD     12
    CP      (IX+win_w)              ; if insufficient space for another name then new line
    JR      C,.cat_pad
    CALL    NewLine
    JR      .cat_printed
.cat_pad:
    LD      A," "                   ; else padding space after filename
    CALL    WinPrtChr
    INC     E
.cat_printed:
    LD      HL,16
    ADD     HL,SP                   ; clean up stack
    LD      SP,HL
.cat_go:
    LD      A,CH376_CMD_FILE_ENUM_GO
    OUT     (CH376_CONTROL_PORT),A  ; command: read next filename
    CALL    usb__wait_int           ; wait until done
.cat_next:
    CP      CH376_INT_DISK_READ     ; more entries?
    jp      Z,.cat_loop             ; yes, get next entry
    EXX
    LD      A,D                     ; at start of line?
    EXX
    OR      A
    CALL    NZ,NewLine              ; no, newline
    LD      A,E                     ; A = number of files listed
    OR      A                       ; nc, z if no files
    RET

.cat_star:
    db "*",0


