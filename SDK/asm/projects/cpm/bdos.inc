;-----------------------------------------------------------------------------
; bdos.inc - Basic Disk Operating System
;-----------------------------------------------------------------------------
FBASE:  jp      _fbase1

;-----------------------------------------------------------------------------
; BDOS error table.
;-----------------------------------------------------------------------------
BADSCTR:defw    ERROR1          ; bad sector on read or write.
BADSLCT:defw    ERROR2          ; bad disk select.
RODISK: defw    ERROR3          ; disk is read only.
ROFILE: defw    ERROR4          ; file is read only.

;-----------------------------------------------------------------------------
; Entry into BDOS. (de) or (E) are the parameters passed. The
; function number desired is in register (C).
;-----------------------------------------------------------------------------
_fbase1:
        ex      de,hl           ; save the (de) parameters.
        ld      (PARAMS),hl
        ex      de,hl
        ld      a,e             ; and save register (E) in particular.
        ld      (EPARAM),a
        ld      hl,0
        ld      (STATUS),hl     ; clear return status.
        add     hl,sp
        ld      (USRSTACK),hl   ; save users stack pointer.
        ld      sp,STKAREA      ; and set our own.
        xor     a               ; clear auto select storage space.
        ld      (AUTOFLAG),a
        ld      (AUTO),a
        ld      hl,GOBACK       ; set return address.
        push    hl
        ld      a,c             ; get function number.
        cp      NFUNCTS         ; valid function number?
        ret     nc
        ld      c,e             ; keep single register function here.
        ld      hl,FUNCTNS      ; now look thru the function table.
        ld      e,a
        ld      d,0             ; (de)=function number.
        add     hl,de
        add     hl,de           ; (hl)=(start of table)+2*(function number).
        ld      e,(hl)
        inc     hl
        ld      d,(hl)          ; now (de)=address for this function.
        ld      hl,(PARAMS)     ; retrieve parameters.
        ex      de,hl           ; now (de) has the original parameters.
        jp      (hl)            ; execute desired function.

;-----------------------------------------------------------------------------
; BDOS function jump table.
;-----------------------------------------------------------------------------
NFUNCTS equ     41              ; number of functions in following table.
FUNCTNS:defw    WBOOT,GETCON,OUTCON,GETRDR,PUNCH,LIST,DIRCIO,GETIOB
        defw    SETIOB,PRTSTR,RDBUFF,GETCSTS,GETVER,RSTDSK,SETDSK,OPENFIL
        defw    CLOSEFIL,GETFST,GETNXT,DELFILE,READSEQ,WRTSEQ,FCREATE
        defw    RENFILE,GETLOG,GETCRNT,PUTDMA,GETALOC,WRTPRTD,GETROV,SETATTR
        defw    GETPARM,GETUSER,RDRANDOM,WTRANDOM,FILESIZE,SETRAN,LOGOFF,RTN
        defw    RTN,WTSPECL

;-----------------------------------------------------------------------------
; BDOS error message section.
;-----------------------------------------------------------------------------
ERROR1: ld      hl,BADSEC       ; bad sector message.
        call    PRTERR          ; print it and get a 1 char responce.
        cp      CTRL_C          ; re-boot request (control-c)?
        jp      z,0             ; yes.
        ret                     ; no, return to retry i/o function.

ERROR2: ld      hl,BADSEL       ; bad drive selected.
        jp      ERROR5

ERROR3: ld      hl,DISKRO       ; disk is read only.
        jp      ERROR5

ERROR4: ld      hl,FILERO       ; file is read only.

ERROR5: call    PRTERR
        jp      0               ; always reboot on these errors.

BDOSERR:defb    'Bdos Err On '
BDOSDRV:defb    ' : $'
BADSEC: defb    'Bad Sector$'
BADSEL: defb    'Select$'
FILERO: defb    'File '
DISKRO: defb    'R/O$'

; Print BDOS error message.
PRTERR: push    hl              ; save second message pointer.
        call    OUTCRLF         ; send (cr)(lf).
        ld      a,(ACTIVE)      ; get active drive.
        add     a,'A'           ; make ascii.
        ld      (BDOSDRV),a     ; and put in message.
        ld      bc,BDOSERR      ; and print it.
        call    PRTMESG
        pop     bc              ; print second message line now.
        call    PRTMESG

;-----------------------------------------------------------------------------
; Get an input character. We will check our 1 character
; buffer first. This may be set by the console status routine.
;-----------------------------------------------------------------------------
GETCHAR:ld      hl,CHARBUF      ; check character buffer.
        ld      a,(hl)          ; anything present already?
        ld      (hl),0          ; ...either case clear it.
        or      a
        ret     nz              ; yes, use it.
        jp      CONIN           ; nope, go get a character responce.

;-----------------------------------------------------------------------------
; Input and echo a character.
;-----------------------------------------------------------------------------
GETECHO:call    GETCHAR         ; input a character.
        call    CHKCHAR         ; carriage control?
        ret     c               ; no, a regular control char so don't echo.
        push    af              ; ok, save character now.
        ld      c,a
        call    OUTCON          ; and echo it.
        pop     af              ; get character and return.
        ret

;-----------------------------------------------------------------------------
; Check character in (A). Set the zero flag on a carriage
; control character and the carry flag on any other control
; character.
;-----------------------------------------------------------------------------
CHKCHAR:cp      CR              ; check for carriage return, line feed, backspace,
        ret     Z               ; or a tab.
        cp      LF
        ret     Z
        cp      TAB
        ret     Z
        cp      BS
        ret     Z
        cp      ' '             ; other control char? Set carry flag.
        ret

;-----------------------------------------------------------------------------
; Check the console during output. Halt on a control-s, then
; reboot on a control-c. If anything else is ready, clear the
; zero flag and return (the calling routine may want to do
; something).
;-----------------------------------------------------------------------------
CKCONSOL:
        ld    A,(CHARBUF)       ; check buffer.
        or      A               ; if anything, just return without checking.
        jp      nz,CKCON2
        call    CONST           ; nothing in buffer. Check console.
        and     01H             ; look at bit 0.
        ret     Z               ; return if nothing.
        call    CONIN           ; ok, get it.
        cp      CTRL_S          ; if not control-s, return with zero cleared.
        jp      nz,CKCON1
        call    CONIN           ; halt processing until another char
        cp      CTRL_C          ; is typed. Control-c?
        jp      Z,0             ; yes, reboot now.
        xor     A               ; no, just pretend nothing was ever ready.
        ret
CKCON1: ld      (CHARBUF),A     ; save character in buffer for later processing.
CKCON2: ld      A,1             ; set (A) to non zero to mean something is ready.
        ret

;-----------------------------------------------------------------------------
; Output (C) to the screen. If the printer flip-flop flag is set, we will
; send character to printer also. The console will be checked in the process.
;-----------------------------------------------------------------------------
OUTCHAR:ld      A,(OUTFLAG)     ;check output flag.
        or      A               ;anything and we won't generate output.
        jp      nz,OUTCHR1
        push    bc
        call    CKCONSOL        ;check console (we don't care whats there).
        pop     bc
        push    bc
        call    CONOUT          ;output (C) to the screen.
        pop     bc
        push    bc
        ld      A,(PRTFLAG)     ;check printer flip-flop flag.
        or      A
        call    nz,LIST         ;print it also if non-zero.
        pop     bc
OUTCHR1:ld      A,C             ;update cursors position.
        ld      hl,CURPOS
        cp      DEL             ;rubouts don't do anything here.
        ret     Z
        inc     (hl)            ;bump line pointer.
        cp      ' '             ;and return if a normal character.
        ret     nc
        dec     (hl)            ;restore and check for the start of the line.
        ld      A,(hl)
        or      A
        ret     Z               ;ingnore control characters at the start of the line.
        ld      A,C
        cp      BS              ;is it a backspace?
        jp      nz,OUTCHR2
        dec     (hl)            ;yes, backup pointer.
        ret
OUTCHR2:cp      LF              ;is it a line feed?
        ret     nz              ;ignore anything else.
        ld      (hl),0          ;reset pointer to start of line.
        ret

;-----------------------------------------------------------------------------
; Output (A) to the screen. If it is a control character (other than
; carriage control), use ^x format.
;-----------------------------------------------------------------------------
SHOWIT: ld      A,C
        call    CHKCHAR         ;check character.
        jp      nc,OUTCON       ;not a control, use normal output.
        push    af
        ld      C,'^'           ;for a control character, preceed it with '^'.
        call    OUTCHAR
        pop     af
        or      '@'             ;and then use the letter equivelant.
        ld      C,A

;-----------------------------------------------------------------------------
; [ 2] OUTCON - Function to output (C) to the console device and expand tabs
;               if necessary.
;-----------------------------------------------------------------------------
OUTCON: ld      A,C
        cp      TAB             ;is it a tab?
        jp      nz,OUTCHAR      ;use regular output.
OUTCON1:ld      C,' '           ;yes it is, use spaces instead.
        call    OUTCHAR
        ld      A,(CURPOS)      ;go until the cursor is at a multiple of 8

        and     07H             ;position.
        jp      nz,OUTCON1
        ret

;-----------------------------------------------------------------------------
; Echo a backspace character. Erase the prevoius character on the screen.
;-----------------------------------------------------------------------------
BACKUP: call    BACKUP1         ;backup the screen 1 place.
        ld      C,' '           ;then blank that character.
        call    CONOUT
BACKUP1:ld      C,BS            ;then back space once more.
        jp      CONOUT

;-----------------------------------------------------------------------------
; Signal a deleted line. Print a '#' at the end and start over.
;-----------------------------------------------------------------------------
NEWLINE:ld      C,'#'
        call    OUTCHAR         ;print this.
        call    OUTCRLF         ;start new line.
NEWLN1: ld      A,(CURPOS)      ;move the cursor to the starting position.
        ld      hl,STARTING
        cp      (hl)
        ret     nc              ;there yet?
        ld      C,' '
        call    OUTCHAR         ;nope, keep going.
        jp      NEWLN1

;-----------------------------------------------------------------------------
; Output a (cr) (lf) to the console device (screen).
;-----------------------------------------------------------------------------
OUTCRLF:ld      C,CR
        call    OUTCHAR
        ld      C,LF
        jp      OUTCHAR

;-----------------------------------------------------------------------------
; Print message pointed to by (bc). It will end with a '$'.
;-----------------------------------------------------------------------------
PRTMESG:ld      A,(bc)          ;check for terminating character.
        cp      '$'
        ret     z
        inc     bc
        push    bc              ;otherwise, bump pointer and print it.
        ld      c,a
        call    OUTCON
        pop     bc
        jp      PRTMESG

;-----------------------------------------------------------------------------
; [10] RDBUFF - Function to execute a buffered read.
;-----------------------------------------------------------------------------
RDBUFF: ld      A,(CURPOS)      ;use present location as starting one.
        ld      (STARTING),A
        ld      hl,(PARAMS)     ;get the maximum buffer space.
        ld      C,(hl)
        inc     hl              ;point to first available space.
        push    hl              ;and save.
        ld      B,0             ;keep a character count.
RDBUF1: push    bc
        push    hl
RDBUF2: call    GETCHAR         ;get the next input character.
        and     $7F             ;strip bit 7.
        pop     hl              ;reset registers.
        pop     bc
        cp      CR              ;en of the line?
        jp      Z,RDBUF17
        cp      LF
        jp      Z,RDBUF17
        cp      BS              ;how about a backspace?
        jp      nz,RDBUF3
        ld      A,B             ;yes, but ignore at the beginning of the line.
        or      A
        jp      Z,RDBUF1
        dec     B               ;ok, update counter.
        ld      A,(CURPOS)      ;if we backspace to the start of the line,
        ld      (OUTFLAG),A     ;treat as a cancel (control-x).
        jp      RDBUF10
RDBUF3: cp      DEL             ;user typed a rubout?
        jp      nz,RDBUF4
        ld      A,B             ;ignore at the start of the line.
        or      A
        jp      Z,RDBUF1
        ld      A,(hl)          ;ok, echo the prevoius character.
        dec     B               ;and reset pointers (counters).
        dec     hl
        jp      RDBUF15
RDBUF4: cp      CTRL_E          ;physical end of line?
        jp      nz,RDBUF5
        push    bc              ;yes, do it.
        push    hl
        call    OUTCRLF
        xor     A               ;and update starting position.
        ld      (STARTING),A
        jp      RDBUF2
RDBUF5: cp      CTRL_P          ;control-p?
        jp      nz,RDBUF6
        push    hl              ;yes, flip the print flag filp-flop byte.
        ld      hl,PRTFLAG
        ld      A,1             ;PRTFLAG=1-PRTFLAG
        sub     (hl)
        ld      (hl),A
        pop     hl
        jp      RDBUF1
RDBUF6: cp      CTRL_X          ;control-x (cancel)?
        jp      nz,RDBUF8
        pop     hl
RDBUF7: ld      A,(STARTING)    ;yes, backup the cursor to here.
        ld      hl,CURPOS
        cp      (hl)
        jp      nc,RDBUFF       ;done yet?
        dec     (hl)            ;no, decrement pointer and output back up one space.
        call    BACKUP
        jp      RDBUF7
RDBUF8: cp      CTRL_U          ;cntrol-u (cancel line)?
        jp      nz,RDBUF9
        call    NEWLINE         ;start a new line.
        pop     hl
        jp      RDBUFF
RDBUF9: cp      CTRL_R          ;control-r?
        jp      nz,RDBUF14
RDBUF10:push    bc              ;yes, start a new line and retype the old one.
        call    NEWLINE
        pop     bc
        pop     hl
        push    hl
        push    bc
RDBUF11:ld      A,B             ;done whole line yet?
        or      A
        jp      Z,RDBUF12
        inc     hl              ;nope, get next character.
        ld      C,(hl)
        dec     B               ;count it.
        push    bc
        push    hl
        call    SHOWIT          ;and display it.
        pop     hl
        pop     bc
        jp      RDBUF11
RDBUF12:push    hl              ;done with line. If we were displaying
        ld      A,(OUTFLAG)     ;then update cursor position.
        or      A
        jp      Z,RDBUF2
        ld      hl,CURPOS       ;because this line is shorter, we must
        sub     (hl)            ;back up the cursor (not the screen however)
        ld      (OUTFLAG),A     ;some number of positions.
RDBUF13:call    BACKUP          ;note that as long as (OUTFLAG) is non
        ld      hl,OUTFLAG      ;zero, the screen will not be changed.
        dec     (hl)
        jp      nz,RDBUF13
        jp      RDBUF2          ;now just get the next character.

        ; Just a normal character, put this in our buffer and echo.
RDBUF14:inc     hl
        ld      (hl),A          ;store character.
        inc     B               ;and count it.
RDBUF15:push    bc
        push    hl
        ld      C,A             ;echo it now.
        call    SHOWIT
        pop     hl
        pop     bc
        ld      A,(hl)          ;was it an abort request?
        cp      CTRL_C          ;control-c abort?
        ld      A,B
        jp      nz,RDBUF16
        cp      1               ;only if at start of line.
        jp      Z,0
RDBUF16:cp      C               ;nope, have we filled the buffer?
        jp      C,RDBUF1
RDBUF17:pop     hl              ;yes end the line and return.
        ld      (hl),B
        ld      C,CR
        jp      OUTCHAR         ;output (cr) and return.

;-----------------------------------------------------------------------------
; [ 1] GETCON - Function to get a character from the console device.
;-----------------------------------------------------------------------------
GETCON: call    GETECHO         ;get and echo.
        jp      SETSTAT         ;save status and return.

;-----------------------------------------------------------------------------
; [ 3] GETRDR - Function to get a character from the tape reader device.
;-----------------------------------------------------------------------------
GETRDR: call    READER          ;get a character from reader, set status and return.
        jp      SETSTAT

;-----------------------------------------------------------------------------
; [ 6] DIRCIO - Function to perform direct console i/o. If (C) contains (FF)
;               then this is an input request. If (C) contains (FE) then this
;               is a status request. Otherwise we are to output (C).
;-----------------------------------------------------------------------------
DIRCIO: ld      A,C             ;test for (FF).
        inc     A
        jp      Z,DIRC1
        inc     A               ;test for (FE).
        jp      Z,CONST
        jp      CONOUT          ;just output (C).
DIRC1:  call    CONST           ;this is an input request.
        or      A
        jp      Z,GOBACK1       ;not ready? Just return (directly).
        call    CONIN           ;yes, get character.
        jp      SETSTAT         ;set status and return.

;-----------------------------------------------------------------------------
; [ 7] GETIOB - Function to return the i/o byte.
;-----------------------------------------------------------------------------
GETIOB: ld      A,(IOBYTE)
        jp      SETSTAT

;-----------------------------------------------------------------------------
; [ 8] SETIOB - Function to set the i/o byte.
;-----------------------------------------------------------------------------
SETIOB: ld      hl,IOBYTE
        ld      (hl),C
        ret

;-----------------------------------------------------------------------------
; [ 9] PRTSTR - Function to print the character string pointed to by (de)
;               on the console device. The string ends with a '$'.
;-----------------------------------------------------------------------------
PRTSTR: ex      de,hl
        ld      c,l
        ld      b,h             ;now (bc) points to it.
        jp      PRTMESG

;-----------------------------------------------------------------------------
; [11] GETCSTS - Function to interrogate the console device.
;-----------------------------------------------------------------------------
GETCSTS:
        call    CKCONSOL
        ; fall-through into SETSTAT

;-----------------------------------------------------------------------------
; Get here to set the status and return to the cleanup
; section. Then back to the user.
;-----------------------------------------------------------------------------
SETSTAT:ld      (STATUS),A
RTN:    ret

;-----------------------------------------------------------------------------
; Set the status to 1 (read or write error code).
;-----------------------------------------------------------------------------
IOERR1: ld      A,1
        jp      SETSTAT

;-----------------------------------------------------------------------------
; Variables used by BDOS
;-----------------------------------------------------------------------------
OUTFLAG:  defb  0               ; output flag (non zero means no output).
STARTING: defb  2               ; starting position for cursor.
CURPOS:   defb  0               ; cursor position (0=start of line).
PRTFLAG:  defb  0               ; printer flag (control-p toggle). List if non zero.
CHARBUF:  defb  0               ; single input character buffer.
USERNO:   defb  0               ; current user number.
ACTIVE:   defb  0               ; currently active drive.
PARAMS:   defw  0               ; save (de) parameters here on entry.
STATUS:   defw  0               ; status returned from BDOS function.

;-----------------------------------------------------------------------------
; Stack area for BDOS calls.
;-----------------------------------------------------------------------------
USRSTACK:
        defw 0                  ; save users stack pointer here.
        defs 48
STKAREA equ     $               ; end of stack area.

;-----------------------------------------------------------------------------
; Select error occured, jump to error routine.
;-----------------------------------------------------------------------------
SLCTERR:
        ld      hl,BADSLCT
        ; fall-through into JUMPHL

;-----------------------------------------------------------------------------
; Jump to (hl) indirectly.
;-----------------------------------------------------------------------------
JUMPHL: ld      e,(hl)
        inc     hl
        ld      d,(hl)          ;now (de) contain the desired address.
        ex      de,hl
        jp      (hl)

;-----------------------------------------------------------------------------
; Block move. (de) to (hl), (C) bytes total.
;-----------------------------------------------------------------------------
DE2HL:  inc     C               ;is count down to zero?
DE2HL1: dec     C
        ret     Z               ;yes, we are done.
        ld      A,(de)          ;no, move one more byte.
        ld      (hl),A
        inc     de
        inc     hl
        jp      DE2HL1          ;and repeat.

;-----------------------------------------------------------------------------
;   Shift (hl) right (C) bits.
;-----------------------------------------------------------------------------
SHIFTR: inc     C
SHIFTR1:dec     C
        ret     Z
        ld      A,H
        or      A
        RRA
        ld      H,A
        ld      A,L
        RRA
        ld      L,A
        jp      SHIFTR1

;-----------------------------------------------------------------------------
;   Shift (hl) left (C) bits.
;-----------------------------------------------------------------------------
SHIFTL: inc     C
SHIFTL1:dec     C
        ret     Z
        ADD     hl,hl           ;shift left 1 bit.
        jp      SHIFTL1

;-----------------------------------------------------------------------------
;   Routine to set a bit in a 16 bit value contained in (bc).
; The bit set depends on the current drive selection.
;-----------------------------------------------------------------------------
SETBIT: push    bc              ;save 16 bit word.
        ld      A,(ACTIVE)      ;get active drive.
        ld      C,A
        ld      hl,1
        call    SHIFTL          ;shift bit 0 into place.
        pop     bc              ;now 'or' this with the original word.
        ld      A,C
        or      L
        ld      L,A             ;low byte done, do high byte.
        ld      A,B
        or      H
        ld      H,A
        ret

;-----------------------------------------------------------------------------
;   Extract the write protect status bit for the current drive.
; The result is returned in (A), bit 0.
;-----------------------------------------------------------------------------
GETWPRT:ld      hl,(WRTPRT)     ;get status bytes.
        ld      A,(ACTIVE)      ;which drive is current?
        ld      C,A
        call    SHIFTR          ;shift status such that bit 0 is the
        ld      A,L             ;one of interest for this drive.
        and     01H             ;and isolate it.
        ret

;-----------------------------------------------------------------------------
; [28] WRTPRTD - Function to write protect the current disk.
;-----------------------------------------------------------------------------
WRTPRTD:ld      hl,WRTPRT       ;point to status word.
        ld      C,(hl)          ;set (bc) equal to the status.
        inc     hl
        ld      B,(hl)
        call    SETBIT          ;and set this bit according to current drive.
        ld      (WRTPRT),hl     ;then save.
        ld      hl,(DIRSIZE)    ;now save directory size limit.
        inc     hl              ;remember the last one.
        EX      de,hl
        ld      hl,(SCRATCH1)   ;and store it here.
        ld      (hl),E          ;put low byte.
        inc     hl
        ld      (hl),D          ;then high byte.
        ret

;-----------------------------------------------------------------------------
;   Check the write protect status of the active disk.
;-----------------------------------------------------------------------------
CHKWPRT:call    GETWPRT
        ret     Z               ;return if ok.
        ld      hl,RODISK       ;else print message and terminate.
        jp      JUMPHL

;-----------------------------------------------------------------------------
;   Routine to set the DMA address to the users choice.
;-----------------------------------------------------------------------------
DEFDMA: ld      hl,USERDMA      ;reset the default DMA address and return.
        jp      DIRDMA1

;-----------------------------------------------------------------------------
;   Set the DMA address. On entry, (hl) points to
; word containing the desired DMA address.
;-----------------------------------------------------------------------------
DIRDMA1:ld      C,(hl)
        inc     hl
        ld      B,(hl)          ;setup (bc) and go to the bios to set it.
        jp      SETDMA

;-----------------------------------------------------------------------------
;   Set the status (STATUS) and return.
;-----------------------------------------------------------------------------
STSTATUS:
        ld    A,(FNDSTAT)
        jp      SETSTAT

;-----------------------------------------------------------------------------
; [36] SETTRAN - Function to return the random record position of a given
;                file which has been read in sequential mode up to now.
;-----------------------------------------------------------------------------
SETRAN:
        jp SETRAN ; FIXME

;-----------------------------------------------------------------------------
;   This routine select the drive specified in (ACTIVE) and
; update the login bitmap and bitmap table if this drive was
; not already active.
;-----------------------------------------------------------------------------
LOGINDRV:
        cp      a,16
        call    nc,SLCTERR      ; valid drive?
        ld      hl,(LOGIN)      ; yes, update the login bitmap.
        ld      c,l
        ld      b,h
        call    SETBIT
        ld      (LOGIN),hl      ; and save.
        ; jp      BITMAP        ; now update the bitmap.
        ret

;-----------------------------------------------------------------------------
; [14] SETDSK - Function to set the active disk number.
;-----------------------------------------------------------------------------
SETDSK:
       ld      A,(EPARAM)      ;get parameter passed and see if this
       ld      hl,ACTIVE       ;represents a change in drives.
       cp      (hl)
       ret     Z
       ld      (hl),A          ;yes it does, log it in.
       jp      LOGINDRV

;-----------------------------------------------------------------------------
;   This is the 'auto disk select' routine. The first byte
; of the FCB is examined for a drive specification. If non
; zero then the drive will be selected and loged in.
;-----------------------------------------------------------------------------
AUTOSEL:ld      A,0FFH          ;say 'auto-select activated'.
        ld      (AUTO),A
        ld      hl,(PARAMS)     ;get drive specified.
        ld      A,(hl)
        and     1FH             ;look at lower 5 bits.
        dec     A               ;adjust for (1=A, 2=B) etc.
        ld      (EPARAM),A      ;and save for the select routine.
        cp      1EH             ;check for 'no change' condition.
        jp      nc,AUTOSL1      ;yes, don't change.
        ld      A,(ACTIVE)      ;we must change, save currently active
        ld      (OLDDRV),A      ;drive.
        ld      A,(hl)          ;and save first byte of FCB also.
        ld      (AUTOFLAG),A    ;this must be non-zero.
        and     0E0H            ;whats this for (bits 6,7 are used for
        ld      (hl),A          ;something)?
        call    SETDSK          ;select and log in this drive.
AUTOSL1:ld      A,(USERNO)      ;move user number into FCB.
        ld      hl,(PARAMS)     ;(* upper half of first byte *)
        or      (hl)
        ld      (hl),A
        ret                     ;and return (all done).

;-----------------------------------------------------------------------------
; [12] GETVER - Function to return the current cp/m version number.
;-----------------------------------------------------------------------------
GETVER: ld      A,$22           ; version 2.2
        jp      SETSTAT

;-----------------------------------------------------------------------------
; [13] RSTDSK - Function to reset the disk system.
;-----------------------------------------------------------------------------
RSTDSK:
        ld      hl,0            ; clear write protect status and log
        ld      (WRTPRT),hl     ; in bitmap.
        ld      (LOGIN),hl
        xor     A               ; select drive 'A'.
        ld      (ACTIVE),A
        ld      hl,TBUFF        ; setup default DMA address.
        ld      (USERDMA),hl
        call    DEFDMA
        jp      LOGINDRV        ; now log in drive 'A'.

;-----------------------------------------------------------------------------
; [15] OPENFIL - Function to open a specified file.
;-----------------------------------------------------------------------------
OPENFIL:
        ld      (SAVEFCB), de   ; Save FCB
        call    _findfst
        jr      nz, .notfound

        ; Open file        
        call    _openfile
        bit     7,a
        jr      nz,.notfound

        ; FIXME

        ; Return success
        xor     a
        jp      SETSTAT

.notfound:
        ld      a,$FF
        jp      SETSTAT

        ; call    CLEARS2         ;clear 's2' byte.
        ; call    AUTOSEL         ;select proper disk.
        ; jp      OPENIT          ;and open the file.

;-----------------------------------------------------------------------------
; [16] CLOSEFIL - Function to close a specified file.
;-----------------------------------------------------------------------------
CLOSEFIL:
        jr      CLOSEFIL

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper disk.
        ; jp      CLOSEIT         ;and close the file.

;-----------------------------------------------------------------------------
; [17] GETFST - Function to return the first occurence of a specified file
;               name. If the first byte of the FCB is '?' then the name will
;               not be checked (get the first entry no matter what).
;
; DE=Address of FCB
;-----------------------------------------------------------------------------
GETFST:
        ld      (SAVEFCB), de   ; Save FCB
        call    _findfst        ; Open directory & find first entry
        jr      _getnxt_result  ; Continue in result handler of GETNXT

;-----------------------------------------------------------------------------
; [18] GETNXT - Function to return the next occurence of a file name.
;-----------------------------------------------------------------------------
GETNXT:
        ; ld      hl,(SAVEFCB)    ;restore pointers. note that no
        ; ld      (PARAMS),hl     ;other dbos calls are allowed.
        ; call    AUTOSEL         ;no error will be returned, but the
        ; call    FINDNXT         ;results will be wrong.
        ; jp      MOVEDIR

        call    _findnxt
_getnxt_result:
        jr      nz,.err
        call    _copy_dirent_to_dma
        xor     a                       ; Indicate success, direntry at first 32 of DMA
        jp      SETSTAT
.err:   ld      a,$FF                   ; Indicate failure
        jp      SETSTAT

;-----------------------------------------------------------------------------
; [19] DELFILE - Function to delete a file by name.
;-----------------------------------------------------------------------------
DELFILE:
        jr      DELFILE

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive.
        ; call    ERAFILE         ;erase the file.
        ; jp      STSTATUS        ;set status and return.

;-----------------------------------------------------------------------------
; [20] READSEQ - Function to execute a sequential read of the specified
;                record number.
;-----------------------------------------------------------------------------
READSEQ:
        ld      hl,(USERDMA)
        ld      de,128
        call    esp_read_bytes
        jr      nz,.err
        ld      a,d
        or      a,e
        jr      z,.eof

        ; Pad up to 128 bytes with EOF mark (CTRL-Z/$1A)
        ld      a,$1A
.1:     bit     7,e
        jr      nz,.done
        ld      (hl),a
        inc     hl
        inc     e
        jr      .1
.done:  xor     a
        jp      SETSTAT

.eof:   ld      a,1
        jp      SETSTAT

.err:   ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive then read.
        ; jp      RDSEQ

;-----------------------------------------------------------------------------
; [21] WRTSEQ - Function to write the next sequential record.
;-----------------------------------------------------------------------------
WRTSEQ:
        jr      WRTSEQ

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive then write.
        ; jp      WTSEQ

;-----------------------------------------------------------------------------
; [22] FCREATE - Create a file function.
;-----------------------------------------------------------------------------
FCREATE:
        jr      FCREATE

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    CLEARS2         ;clear the 's2' byte on all creates.
        ; call    AUTOSEL         ;select proper drive and get the next
        ; jp      GETEMPTY        ;empty directory space.

;-----------------------------------------------------------------------------
; [23] RENFILE - Function to rename a file.
;-----------------------------------------------------------------------------
RENFILE:
        jr      RENFILE

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive and then switch
        ; call    CHGNAMES        ;file names.
        ; jp      STSTATUS

;-----------------------------------------------------------------------------
; [24] GETLOG - Function to return the login bitmap.
;-----------------------------------------------------------------------------
GETLOG: ld      hl,(LOGIN)
        jp      GETPRM1

;-----------------------------------------------------------------------------
; [25] GETCRNT - Function to return the current disk assignment.
;-----------------------------------------------------------------------------
GETCRNT:ld      A,(ACTIVE)
        jp      SETSTAT

;-----------------------------------------------------------------------------
; [26] PUTDMA - Function to set the DMA address.
;-----------------------------------------------------------------------------
PUTDMA: EX      de,hl
        ld      (USERDMA),hl    ;save in our space and then get to
        jp      DEFDMA          ;the bios with this also.

;-----------------------------------------------------------------------------
; [27] GETALOC - Function to return the allocation bitmap.
;-----------------------------------------------------------------------------
GETALOC:ld      hl,(ALOCVECT)
        jp      GETPRM1

;-----------------------------------------------------------------------------
; [29] GETROV - Function to return the read-only status bitmap.
;-----------------------------------------------------------------------------
GETROV: ld      hl,(WRTPRT)
        jp      GETPRM1

;-----------------------------------------------------------------------------
; [30] SETATTR - Function to set the file attributes (read-only, system).
;-----------------------------------------------------------------------------
SETATTR:
        jr      SETATTR

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive then save attributes.
        ; call    SAVEATTR
        ; jp      STSTATUS

;-----------------------------------------------------------------------------
; [31] GETPARM - Function to return the address of the disk parameter block
;                for the current drive.
;-----------------------------------------------------------------------------
GETPARM:ld      hl,(DISKPB)
GETPRM1:ld      (STATUS),hl
        ret

;-----------------------------------------------------------------------------
; [32] GETUSER - Function to get or set the user number. If (E) was (FF)
;                then this is a request to return the current user number.
;                Else set the user number from (E).
;-----------------------------------------------------------------------------
GETUSER:ld      A,(EPARAM)      ;get parameter.
        cp      0FFH            ;get user number?
        jp      nz,SETUSER
        ld      A,(USERNO)      ;yes, just do it.
        jp      SETSTAT
SETUSER:and     1FH             ;no, we should set it instead. keep low
        ld      (USERNO),A      ;bits (0-4) only.
        ret

;-----------------------------------------------------------------------------
; [33] RDRANDOM - Function to read a random record from a file.
;-----------------------------------------------------------------------------
RDRANDOM:
        ld      hl,(PARAMS)
        ld      de,33
        add     hl,de

        ld      e,(hl)
        inc     hl
        ld      d,(hl)
        call    _seek

        jp      READSEQ

        ; call  AUTOSEL         ;select proper drive and read.
        ; jp      READRAN

;-----------------------------------------------------------------------------
; [34] WTRANDOM - Function to compute the file size for random files.
;-----------------------------------------------------------------------------
WTRANDOM:
        jr      WTRANDOM

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive and write.
        ; jp      WRITERAN

;-----------------------------------------------------------------------------
; [35] FILESIZE - Function to compute the size of a random file.
;-----------------------------------------------------------------------------
FILESIZE:
        jr      FILESIZE

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call  AUTOSEL         ;select proper drive and check file length
        ; jp      RANSIZE

;-----------------------------------------------------------------------------
; [37] LOGOFF - This allows a program to log off any drives. On entry,
;               set (de) to contain a word with bits set for those drives
;               that are to be logged off. The log-in bitmap and the write
;               protect bitmap will be updated.
;               This must be a M/PM special function.
;-----------------------------------------------------------------------------
LOGOFF: ld      hl,(PARAMS)     ;get drives to log off.
        ld      A,L             ;for each bit that is set, we want
        CPL                     ;to clear that bit in (LOGIN)
        ld      E,A             ;and (WRTPRT).
        ld      A,H
        CPL
        ld      hl,(LOGIN)      ;reset the login bitmap.
        and     H
        ld      D,A
        ld      A,L
        and     E
        ld      E,A
        ld      hl,(WRTPRT)
        EX      de,hl
        ld      (LOGIN),hl      ;and save.
        ld      A,L             ;now do the write protect bitmap.
        and     E
        ld      L,A
        ld      A,H
        and     D
        ld      H,A
        ld      (WRTPRT),hl     ;and save. all done.
        ret

;-----------------------------------------------------------------------------
; Get here to return to the user.
;-----------------------------------------------------------------------------
GOBACK: ld      a,(AUTO)        ; was auto select activated?
        or      a
        jp      z,GOBACK1
        ld      hl,(PARAMS)     ; yes, but was a change made?
        ld      (hl),0          ; (* reset first byte of FCB *)
        ld      a,(AUTOFLAG)
        or      a
        jp      z,GOBACK1
        ld      (hl),a          ; yes, reset first byte properly.
        ld      a,(OLDDRV)      ; and get the old drive and select it.
        ld      (EPARAM),a
        call    SETDSK
GOBACK1:ld      hl,(USRSTACK)   ; reset the users stack pointer.
        ld      sp,hl
        ld      hl,(STATUS)     ; get return status.
        ld      a,l             ; force version 1.4 compatability.
        ld      b,h
        ret                     ; and go back to user.

;-----------------------------------------------------------------------------
; [40] WTSPECL - This is a special entry to do random i/o.
;                For the case where we are writing to unused disk space, this
;                space will be zeroed out first. This must be a M/PM special
;                purpose function, because why would any normal program even
;                care about the previous contents of a sector about to be
;                written over.
;-----------------------------------------------------------------------------
WTSPECL:
        jr      WTSPECL

        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive.
        ; ld      A,2             ;use special write mode.
        ; ld      (MODE),A
        ; ld      C,0             ;set write indicator.
        ; call    POSITN1         ;position the file.
        ; call    Z,WTSEQ1        ;and write (if no errors).
        ; ret

;-----------------------------------------------------------------------------
;* BDOS data storage pool.
;-----------------------------------------------------------------------------
WRTPRT:   defw  0               ;write protect status for all 16 drives.
LOGIN:    defw  0               ;drive active word (1 bit per drive).
USERDMA:  defw  $80             ;user's DMA address (defaults to 80h).

;-----------------------------------------------------------------------------
; Scratch areas from parameter block.
;-----------------------------------------------------------------------------
SCRATCH1: defw  0               ;relative position within dir segment for file (0-3).

;-----------------------------------------------------------------------------
; Disk storage areas from parameter block.
;-----------------------------------------------------------------------------
DIRBUF:   defw  0               ;address of directory buffer to use.
DISKPB:   defw  0               ;contains address of disk parameter block.
CHKVECT:  defw  0               ;address of check bitmap.
ALOCVECT: defw  0               ;address of allocation bitmap.

;-----------------------------------------------------------------------------
; Parameter block returned from the bios.
;-----------------------------------------------------------------------------
DIRSIZE:  defw  0               ;directory size.
FNDSTAT:  defb  0               ;filename found status (0=found first entry).
EPARAM:   defb  0               ;storage for register (E) on entry to BDOS.
SAVEFCB:  defw  0               ;save space for address of FCB (for directory searches).
AUTO:     defb  0               ;if non-zero, then auto select activated.
OLDDRV:   defb  0               ;on auto select, storage for previous drive.
AUTOFLAG: defb  0               ;if non-zero, then auto select changed drives.
FCBPOS:   defb  0               ;relative position within buffer for FCB of file of interest.

;-----------------------------------------------------------------------------
; Directory entry
;-----------------------------------------------------------------------------
_dirent:
_dirent_date: defw 0
_dirent_time: defw 0
_dirent_attr: defb 0
_dirent_size: defd 0
_dirent_name: defs 8
_dirent_ext:  defs 3
_dirent_end:

;-----------------------------------------------------------------------------
; _read_dirent
;-----------------------------------------------------------------------------
_read_dirent:
        ld      a, ESPCMD_READDIR
        call    esp_cmd
        xor     a
        call    esp_send_byte
        call    esp_get_byte
        or      a
        ret     nz

        ; Read date/time/attribute
        ld      hl,_dirent
        ld      de,9
        call    esp_get_bytes

        ld      hl,_dirent_name

        ; Read filename
        ld      c,8
.1:     call    esp_get_byte
        cp      '.'
        jr      z, .pad2        ; Dot
        or      a
        jr      z, .pad1        ; Zero-byte
        ld      (hl),a
        inc     hl
        dec     c
        jr      nz, .1

        call    esp_get_byte    ; Dot
        or      a
        jr      nz, .ext

.pad1:  inc     c
        inc     c
        inc     c
        jr      .pad3

        ; Pad filename with spaces
.pad2:  ld      a,' '
.2:     ld      (hl),a
        inc     hl
        dec     c
        jr      nz, .2

        ; Read extension
.ext:   ld      c,3
.3:     call    esp_get_byte
        or      a
        jr      z, .pad3        ; Zero-byte
        ld      (hl),a
        inc     hl
        dec     c
        jr      nz, .3
        ret

.pad3:  ; Pad extension with spaces
        ld      a,' '
.4:     ld      (hl),a
        inc     hl
        dec     c
        jr      nz, .4
        ret

;-----------------------------------------------------------------------------
; Find first directory entry matching SAVEFCB
;-----------------------------------------------------------------------------
_findfst:
        ; Close directory descriptor 0
        ld      a, ESPCMD_CLOSEDIR
        call    esp_cmd
        xor     a
        call    esp_send_byte
        call    esp_get_byte

        ; Open directory
        ld      a, ESPCMD_OPENDIR83
        call    esp_cmd
        xor     a
        call    esp_send_byte
        call    esp_get_byte
        or      a
        ret     nz

        ; Find matching entry
        jr      _findnxt

;-----------------------------------------------------------------------------
; Get next directory entry matching SAVEFCB
;-----------------------------------------------------------------------------
_findnxt:
        ; Read entry
        call    _read_dirent
        ret     nz

        ; Check against FCB
        ld      de,(SAVEFCB)
        ld      a,(de)
        cp      '?'             ; '?' in user field gives back all entries
        ret     z
        inc     de

        ; Compare filename against FCB
        ld      c,11
        ld      hl,_dirent_name
.nxt:   ld      a,(de)
        cp      a,'?'           ; Wildcard?
        jr      z,.nxt2
        cp      a,(hl)
        jr      nz,_findnxt     ; Filename does not match, next entry

.nxt2:  inc     hl
        inc     de
        dec     c
        jr      nz,.nxt
        ret

;-----------------------------------------------------------------------------
; Copy current directory entry to DMA buffer
;-----------------------------------------------------------------------------
_copy_dirent_to_dma:
        ld      de, (USERDMA)
        ld      a, (USERNO)             ; Always give back current user (no user filtering implemented)
        ld      (de),a
        inc     de
        ld      hl,_dirent_name         ; Copy filename
        ld      bc,11
        ldir
        ex      de,hl
        xor     a                       ; Fill rest of entry with zeroes for now
        ld      bc,20
        call    memset
        ret

;-----------------------------------------------------------------------------
; Open file in _dirent_name
;-----------------------------------------------------------------------------
_openfile:
        ; Close file descriptor 0
        ld      a, ESPCMD_CLOSE
        call    esp_cmd
        xor     a
        call    esp_send_byte
        call    esp_get_byte



        ld      a, ESPCMD_OPEN
        call    esp_cmd
        ld      a,0             ; Flags (read-only)
        call    esp_send_byte

        ; Send filename
        ld      hl, _dirent_name
        ld      c,8
.1:     ld      a,(hl)
        cp      a,' '
        jr      z,.ext
        call    esp_send_byte
        inc     hl
        dec     c
        jr      nz,.1

        ; Send extension
.ext:   ld      a,'.'
        call    esp_send_byte
        ld      hl, _dirent_ext
        ld      c,3
.2:     ld      a,(hl)
        cp      a,' '
        jr      z,.zterm
        call    esp_send_byte
        inc     hl
        dec     c
        jr      nz,.2

        ; Send zero-termination
.zterm: xor     a
        call    esp_send_byte

        ; Get result
        call    esp_get_byte
        ret

;-----------------------------------------------------------------------------
; Seek to record DE
;-----------------------------------------------------------------------------
_seek:
        ld      a, ESPCMD_SEEK
        call    esp_cmd

        xor     a
        call    esp_send_byte   ; File descriptor

        ; a = (de << 7) & 0xFF
        ld      a,e
        rrca
        and     $80
        call    esp_send_byte

        ; a = (de >> 1) & 0xFF
        ld      a,d
        rra                     ; Shift lowest bit in B into carry
        ld      a,e
        rra
        call    esp_send_byte

        ; a = (de >> 9) & 0xFF
        ld      a,d
        rra
        and     $7F
        call    esp_send_byte

        xor     a
        call    esp_send_byte

        call    esp_get_byte
        ret

;-----------------------------------------------------------------------------
; Read bytes
; Input:  HL: destination address
;         DE: number of bytes to read
; Output: HL: next address (start address if no bytes read)
;         DE: number of bytes actually read
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
esp_read_bytes:
    ld      a, ESPCMD_READ
    call    esp_cmd

    ; Send file descriptor
    xor     a
    call    esp_send_byte

    ; Send read size
    ld      a, e
    call    esp_send_byte
    ld      a, d
    call    esp_send_byte

    ; Get result
    call    esp_get_byte
    or      a
    ret     nz

    ; Get number of bytes actual read
    call    esp_get_byte
    ld      e, a
    call    esp_get_byte
    ld      d, a

    push    de

.loop:
    ; Done reading? (DE=0)
    ld      a, d
    or      a, e
    jr      z, .done

    call    esp_get_byte
    ld      (hl), a
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de

    xor     a
    ret
