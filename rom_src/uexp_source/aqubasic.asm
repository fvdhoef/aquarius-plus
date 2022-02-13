;===============================================================================
;    AQUBASIC: Extended BASIC ROM for Mattel Aquarius With USB MicroExpander
;===============================================================================
; By Bruce Abbott                                            www.bhabbott.net.nz
;                                                        bruce.abbott@xtra.co.nz

VERSION  = 1
REVISION = 0

; code options
;softrom  equ 1    ; loaded from disk into upper 16k of 32k RAM
aqubug   equ 1    ; full featured debugger (else lite version without screen save etc.)
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

;-----------------------------------------------------------------------------
; aquarius hardware and system ROM
;-----------------------------------------------------------------------------

; constants:
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
;Name    location Decimal alt name          description
CURCOL   = $3800 ; 14336  TTYPOS   Current cursor column
CURRAM   = $3801 ; 14337  CHRPOS   Position in CHARACTER RAM of cursor
USRJMP   = $3803 ; 14339  USRGO    JMP instruction for USR.
USRADDR  = $3804 ; 14340  USRAL    address of USR() function
                 ; 14341  USRAH
UDFADDR  = $3806 ; 14342  HOKDSP   RST $30 vector, hooks into various system routines
                 ; 14343
LISTCNT  = $3808 ; 14344  ROWCOUNT Counter for lines listed (pause every 23 lines)
LASTFF   = $3809 ; 14345  PTOLD    Last protection code sent to port($FF)
LSTASCI  = $380a ; 14346  CHARQ    ASCII value of last key pressed.
KWADDR   = $380b ; 14347  SKEY     Address of keyword in the keyword table.
                 ; 14348
CURHOLD  = $380d ; 14349  BUFO     holds character under the cursor.
LASTKEY  = $380E ; 14350           SCAN CODE of last key pressed
SCANCNT  = $380f ; 14351           number of SCANS key has been down for
FDIV     = $3810 ; 14352           subroutine for division ???
                 ;  ...
RANDOM   = $381F ; 14367           used by random number generator
                 ; ...
LPTLST   = $3845 ; 14405           last printer operation status
PRNCOL   = $3846 ; 14406  LPTPOS   The current printer column (0-131).
CHANNEL  = $3847 ; 14407  PRTFLG   Channel: 0=screen, 1=printer.
LINLEN   = $3848 ; 14408           line length (initially set to 40 ???)
CLMLST   = $3849 ; 14409           position of last comma column
RUBSW    = $384A ; 14410           rubout switch
STKTOP   = $384B ; 14411           high address of stack. followed by string storage space
                 ; 14412
CURLIN   = $384D ; 14413           current BASIC line number (-1 in direct mode)
                 ; 14414
BASTART  = $384F ; 14415  TXTTAB   pointer to start of BASIC program
                 ; 14416
CASNAM   = $3851 ; 14417           tape filename (6 chars)
CASNM2   = $3857 ; 14423           tape read filename (6 chars)
CASFL2   = $385D ; 14429           tape flag
CASFL3   = $385E ; 14430           tape flag (break key check)
BUFMIN   = $385F ; 14431           buffer used by INPUT statement
LINBUF   = $3860 ; 14432  BUF      line input buffer (73 bytes).
                 ;  ...
BUFEND   = $38A9 ; 14505           end of line unput buffer
DIMFLG   = $38AA ; 14506           dimension flag 1 = array
VALTYP   = $38AB ; 14507           type: 0 = number, 1 = string
DORES    = $38AC ; 14508           flag for crunch
RAMTOP   = $38AD ; 14509  MEMSIZ   Address of top of physical RAM.
                 ; 14510
STRBUF   = $38AF ; 14511           18 bytes used by string functions
                 ;  ...
FRETOP   = $38C1 ; 14529           pointer to top of string space
                 ; 14530
SYSTEMP  = $38C3 ; 14531  TEMP     temp space used by FOR etc.
                 ;  ...
DATLIN   = $38c9 ; 14537           address of current DATA line
                 ; 14538
FORFLG   = $38CB ; 14439           flag FOR:, GETVAR: 0=variable, 1=array

TMPSTAT  = $38ce ; 14540           temp holder of next statement address
                 ;  ...
CONTLIN  = $38d2 ; 14546,7         Line number to CONTinue from.
CONTPOS  = $38d4 ; 14548,9         address of line to CONTinue from.
BASEND   = $38d6 ; 14550  VARTAB   variable table (end of BASIC program)
                 ; 14551
ARYTAB   = $38D8 ; 14552           start of array table
                 ; 14553
ARYEND   = $38DA ; 14554           end of array table
                 ; 14555
RESTORE  = $38DC ; 14556           Address of line last RESTORE'd
                 ; 14557
;          $38DE ; 14558           pointer and flag for arrays
                 ;  ...
FPREG    = $38E4 ; 14564  FPNUM    floating point number
                 ;  ...
FPSTR    = $38E9 ; 14569           floating point string
                 ;  ...
;          $38F9 ; 14585           used by keybord routine
                 ;  ...
PROGST   = $3900 ; 14592           NULL before start of BASIC program

; end of system variables = start of BASIC program in stock Aquarius
;          $3901 ; 14593

; buffer lengths
LINBUFLEN   = DIMFLG-LINBUF
STRBUFLEN   = FRETOP-STRBUF
SYSTEMPLEN  = DATLIN-SYSTEMP
TMPSTATLEN  = CONTLIN-TMPSTAT
FPREGLEN    = FPSTR-FPREG
FPSTRLEN    = $38F9-FPSTR

;----------------------------------------------------------------------------
;                          system routines
;----------------------------------------------------------------------------
;
; RST $08,xx CHKNXT    syntax error if char at (HL) is not eqaul to xx
; RST $10    GETNXT    get char at (HL)+, Carry set if '0' to '9'
; RST $18    PRNTCHR   print char in A
; RST $20    CMPHLDE   compare HL to DE. Z if equal, C if DE is greater
; RST $28    TSTSIGN   test sign of floating point number
; RST $30,xx CALLUDF   hooks into various places in the ROM (identified by xx)
; RST $38    CALLUSR   maskable interrupt handler
; RST $66       -      NMI entry point. No code in ROM for this, do NOT use it!

PRNCHR      = $1d94  ; print character in A
PRNCHR1     = $1d72  ; print character in A with pause/break at end of page
PRNCRLF     = $19ea  ; print CR+LF
PRINTSTR    = $0e9d  ; print null-terminated string
PRINTINT    = $1675  ; print 16 bit integer in HL

SCROLLUP    = $1dfe  ; scroll the screen up 1 line
SVCURCOL    = $1e3e  ; save cursor position (HL = address, A = column)

LINEDONE    = $19e5  ; line entered (CR pressed)
FINDLIN     = $049f  ; find address of BASIC line (DE = line number)

EVAL        = $0985  ; evaluate expression
OPRND       = $09fd  ; evaluate operand
EVLPAR      = $0a37  ; evaluate expression in brackets
GETINT      = $0b54  ; evaluate numeric expression (integer 0-255)
GETNUM      = $0972  ; evaluate numeric expression
PUTVAR      = $0b22  ; store variable 16 bit (out: B,A = value)
PUTVAR8     = $0b36  ; store variable 8 bit (out: B = value)

CRTST       = $0e5f  ; create string (HL = text ending with NULL)
QSTR        = $0e60  ; create string (HL = text starting with '"')
GETFLNM     = $1006  ; get tape filename string (out: DE = filename, A = 1st char)
GETVAR      = $10d1  ; get variable (out: BC = addr, DE = len)

GETLEN      = $0ff7  ; get string length (in: (FPREG) = string block)
                     ;                   (out: HL = string block, A = length)
TSTNUM      = $0975  ; error if evaluated expression not a number
TSTSTR      = $0976  ; error if evaluated expression not string
CHKTYP      = $0977  ; error if type mismatch

DEINT       = $0682  ; convert fp number to 16 bit signed integer in DE
STRTOVAL    = $069c  ; DE = value of decimal number string at HL-1 (65529 max)
STR2INT     = $069d  ; DE = value of decimal number string at HL
INT2STR     = $1679  ; convert 16 bit ingeter in HL to text at FPSTR (starts with ' ')

KEYWAIT     = $1a33  ; wait for keypress (out: A = key)
UKEYCHK     = $1e7e  ; get current key pressed (through UDF)
KEYCHK      = $1e80  ; get current key pressed (direct)
CLRKEYWT    = $19da  ; flush keyboard buffer and wait for keypress

CHKSTK      = $0ba0  ; check for stack space (in: C = number of words required)


;-----------------------------------------------------------------------------
;                         RST  macros
;-----------------------------------------------------------------------------
CHKNXT  MACRO char
        RST    $08    ; syntax error if char at (HL) is not equal to next byte
        db    'char'
        ENDM

;----------------------------------------------------------------------------
;                         BASIC Error Codes
;----------------------------------------------------------------------------
; code is offset to error name (2 characters)
;
;name        code            description
NF_ERR  =    $00             ; NEXT without FOR
SN_ERR  =    $02             ; Syntax error
RG_ERR  =    $04             ; RETURN without GOSUB
OD_ERR  =    $06             ; Out of DATA
FC_ERR  =    $08             ; Function Call error
OV_ERR  =    $0A             ; Overflow
OM_ERR  =    $0C             ; Out of memory
UL_ERR  =    $0E             ; Undefined line number
BS_ERR  =    $10             ; Bad subscript
DD_ERR  =    $12             ; Re-DIMensioned array
DZ_ERR  =    $14             ; Division by zero (/0)
ID_ERR  =    $16             ; Illegal direct
TM_ERR  =    $18             ; Type mismatch
OS_ERR  =    $1A             ; out of string space
LS_ERR  =    $1C             ; String too long
ST_ERR  =    $1E             ; String formula too complex
CN_ERR  =    $20             ; Can't CONTinue
UF_ERR  =    $22             ; UnDEFined FN function
MO_ERR  =    $24             ; Missing operand


;----------------------------------------------------------------------------
;     jump addresses for BASIC errors (returns to command prompt)
;----------------------------------------------------------------------------
ERROR_SN    = $03c4  ;   syntax error
                     ;   return without gosub
                     ;   out of data
ERROR_FC    = $0697  ;   function code error
ERROR_OV    = $03d3  ;   overflow
ERROR_OM    = $0bb7  ;   out of memory
ERROR_UL    = $06f3  ;   undefined line number
ERROR_BS    = $11cd  ;   bad subscript
ERROR_DD    = $03cd  ;   re-dimensioned array
ERROR_DIV0  = $03c7  ;   divide by zero
ERROR_ID    = $0b4F  ;   illegal direct
ERROR_TM    = $03d9  ;   type mismatch
ERROR_OS    = $0CEF  ;   out of string space
                     ;   string to long
ERROR_ST    = $0E97  ;   string formula too complex
ERROR_CN    = $0C51  ;   can't continue
ERROR_UF    = $03d0  ;   undefined function
                     ;   missing operand

; process error code, E = code (offset to 2 char error name)
DO_ERROR    = $03db






;-----------------------------------------------------------------------------
; structure macros
;-----------------------------------------------------------------------------

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
; SYS_BREAK         jp  break
; SYS_DEBUG         jp  break
; SYS_KEY_CHECK     jp  Key_Check
; SYS_WAIT_KEY      jp  Wait_key
; SYS_EDITLINE      jp  EditLine
; SYS_reserved1     jp  break
; SYS_reserved2     jp  break
; SYS_reserved3     jp  break


; ; USB driver vectors
; USB_OPEN_READ     jp  usb__open_read
; USB_READ_BYTE     jp  usb__read_byte
; USB_READ_BYTES    jp  usb__read_bytes
; USB_OPEN_WRITE    jp  usb__open_write
; USB_WRITE_BYTE    jp  usb__write_byte
; USB_WRITE_BYTES   jp  usb__write_bytes
; USB_CLOSE_FILE    jp  usb__close_file
; USB_DELETE_FILE   jp  usb__delete
; USB_FILE_EXIST    jp  usb__file_exist
; USB_SEEK_FILE     jp  usb__seek
; USB_WILDCARD      jp  usb__wildcard
; USB_DIR           jp  usb__dir
; USB_SORT          jp  usb__sort
; USB_SET_FILENAME  jp  usb__set_filename
; USB_MOUNT         jp  usb__mount
; USB_SET_USB_MODE  jp  usb__set_usb_mode
; USB_CHECK_EXISTS  jp  usb__check_exists
; USB_READY         jp  usb__ready
; USB_Wait_Int      jp  usb__wait_int
; USB_GET_PATH      jp  usb__get_path
; USB_ROOT          jp  usb__root
; USB_OPEN_PATH     jp  usb__open_path
; USB_OPEN_DIR      jp  usb__open_dir
; USB_reserved1     jp  Break
; USB_reserved2     jp  Break

; ; DOS vectors
; DOS_GETFILENAME   jp  dos__getfilename
; DOS_DIRECTORY     jp  dos__directory
; DOS_PRTDIRINFO    jp  dos__prtDirInfo
; DOS_GETFILETYPE   jp  dos__getfiletype
; DOS_NAME          jp  dos__name
; DOS_CHAR          jp  dos__char
; DOS_SET_PATH      jp  dos__set_path
; DOS_reserved1     jp  break
; DOS_reserved2     jp  break

; Break:
;        RET

vars = SysVars

; fill with $FF to $E000
     assert !($E000 < $) ; low rom full!!!
     dc  $E000-$,$FF

;=================================================================
;                     AquBASIC BOOT ROM
;=================================================================

     ORG $E000

; Rom recognition
; 16 bytes
RECOGNITION:
     db  66, 79, 79, 84
     db  83, 156, 84, 176
     db  82, 108, 65, 100
     db  80, 168, 128, 112
ROM_ENTRY:

     ; set flag for NTSC
     ld      a,0
     set     SF_NTSC,a
     ld      (Sysflags),a

     ld      a,$C3
     ld      (USRJMP),a
     ld      HL,0
     ld      (USRADDR),HL       ; set system RST $38 vector

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

; ; show splash screen (Boot menu)
; SPLASH:
;      call    usb__root          ; root directory
;      ld      a,CYAN
;      call    clearscreen
;      ld      b,40
;      ld      hl,$3000
; .topline:
;      ld      (hl),' '
;      set     2,h
;      ld      (hl),WHITE*16+BLACK ; black border, white on black chars in top line
;      res     2,h
;      inc     hl
;      djnz    .topline
;      ld      ix,BootbdrWindow
;      call    OpenWindow
;      ld      ix,bootwindow
;      call    OpenWindow
;      ld      hl,bootmenutext
;      call    WinPrtStr

; ; wait for Boot option key
; SPLKEY:
;      call    Key_Check
;      jr      z,SPLKEY           ; loop until key pressed
;      cp      $0d                ; RTN = cold boot
;      jp      z, COLDBOOT
;      cp      $03                ;  ^C = warm boot
;      jp      z, WARMBOOT
;      jr      SPLKEY

; ; CTRL-C pressed in boot menu
; WARMBOOT:
;      xor     a
;      ld      (RETYPBUF),a       ; clear history buffer
;      ld      a,$0b
;      rst     $18                ; clear screen
;      call    $0be5              ; clear workspace and prepare to enter BASIC
;      call    $1a40              ; enter BASIC at KEYBREAK
; JUMPSTART:
     jp      COLDBOOT           ; if BASIC returns then cold boot it

;
; Show copyright message
;
SHOWCOPYRIGHT:
     call    SHOWCOPY           ; Show system ROM copyright message
     ld      hl,STR_BASIC       ; "USB BASIC"
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
     db      $0D,"USB BASIC V", VERSION+'0', '.', REVISION+'0', $0D, $0A, 0

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
;                      USB Disk Driver
;---------------------------------------------------------------------
    include "ch376.asm"

; ; boot window with border
; BootBdrWindow:
;      db     (1<<WA_BORDER)|(1<<WA_TITLE)|(1<<WA_CENTER)
;      db     CYAN
;      db     CYAN
;      db     2,3,36,20
;      dw     bootWinTitle

; ; boot window text inside border
; BootWindow:
;      db     0
;      db     CYAN
;      db     CYAN
;      db     9,5,26,18
;      dw     0

; BootWinTitle:
;      db     " AQUARIUS USB BASIC V"
;      db     VERSION+'0','.',REVISION+'0',' ',0

; BootMenuText:
;      db     "    <RTN> BASIC",CR
;      db     CR
;      db     "<CTRL-C> Warm Start",0



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
     dw      ST_reserved    ; Previously EDIT
     dw      ST_CLS
     dw      ST_LOCATE
     dw      ST_OUT
     dw      ST_PSG
     dw      ST_reserved    ; Previously DEBUG
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


;-----------------------------------------------------------------------------
;                          Command Line
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; CLS statement
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; Clear Screen
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; OUT statement
; syntax: OUT port, data
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; LOCATE statement
; Syntax: LOCATE col, row
;-----------------------------------------------------------------------------
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


;-----------------------------------------------------------------------------
; PSG statement
; syntax: PSG register, value [, ... ]
;-----------------------------------------------------------------------------
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


;-----------------------------------------------------------------------------
; IN() function
; syntax: var = IN(port)
;-----------------------------------------------------------------------------
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


;-----------------------------------------------------------------------------
; JOY() function
; syntax: var = JOY(stick)
;    stick - 0 will read left or right
;          - 1 will read left joystick only
;          - 2 will read right joystick only
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; HEX$() function
; eg. A$=HEX$(B)
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; print hex byte
; in: A = byte
;-----------------------------------------------------------------------------
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


;-----------------------------------------------------------------------------
;                            CALL
;-----------------------------------------------------------------------------
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

;-----------------------------------------------------------------------------
; string functions
;-----------------------------------------------------------------------------

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
       RET

; fill with $FF to end of ROM
     assert !($FFFF<$)   ; ROM full!

     dc $FFFF-$+1,$FF

     end

