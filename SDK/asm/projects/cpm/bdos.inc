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
; BDOS function jump table.
;-----------------------------------------------------------------------------
NFUNCTS:    equ 41          ; number of functions in following table.
FUNCTNS:
    defw    WBOOT,GETCON,OUTCON,GETRDR,PUNCH,LIST,DIRCIO,GETIOB
    defw    SETIOB,PRTSTR,RDBUFF,GETCSTS,GETVER,RSTDSK,SETDSK,OPENFIL
    defw    CLOSEFIL,GETFST,GETNXT,DELFILE,READSEQ,WRTSEQ,FCREATE
    defw    RENFILE,GETLOG,GETCRNT,PUTDMA,GETALOC,RTN,GETROV,SETATTR
    defw    GETPARM,GETUSER,RDRANDOM,WTRANDOM,FILESIZE,SETRAN,RTN,RTN
    defw    RTN,WTSPECL

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
SAVEFCB:  defw  0           ; save space for address of FCB (for directory searches).
AUTO:     defb  0           ; if non-zero, then auto select activated.
OLDDRV:   defb  0           ; on auto select, storage for previous drive.
AUTOFLAG: defb  0           ; if non-zero, then auto select changed drives.
USERDMA:  defw  $80         ; user's DMA address (defaults to 80h).
OUTFLAG:  defb  0           ; output flag (non zero means no output).
STARTING: defb  2           ; starting position for cursor.
CURPOS:   defb  0           ; cursor position (0=start of line).
PRTFLAG:  defb  0           ; printer flag (control-p toggle). List if non zero.
CHARBUF:  defb  0           ; single input character buffer.
USERNO:   defb  0           ; current user number.
ACTIVE:   defb  0           ; currently active drive.
PARAMS:   defw  0           ; save (de) parameters here on entry.
EPARAM:   defb  0           ; storage for register (E) on entry to BDOS.
STATUS:   defw  0           ; status returned from BDOS function.

_fcb:     defw  0           ; argument for _findfst/_findnxt

;-----------------------------------------------------------------------------
; Stack area for BDOS calls.
;-----------------------------------------------------------------------------
USRSTACK:
        defw    0           ; save users stack pointer here.
        defs    48
STKAREA equ     $           ; end of stack area.

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
; Current path for drive letters A-D
;-----------------------------------------------------------------------------
; _path_a: defs 64
; _path_b: defs 64
; _path_c: defs 64
; _path_d: defs 64

;-----------------------------------------------------------------------------
; Open files
; - 0  2 fcb address
; - 2  1 file descriptor
; - 3  1 disk
; - 4 11 file name
;-----------------------------------------------------------------------------
_file1: defs 16
_file2: defs 16
_file3: defs 16
_file4: defs 16

;-----------------------------------------------------------------------------
; Entry into BDOS. (DE) or (E) are the parameters passed. The
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
    ld      hl,_bdos_exit   ; set return address.
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
; Return to user
;-----------------------------------------------------------------------------
_bdos_exit:
    ld      a,(AUTO)        ; was auto select activated?
    or      a
    jp      z,_bdos_exit2
    ld      hl,(PARAMS)     ; yes, but was a change made?
    ld      (hl),0          ; (* reset first byte of FCB *)
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
; Error: Bad Sector
;-----------------------------------------------------------------------------
_err_bad_sect:
    ld      hl,.msg         ; bad sector message.
    call    _print_err      ; print it and get a 1 char response.
    cp      CTRL_C          ; re-boot request (control-c)?
    jp      z,0             ; yes.
    ret                     ; no, return to retry i/o function.
.msg:
    defb    'Bad Sector$'

;-----------------------------------------------------------------------------
; Error: Bad Drive Selected
;-----------------------------------------------------------------------------
_err_bad_sel:
    ld      hl,.msg         ; bad drive selected.
    jr      _print_err_reboot
.msg:
    defb    'Select$'

;-----------------------------------------------------------------------------
; Error: Disk is Read Only
;-----------------------------------------------------------------------------
_err_disk_ro:
    ld      hl,.msg         ; disk is read only.
    jr      _print_err_reboot
.msg:
    defb    'R/O$'

;-----------------------------------------------------------------------------
; Error: File is Read Only
;-----------------------------------------------------------------------------
_err_file_ro:
    ld      hl,.msg         ; file is read only.
    jr      _print_err_reboot
.msg:
    defb    'File R/O$'

;-----------------------------------------------------------------------------
; Print error, followed by reboot
;-----------------------------------------------------------------------------
_print_err_reboot:
    call    _print_err
    jp      0               ; always reboot on these errors.

;-----------------------------------------------------------------------------
; Print BDOS error message.
;-----------------------------------------------------------------------------
_print_err:
    push    hl              ; save second message pointer.
    call    OUTCRLF         ; send CRLF.
    ld      a,(ACTIVE)      ; get active drive.
    add     a,'A'           ; make ASCII.
    ld      (.msg_drv),a    ; and put in message.
    ld      bc,.msg         ; and print it.
    call    PRTMESG
    pop     bc              ; print second message line now.
    call    PRTMESG
    jr      GETCHAR

.msg:
    defb    'Bdos Err On '
.msg_drv:
    defb    ' : $'

;-----------------------------------------------------------------------------
; Return with error
;-----------------------------------------------------------------------------
_ret_err:
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; Return with 0
;-----------------------------------------------------------------------------
_ret_0:
    xor     a
    jp      SETSTAT

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
    jr      nz,.2
    call    CONST           ; nothing in buffer. Check console.
    and     $01             ; look at bit 0.
    ret     z               ; return if nothing.
    call    CONIN           ; ok, get it.
    cp      CTRL_S          ; if not control-s, return with zero cleared.
    jr      nz,.1
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
    jr      nz,.1
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
    jr      nz,.2
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
    jr      nz,.1
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
    jr      .1

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
    jr      nz,.3
    ld      a,b             ; yes, but ignore at the beginning of the line.
    or      a
    jr      z,.1
    dec     b               ; ok, update counter.
    ld      a,(CURPOS)      ; if we backspace to the start of the line,
    ld      (OUTFLAG),a     ; treat as a cancel (control-x).
    jr      .10
.3: cp      DEL             ; user typed a rubout?
    jr      nz,.4
    ld      a,b             ; ignore at the start of the line.
    or      a
    jr      z,.1
    ld      a,(hl)          ; ok, echo the previous character.
    dec     b               ; and reset pointers (counters).
    dec     hl
    jr      .15
.4: cp      CTRL_E          ; physical end of line?
    jr      nz,.5
    push    bc              ; yes, do it.
    push    hl
    call    OUTCRLF
    xor     a               ; and update starting position.
    ld      (STARTING),a
    jr      .2
.5: cp      CTRL_P          ; control-p?
    jr      nz,.6
    push    hl              ; yes, flip the print flag filp-flop byte.
    ld      hl,PRTFLAG
    ld      a,1             ; PRTFLAG=1-PRTFLAG
    sub     (hl)
    ld      (hl),a
    pop     hl
    jr      .1
.6: cp      CTRL_X          ; control-x (cancel)?
    jr      nz,.8
    pop     hl
.7: ld      a,(STARTING)    ; yes, backup the cursor to here.
    ld      hl,CURPOS
    cp      (hl)
    jr      nc,RDBUFF       ; done yet?
    dec     (hl)            ; no, decrement pointer and output back up one space.
    call    BACKUP
    jr      .7
.8: cp      CTRL_U          ; control-U (cancel line)?
    jr      nz,.9
    call    NEWLINE         ; start a new line.
    pop     hl
    jr      RDBUFF
.9: cp      CTRL_R          ; control-r?
    jr      nz,.14
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
    jr      z,.12
    inc     hl              ; nope, get next character.
    ld      c,(hl)
    dec     b               ; count it.
    push    bc
    push    hl
    call    SHOWIT          ; and display it.
    pop     hl
    pop     bc
    jr      .11
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
    jr      nz,.13
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
    jr      nz,.16
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
    jr      z,.1
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
IOERR1:
    ld      a,1
    jp      SETSTAT

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
    ld      d,(hl)              ; now (de) contain the desired address.
    ex      de,hl
    jp      (hl)

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
; Routine to set the DMA address to the users choice.
;-----------------------------------------------------------------------------
DEFDMA:
    ld      hl,USERDMA      ; reset the default DMA address and return.
    jp      DIRDMA1

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
; [36] SETTRAN - Function to return the random record position of a given
;                file which has been read in sequential mode up to now.
;-----------------------------------------------------------------------------
SETRAN:
    jp SETRAN           ; FIXME

;-----------------------------------------------------------------------------
; [14] SETDSK - Function to set the active disk number.
;-----------------------------------------------------------------------------
SETDSK:
    ld      a,(EPARAM)      ; get parameter passed and see if this
    ld      hl,ACTIVE       ; represents a change in drives.
    cp      (hl)
    ret     z
    ld      (hl),a          ; yes it does, log it in.

    cp      a,4
    call    nc,SLCTERR      ; valid drive?
    ret

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
    xor     a               ; select drive 'A'.
    ld      (ACTIVE),a
    ld      hl,TBUFF        ; setup default DMA address.
    ld      (USERDMA),hl
    call    DEFDMA
    ret

;-----------------------------------------------------------------------------
; [15] OPENFIL - Function to open a specified file.
;-----------------------------------------------------------------------------
OPENFIL:
    ld      (_fcb),de
    call    _close_if_open

    call    _findfst
    jr      nz,.notfound

    ; Open file
    call    _openfile
    bit     7,a
    jr      nz,.notfound

    ; Put file descriptor in FCB
    ld      ix,(_fcb)
    ld      (ix+18),a
    ld      a,'F'
    ld      (ix+16),a
    ld      a,'D'
    ld      (ix+17),a

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
;
; DE=Address of FCB
;-----------------------------------------------------------------------------
CLOSEFIL:
    ld      (_fcb),de
    call    _close_if_open
    xor     a
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
    ld      (SAVEFCB),de    ; Save FCB
    ld      (_fcb),de
    call    _close_if_open

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

    ld      de,(SAVEFCB)
    ld      (_fcb),de
    call    _findnxt
_getnxt_result:
    jr      nz,.err
    call    _copy_dirent_to_dma
    xor     a                       ; Indicate success, direntry at first 32 of DMA
    jp      SETSTAT
.err:
    ld      a,$FF                   ; Indicate failure
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [19] DELFILE - Function to delete a file by name.
;-----------------------------------------------------------------------------
DELFILE:
    ld      (_fcb),de
    call    _findfst
    jr      nz,.notfound

    ld      a,ESPCMD_DELETE
    call    esp_cmd
    ld      hl,_dirent_name
    call    _send_filename
    call    esp_get_byte
    bit     7,a
    jr      nz,.notfound

    ; Return success
    xor     a
    jp      SETSTAT

.notfound:
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [20] READSEQ - Function to execute a sequential read of the specified
;                record number.
;-----------------------------------------------------------------------------
READSEQ:
    ; Get file descriptor from FCB
    ld      de,(PARAMS)
    ld      (_fcb),de
    call    _get_fd
    bit     7,a
    jr      nz,.err

    ld      hl,(USERDMA)
    ld      de,128
    call    esp_read_bytes
    jr      nz,.err
    ld      a,d
    or      a,e
    jr      z,.eof

    ; Pad up to 128 bytes with EOF mark (CTRL-Z/$1A)
    ld      a,$1A
.1: bit     7,e
    jr      nz,.done
    ld      (hl),a
    inc     hl
    inc     e
    jr      .1
.done:
    xor     a
    jp      SETSTAT

.eof:
    ld      a,1
    jp      SETSTAT

.err:
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [21] WRTSEQ - Function to write the next sequential record.
;-----------------------------------------------------------------------------
WRTSEQ:
    jr      WRTSEQ

    ; FIXME
    ld      a,$FF
    jp      SETSTAT

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
    ld      a,ESPCMD_RENAME
    call    esp_cmd
    ld      hl,(PARAMS)
    inc     hl
    call    _send_filename

    ld      hl,(PARAMS)
    ld      bc,17
    add     hl,bc
    call    _send_filename

    call    esp_get_byte
    bit     7,a
    jr      nz,.notfound

    ; Return success
    xor     a
    jp      SETSTAT

.notfound:
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [24] GETLOG - Function to return the login bitmap.
;-----------------------------------------------------------------------------
GETLOG:
    ld      hl,$000F
    jp      _ret_stat_hl

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
    ld      (USERDMA),hl    ;save in our space and then get to
    jp      DEFDMA          ;the bios with this also.

;-----------------------------------------------------------------------------
; [27] GETALOC - Function to return the allocation bitmap.
;-----------------------------------------------------------------------------
GETALOC:
    jp      0

;-----------------------------------------------------------------------------
; [29] GETROV - Function to return the read-only status bitmap.
;-----------------------------------------------------------------------------
GETROV:
    ld      hl,0
    jp      _ret_stat_hl

;-----------------------------------------------------------------------------
; [30] SETATTR - Function to set the file attributes (read-only, system).
;-----------------------------------------------------------------------------
SETATTR:
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [31] GETPARM - Function to return the address of the disk parameter block
;                for the current drive.
;-----------------------------------------------------------------------------
GETPARM:
    jp      0

;-----------------------------------------------------------------------------
; _ret_stat_hl
;-----------------------------------------------------------------------------
_ret_stat_hl:
    ld      (STATUS),hl
    ret

;-----------------------------------------------------------------------------
; [32] GETUSER - Function to get or set the user number. If (E) was (FF)
;                then this is a request to return the current user number.
;                Else set the user number from (E).
;-----------------------------------------------------------------------------
GETUSER:
    ld      a,(EPARAM)          ; get parameter.
    cp      $FF                 ; get user number?
    jr      nz,.set_user
    ld      a,(USERNO)          ; yes, just do it.
    jp      SETSTAT

.set_user:
    and     $1F                 ; no, we should set it instead. keep low
    ld      (USERNO),a          ; bits (0-4) only.
    ret

;-----------------------------------------------------------------------------
; [33] RDRANDOM - Function to read a random record from a file.
;-----------------------------------------------------------------------------
RDRANDOM:
    ; Get file descriptor from FCB
    ld      de,(PARAMS)
    ld      (_fcb),de
    call    _get_fd
    bit     7,a
    jr      nz,.err

    ; Get record index from FCB
    ld      ix,(PARAMS)
    ld      e,(ix+33)
    ld      d,(ix+34)
    call    _seek

    ; Perform normal sequential read to read the record
    jp      READSEQ

    ; call  AUTOSEL         ;select proper drive and read.
    ; jp      READRAN

.err:
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [34] WTRANDOM - Function to compute the file size for random files.
;-----------------------------------------------------------------------------
WTRANDOM:
    jr      WTRANDOM

    ; FIXME
    ld      a,$FF
    jp      SETSTAT

;-----------------------------------------------------------------------------
; [35] FILESIZE - Function to compute the size of a random file.
;-----------------------------------------------------------------------------
FILESIZE:
    jr      FILESIZE

    ; FIXME
    ld      a,$FF
    jp      SETSTAT

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
    ; ld      (MODE),a
    ; ld      C,0             ;set write indicator.
    ; call    POSITN1         ;position the file.
    ; call    Z,WTSEQ1        ;and write (if no errors).
    ; ret

;-----------------------------------------------------------------------------
; _read_dirent
;-----------------------------------------------------------------------------
_read_dirent:
    ld      a,ESPCMD_READDIR
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
.1: call    esp_get_byte
    cp      '.'
    jr      z,.pad2         ; Dot
    or      a
    jr      z,.pad1         ; Zero-byte
    ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.1

    call    esp_get_byte    ; Dot
    or      a
    jr      nz,.ext

.pad1:
    inc     c
    inc     c
    inc     c
    jr      .pad3

    ; Pad filename with spaces
.pad2:
    ld      a,' '
.2: ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.2

    ; Read extension
.ext:
    ld      c,3
.3: call    esp_get_byte
    or      a
    jr      z,.pad3         ; Zero-byte
    ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.3
    ret

.pad3:
    ; Pad extension with spaces
    ld      a,' '
.4: ld      (hl),a
    inc     hl
    dec     c
    jr      nz,.4
    ret

;-----------------------------------------------------------------------------
; Find first directory entry matching _fcb
;-----------------------------------------------------------------------------
_findfst:
    ; Close directory descriptor 0
    ld      a,ESPCMD_CLOSEDIR
    call    esp_cmd
    xor     a
    call    esp_send_byte
    call    esp_get_byte

    ; Open directory
    ld      a,ESPCMD_OPENDIR83
    call    esp_cmd
    xor     a
    call    esp_send_byte
    call    esp_get_byte
    or      a
    ret     nz

    ; Find matching entry
    jr      _findnxt

;-----------------------------------------------------------------------------
; Get next directory entry matching _fcb
;-----------------------------------------------------------------------------
_findnxt:
    ; Read entry
    call    _read_dirent
    ret     nz

    ; Check against FCB
    ld      de,(_fcb)
    ld      a,(de)
    cp      '?'             ; '?' in user field gives back all entries
    ret     z
    inc     de

    ; Compare filename against FCB
    ld      c,11
    ld      hl,_dirent_name
.nxt:
    ld      a,(de)
    cp      a,'?'           ; Wildcard?
    jr      z,.nxt2
    cp      a,(hl)
    jr      nz,_findnxt     ; Filename does not match, next entry

.nxt2:
    inc     hl
    inc     de
    dec     c
    jr      nz,.nxt
    ret

;-----------------------------------------------------------------------------
; Copy current directory entry to DMA buffer
;-----------------------------------------------------------------------------
_copy_dirent_to_dma:
    ld      de,(USERDMA)
    ld      a,(USERNO)              ; Always give back current user (no user filtering implemented)
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
; Send filename in hl
;-----------------------------------------------------------------------------
_send_filename:
    push    hl

    ; Send filename
    ld      c,8
.1: ld      a,(hl)
    cp      a,' '
    jr      z,.ext
    call    esp_send_byte
    inc     hl
    dec     c
    jr      nz,.1

    ; Send extension
.ext:
    ld      a,'.'
    call    esp_send_byte
    pop     hl
    ld      bc,8
    add     hl,bc
    ld      c,3
.2: ld      a,(hl)
    cp      a,' '
    jr      z,.zterm
    call    esp_send_byte
    inc     hl
    dec     c
    jr      nz,.2

    ; Send zero-termination
.zterm:
    xor     a
    call    esp_send_byte
    ret

;-----------------------------------------------------------------------------
; Check if FCB contains open file descriptor and close it
;-----------------------------------------------------------------------------
_close_if_open:
    ; Check if FCB already points to file descriptor, if so close it first
    ld      ix,(_fcb)
    ld      a,(ix+16)
    cp      'F'
    ret     nz
    ld      a,(ix+17)
    cp      'D'
    ret     nz

    ; Close file descriptor
    ld      a,ESPCMD_CLOSE
    call    esp_cmd
    ld      a,(ix+18)
    call    esp_send_byte
    call    esp_get_byte

    ; Erase bytes
    xor     a
    ld      (ix+16),a
    ld      (ix+17),a
    ld      (ix+18),a
    ret

;-----------------------------------------------------------------------------
; Get file descriptor from FCB
;-----------------------------------------------------------------------------
_get_fd:
    ld      ix,(_fcb)
    ld      a,(ix+16)
    cp      'F'
    jr      nz,.err
    ld      a,(ix+17)
    cp      'D'
    jr      nz,.err
    ld      a,(ix+18)
    ret
.err:
    ld      a,$FF
    ret

;-----------------------------------------------------------------------------
; Open file in _dirent_name
;-----------------------------------------------------------------------------
_openfile:
    ld      a,ESPCMD_OPEN
    call    esp_cmd
    ld      a,0             ; Flags (read-only)
    call    esp_send_byte
    ld      hl,_dirent_name
    call    _send_filename

    ; Get result
    call    esp_get_byte
    ret

;-----------------------------------------------------------------------------
; Seek to record DE (file descriptor in A)
;-----------------------------------------------------------------------------
_seek:
    ld      b,a
    ld      a,ESPCMD_SEEK
    call    esp_cmd
    ld      a,b
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
; Input:  A: file descriptor
;         HL: destination address
;         DE: number of bytes to read
; Output: HL: next address (start address if no bytes read)
;         DE: number of bytes actually read
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
esp_read_bytes:
    ld      b,a
    ld      a,ESPCMD_READ
    call    esp_cmd
    ld      a,b

    ; Send file descriptor
    call    esp_send_byte

    ; Send read size
    ld      a,e
    call    esp_send_byte
    ld      a,d
    call    esp_send_byte

    ; Get result
    call    esp_get_byte
    or      a
    ret     nz

    ; Get number of bytes actual read
    call    esp_get_byte
    ld      e,a
    call    esp_get_byte
    ld      d,a

    push    de

.loop:
    ; Done reading? (DE=0)
    ld      a,d
    or      a,e
    jr      z,.done

    call    esp_get_byte
    ld      (hl),a
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de

    xor     a
    ret
