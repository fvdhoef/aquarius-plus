; PT3 player for USB BASIC ROM
;
; 2017-03-22 V0.01 add visuals
; 2017-05-03 V0.03 checking flag bit SF_NTSC to adjust timing for NTSC
; 2017-05-04 V0.04 replaced KEYCHK with AquBASIC Key_Check, SCANCNT set
;                  to 66 after pressing SPACE to reduce debounce time.
; 2017-06-12 V1.0  bumped to release version

60Hz = -1  ; make this -1 to play 50Hz songs on 60Hz machine

; address of variables in RAM
VARMEM = $38A0  ; top of stack at boot (before initializing BASIC)

; AY registers
; STRUCT AYREGS
TonA     = 0
TonB     = 2
TonC     = 4
Noise    = 6
Mixer    = 7
AmplA    = 8
AmplB    = 9
AmplC    = 10
Env      = 11
EnvTp    = 13

;ChannelsVars
; STRUCT  CHP
;reset group
PsInOr   =  0
PsInSm   =  1
CrAmSl   =  2
CrNsSl   =  3
CrEnSl   =  4
TSlCnt   =  5
CrTnSl   =  6
TnAcc    =  8
COnOff   = 10
;reset group

OnOffD   = 11

;IX for PTDECOD here (+12)
OffOnD   = 12
OrnPtr   = 13
SamPtr   = 15
NNtSkp   = 17
Note     = 18
SlToNt   = 19
Env_En   = 20
PtFlags  = 21
 ;Enabled - 0,SimpleGliss - 2
TnSlDl   = 22
TSlStp   = 23
TnDelt   = 25
NtSkCn   = 27
Volume   = 28
;   ENDS
CHP_size = 29

;---------------------------------------------------------
; Variables (and self-mofifying code, if present) in RAM
; NOTE: must be initialized before use!
;---------------------------------------------------------
;
 STRUCTURE VARS,VARMEM
      BYTE SETUP        ; bit7 = 1 when loop point reached
      WORD CrPsPtr
      BYTE AddToEn
      WORD AdInPtA
      WORD AdInPtB
      WORD AdInPtC
      BYTE Env_Del
      WORD MODADDR
      WORD ESldAdd
      BYTE Delay
      WORD SaveSP
      WORD SamPtrs
      WORD OrnPtrs
      WORD PatsPtr
      WORD LPosPtr
      WORD PrSlide
L3       = PrSlide    ; opcode + RET
M2       = PrSlide
      BYTE PrNote
      BYTE PtVersion
;end of variables and self-modifying code
;start of cleared data area
    STRUCT ChanA,CHP_size
    STRUCT ChanB,CHP_size
    STRUCT ChanC,CHP_size

;GlobalVars
      BYTE DelyCnt
      WORD CurESld
      WORD CurEDel
;arrays
      BYTE Ns_Base
Ns_Base_AddToNs = Ns_Base
      BYTE AddToNs
    STRUCT VT_,256  ; 256 bytes CreatedVolumeTableAddress

AYREGS   = VT_      ; 14 AY-3-8910 registers
EnvBase  = VT_+14
VAR0END  = VT_+16   ; end of cleared data area

T1_      = VT_+16   ; Tone table data depacked here
T_OLD_1  = T1_
T_OLD_2  = T_OLD_1+24
T_OLD_3  = T_OLD_2+24
T_OLD_0  = T_OLD_3+2
T_NEW_0  = T_OLD_0
T_NEW_1  = T_OLD_1
T_NEW_2  = T_NEW_0+24
T_NEW_3  = T_OLD_3

;local vars
Ampl     = AYREGS+AmplC

NT_      = VT_+256       ; 192 bytes Note Table
VAREND   = NT_+192       ; end of variable data area
VAR_size = VAREND-VARMEM

; GUI player variables
  STRUCTURE Player,VAREND
       BYTE numsongs          ; number of songs (1-36)
       BYTE song              ; current song number (0-35)
     STRUCT pt3_files,(36*16) ; array to store 36 file infos
     STRUCT SongData,0        ; pt3 file loaded here!
  ENDSTRUCT Player

PT3_PLAY:
       LD   IX,SelectWindow
       CALL OpenWindow
       LD   HL,pt3_files      ; array of file infos to fill
       LD   B,36              ; 36 files max.
       LD   DE,pt3pat         ; "*.PT3"
       CALL RequestFile       ; user selects file
       RET  NZ                ; if no file selected then quit
       LD   (Song),A
       LD   A,C
       LD   (NumSongs),A
PT3_LOAD:
       LD   IX,PlayWindow
       CALL OpenWindow        ; open/clear window
       LD   DE,256*1+21
       CALL WinSetCursor      ; cursor at window bottm/left
       LD   HL,PT3help
       CALL WinPrtStr         ; show key help
       LD   A,(Song)
       LD   HL,pt3_files
       CALL IndexArray        ; HL = pt3files[song]
       PUSH HL
       POP  IY
       BIT  ATTR_B_DIRECTORY,(IY+11) ; directory?
       JR   NZ,.next_song     ; yes, skip file
       PUSH HL                ; push filename
       LD   D,13
       LD   E,1
       CALL WinSetCursor
       LD   A,(SONG)
       ADD  '0'
       CP   '0'+10            ; ID = '0-9', 'A-Z'
       JR   C,.shownum
       ADD  7
.shownum:
       CALL WinPrtChr         ; print ID
       CALL WinPrtMsg         ; print ": "
       db   ": ",0
       LD   A,8
       CALL WinPrtChrs        ; print filename
       POP  HL                ; pop filename
       CALL usb__open_read    ; open song file
       RET  NZ                ; quit if error opening file
       LD   HL,SongData
       LD   DE,-1
       CALL usb__read_bytes   ; read song file into buffer
       PUSH AF
       CALL usb__close_file
       POP  AF
       RET  NZ                ; quit if error reading file
       LD   HL,SongData
       CALL INIT              ; initialize player
.play_quark:
       LD   B,5               ; play 5 'quarks'
.play_loop:
       PUSH BC
       CALL wait_vbl
       CALL Animation
       LD   HL,SongData
       CALL PLAY              ; play next line
       POP  BC
       ld   hl,SETUP
       bit  7,(hl)            ; quit if end of song
       jr   nz,.next_song
       LD   HL,SCANCNT
       LD   A,6               ; starting key up debounce?
       CP   (HL)
       JR   NZ,.debounced     ; no
       LD   (HL),DEBOUNCE-4   ; yes, debounce count to go = 4 scans (~250ms)
.debounced:
       call Key_Check         ; Get ASCII of last key pressed
       cp   $0d
       jr   z,.restart        ; if RTN pressed then return to file list
       cp   " "
       jr   z,.next_song      ; if SPACE pressed then play next song
.no_key:
       DJNZ .play_loop        ; 5 lines at 60Hz
       LD   A,(SysFlags)
       BIT  SF_NTSC,A
       JR   Z,.play_quark
       CALL wait_vbl          ; if NTSC then delay one vblank to get 50Hz playback
       JR   .play_quark
.next_song:
       CALL MUTE              ; stop song
       LD   A,(NumSongs)
       LD   B,A
       LD   A,(Song)
       INC  A                 ; song number + 1
       CP   B                 ; done all songs?
       JR   C,.upd
       XOR  A                 ; yes, wrap to song 0
.upd:  LD   (Song),A
       JP   PT3_LOAD
.restart:
       CALL MUTE              ; stop song
       JP   PT3_PLAY          ; back to file list

; eye candy
Animation:
        PUSH HL
        PUSH DE
        PUSH BC
        LD   HL,$3400+(40*20)+13   ; HL = screen address of bars
        LD   IX,AYREGS
        LD   A,(IX+AmplA)
        LD   E,WHITE*16+BLACK
        LD   D,WHITE*16+RED
        CALL show_bar              ; show red volume bar
        LD   A,(IX+AmplB)
        LD   D,WHITE*16+DKGREEN
        CALL show_bar              ; show green volume bar
        LD   A,(IX+AmplC)
        LD   D,WHITE*16+BLUE
        CALL show_bar              ; show blue volume bar
        POP  BC
        POP  DE
        POP  HL
        RET

;-----------------------------------------
;   show vertical bar 0-15 characters
;-----------------------------------------
;  in: A = bar height
;      D = bar color
;      E = no bar color
;
show_bar:
        LD   B,15
        LD   C,D             ; bar color
        INC  A
.bar11  DEC  A
        JR   NZ,.bar12
        LD   C,E             ; no bar color
.bar12  LD   (HL),C
        INC  HL
        LD   (HL),C
        INC  HL              ; 3 char width bar
        LD   (HL),C
        PUSH BC
        LD   BC,-42
        ADD  HL,BC           ; up to previous line
        POP  BC
        DJNZ .bar11
        LD   BC,(15*40)+5
        ADD  HL,BC           ; move to next bar
        RET

;------------------------------------------------------------------------------
; Wait for Vertical Blank (PAL=50Hz, NTSC=60Hz)
;------------------------------------------------------------------------------
wait_vbl:
       IN   A,($FD)
       BIT  0,A              ; wait for end of previous vertical blank
       JR   Z,wait_vbl
wait_vbl_low:
       IN   A,($FD)
       BIT  0,A              ; wait for start of next vertical blank
       JR   NZ,wait_vbl_low
       RET

;PT3 player version
    DB   "=VTII PT3 Player r.7ROM="

CHECKLP:
    LD   HL,SETUP
    SET  7,(HL)
    BIT  0,(HL)
    RET  Z
    POP  HL
    LD   HL,DelyCnt
    INC  (HL)
    LD   HL,ChanA+NtSkCn
    INC  (HL)
MUTE:
    XOR  A
    LD   H,A
    LD   L,A
    LD   (AYREGS+AmplA),A
    LD   (AYREGS+AmplB),HL
    JP   ROUT_A0

INIT:
;HL - AddressOfModule
    LD   (MODADDR),HL
    PUSH HL
    LD   DE,100
    ADD  HL,DE
    LD   A,(HL)
    LD   (Delay),A
    PUSH HL
    POP  IX
    ADD  HL,DE
    LD   (CrPsPtr),HL
    LD   E,(IX+102-100)
    ADD  HL,DE
    INC  HL
    LD   (LPosPtr),HL
    POP  DE
    LD   L,(IX+103-100)
    LD   H,(IX+104-100)
    ADD  HL,DE
    LD   (PatsPtr),HL
    LD   HL,169
    ADD  HL,DE
    LD   (OrnPtrs),HL
    LD   HL,105
    ADD  HL,DE
    LD   (SamPtrs),HL
    LD   HL,SETUP
    RES  7,(HL)

;note table data depacker
    LD   DE,T_PACK
    LD   BC,T1_+(2*49)-1
TP_0:
    LD   A,(DE)
    INC  DE
    CP   15*2
    JR   NC,TP_1
    LD   H,A
    LD   A,(DE)
    LD   L,A
    INC  DE
    JR   TP_2
TP_1:
    PUSH DE
    LD   D,0
    LD   E,A
    ADD  HL,DE
    ADD  HL,DE
    POP  DE
TP_2:
    LD   A,H
    LD   (BC),A
    DEC  BC
    LD   A,L
    LD   (BC),A
    DEC  BC
    SUB  low($F8*2)
    JR   NZ,TP_0

    LD   HL,ChanA
    LD   (HL),A                   ; clear first variable byte
    LD   D,H
    LD   E,L
    INC  DE
    LD   BC,VAR0END-ChanA-1
    LDIR                          ; clear all other variable bytes
    INC  A
    LD   (DelyCnt),A
    LD   HL,$F001 ;H - Volume, L - NtSkCn
    LD   (ChanA+NtSkCn),HL
    LD   (ChanB+NtSkCn),HL
    LD   (ChanC+NtSkCn),HL

    LD   HL,EMPTYSAMORN
    LD   (AdInPtA),HL ;ptr to zero
    LD   (ChanA+OrnPtr),HL ;ornament 0 is "0,1,0"
    LD   (ChanB+OrnPtr),HL ;in all Versions from
    LD   (ChanC+OrnPtr),HL ;3.xx to 3.6x and VTII

    LD   (ChanA+SamPtr),HL ;S1 There is no default
    LD   (ChanB+SamPtr),HL ;S2 sample in PT3, so, you
    LD   (ChanC+SamPtr),HL ;S3 can comment S1,2,3; see
                    ;also EMPTYSAMORN comment
    LD   A,(IX+13-100) ;EXTRACT Version NUMBER
    SUB  $30
    JR   C,L20
    CP   10
    JR   C,L21
L20 LD   A,6
L21 LD   (PtVersion),A
    PUSH AF
    CP   4
    LD   A,(IX+99-100) ;TONE TABLE NUMBER
    RLA
    AND  7
;NoteTableCreator (c) Ivan Roshin
;A - NoteTableNumber*2+VersionForNoteTable
;(xx1b - 3.xx..3.4r, xx0b - 3.4x..3.6x..VTII1.0)
    LD   HL,NT_DATA
    PUSH DE
    LD   D,B
    ADD  A,A
    LD   E,A
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    SRL  E
    SBC  A,A
    AND  $A7      ; $00 (NOP) or $A7 (AND A)
    LD   (L3),A
    LD   A,$C9    ; RET temporary
    LD   (L3+1),A ; temporary
    EX   DE,HL
    POP  BC       ; BC=T1_
    ADD  HL,BC

    LD   A,(DE)

    ADD  A,low(T_)
    LD   C,A
    ADC  A,high(T_)

    SUB  C
    LD   B,A
    PUSH BC
    LD   DE,NT_
    PUSH DE

    LD   B,12
L1  PUSH BC
    LD   C,(HL)
    INC  HL
    PUSH HL
    LD   B,(HL)

    PUSH DE
    EX   DE,HL
    LD   DE,23
    LD   IXH,8

L2  SRL  B
    RR   C
    CALL L3     ;temporary
;L3 DB    $19   ;AND A or NOP
    LD   A,C
    ADC  A,D    ;=ADC 0
    LD   (HL),A
    INC  HL
    LD   A,B
    ADC  A,D
    LD   (HL),A
    ADD  HL,DE
    DEC  IXH
    JR   NZ,L2

    POP  DE
    INC  DE
    INC  DE
    POP  HL
    INC  HL
    POP  BC
    DJNZ L1

    POP  HL
    POP  DE

    LD   A,E
    CP   low(TCOLD_1)
    JR   NZ,CORR_1
    LD   A,$FD
    LD   (NT_+$2E),A

CORR_1:
    LD   A,(DE)
    AND  A
    JR   Z,TC_EXIT
    RRA
    PUSH AF
    ADD  A,A
    LD   C,A
    ADD  HL,BC
    POP  AF
    JR   NC,CORR_2
    DEC  (HL)
    DEC  (HL)
CORR_2:
    INC  (HL)
    AND  A
    SBC  HL,BC
    INC  DE
    JR   CORR_1

TC_EXIT:
    POP  AF

;VolTableCreator (c) Ivan Roshin
;A - VersionForVolumeTable (0..4 - 3.xx..3.4x;
               ;5.. - 3.5x..3.6x..VTII1.0)
    CP   5
    LD   HL,$11
    LD   D,H
    LD   E,H
    LD   A,$17
    JR   NC,M1
    DEC  L
    LD   E,L
    XOR  A
M1:
    LD   (M2),A      ; $17 or $00

    LD   IX,VT_+16
    LD   C,$10

INITV2:
    PUSH HL
    ADD  HL,DE
    EX   DE,HL
    SBC  HL,HL
INITV1:
    LD   A,L
;M2      DB $7D
    CALL M2 ;temporary
    LD   A,H
    ADC  A,0
    LD   (IX),A
    INC  IX
    ADD  HL,DE
    INC  C
    LD   A,C
    AND  15
    JR   NZ,INITV1

    POP  HL
    LD   A,E
    CP   $77
    JR   NZ,M3
    INC  E
M3:
    LD   A,C
    AND  A
    JR   NZ,INITV2
    JP   ROUT_A0

;pattern decoder
PD_OrSm:
    LD   (IX-12+Env_En),0
    CALL SETORN
    LD   A,(BC)
    INC  BC
    RRCA
PD_SAM:
    ADD  A,A
PD_SAM_:
    LD   E,A
    LD   D,0
    LD   HL,(SamPtrs)
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    LD   HL,(MODADDR)
    ADD  HL,DE
    LD   (IX-12+SamPtr),L
    LD   (IX-12+SamPtr+1),H
    JR   PD_LOOP

PD_VOL:
    RLCA
    RLCA
    RLCA
    RLCA
    LD   (IX-12+Volume),A
    JR   PD_LP2

PD_EOff:
    LD  (IX-12+Env_En),A
    LD  (IX-12+PsInOr),A
    JR   PD_LP2

PD_SorE:
    DEC  A
    JR   NZ,PD_ENV
    LD   A,(BC)
    INC  BC
    LD   (IX-12+NNtSkp),A
    JR   PD_LP2

PD_ENV:
    CALL SETENV
    JR   PD_LP2

PD_ORN:
    CALL SETORN
    JR   PD_LOOP

PD_ESAM:
    LD   (IX-12+Env_En),A
    LD   (IX-12+PsInOr),A
    CALL NZ,SETENV
    LD   A,(BC)
    INC  BC
    JR   PD_SAM_

PTDECOD:
    LD   A,(IX-12+Note)
    LD   (PrNote),A
    LD   L,(IX-12+CrTnSl)
    LD   H,(IX-12+CrTnSl+1)
    LD   (PrSlide),HL
PD_LOOP:
    LD   DE,$2010
PD_LP2:
    LD   A,(BC)
    INC  BC
    ADD  A,E
    JR   C,PD_OrSm
    ADD  A,D
    JR   Z,PD_FIN
    JR   C,PD_SAM
    ADD  A,E
    JR   Z,PD_REL
    JR   C,PD_VOL
    ADD  A,E
    JR   Z,PD_EOff
    JR   C,PD_SorE
    ADD  A,96
    JR   C,PD_NOTE
    ADD  A,E
    JR   C,PD_ORN
    ADD  A,D
    JR   C,PD_NOIS
    ADD  A,E
    JR   C,PD_ESAM
    ADD  A,A
    LD   E,A
    LD   HL,SPCCOMS-$20E0 ; +$FF20-$2000
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    PUSH DE
    JR   PD_LOOP

PD_NOIS:
    LD   (Ns_Base),A
    JR   PD_LP2

PD_REL:
    RES  0,(IX-12+PtFlags)
    JR   PD_RES

PD_NOTE:
    LD   (IX-12+Note),A
    SET  0,(IX-12+PtFlags)
    XOR  A
PD_RES:   ;LD (SaveSP+1),SP
    LD   (SaveSP),SP
    LD   SP,IX
    LD   H,A
    LD   L,A
    PUSH HL
    PUSH HL
    PUSH HL
    PUSH HL
    PUSH HL
    PUSH HL
    LD   SP,(SaveSP)

PD_FIN:
    LD   A,(IX-12+NNtSkp)
    LD   (IX-12+NtSkCn),A
    RET

C_PORTM:
    RES  2,(IX-12+PtFlags)
    LD   A,(BC)
    INC  BC
;SKIP PRECALCULATED TONE DELTA (BECAUSE
;CANNOT BE RIGHT AFTER PT3 COMPILATION)
    INC  BC
    INC  BC
    LD   (IX-12+TnSlDl),A
    LD   (IX-12+TSlCnt),A
    LD   DE,NT_
    LD   A,(IX-12+Note)
    LD   (IX-12+SlToNt),A
    ADD  A,A
    LD   L,A
    LD   H,0
    ADD  HL,DE
    LD   A,(HL)
    INC  HL
    LD   H,(HL)
    LD   L,A
    PUSH HL
    LD   A,(PrNote)
    LD   (IX-12+Note),A
    ADD  A,A
    LD   L,A
    LD   H,0
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    POP  HL
    SBC  HL,DE
    LD   (IX-12+TnDelt),L
    LD   (IX-12+TnDelt+1),H
    LD   E,(IX-12+CrTnSl)
    LD   D,(IX-12+CrTnSl+1)
    LD   A,(PtVersion)
    CP   6
    JR   C,OLDPRTM ;Old 3xxx for PT v3.5-
    LD   DE,(PrSlide)
    LD   (IX-12+CrTnSl),E
    LD   (IX-12+CrTnSl+1),D
OLDPRTM:
    LD   A,(BC) ;SIGNED TONE STEP
    INC  BC
    EX   AF,AF'
    LD   A,(BC)
    INC  BC
    AND  A
    JR   Z,NOSIG
    EX   DE,HL
NOSIG:
    SBC  HL,DE
    JP   P,SET_STP
    CPL
    EX   AF,AF'
    NEG
    EX   AF,AF'
SET_STP:
    LD   (IX-12+TSlStp+1),A
    EX   AF,AF'
    LD   (IX-12+TSlStp),A
    LD   (IX-12+COnOff),0
    RET

C_GLISS:
    SET  2,(IX-12+PtFlags)
    LD   A,(BC)
    INC  BC
    LD   (IX-12+TnSlDl),A
    AND  A
    JR   NZ,GL36
    LD   A,(PtVersion) ;AlCo PT3.7+
    CP   7
    SBC  A,A
    INC  A
GL36:
    LD   (IX-12+TSlCnt),A
    LD   A,(BC)
    INC  BC
    EX   AF,AF'
    LD   A,(BC)
    INC  BC
    JR   SET_STP

C_SMPOS:
    LD   A,(BC)
    INC  BC
    LD   (IX-12+PsInSm),A
    RET

C_ORPOS:
    LD   A,(BC)
    INC  BC
    LD   (IX-12+PsInOr),A
    RET

C_VIBRT:
    LD   A,(BC)
    INC  BC
    LD   (IX-12+OnOffD),A
    LD   (IX-12+COnOff),A
    LD   A,(BC)
    INC  BC
    LD   (IX-12+OffOnD),A
    XOR  A
    LD   (IX-12+TSlCnt),A
    LD   (IX-12+CrTnSl),A
    LD   (IX-12+CrTnSl+1),A
    RET

C_ENGLS:
    LD   A,(BC)
    INC  BC
    LD   (Env_Del),A
    LD   (CurEDel),A
    LD   A,(BC)
    INC  BC
    LD   L,A
    LD   A,(BC)
    INC  BC
    LD   H,A
    LD   (ESldAdd),HL
    RET

C_DELAY:
    LD   A,(BC)
    INC  BC
    LD   (Delay),A
    RET

SETENV:
    LD   (IX-12+Env_En),E
    LD   (AYREGS+EnvTp),A
    LD   A,(BC)
    INC  BC
    LD   H,A
    LD   A,(BC)
    INC  BC
    LD   L,A
    LD   (EnvBase),HL
    XOR  A
    LD   (IX-12+PsInOr),A
    LD   (CurEDel),A
    LD   H,A
    LD   L,A
    LD   (CurESld),HL
C_NOP:
    RET

SETORN:
    ADD  A,A
    LD   E,A
    LD   D,0
    LD   (IX-12+PsInOr),D
    LD   HL,(OrnPtrs)
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    LD   HL,(MODADDR)
    ADD  HL,DE
    LD   (IX-12+OrnPtr),L
    LD   (IX-12+OrnPtr+1),H
    RET

;ALL 16 ADDRESSES TO PROTECT FROM BROKEN PT3 MODULES
SPCCOMS:
    DW   C_NOP
    DW   C_GLISS
    DW   C_PORTM
    DW   C_SMPOS
    DW   C_ORPOS
    DW   C_VIBRT
    DW   C_NOP
    DW   C_NOP
    DW   C_ENGLS
    DW   C_DELAY
    DW   C_NOP
    DW   C_NOP
    DW   C_NOP
    DW   C_NOP
    DW   C_NOP
    DW   C_NOP

CHREGS:
    XOR  A
    LD   (Ampl),A
    BIT  0,(IX+PtFlags)
    PUSH HL
    JP   Z,CH_EXIT
    LD   (SaveSP),SP
    LD   L,(IX+OrnPtr)
    LD   H,(IX+OrnPtr+1)
    LD   SP,HL
    POP  DE
    LD   H,A
    LD   A,(IX+PsInOr)
    LD   L,A
    ADD  HL,SP
    INC  A
    CP   D
    JR   C,CH_ORPS
    LD   A,E
CH_ORPS:
    LD   (IX+PsInOr),A
    LD   A,(IX+Note)
    ADD  A,(HL)
    JP   P,CH_NTP
    XOR  A
CH_NTP:
    CP   96
    JR   C,CH_NOK
    LD   A,95
CH_NOK:
    ADD  A,A
    EX   AF,AF'
    LD   L,(IX+SamPtr)
    LD   H,(IX+SamPtr+1)
    LD   SP,HL
    POP  DE
    LD   H,0
    LD   A,(IX+PsInSm)
    LD   B,A
    ADD  A,A
    ADD  A,A
    LD   L,A
    ADD  HL,SP
    LD   SP,HL
    LD   A,B
    INC  A
    CP   D
    JR   C,CH_SMPS
    LD   A,E
CH_SMPS:
    LD   (IX+PsInSm),A
    POP  BC
    POP  HL
    LD   E,(IX+TnAcc)
    LD   D,(IX+TnAcc+1)
    ADD  HL,DE
    BIT  6,B
    JR   Z,CH_NOAC
    LD   (IX+TnAcc),L
    LD   (IX+TnAcc+1),H
CH_NOAC:
    EX   DE,HL
    EX   AF,AF'
    LD   L,A
    LD   H,0
    LD   SP,NT_
    ADD  HL,SP
    LD   SP,HL
    POP  HL
    ADD  HL,DE
    LD   E,(IX+CrTnSl)
    LD   D,(IX+CrTnSl+1)
    ADD  HL,DE
    LD   SP,(SaveSP)
    EX   (SP),HL
    XOR  A
    OR   (IX+TSlCnt)
    JR   Z,CH_AMP
    DEC  (IX+TSlCnt)
    JR   NZ,CH_AMP
    LD   A,(IX+TnSlDl)
    LD   (IX+TSlCnt),A
    LD   L,(IX+TSlStp)
    LD   H,(IX+TSlStp+1)
    LD   A,H
    ADD  HL,DE
    LD   (IX+CrTnSl),L
    LD   (IX+CrTnSl+1),H
    BIT  2,(IX+PtFlags)
    JR   NZ,CH_AMP
    LD   E,(IX+TnDelt)
    LD   D,(IX+TnDelt+1)
    AND  A
    JR   Z,CH_STPP
    EX   DE,HL
CH_STPP:
    SBC  HL,DE
    JP   M,CH_AMP
    LD   A,(IX+SlToNt)
    LD   (IX+Note),A
    XOR  A
    LD   (IX+TSlCnt),A
    LD   (IX+CrTnSl),A
    LD   (IX+CrTnSl+1),A
CH_AMP:
    LD   A,(IX+CrAmSl)
    BIT  7,C
    JR   Z,CH_NOAM
    BIT  6,C
    JR   Z,CH_AMIN
    CP   15
    JR   Z,CH_NOAM
    INC  A
    JR   CH_SVAM
CH_AMIN:
    CP   -15
    JR   Z,CH_NOAM
    DEC  A
CH_SVAM:
    LD   (IX+CrAmSl),A
CH_NOAM:
    LD   L,A
    LD   A,B
    AND  15
    ADD  A,L
    JP   P,CH_APOS
    XOR  A
CH_APOS:
    CP   16
    JR   C,CH_VOL
    LD   A,15
CH_VOL:
    OR   (IX+Volume)
    LD   L,A
    LD   H,0
    LD   DE,VT_
    ADD  HL,DE
    LD   A,(HL)
CH_ENV:
    BIT  0,C
    JR   NZ,CH_NOEN
    OR   (IX+Env_En)
CH_NOEN:
    LD   (Ampl),A
    BIT  7,B
    LD   A,C
    JR   Z,NO_ENSL
    RLA
    RLA
    SRA  A
    SRA  A
    SRA  A
    ADD  A,(IX+CrEnSl) ;SEE COMMENT BELOW
    BIT  5,B
    JR   Z,NO_ENAC
    LD   (IX+CrEnSl),A
NO_ENAC:
    LD   HL,AddToEn
    ADD  A,(HL)    ;BUG IN PT3 - NEED WORD HERE.
                   ;FIX IT IN NEXT Version?
    LD   (HL),A
    JR   CH_MIX
NO_ENSL:
    RRA
    ADD  A,(IX+CrNsSl)
    LD   (AddToNs),A
    BIT  5,B
    JR   Z,CH_MIX
    LD   (IX+CrNsSl),A
CH_MIX:
    LD   A,B
    RRA
    AND  $48
CH_EXIT:
    LD   HL,AYREGS+Mixer
    OR   (HL)
    RRCA
    LD   (HL),A
    POP  HL
    XOR  A
    OR   (IX+COnOff)
    RET  Z
    DEC  (IX+COnOff)
    RET  NZ
    XOR  (IX+PtFlags)
    LD   (IX+PtFlags),A
    RRA
    LD   A,(IX+OnOffD)
    JR   C,CH_ONDL
    LD   A,(IX+OffOnD)
CH_ONDL:
    LD   (IX+COnOff),A
    RET

PLAY:
    XOR   A
    LD   (AddToEn),A
    LD   (AYREGS+Mixer),A
    DEC   A
    LD   (AYREGS+EnvTp),A
    LD   HL,DelyCnt
    DEC  (HL)
    JP   NZ,PL2
    LD   HL,ChanA+NtSkCn
    DEC  (HL)
    JR   NZ,PL1B
    LD   BC,(AdInPtA)
    LD   A,(BC)
    AND  A
    JR   NZ,PL1A
    LD   D,A
    LD   (Ns_Base),A
    LD   HL,(CrPsPtr)
    INC  HL
    LD   A,(HL)
    INC  A
    JR   NZ,PLNLP
    CALL CHECKLP
    LD   HL,(LPosPtr)
    LD   A,(HL)
    INC  A
PLNLP:
    LD   (CrPsPtr),HL
    DEC  A
    ADD  A,A
    LD   E,A
    RL   D
    LD   HL,(PatsPtr)
    ADD  HL,DE
    LD   DE,(MODADDR)
    LD   (SaveSP),SP
    LD   SP,HL
    POP  HL
    ADD  HL,DE
    LD   B,H
    LD   C,L
    POP  HL
    ADD  HL,DE
    LD   (AdInPtB),HL
    POP  HL
    ADD  HL,DE
    LD   (AdInPtC),HL
    LD   SP,(SaveSP)
PL1A:
    LD   IX,ChanA+12
    CALL PTDECOD
    LD   (AdInPtA),BC

PL1B:
    LD   HL,ChanB+NtSkCn
    DEC  (HL)
    JR   NZ,PL1C
    LD   IX,ChanB+12
    LD   BC,(AdInPtB)
    CALL PTDECOD
    LD   (AdInPtB),BC
PL1C:
    LD   HL,ChanC+NtSkCn
    DEC  (HL)
    JR   NZ,PL1D
    LD   IX,ChanC+12
    LD   BC,(AdInPtC)
    CALL PTDECOD
    LD   (AdInPtC),BC

PL1D:
    LD   A,(Delay)
    LD   (DelyCnt),A

PL2:
    LD   IX,ChanA
    LD   HL,(AYREGS+TonA)
    CALL CHREGS
    LD   (AYREGS+TonA),HL
    LD   A,(Ampl)
    LD   (AYREGS+AmplA),A
    LD   IX,ChanB
    LD   HL,(AYREGS+TonB)
    CALL CHREGS
    LD   (AYREGS+TonB),HL
    LD   A,(Ampl)
    LD   (AYREGS+AmplB),A
    LD   IX,ChanC
    LD   HL,(AYREGS+TonC)
    CALL CHREGS
    LD   (AYREGS+TonC),HL
    LD   HL,(Ns_Base_AddToNs)
    LD   A,H
    ADD  A,L
    LD   (AYREGS+Noise),A
    LD   A,(AddToEn)
    LD   E,A
    ADD  A,A
    SBC  A,A
    LD   D,A
    LD   HL,(EnvBase)
    ADD  HL,DE
    LD   DE,(CurESld)
    ADD  HL,DE
    LD   (AYREGS+Env),HL
    XOR  A
    LD   HL,CurEDel
    OR   (HL)
    JR   Z,ROUT_A0
    DEC (HL)
    JR   NZ,ROUT
    LD   A,(Env_Del)
    LD   (HL),A
    LD   HL,(ESldAdd)
    ADD  HL,DE
    LD   (CurESld),HL
ROUT:
    XOR  A           ; A  = register 0
ROUT_A0:
    LD   HL,AYREGS   ; HL = register data
    LD   C,$F6       ; C  = AY data port
LOUT:
    OUT  ($F7),A     ; select register
    INC  A           ; A = next register
    OUTI             ; (HL) -> AY register, inc HL
    CP   13          ; loaded registers 0~12?
    JR   NZ,LOUT
    OUT  ($F7),A     ; select register 13
    LD   A,(HL)      ; get register 13 data
    AND  A
    RET  M           ; return if bit 7 = 1
    OUT  (C),A       ; load register 13
    RET

;Stupid ALASM limitations
NT_DATA:
    DB (T_NEW_0-T1_)*2 ; 50*2
    DB TCNEW_0-T_
    DB 50*2+1          ;(T_OLD_0-T1_)*2+1
    DB TCOLD_0-T_
    DB 0*2+1           ;(T_NEW_1-T1_)*2+1
    DB TCNEW_1-T_
    DB 0*2+1           ;(T_OLD_1-T1_)*2+1
    DB TCOLD_1-T_
    DB 74*2            ;(T_NEW_2-T1_)*2
    DB TCNEW_2-T_
    DB 24*2            ;(T_OLD_2-T1_)*2
    DB TCOLD_2-T_
    DB 48*2            ;(T_NEW_3-T1_)*2
    DB TCNEW_3-T_
    DB 48*2            ;(T_OLD_3-T1_)*2
    DB TCOLD_3-T_

T_

TCOLD_0:
    DB $00+1,$04+1,$08+1,$0A+1,$0C+1,$0E+1,$12+1,$14+1
    DB $18+1,$24+1,$3C+1,0
TCOLD_1:
TCNEW_1:
    DB $5C+1,0
TCOLD_2:
    DB $30+1,$36+1,$4C+1,$52+1,$5E+1,$70+1,$82,$8C,$9C
    DB $9E,$A0,$A6,$A8,$AA,$AC,$AE,$AE,0
TCNEW_3:
    DB $56+1
TCOLD_3:
    DB $1E+1,$22+1,$24+1,$28+1,$2C+1,$2E+1,$32+1,$BE+1,0
TCNEW_0:
    DB $1C+1,$20+1,$22+1,$26+1,$2A+1,$2C+1,$30+1,$54+1
    DB $BC+1,$BE+1,0
TCNEW_2:
    DB $1A+1,$20+1,$24+1,$28+1,$2A+1,$3A+1,$4C+1,$5E+1
    DB $BA+1,$BC+1,$BE+1
EMPTYSAMORN:
    DB 0
    DB 1,0,$90 ;delete $90 if you don't need default sample

;first 12 values of tone tables (packed)

T_PACK:
    DB high($06EC*2),low($06EC*2)
    DB $0755-$06EC
    DB $07C5-$0755
    DB $083B-$07C5
    DB $08B8-$083B
    DB $093D-$08B8
    DB $09CA-$093D
    DB $0A5F-$09CA
    DB $0AFC-$0A5F
    DB $0BA4-$0AFC
    DB $0C55-$0BA4
    DB $0D10-$0C55
    DB high($066D*2),low($066D*2)
    DB $06CF-$066D
    DB $0737-$06CF
    DB $07A4-$0737
    DB $0819-$07A4
    DB $0894-$0819
    DB $0917-$0894
    DB $09A1-$0917
    DB $0A33-$09A1
    DB $0ACF-$0A33
    DB $0B73-$0ACF
    DB $0C22-$0B73
    DB $0CDA-$0C22
    DB high($0704*2),low($0704*2)
    DB $076E-$0704
    DB $07E0-$076E
    DB $0858-$07E0
    DB $08D6-$0858
    DB $095C-$08D6
    DB $09EC-$095C
    DB $0A82-$09EC
    DB $0B22-$0A82
    DB $0BCC-$0B22
    DB $0C80-$0BCC
    DB $0D3E-$0C80
    DB high($07E0*2),low($07E0*2)
    DB $0858-$07E0
    DB $08E0-$0858
    DB $0960-$08E0
    DB $09F0-$0960
    DB $0A88-$09F0
    DB $0B28-$0A88
    DB $0BD8-$0B28
    DB $0C80-$0BD8
    DB $0D60-$0C80
    DB $0E10-$0D60
    DB $0EF8-$0E10

SelectWindow:
      db   (1<<WA_BORDER)|(1<<WA_TITLE)|(1<<WA_CENTER) ; attributes
      db   WHITE*16+BLACK               ; text colors
      db   CYAN*16+BLACK                ; border colors
      db   1,2,38,22                    ; x,y,w,h
      dw   .title                       ; title
.title:
      db   " PT3 Player ",0

PlayWindow:
      db   0                            ; attributes
      db   WHITE*16+BLACK               ; text colors
      db   0                            ; border colors
      db   1,2,38,22                    ; x,y,w,h
      dw   0                            ; title

PT3pat:
      db   "*.PT3",0

PT3help:
      db   "SPACE = next song    RTN = playlist",0

