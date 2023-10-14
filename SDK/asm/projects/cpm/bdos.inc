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
; Select the desired drive.
;-----------------------------------------------------------------------------
;SELECT: ld      A,(ACTIVE)      ;get active disk.
;        ld      C,A
;        call    SELDSK          ;select it.
;        ld      A,H             ;valid drive?
;        or      L               ;valid drive?
;        ret     Z               ;return if not.
;
;        ;   Here, the BIOS returned the address of the parameter block
;        ; in (hl). We will extract the necessary pointers and save them.
;        ld      E,(hl)          ;yes, get address of translation table into (de).
;        inc     hl
;        ld      D,(hl)
;        inc     hl
;        ld      (SCRATCH1),hl   ;save pointers to scratch areas.
;        inc     hl
;        inc     hl
;        ld      (SCRATCH2),hl   ;ditto.
;        inc     hl
;        inc     hl
;        ld      (SCRATCH3),hl   ;ditto.
;        inc     hl
;        inc     hl
;        EX      de,hl           ;now save the translation table address.
;        ld      (XLATE),hl
;        ld      hl,DIRBUF       ;put the next 8 bytes here.
;        ld      C,8             ;they consist of the directory buffer
;        call    DE2HL           ;pointer, parameter block pointer,
;        ld      hl,(DISKPB)     ;check and allocation vectors.
;        EX      de,hl
;        ld      hl,SECTORS      ;move parameter block into our ram.
;        ld      C,15            ;it is 15 bytes long.
;        call    DE2HL
;        ld      hl,(DSKSIZE)    ;check disk size.
;        ld      A,H             ;more than 256 blocks on this?
;        ld      hl,BIGDISK
;        ld      (hl),0FFH       ;set to samll.
;        or      A
;        jp      Z,SELECT1
;        ld      (hl),0          ;wrong, set to large.
;SELECT1:ld      A,0FFH          ;clear the zero flag.
;        or      A
;        ret

;-----------------------------------------------------------------------------
; Routine to home the disk track head and clear pointers.
;-----------------------------------------------------------------------------
; HOMEDRV:call    HOME            ;home the head.
;         xor     A
;         ld      hl,(SCRATCH2)   ;set our track pointer also.
;         ld      (hl),A
;         inc     hl
;         ld      (hl),A
;         ld      hl,(SCRATCH3)   ;and our sector pointer.
;         ld      (hl),A
;         inc     hl
;         ld      (hl),A
;         ret

;-----------------------------------------------------------------------------
; Do the actual disk read and check the error return status.
;-----------------------------------------------------------------------------
; DOREAD: call    READ
;         jp      IORET

;-----------------------------------------------------------------------------
; Do the actual disk write and handle any bios error.
;-----------------------------------------------------------------------------
; DOWRITE:call    WRITE
; IORET:  or      A
;         ret     Z               ;return unless an error occured.
;         ld      hl,BADSCTR      ;bad read/write on this sector.
;         jp      JUMPHL

;-----------------------------------------------------------------------------
;   Routine to select the track and sector that the desired
; block number falls in.
;-----------------------------------------------------------------------------
; TRKSEC: ld      hl,(FILEPOS)    ;get position of last accessed file
;         ld      C,2             ;in directory and compute sector #.
;         call    SHIFTR          ;sector #=file-position/4.
;         ld      (BLKNMBR),hl    ;save this as the block number of interest.
;         ld      (CKSUMTBL),hl   ;what's it doing here too?
; ;
; ;   if the sector number has already been set (BLKNMBR), enter
; ; at this point.
; ;
; TRKSEC1:ld      hl,BLKNMBR
;         ld      C,(hl)          ;move sector number into (bc).
;         inc     hl
;         ld      B,(hl)
;         ld      hl,(SCRATCH3)   ;get current sector number and
;         ld      E,(hl)          ;move this into (de).
;         inc     hl
;         ld      D,(hl)
;         ld      hl,(SCRATCH2)   ;get current track number.
;         ld      A,(hl)          ;and this into (hl).
;         inc     hl
;         ld      H,(hl)
;         ld      L,A
; TRKSEC2:ld      A,C             ;is desired sector before current one?
;         sub     E
;         ld      A,B
;         SBC     A,D
;         jp      nc,TRKSEC3
;         push    hl              ;yes, decrement sectors by one track.
;         ld      hl,(SECTORS)    ;get sectors per track.
;         ld      A,E
;         sub     L
;         ld      E,A
;         ld      A,D
;         SBC     A,H
;         ld      D,A             ;now we have backed up one full track.
;         pop     hl
;         dec     hl              ;adjust track counter.
;         jp      TRKSEC2
; TRKSEC3:push    hl              ;desired sector is after current one.
;         ld      hl,(SECTORS)    ;get sectors per track.
;         ADD     hl,de           ;bump sector pointer to next track.
;         jp      C,TRKSEC4
;         ld      A,C             ;is desired sector now before current one?
;         sub     L
;         ld      A,B
;         SBC     A,H
;         jp      C,TRKSEC4
;         EX      de,hl           ;not yes, increment track counter
;         pop     hl              ;and continue until it is.
;         inc     hl
;         jp      TRKSEC3

;         ;   here we have determined the track number that contains the
;         ; desired sector.
; TRKSEC4:pop     hl              ;get track number (hl).
;         push    bc
;         push    de
;         push    hl
;         EX      de,hl
;         ld      hl,(OFFSET)     ;adjust for first track offset.
;         ADD     hl,de
;         ld      B,H
;         ld      C,L
;         call    SETTRK          ;select this track.
;         pop     de              ;reset current track pointer.
;         ld      hl,(SCRATCH2)
;         ld      (hl),E
;         inc     hl
;         ld      (hl),D
;         pop     de
;         ld      hl,(SCRATCH3)   ;reset the first sector on this track.
;         ld      (hl),E
;         inc     hl
;         ld      (hl),D
;         pop     bc
;         ld      A,C             ;now subtract the desired one.
;         sub     E               ;to make it relative (1-# sectors/track).
;         ld      C,A
;         ld      A,B
;         SBC     A,D
;         ld      B,A
;         ld      hl,(XLATE)      ;translate this sector according to this table.
;         EX      de,hl
;         call    SECTRN          ;let the bios translate it.
;         ld      C,L
;         ld      B,H
;         jp      SETSEC          ;and select it.

;-----------------------------------------------------------------------------
;   Compute block number from record number (SAVNREC) and
; extent number (SAVEXT).
;-----------------------------------------------------------------------------
; GETBLOCK: ld    hl,BLKSHFT      ;get logical to physical conversion.
;         ld      C,(hl)          ;note that this is base 2 log of ratio.
;         ld      A,(SAVNREC)     ;get record number.
; GETBLK1:or      A               ;compute (A)=(A)/2^BLKSHFT.
;         RRA
;         dec     C
;         jp      nz,GETBLK1
;         ld      B,A             ;save result in (B).
;         ld      A,8
;         sub     (hl)
;         ld      C,A             ;compute (C)=8-BLKSHFT.
;         ld      A,(SAVEXT)
; GETBLK2:dec     C               ;compute (A)=SAVEXT*2^(8-BLKSHFT).
;         jp      Z,GETBLK3
;         or      A
;         RLA
;         jp      GETBLK2
; GETBLK3:ADD     A,B
;         ret

;-----------------------------------------------------------------------------
;   Routine to extract the (bc) block byte from the fcb pointed
; to by (PARAMS). If this is a big-disk, then these are 16 bit
; block numbers, else they are 8 bit numbers.
; Number is returned in (hl).
;-----------------------------------------------------------------------------
; EXTBLK: ld      hl,(PARAMS)     ;get fcb address.
;         ld      de,16           ;block numbers start 16 bytes into fcb.
;         ADD     hl,de
;         ADD     hl,bc
;         ld      A,(BIGDISK)     ;are we using a big-disk?
;         or      A
;         jp      Z,EXTBLK1
;         ld      L,(hl)          ;no, extract an 8 bit number from the fcb.
;         ld      H,0
;         ret
; EXTBLK1:ADD     hl,bc           ;yes, extract a 16 bit number.
;         ld      E,(hl)
;         inc     hl
;         ld      D,(hl)
;         EX      de,hl           ;return in (hl).
;         ret

;-----------------------------------------------------------------------------
;   Compute block number.
;-----------------------------------------------------------------------------
; COMBLK: call    GETBLOCK
;         ld      C,A
;         ld      B,0
;         call    EXTBLK
;         ld      (BLKNMBR),hl
;         ret

;-----------------------------------------------------------------------------
;   Check for a zero block number (unused).
;-----------------------------------------------------------------------------
; CHKBLK: ld      hl,(BLKNMBR)
;         ld      A,L             ;is it zero?
;         or      H
;         ret

;-----------------------------------------------------------------------------
;   Adjust physical block (BLKNMBR) and convert to logical
; sector (LOGSECT). This is the starting sector of this block.
; The actual sector of interest is then added to this and the
; resulting sector number is stored back in (BLKNMBR). This
; will still have to be adjusted for the track number.
;-----------------------------------------------------------------------------
; LOGICAL:ld      A,(BLKSHFT)     ;get log2(physical/logical sectors).
;         ld      hl,(BLKNMBR)    ;get physical sector desired.
; LOGICL1:ADD     hl,hl           ;compute logical sector number.
;         dec     A               ;note logical sectors are 128 bytes long.
;         jp      nz,LOGICL1
;         ld      (LOGSECT),hl    ;save logical sector.
;         ld      A,(BLKMASK)     ;get block mask.
;         ld      C,A
;         ld      A,(SAVNREC)     ;get next sector to access.
;         and     C               ;extract the relative position within physical block.
;         or      L               ;and add it too logical sector.
;         ld      L,A
;         ld      (BLKNMBR),hl    ;and store.
;         ret

;-----------------------------------------------------------------------------
;   Set (hl) to point to extent byte in fcb.
;-----------------------------------------------------------------------------
; SETEXT: ld      hl,(PARAMS)
;         ld      de,12           ;it is the twelth byte.
;         ADD     hl,de
;         ret

;-----------------------------------------------------------------------------
;   Set (hl) to point to record count byte in fcb and (de) to
; next record number byte.
;-----------------------------------------------------------------------------
; SETHLDE:ld      hl,(PARAMS)
;         ld      de,15           ;record count byte (#15).
;         ADD     hl,de
;         EX      de,hl
;         ld      hl,17           ;next record number (#32).
;         ADD     hl,de
;         ret

;-----------------------------------------------------------------------------
;   Save current file data from fcb.
;-----------------------------------------------------------------------------
; STRDATA:call    SETHLDE
;         ld      A,(hl)          ;get and store record count byte.
;         ld      (SAVNREC),A
;         EX      de,hl
;         ld      A,(hl)          ;get and store next record number byte.
;         ld      (SAVNXT),A
;         call    SETEXT          ;point to extent byte.
;         ld      A,(EXTMASK)     ;get extent mask.
;         and     (hl)
;         ld      (SAVEXT),A      ;and save extent here.
;         ret

;-----------------------------------------------------------------------------
;   Set the next record to access. If (MODE) is set to 2, then
; the last record byte (SAVNREC) has the correct number to access.
; For sequential access, (MODE) will be equal to 1.
;-----------------------------------------------------------------------------
; SETNREC:call    SETHLDE
;         ld      A,(MODE)        ;get sequential flag (=1).
;         cp      2               ;a 2 indicates that no adder is needed.
;         jp      nz,STNREC1
;         xor     A               ;clear adder (random access?).
; STNREC1:ld      C,A
;         ld      A,(SAVNREC)     ;get last record number.
;         ADD     A,C             ;increment record count.
;         ld      (hl),A          ;and set fcb's next record byte.
;         EX      de,hl
;         ld      A,(SAVNXT)      ;get next record byte from storage.
;         ld      (hl),A          ;and put this into fcb as number of records used.
;         ret

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
;   Compute the check-sum for the directory buffer. Return
; integer sum in (A).
;-----------------------------------------------------------------------------
; CHECKSUM: ld    C,128           ;length of buffer.
;         ld      hl,(DIRBUF)     ;get its location.
;         xor     A               ;clear summation byte.
; CHKSUM1:
;         ADD A,(hl)                 ;and compute sum ignoring carries.
;         inc     hl
;         dec     C
;         jp      nz,CHKSUM1
;         ret

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
;   Check for a read only file.
;-----------------------------------------------------------------------------
; CHKROFL:call    FCB2HL          ;set (hl) to file entry in directory buffer.
; CKROF1: ld      de,9            ;look at bit 7 of the ninth byte.
;         ADD     hl,de
;         ld      A,(hl)
;         RLA
;         ret     nc              ;return if ok.
;         ld      hl,ROFILE       ;else, print error message and terminate.
;         jp      JUMPHL

;-----------------------------------------------------------------------------
;   Check the write protect status of the active disk.
;-----------------------------------------------------------------------------
CHKWPRT:call    GETWPRT
        ret     Z               ;return if ok.
        ld      hl,RODISK       ;else print message and terminate.
        jp      JUMPHL

;-----------------------------------------------------------------------------
;   Routine to set (hl) pointing to the proper entry in the
; directory buffer.
;-----------------------------------------------------------------------------
; FCB2HL: ld      hl,(DIRBUF)     ;get address of buffer.
;         ld      A,(FCBPOS)      ;relative position of file.

;-----------------------------------------------------------------------------
;   Routine to add (A) to (hl).
;-----------------------------------------------------------------------------
; ADDA2HL:add     A,L
;         ld      L,A
;         ret     nc
;         inc     H               ;take care of any carry.
;         ret

;-----------------------------------------------------------------------------
;   Routine to get the 's2' byte from the fcb supplied in
; the initial parameter specification.
;-----------------------------------------------------------------------------
; GETS2:  ld      hl,(PARAMS)     ;get address of fcb.
;         ld      de,14           ;relative position of 's2'.
;         ADD     hl,de
;         ld      A,(hl)          ;extract this byte.
;         ret

;-----------------------------------------------------------------------------
;   Clear the 's2' byte in the fcb.
;-----------------------------------------------------------------------------
; CLEARS2:call    GETS2           ;this sets (hl) pointing to it.
;         ld      (hl),0          ;now clear it.
;         ret

;-----------------------------------------------------------------------------
;   Set bit 7 in the 's2' byte of the fcb.
;-----------------------------------------------------------------------------
; SETS2B7:call    GETS2           ;get the byte.
;         or      80H             ;and set bit 7.
;         ld      (hl),A          ;then store.
;         ret

;-----------------------------------------------------------------------------
;   Compare (FILEPOS) with (SCRATCH1) and set flags based on
; the difference. This checks to see if there are more file
; names in the directory. We are at (FILEPOS) and there are
; (SCRATCH1) of them to check.
;-----------------------------------------------------------------------------
; MOREFLS:ld      hl,(FILEPOS)    ;we are here.
;         EX      de,hl
;         ld      hl,(SCRATCH1)   ;and don't go past here.
;         ld      A,E             ;compute difference but don't keep.
;         sub     (hl)
;         inc     hl
;         ld      A,D
;         SBC     (hl)            ;set carry if no more names.
;         ret

;-----------------------------------------------------------------------------
;   Call this routine to prevent (SCRATCH1) from being greater
; than (FILEPOS).
;-----------------------------------------------------------------------------
; CHKNMBR:call    MOREFLS         ;SCRATCH1 too big?
;         ret     C
;         inc     de              ;yes, reset it to (FILEPOS).
;         ld      (hl),D
;         dec     hl
;         ld      (hl),E
;         ret

;-----------------------------------------------------------------------------
;   Compute (hl)=(de)-(hl)
;-----------------------------------------------------------------------------
; SUBHL:  ld      A,E             ;compute difference.
;         sub     L
;         ld      L,A             ;store low byte.
;         ld      A,D
;         SBC     A,H
;         ld      H,A             ;and then high byte.
;         ret

;-----------------------------------------------------------------------------
;   Routine to set or compare the directory checksum byte. If
; (C)=0ffh, then this will set the checksum byte. Else the byte
; will be checked. If the check fails (the disk has been changed),
; then this disk will be write protected.
;-----------------------------------------------------------------------------
; CHECKDIR: ld    hl,(CKSUMTBL)
;         EX      de,hl
;         ld      hl,(ALLOC1)
;         call    SUBHL
;         ret     nc              ;ok if (CKSUMTBL) > (ALLOC1), so return.
;         push    bc
;         call    CHECKSUM        ;else compute checksum.
;         ld      hl,(CHKVECT)    ;get address of checksum table.
;         EX      de,hl
;         ld      hl,(CKSUMTBL)
;         ADD     hl,de           ;set (hl) to point to byte for this drive.
;         pop     bc
;         inc     C               ;set or check ?
;         jp      Z,CHKDIR1
;         cp      (hl)            ;check them.
;         ret     Z               ;return if they are the same.
;         call    MOREFLS         ;not the same, do we care?
;         ret     nc
;         call    WRTPRTD         ;yes, mark this as write protected.
;         ret
; CHKDIR1:ld      (hl),A          ;just set the byte.
;         ret

;-----------------------------------------------------------------------------
;   Do a write to the directory of the current disk.
;-----------------------------------------------------------------------------
; DIRWRITE:
;         ld      C,0FFH          ;set checksum byte.
;         call  SETDIR
;         call    DIRDMA          ;set directory DMA address.
;         ld      C,1             ;tell the bios to actually write.
;         call    DOWRITE         ;then do the write.
;         jp      DEFDMA

;-----------------------------------------------------------------------------
;   Read from the directory.
;-----------------------------------------------------------------------------
; DIRREAD:call    DIRDMA          ;set the directory DMA address.
;         call    DOREAD          ;and read it.

;-----------------------------------------------------------------------------
;   Routine to set the DMA address to the users choice.
;-----------------------------------------------------------------------------
DEFDMA: ld      hl,USERDMA      ;reset the default DMA address and return.
        jp      DIRDMA1

;-----------------------------------------------------------------------------
;   Routine to set the DMA address for directory work.
;-----------------------------------------------------------------------------
; DIRDMA: ld      hl,DIRBUF

;-----------------------------------------------------------------------------
;   Set the DMA address. On entry, (hl) points to
; word containing the desired DMA address.
;-----------------------------------------------------------------------------
DIRDMA1:ld      C,(hl)
        inc     hl
        ld      B,(hl)          ;setup (bc) and go to the bios to set it.
        jp      SETDMA

;-----------------------------------------------------------------------------
;   Move the directory buffer into user's DMA space.
;-----------------------------------------------------------------------------
; MOVEDIR:ld      hl,(DIRBUF)     ;buffer is located here, and
;         EX      de,hl
;         ld      hl,(USERDMA)    ; put it here.
;         ld      C,128           ;this is its length.
;         jp      DE2HL           ;move it now and return.

;-----------------------------------------------------------------------------
;   Check (FILEPOS) and set the zero flag if it equals 0ffffh.
;-----------------------------------------------------------------------------
; CKFILPOS: ld    hl,FILEPOS
;         ld      A,(hl)
;         inc     hl
;         cp      (hl)            ;are both bytes the same?
;         ret     nz
;         inc     A               ;yes, but are they each 0ffh?
;         ret

;-----------------------------------------------------------------------------
;   Set location (FILEPOS) to 0ffffh.
;-----------------------------------------------------------------------------
; STFILPOS: ld    hl,0FFFFH
;         ld      (FILEPOS),hl
;         ret

;-----------------------------------------------------------------------------
;   Move on to the next file position within the current
; directory buffer. If no more exist, set pointer to 0ffffh
; and the calling routine will check for this. Enter with (C)
; equal to 0ffh to cause the checksum byte to be set, else we
; will check this disk and set write protect if checksums are
; not the same (applies only if another directory sector must
; be read).
;-----------------------------------------------------------------------------
; NXENTRY:ld      hl,(DIRSIZE)    ;get directory entry size limit.
;         EX      de,hl
;         ld      hl,(FILEPOS)    ;get current count.
;         inc     hl              ;go on to the next one.
;         ld      (FILEPOS),hl
;         call    SUBHL           ;(hl)=(DIRSIZE)-(FILEPOS)
;         jp      nc,NXENT1       ;is there more room left?
;         jp      STFILPOS        ;no. Set this flag and return.
; NXENT1: ld      A,(FILEPOS)     ;get file position within directory.
;         and     03H             ;only look within this sector (only 4 entries fit).
;         ld      B,5             ;convert to relative position (32 bytes each).
; NXENT2: ADD     A,A             ;note that this is not efficient code.
;         dec     B               ;5 'ADD A's would be better.
;         jp      nz,NXENT2
;         ld      (FCBPOS),A      ;save it as position of fcb.
;         or      A
;         ret     nz              ;return if we are within buffer.
;         push    bc
;         call    TRKSEC          ;we need the next directory sector.
;         call    DIRREAD
;         pop     bc
;         jp      CHECKDIR

;-----------------------------------------------------------------------------
;   Routine to to get a bit from the disk space allocation
; map. It is returned in (A), bit position 0. On entry to here,
; set (bc) to the block number on the disk to check.
; On return, (D) will contain the original bit position for
; this block number and (hl) will point to the address for it.
;-----------------------------------------------------------------------------
; CKBITMAP: ld    A,C             ;determine bit number of interest.
;         and     07H             ;compute (D)=(E)=(C and 7)+1.
;         inc     A
;         ld      E,A             ;save particular bit number.
;         ld      D,A

;         ;   compute (bc)=(bc)/8.
;         ld      A,C
;         RRCA                    ;now shift right 3 bits.
;         RRCA
;         RRCA
;         and     1FH             ;and clear bits 7,6,5.
;         ld      C,A
;         ld      A,B
;         ADD     A,A             ;now shift (B) into bits 7,6,5.
;         ADD     A,A
;         ADD     A,A
;         ADD     A,A
;         ADD     A,A
;         or      C               ;and add in (C).
;         ld      C,A             ;ok, (C) ha been completed.
;         ld      A,B             ;is there a better way of doing this?
;         RRCA
;         RRCA
;         RRCA
;         and     1FH
;         ld      B,A             ;and now (B) is completed.

;         ; use this as an offset into the disk space allocation table.
;         ld      hl,(ALOCVECT)
;         ADD     hl,bc
;         ld      A,(hl)          ;now get correct byte.
; CKBMAP1:RLCA                    ;get correct bit into position 0.
;         dec     E
;         jp      nz,CKBMAP1
;         ret

;-----------------------------------------------------------------------------
;   Set or clear the bit map such that block number (bc) will be marked
; as used. On entry, if (E)=0 then this bit will be cleared, if it equals
; 1 then it will be set (don't use anyother values).
;-----------------------------------------------------------------------------
; STBITMAP: push  de
;         call    CKBITMAP        ;get the byte of interest.
;         and     0FEH            ;clear the affected bit.
;         pop     bc
;         or      C               ;and now set it acording to (C).

;-----------------------------------------------------------------------------
;  entry to restore the original bit position and then store
; in table. (A) contains the value, (D) contains the bit
; position (1-8), and (hl) points to the address within the
; space allocation table for this byte.
;-----------------------------------------------------------------------------
; STBMAP1:RRCA                    ;restore original bit position.
;         dec     D
;         jp      nz,STBMAP1
;         ld      (hl),A          ;and stor byte in table.
;         ret

;-----------------------------------------------------------------------------
;   Set/clear space used bits in allocation map for this file.
; On entry, (C)=1 to set the map and (C)=0 to clear it.
;-----------------------------------------------------------------------------
; SETFILE:call    FCB2HL          ;get address of fcb
;         ld      de,16
;         ADD     hl,de           ;get to block number bytes.
;         push    bc
;         ld      C,17            ;check all 17 bytes (max) of table.
; SETFL1: pop     de
;         dec     C               ;done all bytes yet?
;         ret     Z
;         push    de
;         ld      A,(BIGDISK)     ;check disk size for 16 bit block numbers.
;         or      A
;         jp      Z,SETFL2
;         push    bc              ;only 8 bit numbers. set (bc) to this one.
;         push    hl
;         ld      C,(hl)          ;get low byte from table, always
;         ld      B,0             ;set high byte to zero.
;         jp      SETFL3
; SETFL2: dec     C               ;for 16 bit block numbers, adjust counter.
;         push    bc
;         ld      C,(hl)          ;now get both the low and high bytes.
;         inc     hl
;         ld      B,(hl)
;         push    hl
; SETFL3: ld      A,C             ;block used?
;         or      B
;         jp      Z,SETFL4
;         ld      hl,(DSKSIZE)    ;is this block number within the
;         ld      A,L             ;space on the disk?
;         sub     C
;         ld      A,H
;         SBC     A,B
;         call    nc,STBITMAP     ;yes, set the proper bit.
; SETFL4: pop     hl              ;point to next block number in fcb.
;         inc     hl
;         pop     bc
;         jp      SETFL1

;-----------------------------------------------------------------------------
;   Construct the space used allocation bit map for the active
; drive. If a file name starts with '$' and it is under the
; current user number, then (STATUS) is set to minus 1. Otherwise
; it is not set at all.
;-----------------------------------------------------------------------------
; BITMAP: ld      hl,(DSKSIZE)    ;compute size of allocation table.
;         ld      C,3
;         call    SHIFTR          ;(hl)=(hl)/8.
;         inc     hl              ;at lease 1 byte.
;         ld      B,H
;         ld      C,L             ;set (bc) to the allocation table length.

;         ;
;         ;   Initialize the bitmap for this drive. Right now, the first
;         ; two bytes are specified by the disk parameter block. However
;         ; a patch could be entered here if it were necessary to setup
;         ; this table in a special mannor. For example, the bios could
;         ; determine locations of 'bad blocks' and set them as already
;         ; 'used' in the map.
;         ;
;         ld      hl,(ALOCVECT)   ;now zero out the table now.
; BITMAP1:ld      (hl),0
;         inc     hl
;         dec     bc
;         ld      A,B
;         or      C
;         jp      nz,BITMAP1
;         ld      hl,(ALLOC0)     ;get initial space used by directory.
;         EX      de,hl
;         ld      hl,(ALOCVECT)   ;and put this into map.
;         ld      (hl),E
;         inc     hl
;         ld      (hl),D

;         ;   End of initialization portion.
;         call    HOMEDRV         ;now home the drive.
;         ld      hl,(SCRATCH1)
;         ld      (hl),3          ;force next directory request to read
;         inc     hl              ;in a sector.
;         ld      (hl),0
;         call    STFILPOS        ;clear initial file position also.
; BITMAP2:ld      C,0FFH          ;read next file name in directory
;         call    NXENTRY         ;and set checksum byte.
;         call    CKFILPOS        ;is there another file?
;         ret     Z
;         call    FCB2HL          ;yes, get its address.
;         ld      A,0E5H
;         cp      (hl)            ;empty file entry?
;         jp      Z,BITMAP2
;         ld      A,(USERNO)      ;no, correct user number?
;         cp      (hl)
;         jp      nz,BITMAP3
;         inc     hl
;         ld      A,(hl)          ;yes, does name start with a '$'?
;         sub     '$'
;         jp      nz,BITMAP3
;         dec     A               ;yes, set atatus to minus one.
;         ld      (STATUS),A
; BITMAP3:ld      C,1             ;now set this file's space as used in bit map.
;         call    SETFILE
;         call    CHKNMBR         ;keep (SCRATCH1) in bounds.
;         jp      BITMAP2

;-----------------------------------------------------------------------------
;   Set the status (STATUS) and return.
;-----------------------------------------------------------------------------
STSTATUS:
        ld    A,(FNDSTAT)
        jp      SETSTAT

;-----------------------------------------------------------------------------
;   Check extents in (A) and (C). Set the zero flag if they
; are the same. The number of 16k chunks of disk space that
; the directory extent covers is expressad is (EXTMASK+1).
; No registers are modified.
;-----------------------------------------------------------------------------
; SAMEXT: push    bc
;         push    af
;         ld      A,(EXTMASK)     ;get extent mask and use it to
;         CPL                     ;to compare both extent numbers.
;         ld      B,A             ;save resulting mask here.
;         ld      A,C             ;mask first extent and save in (C).
;         and     B
;         ld      C,A
;         pop     af              ;now mask second extent and compare
;         and     B               ;with the first one.
;         sub     C
;         and     1FH             ;(* only check buts 0-4 *)
;         pop     bc              ;the zero flag is set if they are the same.
;         ret                     ;restore (bc) and return.

;-----------------------------------------------------------------------------
;   Search for the first occurence of a file name. On entry,
; register (C) should contain the number of bytes of the fcb
; that must match.
;-----------------------------------------------------------------------------
; FINDFST:ld      A,0FFH
;         ld      (FNDSTAT),A
;         ld      hl,COUNTER      ;save character count.
;         ld      (hl),C
;         ld      hl,(PARAMS)     ;get filename to match.
;         ld      (SAVEFCB),hl    ;and save.
;         call    STFILPOS        ;clear initial file position (set to 0ffffh).
;         call    HOMEDRV         ;home the drive.

;-----------------------------------------------------------------------------
;   Entry to locate the next occurence of a filename within the
; directory. The disk is not expected to have been changed. If
; it was, then it will be write protected.
;-----------------------------------------------------------------------------
; FINDNXT:ld      C,0             ;write protect the disk if changed.
;         call    NXENTRY         ;get next filename entry in directory.
;         call    CKFILPOS        ;is file position = 0ffffh?
;         jp      Z,FNDNXT6       ;yes, exit now then.
;         ld      hl,(SAVEFCB)    ;set (de) pointing to filename to match.
;         EX      de,hl
;         ld      A,(de)
;         cp      0E5H            ;empty directory entry?
;         jp      Z,FNDNXT1       ;(* are we trying to reserect erased entries? *)
;         push    de
;         call    MOREFLS         ;more files in directory?
;         pop     de
;         jp      nc,FNDNXT6      ;no more. Exit now.
; FNDNXT1:call    FCB2HL          ;get address of this fcb in directory.
;         ld      A,(COUNTER)     ;get number of bytes (characters) to check.
;         ld      C,A
;         ld      B,0             ;initialize byte position counter.
; FNDNXT2:ld      A,C             ;are we done with the compare?
;         or      A
;         jp      Z,FNDNXT5
;         ld      A,(de)          ;no, check next byte.
;         cp      '?'             ;don't care about this character?
;         jp      Z,FNDNXT4
;         ld      A,B             ;get bytes position in fcb.
;         cp      13              ;don't care about the thirteenth byte either.
;         jp      Z,FNDNXT4
;         cp      12              ;extent byte?
;         ld      A,(de)
;         jp      Z,FNDNXT3
;         sub     (hl)            ;otherwise compare characters.
;         and     $7F
;         jp      nz,FINDNXT      ;not the same, check next entry.
;         jp      FNDNXT4         ;so far so good, keep checking.
; FNDNXT3:push    bc              ;check the extent byte here.
;         ld      C,(hl)
;         call    SAMEXT
;         pop     bc
;         jp      nz,FINDNXT      ;not the same, look some more.
;         ;   So far the names compare. Bump pointers to the next byte
;         ; and continue until all (C) characters have been checked.
; FNDNXT4:inc     de              ;bump pointers.
;         inc     hl
;         inc     B
;         dec     C               ;adjust character counter.
;         jp      FNDNXT2
; FNDNXT5:ld      A,(FILEPOS)     ;return the position of this entry.
;         and     03H
;         ld      (STATUS),A
;         ld      hl,FNDSTAT
;         ld      A,(hl)
;         RLA
;         ret     nc
;         xor     A
;         ld      (hl),A
;         ret
;         ; Filename was not found. Set appropriate status.
; FNDNXT6:call    STFILPOS        ;set (FILEPOS) to 0ffffh.
;         ld      A,0FFH          ;say not located.
;         jp      SETSTAT

;-----------------------------------------------------------------------------
;   Erase files from the directory. Only the first byte of the
; fcb will be affected. It is set to (E5).
;-----------------------------------------------------------------------------
; ERAFILE:call    CHKWPRT         ;is disk write protected?
;         ld      C,12            ;only compare file names.
;         call    FINDFST         ;get first file name.
; ERAFIL1:call    CKFILPOS        ;any found?
;         ret     Z               ;nope, we must be done.
;         call    CHKROFL         ;is file read only?
;         call    FCB2HL          ;nope, get address of fcb and
;         ld      (hl),0E5H       ;set first byte to 'empty'.
;         ld      C,0             ;clear the space from the bit map.
;         call    SETFILE
;         call    DIRWRITE        ;now write the directory sector back out.
;         call    FINDNXT         ;find the next file name.
;         jp      ERAFIL1         ;and repeat process.

;-----------------------------------------------------------------------------
;   Look through the space allocation map (bit map) for the
; next available block. Start searching at block number (bc-1).
; The search procedure is to look for an empty block that is
; before the starting block. If not empty, look at a later
; block number. In this way, we return the closest empty block
; on either side of the 'target' block number. This will speed
; access on random devices. For serial devices, this should be
; changed to look in the forward direction first and then start
; at the front and search some more.
;
;   On return, (de)= block number that is empty and (hl) =0
; if no empry block was found.
;-----------------------------------------------------------------------------
; FNDSPACE:
;         ld    D,B             ;set (de) as the block that is checked.
;         ld      E,C
;         ;
;         ;   Look before target block. Registers (bc) are used as the lower
;         ; pointer and (de) as the upper pointer.
;         ;
; FNDSPA1:
;         ld      A,C             ;is block 0 specified?
;         or      B
;         jp      Z,FNDSPA2
;         dec     bc              ;nope, check previous block.
;         push    de
;         push    bc
;         call    CKBITMAP
;         RRA                     ;is this block empty?
;         jp      nc,FNDSPA3      ;yes. use this.

;         ;   Note that the above logic gets the first block that it finds
;         ; that is empty. Thus a file could be written 'backward' making
;         ; it very slow to access. This could be changed to look for the
;         ; first empty block and then continue until the start of this
;         ; empty space is located and then used that starting block.
;         ; This should help speed up access to some files especially on
;         ; a well used disk with lots of fairly small 'holes'.
;         pop     bc              ;nope, check some more.
;         pop     de

;         ;   Now look after target block.
; FNDSPA2:ld      hl,(DSKSIZE)    ;is block (de) within disk limits?
;         ld      A,E
;         sub     L
;         ld      A,D
;         SBC     A,H
;         jp      nc,FNDSPA4
;         inc     de              ;yes, move on to next one.
;         push    bc
;         push    de
;         ld      B,D
;         ld      c,e
;         call    CKBITMAP        ;check it.
;         RRA                     ;empty?
;         jp      nc,FNDSPA3
;         pop     de              ;nope, continue searching.
;         pop     bc
;         jp      FNDSPA1

;         ;   Empty block found. Set it as used and return with (hl)
;         ; pointing to it (true?).
; FNDSPA3:RLA                     ;reset byte.
;         inc     A               ;and set bit 0.
;         call    STBMAP1         ;update bit map.
;         pop     hl              ;set return registers.
;         pop     de
;         ret

;         ;   Free block was not found. If (bc) is not zero, then we have
;         ; not checked all of the disk space.
; FNDSPA4:ld      A,C
;         or      B
;         jp      nz,FNDSPA1
;         ld      hl,0            ;set 'not found' status.
;         ret

;-----------------------------------------------------------------------------
;   Move a complete fcb entry into the directory and write it.
;-----------------------------------------------------------------------------
; FCBSET: ld      C,0
;         ld      E,32            ;length of each entry.

;-----------------------------------------------------------------------------
;   Move (E) bytes from the fcb pointed to by (PARAMS) into
; fcb in directory starting at relative byte (C). This updated
; directory buffer is then written to the disk.
;-----------------------------------------------------------------------------
; UPDATE: push    de
;         ld      B,0             ;set (bc) to relative byte position.
;         ld      hl,(PARAMS)     ;get address of fcb.
;         ADD     hl,bc           ;compute starting byte.
;         EX      de,hl
;         call    FCB2HL          ;get address of fcb to update in directory.
;         pop     bc              ;set (C) to number of bytes to change.
;         call    DE2HL
; UPDATE1:call    TRKSEC          ;determine the track and sector affected.
;         jp      DIRWRITE        ;then write this sector out.

;-----------------------------------------------------------------------------
;   Routine to change the name of all files on the disk with a
; specified name. The fcb contains the current name as the
; first 12 characters and the new name 16 bytes into the fcb.
;-----------------------------------------------------------------------------
; CHGNAMES: call  CHKWPRT         ;check for a write protected disk.
;         ld      C,12            ;match first 12 bytes of fcb only.
;         call    FINDFST         ;get first name.
;         ld      hl,(PARAMS)     ;get address of fcb.
;         ld      A,(hl)          ;get user number.
;         ld      de,16           ;move over to desired name.
;         ADD     hl,de
;         ld      (hl),A          ;keep same user number.
; CHGNAM1:call    CKFILPOS        ;any matching file found?
;         ret     Z               ;no, we must be done.
;         call    CHKROFL         ;check for read only file.
;         ld      C,16            ;start 16 bytes into fcb.
;         ld      E,12            ;and update the first 12 bytes of directory.
;         call    UPDATE
;         call    FINDNXT         ;get te next file name.
;         jp      CHGNAM1         ;and continue.

;-----------------------------------------------------------------------------
;   Update a files attributes. The procedure is to search for
; every file with the same name as shown in fcb (ignoring bit 7)
; and then to update it (which includes bit 7). No other changes
; are made.
;-----------------------------------------------------------------------------
; SAVEATTR:
;         ld    C,12              ;match first 12 bytes.
;         call    FINDFST         ;look for first filename.
; SAVATR1:call    CKFILPOS        ;was one found?
;         ret     Z               ;nope, we must be done.
;         ld      C,0             ;yes, update the first 12 bytes now.
;         ld      E,12
;         call    UPDATE          ;update filename and write directory.
;         call    FINDNXT         ;and get the next file.
;         jp      SAVATR1         ;then continue until done.

;-----------------------------------------------------------------------------
;  Open a file (name specified in fcb).
;-----------------------------------------------------------------------------
; OPENIT: ld      C,15            ;compare the first 15 bytes.
;         call    FINDFST         ;get the first one in directory.
;         call    CKFILPOS        ;any at all?
;         ret     Z
; OPENIT1:call    SETEXT          ;point to extent byte within users fcb.
;         ld      A,(hl)          ;and get it.
;         push    af              ;save it and address.
;         push    hl
;         call    FCB2HL          ;point to fcb in directory.
;         EX      de,hl
;         ld      hl,(PARAMS)     ;this is the users copy.
;         ld      C,32            ;move it into users space.
;         push    de
;         call    DE2HL
;         call    SETS2B7         ;set bit 7 in 's2' byte (unmodified).
;         pop     de              ;now get the extent byte from this fcb.
;         ld      hl,12
;         ADD     hl,de
;         ld      C,(hl)          ;into (C).
;         ld      hl,15           ;now get the record count byte into (B).
;         ADD     hl,de
;         ld      B,(hl)
;         pop     hl              ;keep the same extent as the user had originally.
;         pop     af
;         ld      (hl),A
;         ld      A,C             ;is it the same as in the directory fcb?
;         cp      (hl)
;         ld      A,B             ;if yes, then use the same record count.
;         jp      Z,OPENIT2
;         ld      A,0             ;if the user specified an extent greater than
;         jp      C,OPENIT2       ;the one in the directory, then set record count to 0.
;         ld      A,128           ;otherwise set to maximum.
; OPENIT2:ld      hl,(PARAMS)     ;set record count in users fcb to (A).
;         ld      de,15
;         ADD     hl,de           ;compute relative position.
;         ld      (hl),A          ;and set the record count.
;         ret

;-----------------------------------------------------------------------------
;   Move two bytes from (de) to (hl) if (and only if) (hl)
; point to a zero value (16 bit).
;   Return with zero flag set it (de) was moved. Registers (de)
; and (hl) are not changed. However (A) is.
;-----------------------------------------------------------------------------
; MOVEWORD:
;         ld      A,(hl)          ;check for a zero word.
;         inc     hl
;         or      (hl)            ;both bytes zero?
;         dec     hl
;         ret     nz              ;nope, just return.
;         ld      A,(de)          ;yes, move two bytes from (de) into
;         ld      (hl),A          ;this zero space.
;         inc     de
;         inc     hl
;         ld      A,(de)
;         ld      (hl),A
;         dec     de              ;don't disturb these registers.
;         dec     hl
;         ret

;-----------------------------------------------------------------------------
;   Get here to close a file specified by (fcb).
;-----------------------------------------------------------------------------
; CLOSEIT:xor     A               ;clear status and file position bytes.
;         ld      (STATUS),A
;         ld      (FILEPOS),A
;         ld      (FILEPOS+1),A
;         call    GETWPRT         ;get write protect bit for this drive.
;         ret     nz              ;just return if it is set.
;         call    GETS2           ;else get the 's2' byte.
;         and     80H             ;and look at bit 7 (file unmodified?).
;         ret     nz              ;just return if set.
;         ld      C,15            ;else look up this file in directory.
;         call    FINDFST
;         call    CKFILPOS        ;was it found?
;         ret     Z               ;just return if not.
;         ld      bc,16           ;set (hl) pointing to records used section.
;         call    FCB2HL
;         ADD     hl,bc
;         EX      de,hl
;         ld      hl,(PARAMS)     ;do the same for users specified fcb.
;         ADD     hl,bc
;         ld      C,16            ;this many bytes are present in this extent.
; CLOSEIT1: ld    A,(BIGDISK)     ;8 or 16 bit record numbers?
;         or      A
;         jp      Z,CLOSEIT4
;         ld      A,(hl)          ;just 8 bit. Get one from users fcb.
;         or      A
;         ld      A,(de)          ;now get one from directory fcb.
;         jp      nz,CLOSEIT2
;         ld      (hl),A          ;users byte was zero. Update from directory.
; CLOSEIT2: or    A
;         jp      nz,CLOSEIT3
;         ld      A,(hl)          ;directories byte was zero, update from users fcb.
;         ld      (de),A
; CLOSEIT3: cp    (hl)            ;if neither one of these bytes were zero,
;         jp      nz,CLOSEIT7     ;then close error if they are not the same.
;         jp      CLOSEIT5        ;ok so far, get to next byte in fcbs.
; CLOSEIT4: call  MOVEWORD        ;update users fcb if it is zero.
;         EX      de,hl
;         call    MOVEWORD        ;update directories fcb if it is zero.
;         EX      de,hl
;         ld      A,(de)          ;if these two values are no different,
;         cp      (hl)            ;then a close error occured.
;         jp      nz,CLOSEIT7
;         inc     de              ;check second byte.
;         inc     hl
;         ld      A,(de)
;         cp      (hl)
;         jp      nz,CLOSEIT7
;         dec     C               ;remember 16 bit values.
; CLOSEIT5: inc   de              ;bump to next item in table.
;         inc     hl
;         dec     C               ;there are 16 entries only.
;         jp      nz,CLOSEIT1     ;continue if more to do.
;         ld      bc,0FFECH       ;backup 20 places (extent byte).
;         ADD     hl,bc
;         EX      de,hl
;         ADD     hl,bc
;         ld      A,(de)
;         cp      (hl)            ;directory's extent already greater than the
;         jp      C,CLOSEIT6      ;users extent?
;         ld      (hl),A          ;no, update directory extent.
;         ld      bc,3            ;and update the record count byte in
;         ADD     hl,bc           ;directories fcb.
;         EX      de,hl
;         ADD     hl,bc
;         ld      A,(hl)          ;get from user.
;         ld      (de),A          ;and put in directory.
; CLOSEIT6: ld    A,0FFH          ;set 'was open and is now closed' byte.
;         ld      (CLOSEFLG),A
;         jp      UPDATE1         ;update the directory now.
; CLOSEIT7: ld    hl,STATUS       ;set return status and then return.
;         dec     (hl)
;         ret

;-----------------------------------------------------------------------------
;   Routine to get the next empty space in the directory. It
; will then be cleared for use.
;-----------------------------------------------------------------------------
; GETEMPTY: call  CHKWPRT         ;make sure disk is not write protected.
;         ld      hl,(PARAMS)     ;save current parameters (fcb).
;         push    hl
;         ld      hl,EMPTYFCB     ;use special one for empty space.
;         ld      (PARAMS),hl
;         ld      C,1             ;search for first empty spot in directory.
;         call    FINDFST         ;(* only check first byte *)
;         call    CKFILPOS        ;none?
;         pop     hl
;         ld      (PARAMS),hl     ;restore original fcb address.
;         ret     Z               ;return if no more space.
;         EX      de,hl
;         ld      hl,15           ;point to number of records for this file.
;         ADD     hl,de
;         ld      C,17            ;and clear all of this space.
;         xor     A
; GETMT1: ld      (hl),A
;         inc     hl
;         dec     C
;         jp      nz,GETMT1
;         ld      hl,13           ;clear the 's1' byte also.
;         ADD     hl,de
;         ld      (hl),A
;         call    CHKNMBR         ;keep (SCRATCH1) within bounds.
;         call    FCBSET          ;write out this fcb entry to directory.
;         jp      SETS2B7         ;set 's2' byte bit 7 (unmodified at present).

;-----------------------------------------------------------------------------
; Routine to close the current extent and open the next one for reading.
;-----------------------------------------------------------------------------
; GETNEXT:xor     A
;         ld      (CLOSEFLG),A    ;clear close flag.
;         call    CLOSEIT         ;close this extent.
;         call    CKFILPOS
;         ret     Z               ;not there???
;         ld      hl,(PARAMS)     ;get extent byte.
;         ld      bc,12
;         ADD     hl,bc
;         ld      A,(hl)          ;and increment it.
;         inc     A
;         and     1FH             ;keep within range 0-31.
;         ld      (hl),A
;         jp      Z,GTNEXT1       ;overflow?
;         ld      B,A             ;mask extent byte.
;         ld      A,(EXTMASK)
;         and     B
;         ld      hl,CLOSEFLG     ;check close flag (0ffh is ok).
;         and     (hl)
;         jp      Z,GTNEXT2       ;if zero, we must read in next extent.
;         jp      GTNEXT3         ;else, it is already in memory.
; GTNEXT1:ld      bc,2            ;Point to the 's2' byte.
;         ADD     hl,bc
;         inc     (hl)            ;and bump it.
;         ld      A,(hl)          ;too many extents?
;         and     0FH
;         jp      Z,GTNEXT5       ;yes, set error code.

;-----------------------------------------------------------------------------
;   Get here to open the next extent.
;-----------------------------------------------------------------------------
; GTNEXT2:ld      C,15            ;set to check first 15 bytes of fcb.
;         call    FINDFST         ;find the first one.
;         call    CKFILPOS        ;none available?
;         jp      nz,GTNEXT3
;         ld      A,(RDWRTFLG)    ;no extent present. Can we open an empty one?
;         inc     A               ;0ffh means reading (so not possible).
;         jp      Z,GTNEXT5       ;or an error.
;         call    GETEMPTY        ;we are writing, get an empty entry.
;         call    CKFILPOS        ;none?
;         jp      Z,GTNEXT5       ;error if true.
;         jp      GTNEXT4         ;else we are almost done.
; GTNEXT3:call    OPENIT1         ;open this extent.
; GTNEXT4:call    STRDATA         ;move in updated data (rec #, extent #, etc.)
;         xor     A               ;clear status and return.
;         jp      SETSTAT

;         ;   Error in extending the file. Too many extents were needed
;         ; or not enough space on the disk.
; GTNEXT5:call    IOERR1          ;set error code, clear bit 7 of 's2'
;         jp      SETS2B7         ;so this is not written on a close.

;         ;   Read a sequential file.
; RDSEQ:  ld      A,1             ;set sequential access mode.
;         ld      (MODE),A
; RDSEQ1: ld      A,0FFH          ;don't allow reading unwritten space.
;         ld      (RDWRTFLG),A
;         call    STRDATA         ;put rec# and ext# into fcb.
;         ld      A,(SAVNREC)     ;get next record to read.
;         ld      hl,SAVNXT       ;get number of records in extent.
;         cp      (hl)            ;within this extent?
;         jp      C,RDSEQ2
;         cp      128             ;no. Is this extent fully used?
;         jp      nz,RDSEQ3       ;no. End-of-file.
;         call    GETNEXT         ;yes, open the next one.
;         xor     A               ;reset next record to read.
;         ld      (SAVNREC),A
;         ld      A,(STATUS)      ;check on open, successful?
;         or      A
;         jp      nz,RDSEQ3       ;no, error.
; RDSEQ2: call    COMBLK          ;ok. compute block number to read.
;         call    CHKBLK          ;check it. Within bounds?
;         jp      Z,RDSEQ3        ;no, error.
;         call    LOGICAL         ;convert (BLKNMBR) to logical sector (128 byte).
;         call    TRKSEC1         ;set the track and sector for this block #.
;         call    DOREAD          ;and read it.
;         jp      SETNREC         ;and set the next record to be accessed.

;         ;   Read error occured. Set status and return.
; RDSEQ3: jp      IOERR1
;         ;   Write the next sequential record.
; WTSEQ:  ld      A,1             ;set sequential access mode.
;         ld      (MODE),A
; WTSEQ1: ld      A,0             ;allow an addition empty extent to be opened.
;         ld      (RDWRTFLG),A
;         call    CHKWPRT         ;check write protect status.
;         ld      hl,(PARAMS)
;         call    CKROF1          ;check for read only file, (hl) already set to fcb.
;         call    STRDATA         ;put updated data into fcb.
;         ld      A,(SAVNREC)     ;get record number to write.
;         cp      128             ;within range?
;         jp      nc,IOERR1       ;no, error(?).
;         call    COMBLK          ;compute block number.
;         call    CHKBLK          ;check number.
;         ld      C,0             ;is there one to write to?
;         jp      nz,WTSEQ6       ;yes, go do it.
;         call    GETBLOCK        ;get next block number within fcb to use.
;         ld      (RELBLOCK),A    ;and save.
;         ld      bc,0            ;start looking for space from the start
;         or      A               ;if none allocated as yet.
;         jp      Z,WTSEQ2
;         ld      C,A             ;extract previous block number from fcb
;         dec     bc              ;so we can be closest to it.
;         call    EXTBLK
;         ld      B,H
;         ld      C,L
; WTSEQ2: call    FNDSPACE        ;find the next empty block nearest number (bc).
;         ld      A,L             ;check for a zero number.
;         or      H
;         jp      nz,WTSEQ3
;         ld      A,2             ;no more space?
;         jp      SETSTAT
; WTSEQ3: ld      (BLKNMBR),hl    ;save block number to access.
;         EX      de,hl           ;put block number into (de).
;         ld      hl,(PARAMS)     ;now we must update the fcb for this
;         ld      bc,16           ;newly allocated block.
;         ADD     hl,bc
;         ld      A,(BIGDISK)     ;8 or 16 bit block numbers?
;         or      A
;         ld      A,(RELBLOCK)    ;(* update this entry *)
;         jp      Z,WTSEQ4        ;zero means 16 bit ones.
;         call    ADDA2HL         ;(hl)=(hl)+(A)
;         ld      (hl),E          ;store new block number.
;         jp      WTSEQ5
; WTSEQ4: ld      C,A             ;compute spot in this 16 bit table.
;         ld      B,0
;         ADD     hl,bc
;         ADD     hl,bc
;         ld      (hl),E          ;stuff block number (de) there.
;         inc     hl
;         ld      (hl),D
; WTSEQ5: ld      C,2             ;set (C) to indicate writing to un-used disk space.
; WTSEQ6: ld      A,(STATUS)      ;are we ok so far?
;         or      A
;         ret     nz
;         push    bc              ;yes, save write flag for bios (register C).
;         call    LOGICAL         ;convert (BLKNMBR) over to loical sectors.
;         ld      A,(MODE)        ;get access mode flag (1=sequential,
;         dec     A               ;0=random, 2=special?).
;         dec     A
;         jp      nz,WTSEQ9

;         ;   Special random i/o from function #40. Maybe for M/PM, but the
;         ; current block, if it has not been written to, will be zeroed
;         ; out and then written (reason?).
;         pop     bc
;         push    bc
;         ld      A,C             ;get write status flag (2=writing unused space).
;         dec     A
;         dec     A
;         jp      nz,WTSEQ9
;         push    hl
;         ld      hl,(DIRBUF)     ;zero out the directory buffer.
;         ld      D,A             ;note that (A) is zero here.
; WTSEQ7: ld      (hl),A
;         inc     hl
;         inc     D               ;do 128 bytes.
;         jp      P,WTSEQ7
;         call    DIRDMA          ;tell the bios the DMA address for directory access.
;         ld      hl,(LOGSECT)    ;get sector that starts current block.
;         ld      C,2             ;set 'writing to unused space' flag.
; WTSEQ8: ld      (BLKNMBR),hl    ;save sector to write.
;         push    bc
;         call    TRKSEC1         ;determine its track and sector numbers.
;         pop     bc
;         call    DOWRITE         ;now write out 128 bytes of zeros.
;         ld      hl,(BLKNMBR)    ;get sector number.
;         ld      C,0             ;set normal write flag.
;         ld      A,(BLKMASK)     ;determine if we have written the entire
;         ld      B,A             ;physical block.
;         and     L
;         cp      B
;         inc     hl              ;prepare for the next one.
;         jp      nz,WTSEQ8       ;continue until (BLKMASK+1) sectors written.
;         pop     hl              ;reset next sector number.
;         ld      (BLKNMBR),hl
;         call    DEFDMA          ;and reset DMA address.

;         ;   Normal disk write. Set the desired track and sector then
;         ; do the actual write.
; WTSEQ9: call    TRKSEC1         ;determine track and sector for this write.
;         pop     bc              ;get write status flag.
;         push    bc
;         call    DOWRITE         ;and write this out.
;         pop     bc
;         ld      A,(SAVNREC)     ;get number of records in file.
;         ld      hl,SAVNXT       ;get last record written.
;         cp      (hl)
;         jp      C,WTSEQ10
;         ld      (hl),A          ;we have to update record count.
;         inc     (hl)
;         ld      C,2

;         ;*   This area has been patched to correct disk update problem
;         ;* when using blocking and de-blocking in the BIOS.
; WTSEQ10:NOP                     ;was 'dcr c'
;         NOP                     ;was 'dcr c'
;         ld      hl,0            ;was 'jnz wtseq99'
;         ; *   End of patch.

;         push    af
;         call    GETS2           ;set 'extent written to' flag.
;         and     $7F             ;(* clear bit 7 *)
;         ld      (hl),A
;         pop     af              ;get record count for this extent.
; WTSEQ99:cp      127             ;is it full?
;         jp      nz,WTSEQ12
;         ld      A,(MODE)        ;yes, are we in sequential mode?
;         cp      1
;         jp      nz,WTSEQ12
;         call    SETNREC         ;yes, set next record number.
;         call    GETNEXT         ;and get next empty space in directory.
;         ld      hl,STATUS       ;ok?
;         ld      A,(hl)
;         or      A
;         jp      nz,WTSEQ11
;         dec     A               ;yes, set record count to -1.
;         ld      (SAVNREC),A
; WTSEQ11:ld      (hl),0          ;clear status.
; WTSEQ12:jp      SETNREC         ;set next record to access.

;-----------------------------------------------------------------------------
;   For random i/o, set the fcb for the desired record number
; based on the 'r0,r1,r2' bytes. These bytes in the fcb are
; used as follows:
;
;       fcb+35            fcb+34            fcb+33
;  |     'r-2'      |      'r-1'      |      'r-0'     |
;  |7             0 | 7             0 | 7             0|
;  |0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0|
;  |    overflow   | | extra |  extent   |   record #  |
;  | ______________| |_extent|__number___|_____________|
;                     also 's2'
;
;   On entry, register (C) contains 0ffh if this is a read
; and thus we can not access unwritten disk space. Otherwise,
; another extent will be opened (for writing) if required.
;-----------------------------------------------------------------------------
; POSITION:
;         xor   A               ;set random i/o flag.
;         ld      (MODE),A

;         ;   Special entry (function #40). M/PM ?
; POSITN1:push    bc              ;save read/write flag.
;         ld      hl,(PARAMS)     ;get address of fcb.
;         EX      de,hl
;         ld      hl,33           ;now get byte 'r0'.
;         ADD     hl,de
;         ld      A,(hl)
;         and     $7F             ;keep bits 0-6 for the record number to access.
;         push    af
;         ld      A,(hl)          ;now get bit 7 of 'r0' and bits 0-3 of 'r1'.
;         RLA
;         inc     hl
;         ld      A,(hl)
;         RLA
;         and     1FH             ;and save this in bits 0-4 of (C).
;         ld      C,A             ;this is the extent byte.
;         ld      A,(hl)          ;now get the extra extent byte.
;         RRA
;         RRA
;         RRA
;         RRA
;         and     0FH
;         ld      B,A             ;and save it in (B).
;         pop     af              ;get record number back to (A).
;         inc     hl              ;check overflow byte 'r2'.
;         ld      L,(hl)
;         inc     L
;         dec     L
;         ld      L,6             ;prepare for error.
;         jp      nz,POSITN5      ;out of disk space error.
;         ld      hl,32           ;store record number into fcb.
;         ADD     hl,de
;         ld      (hl),A
;         ld      hl,12           ;and now check the extent byte.
;         ADD     hl,de
;         ld      A,C
;         sub     (hl)            ;same extent as before?
;         jp      nz,POSITN2
;         ld      hl,14           ;yes, check extra extent byte 's2' also.
;         ADD     hl,de
;         ld      A,B
;         sub     (hl)
;         and     $7F
;         jp      Z,POSITN3       ;same, we are almost done then.

;         ;  Get here when another extent is required.
; POSITN2:push    bc
;         push    de
;         call    CLOSEIT         ;close current extent.
;         pop     de
;         pop     bc
;         ld      L,3             ;prepare for error.
;         ld      A,(STATUS)
;         inc     A
;         jp      Z,POSITN4       ;close error.
;         ld      hl,12           ;put desired extent into fcb now.
;         ADD     hl,de
;         ld      (hl),C
;         ld      hl,14           ;and store extra extent byte 's2'.
;         ADD     hl,de
;         ld      (hl),B
;         call    OPENIT          ;try and get this extent.
;         ld      A,(STATUS)      ;was it there?
;         inc     A
;         jp      nz,POSITN3
;         pop     bc              ;no. can we create a new one (writing?).
;         push    bc
;         ld      L,4             ;prepare for error.
;         inc     C
;         jp      Z,POSITN4       ;nope, reading unwritten space error.
;         call    GETEMPTY        ;yes we can, try to find space.
;         ld      L,5             ;prepare for error.
;         ld      A,(STATUS)
;         inc     A
;         jp      Z,POSITN4       ;out of space?

;         ;   Normal return location. Clear error code and return.
; POSITN3:pop     bc              ;restore stack.
;         xor     A               ;and clear error code byte.
;         jp      SETSTAT

;         ;   Error. Set the 's2' byte to indicate this (why?).
; POSITN4:push    hl
;         call    GETS2
;         ld      (hl),0C0H
;         pop     hl

;         ;   Return with error code (presently in L).
; POSITN5:pop     bc
;         ld      A,L             ;get error code.
;         ld      (STATUS),A
;         jp      SETS2B7

;-----------------------------------------------------------------------------
;   Read a random record.
;-----------------------------------------------------------------------------
; READRAN:ld      C,0FFH          ;set 'read' status.
;         call    POSITION        ;position the file to proper record.
;         call    Z,RDSEQ1        ;and read it as usual (if no errors).
;         ret

;-----------------------------------------------------------------------------
;   Write to a random record.
;-----------------------------------------------------------------------------
; WRITERAN: ld    C,0             ;set 'writing' flag.
;         call    POSITION        ;position the file to proper record.
;         call    Z,WTSEQ1        ;and write as usual (if no errors).
;         ret

;-----------------------------------------------------------------------------
;   Compute the random record number. Enter with (hl) pointing
; to a fcb an (de) contains a relative location of a record
; number. On exit, (C) contains the 'r0' byte, (B) the 'r1'
; byte, and (A) the 'r2' byte.
;
;   On return, the zero flag is set if the record is within
; bounds. Otherwise, an overflow occured.
;-----------------------------------------------------------------------------
; COMPRAND: EX    de,hl           ;save fcb pointer in (de).
;         ADD     hl,de           ;compute relative position of record #.
;         ld      C,(hl)          ;get record number into (bc).
;         ld      B,0
;         ld      hl,12           ;now get extent.
;         ADD     hl,de
;         ld      A,(hl)          ;compute (bc)=(record #)+(extent)*128.
;         RRCA                    ;move lower bit into bit 7.
;         and     80H             ;and ignore all other bits.
;         ADD     A,C             ;add to our record number.
;         ld      C,A
;         ld      A,0             ;take care of any carry.
;         ADC     A,B
;         ld      B,A
;         ld      A,(hl)          ;now get the upper bits of extent into
;         RRCA                    ;bit positions 0-3.
;         and     0FH             ;and ignore all others.
;         ADD     A,B             ;add this in to 'r1' byte.
;         ld      B,A
;         ld      hl,14           ;get the 's2' byte (extra extent).
;         ADD     hl,de
;         ld      A,(hl)
;         ADD     A,A             ;and shift it left 4 bits (bits 4-7).
;         ADD     A,A
;         ADD     A,A
;         ADD     A,A
;         push    af              ;save carry flag (bit 0 of flag byte).
;         ADD     A,B             ;now add extra extent into 'r1'.
;         ld      B,A
;         push    af              ;and save carry (overflow byte 'r2').
;         pop     hl              ;bit 0 of (L) is the overflow indicator.
;         ld      A,L
;         pop     hl              ;and same for first carry flag.
;         or      L               ;either one of these set?
;         and     01H             ;only check the carry flags.
;         ret

;-----------------------------------------------------------------------------
;   Routine to setup the fcb (bytes 'r0', 'r1', 'r2') to
; reflect the last record used for a random (or other) file.
; This reads the directory and looks at all extents computing
; the largerst record number for each and keeping the maximum
; value only. Then 'r0', 'r1', and 'r2' will reflect this
; maximum record number. This is used to compute the space used
; by a random file.
;-----------------------------------------------------------------------------
; RANSIZE:ld      C,12            ;look thru directory for first entry with
;         call    FINDFST         ;this name.
;         ld      hl,(PARAMS)     ;zero out the 'r0, r1, r2' bytes.
;         ld      de,33
;         ADD     hl,de
;         push    hl
;         ld      (hl),D          ;note that (D)=0.
;         inc     hl
;         ld      (hl),D
;         inc     hl
;         ld      (hl),D
; RANSIZ1:call    CKFILPOS        ;is there an extent to process?
;         jp      Z,RANSIZ3       ;no, we are done.
;         call    FCB2HL          ;set (hl) pointing to proper fcb in dir.
;         ld      de,15           ;point to last record in extent.
;         call    COMPRAND        ;and compute random parameters.
;         pop     hl
;         push    hl              ;now check these values against those
;         ld      E,A             ;already in fcb.
;         ld      A,C             ;the carry flag will be set if those
;         sub     (hl)            ;in the fcb represent a larger size than
;         inc     hl              ;this extent does.
;         ld      A,B
;         SBC     (hl)
;         inc     hl
;         ld      A,E
;         SBC     (hl)
;         jp      C,RANSIZ2
;         ld      (hl),E          ;we found a larger (in size) extent.
;         dec     hl              ;stuff these values into fcb.
;         ld      (hl),B
;         dec     hl
;         ld      (hl),C
; RANSIZ2:call    FINDNXT         ;now get the next extent.
;         jp      RANSIZ1         ;continue til all done.
; RANSIZ3:pop     hl              ;we are done, restore the stack and
;         ret                     ;return.

;-----------------------------------------------------------------------------
; [36] SETTRAN - Function to return the random record position of a given
;                file which has been read in sequential mode up to now.
;-----------------------------------------------------------------------------
SETRAN:
        jp SETRAN ; FIXME

        ; ld      hl,(PARAMS)     ;point to fcb.
        ; ld      de,32           ;and to last used record.
        ; call    COMPRAND        ;compute random position.
        ; ld      hl,33           ;now stuff these values into fcb.
        ; ADD     hl,de
        ; ld      (hl),C          ;move 'r0'.
        ; inc     hl
        ; ld      (hl),B          ;and 'r1'.
        ; inc     hl
        ; ld      (hl),A          ;and lastly 'r2'.
        ; ret

;-----------------------------------------------------------------------------
;   This routine select the drive specified in (ACTIVE) and
; update the login bitmap and bitmap table if this drive was
; not already active.
;-----------------------------------------------------------------------------
LOGINDRV:
        ; ld      hl,(LOGIN)      ; get the login bitmap.
        ; ld      A,(ACTIVE)      ; get the default drive.
        ; ld      C,A
        ; call    SHIFTR          ; position active bit for this drive
        ; push    hl              ; into bit 0.
        ; ex      de,hl
        ; call    SELECT          ; select this drive.
        ; pop     hl
        ; call    z,SLCTERR       ; valid drive?
        ; ld      a,l             ; is this a newly activated drive?
        ; rra
        ; ret     c
        ld      hl,(LOGIN)      ; yes, update the login bitmap.
        ld      c,l
        ld      b,h
        call    SETBIT
        ld      (LOGIN),hl      ; and save.
        ; jp      BITMAP          ; now update the bitmap.
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
; of the fcb is examined for a drive specification. If non
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
        ld      A,(hl)          ;and save first byte of fcb also.
        ld      (AUTOFLAG),A    ;this must be non-zero.
        and     0E0H            ;whats this for (bits 6,7 are used for
        ld      (hl),A          ;something)?
        call    SETDSK          ;select and log in this drive.
AUTOSL1:ld      A,(USERNO)      ;move user number into fcb.
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
        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    CLEARS2         ;clear 's2' byte.
        ; call    AUTOSEL         ;select proper disk.
        ; jp      OPENIT          ;and open the file.

;-----------------------------------------------------------------------------
; [16] CLOSEFIL - Function to close a specified file.
;-----------------------------------------------------------------------------
CLOSEFIL:
        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper disk.
        ; jp      CLOSEIT         ;and close the file.

;-----------------------------------------------------------------------------
; [17] GETFST - Function to return the first occurence of a specified file
;               name. If the first byte of the fcb is '?' then the name will
;               not be checked (get the first entry no matter what).
;
; DE=Address of FCB
;-----------------------------------------------------------------------------
GETFST:
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
        ; TODO check result

        jr      _getnxt

;-----------------------------------------------------------------------------
; [18] GETNXT - Function to return the next occurence of a file name.
;-----------------------------------------------------------------------------
GETNXT:
        ; FIXME
        ; ld      a,$FF
        ; jp      SETSTAT

        ; ld      hl,(SAVEFCB)    ;restore pointers. note that no
        ; ld      (PARAMS),hl     ;other dbos calls are allowed.
        ; call    AUTOSEL         ;no error will be returned, but the
        ; call    FINDNXT         ;results will be wrong.
        ; jp      MOVEDIR

_getnxt:
        ; Read entry
        call    _read_dirent
        jr      nz,.err

        ld      de, (USERDMA)
        ld      a, (USERNO)
        ld      (de),a
        inc     de

        ld      hl,_dirent_name
        ld      bc,11
        ldir

        ex      de,hl

        ld      a,$AA
        ld      bc,20
        call    _memset

        ld      a,0
        jp      SETSTAT

.err:   ld      a,$FF
        jp      SETSTAT



;-----------------------------------------------------------------------------
; [19] DELFILE - Function to delete a file by name.
;-----------------------------------------------------------------------------
DELFILE:
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
        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive then read.
        ; jp      RDSEQ

;-----------------------------------------------------------------------------
; [21] WRTSEQ - Function to write the next sequential record.
;-----------------------------------------------------------------------------
WRTSEQ:
        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive then write.
        ; jp      WTSEQ

;-----------------------------------------------------------------------------
; [22] FCREATE - Create a file function.
;-----------------------------------------------------------------------------
FCREATE:
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
        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call  AUTOSEL         ;select proper drive and read.
        ; jp      READRAN

;-----------------------------------------------------------------------------
; [34] WTRANDOM - Function to compute the file size for random files.
;-----------------------------------------------------------------------------
WTRANDOM:
        ; FIXME
        ld      a,$FF
        jp      SETSTAT

        ; call    AUTOSEL         ;select proper drive and write.
        ; jp      WRITERAN

;-----------------------------------------------------------------------------
; [35] FILESIZE - Function to compute the size of a random file.
;-----------------------------------------------------------------------------
FILESIZE:
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
        ld      (hl),0          ; (* reset first byte of fcb *)
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
        jr      .ext

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
;* BDOS data storage pool.
;-----------------------------------------------------------------------------
; EMPTYFCB: defb  $E5             ;empty directory segment indicator.
WRTPRT:   defw  0               ;write protect status for all 16 drives.
LOGIN:    defw  0               ;drive active word (1 bit per drive).
USERDMA:  defw  $80             ;user's DMA address (defaults to 80h).

_dirent:
_dirent_date: defw 0
_dirent_time: defw 0
_dirent_attr: defb 0
_dirent_size: defd 0
_dirent_name: ds 8
_dirent_ext:  ds 3
_dirent_end:

;-----------------------------------------------------------------------------
; Scratch areas from parameter block.
;-----------------------------------------------------------------------------
SCRATCH1: defw  0               ;relative position within dir segment for file (0-3).
SCRATCH2: defw  0               ;last selected track number.
SCRATCH3: defw  0               ;last selected sector number.

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
;SECTORS:  defw  0               ;sectors per track from bios.
;BLKSHFT:  defb  0               ;block shift.
;BLKMASK:  defb  0               ;block mask.
;EXTMASK:  defb  0               ;extent mask.
;DSKSIZE:  defw  0               ;disk size from bios (number of blocks-1).
DIRSIZE:  defw  0               ;directory size.
;ALLOC0:   defw  0               ;storage for first bytes of bit map (dir space used).
;ALLOC1:   defw  0
;OFFSET:   defw  0               ;first usable track number.
;XLATE:    defw  0               ;sector translation table address.

;CLOSEFLG: defb  0               ;close flag (=0ffh is extent written ok).
;RDWRTFLG: defb  0               ;read/write flag (0ffh=read, 0=write).
FNDSTAT:  defb  0               ;filename found status (0=found first entry).
;MODE:     defb  0               ;I/o mode select (0=random, 1=sequential, 2=special random).
EPARAM:   defb  0               ;storage for register (E) on entry to BDOS.
;RELBLOCK: defb  0               ;relative position within fcb of block number written.
;COUNTER:  defb  0               ;byte counter for directory name searches.
;SAVEFCB:  defw  0,0             ;save space for address of fcb (for directory searches).
;BIGDISK:  defb  0               ;if =0 then disk is > 256 blocks long.
AUTO:     defb  0               ;if non-zero, then auto select activated.
OLDDRV:   defb  0               ;on auto select, storage for previous drive.
AUTOFLAG: defb  0               ;if non-zero, then auto select changed drives.
;SAVNXT:   defb  0               ;storage for next record number to access.
;SAVEXT:   defb  0               ;storage for extent number of file.
;SAVNREC:  defw  0               ;storage for number of records in file.
;BLKNMBR:  defw  0               ;block number (physical sector) used within a file or logical sect
;LOGSECT:  defw  0               ;starting logical (128 byte) sector of block (physical sector).
FCBPOS:   defb  0               ;relative position within buffer for fcb of file of interest.
;FILEPOS:  defw  0               ;files position within directory (0 to max entries -1).

;-----------------------------------------------------------------------------
; Disk directory buffer checksum bytes. One for each of the 16 possible drives.
;-----------------------------------------------------------------------------
; CKSUMTBL: defb  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0











;-----------------------------------------------------------------------------
; Issue command to ESP
;-----------------------------------------------------------------------------
esp_cmd:
    push    a

    ; Drain RX FIFO
.drain:
    in      a, (IO_ESPCTRL)
    and     a, 1
    jr      z, .done
    in      a, (IO_ESPDATA)
    jr      .drain
.done:

    ; Issue start of command
    ld      a, $80
    out     (IO_ESPCTRL), a

    ; Issue command
    pop     a
    jp      esp_send_byte

;-----------------------------------------------------------------------------
; Wait for data from ESP
;-----------------------------------------------------------------------------
esp_get_byte:
.wait:
    in      a, (IO_ESPCTRL)
    and     a, 1
    jr      z, .wait
    in      a, (IO_ESPDATA)
    ret

;-----------------------------------------------------------------------------
; Write data to ESP
;-----------------------------------------------------------------------------
esp_send_byte:
    push    a

.wait:
    in      a, (IO_ESPCTRL)
    and     a, 2
    jr      nz, .wait

    pop     a
    out     (IO_ESPDATA), a
    ret

;-----------------------------------------------------------------------------
; Get bytes
; Input:  HL: destination address
;         DE: number of bytes to read
;-----------------------------------------------------------------------------
esp_get_bytes:
.loop:
    ; Done reading? (DE=0)
    ld      a, d
    or      a, e
    ret     z

    call    esp_get_byte
    ld      (hl), a
    inc     hl
    dec     de
    jr      .loop

;-----------------------------------------------------------------------------
; Get bytes
; Input:  HL: destination address
;         DE: number of bytes to read
;-----------------------------------------------------------------------------
esp_get_bytes_zeroterm:
.loop:
    ; Done reading? (DE=0)
    ld      a, d
    or      a, e
    ret     z

    call    esp_get_byte
    ld      (hl), a
    inc     hl
    dec     de
    or      a
    ret     z
    jr      .loop

;-----------------------------------------------------------------------------
; _memset
;-----------------------------------------------------------------------------
_memset:
    inc     c
    inc     b
    jr      .start
.repeat:
    ld      (hl), a
    inc     hl
.start:
    dec     c
    jr      nz, .repeat
    dec     b
    jr      nz, .repeat
    ret
