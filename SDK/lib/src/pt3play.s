    .z80

; PT3 header
;
; | Offset (hex) | Offset (dec) | Size | Description                         | Contents                           |
; | ------------ | ------------ | ---: | ----------------------------------- | ---------------------------------- |
; | $00 - $0C    | 0-12         |   13 | Magic                               | "ProTracker 3."                    |
; | $0D          | 13           |    1 | Version                             | '5' for Vortex Tracker II          |
; | $0E - $1D    | 14-29        |   16 | String                              | " compilation of "                 |
; | $1E - $3D    | 30-61        |   32 | Name                                | Name of the module                 |
; | $3E - $41    | 62-65        |    4 | String                              | " by "                             |
; | $42 - $62    | 66-98        |   33 | Author                              | Author of the module.              |
; | $63          | 99           |    1 | Frequency table (from 0 to 3)       |                                    |
; | $64          | 100          |    1 | Speed/Delay                         |                                    |
; | $65          | 101          |    1 | Number of patterns+1 (Max Patterns) |                                    |
; | $66          | 102          |    1 | LPosPtr                             | Pattern Loop Pointer               |
; | $67 - $68    | 103-104      |    2 | PatsPtrs                            | Pointers to the patterns           |
; | $69 - $A8    | 105-168      |   64 | SamPtrs[32]                         | Pointers to the samples            |
; | $A9 - $C8    | 169-200      |   32 | OrnPtrs[16]                         | Pointers to the ornaments          |
; | $C9 - ???    | 201-???      |      | Patterns[]                          | $FF terminated, More on this below |

; 60Hz = -1  ; make this -1 to play 50Hz songs on 60Hz machine

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

; ix for PTDECOD here (+12)
OffOnD   = 12
OrnPtr   = 13
SamPtr   = 15
NNtSkp   = 17
Note     = 18
SlToNt   = 19
Env_En   = 20
PtFlags  = 21
; Enabled - 0,SimpleGliss - 2
TnSlDl   = 22
TSlStp   = 23
TnDelt   = 25
NtSkCn   = 27
Volume   = 28
;   ENDS
CHP_size = 29

;---------------------------------------------------------
; Variables (and self-modifying code, if present) in RAM
; NOTE: must be initialized before use!
;---------------------------------------------------------

    .area _DATA

SETUP:     .db 0    ; bit7=1 when loop point reached
CrPsPtr:   .dw 0    ; Pointer to current pattern index
AddToEn:   .db 0
AdInPtA:   .dw 0
AdInPtB:   .dw 0
AdInPtC:   .dw 0
Env_Del:   .db 0
ModAddr:   .dw 0    ; Address of start of PT3 file
ESldAdd:   .dw 0
Delay:     .db 0    ; Number of frames per line
SaveSP:    .dw 0
SamPtrs:   .dw 0
OrnPtrs:   .dw 0
PatsPtr:   .dw 0    ; Pointers to the patterns
LPosPtr:   .dw 0
PrSlide:   .dw 0
PrNote:    .db 0
PtVersion: .db 0
;end of variables and self-modifying code
;start of cleared data area
ChanA:     .ds CHP_size
ChanB:     .ds CHP_size
ChanC:     .ds CHP_size

;GlobalVars
DelyCnt:   .db 0
CurESld:   .dw 0
CurEDel:   .dw 0
;arrays
Ns_Base:   .db 0
AddToNs:   .db 0

VT_:       .ds 256     ; 256 bytes CreatedVolumeTableAddress
NT_:       .ds 192     ; 192 bytes Note Table
VAREND:


Ns_Base_AddToNs = Ns_Base

L3       = PrSlide    ; opcode + ret
M2       = PrSlide

AYREGS   == VT_      ; 14 AY-3-8910 registers
_pt3play_ayregs == AYREGS


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


    .area _CODE

; Wrapper to call INIT from C
; void pt3play_init(void *pt3);
_pt3play_init::
    push ix
    push iy

    call INIT

    pop iy
    pop ix
    ret

; Wrapper to call PLAY from C
; bool pt3play_play(void);
_pt3play_play::
    push ix
    push iy

    call PLAY

    xor a
    ld  hl, #SETUP
    bit 7, (hl)
    jr  z, .done
    ld  a, #1
.done:
    pop iy
    pop ix
    ret

; Wrapper to call MUTE from C
; void _pt3play_mute(void);
_pt3play_mute::
    push ix
    push iy

    call MUTE

    pop iy
    pop ix
    ret



;PT3 player version
    ; .db   "=VTII PT3 Player r.7ROM="

CHECKLP:
    ld   hl,#SETUP
    set  7,(hl)
    bit  0,(hl)
    ret  Z
    pop  hl
    ld   hl,#DelyCnt
    inc  (hl)
    ld   hl,#ChanA+NtSkCn
    inc  (hl)
MUTE:
    xor  a
    ld   h,a
    ld   l,a
    ld   (AYREGS+AmplA),a
    ld   (AYREGS+AmplB),hl
    jp   ROUT_A0

INIT:
    ; hl - AddressOfModule
    ld   (ModAddr),hl

    ; Save hl
    push hl

    ; hl = ModAddr + 100
    ld   de,#100
    add  hl,de

    ; Delay = *(u8*)(ModAddr + 100)
    ld   a,(hl)
    ld   (Delay),a

    ; ix = ModAddr + 100
    push hl
    pop  ix

    ; CrPsPtr = ModAddr + 200  (start of pattern list - 1)
    add  hl,de
    ld   (CrPsPtr),hl

    ; LPosPtr =    ModAddr + 102
    ld   e,2(ix)        ; FIXME: This will fail if loading address isnt aligned properly
    add  hl,de
    inc  hl
    ld   (LPosPtr),hl

    pop  de

    ; 103/104: Pointers to the patterns
    ld   l,3(ix)
    ld   h,4(ix)
    add  hl,de
    ld   (PatsPtr),hl

    ; 
    ld   hl,#169
    add  hl,de
    ld   (OrnPtrs),hl

    ;
    ld   hl,#105
    add  hl,de
    ld   (SamPtrs),hl

    ld   hl,#SETUP
    res  7,(hl)

    ; note table data depacker
    ld   de,#T_PACK
    ld   bc,#T1_+(2*49)-1
TP_0:
    ld   a,(de)
    inc  de
    cp   #15*2
    jr   nc,TP_1
    ld   h,a
    ld   a,(de)
    ld   l,a
    inc  de
    jr   TP_2
TP_1:
    push de
    ld   d,#0
    ld   e,a
    add  hl,de
    add  hl,de
    pop  de
TP_2:
    ld   a,h
    ld   (bc),a
    dec  bc
    ld   a,l
    ld   (bc),a
    dec  bc
    sub  #<(0xF8*2)
    jr   nz,TP_0

    ld   hl,#ChanA
    ld   (hl),a                   ; clear first variable byte
    ld   d,h
    ld   e,l
    inc  de
    ld   bc,#(VAR0END-ChanA-1)
    ldir                          ; clear all other variable bytes
    inc  a
    ld   (DelyCnt),a
    ld   hl,#0xF001 ;H - Volume, L - NtSkCn
    ld   (ChanA+NtSkCn),hl
    ld   (ChanB+NtSkCn),hl
    ld   (ChanC+NtSkCn),hl

    ld   hl,#EMPTYSAMORN
    ld   (AdInPtA),hl       ; ptr to zero
    ld   (ChanA+OrnPtr),hl  ; ornament 0 is "0,1,0"
    ld   (ChanB+OrnPtr),hl  ; in all Versions from
    ld   (ChanC+OrnPtr),hl  ; 3.xx to 3.6x and VTII

    ld   (ChanA+SamPtr),hl  ; S1 There is no default
    ld   (ChanB+SamPtr),hl  ; S2 sample in PT3, so, you
    ld   (ChanC+SamPtr),hl  ; S3 can comment S1,2,3; see
                            ; also EMPTYSAMORN comment
    ld   a,13-100(ix)       ; EXTRACT Version NUMBER
    sub  #0x30
    jr   c,L20
    cp   #10
    jr   c,L21
L20:
    ld   a,#6
L21:
    ld   (PtVersion),a
    push af
    cp   #4
    ld   a,99-100(ix) ;TONE TABLE NUMBER
    rla
    and  #7
;NoteTableCreator (c) Ivan Roshin
;A - NoteTableNumber*2+VersionForNoteTable
;(xx1b - 3.xx..3.4r, xx0b - 3.4x..3.6x..VTII1.0)
    ld   hl,#NT_DATA
    push de
    ld   d,b
    add  a,a
    ld   e,a
    add  hl,de
    ld   e,(hl)
    inc  hl
    srl  e
    sbc  a,a
    and  #0xA7      ; 0x00 (NOP) or 0xA7 (and A)
    ld   (L3),a
    ld   a,#0xC9    ; ret temporary
    ld   (L3+1),a ; temporary
    ex   de,hl
    pop  bc       ; bc=T1_
    add  hl,bc

    ld   a,(de)

    add  a,#<(T_)
    ld   c,a
    adc  a,#>(T_)

    sub  c
    ld   b,a
    push bc
    ld   de,#NT_
    push de

    ld   b,#12
L1: push bc
    ld   c,(hl)
    inc  hl
    push hl
    ld   b,(hl)

    push de
    ex   de,hl
    ld   de,#23

    .db 0xDD,0x26,0x08      ; TODO: use correct syntax instead for: ld ixh,#8

L2: srl  b
    rr   c
    call L3     ;temporary
;L3: DB    0x19   ;and A or NOP
    ld   a,c
    adc  a,d    ;=adc 0
    ld   (hl),a
    inc  hl
    ld   a,b
    adc  a,d
    ld   (hl),a
    add  hl,de
    dec  ixh
    jr   nz,L2

    pop  de
    inc  de
    inc  de
    pop  hl
    inc  hl
    pop  bc
    djnz L1

    pop  hl
    pop  de

    ld   a,e
    cp   #<(TCOLD_1)
    jr   nz,CORR_1
    ld   a,#0xFD
    ld   (NT_+0x2E),a

CORR_1:
    ld   a,(de)
    and  a
    jr   z,TC_EXIT
    rra
    push af
    add  a,a
    ld   c,a
    add  hl,bc
    pop  af
    jr   nc,CORR_2
    dec  (hl)
    dec  (hl)
CORR_2:
    inc  (hl)
    and  a
    sbc  hl,bc
    inc  de
    jr   CORR_1

TC_EXIT:
    pop  af

;VolTableCreator (c) Ivan Roshin
;A - VersionForVolumeTable (0..4 - 3.xx..3.4x;5.. - 3.5x..3.6x..VTII1.0)
    cp   #5
    ld   hl,#0x11
    ld   d,h
    ld   e,h
    ld   a,#0x17
    jr   nc,M1
    dec  l
    ld   e,l
    xor  a
M1:
    ld   (M2),a      ; 0x17 or 0x00

    ld   ix,#VT_+16
    ld   c,#0x10

INITV2:
    push hl
    add  hl,de
    ex   de,hl
    sbc  hl,hl
INITV1:
    ld   a,l
;M2      DB 0x7D
    call M2 ;temporary
    ld   a,h
    adc  a,#0
    ld   (ix),a
    inc  ix
    add  hl,de
    inc  c
    ld   a,c
    and  #15
    jr   nz,INITV1

    pop  hl
    ld   a,e
    cp   #0x77
    jr   nz,M3
    inc  e
M3:
    ld   a,c
    and  a
    jr   nz,INITV2
    jp   ROUT_A0

;pattern decoder
PD_OrSm:
    ld   -12+Env_En(ix),#0
    call SETORN
    ld   a,(bc)
    inc  bc
    rrca
PD_SAM:
    add  a,a
PD_SAM_:
    ld   e,a
    ld   d,#0
    ld   hl,(SamPtrs)
    add  hl,de
    ld   e,(hl)
    inc  hl
    ld   d,(hl)
    ld   hl,(ModAddr)
    add  hl,de
    ld   -12+SamPtr(ix),l
    ld   -12+SamPtr+1(ix),h
    jr   PD_LOOP

PD_VOL:
    rlca
    rlca
    rlca
    rlca
    ld   -12+Volume(ix),a
    jr   PD_LP2

PD_EOff:
    ld  -12+Env_En(ix),a
    ld  -12+PsInOr(ix),a
    jr   PD_LP2

PD_SorE:
    dec  A
    jr   nz,PD_ENV
    ld   A,(bc)
    inc  bc
    ld   -12+NNtSkp(ix),a
    jr   PD_LP2

PD_ENV:
    call SETENV
    jr   PD_LP2

PD_ORN:
    call SETORN
    jr   PD_LOOP

PD_ESAM:
    ld   -12+Env_En(ix),a
    ld   -12+PsInOr(ix),a
    call nz,SETENV
    ld   a,(bc)
    inc  bc
    jr   PD_SAM_

PTDECOD:
    ld   A,-12+Note(ix)
    ld   (PrNote),a
    ld   L,-12+CrTnSl(ix)
    ld   H,-12+CrTnSl+1(ix)
    ld   (PrSlide),hl
PD_LOOP:
    ld   de,#0x2010
PD_LP2:
    ld   A,(bc)
    inc  bc
    add  A,E
    jr   C,PD_OrSm
    add  A,D
    jr   Z,PD_FIN
    jr   C,PD_SAM
    add  A,E
    jr   Z,PD_REL
    jr   C,PD_VOL
    add  A,E
    jr   Z,PD_EOff
    jr   C,PD_SorE
    add  A,#96
    jr   C,PD_NOTE
    add  A,E
    jr   C,PD_ORN
    add  A,D
    jr   C,PD_NOIS
    add  A,E
    jr   C,PD_ESAM
    add  A,a
    ld   E,a
    ld   hl,#SPCCOMS-0x20E0 ; +0xFF20-0x2000
    add  hl,de
    ld   E,(hl)
    inc  hl
    ld   D,(hl)
    push de
    jr   PD_LOOP

PD_NOIS:
    ld   (Ns_Base),a
    jr   PD_LP2

PD_REL:
    res  0,-12+PtFlags(ix)
    jr   PD_RES

PD_NOTE:
    ld   -12+Note(ix),a
    set  0,-12+PtFlags(ix)
    xor  A
PD_RES:   ;ld (SaveSP+1),SP
    ld   (SaveSP),SP
    ld   SP,ix
    ld   H,a
    ld   L,a
    push hl
    push hl
    push hl
    push hl
    push hl
    push hl
    ld   SP,(SaveSP)

PD_FIN:
    ld   A,-12+NNtSkp(ix)
    ld   -12+NtSkCn(ix),a
    ret

C_PORTM:
    res  2,-12+PtFlags(ix)
    ld   A,(bc)
    inc  bc
;SKIP PRECALCULATED TONE DELTA (BECAUSE
;CANNOT BE RIGHT AFTER PT3 COMPILATION)
    inc  bc
    inc  bc
    ld   -12+TnSlDl(ix),a
    ld   -12+TSlCnt(ix),a
    ld   de,#NT_
    ld   A,-12+Note(ix)
    ld   -12+SlToNt(ix),a
    add  A,a
    ld   L,a
    ld   H,#0
    add  hl,de
    ld   A,(hl)
    inc  hl
    ld   H,(hl)
    ld   L,a
    push hl
    ld   A,(PrNote)
    ld   -12+Note(ix),a
    add  A,a
    ld   L,a
    ld   H,#0
    add  hl,de
    ld   E,(hl)
    inc  hl
    ld   D,(hl)
    pop  hl
    sbc  hl,de
    ld   -12+TnDelt(ix),L
    ld   -12+TnDelt+1(ix),H
    ld   E,-12+CrTnSl(ix)
    ld   D,-12+CrTnSl+1(ix)
    ld   A,(PtVersion)
    cp   #6
    jr   C,OLDPRTM ;Old 3xxx for PT v3.5-
    ld   de,(PrSlide)
    ld   -12+CrTnSl(ix),E
    ld   -12+CrTnSl+1(ix),D
OLDPRTM:
    ld   A,(bc) ;SIGNED TONE STEP
    inc  bc
    ex   af,af'
    ld   A,(bc)
    inc  bc
    and  A
    jr   Z,NOSIG
    ex   de,hl
NOSIG:
    sbc  hl,de
    jp   P,SET_STP
    CPL
    ex   af,af'
    NEG
    ex   af,af'
SET_STP:
    ld   -12+TSlStp+1(ix),a
    ex   af,af'
    ld   -12+TSlStp(ix),a
    ld   -12+COnOff(ix),#0
    ret

C_GLISS:
    set  2,-12+PtFlags(ix)
    ld   A,(bc)
    inc  bc
    ld   -12+TnSlDl(ix),a
    and  A
    jr   nz,GL36
    ld   A,(PtVersion) ;AlCo PT3.7+
    cp   #7
    sbc  A,a
    inc  A
GL36:
    ld   -12+TSlCnt(ix),a
    ld   A,(bc)
    inc  bc
    ex   af,af'
    ld   A,(bc)
    inc  bc
    jr   SET_STP

C_SMPOS:
    ld   A,(bc)
    inc  bc
    ld   -12+PsInSm(ix),a
    ret

C_ORPOS:
    ld   A,(bc)
    inc  bc
    ld   -12+PsInOr(ix),a
    ret

C_VIBRT:
    ld   A,(bc)
    inc  bc
    ld   -12+OnOffD(ix),a
    ld   -12+COnOff(ix),a
    ld   A,(bc)
    inc  bc
    ld   -12+OffOnD(ix),a
    xor  A
    ld   -12+TSlCnt(ix),a
    ld   -12+CrTnSl(ix),a
    ld   -12+CrTnSl+1(ix),a
    ret

C_ENGLS:
    ld   A,(bc)
    inc  bc
    ld   (Env_Del),a
    ld   (CurEDel),a
    ld   A,(bc)
    inc  bc
    ld   L,a
    ld   A,(bc)
    inc  bc
    ld   H,a
    ld   (ESldAdd),hl
    ret

C_DELAY:
    ld   A,(bc)
    inc  bc
    ld   (Delay),a
    ret

SETENV:
    ld   -12+Env_En(ix),E
    ld   (AYREGS+EnvTp),a
    ld   A,(bc)
    inc  bc
    ld   H,a
    ld   A,(bc)
    inc  bc
    ld   L,a
    ld   (EnvBase),hl
    xor  A
    ld   -12+PsInOr(ix),a
    ld   (CurEDel),a
    ld   H,a
    ld   L,a
    ld   (CurESld),hl
C_NOP:
    ret

SETORN:
    add  A,a
    ld   E,a
    ld   D,#0
    ld   -12+PsInOr(ix),D
    ld   hl,(OrnPtrs)
    add  hl,de
    ld   E,(hl)
    inc  hl
    ld   D,(hl)
    ld   hl,(ModAddr)
    add  hl,de
    ld   -12+OrnPtr(ix),L
    ld   -12+OrnPtr+1(ix),H
    ret

;ALL 16 ADDRESSES TO PROTECT FROM BROKEN PT3 MODULES
SPCCOMS:
    .dw   C_NOP
    .dw   C_GLISS
    .dw   C_PORTM
    .dw   C_SMPOS
    .dw   C_ORPOS
    .dw   C_VIBRT
    .dw   C_NOP
    .dw   C_NOP
    .dw   C_ENGLS
    .dw   C_DELAY
    .dw   C_NOP
    .dw   C_NOP
    .dw   C_NOP
    .dw   C_NOP
    .dw   C_NOP
    .dw   C_NOP

CHREGS:
    xor  A
    ld   (Ampl),a
    bit  0,PtFlags(ix)
    push hl
    jp   Z,CH_EXIT
    ld   (SaveSP),SP
    ld   L,OrnPtr(ix)
    ld   H,OrnPtr+1(ix)
    ld   SP,hl
    pop  de
    ld   H,a
    ld   A,PsInOr(ix)
    ld   L,a
    add  hl,SP
    inc  A
    cp   D
    jr   C,CH_ORPS
    ld   A,E
CH_ORPS:
    ld   PsInOr(ix),a
    ld   A,Note(ix)
    add  A,(hl)
    jp   P,CH_NTP
    xor  A
CH_NTP:
    cp   #96
    jr   C,CH_NOK
    ld   A,#95
CH_NOK:
    add  A,a
    ex   af,af'
    ld   L,SamPtr(ix)
    ld   H,SamPtr+1(ix)
    ld   SP,hl
    pop  de
    ld   H,#0
    ld   A,PsInSm(ix)
    ld   B,a
    add  A,a
    add  A,a
    ld   L,a
    add  hl,SP
    ld   SP,hl
    ld   A,B
    inc  A
    cp   D
    jr   C,CH_SMPS
    ld   A,E
CH_SMPS:
    ld   PsInSm(ix),a
    pop  bc
    pop  hl
    ld   E,TnAcc(ix)
    ld   D,TnAcc+1(ix)
    add  hl,de
    bit  6,B
    jr   Z,CH_NOAC
    ld   TnAcc(ix),L
    ld   TnAcc+1(ix),H
CH_NOAC:
    ex   de,hl
    ex   af,af'
    ld   L,a
    ld   H,#0
    ld   SP,#NT_
    add  hl,SP
    ld   SP,hl
    pop  hl
    add  hl,de
    ld   E,CrTnSl(ix)
    ld   D,CrTnSl+1(ix)
    add  hl,de
    ld   SP,(SaveSP)
    ex   (SP),hl
    xor  A
    or   TSlCnt(ix)
    jr   Z,CH_AMP
    dec  TSlCnt(ix)
    jr   nz,CH_AMP
    ld   A,TnSlDl(ix)
    ld   TSlCnt(ix),a
    ld   L,TSlStp(ix)
    ld   H,TSlStp+1(ix)
    ld   A,H
    add  hl,de
    ld   CrTnSl(ix),L
    ld   CrTnSl+1(ix),H
    bit  2,PtFlags(ix)
    jr   nz,CH_AMP
    ld   E,TnDelt(ix)
    ld   D,TnDelt+1(ix)
    and  A
    jr   Z,CH_STPP
    ex   de,hl
CH_STPP:
    sbc  hl,de
    jp   M,CH_AMP
    ld   A,SlToNt(ix)
    ld   Note(ix),a
    xor  A
    ld   TSlCnt(ix),a
    ld   CrTnSl(ix),a
    ld   CrTnSl+1(ix),a
CH_AMP:
    ld   A,CrAmSl(ix)
    bit  7,C
    jr   Z,CH_NOAM
    bit  6,C
    jr   Z,CH_AMIN
    cp   #15
    jr   Z,CH_NOAM
    inc  A
    jr   CH_SVAM
CH_AMIN:
    cp   #-15
    jr   Z,CH_NOAM
    dec  A
CH_SVAM:
    ld   CrAmSl(ix),a
CH_NOAM:
    ld   L,a
    ld   A,B
    and  #15
    add  A,L
    jp   P,CH_APOS
    xor  A
CH_APOS:
    cp   #16
    jr   C,CH_VOL
    ld   A,#15
CH_VOL:
    or   Volume(ix)
    ld   L,a
    ld   H,#0
    ld   de,#VT_
    add  hl,de
    ld   A,(hl)
CH_ENV:
    bit  0,C
    jr   nz,CH_NOEN
    or   Env_En(ix)
CH_NOEN:
    ld   (Ampl),a
    bit  7,B
    ld   A,C
    jr   Z,NO_ENSL
    rla
    rla
    SRA  A
    SRA  A
    SRA  A
    add  A,CrEnSl(ix) ;SEE COMMENT BELOW
    bit  5,B
    jr   Z,NO_ENAC
    ld   CrEnSl(ix),a
NO_ENAC:
    ld   hl,#AddToEn
    add  A,(hl)    ;BUG IN PT3 - NEED WORD HERE.
                   ;FIX IT IN NEXT Version?
    ld   (hl),a
    jr   CH_MIX
NO_ENSL:
    rra
    add  A,CrNsSl(ix)
    ld   (AddToNs),a
    bit  5,B
    jr   Z,CH_MIX
    ld   CrNsSl(ix),a
CH_MIX:
    ld   A,B
    rra
    and  #0x48
CH_EXIT:
    ld   hl,#AYREGS+Mixer
    or   (hl)
    rrca
    ld   (hl),a
    pop  hl
    xor  A
    or   COnOff(ix)
    ret  Z
    dec  COnOff(ix)
    ret  nz
    xor  PtFlags(ix)
    ld   PtFlags(ix),a
    rra
    ld   A,OnOffD(ix)
    jr   C,CH_ONDL
    ld   A,OffOnD(ix)
CH_ONDL:
    ld   COnOff(ix),a
    ret

PLAY:
    xor   A
    ld   (AddToEn),a
    ld   (AYREGS+Mixer),a
    dec   A
    ld   (AYREGS+EnvTp),a
    ld   hl,#DelyCnt
    dec  (hl)
    jp   nz,PL2
    ld   hl,#ChanA+NtSkCn
    dec  (hl)
    jr   nz,PL1B
    ld   bc,(AdInPtA)
    ld   A,(bc)
    and  A
    jr   nz,PL1A
    ld   D,a
    ld   (Ns_Base),a
    ld   hl,(CrPsPtr)
    inc  hl
    ld   A,(hl)
    inc  A
    jr   nz,PLNLP
    call CHECKLP
    ld   hl,(LPosPtr)
    ld   A,(hl)
    inc  A
PLNLP:
    ld   (CrPsPtr),hl
    dec  A
    add  A,a
    ld   E,a
    rl   D
    ld   hl,(PatsPtr)
    add  hl,de
    ld   de,(ModAddr)
    ld   (SaveSP),SP
    ld   SP,hl
    pop  hl
    add  hl,de
    ld   B,H
    ld   C,L
    pop  hl
    add  hl,de
    ld   (AdInPtB),hl
    pop  hl
    add  hl,de
    ld   (AdInPtC),hl
    ld   SP,(SaveSP)
PL1A:
    ld   ix,#ChanA+12
    call PTDECOD
    ld   (AdInPtA),bc

PL1B:
    ld   hl,#ChanB+NtSkCn
    dec  (hl)
    jr   nz,PL1C
    ld   ix,#ChanB+12
    ld   bc,(AdInPtB)
    call PTDECOD
    ld   (AdInPtB),bc
PL1C:
    ld   hl,#ChanC+NtSkCn
    dec  (hl)
    jr   nz,PL1D
    ld   ix,#ChanC+12
    ld   bc,(AdInPtC)
    call PTDECOD
    ld   (AdInPtC),bc

PL1D:
    ld   A,(Delay)
    ld   (DelyCnt),a

PL2:
    ld   ix,#ChanA
    ld   hl,(AYREGS+TonA)
    call CHREGS
    ld   (AYREGS+TonA),hl
    ld   A,(Ampl)
    ld   (AYREGS+AmplA),a
    ld   ix,#ChanB
    ld   hl,(AYREGS+TonB)
    call CHREGS
    ld   (AYREGS+TonB),hl
    ld   A,(Ampl)
    ld   (AYREGS+AmplB),a
    ld   ix,#ChanC
    ld   hl,(AYREGS+TonC)
    call CHREGS
    ld   (AYREGS+TonC),hl
    ld   hl,(Ns_Base_AddToNs)
    ld   A,H
    add  A,L
    ld   (AYREGS+Noise),a
    ld   A,(AddToEn)
    ld   E,a
    add  A,a
    sbc  A,a
    ld   D,a
    ld   hl,(EnvBase)
    add  hl,de
    ld   de,(CurESld)
    add  hl,de
    ld   (AYREGS+Env),hl
    xor  A
    ld   hl,#CurEDel
    or   (hl)
    jr   Z,ROUT_A0
    dec (hl)
    jr   nz,ROUT
    ld   A,(Env_Del)
    ld   (hl),a
    ld   hl,(ESldAdd)
    add  hl,de
    ld   (CurESld),hl
ROUT:
    xor  a           ; A  = register 0
ROUT_A0:
    ld   hl,#AYREGS  ; hl = register data
    ld   c,#0xF6     ; C  = AY data port
LOUT:
    out  (0xF7),a    ; select register
    inc  a           ; A = next register
    outi             ; (hl) -> AY register, inc hl
    cp   #13         ; loaded registers 0~12?
    jr   nz,LOUT
    out  (0xF7),a    ; select register 13
    ld   a,(hl)      ; get register 13 data
    and  a
    ret  m           ; return if bit 7 = 1
    out  (c),a       ; load register 13
    ret

;Stupid ALASM limitations
NT_DATA:
    .db (T_NEW_0-T1_)*2 ; 50*2
    .db TCNEW_0-T_
    .db 50*2+1          ;(T_OLD_0-T1_)*2+1
    .db TCOLD_0-T_
    .db 0*2+1           ;(T_NEW_1-T1_)*2+1
    .db TCNEW_1-T_
    .db 0*2+1           ;(T_OLD_1-T1_)*2+1
    .db TCOLD_1-T_
    .db 74*2            ;(T_NEW_2-T1_)*2
    .db TCNEW_2-T_
    .db 24*2            ;(T_OLD_2-T1_)*2
    .db TCOLD_2-T_
    .db 48*2            ;(T_NEW_3-T1_)*2
    .db TCNEW_3-T_
    .db 48*2            ;(T_OLD_3-T1_)*2
    .db TCOLD_3-T_

T_:

TCOLD_0:
    .db 0x00+1,0x04+1,0x08+1,0x0A+1,0x0C+1,0x0E+1,0x12+1,0x14+1
    .db 0x18+1,0x24+1,0x3C+1,0
TCOLD_1:
TCNEW_1:
    .db 0x5C+1,0
TCOLD_2:
    .db 0x30+1,0x36+1,0x4C+1,0x52+1,0x5E+1,0x70+1,0x82,0x8C,0x9C
    .db 0x9E,0xA0,0xA6,0xA8,0xAA,0xAC,0xAE,0xAE,0
TCNEW_3:
    .db 0x56+1
TCOLD_3:
    .db 0x1E+1,0x22+1,0x24+1,0x28+1,0x2C+1,0x2E+1,0x32+1,0xBE+1,0
TCNEW_0:
    .db 0x1C+1,0x20+1,0x22+1,0x26+1,0x2A+1,0x2C+1,0x30+1,0x54+1
    .db 0xBC+1,0xBE+1,0
TCNEW_2:
    .db 0x1A+1,0x20+1,0x24+1,0x28+1,0x2A+1,0x3A+1,0x4C+1,0x5E+1
    .db 0xBA+1,0xBC+1,0xBE+1
EMPTYSAMORN:
    .db 0
    .db 1,0,0x90 ;delete 0x90 if you don't need default sample

;first 12 values of tone tables (packed)

T_PACK:
    .db >(0x06EC*2),<(0x06EC*2)
    .db 0x0755-0x06EC
    .db 0x07C5-0x0755
    .db 0x083B-0x07C5
    .db 0x08B8-0x083B
    .db 0x093D-0x08B8
    .db 0x09CA-0x093D
    .db 0x0A5F-0x09CA
    .db 0x0AFC-0x0A5F
    .db 0x0BA4-0x0AFC
    .db 0x0C55-0x0BA4
    .db 0x0D10-0x0C55
    .db >(0x066D*2),<(0x066D*2)
    .db 0x06CF-0x066D
    .db 0x0737-0x06CF
    .db 0x07A4-0x0737
    .db 0x0819-0x07A4
    .db 0x0894-0x0819
    .db 0x0917-0x0894
    .db 0x09A1-0x0917
    .db 0x0A33-0x09A1
    .db 0x0ACF-0x0A33
    .db 0x0B73-0x0ACF
    .db 0x0C22-0x0B73
    .db 0x0CDA-0x0C22
    .db >(0x0704*2),<(0x0704*2)
    .db 0x076E-0x0704
    .db 0x07E0-0x076E
    .db 0x0858-0x07E0
    .db 0x08D6-0x0858
    .db 0x095C-0x08D6
    .db 0x09EC-0x095C
    .db 0x0A82-0x09EC
    .db 0x0B22-0x0A82
    .db 0x0BCC-0x0B22
    .db 0x0C80-0x0BCC
    .db 0x0D3E-0x0C80
    .db >(0x07E0*2),<(0x07E0*2)
    .db 0x0858-0x07E0
    .db 0x08E0-0x0858
    .db 0x0960-0x08E0
    .db 0x09F0-0x0960
    .db 0x0A88-0x09F0
    .db 0x0B28-0x0A88
    .db 0x0BD8-0x0B28
    .db 0x0C80-0x0BD8
    .db 0x0D60-0x0C80
    .db 0x0E10-0x0D60
    .db 0x0EF8-0x0E10
