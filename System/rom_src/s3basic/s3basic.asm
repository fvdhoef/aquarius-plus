; Mattel Aquarius Microsoft S3 Basic ROM
; Modded 2023 - Curtis F Kaylor 
; 
; Goals: Keep changes to machine code minimal for maximum compatability with 
; machine language programs. Internal machine language routines should be at
; the same addresses as in S2 BASIC.
;
; To assemble:
;   zmac -o s3basic.cim -o s3basic.lst s3basic.asm
;
;   Add -Dnoreskeys to change all Ctrl-Keys to generate ASCII characters 
;   instead of reserved words.
;
;   Add -Daddkeyrows to support extended 64 key keyboard.
;   (Requires new key decode tables in Extended ROM).
;
;   Add -Daqlus to include Aquarius+ Mods
;   This will also turn on noreskeys and addkeyrows
;
ifdef aqplus
noreskeys   equ   1
addkeyrows  equ   1
endif

ifdef fastbltu
    assert !(1) ;Fast BLTU code is broken. Do not use this switch.
endif

;BASIC Constants
LPTSIZ  equ     132     ;{M80} WIDTH OF PRINTER
SCREEN  equ     $3000   ;;Screen Character Matrix
COLOR   equ     $3400   ;;Screen Color Matrix
TTYPOS  equ     $3800   ;[M80] STORE TERMINAL POSITION HERE
CURRAM  equ     $3801   ;;Position in CHARACTER RAM of cursor
USRPOK  equ     $3803   ;;JP instruction for USR() routine
USRADD  equ     $3804   ;;Address of USR routine
HOOK    equ     $3806   ;;Extended ROM Hook Vector
CNTOFL  equ     $3808   ;;Line Counter. If not 0,
SCRMBL  equ     $3809   ;;Last Scramble byte output to port #FF
CHARC   equ     $380A   ;[M80] ISCNTC STORES EATEN CHAR HERE WHEN NOT A ^C
RESPTR  equ     $380B   ;;Pointer to Ctrl-Key Reserved Word
CURCHR  equ     $380D   ;;Character under Cursor
LSTX    equ     $380E   ;[M64] Matrix Coordinate of Last Key Pressed
KCOUNT  equ     $380F   ;;Keyboard debounce counter
FDIVC   equ     $3810   ;
FDIVB   equ     $3814   ;
FDIVA   equ     $3818   ;
FDIVG   equ     $381B   ;
RNDCNT  equ     $381F   ;
RNDTAB  equ     $3821   ;;Random Number TABLE
RNDX    equ     $3841   ;[M80] LAST RANDOM NUMBER GENERATED, BETWEEN 0 AND 1
CLMWID  equ     $3845   ;;Comma Column Width
LPTPOS  equ     $3846   ;[M80] POSITION OF LPT PRINT HEAD
PRTFLG  equ     $3847   ;[M80] WHETHER OUTPUT GOES TO LPT
LINLEN  equ     $3848   ;;Length of a Screen Line
CLMLST  equ     $3849   ;[M80] POSITION OF LAST COMMA COLUMN
RUBSW   equ     $384A   ;[M80] RUBOUT SWITCH =1 INSIDE THE PROCESSING OF A RUBOUT (INLIN)
TOPMEM  equ     $384B   ;[M80] TOP LOCATION TO USE FOR THE STACK INITIALLY SET UP BY INIT
                        ;[M80] ACCORDING TO MEMORY SIZE TO ALLOW FOR 50 BYTES OF STRING SPACE.
                        ;[M80] CHANGED BY A CLEAR COMMAND WITH AN ARGUMENT.
CURLIN  equ     $384D   ;[M80] CURRENT LINE #
TXTTAB  equ     $384F   ;[M80] POINTER TO BEGINNING OF TEXT
                        ;[M80] DOESN'T CHANGE AFTER BEING SETUP BY INIT.
FILNAM  equ     $3851   ;;File Name for CSAVE and CLOAD
FILNAF  equ     $3857   ;;File Name Read from Cassette
INSYNC  equ     $385D   ;;Contains $FF between read of SYNC and data from tape
CLFLAG  equ     $385E   ;;Flags whether doing CLOAD (0) or CLOAD? ($FF)
BUFMIN  equ     $385F   ;{M80} START OF BUFFER
BUF     equ     $3860   ;[M80] TYPE IN STORED HERE. DIRECT STATEMENTS EXECUTE OUT OF HERE.
                        ;[M80] REMEMBER "INPUT" SMASHES BUF. MUST BE AT A LOWER ADDRESS THAN DSCTMP
                        ;[M80] OR ASSIGNMENT OF STRING VALUES IN DIRECT STATEMENTS WON'T COPY INTO
                        ;[M80] STRING SPACE -- WHICH IT MUST ALLOW FOR SINGLE QUOTE IN BIG LINE
OLDSTK  equ     $3865   ;;Deprecated: Top Of Stack - Now Set by STKINI
TMPSTK  equ     $38A0   ;;Temporary Stack - Set by INIT
ENDBUF  equ     $38A9   ;[M80] PLACE TO STOP BIG LINES
DIMFLG  equ     $38AA   ;[M80] IN GETTING A POINTER TO A VARIABLE IT IS IMPORTANT TO REMEMBER
                        ;[M80] WHETHER IT IS BEING DONE FOR "DIM" OR NOT
VALTYP  equ     $38AB   ;[M80] THE TYPE INDICATOR 0=NUMERIC 1=STRING
DORES   equ     $38AC   ;[M80] WHETHER CAN OR CAN'T CRUNCH RES'D WORDS TURNED ON WHEN "DATA"
                        ;[M80] BEING SCANNED BY CRUNCH SO UNQUOTED STRINGS WON'T BE CRUNCHED.
MEMSIZ  equ     $38AD   ;[M80] HIGHEST LOCATION IN MEMORY
TEMPPT  equ     $38AF   ;[M80] POINTER AT FIRST FREE TEMP DESCRIPTOR. INITIALIZED TO POINT TO TEMPST
TEMPST  equ     $38B1   ;[M80] STORAGE FOR NUMTMP TEMP DESCRIPTORS
DSCTMP  equ     $38BD   ;[M80] STRING FUNCTIONS BUILD ANSWER DESCRIPTOR HERE
FRETOP  equ     $38C1   ;{M80} TOP OF FREE STRING SPACE
TEMP3   equ     $38C3   ;[M80] USED MOMENTARILY BY FRMEVL. USED IN EXTENDED BY FOUT AND
                        ;[M80] USER DEFINED FUNCTIONS ARRAY VARIABLE HANDLING TEMPORARY
TEMP8   equ     $38C5   ;[M80] USED TO STORE THE ADDRESS OF THE END OF STRING ARRAYS IN GARBAGE COLLECTION
ENDFOR  equ     $38C7   ;[M80] SAVED TEXT POINTER AT END OF "FOR" STATEMENT
DATLIN  equ     $38C9   ;[M80] DATA LINE # -- REMEMBER FOR ERRORS
SUBFLG  equ     $38CB   ;[M80] FLAG WHETHER SUBSCRIPTED VARIABLE ALLOWED "FOR" AND USER-DEFINED
                        ;[M80] FUNCTION POINTER FETCHING TURN THIS ON BEFORE CALLING PTRGET SO
                        ;[M80] ARRAYS WON'T BE DETECTED. STKINI AND PTRGET CLEAR IT.
USFLG   equ     $38CC   ;;Direct Mode Flag
FLGINP  equ     $38CD   ;[M80] FLAGS WHETHER WE ARE DOING "INPUT" OR A READ
SAVTXT  equ     $38CE   ;[M80] PLACE WHERE NEWSTT SAVES TEXT POINTER
TEMP2   equ     $38D0   ;[M80] FORMULA EVALUATOR TEMP MUST BE PRESERVED BY OPERATORS
                        ;[M80] USED IN EXTENDED BY FOUT AND USER-DEFINED FUNCTIONS
OLDLIN  equ     $38D2   ;[M80] OLD LINE NUMBER (SETUP BY ^C,"STOP" OR "END" IN A PROGRAM)
OLDTXT  equ     $38D4   ;[M80] OLD TEXT POINTER
VARTAB  equ     $38D6   ;[M80] POINTER TO START OF SIMPLE VARIABLE SPACE. UPDATED WHENEVER
ARYTAB  equ     $38D8   ;[M80] POINTER TO BEGINNING OF ARRAY TABLE. INCREMENTED BY 6 WHENEVER
                        ;[M80] A NEW SIMPLE VARIABLE IS FOUND, AND SET TO [VARTAB] BY CLEARC.
STREND  equ     $38DA   ;[M80] END OF STORAGE IN USE. INCREASED WHENEVER A NEW ARRAY
                        ;[M80] OR SIMPLE VARIABLE IS ENCOUNTERED SET TO [VARTAB] BY CLEARC.
DATPTR  equ     $38DC   ;[M80] POINTER TO DATA. INITIALIZED TO POINT AT THE ZERO IN FRONT OF [TXTTAB]
                        ;[M80] BY "RESTORE" WHICH IS CALLED BY CLEARC. UPDATED BY EXECUTION OF A "READ"
VARNAM  equ     $38DE   ;;Variable Name
VARPNT  equ     $38E0   ;;Pointer to Variable
FACLO   equ     $38E4   ;[M80] LOW ORDER OF MANTISSA
FACHO   equ     $38E6   ;[M80] HIGH ORDER OF MANTISSA
FAC     equ     $38E7   ;[M80] EXPONENT
FBUFFR  equ     $38E8   ;[M80] BUFFER FOR FOUT
RESHO   equ     $38F6   ;[M65] RESULT OF MULTIPLIER AND DIVIDER
RESMO   equ     $38F7   ;;RESMO and RESLO are loaded into and stored from HL
SAVSTK  equ     $38F9   ;[M80] NEWSTT SAVES STACK HERE BEFORE SO THAT ERROR REVERY CAN
;;        $38FB-$38FF   ;;??Unused
;;              $3900   ;;This is always 0
BASTXT  equ     $3901   ;;Start of Basic Program
ifdef aqplus
XPLUS   equ     $2000   ;;Extended BASIC Reset
XCOLD   equ     $2003   ;;Extended BASIC Cold Start`
XCART   equ     $2006   ;;Extended BASIC Start Cartridge
endif
EXTBAS  equ     $2000   ;;Start of Extended Basic
XSTART  equ     $2010   ;;Extended BASIC Startup Routine
XINIT   equ     $E010   ;;ROM Cartridge Initialization Entry Point
ifdef addkeyrows
;;;Code Change: Address of Expanded Key Tables, stored in the last 256 bytes of Extended BASIC.
KEYADR  equ     $2F00   ;;Key Tables for 64 key matrix
endif
        org     $0000   ;;Starting Address of Standard BASIC
;;RST 0 - Startup
START:
ifdef aqplus
  ;;; Code Change: Aquarius+ only
  ;;; Instead of checking for the Extended BASIC signature
  ;;; jump straight into Extended BASIC init                                  Original Code
        jp      XPLUS             ;;Start Extended BASIC                      0000  jp      JMPINI
                                  ;;                                          0001
                                  ;;                                          0002
else        
        jp      JMPINI            ;;Start Initialization
endif
        byte    $82,$06,$22       ;;Revision Date 1982-06-22                  Original Code
;;RST 1 - Syntax Check

        byte    11                ;;Revision Number?
        nop                       ;;Pad out the RST routine
SYNCHK: ld      a,(hl)
        ex      (sp),hl
        cp      (hl)              ;[M65] CHARACTERS EQUAL?
        inc     hl
        ex      (sp),hl
        jp      nz,SNERR          ;[M65] "SYNTAX ERROR"
;;RST 2 - Get Next Character
CHRGET: inc     hl                ;[M65] INCREMENT THE TEXT PNTR
        ld      a,(hl)            ;;Entry point to get current character
        cp      ':'               ;[M65] IS IT A ":"?
        ret     nc                ;[M65] IT IS .GE. ":"
        jp      CHRCON            ;;Continue in CHRGETR
;;RST 3 - Output Character
OUTCHR: jp      OUTDO             ;;Execute print character routine           
        byte    0,0,0,0,0         ;;Pad out the RST routine

;;RST 4 - Integer Compare
COMPAR: ld      a,h               ;;Compare [DE] to [HL]
        sub     d                 ;;Sets Z flag if equal
        ret     nz                ;;Sets Carry if [DE] > [HL]
        ld      a,l               ;;Destroys [A]
        sub     e                 ;
        ret                       ;
        byte    0,0               ;;Pad out the RST routine
;;RST 5 - Get sign of Floating Point Argument
FSIGN:  ld      a,(FAC)           ;
        or      a                 ;[M65] IF NUMBER IS ZERO, SO IS RESULT
        jp      nz,SIGNC          ;;Check sign of mantissa if not 0
        ret                       ;
;;RST 6 - Extended BASIC Hook Dispatch
;;
HOOKDO: ld      ix,(HOOK)         ;;Get hook routine address
        jp      (ix)              ;;and jump to it
        byte    0,0               ;;Pad out RST routine
;;RST 7 - Execute USR Routine
USRFN:  jp      USRPOK            ;;Execute USR() routine
;;Default Extended BASIC Hook Routine
NOHOOK: exx                       ;;Save BC, DE, and HL
        pop     hl                ;;Get return address off stack
        inc     hl                ;;Increment to skip byte after RST
        push    hl                ;;Put new return address on stack
        exx                       ;;Restore BC, DE, and HL
        ret                       ;;and Return
INIT:   ld      sp,TMPSTK         ;[M80] SET UP TEMP STACK
        ld      a,11              ;
        call    TTYOUT            ;;Print CLS to Clear Screen
        ld      hl,(CURRAM)       ;
        ld      (hl),' '          ;;Place Space on Screen
        ld      a,7               ;
        call    TTYOUT            ;;Print BEL to Sound Beep
        xor     a                 ;
        out     ($FF),a           ;;Output 0 to I/O Port 255;
        ld      hl,$2FFF          ;
        ld      (INSYNC),hl       ;
;;Check for Catridge ROM at $E010
CRTCHK: ld      de,XINIT+1        ;
        ld      hl,CRTSIG-1       ;
CRTCH1: dec     de                ;
        dec     de                ;
        inc     hl                ;
        ld      a,(de)            ;
        rrca                      ;
        rrca                      ;
        add     a,e               ;
        cp      (hl)              ;
        jr      z,CRTCH1          ;
        ld      a,(hl)            ;
        or      a                 ;
        jr      nz,RESET          ;;ROM not found, start Basic
CRTCHC: ex      de,hl             ;
        ld      b,$0C             ;
CRTCH2: add     a,(hl)            ;
        inc     hl                ;
        add     a,b               ;
        dec     b                 ;
        jr      nz,CRTCH2         ;
        xor     (hl)              ;
        out     ($FF),a           ;
        ld      (SCRMBL),a        ;
ifdef aqplus
        jp      XCART             ;;Run Extended Cart Initialization Routine
else        
        jp      XINIT             ;;Execute Cartridge Code
endif
CRTSIG: byte    "+7$$3,",0        ;;$A000 Cartridge Signature
;;Display Startup Screen
RESET:  ld      de,SCREEN+417     ;;Display "BASIC"
        ld      hl,BASICT         ;;at line 10, column 17
        ld      bc,STARTT-BASICT  ;
        ldir                      ;
        ld      de,SCREEN+528     ;;Display Start Message
        ld      hl,STARTT         ;at line 13, column 8
        ld      bc,STARTE-STARTT  ;
        ldir                      ;
;;Cycle Screen Colors
CYCLE:  ld      b,3               ;;Black on Yellow
        call    COLORS            ;
        ld      b,2               ;;Black on Green
        call    COLORS            ;
        ld      b,6               ;;Black on Light Cyan
        call    COLORS            ;
        jr      CYCLE             ;
;;Start Screen Text
BASICT: byte    "BASIC"
;;Start Screen Prompt
STARTT: byte    "Press RETURN key to start"
STARTE: byte    0                 ;;End of Start Screen Prompt
;;Set Screen Colors and Check for Keypress
COLORS: ld      hl,COLOR          ;;Store Accumulator in all bytes of Color
COLOR1: ld      (hl),b            ;;memory, addresses $3400 through $3FFF
        inc     hl                ;;
        ld      a,h               ;;NOTE: This wipes out $3B80 through $3FFF,
        cp      $38               ;;which are after the end of Color memory.
        jr      nz,COLOR1         ;
        ld      hl,$4000          ;;Loop 12,288 times
COLOR2: call    INCHRC            ;;Check for keypress
        cp      13                ;{M80} IS IT A CARRIAGE RETURN?
        jr      z,COLDST          ;;Cold Start
        cp      3                 ;;Is it CTRL-C?
        jr      z,WARMST          ;;Warm Start
        dec     hl                ;;Decrement Counter
        ld      a,h               ;
        or      l                 ;
        jr      nz,COLOR2         ;;If not 0, loop
        ret                       ;;Back to CYCLE
;;Basic Interpreter Warm Start
WARMST: ld      a,11              ;
        call    TTYCHR            ;;Clear Screen
        ld      a,(SCRMBL)        ;
        out     ($FF),a           ;;Reset I/O Port 255
        call    STKINI            ;;Initialize stack
        call    WRMCON            ;;Finish Up
COLDST: ld      hl,DEFALT         ;Set System Variable Default Values
        ld      bc,81             ;
        ld      de,USRPOK         ;;Copy 80 bytes starting at DEFALT
        ldir                      ;;to the first 80 bytes of System Variables
        xor     a                 ;
        ld      (ENDBUF),a        ;;Clear byte after end of BUF
        ld      (BASTXT-1),a      ;;Clear byte before start of basic program
ifdef aqplus
;;; Code Change: Execute Aquarius+ Extended BASIC Cold Start
;;; On the Aquarius+ From MEMTST to before INITFF is deprecated: 68 bytes     Original Code
        jp      XCOLD             ;;Do Extended BASIC Cold Start              010F  ld      hl,BASTXT+99
                                  ;;                                          0110  
                                  ;;                                          0111
else                              ;;
;;Test Memory to Find Top of RAM  ;;
        ld      hl,BASTXT+99      ;;Set RAM Test starting address
endif
MEMTST: inc     hl                ;;Bump pointer
        ld      c,(hl)            ;;Save contents of location
        ld      a,h               ;
        or      l                 ;;Wrapped around to $0000?
        jr      z,MEMCHK          ;;Yes, go on to check memory
        xor     c                 ;;Scramble bits into A
        ld      (hl),a            ;;and write to location
        ld      b,(hl)            ;;Read back into B
        cpl                       ;;Invert scrambled bits
        ld      (hl),a            ;;Write to location
        ld      a,(hl)            ;;read it back
        cpl                       ;;and revert back
        ld      (hl),c            ;;Write original byte to location
        cp      b                 ;;Did reads match writes?
        jr      z,MEMTST          ;;Yes, check next locationn
;;Check Memory Size               ;
MEMCHK: dec     hl                ;;Back up to last good address
        ld      de,BASTXT+299     ;
        rst     COMPAR            ;;Is there enough RAM?
        jp      c,OMERR           ;;No, Out of Memory error
;;Set Top of memory
        ld      de,$FFCE
        ld      (MEMSIZ),hl       ;;Set MEMSIZ to last RAM location
        add     hl,de
        ld      (TOPMEM),hl       ;;Set TOPMEM t0 MEMSIZ-50
        call    SCRTCH            ;;Perform NEW
        call    PRNTIT            ;;Print copyright message
        ld      sp,OLDSTK         ;;Top of of stack used to be here
        call    STKINI            ;;Set stack pointer to TOPMEM
;;Check for Extended BASIC
        ld      hl,EXTBAS+5       ;;End of signature in Extended BASIC
        ld      de,CRTSIG         ;;Text to check signature against
EXTCHK: ld      a,(de)            ;;Get byte from signature
        or      a                 ;;Did we reach a terminator?
        jp      z,XBASIC          ;;Yes, start Extended BASIC
        cp      (hl)              ;;Does it match byte in ROM?
        jr      nz,INITFF         ;;No, move on
        dec     hl                ;;Move backward in Extended BASIC
        inc     de                ;;and forward in test signature
        jr      EXTCHK            ;;Compare next character
;;Initialize I/O Port 255
INITFF: ld      a,r               ;;Read random number from Refresh Register
        rla                       ;;Rotate left
ifdef aqplus
        xor     a
else 
        add     a,c               ;;Copy in bit that was rotated out
endif
        out     ($FF),a           ;;Write it to I/O Port 255
        ld      (SCRMBL),a        ;;and save it for later
        jp      READY             ;;Enter direct mode
HEDING: byte    11                ;;Copyright Message
        byte    "Copyright ",5," "
        byte    "1982 by Microsoft Inc."
;;; Code Change: Replace "S2" with "S3"                                       
        byte    " S"              ;;                                          Original Code
        byte    "3"               ;;                                          0185  byte "2"
        byte    10,0
;;System Variable Default Values - 81 ($51) bytes
;;; Code Change: USR() defaults to execute code at argument address, instead of FCERR
DEFALT: jp      USRDO             ;;3803 USRPOK                               0187  jp      FCERR
                                  ;;                                          0188
                                  ;;                                          0189
        word    NOHOOK            ;;3806 HOOK
        byte    0                 ;;3808 CNTOFL
        byte    163               ;;3809 SCRMBL
        byte    0                 ;;380A CHARC
        word    0                 ;;380B RESPTR
        byte    ' '               ;;380D CURCHR
        byte    0                 ;;380E LSTX
        byte    0                 ;;380F KCOUNT
        sub     $00               ;;3810 FDIVC
        ld      l,a               ;;3812
        ld      a,h               ;;3813
        sbc     a,00              ;;3814 FDIVB
        ld      h,a               ;;3816
        ld      a,b               ;;3817
        sbc     a,00              ;;3818 FDIVA
        ld      b,a               ;;381A
        ld      a,0               ;;381B FDIVG
        ret                       ;;381D
        byte    0                 ;;381E
        byte    0                 ;;381F RNDCNT
        byte    0                 ;;3820
RNDTBL: byte    $35,$4A,$CA,$99   ;;3821 RNDTAB
        byte    $39,$1C,$76,$98   ;;3825
        byte    $22,$95,$B3,$98   ;;3829
        byte    $0A,$DD,$47,$98   ;;383D
        byte    $53,$D1,$99,$99   ;;3831
        byte    $0A,$1A,$9F,$98   ;;3835
        byte    $65,$BC,$CD,$98   ;;3839
        byte    $D6,$77,$3E,$98   ;;383D
        byte    $52,$C7,$4F,$80   ;;3841 RNDX
        byte    $0E               ;;3845 CLMWID
        byte    $00               ;;3846 LPTPOS
        byte    $00               ;;3847 PRTFLG
        byte    $28               ;;3848 LINLEN
        byte    $0E               ;;3849 CLMLST
        byte    $00               ;;384A RUBSW
        byte    $64,$39           ;;3964 TOPMEM
        byte    $FE,$FF           ;;384D CURLIN
        word    BASTXT            ;;3901 TXTTAB
;[M65] STATEMENT DISPATCH ADDRESSES
STMDSP: ;MARKS START OF STATEMENT LIST
        word    ENDS              ;;$0C21
        word    FOR               ;;$05BC
        word    NEXT              ;;$0D13
        word    DATA              ;;$071C
        word    INPUT             ;;$0893
        word    DIM               ;;$10CC
        word    READ              ;;$08BE
        word    LET               ;;$0731
        word    GOTO              ;;$06DC
        word    RUN               ;;$06BE
        word    IFS               ;;$079C
        word    RESTOR            ;;$0C05
        word    GOSUB             ;;$06CB
        word    RETURN            ;;$06F8
        word    REM               ;;$071E
        word    STOP              ;;$0C1F
        word    ONGOTO            ;;$0780
        word    LPRINT            ;;$07B5
        word    COPY              ;;$1B15
        word    DEF               ;;$0B3B
        word    POKE              ;;$0B6D
        word    PRINT             ;;$07BC
        word    CONT              ;;$0C4B
        word    LIST              ;;$056C
        word    LLIST             ;;$0567
        word    CLEAR             ;;$0CCD
        word    CLOAD             ;;$1C2C
        word    CSAVE             ;;$1C08
        word    PSET              ;;$1A4F
        word    PRESET            ;;$1A4C
        word    SOUND             ;;$1AD6
        word    SCRATH            ;;$0BBD NEW
;;Function Dispatch Table
FUNDSP: word    SGN               ;;$14F5
        word    INT               ;;$15B1
        word    ABS               ;;$1509
        word    USRPOK            ;;$3803 USR()
        word    FRE               ;;$10A8
        word    LPOS              ;;$0B2E
        word    POS               ;;$0B33
        word    SQR               ;;$1775
        word    RND               ;;$1866
        word    LOG               ;;$1385
        word    EXP               ;;$17CD
        word    COS               ;;$18D7
        word    SIN               ;;$18DD
        word    TAN               ;;$1970
        word    ATN               ;;$1985
        word    PEEK              ;;$0B63
        word    LEN               ;;$0FF3
        word    STR               ;;$0E29
        word    VAL               ;;$1084
        word    ASC               ;;$1002
        word    CHR               ;;$1013
        word    LEFT              ;;$1021
        word    RIGHT             ;;$1050
        word    MID               ;;$1059
;;Statement Tokens
RESLST: ;[M65] THE LIST OF RESERVED WORDS:
TK      =       $80               ;;Tokens start at 128
ENDTK   equ     TK                ;
        byte    'E'+$80,"ND"      ;;$80
TK      =            TK+1
FORTK   equ     TK                ;
        byte    'F'+$80,"OR"      ;;$81
TK      =            TK+1
        byte    'N'+$80,"EXT"     ;;$82
TK      =            TK+1
DATATK  equ     TK                ;
        byte    'D'+$80,"ATA"     ;;$83
TK      =            TK+1
INPUTK  equ     TK                ;
        byte    'I'+$80,"NPUT"    ;;$84
TK      =            TK+1
        byte    'D'+$80,"IM"      ;;$85
TK      =            TK+1
        byte    'R'+$80,"EAD"     ;;$86
TK      =            TK+1
        byte    'L'+$80,"ET"      ;;$87
TK      =            TK+1
GOTOTK  equ     TK
        byte    'G'+$80,"OTO"     ;;$88
TK      =            TK+1
        byte    'R'+$80,"UN"      ;;$89
TK      =            TK+1
        byte    'I'+$80,"F"       ;;$8A
TK      =            TK+1
        byte    'R'+$80,"ESTORE"  ;;$8B
TK      =            TK+1
GOSUTK  equ     TK                ;
        byte    'G'+$80,"OSUB"    ;;$8C
TK      =            TK+1
        byte    'R'+$80,"ETURN"   ;;$8D
                                  ;
TK      =            TK+1
REMTK   equ     TK                ;
        byte    'R'+$80,"EM"      ;;$8E
TK      =            TK+1
        byte    'S'+$80,"TOP"     ;;$8F
TK      =            TK+1
        byte    'O'+$80,"N"       ;;$90
TK      =            TK+1
        byte    'L'+$80,"PRINT"   ;;$91
                                  ;
TK      =            TK+1
        byte    'C'+$80,"OPY"     ;;$92
TK      =            TK+1
        byte    'D'+$80,"EF"      ;;$93
TK      =            TK+1
        byte    'P'+$80,"OKE"     ;;$94
TK      =            TK+1
PRINTK  equ     TK                ;
        byte    'P'+$80,"RINT"    ;;$95
TK      =            TK+1
        byte    'C'+$80,"ONT"     ;;$96
TK      =            TK+1
        byte    'L'+$80,"IST"     ;;$97
TK      =            TK+1
        byte    'L'+$80,"LIST"    ;;$98
TK      =            TK+1
        byte    'C'+$80,"LEAR"    ;;$99
TK      =            TK+1
        byte    'C'+$80,"LOAD"    ;;$9A
TK      =            TK+1
        byte    'C'+$80,"SAVE"    ;;$9B
TK      =            TK+1
        byte    'P'+$80,"SET"     ;;$9C
TK      =            TK+1
        byte    'P'+$80,"RESET"   ;;$9D
                                  ;
TK      =            TK+1
        byte    'S'+$80,"OUND"    ;;$9E
TK      =            TK+1
SCRATK  equ     TK                ;
        byte    'N'+$80,"EW"      ;;$9F
TK      =            TK+1
;[M65] END OF COMMAND LIST        ;
TABTK   equ     TK                ;
        byte    'T'+$80,"AB("     ;;$A0
TK      =            TK+1
TOTK    equ     TK                ;
        byte    'T'+$80,"O"       ;;$A1
TK      =            TK+1
FNTK    equ     TK                ;
        byte    'F'+$80,"N"       ;;$A2
TK      =            TK+1
SPCTK   equ     TK                ;
        byte    'S'+$80,"PC("     ;;$A3
TK      =            TK+1
INKETK  equ     TK                ;
        byte    'I'+$80,"NKEY$"   ;;$A4
TK      =            TK+1
THENTK  equ     TK                ;
        byte    'T'+$80,"HEN"     ;;$A5
TK      =            TK+1
NOTTK   equ     TK                ;
        byte    'N'+$80,"OT"      ;;$A6
TK      =            TK+1
STEPTK  equ     TK                ;
        byte    'S'+$80,"TEP"     ;;$A7
TK      =            TK+1
;;Operators
PLUSTK  equ     TK                ;
        byte    '+'+$80           ;;$A8 plus
TK      =            TK+1
MINUTK  equ     TK                ;
        byte    '-'+$80           ;;$A9 minus
TK      =            TK+1
MULTK   equ     TK                ;
        byte    '*'+$80           ;;$AA times
TK      =            TK+1
DIVTK   equ     TK                ;
        byte    '/'+$80           ;;$AB divide
TK      =            TK+1
EXPTK   equ     TK                ;
        byte    '^'+$80           ;;$AC
TK      =            TK+1
ANDTK   equ     TK
        byte    'A'+$80,"ND"      ;;$AD
TK      =            TK+1
ORTK    equ     TK
        byte    'O'+$80,"R"       ;;$AE
TK      =            TK+1
GREATK  equ     TK                ;
        byte    '>'+$80           ;;$AF greater than
TK      =            TK+1
EQUATK  equ     TK
        byte    '='+$80           ;;$B0 equals
TK      =            TK+1
LESSTK  equ     TK                ;
        byte    '<'+$80           ;;$B1 less than
TK      =            TK+1
;;Functions
ONEFUN  equ     TK                ;;*** This might go after SGN
        byte    'S'+$80,"GN"      ;;$B2
TK      =            TK+1
        byte    'I'+$80,"NT"      ;;$B3
TK      =            TK+1
        byte    'A'+$80,"BS"      ;;$B4
TK      =            TK+1
        byte    'U'+$80,"SR"      ;;$B5
TK      =            TK+1
        byte    'F'+$80,"RE"      ;;$B6
TK      =            TK+1
        byte    'L'+$80,"POS"     ;;$B7
TK      =            TK+1
        byte    'P'+$80,"OS"      ;;$B8
TK      =            TK+1
        byte    'S'+$80,"QR"      ;;$B9
TK      =            TK+1
        byte    'R'+$80,"ND"      ;;$BA
TK      =            TK+1
        byte    'L'+$80,"OG"      ;;$BB
TK      =            TK+1
        byte    'E'+$80,"XP"      ;;$BC
TK      =            TK+1
        byte    'C'+$80,"OS"      ;;$BD
TK      =            TK+1
        byte    'S'+$80,"IN"      ;;$BE
TK      =            TK+1
        byte    'T'+$80,"AN"      ;;$BF
TK      =            TK+1
        byte    'A'+$80,"TN"      ;;$C0
TK      =            TK+1
        byte    'P'+$80,"EEK"     ;;$C1
TK      =            TK+1
        byte    'L'+$80,"EN"      ;;$C2
TK      =            TK+1
        byte    'S'+$80,"TR$"     ;;$C3
TK      =            TK+1
        byte    'V'+$80,"AL"      ;;$C4
TK      =            TK+1
        byte    'A'+$80,"SC"      ;;$C5
TK      =            TK+1
CHRTK   equ     TK
        byte    'C'+$80,"HR$"     ;;$C6
TK      =            TK+1
        byte    'L'+$80,"EFT$"    ;;$C7
TK      =            TK+1
        byte    'R'+$80,"IGHT$"   ;;$C8
TK      =            TK+1
        byte    'M'+$80,"ID$"     ;;$C9
TK      =            TK+1
POINTK  equ     TK
        byte    'P'+$80,"OINT"    ;;$CA
        byte    $80               ;;End of List Marker
;OPERATOR TABLE CONTAINS PRECEDENCE FOLLOWED BY THE ROUTINE ADDRESS
OPTAB:  byte    121               ;ADD
        word    FADDT             ;
        byte    121               ;SUBTRACT
        word    FSUBT             ;
        byte    124               ;MULTIPLY
        word    FMULTT            ;
        byte    124               ;DIVIDE
        word    FDIVT             ;
        byte    127               ;POWER
        word    FPWRT             ;
        byte    80                ;AND
        word    ANDOP             ;
        byte    70                ;OR
        word    OROP              ;
;{M80} NEEDED FOR MESSAGES
ERR:    byte    " Error",7,0
INTXT:  byte    " in ",0
REDDY:  byte    "Ok",13,10,0      ;;FINZER and NULRT rely on REDDY-1 being a 0!
BRKTXT: byte    "Break",0
; [M65] ERROR MESSAGES
ERRTAB: ;;List OF Error Messages
ERRNF   equ     $-ERRTAB          ;;$00
        byte    "NF"              ;[M80] NEXT without FOR
ERRSN   equ     $-ERRTAB          ;;$02
        byte    "SN"              ;[M80] Syntax error
ERRRG   equ     $-ERRTAB          ;;$04
        byte    "RG"              ;[M80] RETURN without GOSUB
ERROD   equ     $-ERRTAB          ;;$06
        byte    "OD"              ;[M80] Out of DATA
ERRFC   equ     $-ERRTAB          ;;$08
        byte    "FC"              ;[M80] Illegal function call
ERROV   equ     $-ERRTAB          ;;$0A
        byte    "OV"              ;[M80] Overflow
ERROM   equ     $-ERRTAB          ;;$0C
        byte    "OM"              ;[M80] Out of memory
ERRUS   equ     $-ERRTAB          ;;$0E
        byte    "UL"              ;[M80] Undefined line number
ERRBS   equ     $-ERRTAB          ;;$10
        byte    "BS"              ;[M80] Subscript out of range
ERRDDS  equ     $-ERRTAB          ;;$12
        byte    "DD"              ;[M80] Duplicate Definition
ERRDV0  equ     $-ERRTAB          ;;$14
        byte    "/0"              ;[M80] Division by zero
ERRID   equ     $-ERRTAB          ;;$16
        byte    "ID"              ;[M80] Illegal direct
ERRTM   equ     $-ERRTAB          ;;$18
        byte    "TM"              ;[M80] Type mismatch
ERRSO   equ     $-ERRTAB          ;;$1A
        byte    "OS"              ;[M80] Out of string space
ERRLS   equ     $-ERRTAB          ;;$1C
        byte    "LS"              ;[M80] String too long
ERRST   equ     $-ERRTAB          ;;$1E
        byte    "ST"              ;[M80] String formula too complex
ERRCN   equ     $-ERRTAB          ;;$20
        byte    "CN"              ;[M80] Can't continue
ERRUF   equ     $-ERRTAB          ;;$22
        byte    "UF"              ;[M80] Undefined user function
ERRMO   equ     $-ERRTAB          ;;$24
        byte    "MO"              ;[M80] Missing operand
;[M80] FIND A "FOR" ENTRY ON THE STACK WITH THE VARIABLE POINTER PASSED IN [D,E]
FORSIZ  equ     13                ;;Size of a FOR entry on the stack
FNDFOR: ld      hl,4+0            ;[M80] IGNORING THE RETURN ADDRESS OF
        add     hl,sp             ;[M80] THIS SUBROUTINE, SET [H,L]=SP
LOOPER: ld      a,(hl)            ;[M80] SEE WHAT TYPE OF THING IS ON THE STACK
        inc     hl                ;
        cp      FORTK             ;[M80] IS THIS STACK ENTRY A "FOR"?
        ret     nz                ;[M80] NO SO OK
        ld      c,(hl)            ;[M80] DO EQUIVALENT OF PUSHM / XTHL
        inc     hl                ;
        ld      b,(hl)            ;
        inc     hl                ;
        push    hl                ;[M80] PUT H  ON
        ld      h,b               ;[M80] PUSH B / XTHL IS SLOWER
        ld      l,c               ;
        ld      a,d               ;[M80] FOR THE "NEXT" STATMENT WITHOUT AN ARGUMENT
        or      e                 ;[M80] WE MATCH ON ANYTHING
        ex      de,hl             ;[M80] MAKE SURE WE RETURN [D,E]
        jr      z,POPGOF          ;[M80] POINTING TO THE VARIABLE
        ex      de,hl             ;
        rst     COMPAR            ;
POPGOF: ld      bc,FORSIZ         ;[M80] TO WIPE OUT A "FOR" ENTRY
        pop     hl                ;[M80] IF VARIABLE IN THIS ENTRY MATCHES RETURN
        ret     z                 ;[M80] WITH [H,L] POINTING THE BOTTOM ;OF THE ENTRY
        add     hl,bc             ;[M80] NOW POINTING TO THE START OF THE NEXT ENTRY.
        jr      LOOPER            ;[M80] SEE IF ITS A "FOR" ENTRY AND IF THE VARIABLE MATCHES
DATSNE: ld      hl,(DATLIN)       ;[M80] GET DATA LINE
        ld      (CURLIN),hl       ;[M80] MAKE IT CURRENT LINE
SNERR:  ld      e,ERRSN           ;[M80] "SYNTAX ERROR"
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
DV0ERR: ld      e,ERRDV0          ;[M80] DIVISION BY ZERO
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
NFERR:  ld      e,ERRNF           ;[M80] "NEXT WITHOUT FOR" ERROR
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
DDERR:  ld      e,ERRDDS          ;[M80] "REDIMENSIONED VARIABLE"
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
UFERR:  ld      e,ERRUF           ;[M80] "UNDEFINED FUNCTION" ERROR
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
OVERR:  ld      e,ERROV           ;;      Overflow Error
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
MOERR:  ld      e,ERRMO           ;;     Missing Operand
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
TMERR:  ld      e,ERRTM           ;[M80] TYPE MISMATCH ERROR

;;; Code Change: Call HOOKDO before STKINI (instead of after), 
;;; allowing the Hook Routine to preserve STKSAV for ON ERROR GOTO            Original Code
ERROR:  rst     HOOKDO            ;;call Hook Service Routine                 03DB  call    STKINI
        byte    0                 ;;                                          03DC  
ERRINI: call    STKINI            ;;                                          03DD  
                                  ;;                                          03DE  rst     HOOKDO   
                                  ;;                                          03DF  byte    0        
ERRCRD: call    CRDONZ            ;;
        ld      hl,ERRTAB         ;;
        rst     HOOKDO            ;;call Hook Service Routine
        byte    1                 ;
        ld      d,a               ;;Add Error Offset
        add     hl,de             ;
        ld      a,'?'             ;[M65] PRINT A QUESTION MARK
        rst     OUTCHR            ;
ERRFIN: ld      a,(hl)            ;[M65] GET FIRST CHR OF ERR MSG.
        rst     OUTCHR            ;[M65] OUTPUT IT.
        rst     CHRGET            ;[M65] GET SECOND CHR.
        rst     OUTCHR            ;[M65] OUTPUT IT.
        ld      hl,ERR            ;;" Error"
ERRFN1: call    STROUT            ;[M80] PRINT MESSAGE
        ld      hl,(CURLIN)       ;[M80] RESTORE LINE NUMBER
        ld      a,h               ;[M80] SEE IF IN DIRECT MODE
        and     l                 ;
        inc     a                 ;[M80] ZERO SAYS DIRECT MODE
        call    nz,INPRT          ;[M80] PRINT LINE NUMBER IN [H,L]
        byte    $3E               ;[M80] SKIP THE NEXT BYTE WITH "MVI A,0"
;[M80] FOR "LIST" COMMAND STOPPING
STPRDY: pop     bc
;;Enter Immediate Mode
READY:  rst     HOOKDO            ;;Call hook routine
        byte    2                 ;
        call    FINLPT            ;[M80] PRINT ANY LEFT OVERS
        xor     a                 ;
        ld      (CNTOFL),a        ;[M80] FORCE OUTPUT
        call    CRDONZ            ;[M80] IF NOT ALREADY AT LEFT, SEND CRLF
        ld      hl,REDDY          ;[M80] "OK" CRLF CRLF
        call    STROUT            ;
                                  ;
MAIN:   ld      hl,$FFFF          ;
        ld      (CURLIN),hl       ;[M80] SETUP CURLIN FOR DIRECT MODE
        call    INLIN             ;[M80] GET A LINE FROM TTY
        jr      c,MAIN            ;[M80] IGNORE ^C S
        rst     CHRGET            ;[M80] GET THE FIRST
        inc     a                 ;[M80] SEE IF 0 SAVING THE CARRY FLAG
        dec     a                 ;
        jr      z,MAIN            ;[M80] IF SO, A BLANK LINE WAS INPUT
        push    af                ;[M80] SAVE STATUS INDICATOR FOR 1ST CHARACTER
        call    SCNLIN            ;[M80] READ IN A LINE #
;;Tokenize Entered Line
EDENT:  push    de                ;[M80] SAVE LINE #
        call    CRUNCH            ;[M80] CRUNCH THE LINE DOWN
        ld      b,a               ;[M65] RETAIN CHARACTER COUNT.
        pop     de                ;[M80] RESTORE LINE #
        pop     af                ;[M80] WAS THERE A LINE #?
        rst     HOOKDO            ;;Call Hook Dispatch Routine
        byte    3                 ;
        jp      nc,GONE           ;
        push    de                ;
        push    bc                ;[M80] SAVE LINE # AND CHARACTER COUNT
        xor     a                 ;
        ld      (USFLG),a         ;{M80} RESET THE FLAG
        rst     CHRGET            ;[M80] REMEMBER IF THIS LINE IS
        or      a                 ;[M80] SET THE ZERO FLAG ON ZERO
        push    af                ;[M80] BLANK SO WE DON'T INSERT IT
        call    FNDLIN            ;[M80] GET A POINTER TO THE LINE
        jr      c,LEXIST          ;[M80] LINE EXISTS, DELETE IT
        pop     af                ;[M80] GET FLAG SAYS WHETHER LINE BLANK
        push    af                ;[M80] SAVE BACK
        jp      z,USERR           ;[M80] SAVE BACK
        or      a                 ;[M80] TRYING TO DELETE NON-EXISTANT LINE, ERROR
LEXIST: push    bc                ;[M80] SAVE THE POINTER
        jr      nc,NODEL          ;
;[M80] DELETE THE LINE
        ex      de,hl             ;[M80] [D,E] NOW HAVE THE POINTER TO NEXT LINE
        ld      hl,(VARTAB)       ;[M80] COMPACTIFYING TO VARTAB
MLOOP:  ld      a,(de)            ;
        ld      (bc),a            ;[M80] SHOVING DOWN TO ELIMINATE A LINE
        inc     bc                ;
        inc     de                ;
        rst     COMPAR            ;
        jr      nz,MLOOP          ;[M80] DONE COMPACTIFYING?
        ld      h,b               ;
        ld      l,c               ;;HL = new end of program
        ld      (VARTAB),hl       ;[M65] SETUP [VARTAB]
NODEL:  pop     de                ;[M80] POP POINTER AT PLACE TO INSERT
        pop     af                ;[M80] SEE IF THIS LINE HAD ANYTHING ON IT
        jr      z,FINI            ;[M80] IF NOT DON'T INSERT
LEVFRE: ld      hl,(VARTAB)       ;[M80] CURRENT END
        ex      (sp),hl           ;[M80] [H,L]=CHARACTER COUNT. VARTAB ONTO STACK
        pop     bc                ;[M80] [B,C]=OLD VARTAB
        add     hl,bc             ;
        push    hl                ;[M80] SAVE NEW VARTAB
        call    BLTU              ;;Create space for new line
        pop     hl                ;[M80] POP OFF VARTAB
        ld      (VARTAB),hl       ;[M80] UPDATE VARTAB
        ex      de,hl             ;
        ld      (hl),h            ;[M80] FOOL CHEAD WITH NON-ZERO LINK
        pop     de                ;[M80] GET LINE # OFF STACK
        inc     hl                ;[M80] SO IT DOESN'T THINK THIS LINK
        inc     hl                ;[M80] IS THE END OF THE PROGRAM
        ld      (hl),e            ;
        inc     hl                ;[M80] PUT DOWN LINE #
        ld      (hl),d            ;
        inc     hl                ;
        ld      de,BUF            ;[M80] MOVE LINE FRM BUF TO PROGRAM AREA
MLOOPR: ld      a,(de)            ;[M80] NOW TRANSFERING LINE IN FROM BUF
        ld      (hl),a            ;
        inc     hl                ;
        inc     de                ;
        or      a                 ;;If not line terminator, keep going
        jr      nz,MLOOPR         ;
FINI:   rst     HOOKDO            ;
        byte    4                 ;
        call    RUNC              ;[M80] DO CLEAR & SET UP STACK
        rst     HOOKDO            ;
        byte    5                 ;
        inc     hl                ;;HL=TXTTAB
        ex      de,hl             ;;DE=TXTTAB
;;Fix Basic Line Links
CHEAD:  ld      h,d               ;[H,L]=[D,E]
        ld      l,e               ;
        ld      a,(hl)            ;[M80] SEE IF END OF CHAIN
        inc     hl                ;[M80] BUMP POINTER
        or      (hl)              ;[M80] 2ND BYTE
        jp      z,MAIN            ;
        inc     hl                ;[M80] FIX HL TO START OF TEXT
        inc     hl                ;
        inc     hl                ;
        xor     a                 ;SET CC'S
CZLOOP: cp      (hl)              ;;Skip to end of Basic line
        inc     hl                ;
        jr      nz,CZLOOP         ;
        ex      de,hl             ;SWITCH TEMP
        ld      (hl),e            ;DO FIRST BYTE OF FIXUP
        inc     hl                ;ADVANCE POINTER
        ld      (hl),d            ;2ND BYTE OF FIXUP
        jr      CHEAD             ;KEEP CHAINING TIL DONE
;;Find Basic Line
FNDLIN: ld      hl,(TXTTAB)       ;[M80] GET POINTER TO START OF TEXT
LOOP:   ld      b,h               ;[M80] IF EXITING BECAUSE OF END OF PROGRAM,
        ld      c,l               ;[M80] SET [B,C] TO POINT TO DOUBLE ZEROES.
        ld      a,(hl)            ;[M80] GET WORD POINTER TO
        inc     hl                ;[M80] BUMP POINTER
        or      (hl)              ;[M80] GET 2ND BYTE
        dec     hl                ;[M80] GO BACK
        ret     z                 ;[M80] IF ZERO THEN DONE
        inc     hl                ;[M80] SKIP PAST AND GET THE LINE #
        inc     hl                ;
        ld      a,(hl)            ;[M80] INTO [H,L] FOR COMPARISON WITH
        inc     hl                ;[M80] THE LINE # BEING SEARCHED FOR
        ld      h,(hl)            ;[M80] WHICH IS IN [D,E]
        ld      l,a               ;
        rst     COMPAR            ;[M80] SEE IF IT MATCHES OR IF WE'VE GONE TOO FAR
        ld      h,b               ;[M80] MAKE [H,L] POINT TO THE START OF THE
        ld      l,c               ;[M80] LINE BEYOND THIS ONE, BY PICKING
        ld      a,(hl)            ;[M80] UP THE LINK THAT [B,C] POINTS AT
        inc     hl                ;
        ld      h,(hl)            ;
        ld      l,a               ;
        ccf                       ;[M80] TURN CARRY OFF
        ret     z                 ;[M80] EQUAL RETURN
        ccf                       ;[M80] MAKE CARRY ZERO
        ret     nc                ;[M80] NO MATCH RETURN (GREATER)
        jr      LOOP              ;[M80] KEEP LOOPING
;;Convert Keyword to Token
CRUNCH: xor     a                 ;SAY EXPECTING FLOATING NUMBERS
        ld      (DORES),a         ;ALLOW CRUNCHING
        ld      c,5               ;LENGTH OF KRUNCH BUFFER
        ld      de,BUF            ;SETUP DESTINATION POINTER
KLOOP:  ld      a,(hl)            ;GET CHARACTER FROM BUF
        cp      ' '               ;SPACE?
        jp      z,STUFFH          ;JUST STUFF AWAY
        ld      b,a               ;SETUP B WITH A QUOTE IF IT IS A STRING
        cp      '"'               ;QUOTE SIGN?
        jp      z,STRNG           ;YES, GO TO SPECIAL STRING HANDLING
        or      a                 ;END OF LINE?
        jp      z,CRDONE          ;YES, DONE CRUNCHING
        ld      a,(DORES)         ;IN DATA STATEMENT AND NO CRUNCH?
        or      a
        ld      a,(hl)            ;GET THE CHARACTER AGAIN
        jp      nz,STUFFH         ;IF NO CRUNCHING JUST STORE THE CHARACTER
        cp      '?'               ;A QMARK?
        ld      a,PRINTK          ;
        jp      z,STUFFH          ;THEN USE A "PRINT" TOKEN
        ld      a,(hl)            ;
        cp      '0'               ;[M65] SKIP NUMERICS.
        jr      c,MUSTCR          ;
        cp      '<'               ;[M65] ":" AND ";" ARE ENTERED STRAIGHTAWAY.
        jp      c,STUFFH          ;
MUSTCR: push    de                ;[M65] SAVE BUFFER POINTER.
        ld      de,RESLST-1       ;[M65] LOAD RESLST POINTER.
        push    bc                ;[M65] SAVE TEXT POINTER FOR LATER USE.
        ld      bc,NOTGOS         ;[M80] PLACE TO RETURN IF NOT FUNNY GO
        push    bc                ;
        ld      b,$7F             ;
CRUNCX: ld      a,(hl)            ;[M80] GET CHAR FROM MEMORY
        cp      'a'               ;[M80] IS IT LOWER CASE RANGE
        jr      c,RESCON          ;[M80] LESS
        cp      '{'               ;[M80] GREATER
        jr      nc,RESCON         ;[M80] TEST
        and     $5F               ;[M80] MAKE UPPER CASE
        ld      (hl),a            ;;and put it back
RESCON: ld      c,(hl)            ;[M80 SAVE CHAR IN [C]
;;Find next Reserved Word that starts with this character
        ex      de,hl             ;;HL=RESLST Pointer, DE=Text Pointer
LOPPSI: inc     hl                ;[M80] BUMP RESLST POINTER
        or      (hl)              ;[M80] SET CC'S
        jp      p,LOPPSI          ;[M80] SEE IF REST OF CHARS MATCH
        inc     b                 ;
        ld      a,(hl)            ;[M80] GET BYTE FROM RESERVED WORD LIST
        and     $7F               ;[M80] GET RID OF HIGH BIT
        ret     z                 ;[M80] IF=0 THEN END OF THIS CHARS RESLT
        cp      c                 ;[M80] COMPARE TO CHAR FROM SOURCE LINE
        jr      nz,LOPPSI         ;[M80] IF NO MATCH, SEARCH FOR NEXT RESWRD
        ex      de,hl             ;;DE=RESLST Pointer, HL=Text Pointer
        push    hl                ;;Save Text Pointer
LOPSKP: inc     de                ;[M80] POINT AFTER TOKEN
        ld      a,(de)            ;[M80] GET A BYTE FROM RESWRD LIST
        or      a                 ;[M80] BUMP RESLST POINTER
        jp      m,NOTFNT          ;[M80] SET CC'S
        ld      c,a               ;[M80] NOT END OF RESWRD, KEEP SKIPPING
        ld      a,b               ;
        cp      GOTOTK            ;
        jr      nz,MAKUPL         ;
        rst     CHRGET            ;
        dec     hl                ;[M80] FIX TEXT POINTER
MAKUPL: inc     hl                ;
        ld      a,(hl)            ;
        cp      'a'               ;[M80] IS IT LOWER CASE RANGE
        jr      c,MAKUPS          ;[M80] LESS
        and     $5F               ;[M80] MAKE UPPER CASE
MAKUPS: cp      c                 ;[M80] COMPARE TO CHAR FROM SOURCE LINE
        jr      z,LOPSKP          ;[M80] IF NO MATCH, SEARCH FOR NEXT RESWRD
        pop     hl                ;
        jr      RESCON            ;
NOTFNT: ld      c,b               ;
        pop     af                ;
        ex      de,hl             ;
        ret                       ;
NOTGOS: rst     HOOKDO            ;
        byte    10                ;
        ex      de,hl             ;;HL=text pointer, DE=krunch pointer
        ld      a,c               ;;Get token
        pop     bc                ;
        pop     de                ;
;;Copy character to crunch buffer
STUFFH: inc     hl                ;[M80] ENTRY TO BUMP [H,L]
        ld      (de),a            ;[M80] SAVE BYTE IN KRUNCH BUFFER
        inc     de                ;[M80] BUMP POINTER
        inc     c                 ;;Increment buffer count
        sub     ':'               ;[M65] IS IT A ":"?"
        jr      z,COLIS           ;[M65] YES, ALLOW CRUNCHING AGAIN.
        cp      DATATK-':'        ;[M65] IS IT A DATATK?
        jr      nz,NODATT         ;[M65] NO, SEE IF IT IS REM TOKEN.
COLIS:  ld      (DORES),a         ;[M65] SETUP FLAG.
NODATT: sub     REMTK-':'         ;[M65] REM ONLY STOPS ON NULL.
        jp      nz,KLOOP          ;[M65] NO, CONTINUE CRUNCHING.
        ld      b,a               ;{M80} SAVE TERMINATOR IN [B]
STR1:   ld      a,(hl)            ;[M80] GET A CHAR
        or      a                 ;[M80] SET CONDITION CODES
        jr      z,CRDONE          ;[M80] IF END OF LINE THEN DONE
        cp      b                 ;[M80] COMPARE CHAR WITH THIS TERMINATOR
        jr      z,STUFFH          ;[M80] IF YES, DONE WITH STRING
STRNG:  inc     hl                ;[M80] INCREMENT TEXT POINTER
        ld      (de),a            ;[M80] SAVE CHAR IN KRUNCH BUFFER
        inc     c                 ;;Increment buffer count
        inc     de                ;[M65] INCREMENT BUFFER POINTER.
        jr      STR1              ;[M80] KEEP LOOPING
CRDONE: ld      hl,BUF-1          ;[M80] GET POINTER TO CHAR BEFORE BUF AS "GONE" DOES A CHRGET
        ld      (de),a            ;[M80] NEED THREE 0'S ON THE END
        inc     de                ;[M80] ONE FOR END-OF-LINE
        ld      (de),a            ;[M80] AND 2 FOR A ZERO LINK
        inc     de                ;[M80] SINCE IF THIS IS A DIRECT STATEMENT
        ld      (de),a            ;[M80] ITS END MUST LOOK LIKE THE END OF A PROGRAM
        ret                       ;[M80] END OF CRUNCHING
;;The LLIST and LIST commands
LLIST:  ld      a,1               ;[M80] PRTFLG=1 FOR REGULAR LIST
        ld      (PRTFLG),a        ;[M80] SAVE IN I/O FLAG (END OF LPT)
LIST:   ld      a,23              ;;Set line count to 23
        ld      (CNTOFL),a        ;
        call    SCNLIN            ;[M80] SCAN LINE RANGE
        ret     nz                ;
        pop     bc                ;[M80] GET RID OF NEWSTT RETURN ADDR
        call    FNDLIN            ;[M80] DONT EVEN LIST LINE #
        push    bc                ;[M80] SAVE POINTER TO 1ST LINE
LIST4:  pop     hl                ;[M80] GET POINTER TO LINE
        ld      c,(hl)            ;[M80] [B,C]=THE LINK POINTING TO THE NEXT
        inc     hl                ;
        ld      b,(hl)            ;
        inc     hl                ;
        ld      a,b               ;[M80] SEE IF END OF CHAIN
        or      c                 ;
        jp      z,READY           ;[M80] LAST LINE, STOP.
        call    ISCNTC            ;[M80] CHECK FOR CONTROL-C
        push    bc                ;[M80] SAVE LINK
        call    CRDO              ;
        ld      e,(hl)            ;[M80] [B,C]=THE LINK POINTING TO THE NEXT LINE
        inc     hl                ;
        ld      d,(hl)            ;
        inc     hl                ;
        push    hl                ;[M80] DON'T ALLOW ^C
        ex      de,hl             ;[M80] GET LINE # IN [H,L]
        call    LINPRT            ;[M80] PRINT AS INT WITHOUT LEADING SPACE
        ld      a,' '             ;
        pop     hl                ;
PLOOP:  rst     OUTCHR            ;[M80] PRINT A SPACE AFTER THE LINE #
LISPRT: ld      a,(hl)            ;;Detokenize and Print Line
        inc     hl                ;[M80] INCR POINTER
        or      a                 ;[M80] SET CC
        jr      z,LIST4           ;[M80] IF =0 THEN END OF LINE
        jp      p,PLOOP           ;
        rst     HOOKDO            ;;Handle Extended BASIC Tokens
        byte    22                ;
        sub     $7F               ;
        ld      c,a               ;
        ld      de,RESLST         ;[M80] GET PTR TO START OF RESERVED WORD LIST
RESSRC: ld      a,(de)            ;[M80] GET CHAR FROM RESLST
        inc     de                ;[M80] BUMP SOURCE PTR
        or      a                 ;[M80] SET CC'S
        jp      p,RESSRC          ;[M80] IF NOT END OF THIS RESWRD, THEN KEEP LOOKING
        dec     c                 ;
        jr      nz,RESSRC         ;
MORLNP: and     $7F               ;[M80] AND OFF HIGH ORDER BIT
        rst     OUTCHR            ;[M80] STORE THIS CHAR
        ld      a,(de)            ;[M80] GET BYTE FROM RESWRD
        inc     de                ;[M80] BUMP POINTER
        or      a                 ;[M80] SET CC'S
        jp      p,MORLNP          ;[M80] END OF RESWRD?
        jr      LISPRT            ;[M80] PRINT NEXT CHAR
;[M80] "FOR" STATEMENT
FOR:    ld      a,100
        ld      (SUBFLG),a        ;[M80] DONT RECOGNIZE SUBSCRIPTED VARIABLES
        call    LET               ;[M65] READ VARIABLE AND ASSIGN INITIAL VALUE
        pop     bc                ;
        push    hl                ;[M80] SAVE THE TEXT POINTER
        call    DATA              ;[M80] SET [H,L]=END OF STATEMENT
        ld      (ENDFOR),hl       ;[M80] SAVE FOR COMPARISON
        ld      hl,2              ;[M80] SET UP POINTER INTO STACK
        add     hl,sp             ;
LPFORM: call    LOOPER            ;[M80] MUST HAVE VARIABLE POINTER IN [D,E]
        jr      nz,NOTOL          ;[M80] NO MATCHING ENTRY, DON'T ELIMINATE ANYTHING
        add     hl,bc             ;[M80] ELIMINATE THE MATCHING ENTRY
        push    de                ;[M80] SAVE THE TEXT POINTER
        dec     hl                ;[M80] SEE IF END TEXT POINTER OF MATCHING ENTRY
        ld      d,(hl)            ;[M80] MATCHES THE FOR WE ARE HANDLING
        dec     hl                ;[M80] PICK UP THE END OF THE "FOR" TEXT POINTER
        ld      e,(hl)            ;[M80] FOR THE ENTRY ON THE STACK
        inc     hl                ;[M80] WITHOUT CHANGING [H,L]
        inc     hl                ;
        push    hl                ;[M80] SAVE THE STACK POINTER FOR THE COMPARISON
        ld      hl,(ENDFOR)       ;[M80] GET ENDING TEXT POINTER FOR THIS "FOR"
        rst     COMPAR            ;[M80] SEE IF THEY MATCH
        pop     hl                ;[M80] GET BACK THE STACK POINTER
        pop     de                ;
        jr      nz,LPFORM         ;;[M80] KEEP SEARCHING IF NO MATCH
        pop     de                ;[M80] GET BACK THE TEXT POINTER
        ld      sp,hl             ;[M80] DO THE ELIMINATION
        inc     c                 ;
NOTOL:  pop     de                ;
        ex      de,hl             ;[M80] [H,L]=TEXT POINTER
        ld      c,8               ;[M80] MAKE SURE 16 BYTES ARE AVAILABLE
        call    GETSTK            ;[M80] OFF OF THE STACK
        push    hl                ;[M80] REALLY SAVE THE TEXT POINTER
        ld      hl,(ENDFOR)       ;[M80] PICK UP POINTER AT END OF "FOR"
        ex      (sp),hl           ;[M80] PUT POINTER ON STACK AND RESTORE TEXT POINTER
        push    hl                ;[M80] PUSH POINTER TO VARIABLE ONTO THE STACK
        ld      hl,(CURLIN)       ;[M80] [H,L] GET THE CURRENT LINE #
        ex      (sp),hl           ;[M80] LINE # ON THE STACK AND [H,L] IS THE TEXT POINTER
        call    CHKNUM            ;
        rst     SYNCHK            ;
        byte    TOTK              ;[M80] "TO" IS NECESSARY
        call    FRMNUM            ;[M65] VALUE MUST BE A NUMBER
        push    hl                ;
        call    MOVRF             ;[M80] GET THE STUFF
        pop     hl                ;[M80] REGAIN TEXT POINTER
        push    bc                ;[M80] OPPOSITE OF PUSHR
        push    de                ;[M80] SAVE THE SIGN OF THE INCREMENT
        ld      bc,$8100          ;[M80] DEFAULT THE STEP TO BE 1
        ld      d,c               ;
        ld      e,d               ;[M80] GET 1.0 IN THE REGISTERS
        ld      a,(hl)            ;[M80] GET TERMINATING CHARACTER
        cp      STEPTK            ;[M80] DO WE HAVE "STEP" ?
        ld      a,1               ;[M80] SETUP DEFAULT SIGN
        jr      nz,ONEON          ;[M65] NO. ASSUME 1.0.
        rst     CHRGET            ;[M65] YES. ADVANCE POINTER
        call    FRMNUM            ;
        push    hl                ;
        call    MOVRF             ;[M80] SET UP THE REGISTERS
        rst     FSIGN             ;[M80] GET THE SIGN OF THE INCREMENT
        pop     hl                ;[M80] POP OFF THE TEXT POINTER
ONEON:  push    bc                ;[M80] PUT VALUE ON BACKWARDS
        push    de                ;[M80] OPPOSITE OF PUSHR
        push    af                ;
        inc     sp                ;
        push    hl                ;
        ld      hl,(SAVTXT)       ;
        ex      (sp),hl           ;
NXTCON: ld      b,FORTK           ;[M80] PUT A 'FOR' TOKEN ONTO THE STACK
        push    bc                ;
        inc     sp                ;[M80] THE "TOKEN" ONLY TAKES ONE BYTE OF STACK SPACE
;[M80] NEW STATEMENT FETCHER
NEWSTT:
        ld      (SAVTXT),hl       ;USED BY CONTINUE AND INPUT AND CLEAR AND PRINT USING
        call    INCNTC            ;;*** might be [M65] ISCNTC
        ld      a,(hl)            ;;Get Terminator
        cp      ':'               ;[M80] IS IT A COLON?
        jr      z,GONE            ;
        or      a                 ;
        jp      nz,SNERR          ;[M80] MUST BE A ZERO
        inc     hl                ;
GONE4:  ld      a,(hl)            ;[M80] IF POINTER IS ZERO, END OF PROGRAM
        inc     hl                ;
        or      (hl)              ;[M80] OR IN HIGH PART
        jp      z,ENDCON          ;[M80] FIX SYNTAX ERROR IN UNENDED ERROR ROUTINE
        inc     hl                ;
        ld      e,(hl)            ;
        inc     hl                ;
        ld      d,(hl)            ;[M80] GET LINE # IN [D,E]
        ex      de,hl             ;[M80] [H,L]=LINE #
        ld      (CURLIN),hl       ;[M80] SETUP CURLIN WITH THE CURRENT LINE #
        ex      de,hl             ;;DE=Line#, HL=Text Pointer

GONE:   rst     CHRGET            ;[M80] GET THE STATEMENT TYPE
        ld      de,NEWSTT         ;[M80] PUSH ON A RETURN ADDRESS OF NEWSTT
        push    de                ;[M80] STATEMENT
GONE3:  ret     z                 ;[M80] IF A TERMINATOR TRY AGAIN
;[M80] "IF" COMES HERE
GONE2:  sub     $80               ;[M80] "ON ... GOTO" AND "ON ... GOSUB" COME HERE
        jp      c,LET             ;[M80] MUST BE A LET
        cp      TABTK-$80         ;;End of Statement Tokens
        rst     HOOKDO            ;;Handle Extended BASIC Statement Tokens
        byte    23                ;
        jp      nc,SNERR          ;;Not a Statement Token
        rlca                      ;[M80] MULTIPLY BY 2
        ld      c,a               ;
        ld      b,0               ;;Offset = (Token - 128) * 2
        ex      de,hl             ;
        ld      hl,STMDSP         ;[M80] STATEMENT DISPATCH TABLE
;;Add Offset to Table and Dispatch
GONE5:  add     hl,bc             ;[M80] ADD ON OFFSET
        ld      c,(hl)            ;[M80] PUSH THE ADDRESS TO GO TO ONTO
        inc     hl                ;[M80] THE STACK
        ld      b,(hl)            ;[M80] PUSHM SAVES BYTES BUT NOT SPEED
        push    bc                ;
        ex      de,hl             ;[M80] RESTORE THE TEXT POINTER
;;Execute Statement
;;Skip Statement Token and Execute
CHRGTR: inc     hl                ;[M80] DUPLICATION OF CHRGET RST FOR SPEED
CHRGT2: ld      a,(hl)            ;Alternate CHRGOT
        cp      ':'               ;[M80] SEE CHRGET RST FOR EXPLANATION
        ret     nc                ;
;[M80] CHRCON IS THE CONTINUATION OF THE CHRGET RST
CHRCON: cp      ' '               ;[M80] MUST SKIP SPACES
        jr      z,CHRGTR          ;[M80] GET ANOTHER CHARACTER
        cp      '0'               ;[M80] ALL CHARS .GT. "9" HAVE RET'D SO
        ccf                       ;[M80] TURN CARRY ON IF NUMERIC.
        inc     a                 ;[M80] ALSO, SETZ IF NULL.
        dec     a                 ;
        ret                       ;[M80] RETURN TO CALLER.
;[M80] INTIDX READS A FORMULA FROM THE CURRENT POSITION AND
INTIDX: rst     CHRGET            ;
INTID2: call    FRMNUM            ;{M80} READ FORMULA AND GET RESULT AS INTEGER IN [D,E]
INTFR2: rst     FSIGN             ;[M80] DON'T ALLOW NEGATIVE NUMBERS
        jp      m,FCERR           ;[M80] TOO BIG. FUNCTION CALL ERROR
;;Convert FAC to Integer and Return in [D,E]
;;; Code Change: FAC Exponent <= 145 instead of 144 to allow numbers 
;;; in the range -65535 to 65535 istead of -32768 to 32767.
FRCINT: ld      a,(FAC)           ;
        cp      145               ;[M65] FAC .GT. 32767?          
        jp      c,QINT            ;[M65] GO TO QINT AND SHOVE IT  Original Code
        jr      FCERR             ;;                              068A    ld      bc,$9080
;;; Code Change: POKE 14405 to change Comma Width    11 bytes     068B
COMWID: cp      ','               ;;If Not a Comma                068C
                                  ;;                              068D    ld      de,$0000
        ret     nz                ;;  Return                      068E  
        ld      a,(CLMWID)        ;;Get Column Width              068F    ld      bc,$9180
                                  ;;                              0690        
                                  ;;                              0691    call    FCOMP
        pop     bc                ;;Discard Return Address        0692
        ld      c,a               ;;Put it in C for MORCOM        0693
        jp      COMPRT            ;;                              0694    pop     hl             
                                  ;;Do PRINT Comma Code           0695    ld      d,c    
                                  ;;                              0696    ret     z      
FCERR:  ld      e,ERRFC           ;[M65] "FUNCTION CALL" ERROR
        jp      ERROR             ;
;[M80]  LINGET READS A LINE # FROM THE CURRENT TEXT POSITION
;[M80]
;[M80]  LINE NUMBERS RANGE FROM 0 TO 65529
;[M80]
;[M80]  THE ANSWER IS RETURNED IN [D,E].
;[M80]  [H,L] IS UPDATED TO POINT TO THE TERMINATING CHARACTER
;[M80]  AND [A] CONTAINS THE TERMINATING CHARACTER WITH CONDITION
;[M80]  CODES SET UP TO REFLECT ITS VALUE.
;
;;Back Up and Read Line Number
SCNLIN: dec     hl
LINGET: ld      de,0              ;[M80] ASSUME START LIST AT ZERO
MORLIN: rst     CHRGET            ;;Get next character
        ret     nc                ;[M80] WAS IT A DIGIT
        push    hl                ;
        push    af                ;
        ld      hl,0+6552         ;[M80] SEE IF THE LINE # IS TOO BIG
        rst     COMPAR            ;
        jr      c,POPHSR          ;[M80] YES, DON'T SCAN ANY MORE DIGITS AND GIVE SYNTAX ERROR
        ld      h,d               ;[M80] SAVE [D,E]
        ld      l,e               ;
        add     hl,de             ;
        add     hl,hl             ;
        add     hl,de             ;
        add     hl,hl             ;[M80] PUTTING [D,E]*10 INTO [H,L]
        pop     af                ;
        sub     '0'               ;
        ld      e,a               ;
        ld      d,0               ;
        add     hl,de             ;[M80] ADD THE NEW DIGIT
        ex      de,hl             ;
        pop     hl                ;[M80] GET BACK TEXT POINTER
        jr      MORLIN            ;
POPHSR: pop     af                ;[M80] GET OFF TERMINATING DIGIT
        pop     hl                ;[M80] GET BACK OLD TEXT POINTER
        ret                       ;
RUN:    rst     HOOKDO            ;Call Hook Routine
        byte    24                ;
        jp      z,RUNC            ;[M80] NO LINE # ARGUMENT
        call    CLEARC            ;RESET THE STACK,DATPTR,VARIABLES ...
        ld      bc,NEWSTT         ;
        jr      RUNC2             ;[M80] PUT "NEWSTT" ON AND FALL INTO "GOTO"
;[M80] GOSUB STATEMENT
GOSUB:  ld      c,3               ;[M80] "GOSUB" ENTRIES ARE 5 BYTES LONG
        call    GETSTK            ;[M80] MAKE SURE THERE IS ROOM
        pop     bc                ;[M80] POP OFF RETURN ADDRESS OF "NEWSTT"
        push    hl                ;[M80] REALLY PUSH THE TEXT POINTER
        push    hl                ;[M80] SAVE TEXT POINTER
        ld      hl,(CURLIN)       ;[M80] GET THE CURRENT LINE #
        ex      (sp),hl           ;[M80] PUT CURLIN ON THE STACK AND [H,L]=TEXT PTR
        ld      a,GOSUTK          ;
        push    af                ;[M80] PUT GOSUB TOKEN ON THE STACK
        inc     sp                ;[M80] THE GOSUB TOKEN TAKES ONLY ONE BYTE
RUNC2:  push    bc                ;[M80] RESTORE RETURN ADDRESS OF "NEWSTT"
GOTO:   call    SCNLIN            ;[M80] PICK UP THE LINE # AND PUT IT IN [D,E]
        call    REM               ;[M80] SKIP TO THE END OF THIS LINE
        inc     hl                ;[M80] POINT AT THE LINK BEYOND IT
        push    hl                ;[M80] SAVE THE POINTER
        ld      hl,(CURLIN)       ;[M80] GET THE CURRENT LINE #
        rst     COMPAR            ;;Is target line less than current line
        pop     hl                ;[M80] [H,L]=CURRENT POINTER
        call    c,LOOP            ;[M80] SEARCH FROM THIS POINT
        call    nc,FNDLIN         ;[M80] SEARCH FROM THE BEGINNING -- ACTUALLY
        ld      h,b               ;[M80] [H,L]= POINTER TO THE START OF THE MATCHED LINE
        ld      l,c               ;
        dec     hl                ;
        ret     c                 ;[M80] GO TO NEWSTT
;{M80} GIVE A "US" ERROR
USERR:  ld      e,ERRUS           ;[M80] C=MATCH, SO IF NO MATCH WE
        jp      ERROR             ;[M80] GIVE A "US" ERROR
;[M80] RETURN STATEMENT
RETURN: ret     nz                ;[M80] BLOW HIM UP IF THERE ISN'T A TERMINATOR
        ld      d,255             ;[M80] MAKE SURE VARIABLE POINTER IN [D,E] NEVER GETS MATCHED
        call    FNDFOR            ;[M80] GO PAST ALL THE "FOR" ENTRIES
        ld      sp,hl             ;[M80] UPDATE THE STACK
        cp      GOSUTK            ;
        ld      e,ERRRG           ;[M80] ERROR ERRRG IS "RETURN WITHOUT GOSUB"
        jp      nz,ERROR          ;
        pop     hl                ;GET LINE # "GOSUB" WAS FROM
        ld      (CURLIN),hl       ;PUT IT INTO CURLIN
        inc     hl                ;
        ld      a,h               ;
        or      l                 ;;Is line number $FFFF
        jr      nz,RETU1          ;;No, carry on
        ld      a,(USFLG)         ;
        or      a                 ;;Is flag set?
        jp      nz,STPRDY         ;;Yes, abort to direct mode
RETU1:  ld      hl,NEWSTT         ;[M80] PUT RETURN ADDRESS OF "NEWSTT" BACK ON STACK
        ex      (sp),hl           ;[M80] GET TEXT POINTER FROM "GOSUB"
        byte    $3E               ;{M80} "LD A," AROUND POP HL
DATAH:  pop     hl                ;[M80] GET TEXT POINTER OFF STACK
;;The DATA Statement
DATA:   byte    $01               ;[M80] "LD BC," TO PICK UP ":" INTO C AND SKIP
        byte    ':'               ;{M80} ":" ONLY APPLIES IF QUOTES HAVE MATCHED UP
;;The REM Statement
REM:    byte    $0E               ;[M80] "LD C,"   THE ONLY TERMINATOR IS ZERO
        byte    0                 ;[M80] NO-OPERATION "DATA" ACTUALLY EXECUTES THIS 0
        ld      b,0               ;[M80] INSIDE QUOTES THE ONLY TERMINATOR IS ZERO
EXCHQT: ld      a,c               ;[M80] WHEN A QUOTE IS SEEN THE SECOND
        ld      c,b               ;[M80] TERMINATOR IS TRADED, SO IN "DATA"
        ld      b,a               ;[M80] COLONS INSIDE QUOTATIONS WILL HAVE NO EFFECT
REMER:  ld      a,(hl)            ;[M80] GET A CHAR
        or      a                 ;[M80] ZERO IS ALWAYS A TERMINATOR
        ret     z                 ;
        cp      b                 ;[M80] TEST FOR THE OTHER TERMINATOR
        ret     z                 ;
        inc     hl                ;
        cp      '"'               ;[M80] IS IT A QUOTE?
        jr      z,EXCHQT          ;[M80] IF SO TIME TO TRADE
        jr      REMER             ;
LET:    call    PTRGET            ;[M80] GET POINTER TO VARIABLE INTO [D,E]
        rst     SYNCHK            ;[M80]
        byte    EQUATK            ;[M80] CHECK FOR "="
        push    de                ;[M80] SAVE THE VARIABLE POINTER
        ld      a,(VALTYP)        ;{M80} REMEMBER THE VARIABLE TYPE
        push    af                ;
        call    FRMEVL            ;[M80] GET THE VALUE OF THE FORMULA
        pop     af                ;[M80] GET THE VALTYP OF THE VARIABLE INTO [A] INTO FAC
        ex      (sp),hl           ;[M80] [H,L]=POINTER TO VARIABLE, TEXT POINTER ON TOP OF STACK
        ld      (SAVTXT),hl       ;[???] PLACE TO SAVE THE VALUE
        rra                       ;
        call    CHKVAL            ;[M65] MAKE SURE "VALTYP" MATCHES CARRY AND SET ZERO FLAG FOR NUMERIC
        jp      z,COPNUM          ;[M80] NUMERIC, SO FORCE IT AND COPY
INPCOM: push    hl                ;
        ld      hl,(FACLO)        ;[M80] GET POINTER TO THE DESCRIPTOR OF THE RESULT
        push    hl                ;[M80] SAVE THE POINTER AT THE DESCRIPTOR
        inc     hl                ;
        inc     hl                ;
        ld      e,(hl)            ;
        inc     hl                ;
        ld      d,(hl)            ;
        ld      hl,(TXTTAB)       ;[M80] IF THE DATA IS IN BUF, COPY
        rst     COMPAR            ;[M80] SINCE BUF CHANGES ALL THE TIME
        jr      nc,INBUFC         ;[M80] GO COPY, IF DATA REALLY IS IN BUF
        ld      hl,(STREND)       ;[M80] SEE IF IT POINTS INTO STRING SPACE
        rst     COMPAR            ;[M80] IF NOT DON'T COPY
        pop     de                ;[M80] GET BACK THE POINTER AT THE DESCRIPTOR
        jr      nc,DNTCPY         ;[M80] DON'T COPY LITERALS
        ld      hl,DSCTMP         ;[M80] NOW, SEE IF ITS A VARIABLE BY SEEING IF THE DESCRIPTOR
        rst     COMPAR            ;[M80] IS IN THE TEMPORARY STORAGE AREA (BELOW DSCTMP)
        jr      nc,DNTCPY         ;[M80] DON'T COPY IF ITS NOT A VARIABLE
        byte    $3E               ;[M80] SKIP THE NEXT BYTE WITH A "MVI A,"
INBUFC: pop     de                ;[M80] GET THE POINTER TO THE DESCRIPTOR IN [D,E]
        call    FRETMS            ;[M80] FREE UP A TEMORARY POINTING INTO BUF
        ex      de,hl             ;[M80] STRCPY COPIES [H,L]
        call    STRCPY            ;[M80] COPY VARIABLES IN STRING SPACE OR STRINGS WITH DATA IN BUF
DNTCPY: call    FRETMS            ;[M80] FREE UP TEMPORARY WITHOUT FREEING UP ANY STRING SPACE
        pop     hl                ;[M80]
        call    MOVE              ;[M80] COPY A DESCRIPTOR OR A VALUE
        pop     hl                ;[M80] GET THE TEXT POINTER BACK
        ret                       ;
COPNUM: push    hl                
        call    MOVMF             ;COPY A DESCRIPTOR OR A VALUE
        pop     de                ;FOR "FOR" POP OFF POINTER AT LOOP VARIABLE INTO [D,E]
        pop     hl                ;GET THE TEXT POINTER BACK
        ret
;{M80} ON..GOTO, ON GOSUB CODE
ONGOTO: rst     HOOKDO            ;
        byte    25                ;
        call    GETBYT            ;[M80] GET VALUE INTO [E]
;;Execute ON..GOTO
OMGOTO  ld      a,(hl)            ;[M80] GET THE TERMINATOR BACK
        ld      b,a               ;[M80] SAVE THIS CHARACTER FOR LATER
        cp      GOSUTK            ;[M80] AN "ON ... GOSUB" PERHAPS?
        jr      z,ISGOSU          ;[M80] YES, SOME FEATURE USE
        rst     SYNCHK            ;
        byte    GOTOTK            ;[M80] OTHERWISE MUST BE "GOTO"
        dec     hl                ;[M80] BACK UP CHARACTER POINTER
ISGOSU: ld      c,e               ;[M80] GET COUNT INTO [C]
LOOPON: dec     c                 ;[M80] SEE IF ENOUGH SKIPS
        ld      a,b               ;[M80] PUT DISPATCH CHARACTER IN PLACE
        jp      z,GONE2           ;[M80] IF DONE, GO OFF
        call    LINGET            ;[M80] SKIP OVER A LINE #
        cp      ','               ;[M80] IS IT A COMMA?
        ret     nz                ;{M80} NO COMMA MUST BE THE END OF THE LINE
        jr      LOOPON            ;[M80] CONTINUE GOBBLING LINE #S
;[M80] IF ... THEN CODE
IFS:    call    FRMEVL            ;[M80] EVALUATE A FORMULA
        ld      a,(hl)            ;[M80] GET TERMINATING CHARACTER OF FORMULA
        cp      GOTOTK            ;[M80] ALLOW "GOTO" AS WELL
        jr      z,OKGOTO          ;
        rst     SYNCHK            ;
        byte    THENTK            ;[M80] MUST HAVE A THEN
        dec     hl                ;
OKGOTO: call    CHKNUM            ;[M65] 0=FALSE. ALL OTHERS TRUE
        rst     FSIGN             ;
        jp      z,REM             ;[M65] SKIP REST OF STATEMENT
        rst     CHRGET            ;[M80] PICK UP THE FIRST LINE # CHARACTER
        jp      c,GOTO            ;[M80] DO A "GOTO"
        jp      GONE3             ;[M80] EXECUTE STATEMENT, NOT GOTO
LPRINT: ld      a,1               ;SAY NON ZERO
        ld      (PRTFLG),a        ;SAVE AWAY
NEWCHR: dec     hl                ;
        rst     CHRGET            ;[M80] GET ANOTHER CHARACTER
;;; Code Change: PRINT and LPRINT Comma column width is now stored in System variable CLMWID
;;; To change the column width, POKE the width into 14405 and the last comma position into 14409
;;; The latter affects PRINT only. LPRINT has a hard coded last comma position of 112.
PRINT:  rst     HOOKDO            ;
        byte    6                 ;
        call    z,CRDO            ;[M80] PRINT CRLF IF END WITHOUT PUNCTUATION
PRINTC: jp      z,FINPRT          ;{M80} FINISH BY RESETTING FLAGS, TERMINATOR SHOULD NOY CRLF
        cp      TABTK             ;
        jp      z,TABER           ;[M80] THE TAB FUNCTION?
        cp      SPCTK             ;
        jp      z,TABER           ;[M80] THE SPC FUNCTION?
        push    hl                ;{M80} SAVE THE TEXT POINTER                Original Code - 4 bytes
        call    COMWID            ;;See if it's a comma                       07CF  cp      ','       
                                  ;;                                          07D0
                                  ;;                                          07D1  jr      z,COMPRT  
        nop                       ;;Filler                                    07D2
        cp      $3B               ;{M80} IS IT A ";"
        jp      z,NOTABR          ;
        pop     bc                ;[M80] GET RID OF OLD TEXT POINTER
        call    FRMEVL            ;[M80] EVALUATE THE FORMULA
        push    hl                ;[M80] SAVE TEXT POINTER
        ld      a,(VALTYP)        ;[M80] SEE IF WE HAVE A STRING
        or      a                 ;
        jp      nz,STRDON         ;[M80] IF SO, PRINT SPECIALY
        call    FOUT              ;[M80] MAKE A NUMBER INTO A STRING
        call    STRLIT            ;[M80] MAKE IT  A STRING
        ld      (hl),' '          ;[M80] PUT A SPACE AT THE END
        ld      hl,(FACLO)        ;[M80] SIZE BYTE IS FIRST IN DESCRIPTOR
        ld      a,(PRTFLG)        ;
        or      a                 ;
        jr      z,ISTTY           ;[M80] LPT OR TTY?
        ld      a,(LPTPOS)        ;
        add     a,(hl)            ;
        cp      LPTSIZ            ;[M80] CHECK FOR OVERLAP
        jr      LINCHK            ;[M80] START ON A NEW LINE
ISTTY:  ld      a,(LINLEN)        ;
        ld      b,a               ;
        inc     a                 ;[M80] NO OVERFLOW LINE WIDTH?
        jr      z,LINCH2          ;[M80] YES
        ld      a,(TTYPOS)        ;[M80] SEE WHERE WE ARE
        add     a,(hl)            ;
        dec     a                 ;[M80] ACTUALLY EQUAL TO LINE LENGTH IS OK
        cp      b                 ;
LINCHK: call    nc,CRDO           ;[M80] IF SO CRLF
LINCH2: call    STRPRT            ;[M80] PRINT THE NUMBER
        xor     a                 ;
STRDON: call    nz,STRPRT         ;[M80] PRINT THE NUMBER
        pop     hl                ;
        jr      NEWCHR            ;[M80[ PRINT SOME MORE
COMPRT: ld      a,(PRTFLG)        ;[M80[ OUTPUT TO THE LINE PRINTER?
        or      a                 ;[M80[ NON-ZERO MEANS YES
        jr      z,ISCTTY          ;[M80[ NO, DO TELETYPE COMMA
        ld      a,(LPTPOS)        ;[M80[ ARE WE USING INFINITE WIDTH?
        cp      112               ;[M80[ CHECK IF NO MORE COMMA FIELDS
        jp      CHKCOM            ;[M80[ USE TELETYPE CHECK
ISCTTY: ld      a,(CLMLST)        ;[M80] POSITION OF LAST COMMA COLUMN
        ld      b,a               ;
        ld      a,(TTYPOS)        ;[M80] GET TELETYPE POSITION
        cp      b                 ;
CHKCOM: call    nc,CRDO           ;[M80] TYPE CRLF
        jp      nc,NOTABR         ;[M80] AND QUIT IF BEYOND THE LAST COMMA FIELD
MORCOM: sub     c                 ;[M80] GET [A] MODULUS CLMWID
        jp      nc,MORCOM         ;
        cpl                       ;[M80] FILL OUT TO AN EVEN CLMWID: CLMWID-[A] MOD CLMWID SPACES
        jr      ASPA2             ;[M80] GO PRINT [A]+1 SPACES
TABER:  push    af                ;[M80] REMEMBER IF [A]=SPCTK OR TABTK
        call    GTBYTC            ;[M80] EVALUATE THE ARGUMENT
        rst     SYNCHK            ;
        byte    ')'               ;
        dec     hl                ;
        pop     af                ;[M80] GET BACK SPCTK OR TABTK
        sub     SPCTK             ;[M80] WAS IT SPCTK?
        push    hl                ;[M80] SAVE THE TEXT POINTER
        jr      z,DOSIZT          ;[M80] VALUE IN [A]
        ld      a,(PRTFLG)        ;[M80] LINE PRINTER OR TTY?
        or      a                 ;[M80] NON-ZERO MEANS LPT
        jp      z,TTYIST          ;
        ld      a,(LPTPOS)        ;[M80] GET LINE PRINTER POSITION
        jr      DOSIZT            ;
TTYIST: ld      a,(TTYPOS)        ;[M80] GET TELETYPE PRINT POSITION
DOSIZT: cpl                       ;[M80] PRINT [E]-[A] SPACES
        add     a,e               ;
        jr      nc,NOTABR         ;[M80] PRINT IF PAST CURRENT
ASPA2:  inc     a                 ;
        ld      b,a               ;[M80] [B]=NUMBER OF SPACES TO PRINT
        ld      a,' '             ;[M80] [A]=SPACE
REPOUT: rst     OUTCHR            ;[M80] PRINT [A]
        djnz    REPOUT            ;[M80] DECREMENT THE COUNT
NOTABR: pop     hl                ;[M80] PICK UP TEXT POINTER
        rst     CHRGET            ;[M80] AND THE NEXT CHARACTER
        jp      PRINTC            ;{M80} WE JUST PRINTED SPACES, DON'T CALL CRDO IF END OF THE LINE
FINPRT: rst     HOOKDO            ;
        byte    7                 ;
        xor     a                 ;
        ld      (PRTFLG),a        ;[M80] ZERO OUT PTRFIL
        ret                       ;
TRYAGN: byte    "?Redo from start",13,10,0
;[M80]  HERE WHEN THE DATA THAT WAS TYPED IN OR IN "DATA" STATEMENTS
TRMNOK: rst     HOOKDO            ;
        byte    8                 ;
        ld      a,(FLGINP)        ;[M80] WAS IT READ OR INPUT?
        or      a                 ;[M80] ZERO=INPUT
        jp      nz,DATSNE         ;[M80] GIVE ERROR AT DATA LINE
        pop     bc                ;[M80] GET RID OF THE POINTER INTO THE VARIABLE LIST
        ld      hl,TRYAGN         ;[M80] PRINT "?REDO FROM START"
        call    STROUT            ;
        jp      GTMPRT            ;
INPUT:  rst     HOOKDO            ;
        byte    26                ;
        call    ERRDIR            ;[M65] DIRECT IS NOT OK
        ld      a,(hl)            ;
        cp      '"'               ;[M80] IS IT A QUOTE?
        ld      a,0               ;[M80] BE TALKATIVE
        jp      nz,NOTQTI         ;[M65] NO MESSAGE
        call    STRLTI            ;[M65] LITERALIZE THE STRING IN TEXT
        rst     SYNCHK            ;
        byte    ';'               ;[M80] MUST END WITH SEMICOLON
        push    hl                ;[M80] REMEMBER WHERE IT ENDED
        call    STRPRT            ;[M80] PRINT IT OUT
        byte    $3E               ;;[LD A,] over next instruction
NOTQTI: push    hl                ;{M80} SAVE TEXT POINTER
        call    QINLIN            ;[M65] TYPE "?" AND INPUT A LINE OF TEXT.
        pop     bc                ;{M80} GET BACK THE TEXT POINTER
        jp      c,STPEND          ;{M80} IF CONTROL-C, STOP
        inc     hl                ;
        ld      a,(hl)            ;
        or      a                 ;
        dec     hl                ;
        push    bc                ;
        jp      z,DATAH           ;
        ld      (hl),','          ;[M80] SETUP COMMA AT BUFMIN
        jr      INPCON            ;
;[M80] READ STATEMENT
READ:   push    hl                ;[M80] SAVE THE TEXT POINTER
        ld      hl,(DATPTR)       ;[M80] GET LAST DATA LOCATION
        byte    $F6               ;[M80] "ORI" TO SET [A] NON-ZERO
INPCON: xor     a                 ;[M80] SET FLAG THAT THIS IS AN INPUT
        ld      (FLGINP),a        ;[M80] STORE THE FLAG
        ex      (sp),hl           ;[M80] [H,L]=VARIABLE LIST POINTER
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
LOPDT2: rst     SYNCHK
        byte    ','               ;[M80] MAKE SURE THERE IS A ","
        call    PTRGET            ;[M80] GET THE POINTER TO A VARIABLE INTO [D,E]
        ex      (sp),hl           ;;Swap with Data List Pointer
;[M80] NOTE AT THIS POINT WE HAVE A VARIABLE WHICH WANTS DATA
;[M80] AND SO WE MUST GET DATA OR COMPLAIN
        push    de                ;[M80] SAVE THE POINTER TO THE VARIABLE
        ld      a,(hl)            ;[M80] READ DATA LIST TERMINATOR
        cp      ','               ;
        jr      z,DATBK           ;[M80] A COMMA SO A VALUE MUST FOLLOW
        ld      a,(FLGINP)        ;[M80] SEE WHAT TYPE OF STATEMENT THIS WAS
        or      a                 ;;???ZERO FOR INPUT, OTHERWISE READ
        jp      nz,DATLOP         ;[M80] SEARCH FOR ANOTHER DATA STATEMENT
        ld      a,'?'             ;[M80] TYPE "?" AND INPUT A LINE OF TEXT
        rst     OUTCHR            ;
        call    QINLIN            ;
        pop     de                ;
        pop     bc                ;[M80] TAKE OFF SINCE MAYBE LEAVING
        jp      c,STPEND          ;[M80] IF EMPTY LEAVE
        inc     hl                ;
        ld      a,(hl)            ;
        dec     hl                ;
        or      a                 ;
        push    bc                ;[M80] PUT BACK  SINCE DIDN'T LEAVE
        jp      z,DATAH
        push    de                ;{M80} SAVE THE POINTER TO THE VARIABLE
DATBK:  rst     HOOKDO            ;;Call Extended Hook 28
        byte    28                ;
        ld      a,(VALTYP)        ;[M80] IS IT A STRING?
        or      a                 ;
        jr      z,NUMINS          ;[M80] IF NUMERIC, USE FIN TO GET IT
        rst     CHRGET            ;
        ld      d,a               ;[M80] ASSUME QUOTED STRING
        ld      b,a               ;[M80] SETUP TERMINATORS
        cp      '"'               ;[M80] QUOTE ?
        jr      z,NOWGET          ;[M80] TERMINATORS OK
        ld      a,(FLGINP)        ;[M80] INPUT SHOULDN'T TERMINATE ON ":"
        or      a                 ;[M80] SEE IF READ OR INPUT
        ld      d,a               ;[M80] SET D TO ZERO FOR INPUT
        jr      z,NCOLST          ;
        ld      d,':'             ;[M80] UNQUOTED STRING TERMINATORS
NCOLST: ld      b,','             ;[M80] ARE COLON AND COMMA
        dec     hl                ;[M80] START CHARACTER MUST BE INCLUDED IN THE QUOTED STRING
NOWGET: call    STRLT2            ;[M80] MAKE STRING DESCRIPTOR FOR VALUE AND COPY IF NECESSARY
        ex      de,hl             ;
        ld      hl,STRDN2         ;[M80] RETURN LOC
        ex      (sp),hl           ;[M80] [H,L]=PLACE TO STORE VARIABLE VALUE
        push    de                ;[M80] TEXT POINTER GOES ON
        jp      INPCOM            ;[M80] DO ASSIGNMENT
NUMINS: rst     CHRGET            ;
        call    FIN               ;[M80] CALL # INPUTTER
        ex      (sp),hl           ;*** tail end of [M80] FIN?
        call    MOVMF             ;
        pop     hl                ;
STRDN2: dec     hl                ;
        rst     CHRGET            ;
        jr      z,TRMOK           ;
        cp      ','               ;
        jp      nz,TRMNOK         ;[M80] ENDED PROPERLY?
TRMOK:  ex      (sp),hl           ;
        dec     hl                ;[M80] LOOK AT TERMINATOR
        rst     CHRGET            ;[M80] AND SET UP CONDITION CODES
        jp      nz,LOPDT2         ;[M80] NOT ENDING, CHECK FOR COMMA AND GET ANOTHER VARIABLE
        pop     de                ;[M80] POP OFF THE POINTER INTO DATA
        ld      a,(FLGINP)        ;[M80] FETCH THE STATEMENT TYPE FLAG
        or      a                 ;
        ex      de,hl             ;[M80] INPUT STATEMENT
        jp      nz,RESFIN         ;[M80] UPDATE DATPTR
        push    de                ;[M80] SAVE THE TEXT POINTER
        or      (hl)              ;
        ld      hl,EXIGNT         ;
        call    nz,STROUT         ;[M65] TYPE "?EXTRA IGNORED"
        pop     hl                ;
        ret                       ;[M65[ DO NEXT STATEMENT
EXIGNT: byte    "?Extra ignored",13,10,0
;[M80] THE SEARCH FOR DATA STATMENTS IS MADE BY USING THE EXECUTION CODE
DATLOP: call    DATA              ;
        or      a                 ;
        jr      nz,NOWLIN         ;
        inc     hl                ;
        ld      a,(hl)            ;
        inc     hl                ;
        or      (hl)              ;
        ld      e,ERROD           ;[M80] NO DATA IS ERROR ERROD
        jp      z,ERROR           ;[M80] IF SO COMPLAIN
        inc     hl                ;[M80] SKIP PAST LINE #
        ld      e,(hl)            ;[M80] GET DATA LINE #
        inc     hl                ;
        ld      d,(hl)            ;
        ld      (DATLIN),de       ;
NOWLIN: rst     CHRGET            ;[M80] GET THE STATEMENT TYPE
        cp      DATATK            ;[M80] IS IS "DATA"?
        jr      nz,DATLOP         ;[M80] NOT DATA SO LOOK SOME MORE
        jp      DATBK             ;[M80] CONTINUE READING
;[M65] THESE ROUTINES CHECK FOR CERTAIN "VALTYP".
;[M65] [C] IS NOT PRESERVED.
;
;;Evaluate a Number
FRMNUM: call    FRMEVL            ;{M80} EVALUATE A FORMULA
;;Issue "TM" Error if Not a Number
CHKNUM: byte    $F6               ;;"OR $37" to skip next instruction
CHKSTR: scf                       ;;Set Carry to match strings
CHKVAL: ld      a,(VALTYP)        ;
        adc     a,a               ;;Shift Left and Copy in Cary
        or      a                 ;;Set Flags
        ret     pe                ;;Return if Overflow
CHKERR: jp      TMERR             ;;Else TYPE MISMATCH Error
;[M80] THE FORMULA EVALUATOR STARTS WITH
FRMEQL: rst     SYNCHK            ;
        byte    EQUATK            ;[M80] CHECK FOR EQUAL SIGN
        byte    $01               ;[M80] "LD BC," OVER THE NEXT 2
FRMPRN: rst     SYNCHK            ;[M80] GET PAREN BEFORE FORMULA
        byte    '('               ;
;;Evaluate Formula
;;; Code Change: Add Hook for Operator Evaluation
FRMEVL: dec     hl                ;[M80] BACK UP CHARACTER POINTER
FRMCHK: ld      d,0               ;[M80] INITIAL DUMMY PRECEDENCE IS 0
LPOPER: push    de                ;[M80] SAVE PRECEDENCE
        ld      c,1               ;[M80] EXTRA SPACE NEEDED FOR RETURN ADDRESS
        call    GETSTK            ;[M80] MAKE SURE THERE IS ROOM FOR RECURSIVE CALLS
        call    EVAL              ;[M80] EVALUATE SOMETHING
TSTOP:  ld      (TEMP2),hl        ;[M80] SAVE TEXT POINTER
RETAOP: ld      hl,(TEMP2)        ;[M80] RESTORE TEXT PTR
        pop     bc                ;[M80] POP OFF THE PRECEDENCE OF OLDOP
        ld      a,b               ;
        cp      $78               ;
        call    nc,CHKNUM         ;
        ld      a,(hl)            ;[M80] GET NEXT CHARACTER
        ld      (TEMP3),hl        ;[M80] SAVE UPDATED CHARACTER POINTER       Original Code
        rst     HOOKDO            ;;                                          09A2  cp      PLUSTK     
        byte    29                ;;                                          09A3  
        call    CHKOP             ;;If it's not an Operator                   09A4  ret     c                 
                                  ;;                                          09A5  cp      LESSTK+1          
                                  ;;                                          09A6   
        ret     c                 ;;  Return                                  09A7  ret     nc                
        cp      GREATK            ;[M80] SOME KIND OF RELATIONAL?
        jp      nc,DORELS         ;[M80] YES, DO IT
        sub     PLUSTK            ;[M80] SUBTRACT OFFSET FOR FIRST ARITHMETIC
        ld      e,a               ;[M80] MUST MULTIPLY BY 3
        jr      nz,NTPLUS         ;[M80] NOT ADDITION OP
        ld      a,(VALTYP)        ;[M80] SEE IF LEFT PART IS STRING
        dec     a                 ;[M80] SEE IF LEFT PART IS STRING
        ld      a,e               ;[M80] REFETCH OP-VALUE
        jp      z,CAT             ;[M80] MUST BE CAT
NTPLUS: rlca                      ;[M65] MULTIPLY BY 2
        add     a,e               ;[M65] BY THREE.
        ld      e,a               ;[M65] SET UP FOR LATER
        ld      hl,OPTAB          ;[M80] CREATE INDEX INTO OPTAB
        ld      d,0               ;[M80] MAKE HIGH BYTE OF OFFSET=0
        add     hl,de             ;[M80] ADD IN CALCULATED OFFSET
        ld      a,b               ;[M80] [A] GETS OLD PRECEDENCE
        ld      d,(hl)            ;[M80] REMEMBER NEW PRECEDENCE
        cp      d                 ;[M80] OLD-NEW
        ret     nc                ;[M80] APPLY OLD OP IF >= PRECEDENCE
        inc     hl                ;
        call    CHKNUM            ;
FINTMP: push    bc                ;[M80] SAVE THESE THINGS FOR RETAOP
        ld      bc,RETAOP         ;[M80] GENERAL OPERATOR APPLICATION ROUTINE -- DOES TYPE CONVERSIONS
        push    bc                ;[M80] SAVE PLACE TO GO
        ld      b,e               ;;Save dispatch offset and precedence
        ld      c,d               ;
        call    PUSHF             ;[M80] PUT FAC ON STACK
        ld      e,b               ;
        ld      d,c               ;;Restore dispatch offset and precedence
        ld      c,(hl)            ;;Load FINRE2 address from FINREA into BC
        inc     hl                ;
        ld      b,(hl)            ;
        inc     hl                ;;Now HL contains FINRE2 address
        push    bc                ;;Push FINRE2 address
        ld      hl,(TEMP3)        ;REGET THE TEXT POINTER
        jp      LPOPER            ;PUSH ON THE PRECEDENCE AND READ MORE FORMULA
;;Evaluate Logical Operators
DORELS: ld      d,0               ;{M80} ASSUME NO RELATION OPS, SETUP HIGH ORDER OF INDEX INTO OPTAB
LOPREL: sub     GREATK            ;[M80] IS THIS ONE RELATION?
        jp      c,FINREL          ;[M80] RELATIONS ALL THROUGH
        cp      LESSTK-GREATK+1   ;[M80] IS IT REALLY RELATIONAL?
        jp      nc,FINREL         ;[M80] NO JUST BIG
        cp      1                 ;[M80] SET UP BITS BY MAPPING
        rla                       ;[M80] 0 TO 1 1 TO 2 AND 2 TO 4
        xor     d                 ;[M80] BRING IN THE OLD BITS
        cp      d                 ;[M80] MAKE SURE RESULT IS BIGGER
        ld      d,a               ;[M80] SAVE THE MASK
        jp      c,SNERR           ;[M80] DON'T ALLOW TWO OF THE SAME
        ld      (TEMP3),hl        ;[M80] SAVE CHARACTER POINTER
        rst     CHRGET            ;[M80] GET THE NEXT CANDIDATE
        jr      LOPREL            ;
;[M80] EVALUATE VARIABLE, CONSTANT, FUNCTION CALL
EVAL:   rst     HOOKDO            ;
        byte    9                 ;
        xor     a                 ;
        ld      (VALTYP),a        ;[M65] ASSUME VALUE WILL BE NUMERIC
        rst     CHRGET            ;
        jp      z,MOERR           ;[M80] TEST FOR MISSING OPERAND - IF NONE GIVE ERROR
        jp      c,FIN             ;[M80] IF NUMERIC, INTERPRET CONSTANT
        call    ISLETC            ;[M80] VARIABLE NAME?
        jp      nc,ISVAR          ;[M80] AN ALPHABETIC CHARACTER MEANS YES
        cp      PLUSTK            ;[M80] IGNORE "+"
        jr      z,EVAL            ;
        cp      '.'               ;[M65] LEADING CHARACTER OF CONSTANT?
        jp      z,FIN             ;
        cp      MINUTK            ;[M80] NEGATION?
        jp      z,DOMIN           ;[M65] SHO IS.
        cp      '"'               ;[M80] STRING CONSTANT?
        jp      z,STRLTI          ;[M80] IF SO BUILD A TEMPORARY DESCRIPTOR
        cp      NOTTK             ;[M80] CHECK FOR "NOT" OPERATOR
        jp      z,NOTER           ;
        cp      INKETK            ;[M80] INKEY$ FUNCTION?
        jp      z,INKEY           ;
        cp      FNTK              ;
        jp      z,FNDOER          ;
        sub     ONEFUN            ;[M80] IS IT A FUNCTION CALL
        jp      nc,ISFUN          ;[M80] YES, DO IT
;[M80] ONLY POSSIBILITY LEFT IS A FORMULA IN PARENTHESES
;;Recursively Evaluate Formula in Parentheses
PARCHK: call    FRMPRN            ;[M80] RECURSIVELY EVALUATE THE FORMULA
        rst     SYNCHK            ;
        byte    ')'               ;
        ret                       ;
DOMIN:  ld      d,125             ;[M80] A PRECEDENCE BELOW ^ BUT ABOVE ALL ELSE
        call    LPOPER            ;[M80] SO ^ GREATER THAN UNARY MINUS
        ld      hl,(TEMP2)        ;[M80] GET TEXT POINTER
        push    hl                ;
        call    NEG               ;
LABBCK: call    CHKNUM            ;[M80] FUNCTIONS THAT DON'T RETURN
        pop     hl                ;[M80] STRING VALUES COME BACK HERE
        ret                       ;
;;Get Variable Value or String Pointer
ISVAR:  call    PTRGET            ;[M80] GET A POINTER TO THE VARIABLE IN [D,E]
RETVAR: push    hl                ;[M80] SAVE THE TEXT POINTER
        ex      de,hl             ;{M80} PUT THE POINTER TO THE VARIABLE OR STRING DESCRIPTOR
        ld      (FACLO),hl        ;[M80]IN CASE IT'S STRING STORE POINTER TO THE DESCRIPTOR IN FACLO.
        ld      a,(VALTYP)        ;[M80]FOR STRINGS WE JUST LEAVE
        or      a                 ;[M80]A POINTER IN THE FAC
        call    z,MOVFM           ;[M80]THE FAC USING [H,L] AS THE POINTER.
        pop     hl                ;[M80]RESTORE THE TEXT POINTER
        ret                       ;
;[M80] MOST FUNCTIONS TAKE A SINGLE ARGUMENT.
;[M80] THE RETURN ADDRESS OF THESE FUNCTIONS IS A SMALL ROUTINE
;[M80] THAT CHECKS TO MAKE SURE VALTYP IS 0 (NUMERIC) AND POPS OFF
;[M80] THE TEXT POINTER. SO NORMAL FUNCTIONS THAT RETURN STRING RESULTS (I.E. CHR$)
;[M80] MUST POP OFF THE RETURN ADDRESS OF LABBCK, AND POP OFF THE
;[M80] TEXT POINTER AND THEN RETURN TO FRMEVL.
;[M80]
;[M80] THE SO CALLED "FUNNY" FUNCTIONS CAN TAKE MORE THAN ONE ARGUMENT.
;[M80] THE FIRST OF WHICH MUST BE STRING AND THE SECOND OF WHICH
;[M80] MUST BE A NUMBER BETWEEN 0 AND 256. THE TEXT POINTER IS
;[M80] PASSED TO THESE FUNCTIONS SO ADDITIONAL ARGUMENTS
;[M80] CAN BE READ. THE TEXT POINTER IS PASSED IN [D,E].
;[M80] THE CLOSE PARENTHESIS MUST BE CHECKED AND RETURN IS DIRECTLY
;[M80] TO FRMEVL WITH [H,L] SETUP AS THE TEXT POINTER POINTING BEYOND THE ")".
;[M80] THE POINTER TO THE DESCRIPTOR OF THE STRING ARGUMENT
;[M80] IS STORED ON THE STACK UNDERNEATH THE VALUE OF THE INTEGER
;[M80] ARGUMENT (2 BYTES)
;
NUMGFN  =       (CHRTK-ONEFUN)*2+1
ISFUN:  rst     HOOKDO            ;
        byte    27                ;
        cp      POINTK-ONEFUN     ;;Is it POINT()
        jp      z,POINT           ;;Yes, go do it
        ld      b,0               ;
        rlca                      ;[M80] MULTIPLY BY 2
        ld      c,a               ;
        push    bc                ;[M80] SAVE THE FUNCTION # ON THE STACK
        rst     CHRGET            ;
        ld      a,c               ;[M80] LOOK AT FUNCTION #
        cp      NUMGFN            ;[M80] IS IT PAST LASNUM?
        jr      c,OKNORM          ;[M80] NO, MUST BE A NORMAL FUNCTION
        call    FRMPRN            ;[M80] EAT OPEN PAREN AND FIRST ARG
        rst     SYNCHK            ;
        byte    ','               ;[M80] TWO ARGS SO COMMA MUST DELIMIT
        call    CHKSTR            ;[M80] MAKE SURE THE FIRST ONE WAS STRING
        ex      de,hl             ;[M80] [D,E]=TXTPTR
        ld      hl,(FACLO)        ;[M80] GET PTR AT STRING DESCRIPTOR
        ex      (sp),hl           ;[M80] GET FUNCTION #, SAVE THE STRING PTR
        push    hl                ;[M80] PUT THE FUNCTION # ON
        ex      de,hl             ;[M80] [H,L]=TXTPTR
        call    GETBYT            ;[M80] [E]=VALUE OF FORMULA
        ex      de,hl             ;[M80] TEXT POINTER INTO [D,E], [H,L]=INT VALUE OF SECOND ARGUMENT
        ex      (sp),hl           ;[M80] SAVE INT VALUE OF SECOND ARG. [H,L]=FUNCTION NUMBER
        jr      FINGO             ;[M80] DISPATCH TO FUNCTION
OKNORM: call    PARCHK            ;[M80] CHECK OUT THE ARGUMEN AND MAKE SURE ITS FOLLOWED BY ")"
        ex      (sp),hl           ;[M80] [H,L]=FUNCTION # AND SAVE TEXT POINTER
        ld      de,LABBCK         ;[M80] RETURN ADDRESS
        push    de                ;[M80] MAKE THEM REALLY COME BACK
FINGO:  ld      bc,FUNDSP         ;[M80] FUNCTION DISPATCH TABLE
        add     hl,bc             ;[M80] ADD ON THE OFFSET
        ld      c,(hl)            ;{M80} FASTER THAN "PUSH HL"
        inc     hl                ;
        ld      h,(hl)            ;
        ld      l,c               ;
        jp      (hl)              ;[M80] GO PERFORM THE FUNCTION
;[M80] THE FOLOWING ROUTINE IS CALLED FROM FIN
;[M80] TO SCAN LEADING SIGNS FOR NUMBERS.
MINPLS: dec     d                 ;[M80] SET SIGN OF EXPONENT FLAG
        cp      MINUTK            ;[M80] NEGATIVE EXPONENT?
        ret     z                 ;
        cp      '-'               ;[M80] NO, RESET FLAG
        ret     z                 ;
        inc     d                 ;
        cp      '+'               ;
        ret     z                 ;
        cp      PLUSTK            ;[M80] IGNORE "+"
        ret     z                 ;
        dec     hl                ;[M80] CHECK IF LAST CHARACTER WAS A DIGIT
        ret                       ;[M80] RETURN WITH NON-ZERO SET
;;AND and OR Operators
OROP:   byte    $F6               ;[M80] OR $AF TO SET THE PRECEDENCE "OR"=70
ANDOP:  xor     a                 ;;leave 0 in A for AND
DANDOR: push    af                ;[M80] SAVE THE PRECEDENCE or Operator...
        call    CHKNUM            ;[M65] MUST BE NUMBER
        call    FRCINT            ;COERCE RIGHT HAND ARGUMENT TO INTEGER
        pop     af                ;GET BACK THE PRECEDENCE TO DISTINGUISH "AND" AND "OR"
        ex      de,hl             ;
        pop     bc                ;
        ex      (sp),hl           ;
        ex      de,hl             ;
        call    MOVFR             ;
        push    af                ;
        call    FRCINT            ;
        pop     af                ;
        pop     bc                ;
        ld      a,c               ;
        ld      hl,GIVINT         ;{M80} PLACE TO JUMP WHEN DONE
        jp      nz,NOTAND         ;
        and     e                 ;
        ld      c,a               ;
        ld      a,b               ;
        and     d                 ;
        jp      (hl)              ;[M80] RETURN THE INTEGER [A,L]
NOTAND: or      e                 ;
        ld      c,a               ;
        ld      a,b               ;
        or      d                 ;
        jp      (hl)              ;[M80] RETURN THE INTEGER [A,L]
;[M80] HERE TO BUILD AN ENTRY FOR A RELATIONAL OPERATOR
FINREL: ld      hl,FINREA
        ld      a,(VALTYP)        ;[M80] SEE IF WE HAVE A NUMERIC COMPARE
        rra                       ;{M65} GET VALUE TYPE INTO CARRY
        ld      a,d               ;
        rla                       ;{M65} PUT VALTYP INTO LOW ORDER BIT OF MASK
        ld      e,a               ;[M80] DISPATCH OFFSET FOR COMPARES IN APPLOP
        ld      d,100             ;{MM80] [A]=OLD PRECEDENCE
        ld      a,b               ;
        cp      d                 ;[M80] RELATIONALS HAVE PRECEDENCE 100
        ret     nc                ;[M80] APPLY EARLIER OPERATOR IF IT HAS HIGHER PRECEDENCE
        jp      FINTMP            ;
FINREA: word    FINRE2
FINRE2: ld      a,c
        or      a
        rra
        pop     bc
        pop     de
        push    af
        call    CHKVAL            ;[M80] SEE IF WE HAVE A NUMERIC COMPARE
        ld      hl,DOCMP          ;[M80] ROUTINE TO TAKE COMPARE ROUTINE RESULTAND RELATIONAL BITS AND RETURN THE ANSWER
        push    hl
        jp      z,FCOMP           ;{M89} COMPARE NUMBERS RETURNING $7F IF FAC IS LESS THAN THE REGISTERS
        xor     a
        ld      (VALTYP),a
        jp      STRCMP
DOCMP:  inc     a                 ;[M80] SETUP BITS
        adc     a,a               ;[M80] 4=LESS 2=EQUAL 1=GREATER
        pop     bc                ;[M80] WHAT DID HE WANT?
        and     b                 ;[M80] ANY BITS MATCH?
        add     a,$FF             ;[M80] MAP 0 TO 0
        sbc     a,a               ;[M80] AND ALL OTHERS TO 377
        jp      FLOAT             ;[M80] CONVERT [A] TO AN INTEGER SIGNED
NOTER:  ld      d,90              ;[M80] "NOT" HAS PRECEDENCE 90, SO FORMULA EVALUATION
        call    LPOPER            ;[M80] IS ENTERED WITH A DUMMY ENTRY OF 90 ON THE STACK
        call    CHKNUM            ;[M65] MUST BE NUMBER
        call    FRCINT            ;[M80] COERCE THE ARGUMENT TO INTEGER
        ld      a,e               ;[M80] COMPLEMENT [D,E]
        cpl                       ;
        ld      c,a               ;
        ld      a,d               ;
        cpl                       ;
        call    GIVINT            ;[M80] UPDATE THE FAC
        ;[M80] FRMEVL, AFTER SEEING THE PRECEDENCE OF 90 THINKS IT IS APPLYING AN OPERATOR
        ;[M80] SO IT HAS THE TEXT POINTER IN TEMP2 SO RETURN TO REFETCH IT
        pop     bc                ;
        jp      RETAOP            ;
;{M80} SUBTRACTS [D,E] FROM [H,L] AND FLOAT THE RESULT LEAVING IT IN FAC
;;;Named GIVDBL in [M80] and [GWB] but Aquarius only supports floats
GIVFLT: ld      a,l               ;[M80] [H,L]=[H,L]-[D,E]
        sub     e                 ;
        ld      c,a               ;
        ld      a,h               ;[M80] SAVE HIGH BYTE IN [H]
        sbc     a,d               ;
GIVINT: ld      b,c               ;;Float Integer MSB=[A], LSB=[C]
FLOATB: ld      d,b               ;;Float Integer MSB=[A], LSB=[B]
FLOATD: ld      e,0               ;;Float Integer MSB=[A], LSB=[D]
        ld      hl,VALTYP         ;
        ld      (hl),e            ;[M80] SET VALTYP TO "FLOATING POINT"
        ld      b,144             ;{M80} SET EXPONENT
        jp      FLOATR            ;[M80] GO FLOAT THE NUMBER
LPOS:   ld      a,(LPTPOS)        ;{M80} GET PRINT HEAD POSITION
        jr      SNGFLT            ;
POS:    ld      a,(TTYPOS)        ;[M80] GET TELETYPE POSITION
SNGFLT: ld      b,a               ;[M80] MAKE [A] AN UNSIGNED INTEGER
        xor     a                 ;
        jp      FLOATB            ;
;;DEF FNx Stub
DEF:    rst     HOOKDO            ;;If not hooked
        byte    15
        jp      SNERR             ;;Syntax Error
;;FNx Stub
FNDOER: rst     HOOKDO            ;;If not hooked
        byte    16
        jp      SNERR             ;;Syntax Error
;[M65] SUBROUTINE TO SEE IF WE ARE IN DIRECT MODE AND COMPLAIN IF SO.
ERRDIR: push    hl                ;
        ld      hl,(CURLIN)       ;[M65] DIR MODE HAS [CURLIN]=$FFFF
        inc     hl                ;[M65] SO NOW, IS RESULT ZERO?
        ld      a,h               ;
        or      l                 ;
        pop     hl                ;
        ret     nz                ;[M65] YES
        ld      e,ERRID           ;[M65] INPUT DIRECT ERROR CODE
        jp      ERROR             ;
GTBYTC: rst     CHRGET            ;
;;Evaluate 8 bit Numeric Value
GETBYT: call    FRMNUM            ;[M80] EVALUATE A FORMULA
;;Convert FAC to Byte in [A]
CONINT: call    INTFR2            ;[M80] CONVERT THE FAC TO AN INTEGER IN [D,E]
        ld      a,d               ;[M80] SET THE CONDITION CODES BASED ON THE HIGH ORDER
        or      a                 ;
        jp      nz,FCERR          ;[M80] WASN'T ERROR
        dec     hl                ;[M80] fUNCTIONS CAN GET HERE WITH BAD [H,L] BUT NOT SERIOUS
        rst     CHRGET            ;[M80] SET CONDITION CODES ON TERMINATOR
        ld      a,e               ;[M80] RETURN THE RESULT IN [A] AND [E]
        ret                       ;
;;; Code Change: Remove Memory Protection
PEEK:   call    FRCINT            ;[M80] GET AN INTEGER IN [D,E]
        nop                       ;;                                          0B66  call    PROMEM
        nop                       ;;                                          0B67
        nop                       ;;                                          0B68
        ld      a,(de)            ;[M80] GET THE VALUE TO RETURN
        jp      SNGFLT            ;[M80] AND FLOAT IT
;;; Code Change: Remove Memory Protection
POKE:   call    FRMNUM            ;[M80] READ A FORMULA
        call    FRCINT            ;{M80} FORCE VALUE INTO INT IN [D,E]        Original Code
        nop                       ;;                                          0B73  call    PROMEM
        nop                       ;;                                          0B74
        nop                       ;;                                          0B75
        push    de                ;[M80] PUT VALUE ON STACK
        rst     SYNCHK            ;
        byte    ','               ;[M80] CHECK FOR A COMMA
        call    GETBYT
        pop     de                ;[M80] GET THE ADDRESS BACK
        ld      (de),a            ;[M80] STORE IT AWAY
        ret                       ;[M80] SCANNED EVERYTHING

;; Orphan Code - GETINT does the same thing
;;; Code Change: New Default USR Routine - Execute Code at Argument Address
;;; Replaces orphan routine FRMINT  (use GETINT instead)- 9 bytes             Original Code
USRDO:  call    FRCINT            ;;Convert Argument to Int in DE             0B7F  call    FRMEVL  
                                  ;;                                          0B80
                                  ;;                                          0B81
        ld      ixh,d             ;;Copy into IX                              0B82  push    hl
                                  ;;                                          0B83  call    FRCINT
        ld      ixl,e             ;;                                          0B84
                                  ;;                                          0B85
        jp      (IX)              ;;Jump to it                                0B86  pop     hl
                                  ;;                                          0B87  ret
                                  
ifdef fastbltu
;; Code Change: Do the Block Transfer and return same values as original BLTU routine
;;; Replaces Deprecated Routine: PROMEM - 10 Bytes
BLTUDO: inc     bc                ;;6    Byte Count                  PROMEM:  0B88 push    hl
        lddr                      ;;16+5 Do the Memory Move                   0B89 ld      hl,$2FFF       
                                  ;;4	  BC = End Destination                  0B8A
        ld      b,d               ;;                                          0B8B
        ld      c,e               ;;4	                                        0B8C
        inc     bc                ;;6	  Bump it up                            0B8D rst     COMPAR  
        inc     hl                ;;6	  Bump up End Source                    0B8E pop     hl      
        ld      d,h               ;;4	  Copy into DE                          0B8F jp      nc,FCERR
        ld      e,l               ;;4	                                        0B90 
        ret                       ;;10	                                      0B91
                           
;;; Code Change: Replace Loop with LDDR
;;; Original Code: 46 + byte count * 54 cycles - 207 millseconds per kilobyte (57780 * 3.579545 / 1000) 
;;; Updated Code: 113 + byte count * 21 cycles - 85 millseconds per kilobyte (23877 * 3.579545 / 1000) 
BLTU:   call    REASON            ;[M80] CHECK DESTINATION TO MAKE SURE STACK WON'T BE OVERRUN
;;Execute Block Transfer
;;REASON returned with Carry Set which will be used in the cause sbc hl,de to 
BLTUC:  push    bc                ; +11 [M80] EXCHANGE [B,C] AND [H,L]        Original Code      +11      Setup: 40 cycles
        ex      (sp),hl           ;;+19 HL = Source, Stack = Destination                         +19  
        ld      a,l               ;;+4  Stack = Source, Destination           pop     bc         +10              
        sub     a,e               ;;+4  BC = HL - DE (Byte Delta)     BLTLOP: rst     COMPAR     +11      Copy Loop: 54 cycles
        ld      c,a               ;;+4                                        ld      a,(hl)     +7  
        ld      a,h               ;;+4  BC = Number of Bytes                  ld      (bc),a     +7  
        sbc     a,d               ;;+4                                        ret     z          +5+6     Finish: 6 cycles
        ld      b,a               ;;+4  HL = Source                           dec     bc         +6  
        jp      BLTUDO            ;;+10 Do the LDIR                           dec     hl         +6  
                                  ;;                                          jr      BLTLOP     +12  
                                  ;;Setup: 80 cycles 
else
PROMEM: push    hl                ;
        ld      hl,$2FFF          ;
        rst     COMPAR            ;{M80} IS [D.E] LESS THAN 3000H?
        pop     hl                ;
        jp      nc,FCERR          ;{M80} YES, BLOW HIM UP NOW
        ret                       ;

BLTU:   call    REASON            ;[M80] CHECK DESTINATION TO MAKE SURE STACK WON'T BE OVERRUN
;Execute Block Transfer
BLTUC:  push    bc                ;[M80] EXCHANGE [B,C] AND [H,L]
        ex      (sp),hl           ;
        pop     bc                ;
BLTLOP: rst     COMPAR            ;[M80] SEE IF WE ARE DONE
        ld      a,(hl)            ;[M80] GET THE WORD TO TRANSFER
        ld      (bc),a            ;[M80] TRANSFER IT
        ret     z                 ;
        dec     bc                ;
        dec     hl                ;[M80] BACKUP FOR NEXT GUY
        jr      BLTLOP            ;
endif

;;Check Stack Size
GETSTK: push    hl                ;[M80] SAVE [H,L]
        ld      hl,(STREND)       ;
        ld      b,0               ;
        add     hl,bc             ;
        add     hl,bc             ;[M80] SEE IF WE CAN HAVE THIS MANY
        byte    $3E               ;;"LD A," over next instruction
REASON: push    hl                ;;Save text pointer
        ld      a,208             ;[M80]  EXAMINE [H,L] TO MAKE SURE
        sub     l                 ;[M80]  AT LEAST 104 LOCATIONS
        ld      l,a               ;[M80]  REMAIN BETWEEN IT AND THE
        ld      a,$FF             ;[M80]  TOP OF THE STACK
        sbc     a,h               ;
        ld      h,a               ;[M80] NOW SEE IF [SP] IS LARGER
        jr      c,OMERR           ;[M80] IN CASE [H,L] WAS TOO BIG
        add     hl,sp             ;[M80] IF SO, CARRY WILL BE SET
        pop     hl                ;[M80] GET BACK ORIGINAL [H,L]
        ret     c                 ;[M80] WAS OK?
OMERR:  ld      de,ERROM          ;;"OUT OF MEMORY" Error
        jp      ERROR             ;
;[M80] THE "NEW" COMMAND CLEARS THE PROGRAM TEXT AS WELL AS VARIABLE SPACE
;;SCRATH is the entry point from the Statement Dispatch Table for the NEW command
;;SCRTCH is the entry point from an aborted CLOAD command
SCRATH: ret     nz                ;[M80] MAKE SURE THERE IS A TERMINATOR
;;Execute NEW Command
SCRTCH: rst     HOOKDO            ;Call Hook Dispatch Routine
        byte    12                ;
        ld      hl,(TXTTAB)       ;[M80] GET POINTER TO START OF TEXT
        xor     a                 ;[M80] SET [A]=0
        ld      (hl),a            ;[M80] SAVE AT END OFF TEXT
        inc     hl                ;[M80] BUMP POINTER
        ld      (hl),a            ;[M80] SAVE ZERO
        inc     hl                ;[M80] BUMP POINTER
        ld      (VARTAB),hl       ;[M80] NEW START OF VARIABLES
;;Clear Variablea, Reset Stack, and Reset Text Pointer
RUNC:   ld      hl,(TXTTAB)     ;[M80] POINT AT THE START OF TEXT
        dec     hl              ;
;[M80] CLEARC IS A SUBROUTINE WHICH INITIALIZES THE VARIABLE AND
CLEARC: ld      (SAVTXT),hl       ;
        ld      hl,(MEMSIZ)       ;[M65] FREE UP STRING SPACE
        ld      (FRETOP),hl       ;
        xor     a                 ;
        call    RESTOR            ;[M65] RESTOR DATA
        ld      hl,(VARTAB)       ;[M65] LIBERATE THE
        ld      (ARYTAB),hl       ;[M65] VARIABLES AND
        ld      (STREND),hl       ;[M65] ARRAYS
;; Reset Stack Pointer
STKINI: pop     bc                ;[M80] GET RETURN ADDRESS HERE
        ld      hl,(TOPMEM)       ;
        ld      sp,hl             ;[M80] INITIALIZE STACK
        call    STKSAV            ;[M80] MAKE SURE SAVSTK OK JUST IN CASE.
        ld      (TEMPPT),hl       ;
        call    FINLPT            ;{M80] BACK TO NORMAL PRINT MODE
        xor     a                 ;[M80] ZERO OUT A
        ld      l,a               ;[M80] ZERO OUT H
        ld      h,a               ;[M80] ZERO OUT L
        ld      (OLDTXT),hl       ;
        ld      (SUBFLG),a        ;[M80] ALLOW SUBSCRIPTS
        ld      (VARNAM),hl       ;
        push    hl                ;[M80] PUT ZERO (NON $FOR,$GOSUB) ON THE STACK
        push    bc                ;[M80] PUT RETURN ADDRESS BACK ON
GTMPRT: ld      hl,(SAVTXT)       ;[M80] GET SAVED [H,L]
        ret
;;The RESTORE Command
RESTOR: ex      de,hl             ;[M80] SAVE [H,L] IN [D,E]
        ld      hl,(TXTTAB)       ;
        jr      z,BGNRST          ;[M80] RESTORE DATA POINTER TO BEGINNING OF PROGRAM
        ex      de,hl             ;[M80] TEXT POINTER BACK TO [H,L]
        call    SCNLIN            ;[M80] GET THE FOLLOWING LINE NUMBER
        push    hl                ;[M80] SAVE TEXT POINTER
        call    FNDLIN            ;[M80] FIND THE LINE NUMBER
        ld      h,b               ;[M80] GET POINTER TO LINE IN [H,L]
        ld      l,c               ;
        pop     de                ;[M80] TEXT POINTER BACK TO [D,E]
        jp      nc,USERR          ;[M80] SHOULD HAVE FOUND LINE
BGNRST: dec     hl                ;[M80] INITIALIZE DATPTR TO [TXTTAB]-1
RESFIN: ld      (DATPTR),hl       ;[M80] READ FINISHES COME TO RESFIN
        ex      de,hl             ;[M80] GET THE TEXT POINTER BACK
        ret
;;The STOP and END Statements
;;STOPC is entry point to END from WARMST
STOP:   ret     nz                ;[M80] MAKE SURE "STOP" STATEMENTS HAVE A TERMINATOR
STOPC:  byte    $F6               ;;"OR" to skip next instruction
ENDS:   ret     nz                ;[M80] MAKE SURE "END" STATEMENTS HAVE A TERMINATOR
        ld      (SAVTXT),hl       ;
        byte    $21               ;{M80} SKIP OVER OR USING "LD H,"
STPEND: or      $FF               ;[M80] SET NON-ZERO TO FORCE PRINTING OF BREAK MESSAGE
        pop     bc                ;[M80] POP OFF NEWSTT ADDRESS
ENDCON: ld      hl,(CURLIN)       ;[M80] SAVE CURLIN
        push    af                ;{M80} SAVE MESSAGE FLAG, ZERO MEANS DON'T PRINT "BREAK"
        ld      a,l               ;
        and     h                 ;[M80] SEE IF DIRECT
        inc     a                 ;
        jr      z,DIRIS           ;[M80] IF NOT SET UP FOR CONTINUE
        ld      (OLDLIN),hl       ;[M80] SAVE OLD LINE #
        ld      hl,(SAVTXT)       ;[M80] GET POINTER TO START OF STATEMENT
        ld      (OLDTXT),hl       ;[M80] SAVE IT
DIRIS:  call    FINLPT            ;{M80} BACK TO NORMAL PRINT MODE
        call    CRDONZ            ;[M80] PRINT CR IF TTYPOS .NE. 0
        pop     af                ;[M80] GET BACK ^C FLAG
        ld      hl,BRKTXT         ;[M80] "BREAK"
        jp      nz,ERRFN1         ;[M80] CALL STROUT AND FALL INTO READY
        jp      READY             ;
;;The CONT Command
CONT:   ld      hl,(OLDTXT)       ;[M80] ZERO INDICATES THERE IS NOTHING TO CONTINUE
        ld      a,h               ;[M80] "STOP","END",TYPING CRLF
        or      l                 ;[M80] TO "INPUT" AND ^C SETUP OLDTXT
        ld      de,ERRCN          ;[M80] "CAN'T CONTINUE"
        jp      z,ERROR           ;
        ld      de,(OLDLIN)       ;
        ld      (CURLIN),de       ;[M80] SET UP OLD LINE # AS CURRENT LINE #
        ret                       ;
;;This looks like orphan code
        jp      FCERR
;;CSAVE* and CLOAD*
CSARY:  byte    $3E               ;;"LD A," sets A = 62 for CSAVE*
CLARY:  xor     a                 ;;Set A = 0 for CLOAD*
        or      a                 ;;Set flags
        push    af                ;;and save them
        rst     CHRGET            ;;Skip '*'
        ld      a,1               ;
        ld      (SUBFLG),a        ;;Don't look for '(' after variable name
        call    PTRGET            ;;Get pointer to variable
        jp      nz,FCERR          ;;Doesn't exist? FC Error
        ld      (SUBFLG),a        ;;Turn subscripts back on
        call    CHKNUM            ;;Must be numeric variable or TM Error
        pop     af                ;
        push    hl                ;;Text Pointer
        push    af                ;;CSAVE/CLOAD flag
        push    bc                ;;Pointer to Number of Dimensions
        ld      b,'#'             ;
        jr      z,CLARYP          ;;If flag is 0, do CLOAD
;;Write Array Header to Tape
        call    PRECRD            ;;"Press <RECORD>" and wait for RETURN
        call    WRSYNC            ;;Write SYNC to tape
        ld      a,b               ;;Write filename "######"
        call    WRBYT2            ;
        call    WRBYT2            ;
        call    WRBYT2            ;
        jr      RWARY             ;;Write Array
;;Read Array Header from Tape
CLARYP: call    PPLAY             ;;"Press <PLAY>", wait for RETURN
        call    RDSYNC            ;;Wait for SYNC
CLARYF: ld      c,6               ;;Look for filename "@@@@@@"
CLARYL: call    RDBYTE
        cp      b
        jr      nz,CLARYF
        dec     c
        jr      nz,CLARYL
;;Read Array from or Write Array to Tape
;;DE=End of Array+1, Stack: Array Pointer, CSAVE Flag, Text Pointer
RWARY:  pop     hl                ;;Restore Array Pointer
        ex      de,hl             ;;DE = Array Pointer, HL = Array Length
        add     hl,de             ;;HL = End of Array plus 1
        ex      de,hl             ;;HL = Array Pointer, DE = End of Array
        ld      c,(hl)            ;
        ld      b,0               ;;BC = Number of Dimensions
        add     hl,bc             ;;Each dimension size is two bytes
        add     hl,bc             ;;So add twice
        inc     hl                ;;Add one more for Number of Dimensions
;;HL=Start Address, DE=End Address+1, Stack: CSAVE Flag Z, Text Pointer
RWMEM:  rst     COMPAR            ;;Are we there yet?
        jr      z,RWARYD          ;;Yes, you can get out
        pop     af                ;;Get CSAVE/CLOAD flag
        push    af                ;;And save it again
        ld      a,(hl)            ;;Read byte from array
        call    nz,WRBYTE         ;;If CSAVE* write to tape
        call    z,RDBYTE          ;;If CLOAD* read from tape
        ld      (hl),a            ;;Write byte into array
        inc     hl                ;;Bump pointer
        jr      RWMEM             ;;and do next byte
RWARYD: pop     af                ;;Get CSAVE/CLOAD flag
        jp      nz,WRTAIL         ;;If CSAVE* write trailer and return
        pop     hl                ;;Restore text pointer
        jp      RWARYR            ;;and Return
;
;[M80] TEST FOR A LETTER / CARRY ON=NOT A LETTER
ISLET:  ld      a,(hl)
;;Test if A is letter
ISLETC: cp      'A'
        ret     c               ;[M80] IF LESS THAN "A", RETURN EARLY
        cp      'Z'+1
        ccf
        ret
;
;[M80] THIS CODE IS FOR THE "CLEAR" COMMAND WITH AN ARGUMENT
CLEAR:  rst     HOOKDO            ;;Call Hook Dispatch Routine
        byte    11                ;
        jp      z,CLEARC          ;[M80] IF NO FORMULA JUST CLEAR
        call    INTID2            ;[M80] GET AN INTEGER INTO [D,E]
        dec     hl                ;;Back up text pointer
        rst     CHRGET            ;[M80] SEE IF ITS THE END
        push    hl                ;;Save text pointer
        ld      hl,(MEMSIZ)       ;[M80] GET HIGHEST ADDRESS
        jr      z,CLEARS          ;[M80] SHOULD FINISH THERE
        pop     hl                ;;Restore Text Pointer
        rst     SYNCHK            ;;Require a Comma
        byte    ','               ;
        push    de                ;;Save String Size
        call    INTID2            ;;Get Top of Memory
        dec     hl                ;;Back up text pointer
        rst     CHRGET            ;;Check next character
        jp      nz,SNERR          ;{M80} IF NOT TERMINATOR, GOOD BYE
        ex      (sp),hl           ;;Get String Size, Save Text Pointer
        ex      de,hl             ;;DE=String Size, HL=Top of Memory
;;Set VARTAB, TOPMEM, and MEMSIZ
;;On Entry HL = top of memory, from MEMSIZ or second parameter
;;         DE = string space, from the first parameter
CLEARS: ld      a,l               ;[M80] SUBTRACT [H,L]-[D,E] INTO [D,E]
        sub     e                 ;
        ld      e,a               ;;Leaving start of String Space in [D,E]
        ld      a,h               ;
        sbc     a,d               ;
        ld      d,a               ;
        jp      c,OMERR           ;[M80] WANTED MORE THAN TOTAL!
        push    hl                ;[M80] SAVE MEMSIZ
        ld      hl,(VARTAB)       ;[M80] TOP LOCATION IN USE
        ld      bc,40             ;[M80] TOP LOCATION IN USE
        add     hl,bc             ;[M80] LEAVE BREATHING ROOM
        rst     COMPAR            ;[M80] ROOM?
        jp      nc,OMERR          ;[M80] NO, DON'T EVEN CLEAR
        ex      de,hl             ;[M80] NEW STACK LOCATION [H,L]
        ld      (TOPMEM),hl       ;[M80] SET UP NEW STACK LOCATION
        pop     hl                ;[M80] GET BACK MEMSIZ
        ld      (MEMSIZ),hl       ;[M80] SET IT UP, MUST BE OK
        pop     hl                ;[M80] REGAIN THE TEXT POINTER
        jp      CLEARC            ;[M80] GO CLEAR
;;;Code Change: Patch to make room in RND routine - 7 bytes                   Original Code
RNDSTL: ld      b,0               ;;                                          0D0C  ld      a,l 
                                  ;;                                          0D0D  sub     e
        ld      (hl),a            ;;Store it back in RNDCNT+1                 0D0E  ld      e,a
        ld      hl,RNDTBL         ;;Now HL points to Table in ROM             0D0F  ld      a,h
                                  ;;                                          0D10  sbc     a,d
                                  ;;                                          0D11  ld      d,a
        ret                                                               
;;The NEXT STATEMENT
;;See FOR for description of the stack entry
NEXT:   ld      de,0              ;{M80} FOR "NEXT" WITHOUT ARGS CALL FNDFOR WITH [D,E]=0
NEXTC:  call    nz,PTRGET         ;{M80} GET POINTER TO LOOP VARIABLE INTO [D,E]
        ld      (SAVTXT),hl       ;{M80} SAVE TEXT POINTER IN CASE LOOP TERMINATES
        call    FNDFOR            ;{M80} LOOK FOR ENTRY WHOSE VARIABLE NAME MATCHES THIS ONES
        jp      nz,NFERR          ;[M80] "NEXT WITHOUT FOR"
        ld      sp,hl             ;[M80] SETUP STACK POINTER BY CHOPPING AT THIS POINT
        push    de                ;[M80] PUT THE VARIABLE PTR BACK ON
        ld      a,(hl)            ;
        push    af                ;
        inc     hl                ;
        push    de                ;{M80} PUT POINTER TO LOOP VARIABLE ONTO STACK
        call    MOVFM             ;[M80] STEP VALUE INTO THE FAC
        ex      (sp),hl           ;{M80} PUT POINTER INTO FOR ENTRY ONTO STACK
        push    hl                ;{M80} PUT POINTER TO LOOP VARIABLE BACK ONTO STACK
        call    FADDS             ;
        pop     hl                ;{M80} POP OFF POINTER TO LOOP VARIABLE
        call    MOVMF             ;[M80] MOV FAC INTO LOOP VARIABLE
        pop     hl                ;[M80] GET THE ENTRY POINTER
        call    MOVRM             ;[M80] GET THE FINAL INTO THE REGISTERS
        push    hl                ;[M80] SAVE THE ENTRY POINTER
        call    FCOMP             ;{M80} RETURN 255 IF FAC < REGISTERS, 0 IF =, 1 IF >
        pop     hl                ;{M80} POP OFF "FOR" POINTER NOW POINTING PAST FINAL VALUE
        pop     bc                ;[M80] GET THE SIGN OF THE INCREMENT
        sub     b                 ;{M80} SUBTRACT SIGN FROM (CURRENT VALUE-FINAL VALUE)
        call    MOVRM             ;{M80} "FOR" LINE # INTO [D,E], TEXT POINTER INTO [B,C]
        jr      z,LOOPDN          ;{M80} IF ZERO THEN THE LOOP IS FINISHED
        ex      de,hl             ;
        ld      (CURLIN),hl       ;[M80] STORE THE LINE #
        ld      l,c               ;[M80] SETUP THE TEXT POINTER
        ld      h,b               ;
        jp      NXTCON            ;
LOOPDN: ld      sp,hl             ;{M80} ELIMINATE FOR ENTRY SINCE [H,L] MOVED ALL THE WAY THE ENTRY
        ld      hl,(SAVTXT)       ;UPDATE SAVED STACK
        ld      a,(hl)            ;IS THERE A COMMA AT THE END
        cp      ','               ;IF SO LOOK AT ANOTHER
        jp      nz,NEWSTT         ;VARIABLE NAME TO "NEXT"
        rst     CHRGET            ;READ FIRST CHARCTER
        call    NEXTC             ;DO NEXT, BUT DON'T ALLOW BLANK VARIABLE NAME
;[M80] THIS IS THE LINE INPUT ROUTINE IT READS CHARACTERS INTO BUF
;[M80] THE ROUTINE IS ENTERED AT INLIN, AT QINLIN TO TYPE A QUESTION MARK
;[M80] AND A SPACE FIRST
QINLIN: ld      a,'?'             ;
        rst     OUTCHR            ;
        ld      a,' '             ;
        rst     OUTCHR            ;
        jp      INLIN             ;;;For relative jumps
RUBOUT: ld      a,(RUBSW)         ;[M80] ARE WE ALREADY RUBBING OUT?
        or      a                 ;[M80] SET CC'S
        ld      a,'\'             ;[M80] GET READY TO TYPE BACKSLASH
        ld      (RUBSW),a         ;[M80] MAKE RUBSW NON-ZERO IF NOT ALREADY
        jr      nz,NOTBEG         ;[M80] NOT RUBBING BACK TO BEGGINING
        dec     b                 ;[M80] AT BEGINNING OF LINE?
        jr      z,INLIN           ;[M80] SET FIRST BYTE IN BUF TO ZERO
        rst     OUTCHR            ;[M80] SEND BACKSLASH
        inc     b                 ;[M80] EFFECTIVELY SKIP NEXT INSTRUCTION
NOTBEG: dec     b                 ;[M80] BACK UP CHAR COUNT BY 1
        dec     hl                ;[M80] AND LINE POSIT
        jr      z,INLINN          ;[M80] AND RE-SET UP INPUT
        ld      a,(hl)            ;[M80] OTHERWISE GET CHAR TO ECHO
        rst     OUTCHR            ;[M80] SEND IT
        jr      INLINC            ;[M80] AND GET NEXT CHAR
LINLIN: dec     b                 ;[M80] AT START OF LINE?
        dec     hl                ;[M65] BACKARROW SO BACKUP PNTR AND
        rst     OUTCHR            ;[M80] SEND BACKSPACE
        jr      nz,INLINC         ;
INLINN: rst     OUTCHR            ;
INLINU: call    CRDO              ;[M80] TYPE A CRLF
INLIN:  ld      hl,BUF            ;
        ld      b,1               ;[M80] CHARACTER COUNT
        xor     a                 ;[M80] CLEAR TYPE AHEAD CHAR
        ld      (RUBSW),a         ;[M80] LIKE SO
INLINC: call    INCHR             ;[M80] GET A CHAR
        ld      c,a               ;[M80] SAVE CURRENT CHAR IN [C]
        cp      127               ;[M80] CHARACTER DELETE?
        jr      z,RUBOUT          ;[M80] DO IT
        ld      a,(RUBSW)         ;[M80] BEEN DOING A RUBOUT?
        or      a                 ;[M80] SET CC'S
        jr      z,NOTRUB          ;[M80] NOPE.
        ld      a,'\'             ;[M80] GET READY TO TYPE SLASH
        rst     OUTCHR            ;[M80] SEND IT
        xor     a                 ;[M80] CLEAR RUBSW
        ld      (RUBSW),a         ;[M80] LIKE SO
NOTRUB: ld      a,c               ;[M80] GET BACK CURRENT CHAR
        cp      7                 ;[M80] IS IT BOB ALBRECHT RINGING THE BELL
        jr      z,GOODCH          ;[M80] FOR SCHOOL KIDS?
        cp      3                 ;[M80] CONTROL-C?
        call    z,CRDO            ;[M80] TYPE CHAR, AND CRLFT
        scf                       ;[M80] RETURN WITH CARRY ON
        ret     z                 ;[M80] IF IT WAS CONTROL-C
        cp      13                ;
        jp      z,FININL          ;[M80] IS IT A CARRIAGE RETURN?
        cp      21                ;[M80] ;LINE DELETE? (CONTROL-U)
        jp      z,INLINU          ;[M80] GO DO IT
        nop                       ;;;Orphan Code: Move this down to reuse - 5 bytes
        nop                       ;;;Whatever was removed isn't in the
        nop                       ;;;available source codes
        nop                       ;
        nop                       ;
        cp      8                 ;[M80] BACKSPACE? (CONTROL-H)?
        jp      z,LINLIN          ;[M65] YES
        cp      24                ;[M80] AT START OF LINE?
        jr      nz,NTCTLX         ;[M80] IS IT CONTROL-X (LINE DELETE)
        ld      a,'#'             ;[M80] SEND NUMBER SIGN
        jp      INLINN            ;[M80] SEND # SIGN AND ECHO
NTCTLX: cp      18                ;[M80] CONTROL-R?
        jr      nz,NTCTLR         ;[M80] NO
        push    bc                ;[M80] SAVE [B,C]
        push    de                ;[M80] SAVE [D,E]
        push    hl                ;[M80] SAVE [H,L]
        ld      (hl),0            ;[M80] STORE TERMINATOR
        call    CRDO              ;[M80] DO CRLF
        ld      hl,BUF            ;[M80] POINT TO START OF BUFFER
        call    STROUT            ;;Print It
        pop     hl                ;[M80] RESTORE [H,L]
        pop     de                ;[M80] RESTORE [D,E]
        pop     bc                ;[M80] RESTORE [B,C]
        jp      INLINC            ;[M80] GET NEXT CHAR
NTCTLR: cp      ' '               ;[M80] CHECK FOR FUNNY CHARACTERS
        jp      c,INLINC          ;
GOODCH: ld      a,b               ;[M80] GET CURRENT LENGTH
        cp      ENDBUF-BUF        ;[M80] ;Set Carry if longer than Buffer
        ld      a,7               ;[M80] GET BELL CHAR
        jp      nc,OUTBEL         ;[M80] NO CAUSE FOR BELL
        ld      a,c               ;[M80] RESTORE  CURRENT CHARACTER INTO [A]
        ld      (hl),c            ;[M80] STORE THIS CHARACTER
        ld      (USFLG),a         ;[M80] FLAG THAT VALUE HAS BEEN PRINTED
        inc     hl                ;[M80] INCREMENT CHARACTER COUNT
        inc     b                 ;[M80] BUMP POINTER INTO BUF
OUTBEL: rst     OUTCHR            ;[M80] SEND THE CHAR
        jp      INLINC            ;{M80} GET NEXT CHAR
;[M80] THE FOLLOWING ROUTINE COMPARES TWO STRINGS
STRCMP: push    de
        call    FREFAC            ;[M80] FREE UP FAC STRING, GET POINTER TO DESCRIPTOR IN [H,L]
        ld      a,(hl)            ;[M80] SAVE THE LENGTH OF THE FAC STRING IN [A]
        inc     hl                ;
        inc     hl                ;
        ld      c,(hl)            ;[M80] SAVE THE POINTER AT THE FAC STRING DATA IN [B,C]
        inc     hl                ;
        ld      b,(hl)            ;
        pop     de                ;[M80] GET THE STACK STRING POINTER
        push    bc                ;[M80] SAVE THE POINTER AT THE FAC STRING DATA
        push    af                ;[M80] SAVE THE FAC STRING LENGTH
        call    FRETMP            ;[M80] FREE UP STACK STRING, RETURN POINTER TO DESCRIPTOR IN [H,L]
        call    MOVRM             ;
        pop     af                ;
        ld      d,a               ;
        pop     hl                ;[M80] GET BACK 2ND CHARACTER POINTER
CSLOOP: ld      a,e               ;[M80] BOTH STRINGS ENDED
        or      d                 ;[M80] TEST BY OR'ING THE LENGTHS TOGETHER
        ret     z                 ;[M80] IF SO, RETURN WITH A ZERO
        ld      a,d               ;[M80] GET FACLO STRING LENGTH
        sub     1                 ;[M80] SET CARRY AND MAKE [A]=255 IF [D]=0
        ret     c                 ;[M80] RETURN IF THAT STRING ENDED
        xor     a                 ;[M80] MUST NOT HAVE BEEN ZERO, TEST CASE
        cp      e                 ;[M80] OF B,C,D,E STRING HAVING ENDED FIRST
        inc     a                 ;[M80] RETURN WITH A=1
        ret     nc                ;[M80] TEST THE CONDITION
;[M80] HERE WHEN NEITHER STRING ENDED
        dec     d                 ;[M80] DECREMENT BOTH CHARACTER COUNTS
        dec     e                 ;
        ld      a,(bc)            ;[M80] GET CHARACTER FROM B,C,D,E STRING
        inc     bc                ;
        cp      (hl)              ;[M80] COMPARE WITH FACLO STRING
        inc     hl                ;[M80] BUMP POINTERS (INX DOESNT CLOBBER CC'S)
        jr      z,CSLOOP          ;[M80] IF BOTH THE SAME, MUST BE MORE TO STRINGS
        ccf                       ;[M80] HERE WHEN STRINGS DIFFER
        jp      SIGNS             ;[M80] SET [A] ACCORDING TO CARRY
;;CONVERT NUMBER TO STRING
STR:    call    CHKNUM            ;[M80] IS A NUMERIC
        call    FOUT              ;[M80] DO ITS OUTPUT
        call    STRLIT            ;[M80] SCAN IT AND TURN IT INTO A STRING
        call    FREFAC            ;[M80] FREE UP THE TEMP
        ld      bc,FINBCK         ;
        push    bc                ;[M80] SET UP ANSWER IN NEW TEMP
;;COPY A STRING
STRCPY: ld      a,(hl)            ;[M80] GET LENGTH
        inc     hl                ;[M80] MOVE UP TO THE POINTER
        inc     hl                ;[M80] GET POINTER TO POINTER OF ARG
        push    hl                ;[M80] GET THE SPACE
        call    GETSPA            ;[M80] FIND OUT WHERE STRING TO COPY
        pop     hl                ;
        ld      c,(hl)            ;
        inc     hl                ;
        ld      b,(hl)            ;
        call    STRAD2            ;[M80] SETUP DSCTMP
        push    hl                ;[M80] SAVE POINTER TO DSCTMP
        ld      l,a               ;[M80] GET CHARACTER COUNT INTO [L]
        call    MOVSTR            ;[M80] MOVE THE CHARS IN
        pop     de                ;[M80] RESTORE POINTER TO DSCTMP
        ret                       ;[M80] RETURN
;[M65] "STRINI" GET STRING SPACE FOR THE CREATION OF A STRING AND
;[M65] CREATES A DESCRIPTOR FOR IT IN "DSCTMP".
;;Returns String Text Address in [DE], Descriptor Address in [HL]
STRIN1: ld      a,1               ;[M80] MAKE ONE CHAR STRING (CHR$, INKEY$)
STRINI: call    GETSPA            ;[M80] GET SOME STRING SPACE ([A] CHARS)
STRAD2: ld      hl,DSCTMP         ;[M80] GET DESC. TEMP
        push    hl                ;[M80] SAVE DESC. POINTER
        ld      (hl),a            ;[M80] SAVE CHARACTER COUNT
        inc     hl                ;
STRADX: inc     hl                ;[M80] STORE [D,E]=POINTER TO FREE SPACE
        ld      (hl),e            ;
        inc     hl                ;
        ld      (hl),d            ;
        pop     hl                ;[M80] AND RESTORE [H,L] AS THE DESCRIPTOR POINTER
        ret                       ;
;;Build Descriptor for String Litersl
STRLIT: dec     hl                ;;Back up to '"'
STRLTI: ld      b,'"'             ;[M80] ASSUME STR ENDS ON QUOTE
        ld      d,b               ;
STRLT2: push    hl                ;[M80] SAVE POINTER TO START OF LITERAL
        ld      c,255             ;[M80] INITIALIZE CHARACTER COUNT
STRGET: inc     hl                ;;Move past '"'
        ld      a,(hl)            ;[M80] GET CHAR
        inc     c                 ;[M80] BUMP CHARACTER COUNT
        or      a                 ;[M80] IF 0, (END OF LINE) DONE
        jr      z,STRFIN          ;[M80] TEST
        cp      d                 ;
        jr      z,STRFIN          ;
        cp      b                 ;[M80] CLOSING QUOTE
        jr      nz,STRGET         ;[M80] NO, GO BACK FOR MORE
STRFIN: cp      '"'               ;[M80] IF QUOTE TERMINATES THE STRING
        call    z,CHRGTR          ;[M80] SKIP OVER THE QUOTE
        ex      (sp),hl           ;[M80] SAVE POINTER AT END OF STRING
        inc     hl                ;
        ex      de,hl             ;[M80] GET POINTER TO TEMP
        ld      a,c               ;[M80] GET CHARACTER COUNT IN A
        call    STRAD2            ;[M80] SAVE STR INFO
;;Set Pointer to Temporary String Descriptor
PUTNEW: ld      de,DSCTMP         ;[M80] [D,E] POINT AT RESULT DESCRIPTOR
        ld      hl,(TEMPPT)       ;[M80] [H,L]=POINTER TO FIRST FREE TEMP
        ld      (FACLO),hl        ;[M80] POINTER AT WHERE RESULT DESCRIPTOR WILL BE
        ld      a,1               ;
        ld      (VALTYP),a        ;[M80] FLAG THIS AS A STRING
        call    MOVE              ;[M80] AND MOVE THE VALUE INTO A TEMPORARY
        rst     COMPAR            ;;IF TEMPPT POINTS TO DSCTMP, THERE ARE NO FREE TEMPS
        ld      (TEMPPT),hl       ;[M80] SAVE NEW TEMPORARY POINTER
        pop     hl                ;[M80] GET THE TEXT POINTER
        ld      a,(hl)            ;[M80] GET CURRENT CHARACTER INTO [A]
        ret     nz                ;
        ld      de,ERRST          ;[M80] "STRING TEMPORARY" ERROR
        jp      ERROR             ;[M80] GO TELL HIM
;;Output String
STROUI: inc     hl                ;[M80] POINT AT NEXT CHARACTER
STROUT: call    STRLIT            ;[M80] GET A STRING LITERAL
; PRINT THE STRING WHOSE DESCRIPTOR IS POINTED TO BY FACLO.
STRPRT: call    FREFAC            ;[M80] RETURN TEMP POINTER BY FACLO
        call    MOVRM             ;[M80] [D]=LENGTH [B,C]=POINTER AT DATA
        inc     e                 ;[M80] CHECK FOR NULL STRING
STRPR2: dec     e                 ;[M80] DECREMENT THE LENGTH
        ret     z                 ;[M80] ALL DONE
        ld      a,(bc)            ;[M80] GET CHARACTER TO PRINT
        rst     OUTCHR            ;
        cp      13                ;
        call    z,CRFIN           ;[M65] TYPE REST OF CARRIAGE RETURN
        inc     bc                ;[M80] POINT TO THE NEXT CHARACTER
        jr      STRPR2            ;[M80] AND PRINT IT...
;[M80] GETSPA - GET SPACE FOR CHARACTER STRING
GETSPA: or      a                 ;[M80] MUST BE NON ZERO. SIGNAL NO GARBAG YET
        byte    $0E               ;[M80] "MVI C" AROUND THE NEXT BYTE
TRYGI2: pop     af                ;[M80] IN CASE COLLECTED WHAT WAS LENGTH?
        push    af                ;[M80] SAVE IT BACK
        ld      hl,(TOPMEM)       ;
        ex      de,hl             ;[M80] IN [D,E]
        ld      hl,(FRETOP)       ;[M80] GET TOP OF FREE SPACE IN [H,L]
        cpl                       ;[M80] -# OF CHARS
        ld      c,a               ;[M80] IN [B,C]
        ld      b,$FF             ;
        add     hl,bc             ;[M80] SUBTRACT FROM TOP OF FREE
        inc     hl                ;
        rst     COMPAR            ;[M80] COMPARE THE TWO
        jr      c,GARBAG          ;[M80] NOT ENOUGH ROOM FOR STRING, OFFAL TIME
        ld      (FRETOP),hl       ;[M80] SAVE NEW BOTTOM OF MEMORY
        inc     hl                ;[M80] MOVE BACK TO POINT TO STRING
        ex      de,hl             ;[M80] RETURN WITH POINTER IN [D,E]
        pop     af                ;
        ret                       ;
;Garbage Collector - Removes Orphaned Strings
GARBAG: pop     af                ;[M80] HAVE WE COLLECTED BEFORE?
        ld      de,ERRSO          ;[M80] GET READY FOR OUT OF STRING SPACE ERROR
        jp      z,ERROR           ;[M80] GO TELL USER HE LOST
        cp      a                 ;[M80] SET ZERO FLAG TO SAY WEVE GARBAGED
        push    af                ;[M80] SAVE FLAG BACK ON STACK
        ld      bc,TRYGI2         ;[M80] PLACE FOR GARBAG TO RETURN TO.
        push    bc                ;[M80] SAVE ON STACK
GARBA2: ld      hl,(MEMSIZ)       ;[M80] START FROM TOP DOWN
FNDVAR: ld      (FRETOP),hl       ;[M80] LIKE SO
        ld      hl,0              ;[M80] GET DOUBLE ZERO
        push    hl                ;[M80] SAY DIDNT SEE VARS THIS PASS
        ld      hl,(STREND)       ;[M80] FORCE DVARS TO IGNORE STRINGS IN PROGRAM TEXT
        push    hl                ;[M80] FORCE FIND HIGH ADDRESS
        ld      hl,TEMPST         ;[M80] GET START OF STRING TEMPS
TVAR:   ld      de,(TEMPPT)       ;[M80] SEE IF DONE
        rst     COMPAR            ;[M80] TEST
        ld      bc,TVAR           ;[M80] FORCE JUMP TO TVAR
        jp      nz,DVAR2          ;[M80] DO TEMP VAR GARBAGE COLLECT
        ld      hl,(VARTAB)       ;[M80] GET STARTING POINT IN [H,L]
SVAR:   ld      de,(ARYTAB)       ;[M80] GET STOPPING LOCATION
        rst     COMPAR            ;[M80] SEE IF AT END OF SIMPS
        jr      z,ARYVA4          ;
        inc     hl                ;{M80} BUMP POINTER
        ld      a,(hl)            ;[M80] GET VALTYP
        inc     hl                ;[M80] POINT AT THE VALUE
        or      a                 ;
        call    DVARS             ;
        jr      SVAR              ;
;;;Subsection of [M80] ARYVAR
ARYVA2: pop     bc                ;[M80] GET RID OF STACK GARBAGE
ARYVA4: ld      de,(STREND)       ;[M80] GET RID OF STACK GARBAGE
        rst     COMPAR            ;[M80] SEE IF DONE WITH ARRAYS
        jp      z,GRBPAS          ;[M80] YES, SEE IF DONE COLLECTING
        call    MOVRM             ;
        ld      a,d               ;
        push    hl                ;
        add     hl,bc             ;[M80] ADDING BASE TO LENGTH
        or      a                 ;[M80]
        jp      p,ARYVA2          ;;Loop
        ld      (TEMP8),hl        ;[M80] SAVE END OF ARRAY
        pop     hl                ;[M80] GET BACK CURRENT POSITION
        ld      c,(hl)            ;[M80] PICK UP NUMBER OF DIMS
        ld      b,0               ;[M80] MAKE DOUBLE WITH HIGH ZERO
        add     hl,bc             ;[M80] GO PAST DIMS
        add     hl,bc             ;[M80] BY ADDING ON TWICE #DIMS (2 BYTE GUYS)
        inc     hl                ;[M80] ONE MORE TO ACCOUNT FOR #DIMS.
ARYSTR: ex      de,hl             ;[M80] SAVE CURRENT POSIT IN [D,E]
        ld      hl,(TEMP8)        ;[M80] GET END OF ARRAY
        ex      de,hl             ;[M80] FIX [H,L] BACK TO CURRENT
        rst     COMPAR            ;[M80] SEE IF AT END OF ARRAY
        jr      z,ARYVA4          ;[M80] END OF ARRAY, TRY NEXT ARRAY
        ld      bc,ARYSTR         ;[M80] ADDR OF WHERE TO RETURN TO
DVAR2:  push    bc                ;[M80] GOES ON STACK
        or      $80               ;;Set Flags
DVARS:  ld      a,(hl)            ;[M80] GET VALTYP
        inc     hl                ;[M80] BUMP POINTER TWICE
        inc     hl                ;
        ld      e,(hl)            ;[M80] [D,E]=AMOUNT TO SKIP
        inc     hl                ;
        ld      d,(hl)            ;
        inc     hl                ;
        ret     p                 ;
        or      a                 ;
        ret     z                 ;[M80] NULL STRING, RETURN
        ld      b,h               ;[M80] MOVE [B,C] BACK TO [H,L]
        ld      c,l               ;
        ld      hl,(FRETOP)       ;[M80] GET POINTER TO TOP OF STRING FREE SPACE
        rst     COMPAR            ;[M80] IS THIS STRINGS POINTER .LT. FRETOP
        ld      h,b               ;[M80] MOVE [B,C] BACK TO [H,L]
        ld      l,c               ;
        ret     c                 ;[M80] IF NOT, NO NEED TO MESS WITH IT FURTHUR
        pop     hl                ;[M80] GET RETURN ADDRESS OFF STACK
        ex      (sp),hl           ;[M80] GET MAX SEEN SO FAR & SAVE RETURN ADDRESS
        rst     COMPAR            ;[M80] LETS SEE
        ex      (sp),hl           ;[M80] SAVE MAX SEEN & GET RETURN ADDRESS OFF STACK
        push    hl                ;[M80] SAVE RETURN ADDRESS BACK
        ld      h,b               ;[M80] MOVE [B,C] BACK TO [H,L]
        ld      l,c               ;[
        ret     nc                ;[M80] IF NOT, LETS LOOK AT NEXT VAR
        pop     bc                ;[M80] GET RETURN ADDR OFF STACK
        pop     af                ;[M80] POP OFF MAX SEEN
        pop     af                ;[M80] AND VARIABLE POINTER
        push    hl                ;[M80] SAVE NEW VARIABLE POINTER
        push    de                ;[M80] AND NEW MAX POINTER
        push    bc                ;[M80] SAVE RETURN ADDRESS BACK
        ret                       ;[M80] AND RETURN
;[M80] HERE WHEN MADE ONE COMPLETE PASS THRU STRING VARS
GRBPAS: pop     de                ;[M80] POP OFF MAX POINTER
        pop     hl                ;[M80] AND GET VARIABLE POINTER
        ld      a,h               ;[M80] GET LOW IN
        or      l                 ;[M80] SEE IF ZERO POINTER
        ret     z                 ;[M80] IF END OF COLLECTION, THEN MAYBE RETURN TO GETSPA
        dec     hl                ;[M80] CURRENTLY JUST PAST THE DESCRIPTOR
        ld      b,(hl)            ;[M80] [B]=HIGH BYTE OF DATA POINTER
        dec     hl                ;
        ld      c,(hl)            ;[M80] [B,C]=POINTER AT STRING DATA
        push    hl                ;[M80] SAVE LOCATION TO UPDTE POINTER AFTER STRING IS MOVED
        dec     hl                ;
        dec     hl                ;
        ld      l,(hl)            ;[M80] [L]=STRING LENGTH
        ld      h,0               ;[M80] [H,L] GET CHARACTER COUNT
        add     hl,bc             ;[M80] [H,L]=POINTER BEYOND STRING
        ld      d,b               ;
        ld      e,c               ;[M80] [D,E]=ORIGINAL POINTER
        dec     hl                ;[M80] DON'T MOVE ONE BEYOND STRING
        ld      b,h               ;[M80] GET TOP OF STRING IN [B,C]
        ld      c,l               ;
        ld      hl,(FRETOP)       ;[M80] GET TOP OF FREE SPACE
        call    BLTUC             ;[M80] MOVE STRING
        pop     hl                ;[M80] GET BACK POINTER TO DESC.
        ld      (hl),c            ;[M80] SAVE FIXED ADDR
        inc     hl                ;[M80] MOVE POINTER
        ld      (hl),b            ;[M80] HIGH PART
        ld      h,b               ;
        ld      l,c               ;[M80] [H,L]=NEW POINTER
        dec     hl                ;[M80] FIX UP FRETOP
        jp      FNDVAR            ;[M80] AND TRY TO FIND HIGH AGAIN
;
;[M80] STRING CONCATENATION
;[M80] THE FOLLOWING ROUTINE CONCATENATES TWO STRINGS
;[M80] THE FACLO CONTAINS THE FIRST ONE AT THIS POINT,
;[M80] [H,L] POINTS  BEYOND THE + SIGN AFTER IT
;
CAT:    push    bc                ;[M80] PUT OLD PRECEDENCE BACK ON
        push    hl                ;[M80] SAVE TEXT POINTER
        ld      hl,(FACLO)        ;[M80] GET POINTER TO STRING DESC.
        ex      (sp),hl           ;[M80] SAVE ON STACK & GET TEXT POINTER BACK
        call    EVAL              ;[M80] EVALUATE REST OF FORMULA
        ex      (sp),hl           ;[M80] SAVE TEXT POINTER, GET BACK DESC.
        call    CHKSTR            ;
        ld      a,(hl)            ;
        push    hl                ;[M80] SAVE DESC. POINTER.
        ld      hl,(FACLO)        ;[M80] GET POINTER TO 2ND DESC.
        push    hl                ;[M80] SAVE IT
        add     a,(hl)            ;[M80] ADD TWO LENGTHS TOGETHER
        ld      de,ERRLS          ;[M80] SEE IF RESULT .LT. 256
        jp      c,ERROR           ;[M80] ERROR "LONG STRING"
        call    STRINI            ;[M80] GET INITIAL STRING
        pop     de                ;[M80] GET 2ND DESC.
        call    FRETMP            ;
        ex      (sp),hl           ;[M80] SAVE POINTER TO IT
        call    FRETM2            ;[M80] FREE UP 1ST TEMP
        push    hl                ;[M80] SAVE DESC. POINTER (FIRST)
        ld      hl,(DSCTMP+2)     ;[M80] GET POINTER TO FIRST
        ex      de,hl             ;[M80] IN [D,E]
        call    MOVINS            ;[M80] MOVE IN THE FIRST STRING
        call    MOVINS            ;[M80] AND THE SECOND
        ld      hl,TSTOP          ;[M80] CAT REENTERS FORMULA EVALUATION AT TSTOP
        ex      (sp),hl           ;
        push    hl                ;[M80] TEXT POINTER OFF FIRST
        jp      PUTNEW            ;[M80] THEN RETURN ADDRESS OF TSTOP
MOVINS: pop     hl                ;[M80] GET RETURN ADDR
        ex      (sp),hl           ;[M80] PUT BACK, BUT GET DESC.
        ld      a,(hl)            ;[M80] [A]=STRING LENGTH
        inc     hl                ;
        inc     hl                ;
        ld      c,(hl)            ;[M80] [B,C]=POINTER AT STRING DATA
        inc     hl                ;
        ld      b,(hl)            ;
        ld      l,a               ;[M80] [L]=STRING LENGTH
MOVSTR: inc     l                 ;;Copying [L] Bytes from [BC] to [DE]
MOVLP:  dec     l                 ;[M80] SET CC'S
        ret     z                 ;[M80] 0, NO BYTE TO MOVE
        ld      a,(bc)            ;[M80] GET CHAR
        ld      (de),a            ;[M80] SAVE IT
        inc     bc                ;[M80] MOVE POINTERS
        inc     de                ;
        jr      MOVLP             ;[M80] KEEP DOING IT
;[M80] FREE UP STRING TEMPORARY
;[M80] FRESTR, FREFAC, FRETMP, FRETMS
FRESTR: call    CHKSTR            ;[M80] MAKE SURE ITS A STRING
FREFAC: ld      hl,(FACLO)        ;
FRETM2: ex      de,hl             ;[M80] FREE UP THE TEMP IN THE FACLO
FRETMP: call    FRETMS            ;[M80] FREE UP THE TEMPORARY
        ex      de,hl             ;[M80] PUT THE STRING POINTER INTO [H,L]
        ret     nz                ;
        push    de                ;[M80] SAVE [D,E] TO RETURN IN [H,L]
        ld      d,b               ;[M80] [D,E]=POINTER AT STRING
        ld      e,c               ;
        dec     de                ;[M80] SUBTRACT ONE
        ld      c,(hl)            ;[M80] [C]=LENGTH OF THE STRING FREED UP
        ld      hl,(FRETOP)       ;[M80] SEE IF ITS THE FIRST ONE IN STRING SPACE
        rst     COMPAR            ;
        jr      nz,NOTLST         ;[M80] NO SO DON'T ADD
        ld      b,a               ;[M80] MAKE [B]=0
        add     hl,bc             ;[M80] ADD
        ld      (FRETOP),hl       ;[M80] AND UPDATE FRETOP
NOTLST: pop     hl                ;[M80] GET POINTER AT CURRENT DESCRIPTOR
        ret
FRETMS: ld      hl,(TEMPPT)       ;[M80] GET TEMP POINTER
        dec     hl                ;[M80] LOOK AT WHAT IS IN THE LAST TEMP
        ld      b,(hl)            ;[M80] [B,C]=POINTER AT STRING
        dec     hl                ;[M80] DECREMENT TEMPPT BY STRSIZ
        ld      c,(hl)            ;
        dec     hl                ;
        dec     hl                ;
        rst     COMPAR            ;[M80] SEE IF [D,E] POINT AT THE LAST
        ret     nz                ;[M80] RETURN NOW IF NOW FREEING DONE
        ld      (TEMPPT),hl       ;[M80] UPDATE THE TEMP POINTER SINCE
        ret                       ;
;[M80] THE FUNCTION LEN($)
LEN:    ld      bc,SNGFLT         ;[M80] CALL SNGFLT WHEN DONE
        push    bc                ;[M80] LIKE SO
;Return Length of String pointed to by FAC in [A]
LEN1:   call    FRESTR            ;[M80] FREE UP TEMP POINTED TO BY FACLO
        xor     a                 ;[M80] FORCE NUMERIC FLAG
        ld      d,a               ;[M80] SET HIGH OF [D,E] TO ZERO FOR VAL
        ld      (VALTYP),a        ;
        ld      a,(hl)            ;
        or      a                 ;[M80] SET CONDITION CODES ON LENGTH
        ret                       ;[M80] RETURN
;[M80] THE FOLLOWING IS THE ASC($) FUNCTION.
;[M80] IT RETURNS AN INTEGER WHICH IS THE DECIMAL ASCII EQUIVALENT
ASC:    ld      bc,SNGFLT         ;[M80] WHERE TO GO WHEN DONE
        push    bc                ;[M80] SAVE RETURN ADDR ON STACK
;;Return [DE] = Pointer to String Text, [A]=First Character
ASC2:   call    LEN1              ;[M80] SET UP ORIGINAL STR
        jp      z,FCERR           ;[M80] NULL STR, BAD ARG.
        inc     hl                ;[M80] BUMP POINTER
        inc     hl                ;
        ld      e,(hl)            ;[M80] [D,E]=POINTER AT STRING DATA
        inc     hl                ;
        ld      d,(hl)            ;
        ld      a,(de)            ;[M80] [A]=FIRST CHARACTER
        ret                       ;
;;CHR$ Function
CHR:    call    STRIN1            ;[M80] GET STRING IN DSCTMP
        call    CONINT            ;[M80] GET INTEGER IN RANGE
SETSTR: ld      hl,(DSCTMP+2)     ;[M80] GET ADDR OF STR
        ld      (hl),e            ;[M80] SAVE ASCII BYTE
FINBCK: pop     bc                ;[M80] RETURN TO HIGHER LEVEL & SKIP THE CHKNUM CALL
        jp      PUTNEW            ;[M80] GO CALL PUTNEW
;[M80] THE FOLLOWING IS THE LEFT$($,#) FUNCTION.
LEFT:   call    PREAM             ;[M80] TEST THE PARAMETERS
        xor     a                 ;[M80] LEFT NEVER CHANGES STRING POINTER
LEFT3:  ex      (sp),hl           ;[M80] SAVE TEXT POINTER
        ld      c,a               ;[M80] OFFSET NOW IN [C]
LEFT2:  push    hl                ;[M80] SAVE DESC. FOR  FRETMP
        ld      a,(hl)            ;[M80] GET STRING LENGTH
        cp      b                 ;[M80] ENTIRE STRING WANTED?
        jr      c,ALLSTR          ;[M80] IF #CHARS ASKED FOR.GE.LENGTH,YES
        ld      a,b               ;[M80] GET TRUNCATED LENGTH OF STRING
        byte    $11               ;[M80] SKIP OVER MVI USING "LD D,"
ALLSTR: ld      c,0               ;[M80] MAKE OFFSET ZERO
        push    bc                ;[M80] SAVE OFFSET ON STACK
        call    GETSPA            ;[M80] GET SPACE FOR NEW STRING
        pop     bc                ;[M80] GET BACK OFFSET
        pop     hl                ;[M80] GET BACK DESC POINTER.
        push    hl                ;[M80] BUT KEEP ON STACK
        inc     hl                ;[M80] MOVE TO STRING POINTER FIELD
        inc     hl                ;
        ld      b,(hl)            ;[M80] GET POINTER LOW
        inc     hl                ;
        ld      h,(hl)            ;[M80] POINTER HIGH
        ld      l,b               ;[M80] GET LOW IN  L
        ld      b,0               ;[M80] GET READY TO ADD OFFSET TO POINTER
        add     hl,bc             ;[M80] ADD  IT
        ld      b,h               ;[M80] GET OFFSET POINTER IN [B,C]
        ld      c,l               ;
        call    STRAD2            ;[M80] SAVE INFO IN DSCTMP
        ld      l,a               ;[M80] GET#  OF CHARS TO  MOVE IN L
        call    MOVSTR            ;[M80] MOVE THEM IN
        pop     de                ;[M80] GET BACK DESC. POINTER
        call    FRETMP            ;[M80] FREE IT UP.
        jp      PUTNEW            ;[M80] PUT TEMP IN TEMP LIST
RIGHT:  call    PREAM             ;[M80] CHECK ARG
        pop     de                ;[M80] GET DESC. POINTER
        push    de                ;[M80] SAVE BACK FOR LEFT
        ld      a,(de)            ;[M80] GET PRESENT LEN OF STR
        sub     b                 ;[M80] SUBTRACT 2ND PARM
        jr      LEFT3             ;[M80] CONTINUE WITH LEFT CODE
;[M80] MID ($,#) RETURNS STR WITH CHARS FROM # POSITION ONWARD.
MID:    ex      de,hl             ;[M80] PUT THE TEXT POINTER IN [H,L]
        ld      a,(hl)            ;[M80] GET THE FIRST CHARACTER
        call    PREAM2            ;[M80] GET OFFSET OFF STACK AND MAKE
        inc     b                 ;
        dec     b                 ;[M80] SEE IF EQUAL TO ZERO
        jp      z,FCERR           ;[M80] IT MUST NOT BE 0
        push    bc                ;
        ld      e,$FF             ;[M80] IF TWO ARG GUY, TRUNCATE
        cp      ')'               ;[M80] [E] SAYS USE ALL CHARS
        jr      z,MID2            ;[M80] IF ONE ARGUMENT THIS IS CORRECT
        rst     SYNCHK            ;
        byte    ','               ;[M80] COMMA? MUST DELINEATE 3RD ARG
        call    GETBYT            ;[M80] GET ARGUMENT  IN  [E]
MID2:   rst     SYNCHK            ;
        byte    ')'               ;[M80] MUST BE FOLLOWED BY )
        pop     af                ;[M80] GET OFFSET BACK IN A
        ex      (sp),hl           ;[M80] SAVE TEXT POINTER, GET DESC.
        ld      bc,LEFT2          ;[M80] WHERE TO RETURN TO.
        push    bc                ;[M80] GOES ON STACK
        dec     a                 ;[M80] SUB ONE FROM OFFSET
        cp      (hl)              ;[M80] POINTER PAST END OF STR?
        ld      b,0               ;[M80] ASSUME NULL LENGTH STR
        ret     nc                ;[M80] YES, JUST USE NULL STR
        ld      c,a               ;[M80] SAVE OFFSET OF CHARACTER POINTER
        ld      a,(hl)            ;[M80] GET PRESENT LEN OF STR
        sub     c                 ;[M80] SUBTRACT INDEX (2ND ARG)
        cp      e                 ;[M80] IS IT TRUNCATION
        ld      b,a               ;[M80] GET CALCED LENGTH IN B
        ret     c                 ;[M80] IF NOT USE PARTIAL STR
        ld      b,e               ;[M80] USE TRUNCATED LENGTH
        ret                       ;[M80] RETURN TO LEFT2
;[M80] THE VAL FUNCTION TAKES A STRING AND TURN IT INTO A NUMBER
VAL:    call    LEN1              ;[M80] DO SETUP, SET RESULT=REAL
        jp      z,ZERO            ;[M80] MAKE SURE TYPE SET UP OK IN EXTENDED
        ld      e,a               ;[M80] GET LENGTH OF STR
        inc     hl                ;[M80] TO HANDLE THE FACT THE IF
        inc     hl                ;
        ld      a,(hl)            ;
        inc     hl                ;
        ld      h,(hl)            ;[M80] TWO STRINGS "1" AND "2"
        ld      l,a               ;[M80] ARE STORED NEXT TO EACH OTHER
        push    hl                ;[M80] AND FIN IS CALLED POINTING TO
        add     hl,de             ;[M80] THE FIRST TWELVE WILL BE RETURNED
        ld      b,(hl)            ;[M80] THE IDEA IS TO STORE 0 IN THE
        ld      (hl),d            ;[M80] STRING BEYOND THE ONE VAL
        ex      (sp),hl           ;[M80] IS BEING CALLED ON
        push    bc                ;[M80] THE FIRST CHARACTER OF THE NEXT STRING
        dec     hl                ;[M80] ***CALL CHRGET TO MAKE SURE
        rst     CHRGET            ;[M80] VAL(" -3")=-3
        call    FIN               ;[M80] IN EXTENDED, GET ALL THE PRECISION WE CAN
        pop     bc                ;[M80] GET THE MODIFIED CHARACTER OF THE NEXT STRING INTO [B]
        pop     hl                ;[M80] GET THE POINTER TO THE MODIFIED CHARACTER
        ld      (hl),b            ;[M80] RESTORE THE CHARACTER
        ret                       ;
;[M80] USED BY RIGHT$ AND LEFT$ FOR PARAMETER CHECKING AND SETUP
PREAM:  ex      de,hl             ;[M80] PUT THE TEXT POINTER IN [H,L]
        rst     SYNCHK            ;
        byte    ')'               ;[M80] PARAM LIST SHOULD END
;[M80] USED BY MID$ FOR PARAMETER CHECKING AND SETUP
PREAM2: pop     bc                ;[M80] GET RETURN ADDR OFF STACK
        pop     de                ;[M80] GET LENGTH OF ARG OFF STACK
        push    bc                ;[M80] SAVE RETURN ADDR BACK ON
        ld      b,e               ;[M80] SAVE INIT LENGTH
        ret
;[M65] FRE FUNCTION AND INTEGER TO FLOATING ROUTINES
FRE:    ld      hl,(STREND)       ;
        ex      de,hl             ;
        ld      hl,0              ;
        add     hl,sp             ;
        ld      a,(VALTYP)        ;
        or      a                 ;
        jp      z,GIVFLT          ;
        call    FREFAC            ;[M80] FREE UP ARGUMENT AND SETUP TO GIVE FREE STRING SPACE
        call    GARBA2            ;[M80] DO GARBAGE COLLECTION
        ld      de,(TOPMEM)       ;
        ld      hl,(FRETOP)       ;[M80] TOP OF FREE AREA
        jp      GIVFLT            ;[M80] RETURN [H,L]-[D,E]
DIMCON: dec     hl                ;[M80] SEE IF COMMA ENDED THIS VARIABLE
        rst     CHRGET            ;
        ret     z                 ;[M80] IF TERMINATOR, GOOD BYE
        rst     SYNCHK            ;
        byte    ','               ;[M80] MUST BE COMMA
;{M80} DIMENSION
DIM:    ld      bc,DIMCON       ;[M80] PLACE TO COME BACK TO
        push    bc              ;
        byte    $F6             ;;"OR" to skip next instruction
;{M80} VARIABLE SEARCHING
;;Get Pointer to Variable
PTRGET: xor      a                ;[M80] MAKE [A]=0
        ld      (DIMFLG),a        ;[M80] FLAG IT AS SUCH
        ld      c,(hl)            ;[M80] GET FIRST CHARACTER IN [C]
;;Get Pointer to Variable after Reading First Character
PTRGT2: call    ISLET             ;[M80] CHECK FOR LETTER
        jp      c,SNERR           ;[M80] MUST HAVE A LETTER
        xor     a                 ;
        ld      b,a               ;[M80] ASSUME NO SECOND CHARACTER
        ld      (VALTYP),a        ;[M80] ZERO NAMCNT
        rst     CHRGET            ;[M80] GET CHAR
        jr      c,ISSEC           ;[M80] YES, WAS NUMERIC
        call    ISLETC            ;[M80] SET CARRY IF NOT ALPHABETIC
        jr      c,NOSEC           ;[M80] ALLOW ALPHABETICS
ISSEC:  ld      b,a               ;[M80] IT IS A NUMBER--SAVE IN B
EATEM:  rst     CHRGET            ;[M80] GET CHAR
        jr      c,EATEM           ;[M65] SKIP NUMERICS
        call    ISLETC            ;
        jr      nc,EATEM          ;[M65] SKIP ALPHABETICS
NOSEC:  sub     '$'               ;[M65] IS IT A STRING?
        jr      nz,NOTSTR         ;[M65] IF NOT, [VALTYP]=0.
        inc     a                 ;[M65] SET [VALTYP]=1 (STRING !)
        ld      (VALTYP),a        ;
        rrca                      ;
        add     a,b               ;
        ld      b,a               ;
        rst     CHRGET            ;[M80] READ PAST TYPE MARKER
NOTSTR: ld      a,(SUBFLG)        ;[M80] GET FLAG WHETHER TO ALLOW ARRAYS
        dec     a                 ;[M80] IF SUBFLG=1, "ERASE" HAS CALLED
        jp      z,ERSFIN          ;[M80] PTRGET, AND SPECIAL HANDLING MUST BE DONE
        jp      p,NOARYS          ;[M80] NO ARRAYS ALLOWED
        ld      a,(hl)            ;[M80] GET CHAR BACK
        sub     '('               ;[M80] (CHECK FOR "(") WON'T MATCH IF SUBFLG SET
        jp      z,ISARY           ;[M80] IT IS!
NOARYS: xor     a                 ;[M80]ALLOW PARENS AGAIN
        ld      (SUBFLG),a        ;[M80]SAVE IN FLAG LOCATION
        push    hl                ;[M80] SAVE THE TEXT POINTER
        ld      d,b               ;;???Variable Name
        ld      e,c
        ld      hl,(VARNAM)
        rst     COMPAR
        ld      de,VARPNT
        jp      z,POPHRT
        ld      hl,(ARYTAB)
        ex      de,hl             ;{M80} [D,E]=POINTER INTO ARRAYS
        ld      hl,(VARTAB)       ;{M80} [H,L]=POINTER INTO SIMPLE VARIABLES
LOPFND: rst     COMPAR            ;
        jp      z,SMKVAR          ;[M80] IF SO, CREATE VARIABLE
        ld      a,c               ;[M80] ARE LOW BYTES DIFFERENT
        sub     (hl)              ;[M80] TEST
        inc     hl                ;
        jp      nz,LOPFN2         ;{M80} NO
        ld      a,b               ;[M80] ARE HIGH BYTES DIFFERENT
        sub     (hl)              ;[M80] THE SAME?
LOPFN2: inc     hl                ;
        jp      z,NTFPRT          ;{M80} YES
        inc     hl                ;
        inc     hl                ;
        inc     hl                ;
        inc     hl                ;;Skip past variable
        jp      LOPFND            ;
SMKVAR: pop     hl                ;
        ex      (sp),hl           ;
        push    de                ;
        ld      de,RETVAR         ;[M80] DID EVAL CALL US?
        rst     COMPAR            ;[M80] IF SO, DON'T MAKE A NEW VARIABLE
        pop     de                ;[M80] RESTORE THE POSITION
        jp      z,FINZER          ;[M80] MAKE FAC ZERO (ALL TYPES) AND SKIP RETURN
        ex      (sp),hl           ;[M80] PUT RETURN ADDRESS BACK
        push    hl                ;[M80] PUT THE TEXT POINTER BACK
        push    bc                ;[M80] SAVE THE LOOKS
        ld      bc,6              ;[M80] MAKE THE LENGTH INCLUDE EVERYTHING UP BY
        ld      hl,(STREND)       ;[M80] THE CURRENT END OF STORAGE
        push    hl                ;[M80] SAVE THIS #
        add     hl,bc             ;[M80] ADD ON THE AMOUNT OF SPACE EXTRA NOW BEING USED
        pop     bc                ;[M80] POP OFF HIGH ADDRESS TO MOVE
        push    hl                ;[M80] SAVE NEW CANDIDATE FOR STREND
        call    BLTU              ;{M80} BLOCK TRANSFER AND CHECK FOR STACK OVERFLOW
        pop     hl                ;[M80] [H,L]=NEW STREND
        ld      (STREND),hl       ;[M80] BLOCK TRANSFER WAS DONE, SO UPDATE POINTERS
        ld      h,b               ;[M80] GET BACK [H,L] POINTING AT THE END
        ld      l,c               ;[M80] OF THE NEW VARIABLE
        ld      (ARYTAB),hl       ;[M80] UPDATE THE ARRAY TABLE POINTER
ZEROER: dec     hl                ;[M80] [H,L] IS RETURNED POINTING TO THE
        ld      (hl),0            ;[M80] END OF THE VARIABLE SO WE
        rst     COMPAR            ;[M80] ZERO BACKWARDS TO [D,E] WHICH
        jr      nz,ZEROER         ;[M80] POINTS TO THE START OF THE VARIABLE
        pop     de                ;
        ld      (hl),e            ;[M80] PUT DESCRIPTION
        inc     hl                ;
        ld      (hl),d            ;[M80] OF THIS VARIABLE INTO MEMORY
        inc     hl                ;
NTFPRT: ex      de,hl             ;[M80] TABLE POINTER BACK INTO [D,E]
        pop     hl                ;[M80] GET BACK THE TEXT POINTER
        ret                       ;
; MAKE ALL TYPES ZERO AND SKIP RETURN
FINZER: ld      (FAC),a           ;[M80] MAKE FLOATS ZERO
        ld      hl,REDDY-1        ;[M80] MAKE IT A NULL STRING BY
        ld      (FACLO),hl        ;[M80] POINTING AT A ZERO
        pop     hl                ;[M80] GET THE TEXT POINTER
        ret                       ;[M80] RETURN FROM EVAL
;[M80] FORMAT OF ARRAYS IN CORE
ISARY:  push    hl                ;[M80] SAVE DIMFLG AND VALTYP FOR RECURSION
        ld      hl,(DIMFLG)       ;
        ex      (sp),hl           ;[M80] TEXT POINTER BACK INTO [H,L]
        ld      d,a               ;[M80] SET # DIMENSIONS =0
INDLOP: push    de                ;[M80] SAVE NUMBER OF DIMENSIONS
        push    bc                ;[M80] SAVE LOOKS
        call    INTIDX            ;[M80] EVALUATE INDICE INTO [D,E]
        pop     bc                ;[M80] POP OFF THE LOOKS
        pop     af                ;[M80] [A] = NUMBER OF DIMENSIONS SO FAR;
        ex      de,hl             ;[M80] [D,E]=TEXT POINTER, [H,L]=INDICE
        ex      (sp),hl           ;[M80] PUT NDICE ON STACK, [H,L]=VALTYP & DIMFLG
        push    hl                ;[M80] RESAVE VALTYP AND DIMFLG
        ex      de,hl             ;[M80] [H,L]=TEXT POINTER
        inc     a                 ;[M80] INCREMENT # OF DIMENSIONS
        ld      d,a               ;[M80] [D]=NUMBER OF DIMENSIONS
        ld      a,(hl)            ;[M80] GET TERMINATING CHARACTER
        cp      ','               ;[M80] A COMMA SO MORE INDICES FOLLOW?
        jp      z,INDLOP          ;[M80] IF SO, READ MORE
        rst     SYNCHK            ;
        byte    ')'               ;{M80} MAKE SURE THERE IS A BRACKET
        ld      (TEMP2),hl        ;[M80 ]SAVE THE TEXT POINTER
        pop     hl                ;[M80 ][H,L]= VALTYP & DIMFLG
        ld      (DIMFLG),hl       ;[M80 ]SAVE VALTYP AND DIMFLG
        ld      e,0               ;{M80 }WHEN [D,E] IS POPPED INTO PSW, ZERO FLAG WON'T BE SET
        push    de                ;[M80 ]SAVE NUMBER OF DIMENSIONS
        byte    $11               ;[M80] "LD H," OVER THE NEXT TWO BYTES
;;Get Pointer to Array
;;BC = Array Variable Name on Entry
;;     Pointer to Number of Dimensions on Exit
ERSFIN: push    hl                ;[M80] SAVE THE TEXT POINTER
        push    af                ;[M80] SAVE A DUMMY NUMBER OF DIMENSIONS WITH THE ZERO FLAG SET
;[M80] AT THIS POINT [B,C]=LOOKS. THE TEXT POINTER IS IN TEMP2.
;[M80] THE INDICES ARE ALL ON THE STACK, FOLLOWED BY THE NUMBER OF DIMENSIONS.
        ld      hl,(ARYTAB)       ;[M80] [H,L]=PLACE TO START THE SEARCH
        byte    $3E               ;[M80] "LD A," AROUND THE NEXT BYTE
LOPFDA: add     hl,de             ;[M80] SKIP OVER THIS ARRAY SINCE IT'S NOT THE ONE
        ld      de,(STREND)       ;[M80] GET THE PLACE TO STOP INTO [H,L]
        rst     COMPAR            ;[M80] STOPPING TIME?
        jr      z,NOTFDD          ;[M80] YES, COULDN'T FIND THIS ARRAY
        ld      a,(hl)            ;[M80] GET FIRST CHARACTER
        inc     hl
        cp      c                 ;[M80] SEE IF IT MATCHES
        jr      nz,NMARY1         ;[M80] NOT THIS ONE
        ld      a,(hl)            ;[M80] GET SECOND CHARACTER
        cp      b                 ;[M80] ANOTHER MATCH?
NMARY1: inc     hl                ;[M80] POINT TO SIZE ENTRY
        ld      e,(hl)            ;[M80] [D,E]=LENGTH
        inc     hl                ;[M80] OF THE ARRAY BEING LOOKED AT
        ld      d,(hl)
        inc     hl
        jr      nz,LOPFDA         ;[M80] IF NO MATCH, SKIP THIS ONE AND TRY AGAIN
        ld      a,(DIMFLG)        ;[M80] SEE IF CALLED BY "DIM"
        or      a                 ;[M80] ZERO MEANS NO
        jp      nz,DDERR          ;[M80] "REDIMENSIONED VARIABLE" IF "DIM" CALLING PTRGET
;[M80] TEMP2=THE TEXT POINTER
        pop     af                ;[M80] [A]=NUMBER OF DIMENSIONS
        ld      b,h               ;[M80] SET [B,C] TO POINT AT NUMBER OF DIMENSIONS
        ld      c,l
        jp      z,POPHRT          ;[M80] "ERASE" IS DONE, SO RETURN TO DO THE ACTUAL ERASURE
        sub     (hl)              ;{M80} MAKE SURE DIMENSIONS MATCH
        jp      z,GETDEF          ;[M80] JUMP OFF AND READ E INDICES....
BSERR:  ld      de,ERRBS          ;[M80] "SUBSCRIPT OUT OF RANGE"
        jp      ERROR
;[M80] HERE WHEN VARIABLE IS NOT FOUND IN THE ARRAY TABLE
NOTFDD: ld      de,4              ;[M80] [D,E]=SIZE OF ONE VALUE (VALTYP)
        pop     af                ;[M80] [A]=NUMBER OF DIMENSIONS
        jp      z,FCERR           ;[M80] "ILLEGAL FUNCTION CALL"
        ld      (hl),c            ;[M80] PUT DOWN THE DESCRIPTOR
        inc     hl
        ld      (hl),b
        inc     hl
        ld      c,a               ;{M80} [C]=ENTRIES NEEDED TO STORE SIZE OF EACH DIMENSION
        call    GETSTK            ;[M80] GET SPACE FOR DIMENSION ENTRIES
        inc     hl                ;[M80] SKIP OVER THE SIZE LOCATIONS
        inc     hl
        ld      (TEMP3),hl        ;[M80] SAVE THE LOCATION TO PUT THE SIZE IN
        ld      (hl),c            ;[M80] STORE THE NUMBER OF DIMENSIONS
        inc     hl
        ld      a,(DIMFLG)        ;{M80} CALLED BY DIMENSION?
        rla                       ;[M80] SET CARRY IF SO
        ld      a,c               ;[M80] [A]=NUMBER OF DIMENSIONS
LOPPTA: ld      bc,11             ;[M80] MAP 0 TO 11 AND 1 TO 10
        jr      nc,NOTDIM         ;[M80] DEFAULT DIMENSIONS TO TEN
POPDIM: pop     bc                ;[M80] POP OFF AN INDICE INTO [B,C]
        inc     bc                ;[M80] ADD ONE TO IT FOR THE ZERO ENTRY
NOTDIM: ld      (hl),c            ;[M80] PUT THE MAXIMUM DOWN
        push    af                ;[M80] SAVE NUMBER OF DIMENSIONS AND DIMFLG (CARRY)
        inc     hl
        ld      (hl),b
        inc     hl
        push    hl
        call    UMULT             ;[M80] MULTIPLY [B,C]=NEWMAX BY CURTOL=[D,E]
        ex      de,hl
        pop     hl
        pop     af                ;[M80] GET NUMBER OF DIMENSIONS AND DIMFLG (CARRY) BACK
        dec     a                 ;[M80] DECREMENT THE NUMBER OF DIMENSIONS LEFT
        jr      nz,LOPPTA         ;[M80] HANDLE THE OTHER INDICES
        push    af                ;[M80] SAVE DIMFLG (CARRY)
        ld      b,d               ;[M80] [B,C]=SIZE
        ld      c,e
        ex      de,hl             ;[M80] [D,E]=START OF VALUES
        add     hl,de             ;[M80] [H,L]=END OF VALUES
        jp      c,OMERR           ;[M80] OUT OF MEMORY POINTER BEING GENERATED?
        call    REASON            ;[M80] SEE IF THERE IS ROOM FOR THE VALUES
        ld      (STREND),hl       ;[M80] UPDATE THE END OF STORAGE
ZERITA: dec     hl                ;[M80] ZERO THE NEW ARRAY
        ld      (hl),000H
        rst     COMPAR            ;[M80] BACK AT THE BEGINNING?
        jr      nz,ZERITA         ;[M80] NO, ZERO MORE
        inc     bc                ;(M80) ADD ONE TO INCLUDE BYTE FOR NUMBER OF DIMENSIONS
        ld      d,a               ;[M80[ [D]=ZERO
        ld      hl,(TEMP3)        ;[M80[ GET A POINTER AT THE NUMBER OF DIMENSIONS
        ld      e,(hl)            ;[M80[ [E]=NUMBER OF DIMENSIONS
        ex      de,hl             ;[M80[ [H,L]=NUMBER OF DIMENSIONS
        add     hl,hl             ;[M80[ [H,L]=NUMBER OF DIMENSIONS TIMES TWO
        add     hl,bc             ;[M80[ ADD ON THE SIZE TO GET THE TOTAL NUMBER OF BYTES USED
        ex      de,hl             ;[M80[ [D,E]=TOTAL SIZE
        dec     hl                ;[M80[ BACK UP TO POINT TO LOCATION TO PUT
        dec     hl                ;[M80[ THE SIZE OF THE ARRAY IN BYTES IN.
        ld      (hl),e            ;[M80[ PUT DOWN THE SIZE
        inc     hl
        ld      (hl),d
        inc     hl
        pop     af                ;[M80] GET BACK DIMFLG (CARRY) AND SET [A]=0
        jr      c,FINNOW
;[M80] AT THIS POINT [H,L] POINTS BEYOND THE SIZE TO THE NUMBER OF DIMENSIONS
;[M80] STRATEGY:
;[M80]  NUMDIM=NUMBER OF DIMENSIONS
;[M80]  CURTOL=0
;[M80] INLPNM:GET A NEW INDICE
;[M80]  POP NEW MAX INTO CURMAX
;[M80]  MAKE SURE INDICE IS NOT TOO BIG
;[M80]  MUTLIPLY CURTOL BY CURMAX
;[M80]  ADD INDICE TO CURTOL
;[M80]  NUMDIM=NUMDIM-1
;[M80]  JNZ     INLPNM
;[M80]  USE CURTOL*4 (VALTYP FOR EXTENDED) AS OFFSET
;
GETDEF: ld      b,a               ;[M80] [B,C]=CURTOL=ZERO
        ld      c,a
        ld      a,(hl)            ;[M80] [A]=NUMBER OF DIMENSIONS
        inc     hl                ;[M80] POINT PAST THE NUMBER OF DIMENSIONS
        byte    $16               ;[M80] "LD D," AROUND THE NEXT BYTE
INLPNM: pop     hl                ;[M80] [H,L]= POINTER INTO VARIABLE ENTRY
        ld      e,(hl)            ;[M80] [D,E]=MAXIMUM FOR THE CURRENT INDICE
        inc     hl
        ld      d,(hl)
        inc     hl                ;[M80] [H,L]=CURRENT INDICE
        ex      (sp),hl           ;[M80] POINTER INTO THE VARIABLE GOES ON THE STACK
        push    af                ;[M80] SAVE THE NUMBER OF DIMENSIONS
        rst     COMPAR            ;[M80] SEE IF THE CURRENT INDICE IS TOO BIG
        jp      nc,BSERR          ;[M80] IF SO "BAD SUBSCRIPT" ERROR
        push    hl
        call    UMULT             ;[M80] CURTOL=CURTOL*CURRENT MAXIMUM
        pop     de
        add     hl,de             ;[M80] ADD THE INDICE TO CURTOL
        pop     af                ;[M80] GET THE NUMBER OF DIMENSIONS IN [A]
        dec     a                 ;[M80] SEE IF ALL THE INDICES HAVE BEEN PROCESSED
        ld      b,h               ;[M80] [B,C]=CURTOL IN CASE WE LOOP BACK
        ld      c,l
        jr      nz,INLPNM         ;[M80] PROCESS THE REST OF THE INDICES
        add     hl,hl             ;[M80] MULTIPLY BY TWO
        add     hl,hl             ;[M80] NOW MULTIPLIED BY FOUR
        pop     bc                ;[M80] POP OFF THE ADDRESS OF WHERE THE VALUES BEGIN
        add     hl,bc             ;[M80] ADD IT ONTO CURTOL TO GET PLACE VALUE IS STORED
        ex      de,hl             ;[M80] RETURN THE POINTER IN [D,E]
FINNOW: ld      hl,(TEMP2)        ;[M80] REGET THE TEXT POINTER
        ret
;[M80] MATHPK FOR BASIC MCS 8080  GATES/ALLEN/DAVIDOFF
;[M80] FLOATING POINT ADDITION AND SUBTRACTION
FADDH:  ld      hl,FHALF          ;[M80] ENTRY TO ADD 1/2
FADDS:  call    MOVRM             ;[M80] GET ARGUMENT INTO THE REGISTERS
        jr      FADD              ;[M80] DO THE ADDITION
;[M80] SUBTRACTION      FAC:=ARG-FAC
FSUBS:  call    MOVRM             ;[M80] ENTRY IF POINTER TO ARG IS IN (HL)
        byte    $21               ;;"LD HL," to skip next instruction
FSUBT:  pop     bc                ;;ENTRY TO FSUB IF ARGUMENT IS ON STACK
        pop     de
FSUB:   call    NEG               ;[M80] NEGATE SECOND ARGUMENT
;[M80] ADDITION FAC:=ARG+FAC
FADD:   ld      a,b               ;[M80] CHECK IF FIRST ARGUMENT IS ZERO
        or      a                 ;[M80] GET EXPONENT
        ret     z                 ;[M80] IT IS, RESULT IS NUMBER IN FAC
        ld      a,(FAC)           ;[M80] GET EXPONENT
        or      a                 ;[M80] SEE IF THE NUMBER IS ZERO
        jp      z,MOVFR           ;[M80] IT IS, ANSWER IS IN REGISTERS
;;Align Numbers
        sub     b                 ;[M80] CHECK RELATIVE SIZES
        jr      nc,FADD1          ;[M80] IS FAC SMALLER?
        cpl                       ;[M80] YES, NEGATE SHIFT COUNT
        inc     a
        ex      de,hl             ;[M80] SWITCH FAC AND REGISTERS, SAVE (DE)
        call    PUSHF             ;[M80] PUT FAC ON STACK
        ex      de,hl             ;[M80] GET (DE) BACK WHERE IT BELONGS
        call    MOVFR             ;[M80] PUT REGISTERS IN THE FAC
        pop     bc
        pop     de                ;[M80] GET THE OLD FAC IN THE REGISTERS
FADD1:  cp      25                ;ARE WE WITHIN 24 BITS?
        ret     nc
        push    af                ;SAVE SHIFT COUNT
        call    UNPACK            ;UNPACK THE NUMBERS
        ld      h,a               ;SAVE SUBTRACTION FLAG
        pop     af                ;GET SHIFT COUNT BACK
        call    SHIFTR            ;SHIFT REGISTERS RIGHT THE RIGHT AMOUNT
;;Add or Subtract Numbers
        ld      a,h               ;[M80] GET SUBTRACTION FLAG
        or      a
        ld      hl,FACLO          ;[M80] SET POINTER TO LO'S
        jp      p,FADD3           ;[M80] SUBTRACT IF THE SIGNS WERE DIFFERENT
        call    FADDA             ;[M80] ADD THE NUMBERS
        jr      nc,ROUND          ;[M80] ROUND RESULT IF THERE WAS NO OVERFLOW
        inc     hl                ;[M80] THERE WAS OVERFLOW
        inc     (hl)              ;[M80] INCREMENT EXPONENT
        jp      z,OVERR
        ld      l,1               ;[M80] SHIFT RESULT RIGHT ONE, SHIFT CARRY IN
        call    SHRADD
        jr      ROUND             ;[M80] ROUND RESULT AND WE ARE DONE
;[M80] HERE TO SUBTRACT C,D,E,B FROM ((HL)+0,1,2),0
FADD3:  xor     a                 ;[M80] SUBTRACT NUMBERS, NEGATE UNDERFLOW BYTE
        sub     b
        ld      b,a               ;[M80] SAVE IT
        ld      a,(hl)            ;[M80] SUBTRACT LOW ORDERS
        sbc     a,e
        ld      e,a
        inc     hl                ;[M80] UPDATE POINTER TO NEXT BYTE
        ld      a,(hl)            ;[M80] SUBTRACT MIDDLE ORDERS
        sbc     a,d
        ld      d,a
        inc     hl                ;[M80] UPDATE POINTER TO HIGH ORDERS
        ld      a,(hl)            ;[M80] SUBTRACT HIGH ORDERS
        sbc     a,c
        ld      c,a
;[M80] BECAUSE WE WANT A POSITIVE MANTISSA, CHECK IF WE HAVE TO NEGATE THE NUMBER
FADFLT: call    c,NEGR
;NORMALIZE C,D,E,B
NORMAL: ld      l,b               ;[M80] PUT LOWEST 2 BYTES IN (HL)
        ld      h,e
        xor     a                 ;[M80] ZERO SHIFT COUNT
NORM1:  ld      b,a               ;[M80] SAVE SHIFT COUNT
        ld      a,c               ;[M80] DO WE HAVE 1 BYTE OF ZEROS
        or      a
        jr      nz,NORM3          ;[M80] NO, SHIFT ONE PLACE AT A TIME
;[M80] THIS LOOP SPEEDS THINGS UP BY SHIFTING 8 PLACES AT ONE TIME
        ld      c,d               ;[M80] YES, SHIFT OVER 1 BYTE
        ld      d,h
        ld      h,l
        ld      l,a               ;[M80] SHIFT IN 8 aS FOR THE LOW ORDER
        ld      a,b               ;[M80] UPDATE SHIFT COUNT
        sub     8
        cp      224               ;[M80] DID WE SHIFT IN 4 BYTES OF ZEROS?
        jr      nz,NORM1          ;[M80] NO, TRY TO SHIFT OVER 8 MORE
;[M80] ZERO FAC
ZERO:   xor     a                  ;[M80] ZERO A
ZERO0:  ld      (FAC),a            ;[M80] ZERO THE FAC'S EXPONENT, ENTRY IF A=0
        ret                        ;[M80] ALL DONE
NORM2:  ld      a,h                ;[M80] CHECK FOR CASE OF NORMALIZING A SMALL INT
        or      l
        or      d
        jr      nz,NORM2U          ;[M80] DO USUAL THING
        ld      a,c                ;[M80] GET BYTE TO SHIFT
NORM2F: dec     b                  ;[M80] DECRMENT SHIFT COUNT
        rla                        ;[M80] SHIFT LEFT
        jr      nc,NORM2F          ;[M80] NORMALIZE LIKE SOB
        inc     b                  ;[M80] CORRECT SHIFT COUNT
        rra                        ;[M80] WE DID IT ONE TOO MANY TIMES
        ld      c,a                ;[M80] RESULT TO [C]
        jr      NORM3A             ;[M80] ALL DONE
NORM2U: dec     b                  ;[M80] DECREMENT SHIFT COUNT
        add     hl,hl              ;[M80] ROTATE (HL) LEFT ONE, SHIFT IN A ZERO
        ld      a,d                ;[M80] ROTATE NEXT HIGHER ORDER LEFT ONE
        rla                        ;
        ld      d,a                ;
        ld      a,c                ;[M80] ROTATE HIGH ORDER LEFT ONE
        adc     a,a                ;[M80] SET CONDITION CODES
        ld      c,a                ;
NORM3:  jp      p,NORM2           ;[M80] WE HAVE MORE NORMALIZATION TO DO
NORM3A: ld      a,b               ;[M80] ALL NORMALIZED, GET SHIFT COUNT
        ld      e,h               ;[M80] PUT LO'S BACK IN E,B
        ld      b,l               ;
        or      a                 ;[M80] CHECK IF WE DID NO SHIFTING
        jr      z,ROUND           ;
        ld      hl,FAC            ;[M80] LOOK AT FAC'S EXPONENT
        add     a,(hl)            ;[M80] UPDATE EXPONENT
        ld      (hl),a            ;
        jr      nc,ZERO           ;[M80] CHECK FOR UNDERFLOW
        jr      z,ZERO            ;[M80] NUMBER IS ZERO, ALL DONE
;[M80] ROUND RESULT IN C,D,E,B AND PUT NUMBER IN THE FAC
ROUND:  ld      a,b               ;[M80] SEE IF WE SHOULD ROUND UP
ROUNDB: ld      hl,FAC            ;[M80] ENTRY FROM FDIV, GET POINTER TO EXPONENT
        or      a
        call    m,ROUNDA          ;[M80] DO IT IF NECESSARY
        ld      b,(hl)            ;[M80] PUT EXPONENT IN B
;[80] HERE WE PACK THE HO AND SIGN
        inc     hl                ;[M80]  POINT TO SIGN
        ld      a,(hl)            ;[M80]  GET SIGN
        and     $80               ;[M80]  GET RID OF UNWANTED BITS
        xor     c                 ;[M80]  PACK SIGN AND HO
        ld      c,a               ;[M80]  SAVE IT IN C
        jp      MOVFR             ;[M80]  SAVE NUMBER IN FAC
;[M80] SUBROUTNE FOR ROUND:  ADD ONE TO C,D,E
ROUNDA: inc     e                 ;[M80] ADD ONE TO THE LOW ORDER, ENTRY FROM QINT
        ret     nz                ;[M80] ALL DONE IF IT IS NOT ZERO
        inc     d                 ;[M80] ADD ONE TO NEXT HIGHER ORDER
        ret     nz                ;[M80] ALL DONE IF NO OVERFLOW
        inc     c                 ;[M80] ADD ONE TO THE HIGHEST ORDER
        ret     nz                ;[M80] RETURN IF NO OVEFLOW
        ld      c,$80             ;[M80] THE NUMBER OVERFLOWED, SET NEW HIGH ORDER
        inc     (hl)              ;[M80] UPDATE EXPONENT
        ret     nz                ;[M80] RETURN IF IT DID NOT OVERFLOW
        jp      OVERR             ;[M80] OVERFLOW
;[M80] ADD (HL)+2,1,0 TO C,D,E
FADDA:  ld      a,(hl)            ;[M80] GET LOWEST ORDER
        add     a,e               ;[M80] ADD IN OTHER LOWEST ORDER
        ld      e,a               ;[M80] SAVE IT
        inc     hl                ;[M80] UPDATE POINTER TO NEXT BYTE
        ld      a,(hl)            ;[M80] ADD MIDDLE ORDERS
        adc     a,d
        ld      d,a
        inc     hl                ;[M80] UPDATE POINTER TO HIGH ORDER
        ld      a,(hl)            ;[M80] ADD HIGH ORDERS
        adc     a,c
        ld      c,a
        ret                       ;[M80] ALL DONE
;[M80] NEGATE NUMBER IN C,D,E,B
NEGR:   ld      hl,FAC+1          ;[M80] NEGATE FAC
        ld      a,(hl)            ;[M80] GET SIGN
        cpl                       ;[M80] COMPLEMENT IT
        ld      (hl),a            ;[M80] SAVE IT AGAIN
        xor     a                 ;[M80] ZERO A
        ld      l,a               ;[M80] SAVE ZERO IN L
        sub     b                 ;[M80] NEGATE LOWEST ORDER
        ld      b,a               ;[M80] SAVE IT
        ld      a,l               ;[M80] GET A ZERO
        sbc     a,e               ;[M80] NEGATE NEXT HIGHEST ORDER
        ld      e,a               ;[M80] SAVE IT
        ld      a,l               ;[M80] GET A ZERO
        sbc     a,d               ;[M80] NEGATE NEXT HIGHEST ORDER
        ld      d,a               ;[M80] SAVE IT
        ld      a,l               ;[M80] GET ZERO BACK
        sbc     a,c               ;[M80] NEGATE HIGHEST ORDER
        ld      c,a               ;[M80] SAVE IT
        ret                       ;[M80] ALL DONE
;[M80] SHIFT C,D,E RIGHT
SHIFTR: ld      b,0               ;[M80] ZERO OVERFLOW BYTE
SHFTR1: sub     8                 ;[M80] CAN WE SHIFT IT 8 RIGHT?
        jr      c,SHFTR2          ;[M80] NO, SHIFT IT ONE PLACE AT A TIME
;[M80] THIS LOOP SPEEDS THINGS UP BY SHIFTING 8 PLACES AT ONE TIME
        ld      b,e               ;[M80] SHIFT NUMBER 1 BYTE RIGHT
        ld      e,d
        ld      d,c
        ld      c,0               ;[M80 PUT 0 IN HO
        jr      SHFTR1            ;[M80 TRY TO SHIFT 8 RIGHT AGAIN
SHFTR2: add     a,9               ;[M80 CORRECT SHIFT COUNT
        ld      l,a               ;[M80 SAVE SHIFT COUNT
;[M80] TEST FOR CASE (VERY COMMON) WHERE SHIFTING SMALL INTEGER RIGHT.
;[M80] THIS HAPPENS IN FOR LOOPS, ETC.
        ld      a,d               ;[M80] SEE IF THREE LOWS ARE ZERO.
        or      e
        or      b
        jr      nz,SHFTR3         ;[M80] IF SO, DO USUAL.
        ld      a,c               ;[M80] GET HIGH BYTE TO SHIFT
SHFTRF: dec     l                 ;[M80] DONE SHIFTING?
        ret     z                 ;[M80] YES, DONE
        rra                       ;[M80] ROTATE ONE RIGHT
        ld      c,a               ;[M80] SAVE RESULT
        jr      nc,SHFTRF         ;[M80] ZAP BACK AND DO NEXT ONE IF NONE
        jr      SHFTC             ;[M80] CONTINUE SHIFTING
SHFTR3: xor     a                 ;[M80] CLEAR CARRY
        dec     l                 ;[M80] ARE WE DONE SHIFTING?
        ret     z                 ;[M80] RETURN IF WE ARE
        ld      a,c               ;[M80] GET HO
SHRADD: rra                       ;[M80] ENTRY FROM FADD, SHIFT IT RIGHT
        ld      c,a               ;[M80] SAVE IT
SHFTC:  ld      a,d               ;[M80] SHIFT NEXT BYTE RIGHT
        rra
        ld      d,a
        ld      a,e               ;[M80] SHIFT LOW ORDER RIGHT
        rra
        ld      e,a
        ld      a,b               ;[M80] SHIFT OVERFLOW BYTE RIGHT
        rra
        ld      b,a
        jr      SHFTR3            ;[M80] SEE IF WE ARE DONE
;[M80] CONSTANTS USED BY LOG
FONE:   byte    $00,$00,$00,$81 ;1
LOGP:   byte    4                  ;[M80] HART 2524 COEFFICIENTS
        byte    $9A,$F7,$19,$83    ;[M80] 4.8114746
        byte    $24,$63,$43,$83    ;[M80] 6.105852
        byte    $75,$CD,$8D,$84    ;[M80] -8.86266
        byte    $A9,$7F,$83,$82    ;[M80] -2.054667
LOGQ:   byte    4
        byte    $00,$00,$00,$81    ;[M80] 1.0
        byte    $E2,$B0,$4D,$83    ;[M80] 6.427842
        byte    $0A,$72,$11,$83    ;[M80] 4.545171
        byte    $F4,$04,$35,$7F    ;[M80] .3535534
;[M80] NATURAL LOG FUNCTION
LOG:    rst     FSIGN             ;[M80] CHECK FOR A NEGATIVE OR ZERO ARGUMENT
        or      a                 ;[M80] SET CC'S PROPERLY
        jp      pe,FCERR          ;[M80] FAC .LE. 0, BLOW HIM OUT OF THE WATER
        call    LOG2
        ld      bc,$8031
        ld      de,$7218          ;[M80] GET LN(2)
        jr      FMULT             ;[M80] COMPLETE LOG CALCULATION: USE HART 2524 CALCULATION
;[M80] USE HART 2524 CALCULATION
LOG2:   call    MOVRF             ;[M80] MOVE FAC TO REGISTERS TOO
        ld      a,080H
        ld      (FAC),a           ;[M80] ZERO THE EXPONENT
        xor     b                 ;[M80] REMOVE 200 EXCESS FROM X
        push    af                ;[M80] SAVE EXPONENT
        call    PUSHF             ;[M80] SAVE THE FAC (X)
        ld      hl,LOGP           ;[M80] POINT TO P CONSTANTS
        call    POLY              ;[M80] CALCULATE P(X)
        pop     bc                ;[M80] FETCH X
        pop     hl                ;[M80] PUSHF WOULD ALTER DE
        call    PUSHF             ;[M80] PUSH P(X) ON THE STACK
        ex      de,hl             ;[M80] GET LOW BYTES OF X TO (DE)
        call    MOVFR             ;[M80] AND MOVE TO FAC
        ld      hl,LOGQ           ;[M80] POINT TO Q COEFFICIENTS
        call    POLY              ;[M80] COMPUTE Q(X)
        pop     bc                ;[M80] FETCH P(X) TO REGISTERS
        pop     de                ;
        call    FDIV              ;[M80] CALCULATE P(X)/Q(X)
        pop     af                ;[M80] RE-FETCH EXPONENT
        call    PUSHF             ;[M80] SAVE EVALUATION
        call    FLOAT             ;[M80] FLOAT THE EXPONENT
        pop     bc                ;
        pop     de                ;
        jp      FADD              ;[M80] GET EVAL. BACK
;;***Unused code?
;;;IMULT in [M80] fell into here
        byte    $21               ;;[LD HL.] to skip next two instructions
FMULTT: pop     bc                ;[M80] GET FIRST ARGUMENT OFF STACK, ENTRY FROM POLYX
        pop     de
;[M80] MULTIPLICATION           FAC:=ARG*FAC
FMULT:  rst     FSIGN             ;[M80] CHECK IF FAC IS ZERO
        ret     z                 ;[M80] IF IT IS, RESULT IS ZERO
        ld      l,0               ;[M80] ADD THE TWO EXPONENTS, L IS A FLAG
        call    MULDIV            ;[M80] FIX UP THE EXPONENTS
;[M80] SAVE THE NUMBER IN THE REGISTERS SO WE CAN ADD IT FAST
        ld      a,c               ;[M80] GET HO
        ld      (RESHO),a         ;[M80] STORE HO OF REGISTERS
        ex      de,hl             ;[M80] STORE THE TWO LO'S OF THE REGISTERS
        ld      (RESMO),hl        ;
        ld      bc,0              ;[M80] ZERO THE PRODUCT REGISTERS
        ld      d,b               ;
        ld      e,b               ;
        ld      hl,NORMAL         ;
        push    hl                ;[M80]  ON THE STACK
        ld      hl,FMULT2         ;[M80] PUT FMULT2 ON THE STACK TWICE, SO AFTER
        push    hl                ;[M80]  WE MULTIPLY BY THE LO BYTE, WE WILL
        push    hl                ;[M80]  MULTIPLY BY THE MO AND HO
        ld      hl,FACLO          ;[M80] GET ADDRESS OF LO OF FAC
FMULT2: ld      a,(hl)            ;[M80] GET BYTE TO MULTIPLY BY
        inc     hl                ;[M80] MOVE POINTER TO NEXT BYTE
        or      a                 ;
        jr      z,FMULT3          ;[M80] ARE WE MULTIPLYING BY ZERO?
        push    hl                ;[M80] SAVE POINTER
        ld      l,8               ;[M80] SET UP A COUNT
FMULT4: rra                       ;[M80] ROTATE BYTE RIGHT
        ld      h,a               ;[M80] SAVE IT
        ld      a,c               ;[M80] GET HO
        jr      nc,FMULT5         ;[M80] DON'T ADD IN NUMBER IF BIT WAS ZERO
        push    hl                ;[M80] SAVE COUNTERS
        ld      hl,(RESMO)
        add     hl,de
        ex      de,hl
        pop     hl
        ld      a,(RESHO)
        adc     a,c
FMULT5: rra                       ;[M80] ROTATE RESULT RIGHT ONE
        ld      c,a               ;
        ld      a,d               ;[M80] ROTATE NEXT BYTE
        rra                       ;
        ld      d,a               ;
        ld      a,e               ;[M80] ROTATE NEXT LOWER ORDER
        rra                       ;
        ld      e,a               ;
        ld      a,b               ;[M80] ROTATE LO
        rra                       ;
        ld      b,a               ;
        and     010H              ;[M80] SEE IF WE ROTATED THRU ST
        jr      z,FML5B1          ;[M80] IF NOT DON'T WORRY
        ld      a,b               ;[M80] RE FETCH LO
        or      020H              ;[M80] "OR" IN STICKY
        ld      b,a               ;[M80] BACK TO LO
FML5B1: dec     l                 ;[M80] ARE WE DONE?
        ld      a,h               ;[M80] GET NUMBER WE ARE MULTIPLYING BY
        jr      nz,FMULT4         ;[M80] MULTIPLY AGAIN IF WE ARE NOT DONE
;;Pop [HL] and Return
POPHRT: pop     hl                ;[M80] GET POINTER TO NUMBER TO MULTIPLY BY
        ret                       ;[M80] ALL DONE
FMULT3: ld      b,e               ;[M80] MULTIPLY BY ZERO: SHIFT EVERYTHING 8 RIGHT
        ld      e,d               ;
        ld      d,c               ;
        ld      c,a               ;[M80] SHIFT IN 8 ZEROS ON THE LEFT
        ret                       ;[M80] ALL DONE
;[M80]  DIVIDE FAC BY 10
;[M80]  ALTERS A,B,C,D,E,H,L
DIV10:  call    PUSHF             ;[M80] WE HAVE TO DIVIDE -- SAVE COUNT
        ld      bc,$8420          ;[M80] 10.0
        ld      de,$0000
        call    MOVFR             ;[M80] MOVE TEN INTO THE FAC
FDIVT:  pop     bc                ;[F80] GET NUMBER BACK IN REGISTERS
        pop     de                ;[F80] FALL INTO DIVIDE AND WE ARE DONE
;[M80] DIVISION       FAC:=ARG/FAC
;[M80] ALTERS A,B,C,D,E,H,L
FDIV:   rst     FSIGN             ;[M80] CHECK FOR DIVISION BY ZERO
        jp      z,DV0ERR          ;[M80] DON'T ALLOW DIVIDE BY ZERO
        ld      l,255             ;[M80] SUBTRACT THE TWO EXPONENTS, L IS A FLAG
        call    MULDIV            ;[M80] FIX UP THE EXPONENTS AND THINGS
        inc     (hl)
        jp      z,OVERR           ;[M80] OVERFLOW
        inc     (hl)
        jp      z,OVERR           ;[M80] OVERFLOW
;[M80] HERE WE SAVE THE FAC IN MEMORY SO WE CAN SUBTRACT IT FROM THE NUMBER
;[M80] IN THE REGISTERS QUICKLY.
        dec     hl                ;[M80] POINT TO HO
        ld      a,(hl)            ;[M80] GET HO
        ld      (FDIVA+1),a       ;[M80] SAVE IT
        dec     hl                ;[M80] SAVE MIDDLE ORDER
        ld      a,(hl)
        ld      (FDIVB+1),a       ;[M80] PUT IT WHERE NOTHING WILL HURT IT
        dec     hl                ;[M80] SAVE LO
        ld      a,(hl)
        ld      (FDIVC+1),a
;[M80] THE NUMERATOR WILL BE KEPT IN B,H,L.  THE QUOTIENT WILL BE FORMED IN C,D,E.
        ld      b,c               ;[M80] GET NUMBER IN B,H,L
        ex      de,hl
        xor     a                 ;[M80] ZERO C,D,E AND HIGHEST ORDER
        ld      c,a
        ld      d,a
        ld      e,a
        ld      (FDIVG+1),a
FDIV1:  push    hl                ;[M80] SAVE LO'S OF NUMBER
        push    bc                ;[M80] SAVE HO OF NUMBER
        ld      a,l               ;[M80] SUBTRACT NUMBER THAT WAS IN FAC
        call    FDIVC             ;;Call Divide Routine in RAM for apeed
        sbc     a,0               ;[M80] SUBTRACT LO
        ccf                       ;[M80] SET CARRY TO CORESPOND TO NEXT QUOTIENT BIT
        jr      nc,FDIV2          ;[M80] GET OLD NUMBER BACK IF WE SUBTRACTED TOO MUCH
        ld      (FDIVG+1),a       ;[M80] UPDATE HIGHEST ORDER
        pop     af                ;[M80] THE SUBTRACTION WAS GOOD
        pop     af                ;[M80] GET PREVIOUS NUMBER OFF STACK
        scf                       ;[M80] NEXT BIT IN QUOTIENT IS A ONE
        byte    $D2               ;[M80] "JNC" AROUND NEXT 2 BYTES
FDIV2:  pop     bc                ;[M80] WE SUBTRACTED TOO MUCH
        pop     hl                ;[M80] GET OLD NUMBER BACK
        ld      a,c               ;[M80] ARE WE DONE?
        inc     a                 ;[M80] SET SIGN FLAG WITHOUT AFFECTING CARRY
        dec     a
        rra                       ;[M80]PUT CARRY IN MSB
        jp      p,DIV2A           ;[M80]NOT READY TO ROUND YET
        rla                       ;[M80]BIT BACK TO CARRY
        ld      a,(FDIVG+1)       ;[M80]FETCH EXTRA BIT
        rra                       ;[M80]BOTH NOW IN A
        and     $C0               ;[M80]CLEAR SUPERFLUOUS BITS
        push    af                ;[M80]SAVE FOR LATER
        ld      a,b               ;[M80]FETCH HO OF REMAINDER
        or      h                 ;[M80]FETCH HO
        or      l                 ;[M80]SEE IF OTHER REMAINDER BITS AND IF SO SET ST
        jr      z,DIV2AA          ;[M80]IF NOT IGNORE
        ld      a,$20             ;[M80]ST BIT
DIV2AA: pop     hl                ;[M80]AND THE REST OF REMAINDER
        or      h                 ;[M80]"OR" IN REST
        jp      ROUNDB            ;[M80]USE REMAINDER
DIV2A:  rla                       ;[M80]WE AREN'T, GET OLD CARRY BACK
        ld      a,e               ;[M80]ROTATE EVERYTHING LEFT ONE
        rla                       ;[M80]ROTATE NEXT BIT OF QUOTIENT IN
        ld      e,a
        ld      a,d
        rla
        ld      d,a
        ld      a,c
        rla
        ld      c,a
        add     hl,hl             ;[M80] ROTATE A ZERO INTO RIGHT END OF NUMBER
        ld      a,b               ;[M80] THE HO BYTE, FINALLY!
        rla
        ld      b,a
        ld      a,(FDIVG+1)
        rla
        ld      (FDIVG+1),a
        ld      a,c               ;[M80] ADD ONE TO EXPONENT IF THE FIRST SUBTRACTION
        or      d                 ;[M80]  DID NOT WORK
        or      e
        jr      nz,FDIV1          ;[M80] THIS ISN'T THE CASE
        push    hl                ;[M80] SAVE PART OF NUMBER
        ld      hl,FAC            ;[M80] GET POINTER TO FAC
        dec     (hl)              ;[M80] DECREMENT EXPONENT
        pop     hl                ;[M80] GET NUMBER BACK
        jr      nz,FDIV1          ;[M80] DIVIDE MORE IF NO OVERFLOW OCCURED
        jp      ZERO              ;[M80] UNDERFLOW!!
;[M80] CHECK SPECIAL CASES AND ADD EXPONENTS FOR FMULT, FDIV
MULDIV: ld      a,b               ;[M80] IS NUMBER IN REGISTERS ZERO?
        or      a
        jr      z,MULDV2          ;[M80] IT IS, ZERO FAC AND WE ARE DONE
        ld      a,l               ;[M80] GET ADD OR SUBTRACT FLAG
        ld      hl,FAC            ;[M80] GET POINTER TO EXPONENT
        xor     (hl)              ;[M80] GET EXPONENT
        add     a,b               ;[M80] ADD IN REGISTER EXPONENT
        ld      b,a               ;[M80] SAVE IT
        rra                       ;[M80] CHECK FOR OVERFLOW
        xor     b                 ;[M80] OVERFLOW IF SIGN IS THE SAME AS CARRY
        ld      a,b               ;[M80] GET SUM
        jp      p,MULDV1          ;[M80] WE HAVE OVERFLOW!!
        add     a,$80             ;{M80} PUT EXPONENT IN EXCESS 128
        ld      (hl),a            ;[M80] SAVE IT IN THE FAC
        jp      z,POPHRT          ;[M80] WE HAVE UNDEFLOW!! RETURN.
        call    UNPACK            ;[M80] UNPACK THE ARGUMENTS
        ld      (hl),a            ;[M80] SAVE THE NEW SIGN
        dec     hl                ;[M80] POINT TO EXPONENT
        ret                       ;[M80] ALL DONE, LEAVE HO IN A
        rst     FSIGN             ;[M80] ENTRY FROM EXP, PICK UNDERFLOW IF NEGATIVE
        cpl                       ;[M80] PICK OVERFLOW IF POSITIVE
        pop     hl                ;[M80] DON'T SCREW UP STACK
MULDV1: or      a                 ;[M80] IS ERROR OVERFLOW OR UNDEFLOW?
MULDV2: pop     hl                ;[M80] GET OLD RETURN ADDRESS OFF STACK
        jp      p,ZERO
        jp      OVERR             ;[M80] OVERFLOW
;[M80] MULTIPLY FAC BY 10
MUL10:  call    MOVRF             ;[M80] GET NUMBER IN REGISTERS
        ld      a,b               ;[M80] GET EXPONENT
        or      a                 ;[M80] RESULT IS ZERO IF ARG IS ZERO
        ret     z                 ;[M80] IT IS
        add     a,2               ;[M80] MULTIPLY BY 4 BY ADDING 2 TO EXPONENT
        jp      c,OVERR           ;{M80} OVERFLOW
        ld      b,a               ;[M80] RESTORE EXPONENT
        call    FADD              ;[M80] ADD IN ORIGINAL NUMBER TO GET 5 TIMES IT
        ld      hl,FAC            ;[M80] ADD 1 TO EXPONENT TO MULTIPLY NUMBER BY
        inc     (hl)              ;[M80]  2 TO GET 10 TIMES ORIGINAL NUMBER
        ret     nz                ;[M80] ALL DONE IF NO OVERFLOW
        jp      OVERR             ;{M80} OVERFLOW
;;Jumped to from FSIGN
SIGNC:  ld      a,(FACHO)         ;[M80] GET SIGN OF FACHO, IT IS NON-ZERO
        byte    $FE               ;"CP" AROUND NEXT BYTE
FCOMPS: cpl                       ;ENTRY FROM FCOMP, COMPLEMENT SIGN
ICOMPS: rla                       ;ENTRY FROM ICOMP, PUT SIGN BIT IN CARRY
SIGNS:  sbc     a,a               ;A=0 IF CARRY WAS 0, A=377 IF CARRY WAS 1
        ret     nz                ;RETURN IF NUMBER WAS NEGATIVE
INRART: inc     a                 ;PUT ONE IN A IF NUMBER WAS POSITIVE
        ret                       ;ALL DONE
;;The SGN() Function
SGN:    rst     FSIGN             ;;Get sign of FAC
;[M80] FLOAT THE SIGNED INTEGER IN A
FLOAT:  ld      b,$88             ;[M80] SET EXPONENT CORRECTLY
        ld      de,0              ;[M80] ZERO D,E
;[M80] FLOAT THE SIGNED NUMBER IN B,A,D,E
FLOATR: ld      hl,FAC            ;[M80] GET POINTER TO FAC
        ld      c,a               ;[M80] PUT HO IN C
        ld      (hl),b            ;[M80] PUT EXPONENT IN THE FAC
        ld      b,0               ;[M80] ZERO OVERFLOW BYTE
        inc     hl                ;[M80] POINT TO SIGN
        ld      (hl),128          ;[M80] ASSUME A POSITIVE NUMBER
        rla                       ;[M80] PUT SIGN IN CARRY
        jp      FADFLT            ;[M80] GO AND FLOAT THE NUMBER
;[M80] ABSOLUTE VALUE OF FAC
ABS:    rst     FSIGN             ;[M80] GET THE SIGN OF THE FAC IN A
        ret     p                 ;[M80] IF IT IS POSITIVE, WE ARE DONE
;[M80] NEGATE ANY TYPE VALUE IN THE FAC
NEG:    ld      hl,FACHO          ;[M80[ GET POINTER TO SIGN
        ld      a,(hl)            ;[M80[ GET SIGN
        xor     $80               ;[M80[ COMPLEMENT SIGN BIT
        ld      (hl),a            ;[M80[ SAVE IT
        ret                       ;[M80[ ALL DONE
;[M80] PUT FAC ON STACK, ALTERS D,E
PUSHF:  ex      de,hl             ;[M80] SAVE (HL)
        ld      hl,(FACLO)        ;[M80] GET LO'S
        ex      (sp),hl           ;[M80] SWITCH LO'S AND RET ADDR
        push    hl                ;[M80] PUT RET ADDR BACK ON STACK
        ld      hl,(FACHO)        ;[M80] GET HO'S
        ex      (sp),hl           ;[M80] SWITCH HO'S AND RET ADDR
        push    hl                ;[M80] PUT RET ADDR BACK ON STACK
        ex      de,hl             ;[M80] GET OLD (HL) BACK
        ret                       ;[M80] ALL DONE
MOVFM:  call    MOVRM             ;[M80] MOVE NUMBER FROM MEMORY [(HL)] TO FAC
MOVFR:  ex      de,hl             ;[M80] MOVE REGISTERS (B,C,D,E) TO FAC
        ld      (FACLO),hl        ;[M80] PUT THEM WHERE THEY BELONG
        ld      h,b               ;[M80] GET HO'S IN (HL)
        ld      l,c               ;
        ld      (FACHO),hl        ;[M80] PUT HO'S WHERE THEY BELONG
        ex      de,hl             ;[M80] GET OLD (HL) BACK
        ret                       ;[M80] ALL DONE
MOVRF:  ld      hl,FACLO          ;[M80] MOVE FAC TO REGISTERS (B,C,D,E)
MOVRM:  ld      e,(hl)            ;[M80] GET NUMBER IN REGISTERS (B,C,D,E) FROM MEMORY [(HL)]
        inc     hl                ;[M80] POINT TO MO
        ld      d,(hl)            ;[M80] GET MO, ENTRY FOR BILL
        inc     hl                ;[M80] POINT TO HO
        ld      c,(hl)            ;[M80] GET HO
        inc     hl                ;[M80] POINT TO EXPONENT
        ld      b,(hl)            ;[M80] GET EXPONENT
INXHRT: inc     hl                ;[M80] INC POINTER TO BEGINNING OF NEXT NUMBER
        ret                       ;[M80] ALL DONE
;[M80] MOVE NUMBER FROM FAC TO MEMORY [(HL)]
MOVMF:  ld      de,FACLO          ;GET POINTER TO FAC
;[M80] MOVE NUMBER FROM (DE) TO (HL)
MOVE:   ld      b,4               ;[M80] SET COUNTER
MOVE1:  ld      a,(de)            ;[M80] GET WORD
        ld      (hl),a            ;[M80] PUT IT WHERE IT BELONGS
        inc     de                ;[M80] INCREMENT POINTERS TO NEXT WORD
        inc     hl
        djnz    MOVE1
        ret
;[M80] UNPACK THE FAC AND THE REGISTERS
UNPACK: ld      hl,FACHO          ;[M80] POINT TO HO AND SIGN
        ld      a,(hl)            ;[M80] GET HO AND SIGN
        rlca                      ;[M80] DUPLICATE THE SIGN IN CARRY AND THE LSB
        scf                       ;[M80] RESTORE THE HIDDEN ONE
        rra                       ;[M80] RESTORE THE NUMBER IN A
        ld      (hl),a            ;[M80] SAVE HO
        ccf                       ;[M80] GET THE COMPLEMENT OF THE SIGN
        rra                       ;[M80] GET IT IN THE SIGN BIT
        inc     hl                ;[M80] POINT TO TEMPORARY SIGN BYTE
        inc     hl
        ld      (hl),a            ;[M80] SAVE COMPLEMENT OF SIGN
        ld      a,c               ;[M80] GET HO AND SIGN OF THE REGISTERS
        rlca                      ;[M80] DUPLICATE THE SIGN IN CARRY AND THE LSB
        scf                       ;[M80] RESTORE THE HIDDEN ONE
        rra                       ;[M80] RESTORE THE HO IN A
        ld      c,a               ;[M80] SAVE THE HO
        rra                       ;[M80] GET THE SIGN BACK
        xor     (hl)              ;[M80] COMPARE SIGN OF FAC AND SIGN OF REGISTERS
        ret                       ;[M80] ALL DONE
;[M80] COMPARE TWO NUMBERS
FCOMP:  ld      a,b               ;[M80] CHECK IF ARG IS ZERO
        or      a                 ;
        jp      z,FSIGN           ;
        ld      hl,FCOMPS         ;[M80] WE JUMP TO FCOMPS WHEN WE ARE DONE
        push    hl                ;[M80] PUT THE ADDRESS ON THE STACK
        rst     FSIGN             ;[M80] CHECK IF FAC IS ZERO
        ld      a,c               ;[M80] IF IT IS, RESULT IS MINUS THE SIGN OF ARG
        ret     z                 ;[M80] IT IS
        ld      hl,FACHO          ;[M80] POINT TO SIGN OF FAC
        xor     (hl)              ;[M80] SEE IF THE SIGNS ARE THE SAME
        ld      a,c               ;[M80] IF THEY ARE DIFFERENT, RESULT IS SIGN OF ARG
        ret     m                 ;[M80] THEY ARE DIFFERENT
        call    FCOMP2            ;[M80] CHECK THE REST OF THE NUMBER
        rra                       ;[M80] NUMBERS ARE DIFFERENT, CHANGE SIGN IF
        xor     c                 ;[M80]  BOTH NUMBERS ARE NEGATIVE
        ret                       ;[M80] GO SET UP A
FCOMP2: inc     hl                ;POINT TO EXPONENT
        ld      a,b               ;GET EXPONENT OF ARG
        cp      (hl)              ;COMPARE THE TWO
        ret     nz                ;NUMBERS ARE DIFFERENT
        dec     hl                ;POINT TO HO
        ld      a,c               ;GET HO OF ARG
        cp      (hl)              ;COMPARE WITH HO OF FAC
        ret     nz                ;THEY ARE DIFFERENT
        dec     hl                ;POINT TO MO OF FAC
        ld      a,d               ;GET MO OF ARG
        cp      (hl)              ;COMPARE WITH MO OF FAC
        ret     nz                ;THE NUMBERS ARE DIFFERENT
        dec     hl                ;POINT TO LO OF FAC
        ld      a,e               ;GET LO OF ARG
        sub     (hl)              ;SUBTRACT LO OF FAC
        ret     nz                ;NUMBERS ARE DIFFERENT
        pop     hl                ;NUMBERS ARE THE SAME, DON'T SCREW UP STACK
        pop     hl                ;
        ret                       ;ALL DONE
;[M80] QUICK GREATEST INTEGER FUNCTION
QINT:   ld      b,a               ;[M80] ZERO B,C,D,E IN CASE THE NUMBER IS ZERO
        ld      c,a
        ld      d,a
        ld      e,a
        or      a                 ;[M80] SET CONDITION CODES
        ret     z                 ;[M80] IT IS ZERO, WE ARE DONE
        push    hl                ;[M80] SAVE (HL)
        call    MOVRF             ;[M80] GET NUMBER IN THE REGISTERS
        call    UNPACK            ;[M80] UNPACK THE NUMBER
        xor     (hl)              ;[M80] GET SIGN OF NUMBER
        ld      h,a               ;[M80] DON'T LOSE IT
        call    m,QINTA           ;[M80] SUBTRACT 1 FROM LO IF NUMBER IS NEGATIVE
        ld      a,$98             ;[M80] SEE HOW MANY WE HAVE TO SHIFT TO CHANGE
        sub     b                 ;[M80]  NUMBER TO AN INTEGER
        call    SHIFTR            ;[M80] SHIFT NUMBER TO GET RID OF FRACTIONAL BITS
        ld      a,h               ;[M80] GET SIGN
        rla                       ;[M80] PUT SIGN IN CARRY SO IT WILL NOT BE CHANGED
        call    c,ROUNDA          ;[M80] IF NUMBER WAS NEGATIVE, ADD ONE
        ld      b,0               ;[M80] FORGET THE BITS WE SHIFTED OUT
        call    c,NEGR            ;[M80] NEGATE NUMBER IF IT WAS NEGATIVE
        pop     hl                ;[M80] GET OLD (HL) BACK
        ret                       ;[M80] ALL DONE
QINTA:  dec     de                ;[M80] SUBTRACT ONE FROM C,D,E
        ld      a,d               ;[M80] WE HAVE TO SUBTRACT ONE FROM C IF
        and     e                 ;[M80]  D AND E ARE BOTH ALL ONES
        inc     a                 ;[M80] SEE IF BOTH WERE -1
        ret     nz                ;[M80] THEY WERE NOT, WE ARE DONE
        dec     bc                ;[M80] THIS IS FOR BILL.  C WILL NEVER BE ZERO
        ret                       ;[M80]  (THE MSB WILL ALWAYS BE ONE) SO "DCX    B"
                                  ;[M80] z AND "DCR       C" ARE FUNCTIONALLY EQUIVALENT
;[M80] GREATEST INTEGER FUNCTION
INT:    ld      hl,FAC            ;[M80] GET EXPONENT
        ld      a,(hl)
        cp      $98               ;[M80] SEE IF NUMBER HAS ANY FRACTIONAL BITS
        ld      a,(FACLO)         ;[M80] THE ONLY GUY WHO NEEDS THIS DOESN'T CARE ABOUT THE SIGN
        ret     nc                ;[M80] IT DOES NOT
        ld      a,(hl)            ;[M80] GET EXPONENT BACK
        call    QINT              ;[M80] IT DOES, SHIFT THEM OUT
        ld      (hl),$98          ;[M80] CHANGE EXPONENT SO IT WILL BE CORRECT
        ld      a,e               ;[M80] GET LO
        push    af                ;[M80] SAVE IT
        ld      a,c               ;[M80] NEGATE NUMBER IF IT IS NEGATIVE
        rla                       ;[M80] PUT SIGN IN CARRY
        call    FADFLT            ;[M80] REFLOAT NUMBER
        pop     af                ;[M80] GET LO BACK
        ret                       ;[M80] ALL DONE
;[M80] INTEGER ARITHMETIC ROUTINES
;[M80] INTEGER MULTIPLY FOR MULTIPLY DIMENSIONED ARRAYS
UMULT:  ld      hl,0              ;[M80] ZERO PRODUCT REGISTERS
        ld      a,b               ;[M80] CHECK IF (BC) IS ZERO
        or      c                 ;[M80] IF SO, JUST RETURN, (HL) IS ALREADY ZERO
        ret     z                 ;[M80] THIS IS DONE FOR SPEED
        ld      a,16              ;[M80] THIS IS DONE FOR SPEED
UMULT1: add     hl,hl             ;[M80] ROTATE (HL) LEFT ONE
        jp      c,BSERR           ;{M80} IF OVERFLOW, BAD SUBSCRIPT ERROR
        ex      de,hl
        add     hl,hl             ;[M80] ROTATE (DE) LEFT ONE
        ex      de,hl
        jp      nc,UMULT2         ;[M80] ADD IN (BC) IF HO WAS 1
        add     hl,bc
        jp      c,BSERR
UMULT2: dec     a                 ;[M80] CHECK FOR OVERFLOW
        jp      nz,UMULT1         ;[M80] SEE IF DONE
        ret
;[M80] FLOATING POINT INPUT ROUTINE
;[M80] ALTERS ALL REGISTERS
;[M80] THE NUMBER IS LEFT IN FAC
;[M80] AT ENTRY, (HL) POINTS TO THE FIRST CHARACTER IN A TEXT BUFFER.
;[M80] THE FIRST CHARACTER IS ALSO IN A.  WE PACK THE DIGITS INTO THE FAC
;[M80] AS AN INTEGER AND KEEP TRACK OF WHERE THE DECIMAL POINT IS.
;[M80] C IS 377 IF WE HAVE NOT SEEN A DECIMAL POINT, 0 IF WE HAVE.
;[M80] B IS THE NUMBER OF DIGITS AFTER THE DECIMAL POINT.
;[M80] AT THE END, B AND THE EXPONENT (IN E) ARE USED TO DETERMINE HOW MANY
;[M80] TIMES WE MULTIPLY OR DIVIDE BY TEN TO GET THE CORRECT NUMBER.
FIN:    cp      '-'               ;[M80] SEE IF NUMBER IS NEGATIVE
        push    af                ;[M80] SAVE SIGN
        jr      z,FIN1
        cp      '+'               ;[M80] IGNORE MINUS SIGN
        jr      z,FIN1            ;[M80] IGNORE A LEADING SIGN
        dec     hl                ;[M80] SET CHARACTER POINTER BACK ONE
FIN1:   call    ZERO              ;{M80} GO ZERO THE FAC
        ld      b,a
        ld      d,a               ;;[D,E] = $0000
        ld      e,a
        cpl
        ld      c,a               ;;[B,C] = $00FF
;[M80] HERE TO CHECK FOR A DIGIT, A DECIMAL POINT, "E" OR "D"
FINC:   rst     CHRGET            ;[M80] GET THE NEXT CHARACTER OF THE NUMBER
        jp      c,FINDIG          ;[M80] WE HAVE A DIGIT
        cp      '.'               ;[M80] CHECK FOR A DECIMAL POINT
        jp      z,FINDP           ;[M80] WE HAVE ONE, I GUESS
        cp      'e'
        jp      z,FINC1
        cp      'E'               ;[M80] CHECK FOR A SINGLE PRECISION EXPONENT
        jp      nz,FINE
FINC1:  rst     CHRGET
        call    MINPLS
FINEC:  rst     CHRGET            ;[M80] GET THE NEXT CHARATER
        jp      c,FINEDG          ;[M80] PACK THE NEXT DIGIT INTO THE EXPONENT
        inc     d                 ;[M80] IT WAS NOT A DIGIT, PUT THE CORRECT SIGN ON
        jp      nz,FINE           ;[M80]  THE EXPONENT, IT IS POSITIVE
        xor     a                 ;[M80] THE EXPONENT IS NEGATIVE
        sub     e                 ;[M80] NEGATE IT
        ld      e,a               ;[M80] SAVE IT AGAIN
        inc     c
;[M80] HERE TO SET THE DECIMAL POINT FLAG
FINDP:  inc     c                 ;[M80] SET THE FLAG
        jp      z,FINC            ;[M80] CONTINUE LOOKING FOR DIGITS
;[M80] HERE TO FINISH UP THE NUMBER
FINE:   push    hl                ;[M80] SAVE THE TEXT POINTER
        ld      a,e               ;[M80] FIND OUT HOW MANY TIMES WE HAVE TO MULTIPLY
        sub     b                 ;[M80]  OR DIVIDE BY TEN
;[M80] HERE TO MULTIPLY OR DIVIDE BY TEN THE CORRECT NUMBER OF TIMES
FINE2:  call    p,FINMUL          ;[M80] MULTIPLY IF WE HAVE TO
        jp      p,FINE3
        push    af
        call    DIV10             ;[M80] DIVIDE IF WE HAVE TO
        pop     af                ;[M80] GET THE SIGN
        inc     a
FINE3:  jp      nz,FINE2          ;[M80] MULTIPLY OR DIVIDE AGAIN IF WE ARE NOT DONE
;[M80] HERE TO PUT THE CORRECT SIGN ON THE NUMBER
        pop     de                ;[M80] GET THE TEXT POINTER
        pop     af                ;[M80] GET THE SIGN
        call    z,NEG             ;[M80] NEGATE IF NECESSARY
        ex      de,hl             ;[M80] GET THE TEXT POINTER IN (HL)
        ret
;[M80] THIS SUBROUTINE MULIPLIES BY TEN ONCE.
FINMUL: ret     z                 ;[M80] RETURN IF EXPONENT IS ZERO, ENTRY FROM FOUT
FINMLT: push    af
        call    MUL10             ;[M80] MULTIPLY BY 10.0
        pop     af
        dec     a                 ;[M80] DECREASE IT
        ret                       ;[M80] ALL DONE
;[M80] HERE TO PACK THE NEXT DIGIT OF THE NUMBER INTO THE FAC
;[M80] WE MULTIPLY THE FAC BY TEN AND ADD IN THE NEXT DIGIT
FINDIG: push    de                ;[M80] SAVE EXPONENT INFORMATION
        ld      d,a               ;[M80] INCREMENT DECIMAL PLACE COUNT IF WE ARE
        ld      a,b               ;[M80]  PAST THE DECIMAL POINT
        adc     a,c               ;
        ld      b,a               ;
        push    bc                ;[M80] SAVE DECIMAL POINT INFORMATION
        push    hl                ;[M80] SAVE TEXT POINTER
        push    de                ;
        call    MUL10             ;
        pop     af                ;[M80] GET THE DIGIT
        sub     '0'               ;[M80] CONVERT IT TO ASCII
        call    FINLOG            ;[M65] ADD IT IN
        pop     hl                ;
        pop     bc                ;
        pop     de                ;
        jp      FINC              ;
FINLOG: call    PUSHF             ;[M65] SAVE FAC FOR LATER
        call    FLOAT             ;[M65] FLOAT THE VALUE IN ACCA
FADDT:  pop     bc                ;[M80] GET ARG IN REGISTERS, ENTRY TO FADD IF
        pop     de                ;[M80]  ARGUMENT IS ON STACK.  JUMP TO FADD
        jp      FADD              ;[M80] ADD IT IN
FINEDG: ld      a,e               ;EXPONENT DIGIT -- MULTIPLY EXPONENT BY 10
        rlca                      ;FIRST BY 4
        rlca
        add     a,e               ;ADD 1 TO MAKE 5
        rlca                      ;NOW DOUBLE TO GET 10
        add     a,(hl)            ;ADD IT IN
        sub     '0'               ;SUBTRACT OFF ASCII CODE
        ld      e,a               ;STORE EXPONENT
        jp      FINEC
;[M80] FLOATING POINT OUTPUT ROUTINE
INPRT:  push    hl                ;[M80] ENTRY TO LINPRT
        ld      hl,INTXT          ;[M80] SAVE LINE NUMBER
        call    STROUT            ;[M80] PRINT MESSAGE
        pop     hl
;[M80] PRINT THE 2 BYTE NUMBER IN H,L
LINPRT: ld      de,STROUI
        push    de
;; Convert Unsigned Word in [HL] to String
        ex      de,hl
        xor     a
        ld      b,$98
        call    FLOATR
;[M80] FLOATING OUTPUT OF FAC
FOUT:   ld      hl,FBUFFR+1       ;[M80] GET A POINTER INTO FBUFFR
        push    hl                ;{M80} SAVE IT
        rst     FSIGN
        ld      (hl),' '          ;[M80] PUT A SPACE FOR POSITIVE NUMBERS IN THE BUFFER
        jp      p,FOUT2           ;[M80] IF WE HAVE A NEGATIVE NUMBER, NEGATE IT
        ld      (hl),'-'          ;[M80]  AND PUT A MINUS SIGN IN THE BUFFER
FOUT2:  inc     hl                ;[M80] POINT TO WHERE THE NEXT CHARACTER GOES
        ld      (hl),'0'          ;[M80] PUT A ZERO IN THE BUFFER IN CASE THE NUMBER IS ZERO
        jp      z,FOUTZR          ;[M80] IF THE NUMBER IS ZERO, FINISH IT UP
        push    hl                ;[M80] SAVE THE BUFFER POINTER
        call    m,NEG             ;[M80] NEGATE THE NUMBER
        xor     a                 ;[M80] ZERO THE EXPONENT
        push    af                ;[M80] SAVE IT
        call    FOUNVC            ;[M80] IS THE FAC TOO BIG OR TOO SMALL?
FOUNV1: ld      bc,$9143          ;
        ld      de,$4FF8          ;[M80] GET 99999.95 TO SEE IF THE FAC IS BIG
        call    FCOMP             ;[M80]  ENOUGH YET
        or      a                 ;[M80] GO DO THE CHECK
        jp      po,FOUTCS         ;[M80] IT ISN'T ANY MORE, WE ARE DONE
        pop     af                ;[M80] IT IS, MULTIPLY BY TEN
        call    FINMLT            ;
        push    af                ;[M80] SAVE THE EXPONENT AGAIN
        jp      FOUNV1            ;[M80] NOW SEE IF IT IS BIG ENOUGH
FOUNV2: call    DIV10             ;[M80] THE FAC IS TOO BIG, DIVIDE IT BY TEN
        pop     af                ;
        inc     a                 ;
        push    af                ;
        call    FOUNVC            ;[M80] SEE IF THE FAC IS SMALL ENOUGH
FOUTCS: call    FADDH             ;[M80] ROUND NUMBER TO NEAREST INTEGER
        inc     a                 ;[M80] MAKE A NON-ZERO, SINCE NUMBER IS POSITIVE AND NON-ZERO
        call    QINT              ;[M80] GET INTEGER PART IN C,D,E
        call    MOVFR             ;[M80] SAVE NUMBER IN FAC
        ld      bc,$0306          ;;[B] = 3, [C] = 6
        pop     af                ;
        add     a,c               ;
        inc     a                 ;
        jp      m,FOUCDC          ;
        cp      8                 ;
        jp      nc,FOUCDC         ;
        inc     a                 ;
        ld      b,a               ;
        ld      a,2               ;
FOUCDC: dec     a                 ;
        dec     a                 ;
        pop     hl                ;[M80] GET THE BUFFER POINTER BACK
        push    af                ;
        ld      de,FOSTBL         ;
        dec     b                 ;
        jp      nz,FOUTED         ;
        ld      (hl),'.'          ;[M80] PUT IN D.P.
        inc     hl                ;[M80] POINT TO NEXT BUFFER POSTION
        ld      (hl),'0'          ;
        inc     hl                ;
FOUTED: dec     b                 ;[M80] ENTRY TO PUT A DECIMAL POINT IN THE BUFFER
        ld      (hl),'.'          ;{M80} PUT THE DECIMAL POINT IN
        call    z,INXHRT          ;
        push    bc                ;[M80[ SAVE THE DECIMAL POINT AND COMMA COUNTS
        push    hl                ;[M80[ SAVE THE BUFFER POINTER
        push    de                ;[M80[ SAVE (DE)
        call    MOVRF             ;{M80} GET NUMBER IN THE REGISTERS
        pop     hl                ;[M80] GET THE BUFFER POINTER BACK
        ld      b,'0'-1           ;[M80] SET UP THE COUNT FOR THE DIGIT
FOUCD2: inc     b                 ;[M80] INCREMENT THE DIGIT COUNT
        ld      a,e               ;{M80} SUBTRACT THE TWO NUMBERS
        sub     (hl)              ;;Subtract LO
        ld      e,a               ;
        inc     hl                ;
        ld      a,d               ;
        sbc     a,(hl)            ;;Subtract MO
        ld      d,a               ;
        inc     hl                ;
        ld      a,c               ;
        sbc     a,(hl)            ;;Subtract HO
        ld      c,a               ;
        dec     hl                ;
        dec     hl                ;;Backup Pointer
        jp      nc,FOUCD2         ;[M80] IF NOT LESS THAN THE POWER OF TEN, SUBTRACT AGAIN
        call    FADDA             ;[M80] ADD THE TWO NUMBERS
        inc     hl
        call    MOVFR             ;{M80} PUT REGISTERS IN THE FAC
        ex      de,hl             ;[M80] PUT THE POWER OF TEN POINTER IN (DE).
        pop     hl                ;[M80] GET THE BUFFER POINTER BACK
        ld      (hl),b            ;[M80] PUT THE DIGIT INTO THE BUFFER
        inc     hl                ;[M80] INCREMENT THE BUFFER POINTER
        pop     bc                ;[M80] GET THE DECIMAL POINT AND COMMA COUNTS
        dec     c                 ;[M80] HAVE WE PRINTED THE LAST DIGIT?
        jp      nz,FOUTED         ;[M80] NO, GO DO THE NEXT ONE
        dec     b                 ;
        jp      z,FOFLDN          ;
FOFRS2: dec     hl                ;[M80] MOVE BACK TO THE LAST CHARACTER
        ld      a,(hl)            ;[M80] GET IT AND SEE IF IT WAS ZERO
        cp      '0'               ;
        jp      z,FOFRS2          ;[M80] IT WAS, CONTINUE SUPPRESSING
        cp      '.'               ;[M80] HAVE WE SUPPRESSED ALL THE FRACTIONAL DIGITS?
        call    nz,INXHRT         ;[M80] YES, IGNORE THE DECIMAL POINT ALSO
FOFLDN: pop     af                ;[M80] GET THE EXPONENT BACK
        jp      z,FOUTDN          ;[M80] WE ARE DONE IF WE ARE IN FIXED POINT NOTATION
        ld      (hl),'E'          ;
        inc     hl                ;
        ld      (hl),'+'          ;[M80] A PLUS IF POSITIVE
        jp      p,FOUCE1          ;
        ld      (hl),'-'          ;[M80] A MINUS IF NEGATIVE
        cpl                       ;[M80] NEGATE EXPONENT
        inc     a                 ;
;[M80] CALCULATE THE TWO DIGIT EXPONENT
FOUCE1: ld      b,'0'-1           ;[M80] INITIALIZE TEN'S DIGIT COUNT
FOUCE2: inc     b                 ;[M80] INCREMENT DIGIT
        sub     10                ;[M80] SUBTRACT TEN
        jp      nc,FOUCE2         ;[M80] DO IT AGAIN IF RESULT WAS POSITIVE
        add     a,'0'+10          ;[M80] ADD BACK IN TEN AND CONVERT TO ASCII
        inc     hl                ;[M80] PUT THE EXPONENT IN THE BUFFER
        ld      (hl),b            ;[M80] PUT TEN'S DIGIT OF EXPONENT IN BUFFER
FOUTZR: inc     hl                ;[M80] WHEN WE JUMP TO HERE, A IS ZERO
        ld      (hl),a            ;[M80] PUT ONE'S DIGIT IN BUFFER
        inc     hl                ;[M80] INCREMENT POINTER
;HERE TO FINISH UP PRINTING A FREE FORMAT ZERO
FOUTDN: ld      (hl),c            ;[M80] PUT A ZERO AT THE END OF THE NUMBER
        pop     hl                ;
        ret                       ;
FOUNVC: ld      bc,$9474          ;
        ld      de,$23F7          ;[M80] GET 999999.5 TO SEE IF THE FAC IS TOO BIG
        call    FCOMP             ;
        or      a                 ;
        pop     hl                ;
        jp      po,FOUNV2         ;[M80] GO DO THE CHECK
        jp      (hl)              ;[M80] IT ISN'T TOO BIG, JUST RETURN
FHALF:  byte    $00,$00,$00,$80   ;[M65] 1/2
        ld      b,b               ;;Orphan Code or Remnants of Unused Constant?
        ld      b,d
        rrca
;[M80] SINGLE PRECISION POWER OF TEN TABLE
FOSTBL: byte    $A0,$86,$01       ;[M80] 1E5
        byte    $10,$27,$00       ;[M80] 1E4
        byte    $E8,$03,$00       ;[M80] 1000
        byte    $64,$00,$00       ;[M80] 100
        byte    $0A,$00,$00       ;[M80] 10
        byte    $01,$00,$00       ;[M80] 1
;;Push Address of NEG Routine on to Stack
PSHNEG: ld      hl,NEG            ;[M80] GET THE ADDRESS OF NEG
        ex      (sp),hl           ;[M80] SWITCH RET ADDR AND ADDR OF NEG
        jp      (hl)              ;[M80] RETURN, THE ADDRESS OF NEG IS ON THE STACK
;[M80] SQUARE ROOT FUNCTION: WE USE SQR(X)=X^.5
SQR:    call    PUSHF             ;[M80] SAVE ARG X
        ld      hl,FHALF          ;[M80] GET 1/2
        call    MOVFM             ;[M80] SQR(X)=X^.5
FPWRT:  pop     bc                ;[M80] GET ARG IN REGISTERS, ENTRY TO FPWR IF
        pop     de                ;[M80]  ARGUMENT IS ON STACK.  FALL INTO FPWR
;[M80] EXPONENTIATION    ---    X^Y
FPWR:   rst     FSIGN             ;[M80] SEE IF Y IS ZERO
        ld      a,b               ;[M80] SEE IF X IS ZERO
        jp      z,EXP             ;[M80] IT IS, RESULT IS ONE
        jp      p,POSEXP          ;[M80] POSITIVE EXPONENT
        or      a                 ;[M80] IS IT ZERO TO MINUS POWER?
        jp      z,DV0ERR          ;[M80] GIVE DIV BY ZERO AND CONTINUE
POSEXP: or      a                 ;
        jp      z,ZERO0           ;[M80]IT IS, RESULT IS ZERO
        push    de                ;[M80] SAVE X ON STACK
        push    bc                ;
        ld      a,c               ;[M80] CHECK THE SIGN OF X
        or      $7F               ;[M80] TURN THE ZERO FLAG OFF
        call    MOVRF             ;[M80] GET Y IN THE REGISTERS
        jp      p,FPWR1           ;[M80] NO PROBLEMS IF X IS POSITIVE
        push    af                ;
        ld      a,(FAC)           ;
        cp      $99               ;
        jr      c,FPWRT1          ;
        pop     af                ;
        jr      FPWR1             ;
FPWRT1: pop     af                ;
        push    de                ;
        push    bc                ;[M80] SAVE Y
        call    INT               ;[M80] SEE IF Y IS AN INTEGER
        pop     bc                ;
        pop     de                ;[M80] GET Y BACK
        push    af                ;[M80] SAVE LO OF INT FOR EVEN AND ODD INFORMATION
        call    FCOMP             ;[M80] SEE IF WE HAVE AN INTEGER
        pop     hl                ;[M80] GET EVEN-ODD INFORMATION
        ld      a,h               ;[M80] PUT EVEN-ODD FLAG IN CARRY
        rra                       ;
FPWR1:  pop     hl                ;[M80] GET X BACK IN FAC
        ld      (FACHO),hl        ;[M80] STORE HO'S
        pop     hl                ;[M80] GET LO'S OFF STACK
        ld      (FACLO),hl        ;[M80] STORE THEM IN FAC
        call    c,PSHNEG          ;[M80] NEGATE NUMBER AT END IF Y WAS ODD
        call    z,NEG             ;[M80] NEGATE THE NEGATIVE NUMBER
        push    de                ;[M80] SAVE Y AGAIN
        push    bc                ;
        call    LOG               ;[M80] COMPUTE  EXP(Y*LOG(X))
        pop     bc                ;
        pop     de                ;[M80] IF X WAS NEGATIVE AND Y NOT AN INTEGER THEN
        call    FMULT             ;[M80]  LOG WILL BLOW HIM OUT OF THE WATER
;[M80] THE FUNCTION EXP(X) CALCULATES e^X WHERE e=2.718282
EXP:    ld      bc,$8138          ;[M65] LOG(e) BASE 2
        ld      de,$AA3B          ;[M80] GET LOG2(e)
        call    FMULT             ;[M80] y=FAC*LOG2(e)
        ld      a,(FAC)           ;[M80] MUST SEE IF TOO LARGE
        cp      $88               ;[M80] ABS .GT. 128?
        jr      nc,EXP100         ;[M80] IF SO OVERFLOW
        cp      $68               ;[M80] IF TOO SMALL ANSWER IS 1
        jr      c,EXP200          ;
        call    PUSHF             ;[M80] SAVE y
        call    INT               ;[M80] DETERMINE INTEGER POWER OF 2
        add     a,$81             ;[M80] INTEGER WAS RETURNED IN A, BIAS IS 201
        pop     bc                ;
        pop     de                ;[M80] RECALL y
        jr      z,EXP110          ;[M80] OVERFLOW
        push    af                ;[M80] SAVE EXPONENT
        call    FSUB              ;[M80] FAC=y-INT(y)
        ld      hl,EXPBCN         ;[M80] WILL USE HART 1302 POLY. EVAL NOW
        call    POLY              ;[M80] COMPUTE 2^[y-INT(y)]
        pop     bc                ;[M80] INTEGER POWER OF 2 EXPONENT
        ld      de,0              ;[M80] NOW HAVE FLOATING REPRESENTATION
        ld      c,d               ;[M80] OF INT(y) IN (BCDE)
        jp      FMULT             ;[M80] MULTIPLY BY 2^[y-INT(y)] AND RETURN
EXP100: call    PUSHF             ;[M80] IF NEG. THEN JUMP TO ZERO
EXP110: ld      a,(FACHO)         ;
        or      a                 ;[M80] OVERFLOW IF PLUS
        jp      p,EXP115          ;[M80] NEED STACK RIGHT
        pop     af                ;
        pop     af                ;
        jp      ZERO              ;[M80] GO ZERO THE FAC
EXP115: jp      OVERR             ;[M80] OVERFLOW
EXP200: ld      bc,$8100          ;
        ld      de,$0000          ;[M80] 1.
        jp      MOVFR             ;
;*************************************************************
;       Hart 1302 polynomial coefficients
;*************************************************************
EXPBCN  byte    7                 ;[M80] DEGREE + 1
        byte    $7C,$88,$59,$74   ;[M80] .00020745577403-
        byte    $E0,$97,$26,$77   ;[M80] .00127100574569-
        byte    $C4,$1D,$1E,$7A   ;[M80] .00965065093202+
        byte    $5E,$50,$63,$7C   ;[M80] .05549656508324+
        byte    $1A,$FE,$75,$7E   ;[M80] .24022713817633-
        byte    $18,$72,$31,$80   ;[M80] .69314717213716+
        byte    $00,$00,$00,$81   ;[M80] 1.0
;[M80] POLYNOMIAL EVALUATOR AND THE RANDOM NUMBER GENERATOR
;[M80] EVALUATE P(X^2)*X
;[M80] POINTER TO DEGREE+1 IS IN (HL)
;[M80] THE CONSTANTS FOLLOW THE DEGREE
;[M80] CONSTANTS SHOULD BE STORED IN REVERSE ORDER, FAC HAS X
;[M80] WE COMPUTE:
;[M80]  C0*X+C1*X^3+C2*X^5+C3*X^7+...+C(N)*X^(2*N+1)
POLYX:  call    PUSHF             ;SAVE X
        ld      de,FMULTT         ;PUT ADDRESS OF FMULTT ON STACK SO WHEN WE
        push    de                ; RETURN WE WILL MULTIPLY BY X
        push    hl                ;SAVE CONSTANT POINTER
        call    MOVRF             ;SQUARE X
        call    FMULT             ;
        pop     hl                ;GET CONSTANT POINTER
;[M80] POLYNOMIAL EVALUATOR
POLY:   call    PUSHF             ;[M80] SAVE X
        ld      a,(hl)            ;[M80] GET DEGREE
        inc     hl                ;[M80] INCREMENT POINTER TO FIRST CONSTANT
        call    MOVFM             ;[M80] MOVE FIRST CONSTANT TO FAC
        byte    $06               ;[M80] "MVI  B" OVER NEXT BYTE
POLY1:  pop     af                ;[M80] GET DEGREE
        pop     bc                ;
        pop     de                ;[M80] GET X
        dec     a                 ;[M80] ARE WE DONE?
        ret     z                 ;[M80] YES, RETURN
        push    de                ;
        push    bc                ;[M80] NO, SAVE X
        push    af                ;[M80] SAVE DEGREE
        push    hl                ;[M80] SAVE CONSTANT POINTER
        call    FMULT             ;[M80] EVALUATE THE POLY, MULTIPLY BY X
        pop     hl                ;[M80] GET LOCATION OF CONSTANTS
        call    MOVRM             ;[M80] GET CONSTANT
        push    hl                ;[M80] STORE LOCATION OF CONSTANTS SO FADD AND FMULT
        call    FADD              ;[M80]  WILL NOT SCREW THEM UP, ADD IN CONSTANT
        pop     hl                ;[M80] MOVE CONSTANT POINTER TO NEXT CONSTANT
        jr      POLY1             ;[M80] SEE IF DONE
;[M80] PSUEDO-RANDOM NUMBER GENERATOR
;[M80] IF ARG=0, THE LAST RANDOM NUMBER GENERATED IS RETURNED
;[M80] IF ARG .LT. 0, A NEW SEQUENCE OF RANDOM NUMBERS IS STARTED
;[M80]  USING THE ARGUMENT
;[M80] TO FORM THE NEXT RANDOM NUMBER IN THE SEQUENCE, WE MULTIPLY THE
;[M80] PREVIOUS RANDOM NUMBER BY A RANDOM CONSTANT, AND ADD IN ANOTHER
;[M80] RANDOM CONSTANT.  THEN THE HO AND LO BYTES ARE SWITCHED, THE
;[M80] EXPONENT IS PUT WHERE IT WILL BE SHIFTED IN BY NORMAL, AND THE
;[M80] EXPONENT IN THE FAC SET TO 200 SO THE RESULT WILL BE LESS THAN 1.
;[M80] THIS IS THEN NORMALIZED AND SAVED FOR THE NEXT TIME.
;[M80] THE HO AND LO BYTES WERE SWITCHED SO WE HAVE A RANDOM CHANCE OF
;[M80] GETTING A NUMBER LESS THAN OR GREATER THAN .5
;;;Code Change: Uses the Random Constants Table in ROM at $01A5 instead
;;;of the copy at $3821 in RAM. This frees up the 32 bytes of System RAM 
;;;at $3821 through $3840. The table still gets copied to RAM by COLDST,
;;;but that can be overwritten with no adverse effects.
RND:    rst     FSIGN             ;[M80] GET SIGN OF ARG                    
        ld      hl,RNDCNT+1       ;;Get Starting Address of Permutation Table
        jp      m,RNDSTR          ;[M80] START NEW SEQUENCE IF NEGATIVE
        ld      hl,RNDX           ;[M80] GET LAST NUMBER GENERATED
        call    MOVFM             ;
        ld      hl,RNDCNT+1       ;
        ret     z                 ;[M80] RETURN LAST NUMBER GENERATED IF ZERO
        add     a,(hl)            ;[M80] GET COUNTER INTO CONSTANTS AND ADD ONE
        and     7                 ;;Modulo 8 Counter                          Original Code
        call    RNDSTL            ;;Patch to get RNDTBL into HL               187A  ld      b,0              
                                  ;;                                          187B              
        nop                       ;;                                          187C  ld      (hl),a           
        add     a,a               ;;                                          187D  inc     hl    
        add     a,a               ;;A = A * 4
        ld      c,a               ;;BC = Counter * 4 + Counter
        add     hl,bc             ;;HL = RNDTAB + BC
        call    MOVRM             ;;Copy RNDTAB entry into RAM
        call    FMULT             ;;and Multiply it by the Last Random Number
        ld      a,(RNDCNT)        ;
        inc     a                 ;
        and     3                 ;
        ld      b,0               ;
        cp      1                 ;
        adc     a,b               ;
        ld      (RNDCNT),a        ;
        ld      hl,RNDTB2-4       ;
        add     a,a               ;
        add     a,a               ;
        ld      c,a               ;
        add     hl,bc             ;
        call    FADDS             ;
RND1:   call    MOVRF             ;[M80] SWITCH HO AND LO BYTES,
        ld      a,e               ;[M80] GET LO
        ld      e,c               ;[M80] PUT HO IN LO BYTE
        xor     $4F               ;
        ld      c,a               ;[M80] PUT LO IN HO BYTE
        ld      (hl),128          ;[M80] MAKE RESULT POSITIVE
        dec     hl                ;[M80] GET POINTER TO EXPONENT
        ld      b,(hl)            ;[M80] PUT EXPONENT IN OVERFLOW POSITION
        ld      (hl),128          ;[M80] SET EXP SO RESULT WILL BE BETWEEN 0 AND 1
        ld      hl,RNDCNT-1       ;
        inc     (hl)              ;[M80] INCREMENT THE PERTUBATION COUNT
        ld      a,(hl)            ;[M80] SEE IF ITS TIME
        sub     $AB               ;
        jr      nz,NTPTRB         ;
        ld      (hl),a            ;[M80] ZERO THE COUNTER
        inc     c                 ;
        dec     d                 ;
        inc     e                 ;
NTPTRB: call    NORMAL            ;[M80] NORMALIZE THE RESULT
        ld      hl,RNDX           ;[M80] SAVE RANDOM NUMBER GENERATED FOR NEXT
        jp      MOVMF             ;[M80]  TIME
RNDSTR: ld      (hl),a            ;[M80] ZERO THE COUNTERS
        dec     hl                ;
        ld      (hl),a            ;
        dec     hl                ;
        ld      (hl),a            ;
        jr      RND1              ;
RNDTB2: byte    $68,$B1,$46,$68
        byte    $99,$E9,$92,$69
        byte    $10,$D1,$75,$68
;[M80] COSINE FUNCTION
COS:    ld      hl,PI2            ;[M80] ADD PI/2 TO FAC
        call    FADDS             ;
;[M80] SINE FUNCTION
SIN:    ld      a,(FAC)           ;[M80] WILL SEE IF .LT.2^-10
        cp      $77               ;[M80] AND IF SO SIN(X)=X
        ret     c                 ;
        ld      a,(FACHO)         ;
        or      a                 ;
        jp      p,SIN1            ;[M65] FIRST QUADRANT
        and     07FH              ;
        ld      (FACHO),a         ;
        ld      de,NEG            ;;Return to NEG
        push    de                ;
SIN1:   ld      bc,$7E22          ;
        ld      de,$F983          ;[M80] WILL CALCULATE X=FAC/(2*PI)
        call    FMULT             ;
        call    PUSHF             ;[M80] SAVE X
        call    INT               ;[M80] FAC=INT(X)
        pop     bc                ;
        pop     de                ;[M80] FETCH X TO REGISTERS
        call    FSUB              ;[M80] FAC=X-INT(X)
        ld      bc,$7F00          ;
        ld      de,$0000          ;[M80] GET 1/4
        call    FCOMP             ;[M80] FAC=FAC-1/4
        jp      m,SIN2A           ;
        ld      bc,$7F80          ;
        ld      de,$0000          ;[M80] -1/4
        call    FADD              ;
        ld      bc,$8080          ;[M80] -1/2
        ld      de,$0000          ;
        call    FADD              ;[M80] X=X-1/2
        rst     FSIGN             ;[M80] MAKE SURE IF QUADRANTS II,IV
        call    p,NEG             ;[M80] WE WORK WITH 1/4-X
        ld      bc,$7F00          ;
        ld      de,$0000          ;[M80] 1/4
        call    FADD              ;
        call    NEG               ;
SIN2A:  ld      a,(FACHO)         ;[M80] MUST REDUCE TO [0,1/4]
        or      a                 ;[M80] SIGN IN PSW
        push    af                ;[M80] SAVE FOR POSSIBLE NEG. AFTER CALC
        jp      p,SIN3            ;
        xor     $80               ;
        ld      (FACHO),a         ;[M80] NOW IN [0,1/4]
SIN3:   ld      hl,SINCON         ;[M80] POINT TO HART COEFFICIENTS
        call    POLYX             ;[M80] DO POLY EVAL
        pop     af                ;[M80] NOW TO DO SIGN
        ret     p                 ;[M80] OK IF POS
        ld      a,(FACHO)         ;[M80] FETCH SIGN BYTE
        xor     080H              ;[M80] MAKE NEG
        ld      (FACHO),a         ;[M80] REPLACE SIGN
        ret
PI2:    byte    $DB,$0F,$49,$81   ;[M80] PI/2
        byte    $00,$00,$00,$7F   ;[M80] 1/4
;[M80] HART ALGORITHM 3341 CONSTANTS
SINCON: byte    5                 ;[M80] DEGREE
        byte    $FB,$D7,$1E,$86   ;[M80] .1514851E-3
        byte    $65,$26,$99,$87   ;[M80] -.4673767E-2
        byte    $58,$34,$23,$87   ;[M80] .7968968E-1
        byte    $E1,$5D,$A5,$86   ;[M80] -.6459637
        byte    $DB,$0F,$49,$83   ;[M80] 1.570796
;[M80] TANGENT FUNCTION
TAN:    call    PUSHF             ;[M80] SAVE ARG
        call    SIN               ;[M80]    TAN(X)=SIN(X)/COS(X)
        pop     bc                ;[M80] GET X OFF STACK
        pop     hl                ;[M80] PUSHF SMASHES (DE)
        call    PUSHF             ;
        ex      de,hl             ;[M80] GET LO'S WHERE THEY BELONG
        call    MOVFR             ;
        call    COS               ;
        jp      FDIVT             ;
;ARCTANGENT FUNCTION
ATN:    rst     HOOKDO            ;;execute hook routine 15 (ATN)
        byte    14                ;;if not implemented
        jp      SNERR             ;;  generate SYNTAX error
;;Execute OUTCHR
OUTDO:  rst     HOOKDO            ;;execute hook routine 13 (OUTDOX)
        byte    13                ;
OUTCON: push    af                ;
        ld      a,(PRTFLG)        ;[M80] SEE IF WE WANT TO TALK TO LPT
        or      a                 ;[M80] TEST BITS
        jp      z,TTYPOP          ;[M80] IF ZERO THEN NOT
;;Print character in [A] to printer
        pop     af                ;
        push    af                ;
        cp      9                 ;[M80] TAB
        jr      nz,NOTABL         ;[M80] NO
;;Print spaces until next Tab Stop is reached
MORSPL: ld      a,' '             ;[M80] GET SPACE
        rst     OUTCHR            ;[M80] SEND IT
        ld      a,(LPTPOS)        ;[M80] GET CURRENT PRINT POSIT
        and     7                 ;[M80] AT TAB STOP?
        jr      nz,MORSPL         ;[M80] GO BACK IF MORE TO PRINT
        pop     af                ;[M80] POP OFF CHAR
        ret                       ;[M80] RETURN
NOTABL: pop     af                ;[M80] GET CHAR BACK
        push    af                ;[M80] SAVE AGAIN
        sub     13                ;[M80] IF FUNNY CONTROL CHAR, (LF) DO NOTHING
        jr      z,ZERLP1          ;
        jr      c,LPTPOP          ;[M80] JUST PRINT CHAR
        ld      a,(LPTPOS)        ;[M80] GET POSIT
        inc     a                 ;
        cp      132               ;
        call    z,PRINTW          ;
ZERLP1: ld      (LPTPOS),a        ;;Save print pos
LPTPOP: pop     af                ;;Print character on stack
LPTCHR: jp      LPTOUT            ;;Print raw character to printer
FINLPT: xor     a                 ;[M80] RESET PRINT FLAG SO
        ld      (PRTFLG),a        ;[M80] OUTPUT GOES TO TERMINAL
        ld      a,(LPTPOS)        ;[M80] GET CURRENT LPT POSIT
        or      a                 ;[M80] ON LEFT HAND MARGIN ALREADY?
        ret     z                 ;[M80] YES, RETURN
PRINTW: ld      a,13              ;[M80] PUT OUT CRLF
        call    LPTCHR            ;
        ld      a,10              ;
        call    LPTCHR            ;
        xor     a                 ;[M80] ZERO LPTPOS
        ld      (LPTPOS),a        ;
        ret                       ;[M80] DONE
TTYPOP: pop     af                ;Print character on stack to screen
        jp      TTYCHR            ;
;[M80] INCHR, TRYIN - CHARACTER INPUT ROUTINES
INCHR:  call    TRYIN             ;;Get Character from Keyboard
        ret                       ;
;;CRDONZ Only does a CR if TTYPOS is not Zero.
CRDONZ: ld      a,(TTYPOS)        ;[M80] GET CURRENT TTYPOS
        or      a                 ;[M80] SET CC'S
        ret     z                 ;[M80] IF ALREADY ZERO, RETURN
        jr      CRDO              ;[M80] DO CR
;;Terminate BUF and Print CR
FININL: ld      (hl),0            ;[M80] PUT A ZERO AT THE END OF BUF
        ld      hl,BUFMIN         ;[M80] SETUP POINTER
CRDO:   ld      a,13              ;;Print Carriage Return
        rst     OUTCHR            ;
        ld      a,10              ;;Print Line Feed
        rst     OUTCHR            ;
CRFIN:  ld      a,(PRTFLG)        ;[M80] SEE IF OUTPUTTING TO PRINTER
        or      a                 ;
        jr      z,CRCONT          ;[M80] NOT PRINTER, CONTINUE
        xor     a                 ;[M80] CRFIN MUST ALWAYS RETURN WITH A=0
        ld      (LPTPOS),a        ;;Set Print Head position to 0
CRCONT: ret                       ;
;;The INKEY$ function
INKEY:  rst     CHRGET            ;
        push    hl                ;[M80] SAVE THE TEXT POINTER
        call    CHARCG            ;[M80] GET CHARC AND CLEAR IF SET
        jr      z,NULRT           ;{M80} NO CHAR, RETURN NULL STRING
        push    af                ;
        call    STRIN1            ;[M80] MAKE ONE CHAR STRING
        pop     af                ;
        ld      e,a               ;[M80] CHAR TO [D]
        call    SETSTR            ;[M80] STUFF IN DESCRIPTOR AND GOTO PUTNEW
NULRT:  ld      hl,REDDY-1        ;
        ld      (FACLO),hl        ;
        ld      a,1               ;
        ld      (VALTYP),a        ;;Set Type to String
        pop     hl                ;
        ret                       ;
CHARCG: push    hl                ;
        ld      hl,CHARC          ;
        ld      a,(hl)            ;[M80] GET SAVED CHAR
        ld      (hl),0            ;[M80] CLEAR IT
        or      a                 ;[M80] IS THERE ONE?
        call    z,CNTCCN          ;{M80} SEE IF ITS CONTROL-C
        pop     hl                ;
        ret                       ;
;;Check for ^C and ^S
ISCNTC: call    CNTCCN            ;{M80} SEE IF ITS CONTROL-C
        ret     z                 ;[M80] IF NONE, RETURN
        ld      (CHARC),a         ;{M80} SAVE CHAR
        cp      $13               ;[M80] PAUSE? (^S)
        ret     nz                ;{M80} IF PAUSE, READ NEXT CHAR
TRYIN:  xor     a                 ;;Wait for character from keyboard
        ld      (CHARC),a         ;{M80} CLEAR SAVED CHAR
CONIN:  call    CNTCCN            ;
        jr      z,CONIN           ;
        ret                       ;
CNTCCN: call    INCHRH            ;{M80} READ THE CHARACTER THAT WAS PRESSED
        cp      3                 ;[M80] ^C?
        jr      nz,CNTCCR         ;;No, set flags and return
; Check CLOAD Status and Enter Direct Mode
WRMCON: ld      a,(CLFLAG)        ;;Get CLOAD Status
        or      a                 ;;Is it 0?
        call    z,SCRTCH          ;;Bad CLOAD, do a NEW
        jp      WRMFIN            ;;Finish Warm Start
CNTCCR: or      a                 ;;Set flags
        ret                       ;

;;Pixel Graphics Routines - PSET, PRESET, and POINT
PRESET: xor     a                 ;[EBU] PRESET FLAG
        jr      PPRSET            ;
PSET:   ld      a,1               ;[EBU] PSET FLAG
PPRSET: ex      af,af'            ;;Save PSET/PRESET flag
        call    SCAND             ;;Parse (X,Y)
;;Execute PSET/PRESET
;;Arguments: BC = X-coord, DE = Y-coord, A = 0 for PRESET, else PSET
PPRSDO: call    SCALXY            ;;Convert X,Y
        jr      z,RSETC           ;;Semigraphics at screen location?
        ld      (hl),$A0          ;;No, store base semigraphic
RSETC:  ex      af,af'            ;;Restore PSET/PRESET flag
        or      a                 ;;Set flags
        ld      a,(de)            ;;Get semigraphic offset
        jr      nz,PSETC          ;;PRESET?
        cpl                       ;;Invert to create mask
        and     (hl)              ;;and clear offset bit
        byte    $06               ;;"LD B," around next instruction
PSETC:  or      (hl)              ;;If PRESET, set offset bit
        ld      (hl),a            ;;Store at screen location
        pop     hl                ;;Restore text pointer
        ret                       ;
;;The PPOINT function
POINT:  rst     CHRGET            ;;Eat character
        call    SCAND             ;;Parse (X,Y)
        call    SCALXY            ;;Convert X,Y
        jr      nz,POINTZ         ;;Not semigraphics? Return 0
        ld      a,(de)            ;;Get bit offset
        and     (hl)              ;;Mask with screen character
        ld      d,1               ;
        jr      nz,POINTR         ;;Bit set? Return 1
POINTZ: ld      d,0               ;
POINTR: xor     a                 ;;Clear Accumulator
        call    FLOATD             ;
        pop     hl                ;;Restore text pointer
        ret                       ;
;;Get parameters for PSET, PRESET, POINT, and SOUND
;;Scans program text in the format (x-coord,y-coord)
;;Returns x-coord in BC, y-coord in DE
SCAND:  rst     SYNCHK
        byte    '('               ;[GWB] SKIP OVER OPEN PAREN
        call    GETINT            ;[GWB] SCAN X INTO [D,E]
        push    de                ;[GWB] SAVE WHILE SCANNING Y
        rst     SYNCHK            ;
        byte    ','               ;[GWB] SCAN COMMA
        call    GETINT            ;[GWB] GET Y INTO [D,E]
        rst     SYNCHK            ;
        byte    ')'               ;{GWB} SKIP OVER CLOSE PAREN
        pop     bc                ;[GWB] GET BACK X INTO [B,C]
        ret                       ;
;;Convert PSET Coordinates to Screen Position and Character Mask
SCALXY: ex      (sp),hl           ;;Save Registers
        push    hl
        push    bc                ;;BC=X Coordinate
        push    de                ;;DE=Y Coordinate
        ld      hl,71
        rst     COMPAR            ;;If Y greater than 71
FCERRP: jp      c,FCERR           ;;Function Call error
        ld      hl,79
        push    bc
        pop     de
        rst     COMPAR            ;;If X greater than 79
        jr      c,FCERRP          ;;Function Call error
        pop     de
        pop     bc
        ld      hl,SCREEN+40      ;;Starting screen offset
        ld      a,e               ;;A=Y Coordinate
        ld      de,40             ;;Screen width
SCALPY: cp      3                 ;;Less than 3?
        jr      c,SCALEX          ;;Convert X
        add     hl,de             ;;Add a line to offset
        dec     a                 ;;Subtract 3
        dec     a                 ;
        dec     a                 ;
        jr      SCALPY            ;;Repeat
SCALEX: rlca                      ;;Multiply remainder by 2
        sra     c                 ;;Column = X-Coordinate / 2
        jr      nc,SCALES         ;;Was it odd?
        inc     a                 ;;Yes, add one to remainder
SCALES: add     hl,bc             ;;Add column to screen offset
        ld      de,BITTAB         ;
SCALEB: or      a                 ;;Check bit#
        jr      z,SCALEC          ;;If not 0
        inc     de                ;;  Bump table pointer
        dec     a                 ;;  Decrement bit#
        jr      SCALEB            ;;  and repeat
SCALEC: ld      a,(hl)            ;;Get character at screen offset
        or      $A0               ;;and return it with
        xor     (hl)              ;;bits 5 and 7 cleared
        ret                       ;
;;Semigraphic Pixel Index to Bit Mask Table
BITTAB: byte    00000001b,00000010b,00000100b
        byte    00001000b,00010000b,01000000b
;;Parse an Integer
GETINT: call    FRMEVL            ;;Get a number
        jp      FRCINT            ;;Convert to an Integer
SOUND:  push    de
        call    SCAND
        push    hl
        call    SOUNDS
        pop     hl
        pop     de
        ret
;;Print CR/LF to printer
LPCRLF: ld      a,13              ;;Send CR to printer
        call    LPTOUT            ;
        ld      a,10              ;;Send LF to printer
LPTOUT: ;Primitive print character to printer routine
        rst     HOOKDO            ;;Call Extended ROM Hook Routine
        byte    17                ;
        push    af                ;;Save character
        push    af                ;;Save Registers
        exx                       ;
LPTRDY: in      a,($FE)           ;;Wait for printer to be ready
        and     1                 ;
        jr      z,LPTRDY          ;
;;Send framed byte to printer port
        call    OUTBIZ            ;;Send Start Bit
        ld      e,8               ;;Send 8 bits to printer
        pop     af                ;;Restore character
OUTBTS: call    OUTBIT            ;;Send bit 0 to printer
        rrca                      ;;Rotate bits
        dec     e                 ;;Decrement counter and loop
        jr      nz,OUTBTS         ;
        ld      a,1               ;;Send Stop Bit
        call    OUTBIT            ;
        exx                       ;;Restore Registers
        pop     af                ;
        ret                       ;
;;Send zero bit to printer port
OUTBIZ: ld      a,0               ;Write zero bit to printer port
;;Send bit 0 of A to printer port
OUTBIT: out     ($FE),a           ;;Write bit to printer port
        ld      h,177             ;;Wait 2,849 cycles (700 microseconds)
OUTDLY: dec     h                 ;;Wait [HL]*16+17 cycles
        jr      nz,OUTDLY         ;
        nop                       ;
        nop                       ;
        nop                       ;
        ret                       ;
;;Copy Screen Contents to Printer
;;Assumes printer line width is 40
COPY:   push    hl                ;;Save Text Pointer
        push    de                ;
        call    LPCRLF            ;;Print CR/LF
        ld      hl,SCREEN+40      ;;Row 1, Column 0
        ld      de,SCREEN+1000    ;;End of Screen
COPY1:  ld      a,(hl)            ;;Get Screen Character
        call    LPTOUT            ;;Print it
        inc     hl                ;;Bump pointer
        rst     COMPAR            ;;Are we there yet?
        jr      c,COPY1           ;;No, do it again
        call    LPCRLF            ;;Print CR/LF
        pop     de                ;
        pop     hl                ;;Restore Text Pointer
        ret                       ;
;;Display Tape Control Messages
PPLAY:  push    hl                ;;Save all Registers
        push    de                ;
        push    bc                ;
        ld      hl,PLAYT          ;;"Press <PLAY>"
PRETRN: push    af                ;;Entry point for "Press <RECORD>"
        call    STROUT            ;
        ld      hl,STARTT         ;
        call    STROUT            ;;Print "Press RETURN key to start"
PRETRL: call    INCHRH            ;
        cp      13                ;
        jr      nz,PRETRL         ;;Wait for RETURN key
        call    CRDO              ;Print CR/LF
        pop     af                ;
        pop     bc                ;
        pop     de                ;
        pop     hl                ;;Restore all Registers
        ret                       ;
;;Read Byte from Tape
RDBYTE: exx                       ;;Save index registers
        ld      c,252             ;;Tape I/O Port
RDBYT2: call    RDBIT             ;;Wait for Start Bit (0)
        jr      c,RDBYT2          ;
        ld      h,8               ;;Now read 8 bits
RDBYT3: call    RDBIT             ;;Read next bit
        rl      l                 ;;Rotate into L
        dec     h                 ;
        jr      nz,RDBYT3         ;;Loop until done
        ld      a,l               ;;Copy byte into A
        exx                       ;;Restore index registers
        ret                       ;
;;Read Bit from Tape
;;First wait for the leading negative half of the pulse train
RDBIT:  in      a,(c)             ;;Read tape port
        rra                       ;;Rotate low bit into Carry
        jr      c,RDBIT           ;;If 1, do it again
;;Then wait for the start of the positive half of the square wave
RDBIT2: in      a,(c)             ;;Read tape port
        rra                       ;;Rotate low bit into Carry
        jr      nc,RDBIT2         ;;If 0, do it again
        xor     a                 ;;Start counter at 0
;;Now measure length of positive half
RDBIT3: inc     a                 ;;Increment counter
        in      b,(c)             ;;Read tape port
        rr      b                 ;;Rotate low bit into Carry
        jr      c,RDBIT3          ;;If 1, do it again
;;Add length of negative half
RDBIT4: inc     a                 ;;Increment counter
        in      b,(c)             ;;Read tape port
        rr      b                 ;;Rotate low bit into Carry
        jr      nc,RDBIT4         ;;If 0, do it again
;;Check Total Length
        cp      73                ;;Set Carry if at least 2633 cycles
        ret                       ;
RWARYR: ret                       ;;Return from RWARYD
;;CSAVE tape control
PRECRD: push    hl                ;;Save Registers
        push    de                ;
        push    bc                ;
        ld      hl,RECORT         ;;"Press <RECORD>"
        jr      PRETRN            ;;Wait for RETURN
;;Write byte to tape twice
WRBYT2: call    WRBYTE            ;;Call WRBYTE, then drop into it
;;Write framed byte to tape
;;Writes a start bit (0) , the bits of the byte, then two stop bits (1)
WRBYTE: push    af                ;;Save all registers
        exx                       ;
        ld      c,252             ;;Tape I/O Port
        push    af                ;;Save byte
        xor     a                 ;
        ld      e,1               ;
        call    WRBITS            ;;Write start bit ($00)
        pop     af                ;;Restore byte
        ld      e,8               ;
        call    WRBITS            ;;Write 8 bits of byte
        ld      a,$FF             ;
        ld      e,2               ;
        call    WRBITS            ;;Write stop bits (2 x $FF)
        exx                       ;
        pop     af                ;;Restore all registers
        ret                       ;
;;Write E most significant bits of A to Tape
WRBITS: rla                       ;;Rotate MSB into Carry
        ld      l,64              ;;Preset Pulse Length 1,039 cycles for a 1
        jr      c,WRBIT2          ;;Was bit a 1?
        ld      l,128             ;;No, set Pulse Length 2,063 cycles for a 0
;;Write Bit with Pulse Length L   ;
WRBIT2: ld      b,4               ;;Countdown - + - +
WRBIT3: out     (c),b             ;;Writing 0, 1, 0, 1
        ld      h,l               ;;Each pulse is L*16+15 cycles
WRBIT4: dec     h                 ;                4
        jr      nz,WRBIT4         ;
        dec     b                 ;
        jr      nz,WRBIT3         ;;Write next pulse
        dec     e                 ;
        jr      nz,WRBITS         ;;Write next bit
        ret                       ;
        ret                       ;;Orphan instruction
;;Write SYNC to tape
WRSYNC: push    af                ;;Save registers
        push    bc                ;
        ld      b,12              ;
WRSYN2: ld      a,$FF             ;
        call    WRBYTE            ;;Write $FF to tape
        djnz    WRSYN2            ;;Do it 11 more times
        xor     a                 ;
        call    WRBYTE            ;;Write $00 to tape
        pop     bc                ;
        pop     af                ;;Restore registers
        ret                       ;
;;Read SYNC from tape
RDSYNC: push    af                ;;Save registers
        push    bc                ;
RDSYN1: ld      b,6               ;;Do 6 times
RDSYN2: call    RDBYTE            ;
        inc     a                 ;
        jr      nz,RDSYN1         ;;If not $FF, start all over
        djnz    RDSYN2            ;;Repeat until 6 $FF read
RDSYN3: call    RDBYTE            ;
        or      a                 ;
        jr      z,RDSYN4          ;;If $00, we are done
        inc     a                 ;
        jr      z,RDSYN3          ;;If $FF, read another byte
        jr      RDSYN1            ;;Otherwise, start all over
RDSYN4: pop     bc                ;
        pop     af                ;;Restore registers
        ret                       ;
PLAYT:  byte    "Press <PLAY>",13,10,0          ;
RECORT: byte    "Press <RECORD>",13,10,0        ;
CSAVE:  rst     HOOKDO            ;
        byte    21                ;
        cp      MULTK             ;;If * Token
        jp      z,CSARY           ;;Do CSAVE*
        call    NAMFIL            ;;Scan filename
        push    hl                ;;Save text pointer
        call    WRHEAP            ;
        ld      hl,(TXTTAB)       ;
        call    CSAVEP            ;
WRTAIL: ld      b,15              ;;Write 15 $00 to tape
        xor     a                 ;
CSAVE3: call    WRBYTE            ;
        djnz    CSAVE3            ;
        ld      bc,8000           ;
        call    SLEEP             ;;Delay 200,000 cycles ~ 50 milliseconds
        pop     hl                ;;Restore Text Pointer
        ret                       ;
CLOAD:  rst     HOOKDO
        byte    20
        cp      MULTK             ;;Check for token after CLOAD
        jp      z,CLARY           ;;If *, CLOAD variable
        sub     PRINTK            ;
        jr      z,CLOADQ          ;;If ?, A will be $FF (verify)
        xor     a                 ;;otherwise it will be 0 (load)
        byte    $01               ;;"LD BC," over two instructions
CLOADQ: cpl                       ;;A = $FF
        inc     hl                ;;Bump text pointer
        cp      1                 ;;If A is 0, set C, else clear C
        push    af                ;;Save A and C
        ld      a,$FF             ;
        ld      (CLFLAG),a        ;;Set Cload Flag to $FF
        call    NAMFIN            ;;Get FILNAM if present
;;Look for Filename
CLOADF: xor     a                 ;
        ld      (INSYNC),a        ;;Clear SYNC flag
        push    de                ;
        call    PPLAY             ;;"Press <PLAY>", wait for RETURN
        call    RDHEAD            ;
        ld      hl,FILNAF         ;;Filename found on tape
        call    CMPNAM            ;;Compare with FILNAM
        pop     de                ;
        jr      z,CLOADC          ;;If they match, continue CLOAD
        ld      hl,SKIPT          ;
        call    OUTNAM            ;;"Skip: " + NAMFIL
CLOADE: ld      b,10              ;;???Skip to end of file
CLOAD2: call    RDBYTE            ;
        or      a                 ;
        jr      nz,CLOADE         ;;Not $00, start over
        djnz    CLOAD2            ;;Loop until 10 in a row
        jr      CLOADF            ;;Check next file
CLOADC: ld      hl,FOUNDT         ;
        call    OUTNAM            ;;"Found:" + NAMFIL
        pop     af                ;;Restore mode
        ld      (FACLO),a         ;;and aave it
        call    c,SCRTCH          ;;If not CLOAD?, do a NEW
        ld      a,(FACLO)         ;;Get mode back
        cp      1                 ;;and check it
        ld      (CLFLAG),a        ;;Store in FlagE
        ld      hl,(TXTTAB)       ;;Set pointer to BASIC program
        call    CLOADP            ;;Load/verify program
        jr      nz,CLOADV         ;;Check CLOAD? status
        ld      (VARTAB),hl       ;;Set end of program
CLOADR: ld      hl,REDDY          ;
        call    STROUT            ;[M80] PRINT "OK" PREMATURELY
        ld      a,$FF             ;
        ld      (CLFLAG),a        ;;Set FlagE to $FF
        jp      FINI              ;
CLOADV: inc     hl                ;
        ex      de,hl             ;
        ld      hl,(VARTAB)       ;
        rst     COMPAR            ;;Text pointer past end of program?
        jr      c,CLOADR          ;;Yes, CLOAD? successful
        ld      hl,BADT           ;
        call    STROUT            ;;"Bad"
        jp      STPRDY            ;;Abort to direct mode
BADT:   byte    "Bad"
        byte    13,10,0
;;Scan a Filename for CLOAD command
NAMFIN: xor     a                 ;;Store 0
        ld      (FILNAM),a        ;;in first character of FILNAM
        dec     hl                ;;Backup text pointer
        rst     CHRGET            ;;Check for terminator
        ret     z                 ;;If found, return
;{GWB} Scan a Filename for CSAVE command
NAMFIL: call    FRMEVL            ;[GWB] Evaluate string
        push    hl                ;[GWB] save text pointer
        call    ASC2              ;;DE = Pointer to File Name
        dec     hl                ;
        dec     hl                ;
        dec     hl                ;
        ld      b,(hl)            ;
        ld      c,6               ;;Maximum File Name Length
        ld      hl,FILNAM         ;
NAMFL1: ld      a,(de)            ;;Copy String to FILNAM
        ld      (hl),a            ;
        inc     hl                ;
        inc     de                ;
        dec     c                 ;
        jr      z,NAMFL3          ;
        djnz    NAMFL1            ;
        ld      b,c               ;
NAMFL2: ld      (hl),0            ;;Pad with NULs to length of 6
        inc     hl                ;
        djnz    NAMFL2            ;
NAMFL3: pop     hl                ;
        ret                       ;
;;Read File Header from Tape
RDHEAD: call    RDSYNC            ;;Wait for SYNC
        xor     a                 ;
        ld      (INSYNC),a        ;;Clear SYNC flag
        ld      hl,FILNAF         ;;Read 6 bytes into FINLAF
        ld      b,6               ;
RDSTRL: call    RDBYTE            ;;Read B characters to [HL] from tapw
        ld      (hl),a            ;
        inc     hl                ;
        djnz    RDSTRL            ;
        ret                       ;
;;Compare Filename
;;Compares the 6 bytes string to by HL to FILNAM
;;Returns Z Set if the two are equal or FILNAM is an empty string
;;Otherwise, return Z Clear
CMPNAM: ld      bc,FILNAM         ;
;;Entry Point to Compare FILNAM to String pointed to by BC
        ld      e,6               ;;Check 6 characters
        ld      a,(bc)            ;;
        or      a                 ;;First character of FILNAM
        ret     z                 ;;If NUL return Equsl
;;Compare E characters at [BC] against [HL]
CPSTRL: ld      a,(bc)            ;;Compare FILNAM character
        cp      (hl)              ;;with FILNAF character
        inc     hl                ;;Bump pointers
        inc     bc                ;
        ret     nz                ;;If different, return Not Equal
        dec     e                 ;
        jr      nz,CPSTRL         ;;Decrememt counter and loop
        ret                       ;
FOUNDT: byte    "Found: ",0
SKIPT:  byte    "Skip: ",0
;;Print string then filename read from tape
OUTNAM: push    de                ;;Save DE and AF
        push    af                ;
        call    STROUT            ;;Print string pointed to by HL
;;Print filename read from tape
        ld      hl,FILNAF         ;;Pointer to filename
        ld      b,6               ;;Filenames are 6 characters long
OUTNM1: ld      a,(hl)            ;
        inc     hl                ;
        or      a
        jr      z,OUTNM2          ;;ASCII NUL?
        rst     OUTCHR            ;;No, print character
OUTNM2: djnz    OUTNM1            ;;Countdown and loop
        call    CRDO              ;;Print CR/LF
        pop     af                ;
        pop     de                ;;Restore AF and DE
        ret                       ;
;;Prompt User, Wait for RETURN, and Write Header
WRHEAP: call    PRECRD            ;;"Press <RECORD>", wait for RETURN
;;Write File Header to Tape
        call    WRSYNC            ;;Write SYNC to tape
        ld      b,6
        ld      hl,FILNAM         ;;Write Filename to tape
WRSTRL: ld      a,(hl)            ;;Write B Characters at [HL] to tape
        inc     hl
        call    WRBYTE
        djnz    WRSTRL
        ret
;;Save Program Text to tape
CSAVEP: call    WRSYNC            ;;Write SYNC to tape
        ex      de,hl             ;;Save from TXTTAB
        ld      hl,(VARTAB)       ;;to VARTAB
;;Save RAM from DE to HL on tape
CSAVEB: ld      a,(de)            ;;Get byte from memory
        inc     de                ;;Bump pointer
        call    WRBYTE            ;;Write byte to text
        rst     COMPAR            ;;Are we there yet?
        jr      nz,CSAVEB         ;;No, write another
        ret                       ;
;;;Orphan code?
SLEEPS: ld      bc,0              ;Sleep maximum amount
;;Pause program execution
;;Delays BC*25-5 cycles
SLEEP:  dec     bc                ;;Decrement Counter
        ld      a,b               ;
        or      c                 ;;Is it 0?
        jr      nz,SLEEP          ;;If not, loop
        ret                       ;
;;Load Program Text from tape
CLOADP: call    RDSYNC            ;;Wait for SYNC
        ld      a,$FF
        ld      (INSYNC),a        ;;Set SYNC flag
        sbc     a,a               ;;A = 0
        cpl                       ;;A = $FF ???WHY?
        ld      d,a               ;;D = $FF
CLOADB: ld      b,10              ;
CLOADN: call    RDBYTE            ;;Get Byte
        ld      e,a               ;;Save it
        sub     (hl)              ;;Compare with byte in memory
        and     d                 ;;If CLOAD?
        ret     nz                ;;and different, return with Zero cleared
        ld      (hl),e            ;;Store byte in memory
        call    REASON            ;;If no space left, OM error
        ld      a,(hl)            ;;Get byte back
        or      a                 ;;Set flags
        inc     hl                ;;Bump text pointer
        jr      nz,CLOADB         ;;Not 0, reset counter
        djnz    CLOADN            ;;Loop until 256 $00 are read
        xor     a                 ;;Return with Zero set
        ret
TTYCHR: ;Print character to screen
        rst     HOOKDO            ;;Call Extended BASIC Hook Routine
        byte    19                ;
TTYCH:  push    af                ;;Save character
        cp      10                ;[M80] LINE FEED?
        jr      z,ISLF            ;
        ld      a,(TTYPOS)        ;
        or      a                 ;;At beginning of line?
        jr      nz,ISLF           ;;No, skip line counter check
        ld      a,(CNTOFL)        ;
        or      a                 ;;Is line Counter 0?
        jr      z,ISLF            ;;Then no pauses
        dec     a                 ;
        ld      (CNTOFL),a        ;;Decrement Line Counter
        jr      nz,ISLF           ;;Not 0, don't pause
        ld      a,23              ;
        ld      (CNTOFL),a        ;;Reset line counter
        call    TRYIN             ;;Wait for character from keyboard
ISLF:   pop     af                ;;Retrieve Character
;;Print Character, bypassing Extended Hook and Line Counter checks
TTYOUT: push    af                ;
        exx                       ;;Save HL on Stack
        cp      7                 ;;Is it BEL!
        jp      z,BEEP            ;;Make Beep Sound
        cp      11                ;;Is it CLS?
        jp      z,TTYCLR          ;;Clear the Screen
        ld      e,a               ;;Save A
        ld      hl,(CURRAM)       ;
        ld      a,(CURCHR)        ;;Get character under cursor
        ld      (hl),a            ;;Place at current screen position
        ld      a,e               ;;Restore A
        cp      8                 ;;Is it BS?
        jr      z,BS              ;;Do Back Space
        cp      13                ;;Is it CR?
        jr      z,CR              ;;Do Carriage Return
        cp      10                ;;Is it LF?
        jr      z,LF              ;;Do Line Feed
        ld      hl,(CURRAM)       ;;Place character at
        ld      (hl),a            ;;current position on screen
        call    TTYMOV            ;;Move Cursor Right
        jr      TTYFIN            ;;Finish Up
;;Carriage Return: Move Cursor to Beginning of Current Line
CR:     ld      de,(TTYPOS)       ;
        xor     a                 ;
        ld      d,a               ;;Subtract Cursor Column
        sbc     hl,de             ;;from Screen Position
        jr      TTYFIS            ;
;;Line Feed: Move Cursor Down One Line
LF:     ld      de,SCREEN+960     ;
        rst     COMPAR            ;;Cursor on Last Row?
        jp      nc,LFS            ;;Yes, Scroll and Finish Up
        ld      de,40             ;
        add     hl,de             ;;Add 40 to Move Down One Line
        ld      (CURRAM),hl       ;;Save New Screen Position
        jr      TTYFIN            ;
LFS:    call    SCROLL            ;;Scroll Up and Keep Screen Position
        jr      TTYFIN            ;
;;Back Space: Move Cursor Left and Delete Character
BS:     ld      a,(TTYPOS)        ;
        or      a                 ;;At First Column?
        jr      z,NOBS            ;
        dec     hl                ;;No, Move One to the Left
        dec     a                 ;
NOBS:   ld      (hl),' '          ;;Erase Character at Position
;;Save Character and Display Cursor
TTYFIS: call    TTYSAV            ;;Save Column and Position
TTYFIN: ld      hl,(CURRAM)       ;
        ld      a,(hl)            ;;Get character at position
        ld      (CURCHR),a        ;;Save character under cursor
        ld      (hl),$7F          ;Display Cursor
TTYXPR: exx                       ;Restore [BC], [DE] and [HL]
        pop     af                ;Restore [AF]
        ret

;;Restore Character Under Cursor
;;*** Orphan Code that's still useful
TTYRES: ld      hl,(CURRAM)       ;;Get position
        ld      a,(CURCHR)        ;;Get character
        ld      (hl),a            ;;Put character at position
        ret                       ;

;;Scroll Screen Up one Line
SCROLL: ld      bc,920            ;;Move 23 * 40 bytes
        ld      de,SCREEN+40      ;;To Row 1 Column 0
        ld      hl,SCREEN+80      ;;From Row 2 Column 1
        ldir                      ;
        ld      b,40              ;;Loop 40 Times
        ld      hl,SCREEN+961     ;;Starting at Row 23, Column 0
SCROLP: ld      (hl),' '          ;;Put Space
        inc     hl                ;;Next Column
        djnz    SCROLP            ;;Do it again
        ret                       ;
;;Make a Beep Sound
BEEP:   ld      bc,200            ;
        ld      de,50             ;
        call    SOUNDS            ;;Play freq 50, delay 300
        jr      TTYXPR            ;;Restore Registers and return
;;Move Cursor one character to the right
TTYMOV: ld      hl,(CURRAM)       ;
        ld      a,(TTYPOS)        ;
        inc     hl                ;;Increment Position in Memory
        inc     a                 ;;Increment Cursor Column
        cp      38                ;;Less than 39?
        jr      c,TTYSAV          ;;Save and Return
        inc     hl                ;
        inc     hl                ;;Skip border columns
        ld      de,SCREEN+1000    ;
        rst     COMPAR            ;;Past End of Screen?
        ld      a,0               ;;Cursor Column = 0
        jr      c,TTYSAV          ;;No, Save Position and Column
        ld      hl,033C1H         ;;Yes, Position = Row 24, Column 0
        call    TTYSAV            ;Save Position and Column
        jp      SCROLL            ;Scroll Screen
;;Update Current Screen Position and Cursor Column
TTYSAV: ld      (CURRAM),hl       ;;Position in Screen RAM
        ld      (TTYPOS),a        ;;Cursor Column
        ret                       ;
;;Clear Screen
TTYCLR: ld      b,' '             ;
        ld      hl,SCREEN         ;
        call    FILLIT            ;;Write to Sreen RAM
        ld      b,6               ;;Black on Light Cyan
        call    FILLIT            ;;Write to Color RAM
        ld      hl,SCREEN+41      ;;Home Cursor
        xor     a                 ;;Column = 0
        jp      TTYFIS            ;;Save and Finish
;;Fill 1024 bytes atarting at HL with A
FILLIT: ld      de,$3FF           ;;Count down from 1023
;;Fill BC bytes atarting at HL with A
FILLIP: ld      (hl),b            ;;Store byte
        inc     hl                ;;Increment pointer
        dec     de                ;;Decrement Counter
        ld      a,d               ;
        or      e                 ;;Is it 0?
        jr      nz,FILLIP         ;;No, Loop
        ret                       ;
SOUNDS: ld      a,b               ;;Play frequency [DE] for duration [BC]
        or      c                 ;;If BC = 0 return
        ret     z                 ;
        xor     a                 ;
        out     ($FC),a           ;;Write 0 to port $FC
        call    SDELAY            ;
        inc     a                 ;
        out     ($FC),a           ;;Write 1 to port $FC
        call    SDELAY            ;;Delay [DE]*31+44 cycles
        dec     bc                ;;Decrememt duration counter and loop
        jr      SOUNDS            ;
SDELAY: push    de                ;;Delay [DE]*31+27 cycles
        pop     hl                ;;Copy freq delay to HL (+10)
SDELAL: ld      a,h               ;
        or      l                 ;
        ret     z                 ;;If 0, return
        dec     hl                ;
        jr      SDELAL            ;;Decrement and loop
;;INCHRH, INCHRC, and INCHRI - Get Character from Keyboard
INCHRH: rst     HOOKDO
        byte    18
;;Check for keypress
INCHRC: exx                       ;;Save Registers
INCHRI: ld      hl,(RESPTR)
        ld      a,h
        or      a
        jr      z,KEYSCN
        ex      de,hl             ;;Save HL
        ld      hl,KCOUNT
        inc     (hl)              ;;Increment debounce counter
        ld      a,(hl)            ;;Get Value
        cp      15                ;;Less than 15?
        jr      c,KEYFIN          ;;Yes, finish up
        ld      (hl),5            ;;Set debounce counter to 5
        ex      de,hl             ;;Restore HL
        inc     hl
        ld      a,(hl)
        ld      (RESPTR),hl
        or      a
ifdef noreskeys
;;;Stop auto-styping when ASCII null is encountered
        jp      z,KEYRET                                                      
else
        jp      p,KEYRET                                                      
endif
        xor     a                                                             
        ld      (RESPTR+1),a                                                  
;;Scan Keyboard
ifdef addkeyrows
;;;Code Change: Read extended 64 key Keyboard by also checking Rows 6 and 7
;;;Must be combined with "noreskeys" and requires a compatible Extended BASIC
;;;Extended BASIC must contain replacement key tables and put the address in KEYVCT
;;;
;;; Note: On Windows keyboards without the Context Menu key, try Shift-F10
;;;
;;;Aquarius+ Keyboard Defs at
;;; https://github.com/fvdhoef/aquarius-plus/blob/master/System/esp32/main/AqKeyboardDefs.h
;;;
;;;Extended BASIC Key Matrix
;;; Column   1    2     3    4     5     6    7     8
;;; Row 1    =    -     9    8     6     5    3     2
;;; Row 2   <--   /     O    I     Y     T    E     W
;;; Row 3    :    0     K    7     G     4    S     1
;;; Row 4   RTN   P     M    U     V     R    Z     Q
;;; Row 5    ;    L     N    H     C     D  SPACE SHIFT
;;; Row 6    .    ,     J    B     F     X    A    CTL
;;; Row 7   INS CsrUp CsrLf HOME  PgUp Pause Menu  ALT
;;; Row 8   DEL CrsRt CsrDn END   PgDn PrtSc TAB  META
;;;
;;;
;;;The Decode routine requires 4 key tables: normal, Shift,
;;;Control, and Alt. The Meta key produces and ASCC character.
;;;Each Key Table is 64 bytes long and they must be consecutive
;;;Each line of a Key Table is 8 bytes long instead of 6 bytes.
;;;
;;;Sample Key Table
;;; '=',$08,':',$0D,';','.',$9D,$7F ; Backspace, Return, INS DEL
;;; '-','/','0','p','l',',',$8F,$8E ; CsrUp, CsrRt
;;; '9','o','k','m','n','j',$9E,$9F ; CsrLf, CsrDn
;;; '8','i','7','u','h','b',$9B,$9A ; Home, End
;;; '6','y','g','v','c','f',$8A,$8B ; PgUp, PgDn
;;; '5','t','4','r','d','x',$89,$88 ; Pause, PrtScr
;;; '3','e','s','z',' ','a',$9C,$8C ; Menu, Tab
;;; '2','w','1','q', 0 , 0 , 0 , 0  ;
;;;
;;;Recommended Physical Key Layout for Above Table. Keys in a Column Grouped Together
;;;
;;; META 1   2   3   4   5   6   7   8   9  0   -   =  <---    
;;; TAB    Q   W   E   R   T   Y   U   I   O   P   :             
;;; ALT MNU  A   S   D   F   G   H   J   K   L   ;   RETURN   
;;; SHIFT      Z   X   C   V   B   N   M   ,   .   /    INS           
;;; CTL  SPACE  PRS PSE PUP PUD HOM END CUL CUD CUP CUR DEL
;;;
ROWMSK  equ     $FF               ;;Checking All 8 Rows`
ROWCNT  equ     8                 
CSHMSK  equ     $8F               ;;Check Rows 0 through 3 plus 7
else
ROWMSK  equ     $3F               ;;Check Rows 0 through 5
ROWCNT  equ     6                 
CSHMSK  equ     $0F               ;;Check rows 0 through 3 - %00001111
endif
KEYSCN: ld      bc,$00FF          ;;B=0 to scan all columns
        in      a,(c)             ;;Read rows from I/O Port 255
        cpl                       ;
        and     ROWMSK            ;;Mask Rows to Check
        ld      hl,LSTX           ;;Pointer to last scan code
        jr      z,NOKEYS          ;;No key pressed? ???do a thing
        ;;Scsn Column 7
        ld      b,$7F             ;;Scanning column 7 - %01111111
        in      a,(c)             ;;Read rows and invert
        cpl                       
        and     CSHMSK            ;;Mask Off Control and Shift Keys
        jr      nz,KEYDN          ;;Key down? Process it
        ;;Scan the Rest of the Columns
        ld      b,$BF             ;;Start with column 6 - %10111111 
KEYSCL: in      a,(c)             ;;Read rows and invert
        cpl                       ;
        and     ROWMSK            ;;Mask Rows to Check
        jr      nz,KEYDN          ;;Key down? Process it
        rrc     b                 ;;Next column
        jr      c,KEYSCL          ;;Loop if not out of columns
NOKEYS: inc     hl                ;;Point to KCOUNT
        ld      a,70              ;
        cp      (hl)              ;                                           Original Code
        jr      c,KEYFIN          ;;Less than 70? Clean up and return         1EC9  jr      c,KEYFIN
        jr      z,SCNINC          ;;0? Increment KCOUNT and return
        inc     (hl)              ;
KEYFIN: xor     a                 ;;Clear A                                   

        exx                       ;;Restore Registers
        ret                       ;                  
SCNINC: inc     (hl)              ;;Increment KCOUNT
        dec     hl                ;
        ld      (hl),0            ;;Clear LSTX
        jr      KEYFIN            ;;Clean up and Return
KEYDN:  ld      de,0              ;
KEYROW: inc     e                 ;;Get row number (1-5)
        rra                       ;
        jr      nc,KEYROW         ;
        ld      a,e               ;
KEYCOL: rr      b                 ;;Add column number number of ROWS
        jr      nc,KEYCHK         ;
        add     a,ROWCNT          ;
        jr      KEYCOL            ;
KEYCHK: ld      e,a               ;
        cp      (hl)              ;;Compare scan code to LSTX
        ld      (hl),a            ;;Update LSTX with scan code
        inc     hl                ;;Point to KCOUNT
        jr      nz,KEYCLR         ;;Not the same? Clear KCOUNT and exit
        ld      a,4               
        cp      (hl)              ;;Check KCOUNT
        jr      c,KEYFN6          ;;Greater than 4? Set to 6 and return
        jr      z,KEYASC          ;;Equal to 4? Convert scan code
        inc     (hl)              ;;Increment KCOUNT
        jr      KEYFN2            ;;Clean up and exit
KEYFN6: ld      (hl),6            ;;Set KCOUNT to 6
KEYFN2: xor     a                 ;;Clear A
        exx                       ;;Restore registers
        ret                       ;
KEYCLR: ld      (hl),0            ;;Clear KCOUNT
        jr      KEYFN2            ;;Clean up and return
;;Convert Keyboard Scan Code to ASCII Character
ifdef addkeyrows
;;; KEYASC replacement 25/25 bytes
;;; Meta, Alt, Control, and Shift are bits 7, 6, 5, and 4 of Column 7.        Original Code
KEYASC: inc     (hl)              ;;Increment KCOUNT                          1F00 inc     (hl)        
        ld      bc,$7FFF          ;;Read column 7                             1F01 ld      b,$7F
                                  ;;                                          1F02 
                                  ;;                                          1F03 in      a,(c)        
        in      b,(c)             ;;Get row                                   1F04  
                                  ;;                                          1F05 bit     5,a         
        ld      a,192             ;;Meta Table Offset                         1F06 
                                  ;;                                          1F07 ld      ix,CTLTAB-1 
        rl      b                 ;;Get rid of GUI Bit                        1F08
                                  ;;                                          1F09
KEYALP: rl      b                 ;;Rotate Bits Left                          1F0A
                                  ;;                                          1F0B jr      z,KEYLUP
        jr      nc,KEYLUX         ;;If key pressed, go do lookup              1F0C
                                  ;;                                          1F0D bit     4,a
        sub     a,64              ;;Offset to Previous TABLE                  1F0E
                                  ;;                                          1F1F ld      ix,SHFTAB-1
        jr      nz,KEYALP         ;;Loopif not first table                    1F10
                                  ;;                                          1F11
KEYLUX: ld      ix,KEYADR-1       ;;Point to Start of Lookup Tables           1F12
                                  ;;                                          1F13 jr      z,KEYLUP
                                  ;;                                          1F14
                                  ;;                                          1F15 ld      ix,KEYTAB-1
        add     ix,de             ;;Add Scan Code (Key Offset)                1F16
                                  ;;                                          1F17
        ld      e,a               ;;DE = Table Offet                          1F18
else                              ;;
KEYASC: inc     (hl)              ;;Increment KCOUNT
        ld      b,$7F             ;;Read column 7
        in      a,(c)             ;;Get row
        bit     5,a               ;;Check Control key
        ld      ix,CTLTAB-1       ;;Point to control table
        jr      z,KEYLUP          ;;Control? Do lookup
        bit     4,a               ;;Check Shift key
        ld      ix,SHFTAB-1       ;;Point to shift table
        jr      z,KEYLUP          ;;Shift? Do lookup
        ld      ix,KEYTAB-1         
endif
KEYLUP: add     ix,de             ;;Get pointer into table
        ld      a,(ix+0)          ;;Load ASCII value
        or      a                 ;;Reserved Word? 
ifndef noreskeys
;;;Code Change: Do not expand CTRL-KEYS into Reserved Words
        jp      p,KEYRET          ;;No, loop          
else                                                                         ;Original Code
        jp      KEYRET            ;;                                          1F1F  jp      p,KEYRET
endif
;;;Deprecated Code: 20 bytes                                                  
        sub     $7F               ;;Convert to Reserved Word Count             
        ld      c,a               ;;and copy to C                             
        ld      hl,RESLST-1       ;;Point to Reserved Word List               
KEYRES: inc     hl                ;;Bump pointer                              
        ld      a,(hl)            ;;Get next character                        
        or      a                 ;;First letter of reserved word?            
        jp      p,KEYRES          ;;No, loop                                  
        dec     c                 ;;Decrement Count                           
        jr      nz,KEYRES         ;;Not 0? Find next word                     
        ld      (RESPTR),hl       ;;Save Keyword Address                      
        and     $7F               ;;Strip high bit from first character       
KEYRET: exx                       ;;Restore Registers
        ret
;;Key Lookup Tables - 46 bytes each
;;Unmodified Key Lookup Table
KEYTAB: byte    '=',$08,':',$0D,';','.' ;;Backspace and Return
        byte    '-','/','0','p','l',','
        byte    '9','o','k','m','n','j'
        byte    '8','i','7','u','h','b'
        byte    '6','y','g','v','c','f'
        byte    '5','t','4','r','d','x'
        byte    '3','e','s','z',' ','a'
        byte    '2','w','1','q'
;;Shifted Key Lookup Table
SHFTAB: byte    '+',$5C,'*',$0D,'@','>' ;;Backslash, Return
        byte    '_','^','?','P','L','<'
        byte    ')','O','K','M','N','J'
        byte    '(','I',$27,'U','H','B' ;;Apostrophe
        byte    '&','Y','G','V','C','F'
        byte    '%','T','$','R','D','X'
        byte    '#','E','S','Z',' ','A'
        byte    $22,'W','!','Q'         ;;Quotation Mark
;;Control Key Lookup Table
CTLTAB:
ifdef noreskeys
        byte    $1B,$7F,$1D, 0 ,$A0,$7D ;ESC DEL GS      NUL  }   
        byte    $1F,$1E,$1C,$10,$0C,$7B ;GS  RS  FS  DLE FF   {   
        byte    $5D,$0F,$0B,$0D,$0E,$0A ; ]  SI  VT  CR  SO  LF   
        byte    $5B,$09,$60,$15,$08,$02 ; [  Tab  `  NAK BS  SOH  
        byte    $8E,$19,$07,$16,$03,$06 ;rt  EM  BEL SYN ETX ACK  
        byte    $9F,$14,$8F,$12,$04,$18 ;dn  DC4 up  DC2 EOT CAN  
        byte    $9E,$05,$13,$1A, 0 ,$01 ;lft ENC DC3 SUB     SOH  
        byte    $7E,$17,$7C,$11         ; ~  ETB  |  DC1
else
        byte    $82,$1C,$C1,$0D,$94,$C4 ;;NEXT ^\ PEEK Return POKE VAL
        byte    $81,$1E,$30,$10,$CA,$C3 ;;FOR ^^ 0 ^P POINT STR$
        byte    $92,$0F,$9D,$0D,$C8,$9C ;;COPY ^O PRESET ^M RIGHT$ PSET
        byte    $8D,$09,$8C,$15,$08,$C9 ;;RETURN ^I GOSUB ^U ^H MID$
        byte    $90,$19,$07,$C7,$03,$83 ;;ON ^Y ^G LEFT$ ^C DATA
        byte    $88,$84,$A5,$12,$86,$18 ;;GOTO INPUT THEN ^R READ ^X
        byte    $8A,$85,$13,$9A,$C6,$9B ;;IF DIM ^S CLOAD CHR$ CSAVE
        byte    $97,$8E,$89,$11         ;;LIST REM RUN ^Q
endif
;;Check for Ctrl-C, called from NEWSTT
INCNTC: push    hl                ;;Save text pointer
        ld      hl,4              ;
        add     hl,sp             ;
        ld      (SAVSTK),hl       ;;Save stack pointer less 2 entries
        pop     hl                ;;Restore text pointer
        jp      ISCNTC            ;;Check for Ctrl-C
;;Finish up Warm Start
WRMFIN: ld      hl,(SAVSTK)       ;;Restore stack pointer
        ld      sp,hl             ;
        ld      hl,(SAVTXT)       ;;Restore text pointer
        call    STOPC             ;;Enter direct mode
;;Save Stack Pointer
STKSAV: dec     hl                ;TAKE INTO ACCOUNT FNDFOR STOPPER
        dec     hl                ;
        ld      (SAVSTK),hl       ;MAKE SURE SAVSTK OK JUST IN CASE
        ld      hl,TEMPST         ;INCREMENT BACK FOR SPHL
        ret                       ;
;;Power Up/Reset Routine: Jumped to from RST 0
JMPINI: ld      a,$FF             ;;Turn off printer and Cold Start
        out     ($FE),a           ;;Write $FF to Printer Port
        jp      INIT              ;[M80] INIT IS THE INTIALIZATION ROUTINE
;;Start Extended Basic
XBASIC: ld      a,$AA             ;;
        out     ($FF),a           ;;Write Unlock Code to Port 255
        ld      (SCRMBL),a        ;;Save It
        jp      XSTART            ;;Jump to Extended BASIC Startup
;;Called from COLDST to print BASIC startup message
PRNTIT: ld      hl,HEDING         ;;Print copyright message and return
        jp      STROUT            ;
;;; Code Change: Code to check for Operator moved to make room for new UDF Hook
;;; Replaces Filler Bytes - 7/8 bytes                                         Original Code
CHKOP:  cp      PLUSTK            ;;If < Plus Token                           1FF8 ld      hl,(CURRAM) 
                                  ;;                                          1FF9                       
        ret     c                 ;;  Return with Carry Set                   1FFA                     
        cp      LESSTK+1          ;;If > Less Than Token                      1FFB ld      a,(CURCHR)  
                                  ;;                                          1FFC                       
        ccf                       ;;  Return with Carry Set                   1FFD                    
        ret                       ;;                                          1FFE
        byte    $F5

;;Verify ROM is correct length
        assert !($2000<>$)   ; Incorrect ROM Length!

end                               ;;End of S3 BASIC                             
