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




    include "pt3player.asm"




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








PT3_PLAY:
    LD   IX, SelectWindow
    CALL OpenWindow
    LD   HL, pt3_files     ; array of file infos to fill
    LD   B, 36             ; 36 files max.
    LD   DE, pt3pat        ; "*.PT3"
    CALL RequestFile       ; user selects file
    RET  NZ                ; if no file selected then quit
    LD   (Song), A
    LD   A, C
    LD   (NumSongs), A

PT3_LOAD:
    LD   IX, PlayWindow
    CALL OpenWindow        ; open/clear window
    LD   DE, 256*1+21
    CALL WinSetCursor      ; cursor at window bottm/left
    LD   HL, PT3help
    CALL WinPrtStr         ; show key help
    LD   A, (Song)
    LD   HL, pt3_files
    CALL IndexArray        ; HL = pt3files[song]
    PUSH HL
    POP  IY
    BIT  ATTR_B_DIRECTORY, (IY+11) ; directory?
    JR   NZ, .next_song    ; yes, skip file
    PUSH HL                ; push filename
    LD   D, 13
    LD   E, 1
    CALL WinSetCursor
    LD   A, (SONG)
    ADD  '0'
    CP   '0' + 10          ; ID = '0-9', 'A-Z'
    JR   C, .shownum
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
    LD   HL, SongData
    LD   DE,-1
    CALL usb__read_bytes   ; read song file into buffer
    PUSH AF
    CALL usb__close_file
    POP  AF
    RET  NZ                ; quit if error reading file
    LD   HL, SongData
    CALL INIT              ; initialize player
.play_quark:
    LD   B, 5              ; play 5 'quarks'
.play_loop:
    PUSH BC
    CALL wait_vbl
    CALL Animation
    LD   HL, SongData
    CALL PLAY              ; play next line
    POP  BC
    ld   hl, SETUP
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

    ; delay one vblank to get 50Hz playback
    ;    CALL wait_vbl          ; if NTSC then delay one vblank to get 50Hz playback

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
.upd:
    LD   (Song),A
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
.bar11:
    DEC  A
    JR   NZ,.bar12
    LD   C,E             ; no bar color
.bar12:
    LD   (HL),C
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


