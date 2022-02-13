;===============================================================================
;    AQUBASIC: Extended BASIC ROM for Mattel Aquarius With USB MicroExpander
;===============================================================================
; By Bruce Abbott                                            www.bhabbott.net.nz
;                                                        bruce.abbott@xtra.co.nz
;
; For use with my micro-expander (CH376 USB interface, 32K RAM, AY-3-8910 PSG)
; Incudes commands from BLBasic by Martin Steenoven  http://www.vdSteenoven.com
; changes:-
; 2015-11-4  V0.0  created
; 2015-11-5        EDIT command provides enhanced editing of BASIC lines.
; 2015-11-6  V0.01 PRINT token expands to '?' on edit line (allows editing longer lines).
; 2015-11-9  V0.02 Added Turboload cassette routines
;                  Faster power-on memtest
; 2015-11-13 V0.03 Turbo tape commands TSAVE and TLOAD
;                  Enhanced edit functions on command line (immediate mode)
;                  Fixed scroll issue caused by conflicting ROWCOUNT definitions
; 2015-12-22 V0.04 fixed ctrl-c not removing colored cursor
; 2015-01-17 V0.05 CAT catalog files (minimalist directory listing)
; 2016-01-18 V0.06 RUN command can take name of file (BASIC or BINARY) to load and run
; 2016-01-30 V0.07 added PROGST to allow for extra system variables.
; 2016-02-21 V0.08 CALL passes txt pointer (in HL) to user code.
; 2016-04-11 V0.09 disk function vectors at $C000
; 2016-10-10 V0.10 changed cold boot menu to suit uexp (no ROM menu)
; 2017-01-22 V0.11 DOS commands take fully evaluated arguments eg. LOAD LEFT$(A$,11)
;                  DIR with wildcards
;                  CAT prints 3 filenames per line
; 2017-02-20 V0.12 DEBUG
; 2017-03-03 V0.13 change displayed name to AQUARIUS USB BASIC
; 2017-03-06 V0.14 Incorporate ROM initialization, boot menu, ROM loader.
; 2017-03-09 V0.15 add PT3 player, tidy vector and function names
; 2017-03-12 V0.16 HEX$() function: convert number to Hexadecimal string
; 2017-04-14 V0.17 HL = 0 on entry to debugger from splash screen
; 2017-04-18 V0.18 CRTL-R = retype line
; 2017-04-25 V0.19 Reserve high RAM for retype buffer, debugger, DOS
; 2017-04-29 V0.20 FileName, FileType, DOSflags moved from BASIC variables to private RAM
; 2017-05-04 V0.21 sys_KeyChk: replacement keyboard scan routine
; 2017-05-06 V0.22 clear history buffer before entering BASIC
;                  bugfix: ST_EDIT buffer length equate 1 less than actual buffer size
; 2017-05-08 V0.23 refactored code to get more space in ROM!
; 2017-05-16 V0.24 CLS clears 1000 chars/colors, doesn't touch last 24 bytes.
; 2017-05-22 V0.25 updated vectors
;                  moved wait_key from aqubug.asm to here
; 2017-05-30 V0.26 increased path.size from 36 to 37 (space for at least 4 subdirecties)
; 2017-06-11 V0.27 return to root dir after debug, pt3play etc. in boot menu
; 2017-06-12 V1.0  bumped to release version

VERSION  = 1
REVISION = 0

; code options
;softrom  equ 1    ; loaded from disk into upper 16k of 32k RAM
aqubug   equ 1    ; full featured debugger (else lite version without screen save etc.)
init_pcg equ 1    ; reset programmable character generator (if present)
;debug    equ 1    ; debugging our code. Undefine for release version!
;
; Commands:
; CLS    - Clear screen
; LOCATE - Position on screen
; SCR    - Scroll screen
; OUT    - output data to I/O port
; PSG    - Program PSG register, value
; CALL   - call machine code subroutine
; DEBUG  - call AquBUG Monitor/debugger

; EDIT   - Edit a BASIC line

; LOAD   - load file from USB disk
; SAVE   - save file to USB disk
; DIR    - display USB disk directory with wildcard
; CAT    - display USB disk directory
; CD     - change directory
; KILL   - delete file

; functions:
; IN()   - get data from I/O port
; JOY()  - Read joystick
; HEX$() - convert number to hexadecimal string

; Assembled with ZMAC in 'zmac' mode.
; command: ZMAC.EXE --zmac -n -I aqubasic.asm
;
; symbol scope:-
; .label   local to current function
; _label   local to current source file
; label    global to entire ROM and system (aquarius.i)
; function naming:-
; MODULE_FUNCTION    vector for use by external progams
; module__function   internal name for code in this ROM

    include  "aquarius.i" ; aquarius hardware and system ROM
    include  "macros.i"   ; structure macros
    include  "windows.i"  ; fast windowed text functions
    include  "pcg.i"      ; programmable character generator

; alternative system variable names
VARTAB      = BASEND     ; $38D6 variables table (at end of BASIC program)

  ifdef softrom
RAMEND = $8000           ; we are in RAM, 16k expansion RAM available
  else
RAMEND = $C000           ; we are in ROM, 32k expansion RAM available
  endif

path.size = 37           ; length of file path buffer

; high RAM usage
 STRUCTURE _sysvars,0
    STRUCT _retypbuf,74         ; BASIC command line history
    STRUCT _pathname,path.size  ; file path eg. "/root/subdir1/subdir2",0
    STRUCT _filename,13         ; USB file name 1-11 chars + '.', NULL
    BYTE   _filetype            ; file type BASIC/array/binary/etc.
    WORD   _binstart            ; binary file load/save address
    WORD   _binlen              ; binary file length
    BYTE   _dosflags            ; DOS flags
    BYTE   _sysflags            ; system flags
 ENDSTRUCT _sysvars

SysVars  = RAMEND-_sysvars.size
ReTypBuf = sysvars+_retypbuf
PathName = sysvars+_pathname
FileName = sysvars+_filename
FileType = sysvars+_filetype
BinStart = sysvars+_binstart
BinLen   = sysvars+_binlen
DosFlags = sysvars+_dosflags
SysFlags = sysvars+_sysflags

ifdef debug
  pathname = $3006  ; store path in top line of screen
endif

;system flags
SF_NTSC  = 0       ; 1 = NTSC, 0 = PAL
SF_RETYP = 1       ; 1 = CTRL-O is retype
SF_DEBUG = 7       ; 1 = Debugger available


;=======================================
;             ROM Code
;=======================================
;
; 16k ROM start address
     ORG $C000

;----------------------------------------------
;            External Vectors
;----------------------------------------------
;
; User programs should call ROM functions
; via these vectors only!
;
; system vectors
SYS_BREAK         jp  Break
SYS_DEBUG         jp  ST_DEBUG
SYS_KEY_CHECK     jp  Key_Check
SYS_WAIT_KEY      jp  Wait_key
SYS_EDITLINE      jp  EditLine
SYS_reserved1     jp  break
SYS_reserved2     jp  break
SYS_reserved3     jp  break


; USB driver vectors
USB_OPEN_READ     jp  usb__open_read
USB_READ_BYTE     jp  usb__read_byte
USB_READ_BYTES    jp  usb__read_bytes
USB_OPEN_WRITE    jp  usb__open_write
USB_WRITE_BYTE    jp  usb__write_byte
USB_WRITE_BYTES   jp  usb__write_bytes
USB_CLOSE_FILE    jp  usb__close_file
USB_DELETE_FILE   jp  usb__delete
USB_FILE_EXIST    jp  usb__file_exist
USB_SEEK_FILE     jp  usb__seek
USB_WILDCARD      jp  usb__wildcard
USB_DIR           jp  usb__dir
USB_SORT          jp  usb__sort
USB_SET_FILENAME  jp  usb__set_filename
USB_MOUNT         jp  usb__mount
USB_SET_USB_MODE  jp  usb__set_usb_mode
USB_CHECK_EXISTS  jp  usb__check_exists
USB_READY         jp  usb__ready
USB_Wait_Int      jp  usb__wait_int
USB_GET_PATH      jp  usb__get_path
USB_ROOT          jp  usb__root
USB_OPEN_PATH     jp  usb__open_path
USB_OPEN_DIR      jp  usb__open_dir
USB_reserved1     jp  Break
USB_reserved2     jp  Break

; DOS vectors
DOS_GETFILENAME   jp  dos__getfilename
DOS_DIRECTORY     jp  dos__directory
DOS_PRTDIRINFO    jp  dos__prtDirInfo
DOS_GETFILETYPE   jp  dos__getfiletype
DOS_NAME          jp  dos__name
DOS_CHAR          jp  dos__char
DOS_SET_PATH      jp  dos__set_path
DOS_reserved1     jp  break
DOS_reserved2     jp  break

; file requester
FRQ_FILEREQ       jp  RequestFile
FRQ_LISTFILES     jp  ListFiles
FRQ_SHOWLIST      jp  ShowList
FRQ_SELECT        jp  SelectFile
FRQ_reserved      jp  break

; windows
WIN_OPENWINDOW    jp  OpenWindow
WIN_SETCURSOR     jp  WinSetCursor
WIN_CLEARWINDOW   jp  ClearWindow
WIN_SHOWTITLE     jp  ShowTitle
WIN_DRAWBORDER    jp  DrawBorder
WIN_COLORWINDOW   jp  ColorWindow
WIN_PRTCHR        jp  WinPrtChr
WIN_PRTCHARS      jp  WinPrtChrs
WIN_PRTSTR        jp  WinPrtStr
WIN_PRTMSG        jp  WinPrtMsg
WIN_CURSORADDR    jp  CursorAddr
WIN_TEXTADDR      jp  WinTextAddr
WIN_SCROLLWINDOW  jp  ScrollWindow
WIN_NEWLINE       jp  NewLine
WIN_CLEARTOEND    jp  ClearToEnd
WIN_BACKSPACE     jp  BackSpace
WIN_WAITKEY       jp  Wait_Key
WIN_CAT_DISK      jp  WinCatDisk
WIN_INPUTLINE     jp  InputLine
WIN_reserved1     jp  break
WIN_reserved2     jp  break


; windowed text functions
   include "windows.asm"

; debugger
 ifdef aqubug
   include "aqubug.asm"
 else
   include "debug.asm"
 endif

; fill with $FF to $E000
     assert !($E000 < $) ; low rom full!!!
     dc  $E000-$,$FF

;=================================================================
;                     AquBASIC BOOT ROM
;=================================================================

     ORG $E000

; Rom recognization
; 16 bytes
;
RECOGNIZATION:
     db  66, 79, 79, 84
     db  83, 156, 84, 176
     db  82, 108, 65, 100
     db  80, 168, 128, 112

ROM_ENTRY:

; initialize Programmble Character Generator
  ifdef init_pcg
     ld      hl,PCG_UNLOCK
     ld      (hl),$ff      ; unlock PCG registers
     ld      hl,PCG_MODE   ; PCG_CHAR is first
     ld      b,3           ; 3 registers to clear
     xor     a
.reset_pcg:
     ld      (hl),a        ; clear registers PCG_MODE, DBANK, WBANK
     inc     hl
     djnz    .reset_pcg
     inc     hl            ; skip CHRSET
     ld      (hl),a
     dec     hl            ; back to WBANK
     ld      (hl),a        ; load character set #0
  endif

; set flag for NTSC or PAL
     call    PAL__NTSC     ; measure video frame period: nc = PAL, c = NTSC
     ld      a,0
     jr      nc,.set_sysflags
     set     SF_NTSC,a
.set_sysflags:
     ld      (Sysflags),a
;
; init debugger
     ld      hl,vars
     ld      bc,v.size
.clrbugmem:
     ld      (hl),0             ; clear all debugger variables
     inc     hl
     dec     bc
     ld      a,b
     or      c
     jr      nz,.clrbugmem
     ld      a,$C3
     ld      (USRJMP),a
     ld      HL,0
     ld      (USRADDR),HL       ; set system RST $38 vector
  ifdef aqubug
     ld      de,MemWindows
     ld      hl,dflt_winaddrs
     ld      bc,2*4             ; initialize default memory window addresses
     ldir
  endif
;
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
;
; show splash screen (Boot menu)
SPLASH:
     call    usb__root          ; root directory
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
     ld      ix,BootbdrWindow
     call    OpenWindow
     ld      ix,bootwindow
     call    OpenWindow
     ld      hl,bootmenutext
     call    WinPrtStr

; wait for Boot option key
SPLKEY:
     call    Key_Check
     jr      z,SPLKEY           ; loop until key pressed
  ifndef softrom
     cp      "1"                ; '1' = load ROM
     jr      z,LoadROM
  endif
     cp      "2"                ; '2' = debugger
     jr      z,DEBUG
     cp      "3"                ; '3' = PT3 player
     jr      z,PTPLAY
     cp      $0d                ; RTN = cold boot
     jp      z, COLDBOOT
     cp      $03                ;  ^C = warm boot
     jp      z, WARMBOOT
     jr      SPLKEY

DEBUG:
     call    InitBreak          ; set RST $38 vector to Trace Break
     ld      hl,0               ; HL = 0 (no BASIC text)
     call    ST_DEBUG           ; invoke Debugger
     JR      SPLASH

LoadROM:
     call    Load_ROM           ; ROM loader
     JR      SPLASH

PTPLAY:
     CALL    PT3_PLAY           ; Music player
     JR      SPLASH

; CTRL-C pressed in boot menu
WARMBOOT:
     xor     a
     ld      (RETYPBUF),a       ; clear history buffer
     ld      a,$0b
     rst     $18                ; clear screen
     call    $0be5              ; clear workspace and prepare to enter BASIC
     call    $1a40              ; enter BASIC at KEYBREAK
JUMPSTART:
     jp      COLDBOOT           ; if BASIC returns then cold boot it

;
; Show copyright message
;
SHOWCOPYRIGHT:
     call    SHOWCOPY           ; Show system ROM copyright message
     ld      hl,STR_BASIC       ; "USB BASIC"
     call    $0e9d              ; PRINTSTR
     ld      hl, STR_VERSION    ;
     call    $0e9d              ; PRINTSTR
     ret

;
; Show Copyright message in system ROM
;
SHOWCOPY:
     ld      hl,$0163           ; point to copyright string in ROM
     ld      a,(hl)
     cp      $79                ; is it the 'y' in "Copyright"?
     ret     nz                 ; no, quit
     dec     hl
     dec     hl                 ; yes, back up to start of string
     dec     hl
SHOWIT:
     dec     hl
     call    $0e9d              ; PRINTSTR, Print the string pointed to by HL
     ret

STR_BASIC:
     db      $0D,"USB BASIC"
     db      $00
STR_VERSION:
     db      " V",VERSION+'0','.',REVISION+'0',$0D,$0A,0

; The bytes from $0187 to $01d7 are copied to $3803 onwards as default data.
COLDBOOT:
     ld      hl,$0187           ; default values in system ROM
     ld      bc,$0051           ; 81 bytes to copy
     ld      de,$3803           ; system variables
     ldir                       ; copy default values
     xor     a
     ld      (BUFEND),a         ; NULL end of input buffer
     ld      (BINSTART),a       ; NULL binary file start address
     ld      (RETYPBUF),a       ; NULL history buffer
     ld      a,$0b
     rst     $18                ; clear screen
; Test the memory
; only testing 1st byte in each 256 byte page!
;
     ld      hl,$3A00           ; first page of free RAM
     ld      a,$55              ; pattern = 01010101
MEMTEST:
     ld      c,(hl)             ; save original RAM contents in C
     ld      (hl),a             ; write pattern
     cp      (hl)               ; compare read to write
     jr      nz,MEMREADY        ; if not equal then end of RAM
     cpl                        ; invert pattern
     ld      (hl),a             ; write inverted pattern
     cp      (hl)               ; compare read to write
     jr      nz,MEMREADY        ; if not equal then end of RAM
     ld      (hl),c             ; restore original RAM contents
     cpl                        ; uninvert pattern
     inc     h                  ; advance to next page
     jr      nz,MEMTEST         ; continue testing RAM until end of memory
MEMREADY:
     ld      a,h
  ifdef softrom
     cp      $80                ; 16k expansion
  else
     cp      $c0                ; 32k expansion
  endif
     jp      c,$0bb7            ; OM error if expansion RAM missing
     dec     hl                 ; last good RAM addresss
     ld      hl,vars-1          ; top of public RAM
MEMSIZE:
     ld      ($38ad),hl         ; MEMSIZ, Contains the highest RAM location
     ld      de,-50             ; subtract 50 for strings space
     add     hl,de
     ld      ($384b),hl         ; STKTOP, Top location to be used for stack
     ld      hl,PROGST
     ld      (hl), $00          ; NULL at start of BASIC program
     inc     hl
     ld      (BASTART), hl      ; beginning of BASIC program text
     call    $0bbe              ; ST_NEW2 - NEW without syntax check
     ld      hl,HOOK            ; RST $30 Vector (our UDF service routine)
     ld      (UDFADDR),hl       ; store in UDF vector
     call    SHOWCOPYRIGHT      ; Show our copyright message
     xor     a
     jp      $0402              ; Jump to OKMAIN (BASIC command line)


;---------------------------------------------------------------------
;                         ROM loader
;---------------------------------------------------------------------
     include "load_rom.asm"


;---------------------------------------------------------------------
;                      USB Disk Driver
;---------------------------------------------------------------------
    include "ch376.asm"


;-------------------------------------------------------------------
;                  Test for PAL or NTSC
;-------------------------------------------------------------------
; Measure video frame period, compare to 1.80ms
; NTSC = 16.7ms, PAL = 20ms
;
; out: nc = PAL, c = NTSC
;
; NOTE: waits for ~17-41ms. Do not use in timing-critical code!
;
PAL__NTSC:
    PUSH BC
.wait_vbl1:
    IN   A,($FD)
    RRA                   ; wait for start of vertical blank
    JR   C,.wait_vbl1
.wait_vbh1:
    IN   A,($FD)
    RRA                   ; wait for end of vertical blank
    JR   NC,.wait_vbh1
    LD   BC,0
.wait_vbl2:               ; 1.117us/cycle
    INC  BC               ; 2 count                 ]
    IN   A,($FD)          ; 3 read status reg       ]
    RRA                   ; 1 test VBL bit          ]   9 cycles per loop
    JR   C,.wait_vbl2     ; 3 loop until VLB high   ]   10.06us/loop
.wait_vbh2:               ; cycles (1.12us/cycle)
    INC  BC               ; 2 count                 ]
    IN   A,($FD)          ; 3 read status reg       ]   9 cycles per loop
    RRA                   ; 1 test VBL bit          ]   10.06us/loop
    JR   NC,.wait_vbh2    ; 3 loop until VLB high   ]
    LD   C,A
    LD   A,B              ; ~1657 = 60Hz, ~1989 = 50Hz
    CP   7                ; c = NTSC, nc = PAL
    LD   A,C
    POP  BC
    RET

; boot window with border
BootBdrWindow:
     db     (1<<WA_BORDER)|(1<<WA_TITLE)|(1<<WA_CENTER)
     db     CYAN
     db     CYAN
     db     2,3,36,20
     dw     bootWinTitle

; boot window text inside border
BootWindow:
     db     0
     db     CYAN
     db     CYAN
     db     9,5,26,18
     dw     0

BootWinTitle:
     db     " AQUARIUS USB BASIC V"
     db     VERSION+'0','.',REVISION+'0',' ',0

BootMenuText:
     db     CR
  ifdef softrom
     db     "    1. (disabled)",CR
  else
     db     "    1. Load ROM",CR
  endif
     db     CR,CR
     db     "    2. Debug",CR
     db     CR,CR
     db     "    3. PT3 Player",CR
     db     CR,CR,CR,CR
     db     "    <RTN> BASIC",CR
     db     CR
     db     "<CTRL-C> Warm Start",0



;------------------------------------------------------
;             UDF Hook Service Routine
;------------------------------------------------------
; This address is stored at $3806-7, and is called by
; every RST $30. It allows us to hook into the system
; ROM in several places (anywhere a RST $30 is located).
;
HOOK ex      (sp),hl            ; save HL and get address of byte after RST $30
     push    af                 ; save AF
     ld      a,(hl)             ; A = byte (RST $30 parameter)
     inc     hl                 ; skip over byte after RST $30
     push    hl                 ; push return address (code after RST $30,xx)
     ld      hl,UDFLIST         ; HL = RST 30 parameter table
     push    bc
     ld      bc,UDF_JMP-UDFLIST+1 ; number of UDF parameters
     cpir                       ; find paramater in list
     ld      a,c                ; A = parameter number in list
     pop     bc
     add     a,a                ; A * 2 to index WORD size vectors
     ld      hl,UDF_JMP         ; HL = Jump vector table
do_jump:
     add     a,l
     ld      l,a
     ld      a,$00
     adc     a,h
     ld      h,a                ; HL += vector number
     ld      a,(hl)
     inc     hl
     ld      h,(hl)             ; get vector address
     ld      l,a
     jp      (hl)               ; and jump to it
                                ; will return to HOOKEND

; End of hook
HOOKEND:
     pop     hl                 ; get return address
     pop     af                 ; restore AF
     ex      (sp),hl            ; restore HL and set return address
     ret                        ; return to code after RST $30,xx


; UDF parameter table
; List of RST $30,xx hooks that we are monitoring.
; NOTE: order is reverse of UDF jumps!
UDFLIST:    ; xx     index caller    @addr  performing function:-
     db      $18     ; 7   RUN       $06be  starting BASIC program
     db      $17     ; 6   NEXTSTMT  $064b  interpreting next BASIC statement
     db      $16     ; 5   PEXPAND   $0598  expanding a token
     db      $0a     ; 4   REPLCMD   $0536  converting keyword to token
     db      $1b     ; 3   FUNCTIONS $0a5f  executing a function
     db      $05     ; 2   LINKLINES $0485  updating nextline pointers in BASIC prog
     db      $02     ; 1   OKMAIN    $0402  BASIC command line (immediate mode)
; UDF parameter Jump table
UDF_JMP:
     dw      HOOKEND            ; 0 parameter not found in list
     dw      AQMAIN             ; 1 replacement immediate mode
     dw      LINKLINES          ; 2 update BASIC nextline pointers (returns to AQMAIN)
     dw      AQFUNCTION         ; 3 execute AquBASIC function
     dw      REPLCMD            ; 4 replace keyword with token
     dw      PEXPAND            ; 5 expand token to keyword
     dw      NEXTSTMT           ; 6 execute next BASIC statement
     dw      RUNPROG            ; 7 run program

; Our commands and functions
;
BTOKEN       equ $d4             ; our first token number
TBLCMDS:
     db      $80 + 'E', "DIT"
     db      $80 + 'C', "LS"
     db      $80 + 'L', "OCATE"
     db      $80 + 'O', "UT"
     db      $80 + 'P', "SG"
     db      $80 + 'D', "EBUG"
     db      $80 + 'C', "ALL"
     db      $80 + 'L', "OAD"
     db      $80 + 'S', "AVE"
     db      $80 + 'D', "IR"
     db      $80 + 'C', "AT"
     db      $80 + 'K', "ILL"
     db      $80 + 'C', "D"
; functions
     db      $80 + 'I', "N"
     db      $80 + 'J', "OY"
     db      $80 + 'H', "EX$"
     db      $80                ; End of table marker

TBLJMPS:
     dw      ST_EDIT
     dw      ST_CLS
     dw      ST_LOCATE
     dw      ST_OUT
     dw      ST_PSG
     dw      ST_DEBUG
     dw      ST_CALL
     dw      ST_LOAD
     dw      ST_SAVE
     dw      ST_DIR
     dw      ST_CAT
     dw      ST_KILL
     dw      ST_CD
TBLJEND:

BCOUNT equ (TBLJEND-TBLJMPS)/2    ; number of commands

TBLFNJP:
     dw      FN_IN
     dw      FN_JOY
     dw      FN_HEX
TBLFEND:

FCOUNT equ (TBLFEND-TBLFNJP)/2    ; number of functions

firstf equ BTOKEN+BCOUNT          ; token number of first function in table
lastf  equ firstf+FCOUNT-1        ; token number of last function in table


;--------------------------------------------------------------------
;                          Command Line
;--------------------------------------------------------------------
; Replacement Immediate Mode with Line editing
;
; UDF hook number $02 at OKMAIN ($0402)
;
AQMAIN:
    pop     af                  ; clean up stack
    pop     af                  ; restore AF
    pop     hl                  ; restore HL

    call    $19be               ; PRNHOME if we were printing to printer, LPRINT a CR and LF
    xor     a
    ld      (LISTCNT),a         ; Set ROWCOUNT to 0
    call    $19de               ; RSTCOL reset cursor to start of (next) line
    ld      hl,$036e            ; 'Ok'+CR+LF
    call    $0e9d               ; PRINTSTR
;
; Immediate Mode Main Loop
;l0414:
IMMEDIATE:
    ld      hl,SysFlags
    SET     SF_RETYP,(HL)       ; CRTL-R (RETYP) active
    ld      hl,-1
    ld      (CURLIN),hl         ; Current BASIC line number is -1 (immediate mode)
    ld      hl,LINBUF           ; HL = line input buffer
    ld      (hl),0              ; buffer empty
    ld      b,LINBUFLEN         ; 74 bytes including terminator
    call    EDITLINE            ; Input a line from keyboard.
    ld      hl,SysFlags
    RES     SF_RETYP,(HL)       ; CTRL-R inactive
ENTERLINE:
    ld      hl,LINBUF-1
    jr      c,immediate         ; If c then discard line
    rst     $10                 ; get next char (1st character in line buffer)
    inc     a
    dec     a                   ; set z flag if A = 0
    jr      z,immediate         ; If nothing on line then loop back to immediate mode
    push    hl
    ld      de,ReTypBuf
    ld      bc,LINBUFLEN        ; save line in history buffer
    ldir
    pop     hl
    jp      $0424               ; back to system ROM

; --- linking BASIC lines ---
; Redirected here so we can regain control of immediate mode
; Comes from $0485 via CALLUDF $05
LINKLINES:
    pop     af                 ; clean up stack
    pop     af                 ; restore AF
    pop     hl                 ; restore HL
    inc     hl
    ex      de,hl              ; DE = start of BASIC program
l0489:
    ld      h,d
    ld      l,e                ; HL = DE
    ld      a,(hl)
    inc     hl                 ; get address of next line
    or      (hl)
    jr      z,immediate        ; if next line = 0 then done so return to immediate mode
    inc     hl
    inc     hl                 ; skip line number
    inc     hl
    xor     a
l0495:
    cp      (hl)               ; search for next null byte (end of line)
    inc     hl
    jr      nz,l0495
    ex      de,hl              ; HL = current line, DE = next line
    ld      (hl),e
    inc     hl                 ; update address of next line
    ld      (hl),d
    jr      l0489              ; next line


;-------------------------------------
;        AquBASIC Function
;-------------------------------------
; called from $0a5f by RST $30,$1b
;
AQFUNCTION:
    pop     bc                  ; get return address
    pop     af
    pop     hl
    push    bc                  ; push return address back on stack
    cp      (firstf-$B2)        ; ($B2 = first system BASIC function token)
    ret     c                   ; return if function number below ours
    cp      (lastf-$B2+1)
    ret     nc                  ; return if function number above ours
    sub     (firstf-$B2)
    add     a,a                 ; index = A * 2
    push    hl
    ld      hl,TBLFNJP          ; function address table
    jp      do_jump             ; JP to our function



;-------------------------------------
;         Replace Command
;-------------------------------------
; Called from $0536 by RST $30,$0a
; Replaces keyword with token.
;
REPLCMD:
     ld      a,b                ; A = current index
     cp      $cb                ; if < $CB then keyword was found in BASIC table
     jp      nz,HOOKEND         ;    so return
     pop     bc                 ; get return address from stack
     pop     af                 ; restore AF
     pop     hl                 ; restore HL
     push    bc                 ; put return address back onto stack
     ex      de,hl              ; HL = Line buffer
     ld      de,TBLCMDS-1       ; DE = our keyword table
     ld      b,BTOKEN-1         ; B = our first token
     jp      $04f9              ; continue searching using our keyword table

;-------------------------------------
;             PEXPAND
;-------------------------------------
; Called from $0598 by RST $30,$16
; Expand token to keyword
;
PEXPAND:
    pop     de
    pop     af                  ; restore AF (token)
    pop     hl                  ; restore HL (BASIC text)
    cp      BTOKEN              ; is it one of our tokens?
    jr      nc,PEXPBAB          ; yes, expand it
    push    de
    ret                         ; no, return to system for expansion

PEXPBAB:
    sub     BTOKEN - 1
    ld      c,a                 ; C = offset to AquBASIC command
    ld      de,TBLCMDS          ; DE = table of AquBASIC command names
    jp      $05a8               ; Print keyword indexed by C


;-------------------------------------
;            NEXTSTMT
;-------------------------------------
; Called from $064b by RST 30
; with parameter $17
;
NEXTSTMT:
    pop     bc                  ; BC = return address
    pop     af                  ; AF = token, flags
    pop     hl                  ; HL = text
    jr      nc,BASTMT           ; if NC then process BASIC statement
    push    bc
    ret                         ; else return to system

BASTMT:
    sub     (BTOKEN)-$80
    jp      c,$03c4             ; SN error if < our 1st BASIC command token
    cp      BCOUNT              ; Count number of commands
    jp      nc,$03c4            ; SN error if > out last BASIC command token
    rlca                        ; A*2 indexing WORDs
    ld      c,a
    ld      b,$00               ; BC = index
    ex      de,hl
    ld      hl,TBLJMPS          ; HL = our command jump table
    jp      $0665               ; Continue with NEXTSTMT

; RUN
RUNPROG:
    pop     af                 ; clean up stack
    pop     af                 ; restore AF
    pop     hl                 ; restore HL
    jp      z,$0bcb            ; if no argument then RUN from 1st line
    push    hl
    call    EVAL               ; get argument type
    pop     hl
    ld      a,(VALTYP)
    dec     a                  ; 0 = string
    jr      z,_run_file
    call    $0bcf              ; else line number so init BASIC program and
    ld      bc,$062c
    jp      $06db              ;    GOTO line number
_run_file:
    call    dos__getfilename   ; convert filename, store in FileName
    push    hl                 ; save BASIC text pointer
    ld      hl,FileName
    call    usb__open_read     ; try to open file
    jr      z,.load_run
    cp      CH376_ERR_MISS_FILE ; error = file not found?
    jp      nz,.nofile         ; no, break
    ld      b,9                ; max 9 chars in name (including '.' or NULL)
.instr:
    ld      a,(hl)             ; get next name char
    inc     hl
    cp      '.'                ; if already has '.' then cannot extend
    jp      z,.nofile
    cp      ' '
    jr      z,.extend          ; until SPACE or NULL
    or      a
    jr      z,.extend
    djnz    .instr
.nofile:
    ld      hl,.nofile_msg
    call    PRINTSTR
    pop     hl                 ; restore BASIC text pointer
.error:
    ld      e,FC_ERR           ; function code error
    jp      DO_ERROR           ; return to BASIC
.extend:
    dec     hl
    push    hl                 ; save extn address
    ld      de,.bas_extn
    call    strcat             ; append ".BAS"
    ld      hl,FileName
    call    usb__open_read     ; try to open file
    pop     hl                 ; restore extn address
    jr      z,.load_run
    cp      CH376_ERR_MISS_FILE ; error = file not found?
    jp      nz,.nofile         ; no, break
    ld      de,.bin_extn
    ld      (hl),0             ; remove extn
    call    strcat             ; append ".BIN"
.load_run:
    pop     hl                 ; restore BASIC text pointer
    call    ST_LOADFILE        ; load file from disk, name in FileName
    jp      nz,.error          ; if load failed then return to command prompt
    cp      FT_BAS             ; filetype is BASIC?
    jp      z,$0bcb            ; yes, run loaded BASIC program
    cp      FT_BIN             ; BINARY?
    jp      nz,immediate       ; no, return to command line prompt
    ld      de,immediate
    push    de                 ; set return address
    ld      de,(BINSTART)
    push    de                 ; set jump address
    ret                        ; jump into binary

.bas_extn:
    db     ".BAS",0
.bin_extn:
    db     ".BIN",0

.nofile_msg:
    db     "file not found",$0D,$0A,0


;********************************************************************
;                   Command Entry Points
;********************************************************************

ST_reserved:
    ret


;--------------------------------------------------------------------
;   CLS statement
;
ST_CLS:
    ld      a,CYAN
    call    clearscreen
    ld      de,$3001+40   ; DE cursor at 0,0
    ld      (CURRAM),de
    xor     a
    ld      (CURCOL),a    ; column 0
    ld      a,' '
    ld      (CURHOLD),a   ; SPACE under cursor
    ret

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

;--------------------------------------
; get/put text area
; deprecated because they require extra
; variables, shifting the start of BASIC
; (possible compatitiblity issue)
;
;ST_GET:
;ST_PUT:
;    ret

;--------------------------------------------------------------------
;   OUT statement
;   syntax: OUT port, data
;
ST_OUT:
    call    GETNUM              ; get/evaluate port
    call    DEINT               ; convert number to 16 bit integer (result in DE)
    push    de                  ; stored to be used in BC
    rst     $08                 ; Compare RAM byte with following byte
    db      $2c                 ; character ',' byte used by RST 08
    call    GETINT              ; get/evaluate data
    pop     bc                  ; BC = port
    out     (c),a               ; out data to port
    ret

;--------------------------------------------------------------------
; LOCATE statement
; Syntax: LOCATE col, row
;
ST_LOCATE:
    call    GETINT              ; read number from command line (column). Stored in A and E
    push    af                  ; column store on stack for later use
    dec     a
    cp      38                  ; compare with 38 decimal (max cols on screen)
    jp      nc,$0697            ; If higher then 38 goto FC error
    rst     $08                 ; Compare RAM byte with following byte
    db      $2c                 ; character ',' byte used by RST 08
    call    GETINT              ; read number from command line (row). Stored in A and E
    cp      $18                 ; compare with 24 decimal (max rows on screen)
    jp      nc,$0697            ; if higher then 24 goto FC error
    inc     e
    pop     af                  ; restore column from store
    ld      d,a                 ; column in register D, row in register E
    ex      de,hl               ; switch DE with HL
    call    GOTO_HL             ; cursor to screenlocation HL (H=col, L=row)
    ex      de,hl
    ret

GOTO_HL:
    push    af
    push    hl
    exx
    ld      hl,($3801)          ; CHRPOS - address of cursor within matrix
    ld      a,($380d)           ; BUFO - storage of the character behind the cursor
    ld      (hl),a              ; return the original character on screen
    pop     hl
    ld      a,l
    add     a,a
    add     a,a
    add     a,l
    ex      de,hl
    ld      e,d
    ld      d,$00
    ld      h,d
    ld      l,a
    ld      a,e
    dec     a
    add     hl,hl
    add     hl,hl
    add     hl,hl               ; hl is now 40 * rows
    add     hl,de               ; added the columns
    ld      de,$3000            ; screen character-matrix (= 12288 dec)
    add     hl,de               ; putting it al together
    jp      $1de7               ; Save cursor position and return


;--------------------------------------------------------------------
;   PSG statement
;   syntax: PSG register, value [, ... ]
;
ST_PSG:
    cp      $00
    jp      z,$03d6          ; MO error if no args
psgloop:
    call    GETINT           ; get/evaluate register
    out     ($f7),a          ; set the PSG register
    rst     $08              ; next character must be ','
    db      $2c              ; ','
    call    GETINT           ; get/evaluate value
    out     ($f6),a          ; send data to the selected PSG register
    ld      a,(hl)           ; get next character on command line
    cp      $2c              ; compare with ','
    ret     nz               ; no comma = no more parameters -> return

    inc     hl               ; next character on command line
    jr      psgloop          ; parse next register & value


;--------------------------------------------------------------------
;   IN() function
;   syntax: var = IN(port)
;
FN_IN:
    pop     hl
    inc     hl
    call    EVLPAR           ; Read number from line - ending with a ')'
    ex      (sp),hl
    ld      de,$0a49         ; return address
    push    de               ; on stack
    call    DEINT            ; evalute formula pointed by HL, result in DE
    ld      b,d
    ld      c,e              ; bc = port
    in      a,(c)            ; a = in(port)
    jp      PUTVAR8          ; return with 8 bit input value in variable var


;--------------------------------------------------------------------
;   Entry point for JOY() function
;   syntax: var = JOY( stick )
;                 stick - 0 will read left or right
;                       - 1 will read left joystick only
;                       - 2 will read right joystick only
;
FN_JOY:
         pop     hl             ; Return address
         inc     hl             ; skip rst parameter
         call    $0a37          ; Read number from line - ending with a ')'
         ex      (sp),hl
         ld      de,$0a49       ; set return address
         push    de
         call    $0682          ; DEINT - evalute formula pointed by HL result in DE

         ld      a,e
         or      a
         jr      nz, joy01
         ld      a,$03

joy01:   ld      e,a
         ld      bc,$00f7
         ld      a,$ff
         bit     0,e
         jr      z, joy03
         ld      a,$0e
         out     (c),a
         dec     c
         ld      b,$ff

joy02:   in      a,(c)
         djnz    joy02
         cp      $ff
         jr      nz,joy05

joy03:   bit     1,e
         jr      z,joy05
         ld      bc,$00f7
         ld      a,$0f
         out     (c),a
         dec     c
         ld      b,$ff

joy04:   in      a,(c)
         djnz    joy04

joy05:   cpl
         jp      $0b36


;----------------------------------------
;  Convert number to HEX string
;----------------------------------------
;
; eg. A$=HEX$(B)
;
FN_HEX:
    pop  hl
    inc  hl
    call EVLPAR     ; evaluate parameter in brackets
    ex   (sp),hl
    ld   de,$0a49   ; return address
    push de         ; on stack
    call DEINT      ; evaluate formula @HL, result in DE
    ld   hl,$38e9   ; hl = temp string
    ld   a,d
    or   a          ; > zero ?
    jr   z,.lower_byte
    ld   a,d
    call .hexbyte   ; yes, convert byte in D to hex string
.lower_byte:
    ld   a,e
    call .hexbyte   ; convert byte in E to hex string
    ld   (hl),0     ; null-terminate string
    ld   hl,$38e9
.create_string:
    jp   $0e2f      ; create BASIC string

.hexbyte:
    ld   b,a
    rra
    rra
    rra
    rra
    call .hex
    ld   a,b
.hex:
    and  $0f
    cp   10
    jr   c,.chr
    add  7
.chr:
    add  '0'
    ld   (hl),a
    inc  hl
    ret


;--------------------------
;   print hex byte
;--------------------------
; in: A = byte
PRINTHEX:
        push    bc
        ld      b,a
        and     $f0
        rra
        rra
        rra
        rra
        cp      10
        jr      c,.hi_nib
        add     7
.hi_nib:
        add     '0'
        call    PRNCHR
        ld      a,b
        and     $0f
        cp      10
        jr      c,.low_nib
        add     7
.low_nib:
        add     '0'
        pop     bc
        jp      PRNCHR


;--------------------------------------------------------------------
;                            CALL
;--------------------------------------------------------------------
; syntax: CALL address
; address is signed integer, 0 to 32767   = $0000-$7FFF
;                            -32768 to -1 = $8000-$FFFF
;
; on entry to user code, HL = text after address
; on exit from user code, HL should point to end of statement
;
ST_CALL:
    call    GETNUM           ; get number from BASIC text
    call    DEINT            ; convert to 16 bit integer
    push    de
    ret                      ; jump to user code, HL = BASIC text pointer


;---------------------------------------------------------------------
;                       DOS commands
;---------------------------------------------------------------------
; ST_CD
; ST_LOAD
; ST_SAVE
; ST_DIR
; ST_CAT
; ST_KILL
     include "dos.asm"


;---------------------------------------------------------------------
;                     BASIC Line Editor
;---------------------------------------------------------------------
; EDIT (line number)
;
;ST_EDIT
     include "edit.asm"


;=====================================================================
;                  Miscellaneous functions

; string functions
   include "strings.asm"

; keyboard scan
   include "keycheck.asm"

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

; fill with $FF to end of ROM

     assert !($FFFF<$)   ; ROM full!

     dc $FFFF-$+1,$FF

     end

