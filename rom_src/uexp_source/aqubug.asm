;=============================================================
;             Aquarius Z80 Monitor/Debugger
;=============================================================

NUMBRKS    = 8        ; number of breakpoints
LINESIZE   = 37       ; length of line input buffer

; breakpoint array element
STRUCTURE brk,0
    BYTE  brk.status  ; 1 = active, 0 = inactive
    WORD  brk.addr    ; address of target code
    BYTE  brk.opcode  ; original opcode at target address
ENDSTRUCT brk

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
