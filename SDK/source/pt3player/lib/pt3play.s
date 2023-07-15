    .z80

; 60Hz = -1  ; make this -1 to play 50Hz songs on 60Hz machine

; address of variables in RAM
; VARMEM = 0xB800  ; top of stack at boot (before initializing BASIC)

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

VARMEM:
SETUP:     .db 0     ; bit7 = 1 when loop point reached
CrPsPtr:   .dw 0
AddToEn:   .db 0
AdInPtA:   .dw 0
AdInPtB:   .dw 0
AdInPtC:   .dw 0
Env_Del:   .db 0
MODADDR:   .dw 0
ESldAdd:   .dw 0
Delay:     .db 0
SaveSP:    .dw 0
SamPtrs:   .dw 0
OrnPtrs:   .dw 0
PatsPtr:   .dw 0
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

_pt3play_ayregs::
VT_:       .ds 256     ; 256 bytes CreatedVolumeTableAddress
NT_:       .ds 192     ; 192 bytes Note Table
VAREND:


Ns_Base_AddToNs = Ns_Base

L3       = PrSlide    ; opcode + RET
M2       = PrSlide

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
; void pt3play_play(void);
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
    LD   HL,#SETUP
    SET  7,(HL)
    BIT  0,(HL)
    RET  Z
    POP  HL
    LD   HL,#DelyCnt
    INC  (HL)
    LD   HL,#ChanA+NtSkCn
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
    LD   DE,#100
    ADD  HL,DE
    LD   A,(HL)
    LD   (Delay),A
    PUSH HL
    POP  IX
    ADD  HL,DE
    LD   (CrPsPtr),HL
    LD   E,102-100(ix)
    ADD  HL,DE
    INC  HL
    LD   (LPosPtr),HL
    POP  DE
    LD   L,103-100(ix)
    LD   H,104-100(ix)
    ADD  HL,DE
    LD   (PatsPtr),HL
    LD   HL,#169
    ADD  HL,DE
    LD   (OrnPtrs),HL
    LD   HL,#105
    ADD  HL,DE
    LD   (SamPtrs),HL
    LD   HL,#SETUP
    RES  7,(HL)

    ;note table data depacker
    LD   DE,#T_PACK
    LD   BC,#T1_+(2*49)-1
TP_0:
    LD   A,(DE)
    INC  DE
    CP   #15*2
    JR   NC,TP_1
    LD   H,A
    LD   A,(DE)
    LD   L,A
    INC  DE
    JR   TP_2
TP_1:
    PUSH DE
    LD   D,#0
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
    SUB  #<(0xF8*2)
    JR   NZ,TP_0

    LD   HL,#ChanA
    LD   (HL),A                   ; clear first variable byte
    LD   D,H
    LD   E,L
    INC  DE
    LD   BC,#(VAR0END-ChanA-1)
    LDIR                          ; clear all other variable bytes
    INC  A
    LD   (DelyCnt),A
    LD   HL,#0xF001 ;H - Volume, L - NtSkCn
    LD   (ChanA+NtSkCn),HL
    LD   (ChanB+NtSkCn),HL
    LD   (ChanC+NtSkCn),HL

    LD   HL,#EMPTYSAMORN
    LD   (AdInPtA),HL ;ptr to zero
    LD   (ChanA+OrnPtr),HL ;ornament 0 is "0,1,0"
    LD   (ChanB+OrnPtr),HL ;in all Versions from
    LD   (ChanC+OrnPtr),HL ;3.xx to 3.6x and VTII

    LD   (ChanA+SamPtr),HL ;S1 There is no default
    LD   (ChanB+SamPtr),HL ;S2 sample in PT3, so, you
    LD   (ChanC+SamPtr),HL ;S3 can comment S1,2,3; see
                    ;also EMPTYSAMORN comment
    LD   A,13-100(ix) ;EXTRACT Version NUMBER
    SUB  #0x30
    JR   C,L20
    CP   #10
    JR   C,L21
L20:
    LD   A,#6
L21:
    LD   (PtVersion),A
    PUSH AF
    CP   #4
    LD   A,99-100(ix) ;TONE TABLE NUMBER
    RLA
    AND  #7
;NoteTableCreator (c) Ivan Roshin
;A - NoteTableNumber*2+VersionForNoteTable
;(xx1b - 3.xx..3.4r, xx0b - 3.4x..3.6x..VTII1.0)
    LD   HL,#NT_DATA
    PUSH DE
    LD   D,B
    ADD  A,A
    LD   E,A
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    SRL  E
    SBC  A,A
    AND  #0xA7      ; 0x00 (NOP) or 0xA7 (AND A)
    LD   (L3),A
    LD   A,#0xC9    ; RET temporary
    LD   (L3+1),A ; temporary
    EX   DE,HL
    POP  BC       ; BC=T1_
    ADD  HL,BC

    LD   A,(DE)

    ADD  A,#<(T_)
    LD   C,A
    ADC  A,#>(T_)

    SUB  C
    LD   B,A
    PUSH BC
    LD   DE,#NT_
    PUSH DE

    LD   B,#12
L1: PUSH BC
    LD   C,(HL)
    INC  HL
    PUSH HL
    LD   B,(HL)

    PUSH DE
    EX   DE,HL
    LD   DE,#23


    .db 0xDD,0x26,0x08; LD   IXH,#8

L2: SRL  B
    RR   C
    CALL L3     ;temporary
;L3: DB    0x19   ;AND A or NOP
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
    CP   #<(TCOLD_1)
    JR   NZ,CORR_1
    LD   A,#0xFD
    LD   (NT_+0x2E),A

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
    CP   #5
    LD   HL,#0x11
    LD   D,H
    LD   E,H
    LD   A,#0x17
    JR   NC,M1
    DEC  L
    LD   E,L
    XOR  A
M1:
    LD   (M2),A      ; 0x17 or 0x00

    LD   IX,#VT_+16
    LD   C,#0x10

INITV2:
    PUSH HL
    ADD  HL,DE
    EX   DE,HL
    SBC  HL,HL
INITV1:
    LD   A,L
;M2      DB 0x7D
    CALL M2 ;temporary
    LD   A,H
    ADC  A,#0
    LD   (IX),A
    INC  IX
    ADD  HL,DE
    INC  C
    LD   A,C
    AND  #15
    JR   NZ,INITV1

    POP  HL
    LD   A,E
    CP   #0x77
    JR   NZ,M3
    INC  E
M3:
    LD   A,C
    AND  A
    JR   NZ,INITV2
    JP   ROUT_A0

;pattern decoder
PD_OrSm:
    LD   -12+Env_En(IX),#0
    CALL SETORN
    LD   A,(BC)
    INC  BC
    RRCA
PD_SAM:
    ADD  A,A
PD_SAM_:
    LD   E,A
    LD   D,#0
    LD   HL,(SamPtrs)
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    LD   HL,(MODADDR)
    ADD  HL,DE
    LD   -12+SamPtr(IX),L
    LD   -12+SamPtr+1(IX),H
    JR   PD_LOOP

PD_VOL:
    RLCA
    RLCA
    RLCA
    RLCA
    LD   -12+Volume(IX),A
    JR   PD_LP2

PD_EOff:
    LD  -12+Env_En(IX),A
    LD  -12+PsInOr(IX),A
    JR   PD_LP2

PD_SorE:
    DEC  A
    JR   NZ,PD_ENV
    LD   A,(BC)
    INC  BC
    LD   -12+NNtSkp(IX),A
    JR   PD_LP2

PD_ENV:
    CALL SETENV
    JR   PD_LP2

PD_ORN:
    CALL SETORN
    JR   PD_LOOP

PD_ESAM:
    LD   -12+Env_En(IX),A
    LD   -12+PsInOr(IX),A
    CALL NZ,SETENV
    LD   A,(BC)
    INC  BC
    JR   PD_SAM_

PTDECOD:
    LD   A,-12+Note(IX)
    LD   (PrNote),A
    LD   L,-12+CrTnSl(IX)
    LD   H,-12+CrTnSl+1(IX)
    LD   (PrSlide),HL
PD_LOOP:
    LD   DE,#0x2010
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
    ADD  A,#96
    JR   C,PD_NOTE
    ADD  A,E
    JR   C,PD_ORN
    ADD  A,D
    JR   C,PD_NOIS
    ADD  A,E
    JR   C,PD_ESAM
    ADD  A,A
    LD   E,A
    LD   HL,#SPCCOMS-0x20E0 ; +0xFF20-0x2000
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
    RES  0,-12+PtFlags(IX)
    JR   PD_RES

PD_NOTE:
    LD   -12+Note(IX),A
    SET  0,-12+PtFlags(IX)
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
    LD   A,-12+NNtSkp(IX)
    LD   -12+NtSkCn(IX),A
    RET

C_PORTM:
    RES  2,-12+PtFlags(IX)
    LD   A,(BC)
    INC  BC
;SKIP PRECALCULATED TONE DELTA (BECAUSE
;CANNOT BE RIGHT AFTER PT3 COMPILATION)
    INC  BC
    INC  BC
    LD   -12+TnSlDl(IX),A
    LD   -12+TSlCnt(IX),A
    LD   DE,#NT_
    LD   A,-12+Note(IX)
    LD   -12+SlToNt(IX),A
    ADD  A,A
    LD   L,A
    LD   H,#0
    ADD  HL,DE
    LD   A,(HL)
    INC  HL
    LD   H,(HL)
    LD   L,A
    PUSH HL
    LD   A,(PrNote)
    LD   -12+Note(IX),A
    ADD  A,A
    LD   L,A
    LD   H,#0
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    POP  HL
    SBC  HL,DE
    LD   -12+TnDelt(IX),L
    LD   -12+TnDelt+1(IX),H
    LD   E,-12+CrTnSl(IX)
    LD   D,-12+CrTnSl+1(IX)
    LD   A,(PtVersion)
    CP   #6
    JR   C,OLDPRTM ;Old 3xxx for PT v3.5-
    LD   DE,(PrSlide)
    LD   -12+CrTnSl(IX),E
    LD   -12+CrTnSl+1(IX),D
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
    LD   -12+TSlStp+1(IX),A
    EX   AF,AF'
    LD   -12+TSlStp(IX),A
    LD   -12+COnOff(IX),#0
    RET

C_GLISS:
    SET  2,-12+PtFlags(IX)
    LD   A,(BC)
    INC  BC
    LD   -12+TnSlDl(IX),A
    AND  A
    JR   NZ,GL36
    LD   A,(PtVersion) ;AlCo PT3.7+
    CP   #7
    SBC  A,A
    INC  A
GL36:
    LD   -12+TSlCnt(IX),A
    LD   A,(BC)
    INC  BC
    EX   AF,AF'
    LD   A,(BC)
    INC  BC
    JR   SET_STP

C_SMPOS:
    LD   A,(BC)
    INC  BC
    LD   -12+PsInSm(IX),A
    RET

C_ORPOS:
    LD   A,(BC)
    INC  BC
    LD   -12+PsInOr(IX),A
    RET

C_VIBRT:
    LD   A,(BC)
    INC  BC
    LD   -12+OnOffD(IX),A
    LD   -12+COnOff(IX),A
    LD   A,(BC)
    INC  BC
    LD   -12+OffOnD(IX),A
    XOR  A
    LD   -12+TSlCnt(IX),A
    LD   -12+CrTnSl(IX),A
    LD   -12+CrTnSl+1(IX),A
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
    LD   -12+Env_En(IX),E
    LD   (AYREGS+EnvTp),A
    LD   A,(BC)
    INC  BC
    LD   H,A
    LD   A,(BC)
    INC  BC
    LD   L,A
    LD   (EnvBase),HL
    XOR  A
    LD   -12+PsInOr(IX),A
    LD   (CurEDel),A
    LD   H,A
    LD   L,A
    LD   (CurESld),HL
C_NOP:
    RET

SETORN:
    ADD  A,A
    LD   E,A
    LD   D,#0
    LD   -12+PsInOr(IX),D
    LD   HL,(OrnPtrs)
    ADD  HL,DE
    LD   E,(HL)
    INC  HL
    LD   D,(HL)
    LD   HL,(MODADDR)
    ADD  HL,DE
    LD   -12+OrnPtr(IX),L
    LD   -12+OrnPtr+1(IX),H
    RET

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
    XOR  A
    LD   (Ampl),A
    BIT  0,PtFlags(IX)
    PUSH HL
    JP   Z,CH_EXIT
    LD   (SaveSP),SP
    LD   L,OrnPtr(IX)
    LD   H,OrnPtr+1(IX)
    LD   SP,HL
    POP  DE
    LD   H,A
    LD   A,PsInOr(IX)
    LD   L,A
    ADD  HL,SP
    INC  A
    CP   D
    JR   C,CH_ORPS
    LD   A,E
CH_ORPS:
    LD   PsInOr(IX),A
    LD   A,Note(IX)
    ADD  A,(HL)
    JP   P,CH_NTP
    XOR  A
CH_NTP:
    CP   #96
    JR   C,CH_NOK
    LD   A,#95
CH_NOK:
    ADD  A,A
    EX   AF,AF'
    LD   L,SamPtr(IX)
    LD   H,SamPtr+1(IX)
    LD   SP,HL
    POP  DE
    LD   H,#0
    LD   A,PsInSm(IX)
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
    LD   PsInSm(IX),A
    POP  BC
    POP  HL
    LD   E,TnAcc(IX)
    LD   D,TnAcc+1(IX)
    ADD  HL,DE
    BIT  6,B
    JR   Z,CH_NOAC
    LD   TnAcc(IX),L
    LD   TnAcc+1(IX),H
CH_NOAC:
    EX   DE,HL
    EX   AF,AF'
    LD   L,A
    LD   H,#0
    LD   SP,#NT_
    ADD  HL,SP
    LD   SP,HL
    POP  HL
    ADD  HL,DE
    LD   E,CrTnSl(IX)
    LD   D,CrTnSl+1(IX)
    ADD  HL,DE
    LD   SP,(SaveSP)
    EX   (SP),HL
    XOR  A
    OR   TSlCnt(IX)
    JR   Z,CH_AMP
    DEC  TSlCnt(IX)
    JR   NZ,CH_AMP
    LD   A,TnSlDl(IX)
    LD   TSlCnt(IX),A
    LD   L,TSlStp(IX)
    LD   H,TSlStp+1(IX)
    LD   A,H
    ADD  HL,DE
    LD   CrTnSl(IX),L
    LD   CrTnSl+1(IX),H
    BIT  2,PtFlags(IX)
    JR   NZ,CH_AMP
    LD   E,TnDelt(IX)
    LD   D,TnDelt+1(IX)
    AND  A
    JR   Z,CH_STPP
    EX   DE,HL
CH_STPP:
    SBC  HL,DE
    JP   M,CH_AMP
    LD   A,SlToNt(IX)
    LD   Note(IX),A
    XOR  A
    LD   TSlCnt(IX),A
    LD   CrTnSl(IX),A
    LD   CrTnSl+1(IX),A
CH_AMP:
    LD   A,CrAmSl(IX)
    BIT  7,C
    JR   Z,CH_NOAM
    BIT  6,C
    JR   Z,CH_AMIN
    CP   #15
    JR   Z,CH_NOAM
    INC  A
    JR   CH_SVAM
CH_AMIN:
    CP   #-15
    JR   Z,CH_NOAM
    DEC  A
CH_SVAM:
    LD   CrAmSl(IX),A
CH_NOAM:
    LD   L,A
    LD   A,B
    AND  #15
    ADD  A,L
    JP   P,CH_APOS
    XOR  A
CH_APOS:
    CP   #16
    JR   C,CH_VOL
    LD   A,#15
CH_VOL:
    OR   Volume(IX)
    LD   L,A
    LD   H,#0
    LD   DE,#VT_
    ADD  HL,DE
    LD   A,(HL)
CH_ENV:
    BIT  0,C
    JR   NZ,CH_NOEN
    OR   Env_En(IX)
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
    ADD  A,CrEnSl(IX) ;SEE COMMENT BELOW
    BIT  5,B
    JR   Z,NO_ENAC
    LD   CrEnSl(IX),A
NO_ENAC:
    LD   HL,#AddToEn
    ADD  A,(HL)    ;BUG IN PT3 - NEED WORD HERE.
                   ;FIX IT IN NEXT Version?
    LD   (HL),A
    JR   CH_MIX
NO_ENSL:
    RRA
    ADD  A,CrNsSl(IX)
    LD   (AddToNs),A
    BIT  5,B
    JR   Z,CH_MIX
    LD   CrNsSl(IX),A
CH_MIX:
    LD   A,B
    RRA
    AND  #0x48
CH_EXIT:
    LD   HL,#AYREGS+Mixer
    OR   (HL)
    RRCA
    LD   (HL),A
    POP  HL
    XOR  A
    OR   COnOff(IX)
    RET  Z
    DEC  COnOff(IX)
    RET  NZ
    XOR  PtFlags(IX)
    LD   PtFlags(IX),A
    RRA
    LD   A,OnOffD(IX)
    JR   C,CH_ONDL
    LD   A,OffOnD(IX)
CH_ONDL:
    LD   COnOff(IX),A
    RET

PLAY:
    XOR   A
    LD   (AddToEn),A
    LD   (AYREGS+Mixer),A
    DEC   A
    LD   (AYREGS+EnvTp),A
    LD   HL,#DelyCnt
    DEC  (HL)
    JP   NZ,PL2
    LD   HL,#ChanA+NtSkCn
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
    LD   IX,#ChanA+12
    CALL PTDECOD
    LD   (AdInPtA),BC

PL1B:
    LD   HL,#ChanB+NtSkCn
    DEC  (HL)
    JR   NZ,PL1C
    LD   IX,#ChanB+12
    LD   BC,(AdInPtB)
    CALL PTDECOD
    LD   (AdInPtB),BC
PL1C:
    LD   HL,#ChanC+NtSkCn
    DEC  (HL)
    JR   NZ,PL1D
    LD   IX,#ChanC+12
    LD   BC,(AdInPtC)
    CALL PTDECOD
    LD   (AdInPtC),BC

PL1D:
    LD   A,(Delay)
    LD   (DelyCnt),A

PL2:
    LD   IX,#ChanA
    LD   HL,(AYREGS+TonA)
    CALL CHREGS
    LD   (AYREGS+TonA),HL
    LD   A,(Ampl)
    LD   (AYREGS+AmplA),A
    LD   IX,#ChanB
    LD   HL,(AYREGS+TonB)
    CALL CHREGS
    LD   (AYREGS+TonB),HL
    LD   A,(Ampl)
    LD   (AYREGS+AmplB),A
    LD   IX,#ChanC
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
    LD   HL,#CurEDel
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
    LD   HL,#AYREGS   ; HL = register data
    LD   C,#0xF6       ; C  = AY data port
LOUT:
    OUT  (0xF7),A     ; select register
    INC  A           ; A = next register
    OUTI             ; (HL) -> AY register, inc HL
    CP   #13          ; loaded registers 0~12?
    JR   NZ,LOUT
    OUT  (0xF7),A     ; select register 13
    LD   A,(HL)      ; get register 13 data
    AND  A
    RET  M           ; return if bit 7 = 1
    OUT  (C),A       ; load register 13
    RET

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
