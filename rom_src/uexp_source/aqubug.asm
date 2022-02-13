;=============================================================
;             Aquarius Z80 Monitor/Debugger
;=============================================================
;
; 2017-01-15 V0.00 created from 'CHAMP' assembler/debugger
; 2017-01-30 V0.01 breakpoints use RST $38 (USRJMP)
; 2017-02-01 V0.02 chars <$7E = '.'
;                  fixed corruption in high opcode syntax table
; 2017-02-08 V0.03 trace in separate screen
;                  implementing windows
; 2017-02-20 V0.04 displaymem, fillmem, movemem
; 2017-02-24 V0.05 Put into ROM with BABASIC
; 2017-02-25 V0.06 file load/save
; 2017-03-17 V0.07 Preceed, Go.
; 2017-03-23 V0.08 bugfix: align screen load/save to peek/poke
; 2017-04-06 V0.09 save/restore BASIC text pointer via HL_reg, not stack
; 2017-04-11 V0.10 Search: quit if nothing to search for.
;                  remember settings from previous session
; 2017-04-17 V0.11 combine search buffer with line input buffer
; 2017-04-30 V0.12 key click
; 2017-05-01 V0.13 LOAD shows 'no files' if no files in directory
; 2017-05-07 V0.14 Load and Write show number of bytes read/written.
;                  No window refresh when unallocated key pressed.
;                  Trace: 4 memory windows, selected with key '1'-'4'
;                  Editbreak restarts after entering breakpoint
; 2017-05-11 V0.15 General Help screen.
;                  Registers: don't abort if invalid register entered
; 2017-05-12 V0.16 bugfix: incorrect offset in screen buffer peek/poke
;                  memory window address - + with '-' and '=' keys
; 2017-05-13 V0.17 Memory Window number AND 3 (in case it becomes corrupted)
; 2017-05-17 V0.18 calculating space left to print filname in wincatdisk
; 2017-05-20 V0.19 search string entry defaults to lower case.
;                  line input toggles CAPS lock off/on with ".
;                  bugfix: showbreakpoints showing wrong addresses.
;                  M1-4 - + refreshes itself only, not entire Trace window.
; 2017-06-02 V0.20 CAPSLOCK flag
; 2017-06-12 V1.0  bumped to release version

;  include "macros.i"
;  include "aquarius.i"

NUMBRKS    = 8        ; number of breakpoints
DUMPLINES  = 22       ; lines to show in DumpMem
UNASMLINES = 22       ; lines to show in unassemble
LINESIZE   = 37       ; length of line input buffer

; breakpoint array element
    STRUCTURE brk,0
    BYTE  brk.status  ; 1 = active, 0 = inactive
    WORD  brk.addr    ; address of target code
    BYTE  brk.opcode  ; original opcode at target address
    ENDSTRUCT brk

; bits in Flags
QUITKEY   = 0         ; option: line input 'Q'->^C
HELPKEY   = 1         ; option: line input '?' = help key
CAPSLOCK  = 2         ; option: force to uppercase (unless entering string)
STRING    = 3         ;   flag: entering mixed case string (toggled with ")

;--------------------------------------------------------------------
; offsets to variables in our private RAM (for use with (IY+n))
;
     STRUCTURE v,0   ; starting at IY+0
     BYTE _flags     ; boolean flags
     BYTE StartPos   ; cursor position at start of args
     BYTE Opcode     ; opcode
     BYTE Prefix     ; opcode prefix CB/DD/ED/FD or "X"/"Y"
     BYTE OpLen      ; opcode length
     BYTE OpAttr     ; opcode attributes
     BYTE Offset     ; opcode offset
     BYTE KeyCode    ; last key pressed
     BYTE Command    ; command letter
     BYTE Digits     ; number of digits in number
     BYTE MemWindow  ; trace memory window #
   STRUCT _MemWindows,2*4 ; memory window addresses
     WORD _number    ; 16 bit number
     WORD _BrkAddr   ; address of current breakpoint
     WORD _UserAddr  ; original USRADDR
     WORD _CodeAddr  ; opcode address
     WORD _DumpAddr  ; DumpMem address
     WORD _UnAsmAddr ; UnAssemble Address
     WORD _EditAddr  ; edit memory address
     WORD _SrchAddr  ; search start address
     WORD _AF_alt    ; AF'
     WORD _BC_alt    ; BC'
     WORD _DE_alt    ; DE'
     WORD _HL_alt    ; HL'
     WORD _AF_reg    ; AF
     WORD _BC_reg    ; BC
     WORD _DE_reg    ; DE
     WORD _HL_reg    ; HL
     WORD _IX_reg    ; IX
     WORD _IY_reg    ; IY
     WORD _SP_reg    ; SP
     WORD _PC_reg    ; PC
     WORD _SP_SAVE   ; saved system stack pointer
   STRUCT _TraceOp,4 ; 'sandbox' for instruction being traced
     BYTE _TraceBrk  ; RST $38 at end of sandbox
   STRUCT _Breakpoints,brk.size*NUMBRKS ;breakpoint array
   STRUCT _scrn_save,$40*24*2  ; buffer to hold systen screen
   STRUCT _LineBuffer,LINESIZE ; user input line buffer
   STRUCT _dbg_stack,64        ; our private stack
   ENDSTRUCT v

;-------------------------------------------------------------
;               absolute RAM addresses
;
vars = sysvars-v.size           ; in private RAM below sysvars

MemWindows  = vars+_memwindows
Number      = vars+_number
BrkAddr     = vars+_BrkAddr
UserAddr    = vars+_UserAddr
CodeAddr    = vars+_CodeAddr
DumpAddr    = vars+_DumpAddr
UnAsmAddr   = vars+_UnAsmAddr
EditAddr    = vars+_EditAddr
SrchAddr    = vars+_SrchAddr

ALT_regs    = vars+_AF_alt
AF_REG      = vars+_AF_reg
BC_Reg      = vars+_bc_reg
HL_Reg      = vars+_hl_reg
IX_reg      = vars+_IX_reg
IY_Reg      = vars+_IY_reg
SP_reg      = vars+_SP_reg
PC_reg      = vars+_PC_Reg
SP_SAVE     = vars+_sp_save
TraceOp     = vars+_TraceOp
TraceBrk    = vars+_TraceBrk
Breakpoints = vars+_Breakpoints
scrn_save   = vars+_scrn_save
LineBuffer  = vars+_linebuffer  ; line input buffer
SearchStr   = LineBuffer        ; using line input buffer
stack_top   = vars+v.size       ; stack grows downwards from here

;----------------------------------------------------------
;                      - Debug -
;
ST_DEBUG:
       LD    A,6                ; cancel pending key down
       LD    (SCANCNT),A
       LD    (HL_reg),HL        ; save BASIC text pointer in HL_reg
       LD    (SP_SAVE),SP       ; save system stack pointer
       LD    (SP_reg),SP        ; copy to SP_reg
       LD    SP,stack_top       ; SP = our private stack
       call  save_screen        ; save system screen
       CALL  ClearBreaks        ; clear all breakpoints
       LD    HL,(USRADDR)
       LD    (UserAddr),HL      ; save usr address
DebugMenu:
       LD    SP,stack_top       ; clear our private stack
       LD    IY,Vars            ; IY = our variables
       LD    IX,DebugWindow
       CALL  OpenWindow         ; open debug window
       LD    HL,debug_msg
       CALL  WinPrtStr          ; show commands
.waitkey:
       CALL  Wait_Key           ; wait for key
       LD    (IY+keycode),A
       CP    "t"                ; "T" = trace
       JR    Z,StartTrace
       CP    "q"                ; "Q" = quit
       JR    Z,Quit
       LD    HL,debug_cmds
       LD    B,(debug_cmd_end-debug_cmds)/3
       CALL  DoCommand          ; process other keys
       JR    C,DebugMenu        ; refrsh window if required
       JR    .waitkey           ; else just wait for key

; exit to caller
Quit:
       CALL  restore_screen     ; restore system screen
       LD    HL,(HL_reg)        ; update BASIC text pointer
       LD    SP,(SP_SAVE)       ; restore system stack pointer
       RET                      ; return to system

debug_cmds:
       db "b"
       dw EditBreak
       db "r"
       dw Registers
       db "s"
       dw SearchMem
       db "l"
       dw LoadFile
       db "w"
       dw WriteFile
       db "f"
       dw FillMem
       db "e"
       dw EditMem
       db "m"
       dw MoveMem
       db "u"
       dw Unassemble
       db "d"
       dw DumpMem
       db "p"
       dw Proceed
       db "g"
       dw Go
       db "?"
       dw DebugHelp
       db " "
       dw ToggleScreen
debug_cmd_end:

;-------------------------------------------------------
;             Find Command and Jump to it
;-------------------------------------------------------
;  in: A = command char
; out: c = window refresh required
;
DoCommand:
       CP    (HL)              ; our key?
       INC   HL
       JR    Z,got_command
       INC   HL                ; next entry
       INC   HL
       DJNZ  DoCommand
       CP    A
       RET                     ; nc if key not found
got_command:
       LD    (IY+_flags),(1<<QUITKEY)|(1<<HELPKEY)|(1<<CAPSLOCK)
       LD    E,(HL)
       INC   HL
       LD    D,(HL)            ; DE = command address
       LD    HL,.done
       PUSH  HL                ; return to .done
       EX    DE,HL
       JP    (HL)              ; call command
.done:
       SCF                     ; c = window refresh required
       RET

;---------------------------------------------------------
;              Debug Help Screen
;---------------------------------------------------------
DebugHelp:
       LD    IX,DebugHelpWindow
       CALL  OpenWindow
       LD    HL,debug_help_msg
       CALL  WinPrtStr
       JP    Wait_Key

;---------------------------------------------------------
;                     T - Trace
;---------------------------------------------------------
StartTrace:
       CALL  InputPC            ; input code address
       JR    NZ,TraceAddr
       CP    $03                ; if ^C typed then back to main menu
       JP    Z,DebugMenu
       JR    Trace              ; else use previous address
TraceAddr:
       LD    (PC_reg),HL        ; set code address
Trace:
       LD    IX,TraceWindow
       CALL  OpenWindow         ; open/refresh debug window
       JR    TraceShow          ; start tracing code
ToggleFlags:
       LD    HL,AF_Reg
       LD    A,B
       XOR   (HL)               ; toggle flag bit
       LD    (HL),A
TraceShow:
       LD    DE,0               ; register x,y
       CALL  WinSetCursor
       CALL  ShowRegs           ; show registers
       CALL  NewLine
       CALL  ShowCode           ; show code disassembly
Trace_Mem:
       LD    DE,14              ; x,y = 0,14
       CALL  WinSetCursor       ; set cursor postion for memory window display
       LD    A,'M'
       CALL  WinPrtChr
       LD    A,(IY+memWindow)
       ADD   '1'                ; show memory window #
       CALL  WinPrtChr
       CALL  NewLine
       CALL  GetMemWindow       ; DE = address of current memory window
       EX    DE,HL
       LD    B,7                ; number of rows
       LD    C,8                ; bytes per row
       CALL  DisplayMem         ; display memory block in hex/ascii
_tr_wait_key:
       LD    (IY+_flags),(1<<QUITKEY)|(1<<HELPKEY)|(1<<CAPSLOCK)
       CALL  WAIT_KEY           ; get next keystroke
       LD    (IY+KeyCode),A
       CP    $0D                ; <RTN> = trace over CALL
       JP    Z,_trace_next
       CP    "."                ; "." = trace into CALL
       JP    Z,_trace_next
       CP    ","                ; "," = skip instruction
       JP    Z,_skip
       CP    ";"                ; ";" = trace out of CALL
       JP    Z,_trace_out
       CP    ":"                ; ":" = trace to break
       JP    Z,_trace_go
       CP    $08                ; <BACKSPACE> = PC-1
       JP    Z,_trace_back
       LD    B,$01
       CP    "c"                ; "C" = toggle Carry flag
       JR    Z,ToggleFlags
       LD    B,$02
       CP    "n"                ; "N" = toggle Negative flag
       JR    Z,ToggleFlags
       LD    B,$04
       CP    "v"                ; "V" = toggle parity/oVerflow flag
       JR    Z,ToggleFlags
       LD    B,$10
       CP    "h"                ; "H" = toggle Half carry flag
       JP    Z,ToggleFlags
       LD    B,$40
       CP    "z"                ; "Z" = toggle Zero flag
       JP    Z,ToggleFlags
       LD    B,$80
       CP    "s"                ; "S" = toggle Sign flag
       JP    Z,ToggleFlags
       CP    "-"
       CALL  Z,mem_minus        ; "-" = scroll memory window down
       JR    Z,Trace_mem
       CP    "="
       CALL  Z,mem_plus         ; "=" = scroll memory window up
       JR    Z,Trace_mem
.not_mem:
       CP    'q'                ; "Q" = quit
       JP    Z,DebugMenu
       CP    '1'
       JR    C,.other_cmd
       CP    '4'+1              ; '1'-'4' = memory window
       JR    C,MemAddr
.other_cmd:
       LD    HL,trace_cmds
       LD    B,(trace_cmd_end-trace_cmds)/3
       CALL  DoCommand
       JP    C,Trace            ; refresh window if required
       JP    _tr_wait_key       ; else just wait for key

trace_cmds:
       db    "p"
       dw    Proceed
       db    "g"
       dw    Go
       db    "b"
       dw    EditBreak
       db    "l"
       dw    LoadFile
       db    "w"
       dw    WriteFile
       db    "t"
       dw    Jump
       db    "e"
       dw    EditMem
       db    "f"
       dw    FillMem
       db    "m"
       dw    MoveMem
       db    "u"
       dw    Unassemble
       db    "d"
       dw    DumpMem
       db    "r"
       dw    EditReg
       db    " "
       dw    ToggleScreen
       db    "?"
       dw    TraceHelp
trace_cmd_end:


TraceHelp:
       LD    IX,TraceHelpWindow
       CALL  OpenWindow
       LD    HL,trace_msg
       CALL  WinPrtStr
       JP    WAIT_KEY       ; wait for any key


Jump:  CALL  InputPC
       RET   Z
       LD    (PC_reg),HL
       RET

InputPC:
       LD    IX,JumpWindow
       CALL  OpenWindow
       JP    InputAddress


; set trace memory window address
MemAddr:
       SUB   '1'
       LD    (IY+MemWindow),A  ; store selected window # 0-3
       LD    IX,SetMemWindow
       CALL  OpenWindow
       LD    A,'M'
       CALL  WinPrtChr
       LD    A,(IY+MemWindow)
       ADD   '1'
       CALL  WinPrtChr
       LD    A,' '
       CALL  WinPrtChr
       CALL  InputAddress
       JR    Z,.done
       EX    DE,HL
       CALL  PutMemWindow
.done:
       JP    Trace

mem_minus:
       CALL  GetMemWindow
       LD    HL,-8
       JR    _mem_adj
mem_plus:
       CALL  GetMemWindow
       LD    HL,8
_mem_adj:
       ADD   HL,DE
       EX    DE,HL

; in: DE = memory address
PutMemWindow:
       LD    A,(IY+MemWindow)
       ADD   A                ; *2 = 16 bit index
       LD    C,A
       LD    B,0              ; BC = index
       LD    HL,MemWindows
       ADD   HL,BC            ; index into memory window address array
       LD    (HL),E
       INC   HL               ; store window address
       LD    (HL),D
       CP    A                ; ret z
       RET

GetMemWindow:
       LD    A,(IY+MemWindow)
       AND   3                  ; 0-3 = window 1-4
       ADD   A                  ; A*2 = index 16 bit value
       LD    C,A
       LD    B,0                ; BC = index
       LD    HL,MemWindows
       ADD   HL,BC
       LD    E,(HL)
       INC   HL                 ; DE - memory window address
       LD    D,(HL)
       RET


; show saved systen screen
ToggleScreen:
       call  restore_screen   ; show BASIC screen
       jp    WAIT_KEY         ; wait for any key


; skip over current instruction
_Skip:
       LD    HL,(PC_reg)      ; HL = target code address
       PUSH  HL
       CALL  Decode           ; get opcode attributes
       POP   HL
       LD    A,C
       AND   $0F
       LD    C,A              ; BC = instruction length
       LD    B,0
       ADD   HL,BC
       LD    (PC_reg),HL
       JP    TraceShow

; previous PC
_trace_back:
       LD    HL,(PC_reg)
       DEC   HL               ; PC-1
       LD    (PC_reg),HL
       JP    TraceShow

;--------------------------------------------------------
; RTN - single step (trace next instructoun)
;
; copies instruction to sandbox, appends a Break (RST $38),
; then executes the instruction. Instructions that may
; run away (eg. JR, JP, CALL) are emulated.
;
_trace_next:
       call  restore_screen
       LD    HL,TraceCode
       LD    DE,TraceOp
       LD    BC,5             ; initialize trace code buffer to NOP*4+BREAK
       LDIR
       LD    HL,(PC_reg)      ; HL = target code address
       PUSH  HL
       CALL  Decode           ; get opcode attributes
       POP   HL
       LD    B,C
       LD    A,C
       AND   $0F
       LD    C,A              ; BC = instruction length
       LD    A,B
       LD    B,0
       LD    DE,TraceOp
       LDIR                   ; copy instruction to trace code buffer
       LD    (BrkAddr),HL     ; set current breakpoint address
       BIT   6,A
       JR    NZ,_trace_ex     ; class = (bit 6)?
       BIT   2,A
       JR    NZ,_trace_ex     ; class = (bit 2)?
       CALL  Trap             ; emulate dangerous instructions
       JP    Z,TraceAddr      ; if emulated then don't execute
_trace_ex:
       LD    HL,TraceOp
       LD    (PC_reg),HL      ; PC = code in trace buffer
       JR    _trace_go        ; execute instruction, then break

;-----------------------------------------------------------
;   ';' - Trace out - execute until RET
;-----------------------------------------------------------

_trace_out:
       LD   HL,(PC_reg)
       jr   _trace_call

;-----------------------------------------------------------
;     P - Proceed
;
; Execute code as subroutine
;
; equivalent to CALL xxxx, break
;
Proceed:
       ld    IX,ProcWindow
       call  OpenWindow
       call  InputAddress
       ret   z
_trace_call:
       ex    de,hl
       ld    HL,TraceOp+3
       ld    (HL),$FF        ; RST $38 = break
       dec   HL
       ld    (hl),d
       dec   hl
       ld    (hl),e          ; CALL nnnn
       dec   hl
       ld    (hl),$cd
       jr    _goHL           ; execute our code, then break

;-----------------------------------------------------------
; G - Go
;-----------------------------------------------------------
;
; execute code without RET (until breakpoint or forever!)
;
; equivalent to JP xxxx
;
Go:    ld    ix,GoWindow
       call  OpenWindow
       call  InputAddress
       ret   z
;-----------------------------------------------------------
;   Trace instruction(s)
;-----------------------------------------------------------
;
; Save all registers, then jump into target code.
; 'returns' to Break through RST $38.
;
; from address in HL
_goHL: LD    (PC_reg),HL     ; set PC
; from PC_reg
_trace_go:
       call  restore_screen  ; show system screem
       CALL  SetBreakpoints  ; set all breakpoints
       CALL  InitBreak       ; redirect RST $38 to Break
; continue tracing
_trace_cont:
       DI
       LD    SP,Alt_Regs     ; SP-> alternate registers
       EX    AF,AF'
       POP   AF              ; load AF'
       EX    AF,AF'
       EXX
       POP   BC
       POP   DE              ; load alternate registers
       POP   HL
       EXX
       POP   AF
       POP   BC
       POP   DE              ; load base registers
       POP   HL
       POP   IX
       POP   IY
       LD    SP,(SP_reg)     ; SP = SP_reg
       PUSH  HL              ; push HL_reg onto stack
       LD    HL,(PC_reg)     ; HL = PC_reg
       EX    (SP),HL         ; PC_reg on stack, HL = HL_reg
;       EI
       RET                   ; jump into code at PC_reg

;------------------------------------------------------
;  Breakpoint hit, save all registers and enter Trace.
;  Also comes here from BASIC USR() function.
;
Break:
       DI
       EX    (SP),HL         ; get return address
       LD    (PC_reg),HL     ; save return address (PC after breakpoint)
       EX    (SP),HL         ; restore HL
       INC   SP
       INC   SP              ; SP+2 = value before execution of RST $38
       LD    (SP_reg),SP     ; save SP
       LD    SP,SP_reg       ; switch stack to register save area
       PUSH  IY
       PUSH  IX
       PUSH  HL
       PUSH  DE              ; save base registers
       PUSH  BC
       PUSH  AF
       EXX
       PUSH  HL
       PUSH  DE              ; save alternate registers
       PUSH  BC
       EXX
       EX    AF,AF'
       PUSH  AF              ; save AF'
       EX    AF,AF'
       LD    IY,Vars         ; restore debug IY
       LD    SP,stack_top    ; switch SP to debug stack
;       EI
       LD    HL,(UserAddr)
       LD    (USRADDR),HL    ; restore original USRADDR

       LD    HL,(PC_reg)
       LD    BC,TraceBrk+1   ; BC = after RST $38 at end of sandbox
       OR    A
       SBC   HL,BC           ; = our PC? (did single step)
       jr    nz,.not_singlestep
       LD    HL,(BrkAddr)    ; yes, PC = address of next instruction
       JR    .in_sandbox
.not_singlestep:
       ADD   HL,BC
       DEC   BC              ; BC = after CALL nnnn, RST $38 in sandbox
       OR    A
       SBC   HL,BC           ; = our PC ? (stepped into subroutine)
       jr    nz,.not_sandbox
       LD    HL,(SP_reg)     ; yes, HL = target stack
       LD    E,(HL)
       INC   HL
       LD    D,(HL)          ; pop return address off target stack
       INC   HL
       LD    (SP_reg),HL
       ex    DE,HL           ; tracing from return address
.in_sandbox:
       CALL  save_screen     ; save system screen
       JP    TraceAddr
; may get here from:-
; - RST $38    replacing user code, address in breakpoint array
; - RST $38    in user code
; - CALL Break in user code            ''
; - JP USRJMP  via X=USR(x)  HL = USRJMP, BC = $0203
;
.not_sandbox:
       LD    HL,(HL_reg)
       LD    DE,USRJMP
       CMPHLDE
       JR    NZ,.not_usrfunc
       LD    HL,(BC_reg)
       LD    DE,$0203
       CMPHLDE
       JR    NZ,.not_usrfunc
       LD    HL,(UserAddr)   ; get original user address
       LD    (PC_reg),HL     ; PC = user code
       JP    _trace_cont     ; return to user code
.not_usrfunc:
       LD    DE,(PC_reg)
       DEC   DE              ; PC-1 = address of RST $38
       CALL  FindBreak       ; hit one of our breakpoints?
       JR    NZ,.nobrk
       LD    (PC_reg),DE     ; yes, PC = breakpoint addreess
.nobrk:
       CALL  RemoveBreaks    ; restore original code at all breakpoints
       CALL  save_screen     ; save system screen
       JP    Trace           ; tracing from breakpoint


;-------------------------------------------------------
;    R - Registers
;-------------------------------------------------------

Registers:
       LD    IX,RegisterWindow
       CALL  OpenWindow
       CALL  ShowRegs
       CALL  _edit_reg
       JR    NC,Registers        ; until exit requested
       RET

;-------------------------------------------------------
;   F - Fill Memory -
;-------------------------------------------------------
fillmem:
       LD    IX,FillMemWindow
       CALL  OpenWindow
       CALL  InputStartEnd       ; input start & end addresses
       RET   Z
       PUSH  DE                  ; push start address
       PUSH  HL                  ; push end address
       CALL  WinPrtMsg
       db    "     Fill Byte?",0
       CALL  InputByte           ; input fillbyte
       POP   HL                  ; HL = end
       POP   DE                  ; DE = start
       RET   Z                   ; return if no fill byte
       CALL  restore_screen      ; show system screen
       CP    A                   ; reset Carry flag
       SBC   HL,DE               ; HL = length
       RET   C                   ; quit if end < start
       LD    (DE),A              ; fill first location
       JR    Z,.done             ; if end = start then filled
       LD    B,H
       LD    C,L                 ; BC = length
       LD    H,D                 ; HL = start
       LD    L,E
       INC   DE                  ; DE = start +1
       LDIR                      ; copy fillbyte to other locations
.done:
       call  save_screen         ; save system screen
       RET

;-------------------------------------------------------
;   M - Move Memory -
;-------------------------------------------------------
movemem:
       LD    IX,MoveMemWindow
       CALL  OpenWindow
       CALL  InputStartEnd       ; DE = start, HL = end
       RET   Z
       OR    A
       SBC   HL,DE               ; HL = end - start
       RET   C                   ; quit if end < start
       INC   HL                  ; HL = length
       PUSH  HL                  ; push length
       PUSH  DE                  ; push start
       CALL  WinPrtMsg
       db    "  Dest Address?",0
       CALL  InputWord
       EX    DE,HL               ; DE = dest
       POP   HL                  ; HL = start
       POP   BC                  ; BC = length
       RET   Z                   ; quit if no dest
       call  restore_screen      ; show system screen
       OR    A
       SBC   HL,DE
       ADD   HL,DE
       JR    C,mm_lddr           ; if start < dest then LDDR
mm_ldir:
       LDIR                      ; else LDIR
       JR    mm_done
mm_lddr:
       DEC   BC
       EX    DE,HL
       ADD   HL,BC               ; dest = dest + (length-1)
       EX    DE,HL
       ADD   HL,BC               ; start = start + (length-1)
       INC   BC
       LDDR
mm_done:
       call  save_screen         ; save system screen
       RET

;-------------------------------------------------------
;   L - load file into RAM
;-------------------------------------------------------
LoadFile:
       LD    IX,LoadFileWindow
       CALL  OpenWindow
.catalog:
       CALL  WinCatDisk        ; list files in directory
       JP    C,_disk_error
       JR    NZ,.getfilename
       LD    DE,256*14+10      ; put cursor in middle of window
       CALL  WinSetCursor
       LD    HL,.nofiles_msg
       CALL  WinPrtStr         ; print 'no files'
       JP    Wait_Key          ; wait for any key
.getfilename:
       CALL  InputFilename     ; input filename
       RET   Z
       CALL  usb__ready        ; check for USB drive
       JP    NZ,_disk_error
       CALL  usb__open_read    ; open file
       JR    Z,.loadfile       ; if opened file then load it
       CP    CH376_ERR_OPEN_DIR
       JR    Z,.changedir        ; if directory then CD to it
       CP    CH376_ERR_MISS_FILE
       JP    NZ,_file_error      ; if not missing file then error
       LD    HL,.file_missing_msg
       CALL  WinPrtStr         ; print "file not found"
       JR    .getfilename      ; back to input filename
.changedir:
       LD    HL,LineBuffer
       CALL  strlen
       CALL  dos__set_path     ; add directory to path
       JR    .catalog
.loadfile:
       CALL  InputAddress      ; input load address
       RET   Z
       call  restore_screen    ; switch to system screen
       LD    DE,-1
       CALL  usb__read_bytes   ; read file into memory
       CALL  usb__close_file
       call  save_screen       ; save system screen
       CALL  ShowFileBytes
       ld    hl,.bytesloaded_msg
       call  WinPrtStr
       call  Wait_Key
       ret

.bytesloaded_msg:
       DB    " Bytes loaded",0
.nofiles_msg:
       DB    "No files",0
.file_missing_msg:
       DB    "File not found",CR,0

;-------------------------------------------------------
;   W - Write memory to file
;-------------------------------------------------------
;
WriteFile:
       LD    IX,WriteFileWindow
       CALL  OpenWindow
.catalog:
       CALL  WinCatDisk       ; show files
       JP    C,_disk_error    ; disk error if can't read directory
.input_name:
       CALL  InputFilename
       RET   Z                ; quit if no file entered
       ld    de,FileName
       call  strcpy           ; copy file name to FileName
       ld    (de),a           ; null-terminate
       LD    HL,FileName
       CALL  usb_open_read    ; try to open existing file
       JR    Z,.write_file    ; if file present then overwrite it
       CP    CH376_ERR_MISS_FILE ; if file missing then write new file
       JR    Z,.write_file
       CP    CH376_ERR_OPEN_DIR ; if directory then CD to it
       JP    NZ,_file_error   ; else file error
       LD    HL,LineBuffer
       CALL  strlen
       CALL  dos__set_path     ; add directory to path
       JR    .catalog
.write_file:
       CALL  InputAddress     ; input address
       RET   Z
       ex    de,hl            ; DE = address
       CALL  WinPrtMsg
       db    CR," Length?",0
       CALL  InputWord        ; input length
       RET   Z
       ex    de,hl            ; HL = address, DE = length
       push  hl
       ld    hl,FileName
       CALL  usb__open_write  ; open file for writing
       pop   hl
       JR    NZ,_file_error   ; error if can't open file
       LD    A,H
       CP    $38              ; saving screen RAM?
       call  c,restore_screen ; yes, switch to system screen
.write:
       CALL  usb__write_bytes ; write data to disk
       CALL  usb__close_file
       CALL  OpenWindow       ; refresh window
       CALL  ShowFileBytes
       LD    HL,_wr_bytes_msg
       CALL  WinPrtStr
       JP    Wait_Key         ; wait for any key

ShowFileBytes:
       EX    DE,HL            ; HL = number of bytes written
       CALL  INT2STR          ; convert 16 bit number to decimal string
       call  OpenWindow        ; refresh window
       ld    d,10
       ld    e,(IX+win_h)
       srl   e
       call  WinSetCursor
       LD    HL,FPSTR+1
       JP    WinPrtStr        ; print bytes written


_wr_bytes_msg:
       DB    " Bytes written",0
_disk_err_msg:
       db    "disk error",0
_file_err_msg:
       db    "file error",0

_disk_error:
       LD    HL,_disk_err_msg
       jr    _do_file_error
_file_error:
       LD    HL,_file_err_msg
_do_file_error:
       LD    DE,256*13+10
       CALL  WinSetCursor
       CALL  WinPrtStr
       JP    WAIT_KEY     ; wait for any key

;
; input Start & End address
;
; out DE = start
;     HL = end
;
InputStartEnd:
       CALL  WinPrtMsg
       db    " Start Address?",0
       CALL  InputWord
       RET   Z
       EX    DE,HL               ; DE = start address
       CALL  WinPrtMsg
       db    "   End Address?",0

;------------------------------------------------------
;           input 4 digit Hex number
;------------------------------------------------------
;
; output: HL = 16 bit number
;
InputWord:
       PUSH  DE
       LD    DE,LineBuffer
       LD    A,4                 ; 4 hex chars to enter
       CALL  InputLine
       CP    $03                 ; if ^C or Q then return z
       JR    Z,.done
       CALL  NewLine
       CALL  Hex2binHL           ; HL = address, z = invalid
.done:
       POP   DE
       RET

;------------------------------------------------------
;            input 2 digit Hex number
;------------------------------------------------------
;
; output: A = 8 bit number
;
InputByte:
       PUSH  DE
       LD    DE,LineBuffer
       LD    A,2                 ; 2 hex chars to enter
       CALL  InputLine
       CP    $03                 ; if ^C or Q then return z
       JR    Z,.done
       CALL  NewLine
       LD    A,2                 ; 2 chars max
       CALL  Hex2bin             ; number = byte, z = invalid
       LD    A,(IY+_number)
.done:
       POP   DE
       RET

;----------------------------------------------------
;             Input Digit 0-9
;----------------------------------------------------
;
; out: A = digit
;      z = no digit
;
InputDigit:
       PUSH  DE
       LD    DE,LineBuffer
       LD    A,1           ; 1 character to enter
       CALL  InputLine
       CP    $03           ; if ^C or Q then return z
       JR    Z,.abort
       CALL  NewLine
       LD    A,(DE)        ; get char
       SUB   "0"           ; convert to binary
       JR    C,.abort
       CP    10            ; if not 0-9 then abort
       JR    C,.done
.abort:
       CP    A             ; return z
.done:
       POP   DE
       RET


;-------------------------------------------------
;                - LINE INPUT -
;
;  in: DE = input buffer
;       A = buffer length
; (IY+_flags) = options CAPSLOCK, QUITKEY, HELPKEY
;
; out:  A = terminating character
;       z = ^C
;            or 'Q' as first char (QUITKEY flag set)
;            or '?' as first char (HELPKEY flag set)
;
;      nz = RTN (line may be empty)
;
InputLine:
       PUSH  HL
       PUSH  DE
       PUSH  BC
       EX    DE,HL           ; HL = buffer
       LD    B,A             ;  B = buffer length
       INC   B               ; +1 for NULL at end of buffer
       LD    C,0             ;  C = character count
       LD    E,(IY+_flags)   ; E = flags
       LD    (HL),C          ; clear buffer
.getkey
       EXX
       LD    (HL),255        ; show cursor
       EXX
       CALL  WAIT_KEY        ; wait for next keypress
       BIT   STRING,E
       JR    NZ,.dokey
       BIT   CAPSLOCK,E
       CALL  NZ,Uppercase    ; if capslock on and not string then force to uppercase
.dokey:
       EXX
       LD    (HL)," "        ; hide cursor
       EXX
       LD    D,A             ; D = key char
       CP    $08             ; <BACKSPACE> = cursor left
       JR    Z,.backspc
       CP    $0D             ; <CR> = line entered
       JR    Z,.done
       CP    $03             ; ^C = abort
       JR    Z,.quit
       CP    " "             ; space or higher?
       JR    C,.getkey       ; eat other control chars
       BIT   QUITKEY,E
       JR    Z,.notq
       CP    "Q"             ; if quit on Q enabled and key = 'Q' then quit
       JR    Z,.quit
.notq:
       BIT   HELPKEY,E       ; Help on '?' enabled?
       JR    Z,.ascii
       CP    '?'
       JR    Z,.done          ; if ? then quit
.ascii:
       LD    A,B
       DEC   A
       CP    C               ; cursor at end of buffer?
       JR    Z,.getkey       ; yes, buffer full so don't store char
       LD    A,D
       CP    '"'
       JR    NZ,.store
       LD    A,E
       XOR   1<<STRING       ; if '"' then toggle string flag
       LD    E,A
.store:
       LD    A,D
       LD    (HL),A          ; store char in buffer
       CALL  WinPrtChr       ; show character on screen
       INC   HL              ; next location in buffer
       INC   C               ; increment character count
       JR    .getkey         ; done, get next key
.backspc:
       LD    A,C
       OR    A               ; don't backspace beyond start of buffer
       JR    Z,.getkey
       DEC   HL              ; HL = position of last char in buffer
       LD    A,(HL)
       CP    '"'
       JR    NZ,.rubout
       LD    A,E
       XOR   1<<STRING       ; if char is '"' then toggle string flag
       LD    E,A
.rubout:
       LD    (HL),0          ; delete last char in buffer
       DEC   C               ; position - 1
       CALL  BackSpace       ; print <BACKSPACE>
       JR    .getkey         ; get next key
.quit:
       LD    D,$03           ; change 'Q' to ^C
.done:
       LD    (HL),0          ; terminate end of entered text
       LD    A,D             ; A = last key pressed
       CP    $03             ; z = ^C, nz = RTN
       POP   BC
       POP   DE
       POP   HL
       RET


;------------------------------------------------
;        Check for Key Pressed
;------------------------------------------------
;
; Reads keyboard matrix directly, no debounce!
;
; out: NZ = key pressed
;       Z = key not pressed
;
AnyKey:
    push bc
    ld   bc,$00ff  ; Scan all columns at once.
    in   a,(c)     ; Read the results.
    cpl            ; invert - (key down now gives 1)
    pop  bc
    and  $3f       ; check all rows.
    ret

ContKey: ; 'C' key
    push bc
    ld   bc,$efff  ; Scan A12 column
    in   a,(c)     ; Read the results
    pop  bc
    cp   $ef       ; z = only D5 row is down
    ret

SpaceKey:
    push bc
    ld   bc,$bfff  ; Scan A14 column
    in   a,(c)     ; Read the results
    pop  bc
    cp   $ef       ; z = only D5 row is down
    ret

;------------------------------------------------
;  compare memory to string of specified length
;------------------------------------------------
;  in: DE = string
;      HL = memory
;       C = length of string
;
; out: Z = found
;
strlcmp:
       CALL  PEEK       ; read byte
       EX    DE,HL
       CP    (HL)       ; compare to our byte
       EX    DE,HL
       INC   DE
       INC   HL         ; point to next byte
       RET   NZ
       DEC   C          ; decrement count
       JR    NZ,strlcmp ; compare next byte
       RET

;-----------------------------------
;    pad with spaces to 6 chars
;-----------------------------------
Pad_6  LD    A,5
       SUB   C
       LD    B,A
_pad_loop:
       CALL  prt_space
       DJNZ  _pad_loop
       RET

;----------------------------------------------------------
;                Print Opcode Name
;----------------------------------------------------------
;   in: A = name#, DE = names
;
PrtOpName:
       PUSH  HL
       EX    DE,HL
       INC   A        ; starting at name #1
       CALL  L7E8D    ; print name
       POP   HL
       RET

;----------------------------------------------------------
;    Convert Numeric string to 16 bit Binary Number
;----------------------------------------------------------
;
;  in: DE = numeric string
;
; out: HL = number
;      NZ = valid number, Z = invalid
;
Num2BinHL:
       LD    A,(DE)         ; get char
       CP    "$"            ; Hex number?
       JP    Z,Hex2BinHL    ; yes, get Hex number
       CP    "0"
       JR    C,n2b_nan      ; if < "0" then not a number
       CP    "9"+1          ; decimal?
       JP    C,Dec2BinHL    ; yes, get decimal number
n2b_nan:
       CP    A
       RET                  ; no, ret Z

;-------------------------------------------------------
;                   Edit memory
;-------------------------------------------------------
EditMem:
       LD    IX,EditWindow
       CALL  OpenWindow
       LD    HL,Edit_msg
       CALL  WinPrtStr           ; show help message
       CALL  InputAddress
       JR    NZ,.em_start
       CP    $03                 ; if ^C typed then quit
       RET   Z
       LD    HL,(EditAddr)       ; use last address
.em_start:
       LD    (EditADDR),HL
_em_line:
       CALL  NewLine
       CALL  PrtHexWord          ; print address
       CALL  prt_space
       CALL  PEEK                ; get byte at address
       LD    B,A
       CALL  prt_byte            ; print byte in Hex
       CALL  prt_space
       LD    A,B
       CALL  prt_ascii           ; print byte as char
       CALL  prt_space
       LD    DE,linebuffer       ; line input buffer
       LD    A,(IX+win_w)
       SUB   11                  ; A = line input buffer size
       CALL  InputLine           ; type in hex byte(s) or string
       JR    Z,_em_done          ; ^C or Q so quit
.get_next:
       LD    A,(DE)
       CP    " "                 ; get 1st non-space char in buffer
       JR    NZ,.got_next
       INC   DE
       JR    .get_next
.got_next:
       OR    A                   ; NULL?
       JR    Z,_em_rtn           ; yes, advance to next address
       JR    _em_next_byte       ; start poking bytes
;_em_rtn:
_em_rtn:
       INC   HL                  ; advance to next address
       JR    _em_line
_em_next_byte:
       LD    A,2                 ; 2 chars max per byte
       CALL  Hex2bin             ; convert Hex byte to binary
       JR    Z,_em_string        ; if not Hex then may be string
; hex byte(s)
       LD    A,(IY+_number)
       CALL  POKE                ; poke byte into RAM
       INC   HL
       JR    _em_next_byte       ; loop until all bytes done
; ascii string
_em_string
       LD    A,(DE)              ; get char in buffer
       INC   DE                  ; De-> next char in buffer
       CP    '"'                 ; opening quote?
       JR    NZ,_em_cmd          ; no,
_em_nxtchr
       LD    A,(DE)              ; yes, get next char
       INC   DE
       CP    '"'                 ; closing quote?
       JR    Z,_em_next_byte     ; yes, switch to getting bytes
       OR    A                   ; NULL?
       JR    Z,_em_poked         ; yes, end of input
       CALL  POKE                ; poke char into RAM
       INC   HL                  ; next addr
       jr    _em_nxtchr          ; get next string char
; inline command
_em_cmd:
       CP    "@"                 ; entering new address?
       CALL  Z,Hex2binHL         ; yes, convert hex address to binary
_em_poked:
       JR    _em_line            ; do next input line
_em_done:
       RET


;------------------------------------------------
;             Input Filename
;------------------------------------------------
;
; out: HL = filename (linebuffer)
;       Z = no name (^C or zero length)
;
InputFilename:
       CALL  WinPrtMsg
       db    "Filename?",0
       LD    DE,LineBuffer
       LD    A,12             ; 12 chars max
       LD    (IY+_flags),1<<CAPSLOCK  ; Q is not quit, ? is not help
       CALL  InputLine        ; input filename
       LD    (IY+_flags),(1<<CAPSLOCK)|(1<<QUITKEY)|(1<<HELPKEY)
       RET   Z                ; quit if no name
       CALL  NewLine
       EX    DE,HL            ; HL = filename
       LD    A,(HL)
       OR    A                ; Z = no filename
       RET


;------------------------------------------------
;               Input Address
;------------------------------------------------
;
;   in: IX = window
;
;  out: NZ, HL = address
;        Z,  A = abort code: 0 = no digits, 3 = ^C
;
InputAddress:
       PUSH  DE
       PUSH  HL
       CALL  WinPrtMsg
       db    "Address?",0
       LD    DE,LineBuffer
       LD    A,4            ; 4 hex chars to enter
       CALL  InputLine
       JR    Z,.done
       CALL  Hex2binHL      ; HL = address, Z = no hex
       JR    Z,.done
       EX    (SP),HL        ; address onto stack
.done:
       POP   HL             ; HL = address or restored
       POP   DE
       RET

;------------------------------------------------
; - D - Dump Memory
;------------------------------------------------
DumpMem:
       LD    IX,DumpAddrWindow
       CALL  OpenWindow
       CALL  InputAddress      ; input address
       JR    Z,.noaddr
       LD    (DumpAddr),HL
       JR    _dmp_window
.noaddr:
       CP    $03               ; if ^C then quit
       RET   Z
       LD    HL,(DumpAddr)     ; use previous address
_dmp_window:
       LD    IX,DumpWindow
       CALL  OpenWindow        ; open/refresh Dump window
_dmp_page:
       LD    DE,0              ; cursor at 0,0
       CALL  WinSetCursor
       LD    B,DUMPLINES       ; lines to dump
       JR    _dmp_line
_dmp_lines:
       CALL  NewLine
_dmp_line:
       PUSH  BC
       LD    BC,(1*256)+8      ; 1 line, 8 bytes per line
       CALL  DisplayMem        ; display memory in Hex and ASCII
       POP   BC
       CALL  ContKey           ; if C key pressed then dump continously
       JR    NZ,_dmp_next
       LD    B,1
       JR    _dmp_lines
_dmp_next:
       DJNZ  _dmp_lines
_dmp_waitkey:
       CALL  WAIT_KEY
       CP    "q"               ; Q = quit
       RET   Z
       CP    "?"               ; ? = help
       JR    Z,_dmp_help
       CP    $0D               ; RTN = dump page
       JR    Z,_dmp_page
       LD    B,1
       CP    "c"               ; "C" = continuous
       JR    Z,_dmp_lines
       CP    "d"               ; "D" = enter Dump address
       JR    Z,_dmp_addr
       CP    $08               ; BACKSPACE = rewind
       JR    Z,_dmp_back
       CP    " "               ; SPACE = show system screen
       JR    NZ,_dmp_waitkey
       CALL  ToggleScreen
       JR    _dmp_again
_dmp_back:
       LD    BC,-(DUMPLINES*16)
       ADD   HL,BC             ; rewind to previous page
       JR    _dmp_page
_dmp_addr:
       LD    IX,AddrWindow
       CALL  OpenWindow
       CALL  InputAddress      ; input new dump address
       JR    _dmp_window
_dmp_help:
       PUSH  HL
       LD    IX,DumpHelpWindow
       CALL  OpenWindow
       LD    HL,dump_msg
       CALL  WinPrtStr
       CALL  WAIT_KEY          ; wait for any key
       POP   HL
_dmp_again:
       LD    BC,-(DUMPLINES*8)
       ADD   HL,BC             ; rewind to start of page
       JR    _dmp_window


;------------------------------------------------------------
;           Display Memory in HEX and ASCII
;------------------------------------------------------------
; in: HL = address
;      B = lines per block
;      C = bytes per line
;
_dm_row:
       CALL  NewLine
DisplayMem:
       PUSH  BC           ; save line count
       CALL  PrtHexWord   ; show address
       CALL  prt_space
       PUSH  HL
       LD    B,C          ; B = bytes per row
_dm_hex:
       CALL  PEEK         ; get byte from RAM
       CALL  prt_byte     ; show as Hex bytes
       CALL  prt_space
       INC   HL           ; next address
       DJNZ  _dm_hex      ; next hex byte
_dm_done_hex:
       POP   HL           ; restore memory address
       LD    B,C          ; restore bytes per row
_dm_asc:
       LD    A,C
       CP    9            ; if >8 bpr then don't show ASCII
       JR    NC,_dm_next_asc
       CALL  PEEK
       CALL  prt_ascii    ; show as ASCII chars
_dm_next_asc:
       INC   HL           ; next address
       DJNZ  _dm_asc      ; next ascii char
       POP   BC           ; restore line count
_dm_next:
       DJNZ  _dm_row      ; loop back to do next line
_dump_done:
       RET

;----------------------------------------------------------
;         Show Byte as ASCII char
;
prt_ascii
       CP    $7f           ; graphics chars?
       JR    NC,_non_ascii ; yes, show '.'
       CP    " "           ; viewable ascii?
       JR    NC,_prta      ; yes, show char
_non_ascii:
       LD    A,"."         ; else '.'
_prta  JP    WinPrtChr


;----------------------------------------------------------
;      - U - unassemble
;
Unassemble:
       LD    IX,UnasmAddrWindow
       CALL  OpenWindow
       CALL  InputAddress     ; input start address
       JR    NZ,.gotaddr      ; got valid address?
.noaddr:
       CP    $03              ; no, quit if ^C
       RET   Z
       LD    HL,(UnasmAddr)   ; else get previous address
       JR    .gotaddr
.enter_addr:
       CALL  GetTopAddr       ; get address of top line of code
       PUSH  HL
       LD    IX,AddrWindow
       CALL  OpenWindow       ; open address input window
       CALL  InputAddress
       POP   BC
       JR    NZ,.gotaddr      ; valid address entered?
       LD    H,B
       LD    L,C              ; no, HL = top line code address
       JR    .refresh
.gotaddr:
       LD    (UnasmAddr),HL   ; set start address
.refresh:
       LD    IX,UnasmWindow
       CALL  OpenWindow       ; open/refresh Unasm window
.showpage:
       LD    DE,0
       CALL  WinSetCursor     ; set cursor to top of window
       LD    B,UNASMLINES     ; lines per page
       JR    .first_line
.next_line:
       CALL  NewLine          ; down to next line
.first_line:
       CALL  unasm            ; unassemble 1 line
       CALL  ClearToEnd       ; spaces to end of line
       CALL  ContKey          ; 'C' key pressed?
       JR    Z,.next_line
       DJNZ  .next_line       ; unassmble B lines
.wait_key:
       CALL  WAIT_KEY
.got_key:
       CP    "u"              ; "U" = enter Unassemble address
       JR    Z,.enter_addr
       CP    CR               ; RTN = show 1 page
       JR    Z,.showpage
       CP    "q"              ; Q = quit
       RET   Z
       CP    $03              ; ^C = quit
       RET   Z
       CP    "c"              ; C = list continuously
       LD    B,1
       JR    Z,.next_line
       CP    " "              ; SPACE = show system screen
       JR    Z,.screen
       CP    "?"              ; "?" = help
       JR    NZ,.wait_key     ; else eat key
.help:
       CALL  GetTopAddr       ; get code address from 1st line
       PUSH  HL
       LD    IX,UnAsmHelpWindow
       CALL  OpenWindow       ; open help window
       LD    HL,unasm_msg
       CALL  WinPrtStr        ; show help
       CALL  WAIT_KEY         ; wait for any key
       POP   HL
       JP    .refresh         ; refresh windw
.screen:
       CALL  GetTopAddr       ; get code address from 1st line
       CALL  ToggleScreen     ; show system screen (and pause until key pressed)
       JP    .refresh         ; refresh window

;------------------------------------------------------------
;         Get hex number at window 0,0
;------------------------------------------------------------
; Peeks into char screen and converts hex string found there
; to binary number in HL.
;
;  in: hex string (1-4 chars) displayed in window
;
; out: HL = binary address
;
GetTopAddr:
       LD    DE,0             ; DE = window x,y of hex address shown on top line
       CALL  CursorAddr
       EX    DE,HL            ; DE = hex address characters in screen RAM
       JP    Hex2BinHL        ; convert hex string to binary number

;---------------------------------------------
;      Unassemble an Opcode
;---------------------------------------------
;   in: HL = address
;  out: HL = next address
;
unasm:
       PUSH  BC
       PUSH  DE
       PUSH  HL
       CALL  Decode     ; analyze the opcode
       POP   HL
       LD    A,C
       AND   7
       LD    D,A        ; D = number of bytes in opcode
       INC   A
       LD    (IY+OpLen),A
       CALL  PrtHexWord ; print address
       CALL  prt_space
       LD    B,D        ; B = number of bytes in opcode
       PUSH  HL
_uhex  CALL  PEEK       ; read byte from memory
       INC   HL
       CALL  prt_byte   ; show opcode byte
       DJNZ  _uhex      ; next byte
       LD    A,4
       SUB   D          ; if < 4 bytes then...
       JR    Z,_u_op
       LD    B,A
_upad  CALL  prt_space
       CALL  prt_space  ; ...pad with spaces
       DJNZ  _upad
_u_op  CALL  prt_space  ; trailing space
       POP   HL
       LD    (CodeAddr),HL
       SET   4,C        ; not source code
       PUSH  HL
       CALL  prt_opcode ; print decoded opcode
       POP   HL
       LD    B,0
       LD    C,(IY+OpLen) ; C = opcode length
       DEC   C
       ADD   HL,BC      ; HL = next opcode
       POP   DE
       POP   BC
       RET

;----------------------------------------------------------
;                 Decode Opcode
;
;;  in: HL = code memory
;
;; out: C bit 7 set = has 16 bit number
;             6 set = has 8 bit number
;             5 set = has 8 bit PC-relative offset
;             4
;             3
;             2..0  = number of opcode bytes
;
;
Decode LD    C,2        ; assume no args, 2 bytes
       CALL  PEEK       ; get opcode byte
       CP    $FD
       JP    Z,L6DA0
       CP    $DD
       JP    Z,L6DA0
       CP    $ED
       JR    NZ,L6D60
; ED
       INC   HL
       CALL  PEEK       ; get next opcode byte
       AND   $C7
       CP    $43        ; LD (nn)rr, LD rr,(nn)
       RET   NZ
       LD    C,$84      ; 16 bit number, 4 bytes
       RET
; not FD/DD/ED
L6D60  CP    $CB        ; CB
       RET   Z
; base opcode
       LD    C,$42      ; 8 bit number, 2 bytes
       CP    $D3        ; OUT(n),A
       RET   Z
       CP    $DB        ; IN A,(n)
       RET   Z
       AND   $C7
       CP    6          ; LD r,n
       RET   Z
       CP    $C6        ; ADDA n
       RET   Z
       LD    C,$83      ; 16 bit number, 3 bytes
       CALL  PEEK       ; get opcode byte again
       CP    $C3        ; JP
       RET   Z
       CP    $CD        ; CALL nn
       RET   Z
       AND   $E7
       CP    $22        ; LD (nn),HL etc.
       RET   Z
       CALL  PEEK       ; get opcode byte again
       AND   $CF
       CP    1          ; LD BC,nn
       RET   Z
       AND   $C7
       CP    $C2        ; JP CC,nn
       RET   Z
       CP    $C4        ; CALL CC,nn
       RET   Z
       LD    C,$22      ; 8 bit offset, 2 bytes
       CALL  PEEK       ; get opcode byte again
       CP    $10        ; DJNZ d
       RET   Z
       CP    $18        ; JR d
       RET   Z
       AND   $E7
       CP    $20        ; JR CC, d
       RET   Z
       LD    C,1        ; else no number, 1 byte
       RET
; FD/DD
L6DA0  LD    C,$43      ; 8 bit number, 3 bytes
       INC   HL
       CALL  PEEK       ; get next opcode byte
       CP    $34        ; INC (IX+d)
       RET   Z
       CP    $35        ; DEC(IX+d)
       RET   Z
       AND   $C7
       CP    $46        ; LD r,(IX+d)
       RET   Z
       CP    $86        ; ADD A,(IX+d)
       RET   Z
       CALL  PEEK       ; get opcode byte again
       AND   $F8
       CP    $70        ;
       RET   Z
       LD    C,$44      ; 8 bit number, 4 bytes
       CALL  PEEK       ; get opcode byte again
       CP    $36        ; LD (IX+d),n
       RET   Z
       CP    $CB        ; CB
       RET   Z
       LD    C,$84      ; 16 bit number, 4 bytes
       CP    $21        ; LD IX,nn
       RET   Z
       AND   $E7
       CP    $22        ; LD (nn),IX etc.
       RET   Z
       LD    C,2        ; no number, 2 bytes
       RET


;----------------------------------------------------------
; Print opcode character, expanding ctrl codes;
;
; ctrl codes:-
;   1 = index register offset (IX+d)
;   2 = 16 bit number         (nnnn)
;   3 = index register       (IX,IY)
;
PrtOpChar:
       CP    $10              ; printable char?
       JP    NC,WinPrtChr        ; yes, print it
; control code
       CP    3                ; index register?
; 3 = IX or IY
       JR    NZ,L7D97
       LD    A,(IY+Prefix)    ; 'X' or 'Y'
       JP    WinPrtChr
L7D97  CP    1                ; index register offset?
       JR    NZ,L7DB0
; 1 = (IX+-d)
       LD    A,(IY+Offset)    ; A = offset
       LD    B,"+"
       OR    A                ; + or -?
       JP    P,L7DA8
       LD    B,"-"
       NEG                    ; make offset positive
L7DA8  PUSH  AF
       LD    A,B
       CALL  WinPrtChr           ; print "+" or "-"
       POP   AF
       JR    PrtByteA         ; print offset
; 2 = nnnn
L7DB0  PUSH  HL
       LD    HL,(CodeAddr)
       LD    A,(IY+OpLen)     ; A = header byte
       AND   $3F              ; isolate line length
       DEC   A                ; - 1 to exclude header byte
       LD    C,A
       LD    B,0              ; BC = opcode length
       ADD   HL,BC
       DEC   HL               ; HL -> end of opcode
       CALL  PEEK
       LD    D,A
       DEC   HL               ; get word at end of opcode
       CALL  PEEK
       LD    E,A
       LD    A,(IY+OpAttr)    ; A = opcode attributes
       LD    C,(IY+OpLen)     ; C = opcode length
       BIT   5,A              ; 16 bit value?
       JR    Z,L7DE2          ; no,
       BIT   4,A              ; relative jump?
       JR    Z,L7DEA          ; no,
       LD    E,D              ; yes, E = offset
       RL    D
       SBC   A,A              ; sign extend to 16 bits
       LD    D,A
       LD    HL,(CodeAddr)    ; HL = opcode address
       INC   HL
       INC   HL               ; skip opcode bytes
       ADD   HL,DE
       EX    DE,HL            ; DE = destination PC
       JR    L7DF8            ; print dest address

L7DE2  BIT   6,A              ; constant byte?
       JR    Z,L7DF8          ; no, quit
L7DEA  LD    A,D              ; A = byte
       POP   HL
       JR    PrtByteA         ; print it

L7DF8  POP   HL

; opcode atttribute flag bits
;    6 = byte constant
;    5 = 16 bit number
;    4 = relative offset

;----------------------------------------------------------
;     Print 16 bit number in hex, prepending "$" if > 10
;
; in: DE = number
;
PrtNum LD    A,D
       OR    A
       JR    NZ,PrtHexDE  ; if > 255 then print 16 bit hex
       LD    A,E
       CP    10
       JR    C,PrtByteE   ; if < 10 then print decimal 0-9
PrtHexDE
       LD    A,D
       CALL  PrtHexByte   ; print "$" + hex high byte
       LD    A,E
       JP    prt_byte     ; print hex low byte

;----------------------------------------------------------
; Print Byte in Hex, or decimal if 0~9
;
PrtByteE
       LD    A,E
PrtByteA
       CP    10            ; if > 9 then print "$" + byte
       JR    NC,PrtHexByte
       ADD   A,"0"         ; print 0.9
       JP    WinPrtChr

;----------------------------------------------------------
; print "$" + byte in hex
;
PrtHexByte
       PUSH  AF
       LD    A,"$"
       CALL  WinPrtChr        ; print "$"
       POP   AF
       JP    prt_byte      ; print byte in Hex

;----------------------------------------------------------
;      Print Opcode
;
;  in: HL = opcode table, A = index, C = attributes
;
prt_op:
       ADD   A,A           ; index * 2
       ADD   A,L
       LD    L,A
       LD    A,H
       ADC   A,0
       LD    H,A
       LD    D,(HL)
       INC   HL            ; D,E = opcode attributes
       LD    E,(HL)
       LD    A,D
       RRA
       RR    C
       RRA
       RR    C             ; C bits 7,6 = D bits 1,0
       AND   $3F
       LD    B,A           ; B bits 5..0 = D bits 7.2
       RLC   C
       RLC   C             ; C bits 1,0 = D bits 1,0
       LD    A,E
       RLA
       RL    C
       RLA
       RL    C             ; C bits 2..0 = E bits 2..0
       RLA                 ; C bits 4,3 = D bits 1,0
       RL    C
       LD    A,C
       AND   $1F
       LD    D,A           ; D = 1st operand
       LD    A,E
       AND   $1F
       LD    E,A           ; E = 2nd operand
       LD    A,B
       OR    A             ; if B <> 0 then
       JR    NZ,L7E47
       OR    $40           ;    set bit 6
L7E47  LD    HL,opcode_names
       CALL  L7E8D         ; print opcode
       LD    A,(IY+Opcode)
       OR    A
       JR    Z,L7E5A       ; if opcode <> 0 then
       LD    (IY+Opcode),0 ; reset opcode
       LD    A,"R"         ;    print "R"
       CALL  WinPrtChr
       XOR   A             ;    A = 0
       INC   C             ;    C + 1
L7E5A  CP    D             ; = D?
       RET   Z             ; yes, done
       CALL  Pad_6
       LD    A,D           ; A = 1st operand number
       CALL  L7E6C         ; print 1st operand
       LD    A,E           ; have a 2nd operand?
       OR    A
       RET   Z             ; no, done
       LD    A,","
       CALL  WinPrtChr
       LD    A,E           ; A = 2nd operand number
; print operand
L7E6C  LD    HL,cc_names   ; operand names
       BIT   3,(IY+Prefix) ; if prefix bit 3 set then
       JR    Z,L7E8D
       CP    $0A           ; if < 10 then
       LD    C,$0C         ; name = 12
       JR    Z,L7E8C
       CP    $d6           ; if 21 then
       JR    NZ,L7E8D
       LD    A,(IY+OpLen)  ; A = opcode length
       AND   $0F           ; 0..15
       CP    3             ; if not 3 then
       LD    C,$0E         ;   name = 14
       JR    NZ,L7E8C      ; else
       LD    C,$19         ;   name = 25
L7E8C  LD    A,C           ; A = name count
; get name
L7E8D  DEC   A             ; count down
       JR    Z,L7E97       ; until target name reached
L7E90  INC   HL            ; skip name char
       BIT   7,(HL)        ; start of next name?
       JR    Z,L7E90       ; no, keep skipping
       JR    L7E8D         ; yes, next name
; print name
L7E97  LD    C,A           ; char count = 0
L7E98  LD    A,(HL)        ; get char of opcode name
       AND   $7F           ; reset bit 7
       PUSH  DE
       CALL  PrtOpChar     ; expand and print opcode char
       POP   DE
       INC   HL            ; point to next char
       INC   C             ; char count + 1
       BIT   7,(HL)        ; until start of next name
       JR    Z,L7E98       ; do next char
       RET

;----------------------------------------------------------
; get opcode bits 5..3 (register number)
;
L7EA7  CALL  PEEK          ; get opcode
       RRA
       RRA
       RRA
       AND   7
       RET

;----------------------------------------------------------
;              Print Opcode and Args
;
; in: HL = code
;      C = attributes
;
prt_opcode:
       LD    (IY+OpAttr),C ; save attributes
       XOR   A
       LD    (IY+Opcode),A ; reset DD/FD opcode
       LD    (IY+Prefix),A ; reset DD/FD prefix
       CALL  PEEK          ; get opcode
       CP    $ED           ; ED prefix?
       JR    NZ,L7EFD      ; no,
; ED
       INC   HL
       CALL  PEEK          ; get opcode 2nd byte
       CP    $A0
       LD    DE,ED_table   ; ED 00..9F table
       JR    C,L7EF6       ; < $A0?
       CP    $B0
       JR    C,L7EF1       ; < $B0?
       LD    (IY+Opcode),A ; opcode = LDIR/CPIR etc.
       RES   4,A
L7EF1  LD    DE,LDI_table  ; ED A0..FF table
       SUB   $60           ; opcodes $60..
L7EF6  EX    DE,HL
       SUB   $40           ; opcodes $40..$9F
L7EF9  JP    prt_op        ; print opcode

L7EFD  CP    $FD
L7EFF  LD    B,"Y"
       JR    Z,L7F09
       LD    B,"X"
       CP    $DD
       JR    NZ,L7F14
; DD,FD
L7F09  LD    (IY+Prefix),B
       INC   HL
       INC   HL
       CALL  PEEK
       LD    (IY+Offset),A
       DEC   HL
       CALL  PEEK
L7F14  CP    $CB
       JR    NZ,L7F54
; CB
       LD    A,(IY+Prefix) ; get prefix
       OR    A
       JR    Z,L7F1F       ; if IX or IY then
       INC   HL            ;   skip DD/FD
L7F1F  INC   HL            ; skip CB
       CALL  PEEK          ; get CB opcode
       CP    $40
       JR    C,L7F46       ; if >= $40
L7F25  LD    DE,srl_names  ; "SRL",...
       RLCA
       RLCA
       AND   3             ; opcode name #
       CALL  PrtOpName         ; print name
       CALL  WinPrtMsg
       DB    "  ",$00
L7F36  CALL  L7EA7         ; get bit number
       ADD   A,"0"
       CALL  WinPrtChr        ; print bit number
       LD    A,","
       CALL  WinPrtChr
       JP    L7FCA         ; print register
; CB opcode < $40
L7F46  LD    DE,rlc_names  ; "RLC",...
       CALL  L7EA7         ; get opcode name #
       CALL  PrtOpName         ; print opcode name
       CALL  Pad_6
       JR    L7FCA         ; print register

L7F54  CP    $40
       LD    DE,L6B5F
       JR    C,L7FA6       ; $00..$3F
       CP    $80
       JR    NC,L7F85      ; $80..$FF
; $40..$7F
       CP    $76
       JR    NZ,L7F6C
; HALT
       CALL  WinPrtMsg
       DB    "HALT",$00
       RET
; LD r,r
L7F6C  CALL  WinPrtMsg
       DB    "LD   ",$00
       CALL  L7EA7
       PUSH  HL
       CALL  L7FCB         ; 1st register
       POP   HL
       LD    A,","
       CALL  WinPrtChr
       JR    L7FCA         ; 2nd register
; $80..$FF
L7F85  CP    $C0
       JR    C,L7FAB
; $C0..$FF
       LD    D,A
       AND   $C7
       CP    $C7
       LD    A,D
       JR    NZ,L7FA1
; RST
       CALL  WinPrtMsg
       DB    "RST  ",$00
L7F9B  LD    A,D
       AND   $38
       JP    PrtByteA      ; print RST number

L7FA1  LD    DE,L6BE2
       SUB   $C0
L7FA6  EX    DE,HL
       JP    prt_op

L7FAB  LD    DE,add_names  ; ADD...
       CALL  L7EA7         ; get opcode
       CALL  PrtOpName         ; print opcode
       CALL  Pad_6
       CALL  PEEK          ; opcode 2nd byte
       AND   $38
       JR    Z,L7FC4       ; if 0 then register = A
       CP    8
       JR    Z,L7FC4       ; if 8 then register = A
L7FC0  CP    $18
       JR    NZ,L7FCA
L7FC4  CALL  WinPrtMsg
       DB    "A,",$00
L7FCA  CALL  PEEK          ; get 2nd register
L7FCB  LD    HL,r_names
       AND   7             ; 3 bit register field
       INC   A
       INC   A             ; starting at "B"
       CP    8
       JR    NZ,L7FDE      ; if "(HL)" then
       BIT   3,(IY+Prefix)
       JR    Z,L7FDE       ; if prefix = $DD or $FD then
       LD    A,1           ; opcode name #1 ("IX/Y+d")
L7FDE  JP   L7E8D          ; print register name




;----------------------------------------------------------
;                 Create Breakpoint
;----------------------------------------------------------
;
;  input:  B = breakpoint number
;         DE = address
;
;   if address = 0 then kill breakpoint
;
CreateBreak:
       XOR   A
.mult:
       DEC   B
       JR    Z,.index
       ADD   brk.size       ; A = number * brk.size
       JR    .mult
.index:
       LD    HL,Breakpoints
       LD    C,A
       ADD   HL,BC          ; HL = breakpoint in array
       LD    A,D
       OR    E
       JR    Z,.setbrk      ; if address = 0 then status = 0 (inactive)
       LD    A,1            ; else status = 1 (active)
.setbrk:
       LD    (HL),A         ; brk.status = status
       INC   HL
       LD    (HL),E         ; brk.addr = address
       INC   HL
       LD    (HL),D
       RET

;----------------------------------------------------------
;        Destroy All Breakpoints
;
ClearBreaks:
       LD    HL,Breakpoints
       LD    B,brk.size*NUMBRKS
       LD    C,0
.next:
       LD    (HL),C
       INC   HL
       DJNZ  .next
       RET

;----------------------------------------------------------
;        Print a Breakpoint Number
;
; in: C = breakpoint number in ascii
;
PrtBreakpoint:
       CALL  WinPrtMsg
       DB    " Breakpoint ",$00
       LD    A,C
       JP    WinPrtChr

;----------------------------------------------------------
;        Show All Breakpoints
;
ShowBreakpoints:
       LD    HL,Breakpoints
       LD    B,NUMBRKS        ; B = number of breakpoints
       LD    C,"1"            ; C = breakpoint number "1"
.sb_loop:
       CALL  PrtBreakpoint
       CALL  prt_space
       LD    A,(HL)           ; get status
       OR    A
       JR    NZ,.sb_active    ; 1  = active
       CALL  WinPrtMsg
       DB    "= ----",$00     ; 0 = inactive
       JR    .sb_next
.sb_active:
       PUSH  HL
       INC   HL
       LD    E,(HL)
       INC   HL               ; DE = breakpoint address
       LD    D,(HL)
       CALL  WinPrtMsg
       DB    "=",$00
       CALL  prt_space
       EX    DE,HL
       CALL  PrtHexWord       ; print breakpoint address
       CALL  prt_space
       POP   HL
.sb_next:
       LD    DE,brk.size
       ADD   HL,DE            ; advance to next entry
       INC   C                ; next breakpoint number
       CALL  NewLine
       DJNZ  .sb_loop
       RET

regnames:
       DB    "AFBCDEHL"
       DB    "IXIYSPPC"
flagnames:
       DB    "SZ H VNC"

;----------------------------------------------------------
;      - EDIT REGISTER -
;
EditReg:
       LD    IX,EditRegWindow
       CALL  OpenWindow
; edit register in current window
;
; out: nc = register edited
;       c = ^C, Q, or no register entered
;
; uses: HL,DE,BC
_edit_reg:
       CALL  WinPrtMsg
       db    CR,"Register?",0
       LD    DE,LineBuffer
       LD    A,2            ; max chars to input
       CALL  InputLine
       SCF
       RET   Z              ; quit c if ^C or Q
       CALL  NewLine
       LD    C,1            ; assume single register
       LD    A,(DE)         ; get 1st char
       CP    "A"
       RET   C              ; if less than 'A' then quit
       INC   DE
       LD    A,(DE)         ; get 2nd char
       CP    "C"            ; could be paired register?
       JR    C,.er1         ; no
       INC   C              ; yes, C = 2
.er1:  DEC   DE
       LD    HL,regnames
       LD    B,0            ; index = 0
.er_find:
       PUSH  BC
       PUSH  DE
       PUSH  HL
       CALL  strlcmp        ; compare input to name in list
       POP   HL
       POP   DE
       POP   BC
       JR    Z,.er_gotreg
       INC   HL            ; next reg name
       BIT   1,C           ; register pair?
       JR    Z,.er_next
       INC   HL            ; yes, next pair name
.er_next:
       INC   B
       LD    A,8           ; 8 register names
       CP    B
       RET   Z             ; quit nc if can't find register name
       JR    .er_find
.er_gotreg:
       LD    A,B
       BIT   1,C           ; register pair?
       JR    Z,.er_rev
       ADD   A             ; yes index * 2
       JR    .er_index
.er_rev:
       XOR   1             ; no, reverse register order
.er_index:
       PUSH  BC
       LD    C,A
       LD    B,0
       LD    HL,AF_Reg
       ADD   HL,BC         ; HL = register value
       POP   BC
       PUSH  HL            ; save register address
       CALL  WinPrtMsg
       DB    "   Value?",0
       LD    DE,linebuffer ; line input buffer
       LD    A,C
       SLA   A             ; A = max Hex chars (2 or 4)
       CALL  InputLine     ; input new value
       CALL  NZ,Hex2binHL  ; if value entered then convert to hex
       EX    DE,HL         ; DE = value entered
       POP   HL            ; restore register address
       JR    Z,.er_done    ; if no hex then quit
       LD    (HL),E        ; store register value
       BIT   1,C           ; register pair?
       JR    Z,.er_done
       INC   HL            ; yes, store upper register value
       LD    (HL),D
.er_done:
       XOR   A             ; nc = register edited
       RET


;----------------------------------------------------------
;    Get next Hexadecimal Digit from Buffer
;
;;  in: DE = char in Hexadecimal string
;; out: DE = next char
;        A = number 0.15 (0.9,A.F)
;       Carry set if not Hex
;
GetHexDigit
       LD    A,(DE)   ; get digit (ASCII char)
       INC   DE       ; point to next char
       CP    "a"
       JR    C,_ghdu  ; convert lower case to upper case
       AND   $DF
_ghdu  SUB   "0"      ; convert "0"."9" to 0.9
       RET   C
       CP    10       ; if 9 or less then done
       JR    C,_ghd1
       SUB   7        ; convert "A"."F" to 10.15
       RET   C
_ghd1  CP    16
       CCF
       RET

;----------------------------------------------------------
; Search Memory for Byte pattern or ASCII text string
;----------------------------------------------------------
;
SearchMem:
       LD    IX,SearchWindow
       CALL  OpenWindow       ; open 'Search' window
       CALL  WinPrtMsg
       db    "Start ",0
       CALL  InputAddress     ; request start address
       JR    NZ,_sm_startaddr ; NZ = got an address
       CP    $03              ; if ^C then quit
       RET   Z
       LD    HL,0             ; set address to $0000
_sm_startaddr:
       LD    (SrchAddr),HL    ; store start address
       CALL  WinPrtMsg
       db    CR,'Enter Hex Bytes or "string...',CR,"?",0
       LD    A,(IX+win_w)
       DEC   A                ; A = line input buffer size
       LD    DE,LineBuffer
       CALL  InputLine        ; enter search string to line buffer
       RET   Z
       LD    DE,LineBuffer    ; DE = line buffer
       LD    B,D
       LD    C,E              ; BC = search string (reusing line buffer)
_sm_hex:
       LD    A,2              ; 2 chars max
       CALL  Hex2bin          ; get hex byte from line buffer
       JR    Z,_sm_nothex
       LD    A,(IY+_number)
       LD    (BC),A           ; store byte in search buffer
       INC   BC
       JR    _sm_hex
_sm_nothex:
       LD    A,(DE)           ; get char
       CP    '"'              ; quote?
       JR    NZ,_gotstr       ; no
       INC   DE
_s_ch  LD    A,(DE)           ; get next char
       INC   DE
       CP    '"'              ; ending quote?
       JR    Z,_sm_hex        ; yes
       OR    A                ; end of line?
       JR    Z,_gotstr        ; yes
       LD    (BC),A           ; store char
       INC   BC
       JR    _s_ch            ; next char
_gotstr:
       LD    H,B
       LD    L,C              ; BC = end of search string
       LD    DE,LineBuffer    ; DE = start of search string
       OR    A
       SBC   HL,DE            ; string length = end - start
       RET   Z                ; quit if nothing to search for
       LD    B,H
       LD    C,L              ; BC = string length
       LD    HL,(SrchAddr)    ; HL = start address
       DEC   HL
_search_next:
       INC   HL
       CALL  NewLine
       CALL  WinPrtMsg
       DB    "searching...",$00
       JR    _search
_search_skip:
       LD    HL,$C000         ; skip over invalid RAM
_search:
       PUSH  BC
       PUSH  HL
       LD    DE,SearchStr
       CALL  strlcmp          ; compare string to memory
       POP   HL
       POP   BC
       JR    Z,_found_string
       INC   HL               ; no match, try next location
       LD    A,H
       OR    L
       JR    NZ,_search       ; until end of memory
       CALL  WinPrtMsg
       db    "done",0
       CALL  WAIT_KEY         ; wait for any key
       JR    _search_done
_found_string
       LD    DE,Vars
       CMPHLDE                ; below our variables?
       JR    C,.srch_low      ; yes, show it
       LD    DE,$C000
       CMPHLDE                ; in our variables or invalid RAM?
       JR    C,_search_skip   ; yes, skip to high ROM area
.srch_low:
       CALL  WinPrtMsg
       DB    "found at ",0
       CALL  PrtHexWord       ; print location of string found
       CALL  WAIT_KEY         ; wait for key
       CP    $0D              ; if RTN then continue search
       JR    Z,_search_next
_search_done
       RET


;----------------------------------------------------------
;   - B - edit Breakpoint
;----------------------------------------------------------
;
EditBreak:
       LD   IX,BreakWindow
       CALL OpenWindow
       CALL NewLine
       CALL ShowBreakpoints
       CALL WinPrtMsg
       db   CR,"  Number?",0
       CALL InputDigit
       RET  Z
       PUSH AF
       CALL WinPrtMsg
       db   " Address?",0
       CALL InputWord
       POP  BC                ; B = breakpoint number
       RET  Z
       EX   DE,HL             ; DE = address
       CALL CreateBreak
       JR   EditBreak         ; loop back to show/edit breakpoint

;----------------------------------------------------------
;         Remove breakpoint from target code
;
;  in: HL = breakpoint in array
;
RemoveBrk:
       LD    A,(HL)
       OR    A
       RET   Z       ; return if breakpoint not active
       PUSH  HL
       INC   HL
       LD    E,(HL)
       INC   HL
       LD    D,(HL)  ; DE = target address
       INC   HL
       LD    A,(HL)  ; get saved opcode
       LD    (DE),A  ; restore original opcode at target
       POP   HL
       RET

;----------------------------------------------------------
;     Remove all Breakpoints from target code
;
RemoveBreaks:
       LD    B,NUMBRKS      ; B = nuumber of breakpoints
       LD    HL,Breakpoints ; HL = breakpoint array
.next:
       CALL  RemoveBrk      ; remove the breakpoint
       LD    DE,brk.size
       ADD   HL,DE
       DJNZ  .next          ; next breakpoint
       RET

;----------------------------------------------------------
;          Find Breakpoint at our PC
;
;  in: DE = PC
;       z = breakpoint found
;      nz = not found
;
FindBreak:
       LD    B,NUMBRKS      ; B = nuumber of breakpoints
       LD    HL,Breakpoints ; HL = breakpoint array
.next: INC   HL             ; skip brk.status
       LD    C,(HL)
       INC   HL
       LD    A,(HL)         ; A,C = brk.address
       INC   HL
       INC   HL             ; skip brk.opcode
       CP    D
       JR    NZ,.notours
       LD    A,C            ; = our address?
       CP    E
       RET   Z              ; ret z if found
.notours:
       DJNZ  .next          ; next breakpoint
       RET

;----------------------------------------------------------
;                  Set Breakpoint
;----------------------------------------------------------
; Replaces target opcode with RST $38 ($FF).
; Saves original opcode so breakpoint can be removed later.
;
;  in: HL = breakpoint in array
;
SetBrk:
       LD    A,(HL)         ; A = status
       OR    A
       RET   Z              ; return if inactive
       PUSH  HL
       INC   HL
       LD    E,(HL)
       INC   HL
       LD    D,(HL)         ; DE = target address
       INC   HL
       LD    A,(DE)         ; get opcode at target address
       LD    (HL),A         ; save original opcode
       LD    A,$FF
       LD    (DE),A         ; replace opcode with RST $38
       POP   HL
       RET

;----------------------------------------------------------
;             Set all Breakpoints
;
SetBreakpoints:
       LD    B,NUMBRKS      ; B = number of breakpoints
       LD    HL,Breakpoints ; HL = 1st breakpoint
.next:
       CALL  SetBrk         ; set breakpoint
       LD    DE,brk.size    ; HL = next breakpoint in array
       ADD   HL,DE
       DJNZ  .next          ; next breakpoint
       RET


;----------------------------------------------------------
; Show registers
;
ShowRegs:
       LD    C,0          ; register index
_show_reg:
       LD    A,0          ; normal registers
       CALL  ShowRegPair  ; show register and its contents
       LD    A,C
       OR    A              ; AF?
       JR    NZ,_other_regs ; no,
; AF
       CALL  prt_space
       LD    A,'"'
       CALL  WinPrtChr
       LD    A,D
       CALL  prt_ascii    ; print contents of A as char
       LD    A,'"'
       CALL  WinPrtChr
       CALL  WinPrtMsg
       DB    "   ",0
       LD    A,E
       LD    DE,flagnames ; DE = flag names
       LD    B,8          ; 8 flag bits
_next_flag:
       RLA                ; Carry = flag bit
       PUSH  AF
       LD    A,(DE)       ; get flag name
       JR    C,_show_flag ; flag bit set?
       LD    A,"-"        ; no, show "-"
_show_flag:
       CALL  WinPrtChr       ; print flag name or space
       POP   AF
       INC   DE
       DJNZ  _next_flag   ; show next flag bit
       CALL  WinPrtMsg
       DB    "        ",0
       JR    ShowAltReg
; BC,DE,HL,IX,IY,SP
_other_regs:
       LD    B,5          ; B = number of bytes to show
       LD    A,C
       CP    6            ; SP?
       JR    NZ,reg_dump
       LD    B,6          ; yes, B = number of words
; show memory that register is pointing to
reg_dump
       PUSH  BC
       PUSH  DE
reg_dump_bytes:
       CALL  prt_space
reg_dump_byte:
       LD    A,C
       CP    6
       JR    Z,reg_dump_word
       LD    A,(DE)
       CALL  prt_byte           ; show target memory as hex byte
       JR    _next_reg_byte
reg_dump_word:
       LD    A,(DE)
       LD    L,A
       INC   DE
       LD    A,(DE)
       LD    H,A
       CALL  PrtHexWord
_next_reg_byte:
       INC   DE
       DJNZ  reg_dump_bytes
       POP   DE
       POP   BC
       LD    A,C
       CP    6
       JR    Z,_next_reg   ; no ascii if SP
       CALL  prt_space
reg_dm_asc
       LD    A,(DE)
       INC   DE
       CALL  prt_ascii     ; show target memory as ascii chars
       DJNZ  reg_dm_asc
       CALL  prt_space
       LD    A,C
       CP    4             ; has an alternate register?
       JR    NC,_next_reg  ; no
       CALL  prt_space
; AF', BC', DE', HL'
ShowAltReg:
       LD    A,1           ; alternate registers
       CALL  ShowRegPair   ; show alternate register
_next_reg:
       CALL  NewLine
       INC   C
       LD    A,C
       CP    7
       JP    NZ,_show_reg  ; next register
       RET

;---------------------------------------------------------
;              Show Code Disassembly
;
;  in: B = number of lines to lines to show
;
ShowCode:
       CALL  WinPrtMsg
       db    "PC ",0
       LD    HL,(PC_reg)
       LD    B,6           ; number of lines to disassemble
       JR    _ulines
_uline:
       CALL  WinPrtMsg
       DB    "   ",0
_ulines:
       CALL  unasm         ; show 1 line of unassembled code
       CALL  NewLine
       DJNZ  _uline
       RET


;----------------------------------------------------------
;   Show 16 bit Register and its contents
;
;;  in: DE = name array, C = index, A = register set
;; out: DE = register contents
;
ShowRegPair:
       LD    DE,regnames  ; register names
       LD    HL,AF_reg    ; register contents
       CP    1
       JR    NZ,_srp_index
       LD    HL,Alt_regs  ; alt register values
_srp_index
       LD    B,0
       RLC   C
       ADD   HL,BC        ; index into register value
       EX    DE,HL
       ADD   HL,BC        ; index into register names
       EX    DE,HL
       RRC   C
       LD    B,A          ; B = register set
       LD    A,(DE)
       INC   DE
       CALL  WinPrtChr       ; print upper register name
       LD    A,(DE)
       INC   DE
       CALL  WinPrtChr       ; print lower register name
       DJNZ  _srp_n
       LD    A,"'"        ; print "'" if alt reg
       CALL  WinPrtChr
_srp_n
       LD    E,(HL)       ; E = lower register value
       INC   HL
       LD    D,(HL)       ; D  = upper register value
       INC   HL
       EX    DE,HL
       CALL  prt_space
       CALL  PrtHexWord   ; print register value
       EX    DE,HL
       RET

;-------------------------------------------------------------
;            Trap dangerous instructions
;
; Instructions such as JR, JP, CALL are simulated to maintain
; control of code execution. Others such as HALT are simply
; ignored.
;
;  in: HL = target PC
;      DE = copy of instruction in sand box
;
; out:  Z = instruction was simulated, HL = new PC
;      NZ = instruction is safe to execute
;
Trap:  LD    DE,TraceOp
       LD    A,(DE)         ; get opcode
       CP    $76            ; 76 = HALT
       DEC   HL             ; if HALT then PC-1
       RET   Z
       INC   HL
       CP    $FB            ; FB = EI
       RET   Z              ; ignore EI
       CP    $ED            ; ED ?
       INC   DE             ; next addr
       JR    NZ,.do_jphl    ; no,
       LD    A,(DE)         ; yes, get next opcode byte
       CP    $46            ; ED46 = IM0
       RET   Z
       CP    $5E            ; ED5E = IM2
       RET   Z
       CP    $4D
       JP    Z,.retn        ; ED4D = RETI
       CP    $45
       JP    Z,.retn        ; ED45 = RETN
       RET
.do_jphl:
       CP    $E9            ; E9 = JP (HL)
       JR    NZ,.do_dd
       LD    HL,(HL_reg)    ; PC = HL
       RET                  ; return simulated
.do_dd:
       CP    $DD            ; DD instruction?
       JR    NZ,.do_fd      ; no,
       LD    A,(DE)         ; yes get next opcode byte
       CP    $E9            ; DDE9 = JP(IX)
       JR    NZ,.do_rst     ; no, absorb DD
       LD    HL,(IX_reg)    ; yes, PC = IX
       RET
.do_fd:
       CP    $FD            ; FD instruction?
       JR    NZ,.do_rst     ; no,
       LD    A,(DE)         ; yes, get next opcode byte
       CP    $E9            ; FDE9 = JP (IY)
       JR    NZ,.do_rst     ; no, absorb FD
       LD    HL,(IY_reg)    ; yes, PC = IY
       RET
.do_rst:
       LD    C,A
       CP    $FF            ; RST $38? (break instruction)
       RET   Z              ; yes, treat it as NOP
       AND   $C7
       CP    $C7            ; RST xx?
       LD    A,C
       JR    NZ,.do_ret     ; no,
; handle other RST $nn behaviors here!
       AND   $38
       LD    (DE),A         ; mask bits get low byte of target address
       INC   DE
       XOR   A              ; target address = $0008~$0030
       LD    (DE),A
       JP    .trace_call    ; treat as CALL
.do_ret:
       CP    $C9            ; RET
       JP    Z,.retn
       CP    $C3            ; JP
       JR    Z,.jump
       CP    $CD            ; CALL
       JP    Z,.trace_call
       CP    $18            ; JR
       JR    Z,.jpr
       LD    B,3
       CP    $38            ; JR C
       JR    Z,.jrcc
       DEC   B
       CP    $30            ; JR NC
       JR    Z,.jrcc
       DEC   B
       CP    $28            ; JR Z
       JR    Z,.jrcc
       DEC   B
       CP    $20            ; JR NZ
       JR    Z,.jrcc
       CP    $10            ; DJNZ
       JR    Z,.decjnz
       RLA
       RET   NC
       RLA
       RET   NC
       DEC   DE
       LD    A,(DE)         ; get opcode
       RRA
       RET   C              ; if bit 0 = 1 then return c
       RRA
       JR    C,.bit1
       RRA
       JP    C,.callcc
       JR    .retcc         ; RET cc
.bit1:
       RRA
       RET   C              ; if bit 2 = 1 then return
; JP CC
       CALL  Test_CC        ; condition met?
       JR    NC,.sim1
.jump: LD    HL,(TraceOp+1) ; yes, HL = dest address
.sim1: XOR   A              ; return simulated
       RET
; JR CC
.jrcc:
       LD    A,B
       CALL  Test_CC       ; condition met?
       JR    NC,.sim2      ; no,
.jpr:  LD    A,(DE)
       LD    C,A
       RLA                 ; calculate destination address
       SBC   A,A
       LD    B,A
       ADD   HL,BC         ; HL = destination address
.sim2: XOR   A
       RET                 ; return simulated
; DJNZ
.decjnz:
       LD    BC,(BC_reg)
       DEC   B
       LD    (BC_reg),BC
       JR    NZ,.jpr
       RET                 ; return simulated
; RET CC
.retcc:
       CALL  Test_CC       ; condition met?
       JR    NC,.sim2      ; no,
; RET
.retn: LD    HL,(SP_reg)
       LD    E,(HL)
       INC   HL            ; pop address off trace stack
       LD    D,(HL)
       INC   HL
       LD    (SP_reg),HL
       EX    DE,HL         ; HL = address
       XOR   A
       RET                 ; return simulated
; CALL CC
.callcc:
       CALL  Test_CC        ; conditon met?
       JR    C,.trace_call  ; yes,
       XOR   A
       RET                  ; no, return simulated

.trace_call:
       LD    A,(IY+KeyCode)
       CP    "."            ; stepping into?
       RET   NZ             ; no, return nz = trace over
.trace_into:
       EX    DE,HL          ; DE = return address
       LD    HL,(SP_reg)    ; HL = trace SP
       DEC   HL
       LD    (HL),D
       DEC   HL             ; push return address onto trace stack
       LD    (HL),E
       LD    (SP_reg),HL    ; update trace SP
       JP    .jump          ; return Z = trace into


;----------------------------------------------------------
;   Process Condition Code
;
;;  in: A = Condition Code
;; out: Carry set = Condition met
;
Test_CC:
       LD    BC,(AF_Reg)  ; C = Flags
       AND   7            ; 3 bits in CC
       LD    B,A          ; B = CC
       LD    A,C          ; A = Flags
       LD    C,B
       SRL   C            ; CC bits 2.1 = 00?
       JR    Z,L8CD4      ; yes,
       DEC   C            ; = 01?
       JR    Z,L8CDA      ; yes,
       DEC   C            ; = 10?
       JR    Z,L8CD8      ; yes,
       RRCA               ; CC = 11x, test S flag (bit 7)
L8CD4  RRCA               ; CC = 00x, test Z flag (bit 6)
       RRCA
       RRCA
       RRCA
L8CD8  RRCA               ; CC = 10x, test P/V flag (bit 2)
       RRCA
L8CDA  RRCA               ; CC = 01x, test C flag (bit 0)
       BIT   0,B          ; CC bit 0 = 1?
       RET   NZ           ; yes, CC (eg. Z)
       CCF                ; else NOT CC (eg. NZ)
       RET

prt_space:
       LD    A," "
       JP    WinPrtChr

;-----------------------------------------------
;     - Print Word in HL as 4 Hex digits -
;
PrtHexWord:
       LD    A,H             ; upper byte
       CALL  prt_byte
       LD    A,L             ; lower byte

;-----------------------------------------------
;      - Print Byte in A as 2 Hex digits -
;
prt_byte:
       PUSH  AF
       RRA
       RRA                   ; get upper nybble
       RRA
       RRA
       CALL  prt_hex         ; print it
       POP   AF              ; get lower nybble
prt_hex:
       AND   $0F             ; select nybble
       ADD   A,$30
       CP    $3A             ; A-F ?
       JR    C,_prt_hex_done
       ADD   A,7             ; yes,
_prt_hex_done
       JP    WinPrtChr

;----------------------------------------------------------
;  Convert Hex string to number in HL
;----------------------------------------------------------
;
;  in: DE = text
; out: HL = binary number
;
Hex2binHL:
       LD    A,4            ; 4 digits max
       CALL  Hex2bin
       LD    HL,(Number)    ; HL = number
       RET

;----------------------------------------------------------
;      Convert Decimal string to number in HL
;
;  in: DE = decimal string
; out: HL = 16 bit number
;       A = number of digits
;       Z = no digits
;
Dec2BinHL:
       PUSH  BC
       LD    HL,0
       LD    (IY+Digits),0 ; digits = 0
d2b_loop
       LD    A,(DE)        ; get char
       CALL  HEXorDEC
       JR    C,_d2b_end    ; end of number if not decimal
       INC   (IY+Digits)   ; digits +1
       INC   DE            ; DE-> next char
       SUB   "0"           ; digit -> 0.9
       ADD   HL,HL
       LD    B,H
       LD    C,L
       ADD   HL,HL
       ADD   HL,HL
       ADD   HL,BC         ; HL = HL*10
       LD    C,A
       LD    B,0
       ADD   HL,BC         ; add digit
       JR    d2b_loop
_d2b_end
       LD    (Number),HL
       LD    A,(IY+Digits) ; A = digit count
       OR    A
       POP   BC
       RET

;----------------------------------------------------------
;      Convert Hexadecimal String to Binary Number
;
;  in: DE = text buffer
;       A = max digits
; out: Number = binary number, Digits = digit count
;
Hex2bin:
       PUSH  BC
       PUSH  HL
       LD    B,A             ; B = max digits
       XOR   A
       LD    (IY+Digits),A    ; no digits
       LD    (IY+_number),A   ; 16 bit number = 0
       LD    (IY+_number+1),A
       LD    HL,Number       ; HL points to Number
_h2b_skip_space:
       LD    A,(DE)
       INC   DE              ; skip leading spaces
       CP    " "
       JR    Z,_h2b_skip_space
       CP    "$"
       JR    Z,_h2b_next     ; skip "$"
       DEC   DE
_h2b_next:
       LD    A,(DE)          ; get char
       CALL  UpperCase
       SUB   "0"             ; convert "0"."9" to 0.9
       JR    C,_H2b_done     ; if less than "0" then done
       CP    10
       JR    C,_h2b_gothex
       SUB   7               ; "A"."F" -> 10.15
       CP    10
       JR    C,_h2b_done     ; done if less than 10
       CP    16
       JR    NC,_h2b_done    ; done if greater than 15
_h2b_gothex:
       INC   DE
       INC   (IY+Digits)     ; digit count +1
       RLD
       INC   HL              ; number *16 + digit
       RLD
       DEC   HL
       DJNZ  _h2b_next       ; next char
_h2b_done:
       POP   HL
       POP   BC
       LD    A,(IY+Digits)  ; A = number of digits
       OR    A              ; Z if no digits
       RET


;----------------------------------------------------------
;             Hex or Decimal?
;
;;  in: A = char to test
;; out: Z = Hex
;      NC = decimal
;       C = neither
;
HEXorDEC
       CP    "$"
       RET   Z      ; Z = Hex
       CP    "0"
       RET   C
       CP    "9"+1
       CCF          ; NC = decimal
       RET


;----------------------------------------------------------
;      is Address inside visible Screen area?
;----------------------------------------------------------
;   in: HL = address
;  out:  C = in screen
;       NC = not in screen
;
; uses: A
;
inscreen:
       LD   A,H
       CP   $38            ; above $3800?
       RET  NC
       CP   $30            ; below $3000?
       JR   NC,.cmp1st
       CCF                 ; NC = not in screen
       RET
.cmp1st:
       JR   NZ,.not1stline ; $3000 to $30FF?
       LD   A,L
       CP   40             ; yes, on 1st line?
       CCF                 ; C = no, NC = yes
       RET
.not1stline:
       CP   $33
       RET  C              ; below $3300 = in screen
       CP   $34
       JR   NC,.color
       LD   A,L
       CP   $E8            ; C = below $33E8, NC = $33E8-$33FF
       RET
.color:
       JR   NZ,.not1stcolor
       LD   A,L
       CP   40             ; 1st line of color RAM?
       CCF                 ; C = no, NC = yes
       RET
.not1stcolor:
       CP   $37
       RET  C
       LD   A,L            ; $37E8-37FF?
       CP   $E8            ; C = no, NC = yes
       RET



; Screen memory is 1024 chars + 1024 colors = 40 bytes per line * 24 lines
; + 24 bytes spare at the end of each 1024 byte block.
;
; Screen buffer is 23 lines starting at 2nd line = 960 chars + 960 colors.
;
; Offset from screen memory to buffer memory is the sum of:-
;   Distance from screen RAM to buffer  = screen_save-$3000
;   - 1 line to start of char RAM in buffer, 2 lines to color RAM.
;   - 28 bytes between char RAM and color RAM.

CHAR_OFFSET  = scrn_save-$3000-40
COLOR_OFFSET = scrn_save-$3000-(40*2)-28

;-------------------------------------------
;        Read Byte from Memory
;-------------------------------------------
;
;  in: HL = address
;
; out:  A = byte
;
; if in screen then read from saved system
; screen.
;
PEEK:
       CALL inscreen
       JR   C,.in_screen
       LD   A,(HL)          ; load direct from memory
       RET
.in_screen:
       PUSH HL
       PUSH DE
       LD   DE,CHAR_OFFSET
       BIT  2,H             ; in color RAM?
       JR   Z,.chars
       LD   DE,COLOR_OFFSET ; yes
.chars:
       ADD  HL,DE           ; add offset to peek address
       POP  DE
       LD   A,(HL)          ; peek screen buffer
       POP  HL
       RET

;-------------------------------------------
;        Write Byte to Memory
;-------------------------------------------
;
;  in: HL = address
;       A = byte
; if in screen then write to saved system screen.
;
POKE:  PUSH AF
       CALL inscreen
       JR   C,.in_screen
       POP  AF
       LD   (HL),A          ; store direct to memory
       RET
.in_screen:
       POP  AF
       PUSH HL
       PUSH DE
       LD   DE,CHAR_OFFSET
       BIT  2,H             ; in color RAM?
       JR   Z,.chars
       LD   DE,COLOR_OFFSET ; yes
.chars:
       ADD  HL,DE
       POP  DE
       LD   (HL),A          ; store in screen buffer
       POP  HL
       RET


; show error message
Say_Error
       RET

;-------------------------------------------------------
;       Rediract RST $38 Vector to Trace Break
;-------------------------------------------------------
InitBreak:
     push    hl
     ld      a,$C3
     ld      (USRJMP),a       ; create JP instruction
     ld      HL,(USRADDR)
     ld      (UserAddr),HL    ; save current RST $38 vector
     ld      hl,Break
     ld      (USRADDR),HL     ; redirect to Trace Break
     pop     hl
     ret


;
; opcode name lists
;
opcode_names
       DB    $80+"L","D","I"
       DB    $80+"L","D","D"
       DB    $80+"L","D"
       DB    $80+"P","U","S","H"
       DB    $80+"P","O","P"
       DB    $80+"R","E","T","I"
       DB    $80+"R","E","T","N"
       DB    $80+"C","P","L"
       DB    $80+"C","A","L","L"
       DB    $80+"J","R"
       DB    $80+"J","P"
       DB    $80+"I","N","C"
       DB    $80+"D","E","C"
       DB    $80+"C","P","I"
       DB    $80+"C","P","D"
add_names
       DB    $80+"A","D","D"
       DB    $80+"A","D","C"
       DB    $80+"S","U","B"
       DB    $80+"S","B","C"
       DB    $80+"A","N","D"
       DB    $80+"X","O","R"
       DB    $80+"O","R"
       DB    $80+"C","P"
       DB    $80+"R","L","C","A"
       DB    $80+"R","L","A"
       DB    $80+"R","L","D"
       DB    $80+"E","X","X"
       DB    $80+"E","X"
       DB    $80+"R","R","C","A"
       DB    $80+"R","R","A"
       DB    $80+"R","R","D"
rlc_names
       DB    $80+"R","L","C"
       DB    $80+"R","R","C"
       DB    $80+"R","L"
       DB    $80+"R","R"
       DB    $80+"S","L","A"
       DB    $80+"S","R","A"
       DB    $80+"S","L","L"
srl_names
       DB    $80+"S","R","L"
       DB    $80+"B","I","T"
       DB    $80+"R","E","S"
       DB    $80+"S","E","T"
       DB    $80+"D","J","N","Z"
       DB    $80+"R","S","T"
       DB    $80+"R","C","A","L"
       DB    $80+"R","S","C","L"
       DB    $80+"N","O","P"
       DB    $80+"R","E","T"
       DB    $80+"N","E","G"
       DB    $80+"C","C","F"
       DB    $80+"S","C","F"
       DB    $80+"H","A","L","T"
       DB    $80+"D","I"
       DB    $80+"E","I"
       DB    $80+"I","M","0"
       DB    $80+"I","M","1"
       DB    $80+"I","M","2"
       DB    $80+"D","A","A"
       DB    $80+"I","N","I"
       DB    $80+"I","N","D"
       DB    $80+"I","N"
       DB    $80+"O","U","T"
       DB    $80+"O","T","I"
       DB    $80+"O","T","D"
       DB    $80
cc_names
       DB    $80+"Z"
       DB    $80+"N","Z"
       DB    $80+"N","C"
       DB    $80+"M"
       DB    $80+"P","O"
       DB    $80+"P","E"
       DB    $80+"P"
       DB    $80+"B","C"
       DB    $80+"D","E"
       DB    $80+"H","L"
       DB    $80+"S","P"
       DB    $80+"I",3             ; 3 = IX/IY
       DB    $80+"A","F"
r_names
       DB    $80+"(","I",3,1,")"   ; 3 = IX/IY, 1 = +d
       DB    $80+"B"
       DB    $80+"C"
       DB    $80+"D"
       DB    $80+"E"
       DB    $80+"H"
       DB    $80+"L"
       DB    $80+"(","H","L",")"
       DB    $80+"A"
       DB    $80+"I"
       DB    $80+"R"
       DB    $80+"(","I",3,")"     ; 3 = IX/IY
       DB    $80+"(","C",")"
       DB    $80+"(","S","P",")"
       DB    $80+"(","B","C",")"
       DB    $80+"(","D","E",")"
       DB    $80+"(",2,")"         ; 2 = nnnn
       DB    $80+2                 ; 2 = nnnn
       DB    $80

;----------------------------------------------
;          Opcode syntax tables
;----------------------------------------------

; low opcodes $00..
L6B5F  DB    $BC, $00   ; NOP
       DB    $0D, $1F   ; LD BC,nn
       DB    $0F, $96   ; LD (BC),A
       DB    $31, $00   ; INC BC
       DB    $31, $E0   ; INC B
       DB    $35, $E0   ; DEC B
       DB    $0D, $FF   ; LD B,n
       DB    $60, $00   ; RLCA
       DB    $71, $AD   ; ...
       DB    $41, $48
       DB    $0E, $DC
       DB    $35, $00
       DB    $32, $00
       DB    $36, $00
       DB    $0E, $1F
       DB    $74, $00
       DB    $AF, $E0
       DB    $0D, $3F
       DB    $0F, $B6
       DB    $31, $20
       DB    $32, $20
       DB    $36, $20
       DB    $0E, $3F
       DB    $64, $00
       DB    $2B, $E0
       DB    $41, $49
       DB    $0E, $DD
       DB    $35, $20
       DB    $32, $40
       DB    $36, $40
       DB    $0E, $5F
       DB    $78, $00
       DB    $28, $5F
       DB    $0D, $5F
       DB    $0F, $CA
       DB    $31, $40
       DB    $32, $60
       DB    $36, $60
       DB    $0E, $7F
       DB    $E8, $00
       DB    $28, $3F
       DB    $41, $4A
       DB    $0D, $5E
       DB    $35, $40
       DB    $32, $80
       DB    $36, $80
       DB    $0E, $9F
       DB    $20, $00
       DB    $28, $7F
       DB    $0D, $7F
       DB    $0F, $D6
       DB    $31, $60
       DB    $32, $A0
       DB    $36, $A0
       DB    $0E, $BF
       DB    $CC, $00
       DB    $2A, $1F
       DB    $41, $4B
       DB    $0E, $DE
       DB    $35, $60
       DB    $32, $C0
       DB    $36, $C0
       DB    $0E, $DF
       DB    $C8, $00
       DB    $FF, $FF

; high opcodes $C0..$FF
L6BE2:
       DB    $C0, $40
       DB    $15, $00
       DB    $2C, $5F
       DB    $2F, $E0
       DB    $24, $5F
       DB    $11, $00
       DB    $42, $DF
       DB    $B3, $E0
       DB    $C0, $20
       DB    $C0, $00
       DB    $2C, $3F
       DB    $BC, $00
       DB    $24, $3F
       DB    $27, $E0
       DB    $46, $DF
       DB    $B3, $E0
       DB    $C0, $60
       DB    $15, $20
       DB    $2C, $7F
       DB    $FB, $D6
       DB    $24, $7F
       DB    $11, $20
       DB    $4B, $E0
       DB    $B7, $E0
       DB    $C2, $00
       DB    $6C, $00
       DB    $2E, $1F
       DB    $F6, $DE
       DB    $26, $1F
       DB    $BC, $00
       DB    $4E, $DF
       DB    $BB, $E0
       DB    $C0, $A0
       DB    $15, $40
       DB    $2C, $BF
       DB    $73, $6A
       DB    $24, $BF
       DB    $11, $40
       DB    $53, $E0
       DB    $B3, $E0
       DB    $C0, $C0
       DB    $2E, $A0
       DB    $2C, $DF
       DB    $71, $2A
       DB    $24, $DF
       DB    $BC, $00
       DB    $57, $E0
       DB    $B3, $E0
       DB    $C0, $E0
       DB    $15, $A0
       DB    $2C, $FF
       DB    $D4, $00
       DB    $24, $FF
       DB    $11, $A0
       DB    $5B, $E0
       DB    $B3, $E0
       DB    $C0, $80
       DB    $0D, $6A
       DB    $2C, $9F
       DB    $D8, $00
       DB    $24, $9F
       DB    $BC, $00
       DB    $5F, $E0
       DB    $FF, $FF
       DB    $FF

; Opcodes with ED prefix
ED_table:
       DB    $F5, $FA
       DB    $FB, $4F
       DB    $4D, $48
       DB    $0F, $C8
       DB    $C4, $00
       DB    $1C, $00
       DB    $DC, $00
       DB    $0E, $F6
       DB    $F6, $ac
       DB    $FB, $50
       DB    $45, $48
       DB    $0D, $1E
       DB    $BC, $00
       DB    $18, $00
       DB    $BC, $00
       DB    $0F, $16
       DB    $F6, $3A
       DB    $FB, $51
       DB    $4D, $49
       DB    $0F, $C9
       DB    $BC, $00
       DB    $BC, $00
       DB    $E0, $00
       DB    $0E, $D7
       DB    $F6, $5A
       DB    $FB, $52
       DB    $45, $49
       DB    $0D, $3E
       DB    $BC, $00
       DB    $BC, $00
       DB    $E4, $00
       DB    $0E, $D8
       DB    $F6, $7A
       DB    $FB, $53
       DB    $4D, $4A
       DB    $0F, $CA
       DB    $BC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $7C, $00
       DB    $F6, $9A
       DB    $FB, $54
       DB    $45, $4A
       DB    $0D, $5E
       DB    $BC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $68, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $4D, $4B
       DB    $0F, $CB
       DB    $BC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $F6, $DA
       DB    $FB, $56
       DB    $45, $4B
       DB    $0D, $7E
       DB    $FF, $FF
       DB    $FF

LDI_table:
       DB    $04, $00
       DB    $38, $00
       DB    $EC, $00
       DB    $FC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $BC, $00
       DB    $08, $00
       DB    $3C, $00
       DB    $F0, $00
       DB    $00, $00
       DB    $FF, $FF
       DB    $FF

;----------------------------------------
;        Save system screen
;
save_screen:
       push  hl
       push  de
       push  bc
       LD    HL,$3000+40
       LD    DE,scrn_save
       LD    BC,1000-40
       LDIR                       ; save chars
       LD    HL,$3400+40
       LD    DE,scrn_save+(40*24)
       LD    BC,1000-40
       LDIR                       ; save colors
       pop   bc
       pop   de
       pop   hl
       RET

;----------------------------------------
;     Restore system screen
;
restore_screen:
       push  hl
       push  de
       push  bc
       LD    HL,scrn_save
       LD    DE,$3000+40
       LD    BC,1000-40
       LDIR                       ; restore chars
       LD    HL,scrn_save+(40*24)
       LD    DE,$3400+40
       LD    BC,1000-40
       LDIR                       ; restore colors
       pop   bc
       pop   de
       pop   hl
       RET


dflt_winaddrs:
       dw    $0000
       dw    $3000
       dw    $3800
       dw    $3A00

debug_msg:
       db    CR
       db    "   T   Trace    (step through code)",CR
       db    "   P   Proceed    (CALL subroutine)",CR
       db    "   G   Go           (JP to address)",CR
       db    "   B   Breakpoints",CR
       db    "   R   Registers",CR
       db    CR
       db    "   D   Dump memory",CR
       db    "   E   Edit memory",CR
       db    "   F   Fill memory",CR
       db    "   M   Move memory",CR
       db    "   S   Search",CR
       db    "   U   Unassemble",CR
       db    CR
       db    "   L   Load file",CR
       db    "   W   Write file",CR
       db    CR
       db    CR
       db    " SPACE show screen",CR
       db    "   ?   help",CR
       db    "   Q   Quit",0

debug_help_msg:
       db    "         Welcome to AquBug!",CR
       db    CR
       db    "With this machine code monitor you can",CR
       db    "view or edit memory, step through code",CR
       db    "in RAM or ROM, set breakpoints in RAM,",CR
       db    "load & save binary files etc.",CR
       db    CR,CR
       db    "        --- General Help ---",CR
       db    CR
       db    198,"Q quits most operations",CR
       db    CR
       db    198,"SPACE toggles between debug screen",CR
       db    " and system/output screen (view only)",CR
       db    CR
       db    198,"If an address is requested, you may",CR
       db    " press RTN to use the default or last",CR
       db    " entered address",CR
       db    CR
       db    198,"Press ? for help on any screen",CR
       db    0

trace_msg:
       db    "  RTN  trace over (single step)",CR
       db    "   .   trace into (step into CALL)",CR
       db    "   ,   skip current instruction",CR
       db    "   ;   run until RET",CR
       db    "   :   run until Breakpoint",CR
       db    "   ",7,"   previous address (PC-1)",CR
       db    "   T   set Trace address (=PC)",CR
       db    "   P   Proceed @addr (run until RET)",CR
       db    "   G   Go @address (run until break)",CR
       db    CR
       db    "   B   Breakpoints",CR
       db    "SZHVNC toggle CPU flag",CR
       db    "   R   edit Register",CR
       db    "  1-4  memory window M1-M4",CR
       db    "  - =  memory window addr - +",CR
       db    CR
       db    " DEFMU Dump/Edit/Fill/Move/UnASM",CR
       db    CR
       db    " SPACE show screen",CR
       db    "   Q   exit Trace",0

break_msg:
       db    " 1-8  set/clr breakpoint",CR
       db    "  C   Clear all breakpoints",CR
       db    CR
       db    "  Q   exit",0

edit_msg:
       db    CR
       db    "  RTN     = next address",CR
       db    "   ",7,"      = previous address",CR
       db    " nn nn .. = enter hex bytes",CR
       db    ' "abc..   = enter ASCII string',CR
       db    " @nnnn    = set address",CR
       db    "   Q      = exit",CR,CR
       db    0

dump_msg:
       db    " RTN   next page",CR
       db    "  ",7,"    previous page",CR
       db    "  C    list continuously",CR
       db    "  D    Dump address",CR
       db    CR
       db    "SPACE  show screen",CR
       db    "  Q    exit",0

unasm_msg:
       db    " RTN   next page",CR
       db    "  C    list continuously",CR
       db    "  U    Unassemble address",CR
       db    CR
       db    "SPACE  show screen",CR
       db    "  Q    exit",0

; code executed when tracing. 5 bytes copied to RAM
; at TraceOp
TraceCode:
    DB    0,0,0,0       ; target instruction
    RST   $38           ; break


DebugWindow:
      db   (1<<WA_BORDER)|(1<<WA_TITLE)|(1<<WA_CENTER) ; attributes
      db   WHITE*16+DKBLUE              ; text colors
      db   WHITE*16+BLUE                ; border colors
      db   1,2,38,22                    ; x,y,w,h
      dw   debug_title                  ; title
debug_title:
      db   " A Q U B U G ",0

DebugHelpWindow:
      db   (1<<WA_BORDER)               ; attributes
      db   BLACK*16+CYAN                ; text colors
      db   BLACK*16+CYAN                ; border colors
      db   1,2,38,22                    ; x,y,w,h
      dw   0                            ; title

TraceWindow:
      db   (1<<WA_BORDER)|(1<<WA_TITLE)|(1<<WA_CENTER) ; attributes
      db   GREEN*16+BLACK               ; text colors
      db   WHITE*16+BLUE                ; border colors
      db   1,2,38,22                    ; x,y,w,h
      dw   aqubug_title                 ; title
aqubug_title:
      db   " T R A C E ",0

TraceHelpWindow:
      db   (1<<WA_BORDER)               ; attributes
      db   BLACK*16+CYAN                ; text colors
      db   BLACK*16+CYAN                ; border colors
      db   2,3,36,20                    ; x,y,w,h
      dw   0                            ; title

EditWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER) ; attributes
      db   YELLOW*16+BLACK              ; text colors
      db   WHITE*16+BLUE                ; border colors
      db   3,4,34,18                    ; x,y,w,h
      dw   edit_title                   ; title
edit_title:
      db   "Edit Memory",0

BreakWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER) ; attributes
      db   YELLOW*16+BLACK              ; text colors
      db   WHITE*16+BLUE                ; border colors
      db   9,7,21,13                    ; x,y,w,h
      dw   .title                       ; title
.title:
      db   "Edit Breakpoint",0

SearchWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   4,7,32,13
      dw   search_title
search_title:
      db   "Search Memory",0

UNASMLINES = 22
UnasmWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)|(1<<WA_CENTER)
      db   WHITE*16+DKBLUE
      db   WHITE*16+BLUE
      db   1,2,38,UNASMLINES
      dw   unasmtitle
unasmtitle:
      db   "Unassemble",0

UnasmAddrWindow:
      db   (1<<WA_BORDER)|(1<<WA_TITLE)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   13,11,13,1     ; x,y,w,h
      dw   unasmtitle


AddrWindow:
      db   (1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   13,11,13,1     ; x,y,w,h
      dw   0

DumpAddrWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   13,11,13,1     ; x,y,w,h
      dw   dm_title

DumpWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)|(1<<WA_CENTER)
      db   WHITE*16+DKBLUE
      db   WHITE*16+BLUE
      db   1,2,38,DUMPLINES
      dw   dm_title
dm_title:
      db   "Dump Memory",0

DumpHelpWindow:
      db   (1<<WA_BORDER)               ; attributes
      db   BLACK*16+CYAN                ; text colors
      db   BLACK*16+CYAN                ; border colors
      db   7,9,25,7                     ; x,y,w,h
      dw   0                            ; title

UnAsmHelpWindow:
      db   (1<<WA_BORDER)               ; attributes
      db   BLACK*16+CYAN                ; text colors
      db   BLACK*16+CYAN                ; border colors
      db   7,9,25,6                     ; x,y,w,h
      dw   0                            ; title

FillMemWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+DKRED
      db   8,10,24,5     ; x,y,w,h
      dw   fm_title
fm_title:
      db   "Fill Memory",0

JumpWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK ; text
      db   WHITE*16+BLUE   ; border
      db   13,12,13,1      ; x,y,w,h
      dw   jmp_title
jmp_title:
      db   " Trace ",0

MoveMemWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+DKRED
      db   9,10,22,5     ; x,y,w,h
      dw   mm_title
mm_title:
      db   "Move Memory",0

SetMemWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   11,15,16,1     ; x,y,w,h
      dw   sm_title
sm_title:
      db   "Memory Window",0

LoadFileWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   BLACK*16+GREY
      db   BLACK*16+GREY
      db   1,2,38,22     ; x,y,w,h
      dw   lf_title
lf_title:
      db   "Load File",0

WriteFileWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   BLACK*16+GREY
      db   BLACK*16+GREY
      db   1,2,38,22     ; x,y,w,h
      dw   wf_title
wf_title:
      db   "Write File",0

EditRegWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   10,9,15,5     ; x,y,w,h
      dw   reg_title
reg_title:
      db   "Edit Register",0

RegisterWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+BLUE
      db   1,8,38,10     ; x,y,w,h
      dw   .title
.title:
      db   "Registers",0

GoWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+DKRED
      db   12,11,13,1     ; x,y,w,h
      dw   .title
.title:
      db   " Go ",0

ProcWindow:
      db   (1<<WA_TITLE)|(1<<WA_BORDER)
      db   YELLOW*16+BLACK
      db   WHITE*16+DKRED
      db   12,11,13,1     ; x,y,w,h
      dw   .title
.title:
      db   " Proceed ",0