VERSION  = 1
REVISION = 0

CR        = $0D

; colors
BLACK     = 0
RED       = 1
GREEN     = 2
YELLOW    = 3
BLUE      = 4
MAGENTA   = 5
CYAN      = 6
WHITE     = 7
GREY      = 8
AQUA      = 9
PURPLE    = 10
DKBLUE    = 11
STRAW     = 12
DKGREEN   = 13
DKRED     = 14
BLACK2    = 15

;-------------------------------------------------------------------
;                       System Variables
;-------------------------------------------------------------------
LASTKEY  = $380E ; 14350           SCAN CODE of last key pressed
SCANCNT  = $380f ; 14351           number of SCANS key has been down for

; start a structure definition
; eg. STRUCTURE mystruct,0
STRUCTURE MACRO name,offset
`name`_offset EQU offset
count     = offset
ENDM

; allocate 1 byte in structure
; eg. BYTE char1
BYTE      MACRO name
name      EQU   count
count     = count+1
ENDM

; allocate 2 bytes in structure
; eg. WORD int1
WORD      MACRO name
name      EQU count
count     = count+2
ENDM

; allocate 4 bytes in structure
; eg. LONG longint1
LONG      MACRO name
name      EQU count
count     = count+4
ENDM

; allocate multiple bytes
; typically used to embed a structure inside another
; eg. STRUCT filename,11
STRUCT    MACRO name,size
name      EQU   count
count     = count+size
ENDM

; finish defining structure
; eg. ENDSTRUCT mystruct
ENDSTRUCT MACRO name
`name`.size EQU count-`name`_offset
ENDM

; window structure
STRUCTURE window,0
    BYTE win_flags    ; window attributes
    BYTE win_color    ; text color (foreground*16+background)
    BYTE win_bcolor   ; border color
    BYTE win_x        ; x position (column)
    BYTE win_y        ; y position (line)
    BYTE win_w        ; width of interior
    BYTE win_h        ; height of interior
    WORD win_title    ; pointer to title string
ENDSTRUCT window

; window flag bits
WA_BORDER = 0           ; window has a border
WA_TITLE  = 1           ; window has a title
WA_CENTER = 2           ; title is centered
WA_SCROLL = 3           ; scroll enabled


    org $38E1

    ; Header and BASIC stub
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db "AQPLUS"
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp main


path.size = 37           ; length of file path buffer

PathName: defs path.size 
FileName: defs 13
FileType: db 0
BinStart: dw 0
BinLen:   dw 0
DosFlags: db 0


main:


; ; Rom recognization
; ; 16 bytes
; RECOGNIZATION:
;     db  66, 79, 79, 84, 83, 156, 84, 176, 82, 108, 65, 100, 80, 168, 128, 112

    ; ROM_ENTRY:
    ; init CH376
    call    usb__check_exists  ; CH376 present?
    jr      nz,.no_ch376
    call    usb__set_usb_mode  ; yes, set USB mode
.no_ch376:
    call    usb__root          ; root directory

    ; init keyboard vars
    xor     a
    ld      (LASTKEY),a
    ld      (SCANCNT),a

    ; show splash screen (Boot menu)
SPLASH:
    ld      a,CYAN
    call    clearscreen
    ld      b,40
    ld      hl,$3000
.topline:
    ld      (hl),' '
    set     2,h
    ld      (hl),WHITE*16+BLACK ; black border, white on black chars in top line
    res     2,h
    inc     hl
    djnz    .topline

    call    usb__root          ; root directory
    CALL    PT3_PLAY           ; Music player
    JR      SPLASH

    ; windowed text functions
    include "windows.asm"

    ; USB Disk Driver
    include "ch376.asm"

;-----------------------------------
;       Clear Screen
;-----------------------------------
; - user-defined colors
; - doesn't clear last 24 bytes
; - doesn't show cursor
;
; in: A = color attribute (background*16 + foreground)
clearscreen:
    push    hl
    ld      hl,$3000
    ld      c,25
.line:
    ld      b,40
.char:
    ld      (hl),' '
    set     2,h
    ld      (hl),a
    res     2,h
    inc     hl
    djnz    .char
    dec     c
    jr      nz,.line
    pop     hl
    ret


;----------------------------------------------------------------
;                         Set Path
;----------------------------------------------------------------
;
;    In:    HL = string to add to path (NOT null-terminated!)
;            A = string length
;
;   out:    DE = original end of path
;            Z = OK
;           NZ = path too long
;
; path with no leading '/' is added to existing path
;         with leading '/' replaces existing path
;        ".." = removes last subdir from path
;
dos__set_path:
    PUSH   BC
    LD     C,A               ; C = string length
    LD     DE,PathName
    LD     A,(DE)
    CP     '/'               ; does current path start with '/'?
    JR     Z,.gotpath
    CALL   usb__root         ; no, create root path
.gotpath:
    INC    DE                ; DE = 2nd char in pathname (after '/')
    LD     B,path.size-1     ; B = max number of chars in pathname (less leading '/')
    LD     A,(HL)
    CP     '/'               ; does string start with '/'?
    JR     Z,.rootdir        ; yes, replace entire path
    JR     .path_end         ; no, goto end of path
.path_end_loop:
    INC    DE                ; advance DE towards end of path
    DEC    B
    JR     Z,.fail           ; fail if path full
.path_end:
    LD     A,(DE)
    OR     A
    JR     NZ,.path_end_loop
; at end-of-path
    LD     A,'.'             ; does string start with '.' ?
    CP     (HL)
    JR     NZ,.subdir        ; no
; "." or ".."
    INC    HL
    CP     (HL)              ; ".." ?
    JR     NZ,.ok            ; no, staying in current directory so quit
.dotdot:
    DEC    DE
    LD     A,(DE)
    CP     '/'               ; back to last '/'
    JR     NZ,.dotdot
    LD     A,E
    CP     low(PathName)     ; at root?
    JR     NZ,.trim
    INC    DE                ; yes, leave root '/' in
.trim:
    XOR    A
    LD     (DE),A            ; NULL terminate pathname
    JR     .ok               ; return OK
.rootdir:
    PUSH   DE                ; push end-of-path
    JR     .nextc            ; skip '/' in string, then copy to path
.subdir:
    PUSH   DE                ; push end-of-path before adding '/'
    LD     A,E
    CP     low(PathName)+1   ; at root?
    JR     Z,.copypath       ; yes,
    LD     A,'/'
    LD     (DE),A            ; add '/' separator
    INC    DE
    DEC    B
    JR     Z,.undo           ; if path full then undo
.copypath:
    LD     A,(HL)            ; get next string char
    CALL   dos__char         ; convert to MSDOS
    LD     (DE),A            ; store char in pathname
    INC    DE
    DEC    B
    JR     Z,.undo           ; if path full then undo and fail
.nextc:
    INC    HL
    DEC    C
    JR     NZ,.copypath      ; until end of string
.nullend:
    XOR    A
    LD     (DE),A            ; NULL terminate pathname
    JR     .copied
; if path full then undo add
.undo:
    POP    DE                ; pop original end-of-path
.fail:
    XOR    A
    LD     (DE),A            ; remove added subdir from path
    INC    A                 ; return NZ
    JR     .done
.copied:
    POP    DE                ; DE = original end-of-path
.ok:
    CP     A                 ; return Z
.done:
    POP    BC
    RET


;----------------------------------------------------------
;      Convert FAT filename to DOS filename
;----------------------------------------------------------
;
; eg. "NAME    EXT" -> "NAME.EXT",0
;
;   in: HL = FAT filename (11 chars)
;       DE = DOS filename string (13 chars)
;
; NOTE: source and destination can be the same string, but
;       string must have space for 13 chars.
;
dos__name:
    push  bc
    push  de
    push  hl
    ld    b,8
.getname:
    ld    a,(hl)       ; get name char
    inc   hl
    cp    " "          ; don't copy spaces
    jr    z,.next
    ld    (de),a       ; store name char
    inc   de
.next:
    djnz  .getname
    ld    a,(hl)       ; A = 1st extn char
    cp    " "
    jr    z,.end       ; if " " then no extn
    ex    de,hl
    ld    (hl),"."     ; add separator
    ex    de,hl
    inc   de
    ld    b,3          ; 3 chars in extn
.extn:
    inc   hl
    ld    c,(hl)       ; C = next extn char
    ld    (de),a       ; store current extn char
    inc   de
    dec   b
    jr    z,.end       ; if done 3 chars then end of extn
    ld    a,c
    cp    ' '          ; if space then end of extn
    jr    nz,.extn
.end:
    xor   a
    ld    (de),a       ; NULL end of DOS filename string
.done:
    pop   hl
    pop   de
    pop   bc
    ret

;------------------------------------------------------------------------------
;              Convert Character to MSDOS equivalent
;------------------------------------------------------------------------------
;  Input:  A = char
; Output:  A = MDOS compatible char
;
; converts:-
;     lowercase to upppercase
;     '=' -> '~' (in case we cannot type '~' on the keyboard!)
;
dos__char:
    CP      'a'
    JR      C,.uppercase
    CP      'z'+1          ; convert lowercase to uppercase
    JR      NC,.uppercase
    AND     $5f
.uppercase:
    CP      '='
    RET     NZ             ; convert '=' to '~'
    LD      A,'~'
    RET


;=====================================================================
;                  Miscellaneous functions
;=====================================================================

; string functions

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

    ; keyboard scan
    include "keycheck.asm"
DEBOUNCE = 70   ; number of key-up scans before believing key is up

;-----------------------------------------------
;          Wait for Key Press
;-----------------------------------------------
; Wait for next key to be pressed.
;
;   out A = char
;
Wait_key:
    CALL key_check    ; check for key pressed
    JR   Z,Wait_Key   ; loop until key pressed
.key_click:
    push af
    ld   a,$FF        ; speaker ON
    out  ($fc),a
    ld   a,128
.click_wait:
    dec  a
    jr   nz,.click_wait
    out  ($fc),a      ; speaker OFF
    pop  af
    RET

    ; disk file selector
    include "filerequest.asm"

    ; PT3 music player
    include "pt3play.asm"


SongData:
