;-----------------------------------------------------------------------------
; bdos.inc - Basic Disk Operating System
;-----------------------------------------------------------------------------
FBASE:
    jp      _bdos_entry

;-----------------------------------------------------------------------------
; BDOS error table
;-----------------------------------------------------------------------------
BADSCTR:    defw    _err_bad_sect   ; bad sector on read or write.
BADSLCT:    defw    _err_bad_sel    ; bad disk select.
RODISK:     defw    _err_disk_ro    ; disk is read only.
ROFILE:     defw    _err_file_ro    ; file is read only.

;-----------------------------------------------------------------------------
; Entry into BDOS. (de) or (E) are the parameters passed. The
; function number desired is in register (C).
;-----------------------------------------------------------------------------
_bdos_entry:
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
NFUNCTS:    equ 41          ; number of functions in following table.
FUNCTNS:
    defw    WBOOT,GETCON,OUTCON,GETRDR,PUNCH,LIST,DIRCIO,GETIOB
    defw    SETIOB,PRTSTR,RDBUFF,GETCSTS,GETVER,RSTDSK,SETDSK,OPENFIL
    defw    CLOSEFIL,GETFST,GETNXT,DELFILE,READSEQ,WRTSEQ,FCREATE
    defw    RENFILE,GETLOG,GETCRNT,PUTDMA,GETALOC,WRTPRTD,GETROV,SETATTR
    defw    GETPARM,GETUSER,RDRANDOM,WTRANDOM,FILESIZE,SETRAN,LOGOFF,RTN
    defw    RTN,WTSPECL

;-----------------------------------------------------------------------------
; Error: Bad Sector
;-----------------------------------------------------------------------------
_err_bad_sect:
    ld      hl,BADSEC       ; bad sector message.
    call    _print_err      ; print it and get a 1 char response.
    cp      CTRL_C          ; re-boot request (control-c)?
    jp      z,0             ; yes.
    ret                     ; no, return to retry i/o function.

;-----------------------------------------------------------------------------
; Error: Bad Drive Selected
;-----------------------------------------------------------------------------
_err_bad_sel:
    ld      hl,BADSEL       ; bad drive selected.
    jp      _print_err_reboot

;-----------------------------------------------------------------------------
; Error: Disk is Read Only
;-----------------------------------------------------------------------------
_err_disk_ro:
    ld      hl,DISKRO   ;disk is read only.
    jp      _print_err_reboot

;-----------------------------------------------------------------------------
; Error: File is Read Only
;-----------------------------------------------------------------------------
_err_file_ro:
    ld      hl,FILERO       ; file is read only.

;-----------------------------------------------------------------------------
; Print error, followed by reboot
;-----------------------------------------------------------------------------
_print_err_reboot:
    call    _print_err
    jp      0               ; always reboot on these errors.

;-----------------------------------------------------------------------------
; Error strings
;-----------------------------------------------------------------------------
BDOSERR:    defb    'Bdos Err On '
BDOSDRV:    defb    ' : $'
BADSEC:     defb    'Bad Sector$'
BADSEL:     defb    'Select$'
FILERO:     defb    'File '
DISKRO:     defb    'R/O$'

;-----------------------------------------------------------------------------
; Print BDOS error message.
;-----------------------------------------------------------------------------
_print_err:
    push    hl              ; save second message pointer.
    call    OUTCRLF         ; send CRLF.
    ld      a,(ACTIVE)      ; get active drive.
    add     a,'A'           ; make ASCII.
    ld      (BDOSDRV),a     ; and put in message.
    ld      bc,BDOSERR      ; and print it.
    call    PRTMESG
    pop     bc              ; print second message line now.
    call    PRTMESG

;-----------------------------------------------------------------------------
; Get an input character. We will check our 1 character
; buffer first. This may be set by the console status routine.
;-----------------------------------------------------------------------------
GETCHAR:
    ld      hl,CHARBUF      ; check character buffer.
    ld      a,(hl)          ; anything present already?
    ld      (hl),0          ; ...either case clear it.
    or      a
    ret     nz              ; yes, use it.
    jp      CONIN           ; nope, go get a character response.

;-----------------------------------------------------------------------------
; Input and echo a character.
;-----------------------------------------------------------------------------
GETECHO:
    call    GETCHAR         ; input a character.
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
CHKCHAR:
    cp      CR              ; check for carriage return, line feed, backspace,
    ret     z               ; or a tab.
    cp      LF
    ret     z
    cp      TAB
    ret     z
    cp      BS
    ret     z
    cp      ' '             ; other control char? Set carry flag.
    ret

;-----------------------------------------------------------------------------
; Check the console during output. Halt on a control-s, then
; reboot on a control-c. If anything else is ready, clear the
; zero flag and return (the calling routine may want to do
; something).
;-----------------------------------------------------------------------------
CKCONSOL:
    ld      a,(CHARBUF)     ; check buffer.
    or      a               ; if anything, just return without checking.
    jp      nz,.2
    call    CONST           ; nothing in buffer. Check console.
    and     $01             ; look at bit 0.
    ret     z               ; return if nothing.
    call    CONIN           ; ok, get it.
    cp      CTRL_S          ; if not control-s, return with zero cleared.
    jp      nz,.1
    call    CONIN           ; halt processing until another char
    cp      CTRL_C          ; is typed. Control-c?
    jp      z,0             ; yes, reboot now.
    xor     a               ; no, just pretend nothing was ever ready.
    ret
.1: ld      (CHARBUF),a     ; save character in buffer for later processing.
.2: ld      a,1             ; set (A) to non zero to mean something is ready.
    ret

;-----------------------------------------------------------------------------
; Output (C) to the screen. If the printer flip-flop flag is set, we will
; send character to printer also. The console will be checked in the process.
;-----------------------------------------------------------------------------
OUTCHAR:
    ld      a,(OUTFLAG)     ; check output flag.
    or      a               ; anything and we won't generate output.
    jp      nz,.1
    push    bc
    call    CKCONSOL        ; check console (we don't care whats there).
    pop     bc
    push    bc
    call    CONOUT          ; output (C) to the screen.
    pop     bc
    push    bc
    ld      a,(PRTFLAG)     ; check printer flip-flop flag.
    or      a
    call    nz,LIST         ; print it also if non-zero.
    pop     bc
.1: ld      a,c             ; update cursors position.
    ld      hl,CURPOS
    cp      DEL             ; rubouts don't do anything here.
    ret     z
    inc     (hl)            ; bump line pointer.
    cp      ' '             ; and return if a normal character.
    ret     nc
    dec     (hl)            ; restore and check for the start of the line.
    ld      a,(hl)
    or      a
    ret     z               ; ingnore control characters at the start of the line.
    ld      a,c
    cp      BS              ; is it a backspace?
    jp      nz,.2
    dec     (hl)            ; yes, backup pointer.
    ret
.2: cp      LF              ; is it a line feed?
    ret     nz              ; ignore anything else.
    ld      (hl),0          ; reset pointer to start of line.
    ret

;-----------------------------------------------------------------------------
; Output (A) to the screen. If it is a control character (other than
; carriage control), use ^x format.
;-----------------------------------------------------------------------------
SHOWIT:
    ld      a,c
    call    CHKCHAR         ; check character.
    jp      nc,OUTCON       ; not a control, use normal output.
    push    af
    ld      c,'^'           ; for a control character, preceed it with '^'.
    call    OUTCHAR
    pop     af
    or      '@'             ; and then use the letter equivelant.
    ld      c,a

    ; fall-through into OUTCON

;-----------------------------------------------------------------------------
; [ 2] OUTCON - Function to output (C) to the console device and expand tabs
;               if necessary.
;-----------------------------------------------------------------------------
OUTCON:
    ld      a,c
    cp      TAB             ; is it a tab?
    jp      nz,OUTCHAR      ; use regular output.
.1: ld      c,' '           ; yes it is, use spaces instead.
    call    OUTCHAR
    ld      a,(CURPOS)      ; go until the cursor is at a multiple of 8
    and     $07             ; position.
    jp      nz,.1
    ret

;-----------------------------------------------------------------------------
; Echo a backspace character. Erase the prevoius character on the screen.
;-----------------------------------------------------------------------------
BACKUP:
    call    .1              ; backup the screen 1 place.
    ld      c,' '           ; then blank that character.
    call    CONOUT
.1: ld      c,BS            ; then back space once more.
    jp      CONOUT

;-----------------------------------------------------------------------------
; Signal a deleted line. Print a '#' at the end and start over.
;-----------------------------------------------------------------------------
NEWLINE:
    ld      c,'#'
    call    OUTCHAR         ; print this.
    call    OUTCRLF         ; start new line.
.1: ld      a,(CURPOS)      ; move the cursor to the starting position.
    ld      hl,STARTING
    cp      (hl)
    ret     nc              ; there yet?
    ld      c,' '
    call    OUTCHAR         ; nope, keep going.
    jp      .1

;-----------------------------------------------------------------------------
; Output a CRLF to the console device (screen).
;-----------------------------------------------------------------------------
OUTCRLF:
    ld      c,CR
    call    OUTCHAR
    ld      c,LF
    jp      OUTCHAR

;-----------------------------------------------------------------------------
; Print message pointed to by (bc). It will end with a '$'.
;-----------------------------------------------------------------------------
PRTMESG:
    ld      a,(bc)          ; check for terminating character.
    cp      '$'
    ret     z
    inc     bc
    push    bc              ; otherwise, bump pointer and print it.
    ld      c,a
    call    OUTCON
    pop     bc
    jp      PRTMESG

;-----------------------------------------------------------------------------
; [10] RDBUFF - Function to execute a buffered read.
;-----------------------------------------------------------------------------
RDBUFF:
    ld      a,(CURPOS)      ; use present location as starting one.
    ld      (STARTING),a
    ld      hl,(PARAMS)     ; get the maximum buffer space.
    ld      c,(hl)
    inc     hl              ; point to first available space.
    push    hl              ; and save.
    ld      b,0             ; keep a character count.
.1: push    bc
    push    hl
.2: call    GETCHAR         ; get the next input character.
    and     $7F             ; strip bit 7.
    pop     hl              ; reset registers.
    pop     bc
    cp      CR              ; en of the line?
    jp      z,.17
    cp      LF
    jp      z,.17
    cp      BS              ; how about a backspace?
    jp      nz,.3
    ld      a,b             ; yes, but ignore at the beginning of the line.
    or      a
    jp      z,.1
    dec     b               ; ok, update counter.
    ld      a,(CURPOS)      ; if we backspace to the start of the line,
    ld      (OUTFLAG),a     ; treat as a cancel (control-x).
    jp      .10
.3: cp      DEL             ; user typed a rubout?
    jp      nz,.4
    ld      a,b             ; ignore at the start of the line.
    or      a
    jp      z,.1
    ld      a,(hl)          ; ok, echo the previous character.
    dec     b               ; and reset pointers (counters).
    dec     hl
    jp      .15
.4: cp      CTRL_E          ; physical end of line?
    jp      nz,.5
    push    bc              ; yes, do it.
    push    hl
    call    OUTCRLF
    xor     a               ; and update starting position.
    ld      (STARTING),a
    jp      .2
.5: cp      CTRL_P          ; control-p?
    jp      nz,.6
    push    hl              ; yes, flip the print flag filp-flop byte.
    ld      hl,PRTFLAG
    ld      a,1             ; PRTFLAG=1-PRTFLAG
    sub     (hl)
    ld      (hl),a
    pop     hl
    jp      .1
.6: cp      CTRL_X          ; control-x (cancel)?
    jp      nz,.8
    pop     hl
.7: ld      a,(STARTING)    ; yes, backup the cursor to here.
    ld      hl,CURPOS
    cp      (hl)
    jp      nc,RDBUFF       ; done yet?
    dec     (hl)            ; no, decrement pointer and output back up one space.
    call    BACKUP
    jp      .7
.8: cp      CTRL_U          ; control-U (cancel line)?
    jp      nz,.9
    call    NEWLINE         ; start a new line.
    pop     hl
    jp      RDBUFF
.9: cp      CTRL_R          ; control-r?
    jp      nz,.14
.10:
    push    bc              ; yes, start a new line and retype the old one.
    call    NEWLINE
    pop     bc
    pop     hl
    push    hl
    push    bc
.11:
    ld      a,b             ; done whole line yet?
    or      a
    jp      z,.12
    inc     hl              ; nope, get next character.
    ld      c,(hl)
    dec     b               ; count it.
    push    bc
    push    hl
    call    SHOWIT          ; and display it.
    pop     hl
    pop     bc
    jp      .11
.12:
    push    hl              ; done with line. If we were displaying
    ld      a,(OUTFLAG)     ; then update cursor position.
    or      a
    jp      z,.2
    ld      hl,CURPOS       ; because this line is shorter, we must
    sub     (hl)            ; back up the cursor (not the screen however)
    ld      (OUTFLAG),a     ; some number of positions.
.13:
    call    BACKUP          ; note that as long as (OUTFLAG) is non
    ld      hl,OUTFLAG      ; zero, the screen will not be changed.
    dec     (hl)
    jp      nz,.13
    jp      .2              ; now just get the next character.

    ; Just a normal character, put this in our buffer and echo.
.14:
    inc     hl
    ld      (hl),a          ; store character.
    inc     b               ; and count it.
.15:
    push    bc
    push    hl
    ld      c,a             ; echo it now.
    call    SHOWIT
    pop     hl
    pop     bc
    ld      a,(hl)          ; was it an abort request?
    cp      CTRL_C          ; control-c abort?
    ld      a,b
    jp      nz,.16
    cp      1               ; only if at start of line.
    jp      z,0
.16:
    cp      c               ; nope, have we filled the buffer?
    jp      c,.1
.17:
    pop     hl              ; yes end the line and return.
    ld      (hl),b
    ld      c,CR
    jp      OUTCHAR         ; output CR and return.

;-----------------------------------------------------------------------------
; [ 1] GETCON - Function to get a character from the console device.
;-----------------------------------------------------------------------------
GETCON:
    call    GETECHO         ; get and echo.
    jp      SETSTAT         ; save status and return.

;-----------------------------------------------------------------------------
; [ 3] GETRDR - Function to get a character from the tape reader device.
;-----------------------------------------------------------------------------
GETRDR:
    call    READER          ; get a character from reader, set status and return.
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [ 6] DIRCIO - Function to perform direct console i/o. If (C) contains (FF)
;               then this is an input request. If (C) contains (FE) then this
;               is a status request. Otherwise we are to output (C).
;-----------------------------------------------------------------------------
DIRCIO:
    ld      a,c             ; test for (FF).
    inc     a
    jp      z,.1
    inc     a               ; test for (FE).
    jp      z,CONST
    jp      CONOUT          ; just output (C).
.1: call    CONST           ; this is an input request.
    or      a
    jp      z,_bdos_exit2   ; not ready? Just return (directly).
    call    CONIN           ; yes, get character.
    jp      SETSTAT         ; set status and return.

;-----------------------------------------------------------------------------
; [ 7] GETIOB - Function to return the i/o byte.
;-----------------------------------------------------------------------------
GETIOB:
    ld      a,(IOBYTE)
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [ 8] SETIOB - Function to set the i/o byte.
;-----------------------------------------------------------------------------
SETIOB:
    ld      hl,IOBYTE
    ld      (hl),c
    ret

;-----------------------------------------------------------------------------
; [ 9] PRTSTR - Function to print the character string pointed to by (de)
;               on the console device. The string ends with a '$'.
;-----------------------------------------------------------------------------
PRTSTR:
    ex      de,hl
    ld      c,l
    ld      b,h             ; now (bc) points to it.
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
SETSTAT:
    ld      (STATUS),a
RTN:
    ret

;-----------------------------------------------------------------------------
; Set the status to 1 (read or write error code).
;-----------------------------------------------------------------------------
_ret_1:
    ld      a,1
    jp      SETSTAT

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
OUTFLAG:    defb    0   ; output flag (non zero means no output).
STARTING:   defb    2   ; starting position for cursor.
CURPOS:     defb    0   ; cursor position (0=start of line).
PRTFLAG:    defb    0   ; printer flag (control-p toggle). List if non zero.
CHARBUF:    defb    0   ; single input character buffer.

;-----------------------------------------------------------------------------
; Stack area for BDOS calls.
;-----------------------------------------------------------------------------
USRSTACK:   defw    0   ; save users stack pointer here.
            defs    48  ; stack area
STKAREA     equ     $   ; end of stack area.

USERNO:     defb    0   ; current user number.
ACTIVE:     defb    0   ; currently active drive.
PARAMS:     defw    0   ; save (de) parameters here on entry.
STATUS:     defw    0   ; status returned from bdos function.

;-----------------------------------------------------------------------------
; Select error occured, jump to error routine.
;-----------------------------------------------------------------------------
SLCTERR:
    ld      hl,BADSLCT
    ; fall-through into JUMPHL

;-----------------------------------------------------------------------------
; Jump to (hl) indirectly.
;-----------------------------------------------------------------------------
JUMPHL:
    ld      e,(hl)
    inc     hl
    ld      d,(hl)      ; now (de) contain the desired address.
    ex      de,hl
    jp      (hl)

;-----------------------------------------------------------------------------
; Block move. (de) to (hl), (C) bytes total.
;-----------------------------------------------------------------------------
DE2HL:
    inc     c           ; is count down to zero?
.1: dec     c
    ret     z           ; yes, we are done.
    ld      a,(de)      ; no, move one more byte.
    ld      (hl),a
    inc     de
    inc     hl
    jp      .1          ; and repeat.

;-----------------------------------------------------------------------------
; Select the desired drive.
;-----------------------------------------------------------------------------
SELECT:
    ld      a,(ACTIVE)      ; get active disk.
    ld      c,a
    call    SELDSK          ; select it.
    ld      a,h             ; valid drive?
    or      l               ; valid drive?
    ret     z               ; return if not.

    ; Here, the BIOS returned the address of the parameter block
    ; in (hl). We will extract the necessary pointers and save them.
    ld      e,(hl)          ; yes, get address of translation table into (de).
    inc     hl
    ld      d,(hl)
    inc     hl
    ld      (SCRATCH1),hl   ; save pointers to scratch areas.
    inc     hl
    inc     hl
    ld      (SCRATCH2),hl   ; ditto.
    inc     hl
    inc     hl
    ld      (SCRATCH3),hl   ; ditto.
    inc     hl
    inc     hl
    ex      de,hl           ; now save the translation table address.
    ld      (XLATE),hl
    ld      hl,DIRBUF       ; put the next 8 bytes here.
    ld      c,8             ; they consist of the directory buffer
    call    DE2HL           ; pointer, parameter block pointer,
    ld      hl,(DISKPB)     ; check and allocation vectors.
    ex      de,hl
    ld      hl,SECTORS      ; move parameter block into our ram.
    ld      c,15            ; it is 15 bytes long.
    call    DE2HL
    ld      hl,(DSKSIZE)    ; check disk size.
    ld      a,h             ; more than 256 blocks on this?
    ld      hl,BIGDISK
    ld      (hl),$FF        ; set to samll.
    or      a
    jp      z,.1
    ld      (hl),0          ; wrong, set to large.
.1: ld      a,$FF           ; clear the zero flag.
    or      a
    ret

;-----------------------------------------------------------------------------
; Routine to home the disk track head and clear pointers.
;-----------------------------------------------------------------------------
HOMEDRV:
    call    HOME            ; home the head.
    xor     a
    ld      hl,(SCRATCH2)   ; set our track pointer also.
    ld      (hl),a
    inc     hl
    ld      (hl),a
    ld      hl,(SCRATCH3)   ; and our sector pointer.
    ld      (hl),a
    inc     hl
    ld      (hl),a
    ret

;-----------------------------------------------------------------------------
; Do the actual disk read and check the error return status.
;-----------------------------------------------------------------------------
DOREAD:
    call    READ
    jp      IORET

;-----------------------------------------------------------------------------
; Do the actual disk write and handle any bios error.
;-----------------------------------------------------------------------------
DOWRITE:
    call    WRITE
IORET:
    or      a
    ret     z                   ; return unless an error occured.
    ld      hl,BADSCTR          ; bad read/write on this sector.
    jp      JUMPHL

;-----------------------------------------------------------------------------
; Routine to select the track and sector that the desired
; block number falls in.
;-----------------------------------------------------------------------------
TRKSEC:
    ld      hl,(FILEPOS)        ; get position of last accessed file
    ld      c,2                 ; in directory and compute sector #.
    call    SHIFTR              ; sector #=file-position/4.
    ld      (BLKNMBR),hl        ; save this as the block number of interest.
    ld      (CKSUMTBL),hl       ; what's it doing here too?

    ; if the sector number has already been set (BLKNMBR), enter at this point.
TRKSEC1:
    ld      hl,BLKNMBR
    ld      c,(hl)              ; move sector number into (bc).
    inc     hl
    ld      b,(hl)
    ld      hl,(SCRATCH3)       ; get current sector number and
    ld      e,(hl)              ; move this into (de).
    inc     hl
    ld      d,(hl)
    ld      hl,(SCRATCH2)       ; get current track number.
    ld      a,(hl)              ; and this into (hl).
    inc     hl
    ld      h,(hl)
    ld      l,a
.2: ld      a,c                 ; is desired sector before current one?
    sub     e
    ld      a,b
    sbc     a,d
    jp      nc,.3
    push    hl                  ; yes, decrement sectors by one track.
    ld      hl,(SECTORS)        ; get sectors per track.
    ld      a,e
    sub     l
    ld      e,a
    ld      a,d
    sbc     a,h
    ld      d,a                 ; now we have backed up one full track.
    pop     hl
    dec     hl                  ; adjust track counter.
    jp      .2
.3: push    hl                  ; desired sector is after current one.
    ld      hl,(SECTORS)        ; get sectors per track.
    add     hl,de               ; bump sector pointer to next track.
    jp      c,.4
    ld      a,c                 ; is desired sector now before current one?
    sub     l
    ld      a,b
    sbc     a,h
    jp      c,.4
    ex      de,hl               ; not yes, increment track counter
    pop     hl                  ; and continue until it is.
    inc     hl
    jp      .3

    ; here we have determined the track number that contains the
    ; desired sector.
.4: pop     hl                  ; get track number (hl).
    push    bc
    push    de
    push    hl
    ex      de,hl
    ld      hl,(OFFSET)         ; adjust for first track offset.
    add     hl,de
    ld      b,h
    ld      c,l
    call    SETTRK              ; select this track.
    pop     de                  ; reset current track pointer.
    ld      hl,(SCRATCH2)
    ld      (hl),e
    inc     hl
    ld      (hl),d
    pop     de
    ld      hl,(SCRATCH3)       ; reset the first sector on this track.
    ld      (hl),e
    inc     hl
    ld      (hl),d
    pop     bc
    ld      a,c                 ; now subtract the desired one.
    sub     e                   ; to make it relative (1-# sectors/track).
    ld      c,a
    ld      a,b
    sbc     a,d
    ld      b,a
    ld      hl,(XLATE)          ; translate this sector according to this table.
    ex      de,hl
    call    SECTRN              ; let the bios translate it.
    ld      c,l
    ld      b,h
    jp      SETSEC              ; and select it.

;-----------------------------------------------------------------------------
; Compute block number from record number (SAVNREC) and extent number (SAVEXT).
;-----------------------------------------------------------------------------
GETBLOCK:
    ld      hl,BLKSHFT          ; get logical to physical conversion.
    ld      c,(hl)              ; note that this is base 2 log of ratio.
    ld      a,(SAVNREC)         ; get record number.
.1: or      a                   ; compute (A)=(A)/2^BLKSHFT.
    rra
    dec     c
    jp      nz,.1
    ld      b,a                 ; save result in (B).
    ld      a,8
    sub     (hl)
    ld      c,a                 ; compute (C)=8-BLKSHFT.
    ld      a,(SAVEXT)
.2: dec     c                   ; compute (A)=SAVEXT*2^(8-BLKSHFT).
    jp      z,.3
    or      a
    rla
    jp      .2
.3: add     a,b
    ret

;-----------------------------------------------------------------------------
; Routine to extract the (bc) block byte from the fcb pointed to by (PARAMS).
; If this is a big-disk, then these are 16 bit block numbers, else they are
; 8 bit numbers.
; Number is returned in (hl).
;-----------------------------------------------------------------------------
EXTBLK:
    ld      hl,(PARAMS)         ; get fcb address.
    ld      de,16               ; block numbers start 16 bytes into fcb.
    add     hl,de
    add     hl,bc
    ld      a,(BIGDISK)         ; are we using a big-disk?
    or      a
    jp      z,.1
    ld      l,(hl)              ; no, extract an 8 bit number from the fcb.
    ld      h,0
    ret
.1: add     hl,bc               ; yes, extract a 16 bit number.
    ld      e,(hl)
    inc     hl
    ld      d,(hl)
    ex      de,hl               ; return in (hl).
    ret

;-----------------------------------------------------------------------------
; Compute block number.
;-----------------------------------------------------------------------------
COMBLK:
    call    GETBLOCK
    ld      c,a
    ld      b,0
    call    EXTBLK
    ld      (BLKNMBR),hl
    ret

;-----------------------------------------------------------------------------
; Check for a zero block number (unused).
;-----------------------------------------------------------------------------
CHKBLK:
    ld      hl,(BLKNMBR)
    ld      a,l                 ; is it zero?
    or      h
    ret

;-----------------------------------------------------------------------------
; Adjust physical block (BLKNMBR) and convert to logical
; sector (LOGSECT). This is the starting sector of this block.
; The actual sector of interest is then added to this and the
; resulting sector number is stored back in (BLKNMBR). This
; will still have to be adjusted for the track number.
;-----------------------------------------------------------------------------
LOGICAL:
    ld       a,(BLKSHFT)        ; get log2(physical/logical sectors).
    ld       hl,(BLKNMBR)       ; get physical sector desired.
.1: add      hl,hl              ; compute logical sector number.
    dec      a                  ; note logical sectors are 128 bytes long.
    jp       nz,.1
    ld       (LOGSECT),hl       ; save logical sector.
    ld       a,(BLKMASK)        ; get block mask.
    ld       c,a
    ld       a,(SAVNREC)        ; get next sector to access.
    and      c                  ; extract the relative position within physical block.
    or       l                  ; and add it too logical sector.
    ld       l,a
    ld       (BLKNMBR),hl       ; and store.
    ret     

;-----------------------------------------------------------------------------
; Set (hl) to point to extent byte in fcb.
;-----------------------------------------------------------------------------
SETEXT:
    ld      hl,(PARAMS)
    ld      de,12           ; it is the twelth byte.
    add     hl,de
    ret

;-----------------------------------------------------------------------------
; Set (hl) to point to record count byte in fcb and (de) to
; next record number byte.
;-----------------------------------------------------------------------------
SETHLDE:
    ld      hl,(PARAMS)
    ld      de,15           ; record count byte (#15).
    add     hl,de
    ex      de,hl
    ld      hl,17           ; next record number (#32).
    add     hl,de
    ret

;-----------------------------------------------------------------------------
; Save current file data from fcb.
;-----------------------------------------------------------------------------
STRDATA:
    call    SETHLDE
    ld      a,(hl)          ; get and store record count byte.
    ld      (SAVNREC),a
    ex      de,hl
    ld      a,(hl)          ; get and store next record number byte.
    ld      (SAVNXT),a
    call    SETEXT          ; point to extent byte.
    ld      a,(EXTMASK)     ; get extent mask.
    and     (hl)
    ld      (SAVEXT),a      ; and save extent here.
    ret

;-----------------------------------------------------------------------------
; Set the next record to access. If (MODE) is set to 2, then the last record
; byte (SAVNREC) has the correct number to access.
; For sequential access, (MODE) will be equal to 1.
;-----------------------------------------------------------------------------
SETNREC:
    call    SETHLDE
    ld      a,(MODE)        ; get sequential flag (=1).
    cp      2               ; a 2 indicates that no adder is needed.
    jp      nz,.1
    xor     a               ; clear adder (random access?).
.1: ld      c,a
    ld      a,(SAVNREC)     ; get last record number.
    add     a,c             ; increment record count.
    ld      (hl),a          ; and set fcb's next record byte.
    ex      de,hl
    ld      a,(SAVNXT)      ; get next record byte from storage.
    ld      (hl),a          ; and put this into fcb as number of records used.
    ret

;-----------------------------------------------------------------------------
; Shift (hl) right (C) bits.
;-----------------------------------------------------------------------------
SHIFTR:
    inc     c
.1: dec     c
    ret     z
    ld      a,h
    or      a
    rra
    ld      h,a
    ld      a,l
    rra
    ld      l,a
    jp      .1

;-----------------------------------------------------------------------------
; Compute the check-sum for the directory buffer. Return integer sum in (A).
;-----------------------------------------------------------------------------
CHECKSUM:
    ld      c,128           ; length of buffer.
    ld      hl,(DIRBUF)     ; get its location.
    xor     a               ; clear summation byte.
.1: add     a,(hl)          ; and compute sum ignoring carries.
    inc     hl
    dec     c
    jp      nz,.1
    ret

;-----------------------------------------------------------------------------
; Shift (hl) left (C) bits.
;-----------------------------------------------------------------------------
SHIFTL:
    inc     c
.1: dec     c
    ret     z
    add     hl,hl           ; shift left 1 bit.
    jp      .1

;-----------------------------------------------------------------------------
; Routine to set a bit in a 16 bit value contained in (bc).
; The bit set depends on the current drive selection.
;-----------------------------------------------------------------------------
SETBIT:
    push    bc              ; save 16 bit word.
    ld      a,(ACTIVE)      ; get active drive.
    ld      c,a
    ld      hl,1
    call    SHIFTL          ; shift bit 0 into place.
    pop     bc              ; now 'or' this with the original word.
    ld      a,c
    or      l
    ld      l,a             ; low byte done, do high byte.
    ld      a,b
    or      h
    ld      h,a
    ret

;-----------------------------------------------------------------------------
; Extract the write protect status bit for the current drive.
; The result is returned in (A), bit 0.
;-----------------------------------------------------------------------------
GETWPRT:
    ld      hl,(WRTPRT)     ; get status bytes.
    ld      a,(ACTIVE)      ; which drive is current?
    ld      c,a
    call    SHIFTR          ; shift status such that bit 0 is the
    ld      a,l             ; one of interest for this drive.
    and     $01             ; and isolate it.
    ret

;-----------------------------------------------------------------------------
; Function to write protect the current disk.
;-----------------------------------------------------------------------------
WRTPRTD:
    ld      hl,WRTPRT       ; point to status word.
    ld      c,(hl)          ; set (bc) equal to the status.
    inc     hl
    ld      b,(hl)
    call    SETBIT          ; and set this bit according to current drive.
    ld      (WRTPRT),hl     ; then save.
    ld      hl,(DIRSIZE)    ; now save directory size limit.
    inc     hl              ; remember the last one.
    ex      de,hl
    ld      hl,(SCRATCH1)   ; and store it here.
    ld      (hl),e          ; put low byte.
    inc     hl
    ld      (hl),d          ; then high byte.
    ret

;-----------------------------------------------------------------------------
; Check for a read only file.
;-----------------------------------------------------------------------------
CHKROFL:
    call    FCB2HL          ; set (hl) to file entry in directory buffer.
CKROF1:
    ld      de,9            ; look at bit 7 of the ninth byte.
    add     hl,de
    ld      a,(hl)
    rla
    ret     nc              ; return if ok.
    ld      hl,ROFILE       ; else, print error message and terminate.
    jp      JUMPHL

;-----------------------------------------------------------------------------
;   Check the write protect status of the active disk.
;-----------------------------------------------------------------------------
CHKWPRT:
    call    GETWPRT
    ret     z               ; return if ok.
    ld      hl,RODISK       ; else print message and terminate.
    jp      JUMPHL

;-----------------------------------------------------------------------------
; Routine to set (hl) pointing to the proper entry in the directory buffer.
;-----------------------------------------------------------------------------
FCB2HL:
    ld      hl,(DIRBUF)     ; get address of buffer.
    ld      a,(FCBPOS)      ; relative position of file.

;-----------------------------------------------------------------------------
; Routine to add (A) to (hl).
;-----------------------------------------------------------------------------
ADDA2HL:
    add     a,l
    ld      l,a
    ret     nc
    inc     h               ; take care of any carry.
    ret

;-----------------------------------------------------------------------------
; Routine to get the 's2' byte from the fcb supplied in
; the initial parameter specification.
;-----------------------------------------------------------------------------
GETS2:
    ld      hl,(PARAMS)     ; get address of fcb.
    ld      de,14           ; relative position of 's2'.
    add     hl,de
    ld      a,(hl)          ; extract this byte.
    ret

;-----------------------------------------------------------------------------
; Clear the 's2' byte in the fcb.
;-----------------------------------------------------------------------------
CLEARS2:
    call    GETS2           ; this sets (hl) pointing to it.
    ld      (hl),0          ; now clear it.
    ret

;-----------------------------------------------------------------------------
; Set bit 7 in the 's2' byte of the fcb.
;-----------------------------------------------------------------------------
SETS2B7:
    call    GETS2           ; get the byte.
    or      $80             ; and set bit 7.
    ld      (hl),a          ; then store.
    ret

;-----------------------------------------------------------------------------
; Compare (FILEPOS) with (SCRATCH1) and set flags based on
; the difference. This checks to see if there are more file
; names in the directory. We are at (FILEPOS) and there are
; (SCRATCH1) of them to check.
;-----------------------------------------------------------------------------
MOREFLS:
    ld      hl,(FILEPOS)    ; we are here.
    ex      de,hl
    ld      hl,(SCRATCH1)   ; and don't go past here.
    ld      a,e             ; compute difference but don't keep.
    sub     (hl)
    inc     hl
    ld      a,d
    sbc     (hl)            ; set carry if no more names.
    ret

;-----------------------------------------------------------------------------
; Call this routine to prevent (SCRATCH1) from being greater than (FILEPOS).
;-----------------------------------------------------------------------------
CHKNMBR:
    call    MOREFLS     ; SCRATCH1 too big?
    ret     c
    inc     de          ; yes, reset it to (FILEPOS).
    ld      (hl),d
    dec     hl
    ld      (hl),e
    ret

;-----------------------------------------------------------------------------
; Compute (hl)=(de)-(hl)
;-----------------------------------------------------------------------------
SUBHL:
    ld      a,e         ; compute difference.
    sub     l
    ld      l,a         ; store low byte.
    ld      a,d
    sbc     a,h
    ld      h,a         ; and then high byte.
    ret

;-----------------------------------------------------------------------------
; Set the directory checksum byte.
;-----------------------------------------------------------------------------
SETDIR:
    ld      c,$FF

    ; Routine to set or compare the directory checksum byte. If
    ; (C)=$FF, then this will set the checksum byte. Else the byte
    ; will be checked. If the check fails (the disk has been changed),
    ; then this disk will be write protected.
CHECKDIR:
    ld      hl,(CKSUMTBL)
    ex      de,hl
    ld      hl,(ALLOC1)
    call    SUBHL
    ret     nc                  ; ok if (CKSUMTBL) > (ALLOC1), so return.
    push    bc
    call    CHECKSUM            ; else compute checksum.
    ld      hl,(CHKVECT)        ; get address of checksum table.
    ex      de,hl
    ld      hl,(CKSUMTBL)
    add     hl,de               ; set (hl) to point to byte for this drive.
    pop     bc
    inc     c                   ; set or check ?
    jp      z,.1
    cp      (hl)                ; check them.
    ret     z                   ; return if they are the same.
    call    MOREFLS             ; not the same, do we care?
    ret     nc
    call    WRTPRTD             ; yes, mark this as write protected.
    ret
.1: ld      (hl),a              ; just set the byte.
    ret

;-----------------------------------------------------------------------------
; Do a write to the directory of the current disk.
;-----------------------------------------------------------------------------
DIRWRITE:
    call    SETDIR          ; set checksum byte.
    call    DIRDMA          ; set directory DMA address.
    ld      c,1             ; tell the bios to actually write.
    call    DOWRITE         ; then do the write.
    jp      DEFDMA

;-----------------------------------------------------------------------------
; Read from the directory.
;-----------------------------------------------------------------------------
DIRREAD
    call    DIRDMA          ; set the directory DMA address.
    call    DOREAD          ; and read it.

;-----------------------------------------------------------------------------
; Routine to set the DMA address to the users choice.
;-----------------------------------------------------------------------------
DEFDMA:
    ld      hl,USERDMA      ; reset the default DMA address and return.
    jp      DIRDMA1

;-----------------------------------------------------------------------------
; Routine to set the DMA address for directory work.
;-----------------------------------------------------------------------------
DIRDMA:
    ld      hl,DIRBUF

;-----------------------------------------------------------------------------
; Set the DMA address. On entry, (hl) points to
; word containing the desired DMA address.
;-----------------------------------------------------------------------------
DIRDMA1:
    ld      c,(hl)
    inc     hl
    ld      b,(hl)          ; setup (bc) and go to the bios to set it.
    jp      SETDMA

;-----------------------------------------------------------------------------
; Move the directory buffer into user's DMA space.
;-----------------------------------------------------------------------------
MOVEDIR:
    ld      hl,(DIRBUF)     ; buffer is located here, and
    ex      de,hl
    ld      hl,(USERDMA)    ; put it here.
    ld      c,128           ; this is its length.
    jp      DE2HL           ; move it now and return.

;-----------------------------------------------------------------------------
; Check (FILEPOS) and set the zero flag if it equals $FFFF.
;-----------------------------------------------------------------------------
CKFILPOS:
    ld      hl,FILEPOS
    ld      a,(hl)
    inc     hl
    cp      (hl)            ; are both bytes the same?
    ret     nz
    inc     a               ; yes, but are they each $FF?
    ret

;-----------------------------------------------------------------------------
; Set location (FILEPOS) to $FFFF.
;-----------------------------------------------------------------------------
STFILPOS:
    ld      hl,$FFFF
    ld      (FILEPOS),hl
    ret

;-----------------------------------------------------------------------------
; Move on to the next file position within the current directory buffer.
; If no more exist, set pointer to $FFFF and the calling routine will check
; for this. Enter with (C) equal to $FF to cause the checksum byte to be set,
; else we will check this disk and set write protect if checksums are not the
; same (applies only if another directory sector must be read).
;-----------------------------------------------------------------------------
NXENTRY:
    ld       hl,(DIRSIZE)       ; get directory entry size limit.
    ex       de,hl
    ld       hl,(FILEPOS)       ; get current count.
    inc      hl                 ; go on to the next one.
    ld       (FILEPOS),hl
    call     SUBHL              ; (hl)=(DIRSIZE)-(FILEPOS)
    jp       nc,.1              ; is there more room left?
    jp       STFILPOS           ; no. Set this flag and return.
.1: ld       a,(FILEPOS)        ; get file position within directory.
    and      $03                ; only look within this sector (only 4 entries fit).
    ld       b,5                ; convert to relative position (32 bytes each).
.2: add      a,a                ; note that this is not efficient code.
    dec      b                  ; 5 'add A's would be better.
    jp       nz,.2
    ld       (FCBPOS),a         ; save it as position of fcb.
    or       a
    ret      nz                 ; return if we are within buffer.
    push     bc
    call     TRKSEC             ; we need the next directory sector.
    call     DIRREAD
    pop      bc
    jp       CHECKDIR

;-----------------------------------------------------------------------------
; Routine to to get a bit from the disk space allocation map. It is returned
; in (A), bit position 0. On entry to here, set (bc) to the block number on
; the disk to check. On return, (D) will contain the original bit position
; for this block number and (hl) will point to the address for it.
;-----------------------------------------------------------------------------
CKBITMAP:
    ld      a,c         ; determine bit number of interest.
    and     $07         ; compute (D)=(E)=(C and 7)+1.
    inc     a
    ld      e,a         ; save particular bit number.
    ld      d,a

    ; compute (bc)=(bc)/8.
    ld      a,c
    rrca                ; now shift right 3 bits.
    rrca
    rrca
    and     $1F         ; and clear bits 7,6,5.
    ld      c,a
    ld      a,b
    add     a,a         ; now shift (B) into bits 7,6,5.
    add     a,a
    add     a,a
    add     a,a
    add     a,a
    or      c           ; and add in (C).
    ld      c,a         ; ok, (C) ha been completed.
    ld      a,b         ; is there a better way of doing this?
    rrca
    rrca
    rrca
    and     $1F
    ld      b,a         ; and now (B) is completed.

    ; use this as an offset into the disk space allocation table.
    ld      hl,(ALOCVECT)
    add     hl,bc
    ld      a,(hl)      ; now get correct byte.
.1: rlca                ; get correct bit into position 0.
    dec     e
    jp      nz,.1
    ret

;-----------------------------------------------------------------------------
; Set or clear the bit map such that block number (bc) will be marked
; as used. On entry, if (E)=0 then this bit will be cleared, if it equals
; 1 then it will be set (don't use anyother values).
;-----------------------------------------------------------------------------
STBITMAP:
    push    de
    call    CKBITMAP    ; get the byte of interest.
    and     $FE         ; clear the affected bit.
    pop     bc
    or      c           ; and now set it acording to (C).

    ; entry to restore the original bit position and then store
    ; in table. (A) contains the value, (D) contains the bit
    ; position (1-8), and (hl) points to the address within the
    ; space allocation table for this byte.
STBMAP1:
    rrca                ; restore original bit position.
    dec     d
    jp      nz,STBMAP1
    ld      (hl),a      ; and stor byte in table.
    ret

;-----------------------------------------------------------------------------
; Set/clear space used bits in allocation map for this file.
; On entry, (C)=1 to set the map and (C)=0 to clear it.
;-----------------------------------------------------------------------------
SETFILE:
    call    FCB2HL          ; get address of fcb
    ld      de,16
    add     hl,de           ; get to block number bytes.
    push    bc
    ld      c,17            ; check all 17 bytes (max) of table.
.1: pop     de
    dec     c               ; done all bytes yet?
    ret     z
    push    de
    ld      a,(BIGDISK)     ; check disk size for 16 bit block numbers.
    or      a
    jp      z,.2
    push    bc              ; only 8 bit numbers. set (bc) to this one.
    push    hl
    ld      c,(hl)          ; get low byte from table, always
    ld      b,0             ; set high byte to zero.
    jp      .3
.2: dec     c               ; for 16 bit block numbers, adjust counter.
    push    bc
    ld      c,(hl)          ; now get both the low and high bytes.
    inc     hl
    ld      b,(hl)
    push    hl
.3: ld      a,c             ; block used?
    or      b
    jp      z,.4
    ld      hl,(DSKSIZE)    ; is this block number within the
    ld      a,l             ; space on the disk?
    sub     c
    ld      a,h
    sbc     a,b
    call    nc,STBITMAP     ; yes, set the proper bit.
.4: pop     hl              ; point to next block number in fcb.
    inc     hl
    pop     bc
    jp      .1

;-----------------------------------------------------------------------------
; Construct the space used allocation bit map for the active drive. If a
; file name starts with '$' and it is under the current user number, then
; (STATUS) is set to minus 1. Otherwise it is not set at all.
;-----------------------------------------------------------------------------
BITMAP:
    ld      hl,(DSKSIZE)    ; compute size of allocation table.
    ld      c,3
    call    SHIFTR          ; (hl)=(hl)/8.
    inc     hl              ; at lease 1 byte.
    ld      b,h
    ld      c,l             ; set (bc) to the allocation table length.

    ; Initialize the bitmap for this drive. Right now, the first
    ; two bytes are specified by the disk parameter block. However
    ; a patch could be entered here if it were necessary to setup
    ; this table in a special mannor. For example, the bios could
    ; determine locations of 'bad blocks' and set them as already
    ; 'used' in the map.
    ld      hl,(ALOCVECT)   ; now zero out the table now.
.1: ld      (hl),0
    inc     hl
    dec     bc
    ld      a,b
    or      c
    jp      nz,.1
    ld      hl,(ALLOC0)     ; get initial space used by directory.
    ex      de,hl
    ld      hl,(ALOCVECT)   ; and put this into map.
    ld      (hl),e
    inc     hl
    ld      (hl),d

    ; End of initialization portion.
    call    HOMEDRV         ; now home the drive.
    ld      hl,(SCRATCH1)
    ld      (hl),3          ; force next directory request to read
    inc     hl              ; in a sector.
    ld      (hl),0
    call    STFILPOS        ; clear initial file position also.
.2: ld      c,$FF           ; read next file name in directory
    call    NXENTRY         ; and set checksum byte.
    call    CKFILPOS        ; is there another file?
    ret     z
    call    FCB2HL          ; yes, get its address.
    ld      a,$E5
    cp      (hl)            ; empty file entry?
    jp      z,.2
    ld      a,(USERNO)      ; no, correct user number?
    cp      (hl)
    jp      nz,.3
    inc     hl
    ld      a,(hl)          ; yes, does name start with a '$'?
    sub     '$'
    jp      nz,.3
    dec     a               ; yes, set atatus to minus one.
    ld      (STATUS),a
.3: ld      c,1             ; now set this file's space as used in bit map.
    call    SETFILE
    call    CHKNMBR         ; keep (SCRATCH1) in bounds.
    jp      .2

;-----------------------------------------------------------------------------
; Set the status (STATUS) and return.
;-----------------------------------------------------------------------------
STSTATUS:
    ld      a,(FNDSTAT)
    jp      SETSTAT

;-----------------------------------------------------------------------------
; Check extents in (A) and (C). Set the zero flag if they are the same.
; The number of 16k chunks of disk space that the directory extent covers
; is expressad is (EXTMASK+1). No registers are modified.
;-----------------------------------------------------------------------------
SAMEXT:
    push    bc
    push    af
    ld      a,(EXTMASK)     ; get extent mask and use it to
    cpl                     ; to compare both extent numbers.
    ld      b,a             ; save resulting mask here.
    ld      a,c             ; mask first extent and save in (C).
    and     b
    ld      c,a
    pop     af              ; now mask second extent and compare
    and     b               ; with the first one.
    sub     c
    and     $1F             ; (* only check buts 0-4 *)
    pop     bc              ; the zero flag is set if they are the same.
    ret                     ; restore (bc) and return.

;-----------------------------------------------------------------------------
; Search for the first occurence of a file name. On entry, register (C)
; should contain the number of bytes of the fcb that must match.
;-----------------------------------------------------------------------------
FINDFST:
    ld      a,$FF
    ld      (FNDSTAT),a
    ld      hl,COUNTER      ; save character count.
    ld      (hl),c
    ld      hl,(PARAMS)     ; get filename to match.
    ld      (SAVEFCB),hl    ; and save.
    call    STFILPOS        ; clear initial file position (set to $FFFF).
    call    HOMEDRV         ; home the drive.

;-----------------------------------------------------------------------------
; Entry to locate the next occurence of a filename within the
; directory. The disk is not expected to have been changed. If
; it was, then it will be write protected.
;-----------------------------------------------------------------------------
FINDNXT:
    ld      c,0             ; write protect the disk if changed.
    call    NXENTRY         ; get next filename entry in directory.
    call    CKFILPOS        ; is file position = $FFFF?
    jp      z,.6            ; yes, exit now then.
    ld      hl,(SAVEFCB)    ; set (de) pointing to filename to match.
    ex      de,hl
    ld      a,(de)
    cp      $E5             ; empty directory entry?
    jp      z,.1            ; (* are we trying to reserect erased entries? *)
    push    de
    call    MOREFLS         ; more files in directory?
    pop     de
    jp      nc,.6           ; no more. Exit now.
.1: call    FCB2HL          ; get address of this fcb in directory.
    ld      a,(COUNTER)     ; get number of bytes (characters) to check.
    ld      c,a
    ld      b,0             ; initialize byte position counter.
.2: ld      a,c             ; are we done with the compare?
    or      a
    jp      z,.5
    ld      a,(de)          ; no, check next byte.
    cp      '?'             ; don't care about this character?
    jp      z,.4
    ld      a,b             ; get bytes position in fcb.
    cp      13              ; don't care about the thirteenth byte either.
    jp      z,.4
    cp      12              ; extent byte?
    ld      a,(de)
    jp      z,.3
    sub     (hl)            ; otherwise compare characters.
    and     $7F
    jp      nz,FINDNXT      ; not the same, check next entry.
    jp      .4              ; so far so good, keep checking.
.3: push    bc              ; check the extent byte here.
    ld      c,(hl)
    call    SAMEXT
    pop     bc
    jp      nz,FINDNXT      ; not the same, look some more.
    ; So far the names compare. Bump pointers to the next byte
    ; and continue until all (C) characters have been checked.
.4: inc     de              ; bump pointers.
    inc     hl
    inc     b
    dec     c               ; adjust character counter.
    jp      .2
.5: ld      a,(FILEPOS)     ; return the position of this entry.
    and     $03
    ld      (STATUS),a
    ld      hl,FNDSTAT
    ld      a,(hl)
    rla
    ret     nc
    xor     a
    ld      (hl),a
    ret
    ; Filename was not found. Set appropriate status.
.6: call    STFILPOS        ; set (FILEPOS) to $FFFF.
    ld      a,$FF           ; say not located.
    jp      SETSTAT

;-----------------------------------------------------------------------------
; Erase files from the directory. Only the first byte of the
; fcb will be affected. It is set to (E5).
;-----------------------------------------------------------------------------
ERAFILE:
    call    CHKWPRT         ; is disk write protected?
    ld      c,12            ; only compare file names.
    call    FINDFST         ; get first file name.
.1: call    CKFILPOS        ; any found?
    ret     z               ; nope, we must be done.
    call    CHKROFL         ; is file read only?
    call    FCB2HL          ; nope, get address of fcb and
    ld      (hl),0E5H       ; set first byte to 'empty'.
    ld      c,0             ; clear the space from the bit map.
    call    SETFILE
    call    DIRWRITE        ; now write the directory sector back out.
    call    FINDNXT         ; find the next file name.
    jp      .1              ; and repeat process.

;-----------------------------------------------------------------------------
; Look through the space allocation map (bit map) for the
; next available block. Start searching at block number (bc-1).
; The search procedure is to look for an empty block that is
; before the starting block. If not empty, look at a later
; block number. In this way, we return the closest empty block
; on either side of the 'target' block number. This will speed
; access on random devices. For serial devices, this should be
; changed to look in the forward direction first and then start
; at the front and search some more.
;
; On return, (de)= block number that is empty and (hl) =0
; if no empry block was found.
;-----------------------------------------------------------------------------
FNDSPACE:
    ld      d,b             ; set (de) as the block that is checked.
    ld      e,c

    ; Look before target block. Registers (bc) are used as the lower
    ; pointer and (de) as the upper pointer.
.1: ld      a,c             ; is block 0 specified?
    or      b
    jp      z,.2
    dec     bc              ; nope, check previous block.
    push    de
    push    bc
    call    CKBITMAP
    rra                     ; is this block empty?
    jp      nc,.3           ; yes. use this.

    ; Note that the above logic gets the first block that it finds
    ; that is empty. Thus a file could be written 'backward' making
    ; it very slow to access. This could be changed to look for the
    ; first empty block and then continue until the start of this
    ; empty space is located and then used that starting block.
    ; This should help speed up access to some files especially on
    ; a well used disk with lots of fairly small 'holes'.
    pop     bc              ; nope, check some more.
    pop     de

    ;   Now look after target block.
.2: ld      hl,(DSKSIZE)    ; is block (de) within disk limits?
    ld      a,e
    sub     L
    ld      A,D
    sbc     a,h
    jp      nc,.4
    inc     de              ; yes, move on to next one.
    push    bc
    push    de
    ld      b,d
    ld      c,e
    call    CKBITMAP        ; check it.
    rra                     ; empty?
    jp      nc,.3
    pop     de              ; nope, continue searching.
    pop     bc
    jp      .1

    ; Empty block found. Set it as used and return with (hl)
    ; pointing to it (true?).
.3: rla                     ; reset byte.
    inc     a               ; and set bit 0.
    call    STBMAP1         ; update bit map.
    pop     hl              ; set return registers.
    pop     de
    ret

    ; Free block was not found. If (bc) is not zero, then we have
    ; not checked all of the disk space.
.4: ld      a,c
    or      b
    jp      nz,.1
    ld      hl,0            ; set 'not found' status.
    ret

;-----------------------------------------------------------------------------
; Move a complete fcb entry into the directory and write it.
;-----------------------------------------------------------------------------
FCBSET:
    ld      c,0
    ld      e,32            ; length of each entry.

;-----------------------------------------------------------------------------
; Move (E) bytes from the fcb pointed to by (PARAMS) into fcb in directory
; starting at relative byte (C). This updated directory buffer is then
; written to the disk.
;-----------------------------------------------------------------------------
UPDATE:
    push    de
    ld      b,0             ; set (bc) to relative byte position.
    ld      hl,(PARAMS)     ; get address of fcb.
    add     hl,bc           ; compute starting byte.
    ex      de,hl
    call    FCB2HL          ; get address of fcb to update in directory.
    pop     bc              ; set (C) to number of bytes to change.
    call    DE2HL
UPDATE1:
    call    TRKSEC          ; determine the track and sector affected.
    jp      DIRWRITE        ; then write this sector out.

;-----------------------------------------------------------------------------
; Routine to change the name of all files on the disk with a specified name.
; The fcb contains the current name as the first 12 characters and the new
; name 16 bytes into the fcb.
;-----------------------------------------------------------------------------
CHGNAMES:
    call    CHKWPRT         ; check for a write protected disk.
    ld      c,12            ; match first 12 bytes of fcb only.
    call    FINDFST         ; get first name.
    ld      hl,(PARAMS)     ; get address of fcb.
    ld      a,(hl)          ; get user number.
    ld      de,16           ; move over to desired name.
    add     hl,de
    ld      (hl),a          ; keep same user number.
.1: call    CKFILPOS        ; any matching file found?
    ret     z               ; no, we must be done.
    call    CHKROFL         ; check for read only file.
    ld      c,16            ; start 16 bytes into fcb.
    ld      e,12            ; and update the first 12 bytes of directory.
    call    UPDATE
    call    FINDNXT         ; get te next file name.
    jp      .1              ; and continue.

;-----------------------------------------------------------------------------
; Update a files attributes. The procedure is to search for every file with
; the same name as shown in fcb (ignoring bit 7) and then to update it
; (which includes bit 7). No other changes are made.
;-----------------------------------------------------------------------------
SAVEATTR:
    ld      c,12            ; match first 12 bytes.
    call    FINDFST         ; look for first filename.
.1: call    CKFILPOS        ; was one found?
    ret     z               ; nope, we must be done.
    ld      c,0             ; yes, update the first 12 bytes now.
    ld      e,12
    call    UPDATE          ; update filename and write directory.
    call    FINDNXT         ; and get the next file.
    jp      .1              ; then continue until done.

;-----------------------------------------------------------------------------
; Open a file (name specified in fcb).
;-----------------------------------------------------------------------------
OPENIT:
    ld      c,15            ; compare the first 15 bytes.
    call    FINDFST         ; get the first one in directory.
    call    CKFILPOS        ; any at all?
    ret     z
OPENIT1:
    call    SETEXT          ; point to extent byte within users fcb.
    ld      a,(hl)          ; and get it.
    push    af              ; save it and address.
    push    hl
    call    FCB2HL          ; point to fcb in directory.
    ex      de,hl
    ld      hl,(PARAMS)     ; this is the users copy.
    ld      c,32            ; move it into users space.
    push    de
    call    DE2HL
    call    SETS2B7         ; set bit 7 in 's2' byte (unmodified).
    pop     de              ; now get the extent byte from this fcb.
    ld      hl,12
    add     hl,de
    ld      c,(hl)          ; into (C).
    ld      hl,15           ; now get the record count byte into (B).
    add     hl,de
    ld      b,(hl)
    pop     hl              ; keep the same extent as the user had originally.
    pop     af
    ld      (hl),a
    ld      a,c             ; is it the same as in the directory fcb?
    cp      (hl)
    ld      a,b             ; if yes, then use the same record count.
    jp      z,.2
    ld      a,0             ; if the user specified an extent greater than
    jp      c,.2            ; the one in the directory, then set record count to 0.
    ld      a,128           ; otherwise set to maximum.
.2: ld      hl,(PARAMS)     ; set record count in users fcb to (A).
    ld      de,15
    add     hl,de           ; compute relative position.
    ld      (hl),a          ; and set the record count.
    ret

;-----------------------------------------------------------------------------
; Move two bytes from (de) to (hl) if (and only if) (hl) point to a zero
; value (16 bit). Return with zero flag set it (de) was moved. Registers (de)
; and (hl) are not changed. However (A) is.
;-----------------------------------------------------------------------------
MOVEWORD:
    ld      a,(hl)          ; check for a zero word.
    inc     hl
    or      (hl)            ; both bytes zero?
    dec     hl
    ret     nz              ; nope, just return.
    ld      a,(de)          ; yes, move two bytes from (de) into
    ld      (hl),a          ; this zero space.
    inc     de
    inc     hl
    ld      a,(de)
    ld      (hl),a
    dec     de              ; don't disturb these registers.
    dec     hl
    ret

;-----------------------------------------------------------------------------
; Get here to close a file specified by (fcb).
;-----------------------------------------------------------------------------
CLOSEIT:
    xor     a               ; clear status and file position bytes.
    ld      (STATUS),a
    ld      (FILEPOS),a
    ld      (FILEPOS+1),a
    call    GETWPRT         ; get write protect bit for this drive.
    ret     nz              ; just return if it is set.
    call    GETS2           ; else get the 's2' byte.
    and     $80             ; and look at bit 7 (file unmodified?).
    ret     nz              ; just return if set.
    ld      c,15            ; else look up this file in directory.
    call    FINDFST
    call    CKFILPOS        ; was it found?
    ret     z               ; just return if not.
    ld      bc,16           ; set (hl) pointing to records used section.
    call    FCB2HL
    add     hl,bc
    ex      de,hl
    ld      hl,(PARAMS)     ; do the same for users specified fcb.
    add     hl,bc
    ld      c,16            ; this many bytes are present in this extent.
.1: ld      a,(BIGDISK)     ; 8 or 16 bit record numbers?
    or      a
    jp      z,.4
    ld      a,(hl)          ; just 8 bit. Get one from users fcb.
    or      a
    ld      a,(de)          ; now get one from directory fcb.
    jp      nz,.2
    ld      (hl),a          ; users byte was zero. Update from directory.
.2: or      a
    jp      nz,.3
    ld      a,(hl)          ; directories byte was zero, update from users fcb.
    ld      (de),a
.3: cp      (hl)            ; if neither one of these bytes were zero,
    jp      nz,.7           ; then close error if they are not the same.
    jp      .5              ; ok so far, get to next byte in fcbs.
.4: call    MOVEWORD        ; update users fcb if it is zero.
    ex      de,hl
    call    MOVEWORD        ; update directories fcb if it is zero.
    ex      de,hl
    ld      a,(de)          ; if these two values are no different,
    cp      (hl)            ; then a close error occured.
    jp      nz,.7
    inc     de              ; check second byte.
    inc     hl
    ld      a,(de)
    cp      (hl)
    jp      nz,.7
    dec     c               ; remember 16 bit values.
.5: inc     de              ; bump to next item in table.
    inc     hl
    dec     c               ; there are 16 entries only.
    jp      nz,.1           ; continue if more to do.
    ld      bc,$FFEC        ; backup 20 places (extent byte).
    add     hl,bc
    ex      de,hl
    add     hl,bc
    ld      a,(de)
    cp      (hl)            ; directory's extent already greater than the
    jp      c,.6            ; users extent?
    ld      (hl),a          ; no, update directory extent.
    ld      bc,3            ; and update the record count byte in
    add     hl,bc           ; directories fcb.
    ex      de,hl
    add     hl,bc
    ld      a,(hl)          ; get from user.
    ld      (de),a          ; and put in directory.
.6: ld      a,$FF           ; set 'was open and is now closed' byte.
    ld      (CLOSEFLG),a
    jp      UPDATE1         ; update the directory now.
.7: ld      hl,STATUS       ; set return status and then return.
    dec     (hl)
    ret

;-----------------------------------------------------------------------------
; Routine to get the next empty space in the directory. It
; will then be cleared for use.
;-----------------------------------------------------------------------------
GETEMPTY:
    call    CHKWPRT         ; make sure disk is not write protected.
    ld      hl,(PARAMS)     ; save current parameters (fcb).
    push    hl
    ld      hl,EMPTYFCB     ; use special one for empty space.
    ld      (PARAMS),hl
    ld      c,1             ; search for first empty spot in directory.
    call    FINDFST         ; (* only check first byte *)
    call    CKFILPOS        ; none?
    pop     hl
    ld      (PARAMS),hl     ; restore original fcb address.
    ret     z               ; return if no more space.
    ex      de,hl
    ld      hl,15           ; point to number of records for this file.
    add     hl,de
    ld      c,17            ; and clear all of this space.
    xor     a
.1: ld      (hl),a
    inc     hl
    dec     c
    jp      nz,.1
    ld      hl,13           ; clear the 's1' byte also.
    add     hl,de
    ld      (hl),a
    call    CHKNMBR         ; keep (SCRATCH1) within bounds.
    call    FCBSET          ; write out this fcb entry to directory.
    jp      SETS2B7         ; set 's2' byte bit 7 (unmodified at present).

;-----------------------------------------------------------------------------
; Routine to close the current extent and open the next one for reading.
;-----------------------------------------------------------------------------
GETNEXT:
    xor     a
    ld      (CLOSEFLG),a    ; clear close flag.
    call    CLOSEIT         ; close this extent.
    call    CKFILPOS
    ret     z               ; not there???
    ld      hl,(PARAMS)     ; get extent byte.
    ld      bc,12
    add     hl,bc
    ld      a,(hl)          ; and increment it.
    inc     a
    and     $1F             ; keep within range 0-31.
    ld      (hl),a
    jp      z,.1            ; overflow?
    ld      b,a             ; mask extent byte.
    ld      a,(EXTMASK)
    and     b
    ld      hl,CLOSEFLG     ; check close flag ($FF is ok).
    and     (hl)
    jp      z,.2            ; if zero, we must read in next extent.
    jp      .3              ; else, it is already in memory.
.1: ld      bc,2            ; Point to the 's2' byte.
    add     hl,bc
    inc     (hl)            ; and bump it.
    ld      a,(hl)          ; too many extents?
    and     $0F
    jp      z,.5            ; yes, set error code.

    ; Get here to open the next extent.
.2: ld      c,15            ; set to check first 15 bytes of fcb.
    call    FINDFST         ; find the first one.
    call    CKFILPOS        ; none available?
    jp      nz,.3
    ld      a,(RDWRTFLG)    ; no extent present. Can we open an empty one?
    inc     a               ; $FF means reading (so not possible).
    jp      z,.5            ; or an error.
    call    GETEMPTY        ; we are writing, get an empty entry.
    call    CKFILPOS        ; none?
    jp      z,.5            ; error if true.
    jp      .4              ; else we are almost done.
.3: call    OPENIT1         ; open this extent.
.4: call    STRDATA         ; move in updated data (rec #, extent #, etc.)
    xor     a               ; clear status and return.
    jp      SETSTAT

    ; Error in extending the file. Too many extents were needed
    ; or not enough space on the disk.
.5: call    _ret_1          ; set error code, clear bit 7 of 's2'
    jp      SETS2B7         ; so this is not written on a close.

;-----------------------------------------------------------------------------
; Read a sequential file.
;-----------------------------------------------------------------------------
RDSEQ:
    ld      a,1             ; set sequential access mode.
    ld      (MODE),a
RDSEQ1:
    ld      a,$FF           ; don't allow reading unwritten space.
    ld      (RDWRTFLG),a
    call    STRDATA         ; put rec# and ext# into fcb.
    ld      a,(SAVNREC)     ; get next record to read.
    ld      hl,SAVNXT       ; get number of records in extent.
    cp      (hl)            ; within this extent?
    jp      c,.2
    cp      128             ; no. Is this extent fully used?
    jp      nz,.3           ; no. End-of-file.
    call    GETNEXT         ; yes, open the next one.
    xor     a               ; reset next record to read.
    ld      (SAVNREC),a
    ld      a,(STATUS)      ; check on open, successful?
    or      a
    jp      nz,.3           ; no, error.
.2: call    COMBLK          ; ok. compute block number to read.
    call    CHKBLK          ; check it. Within bounds?
    jp      z,.3            ; no, error.
    call    LOGICAL         ; convert (BLKNMBR) to logical sector (128 byte).
    call    TRKSEC1         ; set the track and sector for this block #.
    call    DOREAD          ; and read it.
    jp      SETNREC         ; and set the next record to be accessed.
    ; Read error occured. Set status and return.
.3: jp      _ret_1

;-----------------------------------------------------------------------------
; Write the next sequential record.
;-----------------------------------------------------------------------------
WTSEQ:
    ld      a,1             ; set sequential access mode.
    ld      (MODE),a
WTSEQ1:
    ld      a,0             ; allow an addition empty extent to be opened.
    ld      (RDWRTFLG),a
    call    CHKWPRT         ; check write protect status.
    ld      hl,(PARAMS)
    call    CKROF1          ; check for read only file, (hl) already set to fcb.
    call    STRDATA         ; put updated data into fcb.
    ld      a,(SAVNREC)     ; get record number to write.
    cp      128             ; within range?
    jp      nc,_ret_1       ; no, error(?).
    call    COMBLK          ; compute block number.
    call    CHKBLK          ; check number.
    ld      c,0             ; is there one to write to?
    jp      nz,.6           ; yes, go do it.
    call    GETBLOCK        ; get next block number within fcb to use.
    ld      (RELBLOCK),a    ; and save.
    ld      bc,0            ; start looking for space from the start
    or      a               ; if none allocated as yet.
    jp      z,.2
    ld      c,a             ; extract previous block number from fcb
    dec     bc              ; so we can be closest to it.
    call    EXTBLK
    ld      b,h
    ld      c,l
.2: call    FNDSPACE        ; find the next empty block nearest number (bc).
    ld      a,l             ; check for a zero number.
    or      h
    jp      nz,.3
    ld      a,2             ; no more space?
    jp      SETSTAT
.3: ld      (BLKNMBR),hl    ; save block number to access.
    ex      de,hl           ; put block number into (de).
    ld      hl,(PARAMS)     ; now we must update the fcb for this
    ld      bc,16           ; newly allocated block.
    add     hl,bc
    ld      a,(BIGDISK)     ; 8 or 16 bit block numbers?
    or      a
    ld      a,(RELBLOCK)    ; (* update this entry *)
    jp      z,.4            ; zero means 16 bit ones.
    call    ADDA2HL         ; (hl)=(hl)+(A)
    ld      (hl),e          ; store new block number.
    jp      .5
.4: ld      c,a             ; compute spot in this 16 bit table.
    ld      b,0
    add     hl,bc
    add     hl,bc
    ld      (hl),e          ; stuff block number (de) there.
    inc     hl
    ld      (hl),d
.5: ld      c,2             ; set (C) to indicate writing to un-used disk space.
.6: ld      a,(STATUS)      ; are we ok so far?
    or      a
    ret     nz
    push    bc              ; yes, save write flag for bios (register C).
    call    LOGICAL         ; convert (BLKNMBR) over to loical sectors.
    ld      a,(MODE)        ; get access mode flag (1=sequential,
    dec     a               ; 0=random, 2=special?).
    dec     a
    jp      nz,.9

    ; Special random i/o from function #40. Maybe for M/PM, but the
    ; current block, if it has not been written to, will be zeroed
    ; out and then written (reason?).
    pop     bc
    push    bc
    ld      a,c             ; get write status flag (2=writing unused space).
    dec     a
    dec     a
    jp      nz,.9
    push    hl
    ld      hl,(DIRBUF)     ; zero out the directory buffer.
    ld      d,a             ; note that (A) is zero here.
.7: ld      (hl),a
    inc     hl
    inc     d               ; do 128 bytes.
    jp      p,.7
    call    DIRDMA          ; tell the bios the DMA address for directory access.
    ld      hl,(LOGSECT)    ; get sector that starts current block.
    ld      c,2             ; set 'writing to unused space' flag.
.8: ld      (BLKNMBR),hl    ; save sector to write.
    push    bc
    call    TRKSEC1         ; determine its track and sector numbers.
    pop     bc
    call    DOWRITE         ; now write out 128 bytes of zeros.
    ld      hl,(BLKNMBR)    ; get sector number.
    ld      c,0             ; set normal write flag.
    ld      a,(BLKMASK)     ; determine if we have written the entire
    ld      b,a             ; physical block.
    and     l
    cp      b
    inc     hl              ; prepare for the next one.
    jp      nz,.8           ; continue until (BLKMASK+1) sectors written.
    pop     hl              ; reset next sector number.
    ld      (BLKNMBR),hl
    call    DEFDMA          ; and reset DMA address.

    ; Normal disk write. Set the desired track and sector then do the actual write.
.9: call    TRKSEC1         ; determine track and sector for this write.
    pop     bc              ; get write status flag.
    push    bc
    call    DOWRITE         ; and write this out.
    pop     bc
    ld      a,(SAVNREC)     ; get number of records in file.
    ld      hl,SAVNXT       ; get last record written.
    cp      (hl)
    jp      c,.10
    ld      (hl),a          ; we have to update record count.
    inc     (hl)
    ld      c,2

    ; * This area has been patched to correct disk update problem
    ; * when using blocking and de-blocking in the BIOS.
.10:
    nop                     ; was 'dcr c'
    nop                     ; was 'dcr c'
    ld  hl,0                ; was 'jnz wtseq99'
    ; * End of patch.
    push    af
    call    GETS2           ; set 'extent written to' flag.
    and     $7F             ; (* clear bit 7 *)
    ld      (hl),a
    pop     af              ; get record count for this extent.
    cp      127             ; is it full?
    jp      nz,.12
    ld      a,(MODE)        ; yes, are we in sequential mode?
    cp      1
    jp      nz,.12
    call    SETNREC         ; yes, set next record number.
    call    GETNEXT         ; and get next empty space in directory.
    ld      hl,STATUS       ; ok?
    ld      a,(hl)
    or      a
    jp      nz,.11
    dec     a               ; yes, set record count to -1.
    ld      (SAVNREC),a
.11:
    ld      (hl),0          ; clear status.
.12:
    jp      SETNREC         ; set next record to access.

;-----------------------------------------------------------------------------
; For random i/o, set the fcb for the desired record number
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
; On entry, register (C) contains $FF if this is a read
; and thus we can not access unwritten disk space. Otherwise,
; another extent will be opened (for writing) if required.
;-----------------------------------------------------------------------------
POSITION:
    xor     a               ; set random i/o flag.
    ld      (MODE),a

    ; Special entry (function #40). M/PM ?
POSITN1:
    push    bc              ; save read/write flag.
    ld      hl,(PARAMS)     ; get address of fcb.
    ex      de,hl
    ld      hl,33           ; now get byte 'r0'.
    add     hl,de
    ld      a,(hl)
    and     $7F             ; keep bits 0-6 for the record number to access.
    push    af
    ld      a,(hl)          ; now get bit 7 of 'r0' and bits 0-3 of 'r1'.
    rla
    inc     hl
    ld      a,(hl)
    rla
    and     $1F             ; and save this in bits 0-4 of (C).
    ld      c,a             ; this is the extent byte.
    ld      a,(hl)          ; now get the extra extent byte.
    rra
    rra
    rra
    rra
    and     $0F
    ld      b,a             ; and save it in (B).
    pop     af              ; get record number back to (A).
    inc     hl              ; check overflow byte 'r2'.
    ld      l,(hl)
    inc     l
    dec     l
    ld      l,6             ; prepare for error.
    jp      nz,.5           ; out of disk space error.
    ld      hl,32           ; store record number into fcb.
    add     hl,de
    ld      (hl),a
    ld      hl,12           ; and now check the extent byte.
    add     hl,de
    ld      a,c
    sub     (hl)            ; same extent as before?
    jp      nz,.2
    ld      hl,14           ; yes, check extra extent byte 's2' also.
    add     hl,de
    ld      a,b
    sub     (hl)
    and     $7F
    jp      z,.3            ; same, we are almost done then.

    ; Get here when another extent is required.
.2: push    bc
    push    de
    call    CLOSEIT         ; close current extent.
    pop     de
    pop     bc
    ld      l,3             ; prepare for error.
    ld      a,(STATUS)
    inc     a
    jp      z,.4            ; close error.
    ld      hl,12           ; put desired extent into fcb now.
    add     hl,de
    ld      (hl),c
    ld      hl,14           ; and store extra extent byte 's2'.
    add     hl,de
    ld      (hl),b
    call    OPENIT          ; try and get this extent.
    ld      a,(STATUS)      ; was it there?
    inc     a
    jp      nz,.3
    pop     bc              ; no. can we create a new one (writing?).
    push    bc
    ld      l,4             ; prepare for error.
    inc     c
    jp      z,.4            ; nope, reading unwritten space error.
    call    GETEMPTY        ; yes we can, try to find space.
    ld      l,5             ; prepare for error.
    ld      a,(STATUS)
    inc     a
    jp      z,.4            ; out of space?

    ; Normal return location. Clear error code and return.
.3: pop     bc              ; restore stack.
    xor     a               ; and clear error code byte.
    jp      SETSTAT

    ; Error. Set the 's2' byte to indicate this (why?).
.4: push    hl
    call    GETS2
    ld      (hl),$C0
    pop     hl

    ; Return with error code (presently in L).
.5: pop     bc
    ld      a,l             ; get error code.
    ld      (STATUS),a
    jp      SETS2B7

;-----------------------------------------------------------------------------
; Read a random record.
;-----------------------------------------------------------------------------
READRAN:
    ld      c,$FF           ; set 'read' status.
    call    POSITION        ; position the file to proper record.
    call    z,RDSEQ1        ; and read it as usual (if no errors).
    ret

;-----------------------------------------------------------------------------
; Write to a random record.
;-----------------------------------------------------------------------------
WRITERAN:
    ld      c,0             ; set 'writing' flag.
    call    POSITION        ; position the file to proper record.
    call    z,WTSEQ1        ; and write as usual (if no errors).
    ret

;-----------------------------------------------------------------------------
; Compute the random record number. Enter with (hl) pointing to a fcb and
; (de) contains a relative location of a record number. On exit, (C) contains
; the 'r0' byte, (B) the 'r1' byte, and (A) the 'r2' byte.
;
; On return, the zero flag is set if the record is within bounds.
; Otherwise, an overflow occured.
;-----------------------------------------------------------------------------
COMPRAND:
    ex      de,hl           ; save fcb pointer in (de).
    add     hl,de           ; compute relative position of record #.
    ld      c,(hl)          ; get record number into (bc).
    ld      b,0
    ld      hl,12           ; now get extent.
    add     hl,de
    ld      a,(hl)          ; compute (bc)=(record #)+(extent)*128.
    rrca                    ; move lower bit into bit 7.
    and     $80             ; and ignore all other bits.
    add     a,c             ; add to our record number.
    ld      c,a
    ld      a,0             ; take care of any carry.
    adc     a,b
    ld      b,a
    ld      a,(hl)          ; now get the upper bits of extent into
    rrca                    ; bit positions 0-3.
    and     $0F             ; and ignore all others.
    add     a,b             ; add this in to 'r1' byte.
    ld      b,a
    ld      hl,14           ; get the 's2' byte (extra extent).
    add     hl,de
    ld      a,(hl)
    add     a,a             ; and shift it left 4 bits (bits 4-7).
    add     a,a
    add     a,a
    add     a,a
    push    af              ; save carry flag (bit 0 of flag byte).
    add     a,b             ; now add extra extent into 'r1'.
    ld      b,a
    push    af              ; and save carry (overflow byte 'r2').
    pop     hl              ; bit 0 of (L) is the overflow indicator.
    ld      a,l
    pop     hl              ; and same for first carry flag.
    or      l               ; either one of these set?
    and     $01             ; only check the carry flags.
    ret

;-----------------------------------------------------------------------------
; Routine to setup the fcb (bytes 'r0', 'r1', 'r2') to reflect the last
; record used for a random (or other) file. This reads the directory and
; looks at all extents computing the largest record number for each and
; keeping the maximum  value only. Then 'r0', 'r1', and 'r2' will reflect
; this maximum record number. This is used to compute the space used by a
; random file.
;-----------------------------------------------------------------------------
RANSIZE:
    ld      c,12            ; look thru directory for first entry with
    call    FINDFST         ; this name.
    ld      hl,(PARAMS)     ; zero out the 'r0, r1, r2' bytes.
    ld      de,33
    add     hl,de
    push    hl
    ld      (hl),d          ; note that (D)=0.
    inc     hl
    ld      (hl),d
    inc     hl
    ld      (hl),d
.1: call    CKFILPOS        ; is there an extent to process?
    jp      z,.3            ; no, we are done.
    call    FCB2HL          ; set (hl) pointing to proper fcb in dir.
    ld      de,15           ; point to last record in extent.
    call    COMPRAND        ; and compute random parameters.
    pop     hl
    push    hl              ; now check these values against those
    ld      e,a             ; already in fcb.
    ld      a,c             ; the carry flag will be set if those
    sub     (hl)            ; in the fcb represent a larger size than
    inc     hl              ; this extent does.
    ld      a,b
    sbc     (hl)
    inc     hl
    ld      a,e
    sbc     (hl)
    jp      c,.2
    ld      (hl),e          ; we found a larger (in size) extent.
    dec     hl              ; stuff these values into fcb.
    ld      (hl),b
    dec     hl
    ld      (hl),c
.2: call    FINDNXT         ; now get the next extent.
    jp      .1              ; continue til all done.
.3: pop     hl              ; we are done, restore the stack and
    ret                     ; return.

;-----------------------------------------------------------------------------
; [36] SETTRAN - Function to return the random record position of a given
;                file which has been read in sequential mode up to now.
;-----------------------------------------------------------------------------
SETRAN:
    ld      hl,(PARAMS)     ; point to fcb.
    ld      de,32           ; and to last used record.
    call    COMPRAND        ; compute random position.
    ld      hl,33           ; now stuff these values into fcb.
    add     hl,de
    ld      (hl),c          ; move 'r0'.
    inc     hl
    ld      (hl),b          ; and 'r1'.
    inc     hl
    ld      (hl),a          ; and lastly 'r2'.
    ret

;-----------------------------------------------------------------------------
; This routine select the drive specified in (ACTIVE) and update the login
; vector and bitmap table if this drive was not already active.
;-----------------------------------------------------------------------------
LOGINDRV:
    ld      hl,(LOGIN)      ; get the login vector.
    ld      A,(ACTIVE)      ; get the default drive.
    ld      c,a
    call    SHIFTR          ; position active bit for this drive
    push    hl              ; into bit 0.
    ex      de,hl
    call    SELECT          ; select this drive.
    pop     hl
    call    z,SLCTERR       ; valid drive?
    ld      a,l             ; is this a newly activated drive?
    rra
    ret     c
    ld      hl,(LOGIN)      ; yes, update the login vector.
    ld      c,l
    ld      b,h
    call    SETBIT
    ld      (LOGIN),hl      ; and save.
    jp      BITMAP          ; now update the bitmap.

;-----------------------------------------------------------------------------
; [14] SETDSK - Function to set the active disk number.
;-----------------------------------------------------------------------------
SETDSK:
    ld      a,(EPARAM)      ; get parameter passed and see if this
    ld      hl,ACTIVE       ; represents a change in drives.
    cp      (hl)
    ret     z
    ld      (hl),a          ; yes it does, log it in.
    jp      LOGINDRV

;-----------------------------------------------------------------------------
;   This is the 'auto disk select' routine. The first byte
; of the FCB is examined for a drive specification. If non
; zero then the drive will be selected and loged in.
;-----------------------------------------------------------------------------
AUTOSEL:
    ld      a,$FF           ; say 'auto-select activated'.
    ld      (AUTO),a
    ld      hl,(PARAMS)     ; get drive specified.
    ld      a,(hl)
    and     $1F             ; look at lower 5 bits.
    dec     a               ; adjust for (1=A, 2=B) etc.
    ld      (EPARAM),a      ; and save for the select routine.
    cp      $1E             ; check for 'no change' condition.
    jp      nc,.1           ; yes, don't change.
    ld      a,(ACTIVE)      ; we must change, save currently active
    ld      (OLDDRV),a      ; drive.
    ld      a,(hl)          ; and save first byte of FCB also.
    ld      (AUTOFLAG),a    ; this must be non-zero.
    and     $E0             ; whats this for (bits 6,7 are used for
    ld      (hl),a          ; something)?
    call    SETDSK          ; select and log in this drive.
.1: ld      a,(USERNO)      ; move user number into FCB.
    ld      hl,(PARAMS)     ; (* upper half of first byte *)
    or      (hl)
    ld      (hl),a
    ret                     ; and return (all done).

;-----------------------------------------------------------------------------
; [12] GETVER - Function to return the current cp/m version number.
;-----------------------------------------------------------------------------
GETVER:
    ld      a,$22           ; version 2.2
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [13] RSTDSK - Function to reset the disk system.
;-----------------------------------------------------------------------------
RSTDSK:
    ld      hl,0            ; clear write protect status and log
    ld      (WRTPRT),hl     ; in vector.
    ld      (LOGIN),hl
    xor     a               ; select drive 'A'.
    ld      (ACTIVE),a
    ld      hl,TBUFF        ; setup default DMA address.
    ld      (USERDMA),hl
    call    DEFDMA
    jp      LOGINDRV        ; now log in drive 'A'.

;-----------------------------------------------------------------------------
; [15] OPENFIL - Function to open a specified file.
;-----------------------------------------------------------------------------
OPENFIL:
    call    CLEARS2         ; clear 's2' byte.
    call    AUTOSEL         ; select proper disk.
    jp      OPENIT          ; and open the file.

;-----------------------------------------------------------------------------
; [16] CLOSEFIL - Function to close a specified file.
;
; de=Address of FCB
;-----------------------------------------------------------------------------
CLOSEFIL:
    call    AUTOSEL         ; select proper disk.
    jp      CLOSEIT         ; and close the file.

;-----------------------------------------------------------------------------
; [17] GETFST - Function to return the first occurence of a specified file
;               name. If the first byte of the FCB is '?' then the name will
;               not be checked (get the first entry no matter what).
;
; de=Address of FCB
;-----------------------------------------------------------------------------
GETFST:
    ld      c,0             ; prepare for special search.
    ex      de,hl
    ld      a,(hl)          ; is first byte a '?'?
    cp      '?'
    jp      z,.1            ; yes, just get very first entry (zero length match).
    call    SETEXT          ; get the extension byte from fcb.
    ld      a,(hl)          ; is it '?'? if yes, then we want
    cp      '?'             ; an entry with a specific 's2' byte.
    call    nz,CLEARS2      ; otherwise, look for a zero 's2' byte.
    call    AUTOSEL         ; select proper drive.
    ld      c,15            ; compare bytes 0-14 in fcb (12&13 excluded).
.1: call    FINDFST         ; find an entry and then move it into
    jp      MOVEDIR         ; the users DMA space.

;-----------------------------------------------------------------------------
; [18] GETNXT - Function to return the next occurence of a file name.
;-----------------------------------------------------------------------------
GETNXT:
    ld      hl,(SAVEFCB)    ; restore pointers. note that no
    ld      (PARAMS),hl     ; other dbos calls are allowed.
    call    AUTOSEL         ; no error will be returned, but the
    call    FINDNXT         ; results will be wrong.
    jp      MOVEDIR

;-----------------------------------------------------------------------------
; [19] DELFILE - Function to delete a file by name.
;-----------------------------------------------------------------------------
DELFILE:
    call    AUTOSEL         ; select proper drive.
    call    ERAFILE         ; erase the file.
    jp      STSTATUS        ; set status and return.

;-----------------------------------------------------------------------------
; [20] READSEQ - Function to execute a sequential read of the specified
;                record number.
;-----------------------------------------------------------------------------
READSEQ:
    call    AUTOSEL         ; select proper drive then read.
    jp      RDSEQ

;-----------------------------------------------------------------------------
; [21] WRTSEQ - Function to write the next sequential record.
;-----------------------------------------------------------------------------
WRTSEQ:
    call    AUTOSEL         ; select proper drive then write.
    jp      WTSEQ

;-----------------------------------------------------------------------------
; [22] FCREATE - Create a file function.
;-----------------------------------------------------------------------------
FCREATE:
    call    CLEARS2     ; clear the 's2' byte on all creates.
    call    AUTOSEL     ; select proper drive and get the next
    jp      GETEMPTY    ; empty directory space.

;-----------------------------------------------------------------------------
; [23] RENFILE - Function to rename a file.
;-----------------------------------------------------------------------------
RENFILE:
    call    AUTOSEL     ; select proper drive and then switch
    call    CHGNAMES    ; file names.
    jp      STSTATUS

;-----------------------------------------------------------------------------
; [24] GETLOG - Function to return the login bitmap.
;-----------------------------------------------------------------------------
GETLOG:
    ld      hl,(LOGIN)
    jp      GETPRM1

;-----------------------------------------------------------------------------
; [25] GETCRNT - Function to return the current disk assignment.
;-----------------------------------------------------------------------------
GETCRNT:
    ld      a,(ACTIVE)
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [26] PUTDMA - Function to set the DMA address.
;-----------------------------------------------------------------------------
PUTDMA:
    ex      de,hl
    ld      (USERDMA),hl    ; save in our space and then get to
    jp      DEFDMA          ; the bios with this also.

;-----------------------------------------------------------------------------
; [27] GETALOC - Function to return the allocation bitmap.
;-----------------------------------------------------------------------------
GETALOC:
    ld      hl,(ALOCVECT)
    jp      GETPRM1

;-----------------------------------------------------------------------------
; [29] GETROV - Function to return the read-only status bitmap.
;-----------------------------------------------------------------------------
GETROV:
    ld      hl,(WRTPRT)
    jp      GETPRM1

;-----------------------------------------------------------------------------
; [30] SETATTR - Function to set the file attributes (read-only, system).
;-----------------------------------------------------------------------------
SETATTR:
    call    AUTOSEL         ; select proper drive then save attributes.
    call    SAVEATTR
    jp      STSTATUS

;-----------------------------------------------------------------------------
; [31] GETPARM - Function to return the address of the disk parameter block
;                for the current drive.
;-----------------------------------------------------------------------------
GETPARM:
    ld      hl,(DISKPB)
GETPRM1:
    ld      (STATUS),hl
    ret

;-----------------------------------------------------------------------------
; [32] GETUSER - Function to get or set the user number. If (E) was (FF)
;                then this is a request to return the current user number.
;                Else set the user number from (E).
;-----------------------------------------------------------------------------
GETUSER:
    ld      A,(EPARAM)      ; get parameter.
    cp      $FF             ; get user number?
    jp      nz,SETUSER
    ld      A,(USERNO)      ; yes, just do it.
    jp      SETSTAT
SETUSER:
    and     $1F             ; no, we should set it instead. keep low
    ld      (USERNO),A      ; bits (0-4) only.
    ret

;-----------------------------------------------------------------------------
; [33] RDRANDOM - Function to read a random record from a file.
;-----------------------------------------------------------------------------
RDRANDOM:
    call    AUTOSEL         ; select proper drive and read.
    jp      READRAN

;-----------------------------------------------------------------------------
; [34] WTRANDOM - Function to compute the file size for random files.
;-----------------------------------------------------------------------------
WTRANDOM:
    call    AUTOSEL         ; select proper drive and write.
    jp      WRITERAN

;-----------------------------------------------------------------------------
; [35] FILESIZE - Function to compute the size of a random file.
;-----------------------------------------------------------------------------
FILESIZE:
    call    AUTOSEL         ; select proper drive and check file length
    jp      RANSIZE

;-----------------------------------------------------------------------------
; [37] LOGOFF - This allows a program to log off any drives.
;               On entry, set (de) to contain a word with bits set for those
;               drives that are to be logged off. The log-in vector and the
;               write protect vector will be updated. This must be a M/PM
;               special function.
;-----------------------------------------------------------------------------
LOGOFF:
    ld      hl,(PARAMS)     ; get drives to log off.
    ld      a,l             ; for each bit that is set, we want
    cpl                     ; to clear that bit in (LOGIN)
    ld      e,a             ; and (WRTPRT).
    ld      a,h
    cpl
    ld      hl,(LOGIN)      ; reset the login vector.
    and     h
    ld      d,a
    ld      a,l
    and     e
    ld      e,a
    ld      hl,(WRTPRT)
    ex      de,hl
    ld      (LOGIN),hl      ; and save.
    ld      a,l             ; now do the write protect vector.
    and     e
    ld      l,a
    ld      a,h
    and     d
    ld      h,a
    ld      (WRTPRT),hl     ; and save. all done.
    ret

;-----------------------------------------------------------------------------
; Get here to return to the user.
;-----------------------------------------------------------------------------
GOBACK:
    ld      a,(AUTO)        ; was auto select activated?
    or      a
    jp      z,_bdos_exit2
    ld      hl,(PARAMS)     ; yes, but was a change made?
    ld      (hl),0          ; (* reset first byte of fcb *)
    ld      a,(AUTOFLAG)
    or      a
    jp      z,_bdos_exit2
    ld      (hl),a          ; yes, reset first byte properly.
    ld      a,(OLDDRV)      ; and get the old drive and select it.
    ld      (EPARAM),a
    call    SETDSK
_bdos_exit2:
    ld      hl,(USRSTACK)   ; reset the users stack pointer.
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
    call    AUTOSEL         ; select proper drive.
    ld      a,2             ; use special write mode.
    ld      (MODE),a
    ld      c,0             ; set write indicator.
    call    POSITN1         ; position the file.
    call    z,WTSEQ1        ; and write (if no errors).
    ret

;-----------------------------------------------------------------------------
; BDOS data storage pool
;-----------------------------------------------------------------------------
EMPTYFCB:   defb    $E5     ; empty directory segment indicator.
WRTPRT:     defw    0       ; write protect status for all 16 drives.
LOGIN:      defw    0       ; drive active word (1 bit per drive).
USERDMA:    defw    $80     ; user's DMA address (defaults to 80h).

;-----------------------------------------------------------------------------
; Scratch areas from parameter block.
;-----------------------------------------------------------------------------
SCRATCH1:   defw    0       ; relative position within dir segment for file (0-3).
SCRATCH2:   defw    0       ; last selected track number.
SCRATCH3:   defw    0       ; last selected sector number.

;-----------------------------------------------------------------------------
; Disk storage areas from parameter block.
;-----------------------------------------------------------------------------
DIRBUF:     defw    0       ; address of directory buffer to use.
DISKPB:     defw    0       ; contains address of disk parameter block.
CHKVECT:    defw    0       ; address of check vector.
ALOCVECT:   defw    0       ; address of allocation vector (bit map).

;-----------------------------------------------------------------------------
; Parameter block returned from the bios.
;-----------------------------------------------------------------------------
SECTORS:    defw    0       ; sectors per track from bios.
BLKSHFT:    defb    0       ; block shift.
BLKMASK:    defb    0       ; block mask.
EXTMASK:    defb    0       ; extent mask.
DSKSIZE:    defw    0       ; disk size from bios (number of blocks-1).
DIRSIZE:    defw    0       ; directory size.
ALLOC0:     defw    0       ; storage for first bytes of bit map (dir space used).
ALLOC1:     defw    0
OFFSET:     defw    0       ; first usable track number.
XLATE:      defw    0       ; sector translation table address.
CLOSEFLG:   defb    0       ; close flag (=$FF is extent written ok).
RDWRTFLG:   defb    0       ; read/write flag ($FF=read, 0=write).
FNDSTAT:    defb    0       ; filename found status (0=found first entry).
MODE:       defb    0       ; I/o mode select (0=random, 1=sequential, 2=special random).
EPARAM:     defb    0       ; storage for register (E) on entry to bdos.
RELBLOCK:   defb    0       ; relative position within fcb of block number written.
COUNTER:    defb    0       ; byte counter for directory name searches.
SAVEFCB:    defw    0,0     ; save space for address of fcb (for directory searches).
BIGDISK:    defb    0       ; if =0 then disk is > 256 blocks long.
AUTO:       defb    0       ; if non-zero, then auto select activated.
OLDDRV:     defb    0       ; on auto select, storage for previous drive.
AUTOFLAG:   defb    0       ; if non-zero, then auto select changed drives.
SAVNXT:     defb    0       ; storage for next record number to access.
SAVEXT:     defb    0       ; storage for extent number of file.
SAVNREC:    defw    0       ; storage for number of records in file.
BLKNMBR:    defw    0       ; block number (physical sector) used within a file or logical sect
LOGSECT:    defw    0       ; starting logical (128 byte) sector of block (physical sector).
FCBPOS:     defb    0       ; relative position within buffer for fcb of file of interest.
FILEPOS:    defw    0       ; files position within directory (0 to max entries -1).

;-----------------------------------------------------------------------------
; Disk directory buffer checksum bytes. One for each of the
; 16 possible drives.
;-----------------------------------------------------------------------------
CKSUMTBL:
    defs    16

;-----------------------------------------------------------------------------
; Extra space?
;-----------------------------------------------------------------------------
    defb    0,0,0,0
