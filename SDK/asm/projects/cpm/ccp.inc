;-----------------------------------------------------------------------------
; ccp.inc - command processor
;-----------------------------------------------------------------------------

CBASE:
    jp      _command        ; execute command processor (CCP).
    jp      _clearbuf       ; entry to empty input buffer before starting CCP.

;-----------------------------------------------------------------------------
; Standard CP/M CCP input buffer. Format is (max length),
; (actual length), (char #1), (char #2), (char #3), etc.
;-----------------------------------------------------------------------------
_inbuff:
    defb    127             ; length of input buffer.
    defb    0               ; current length of contents.
    defb    'Copyright'
    defb    ' 1979 (c) by Digital Research      '
    defb    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    defb    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    defb    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    defb    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
_inpoint:
    defw    _inbuff+2       ; input line pointer
_namepnt:
    defw    0               ; input line pointer used for error message. Points to start of name in error.

;-----------------------------------------------------------------------------
; Routine to print (A) on the console. All registers used.
;-----------------------------------------------------------------------------
_print:
    ld      e,a             ; setup bdos call.
    ld      c,2
    jp      ENTRY

;-----------------------------------------------------------------------------
; Routine to print (A) on the console and to save (bc).
;-----------------------------------------------------------------------------
_printb:
    push    bc
    call    _print
    pop     bc
    ret

;-----------------------------------------------------------------------------
; Routine to send a carriage return, line feed combination to the console.
;-----------------------------------------------------------------------------
_crlf:
    ld      a,CR
    call    _printb
    ld      a,LF
    jp      _printb

;-----------------------------------------------------------------------------
; Routine to send one space to the console and save (bc).
;-----------------------------------------------------------------------------
_space:
    ld      a,' '
    jp      _printb

;-----------------------------------------------------------------------------
; Routine to print character string pointed to be (bc) on the
; console. It must terminate with a null byte.
;-----------------------------------------------------------------------------
_pline:
    push    bc
    call    _crlf
    pop     hl
_pline2:
    ld      a,(hl)
    or      a
    ret     z
    inc     hl
    push    hl
    call    _print
    pop     hl
    jp      _pline2

;-----------------------------------------------------------------------------
; Routine to reset the disk system.
;-----------------------------------------------------------------------------
_resdsk:
    ld      c,13
    jp      ENTRY

;-----------------------------------------------------------------------------
; Routine to select disk (A).
;-----------------------------------------------------------------------------
_dsksel:
    ld      e,a
    ld      c,14
    jp      ENTRY

;-----------------------------------------------------------------------------
; Routine to call bdos and save the return code. The zero
; flag is set on a return of 0ffh.
;-----------------------------------------------------------------------------
_entry1:
    call    ENTRY
    ld      (_rtncode),a     ; save return code.
    inc     a               ; set zero if 0ffh returned.
    ret

;-----------------------------------------------------------------------------
; Routine to open a file. (de) must point to the _fcb.
;-----------------------------------------------------------------------------
_open:
    ld      c,15
    jp      _entry1

;-----------------------------------------------------------------------------
; Routine to open file at (_fcb).
;-----------------------------------------------------------------------------
_openfcb:
    xor     a               ; clear the record number byte at fcb+32
    ld      (_fcb + 32),a
    ld      de,_fcb
    jp      _open

;-----------------------------------------------------------------------------
; Routine to close a file. (de) points to _fcb.
;-----------------------------------------------------------------------------
_close:
    ld      c,16
    jp      _entry1

;-----------------------------------------------------------------------------
; Routine to search for the first file with ambiguous name (de).
;-----------------------------------------------------------------------------
_srchfst:
    ld      c,17
    jp      _entry1

;-----------------------------------------------------------------------------
; Search for the next ambiguous file name.
;-----------------------------------------------------------------------------
_srchnxt:
    ld      c,18
    jp      _entry1

;-----------------------------------------------------------------------------
; Search for file at (_fcb).
;-----------------------------------------------------------------------------
_srchfcb:
    ld      de,_fcb
    jp      _srchfst

;-----------------------------------------------------------------------------
; Routine to delete a file pointed to by (de).
;-----------------------------------------------------------------------------
_delete:
    ld      c,19
    jp      ENTRY

;-----------------------------------------------------------------------------
; Routine to call the bdos and set the zero flag if a zero status is returned.
;-----------------------------------------------------------------------------
_entry2:
    call    ENTRY
    or      a               ; set zero flag if appropriate.
    ret

;-----------------------------------------------------------------------------
; Routine to read the next record from a sequential file.
; (de) points to the _fcb.
;-----------------------------------------------------------------------------
_rdrec:
    ld      c,20
    jp      _entry2

;-----------------------------------------------------------------------------
; Routine to read file at (_fcb).
;-----------------------------------------------------------------------------
_readfcb:
    ld      de,_fcb
    jp      _rdrec

;-----------------------------------------------------------------------------
; Routine to write the next record of a sequential file.
; (de) points to the _fcb.
;-----------------------------------------------------------------------------
_wrtrec:
    ld      c,21
    jp      _entry2

;-----------------------------------------------------------------------------
; Routine to create the file pointed to by (de).
;-----------------------------------------------------------------------------
_create:
    ld      c,22
    jp      _entry1

;-----------------------------------------------------------------------------
; Routine to rename the file pointed to by (de). Note that
; the new name starts at (de+16).
;-----------------------------------------------------------------------------
_renam:
    ld      c,23
    jp      ENTRY

;-----------------------------------------------------------------------------
; Get the current user code.
;-----------------------------------------------------------------------------
_getusr:
    ld      e,$FF
    ; fall into _getsetuc

;-----------------------------------------------------------------------------
; Routine to get or set the current user code.
; If (E) is FF then this is a GET, else it is a SET.
;-----------------------------------------------------------------------------
_getsetuc:
    ld      c,32
    jp      ENTRY

;-----------------------------------------------------------------------------
; Routine to set the current drive byte at (TDRIVE).
;-----------------------------------------------------------------------------
_setcdrv:
    call    _getusr         ; get user number
    add     a,a             ; and shift into the upper 4 bits.
    add     a,a
    add     a,a
    add     a,a
    ld      hl,_cdrive      ; now add in the current drive number.
    or      (hl)
    ld      (TDRIVE),a      ; and save.
    ret

;-----------------------------------------------------------------------------
; Move currently active drive down to (TDRIVE).
;-----------------------------------------------------------------------------
_movecd:
    ld      a,(_cdrive)
    ld      (TDRIVE),a
    ret

;-----------------------------------------------------------------------------
; Routine to convert (A) into upper case ascii. Only letters
; are affected.
;-----------------------------------------------------------------------------
_upper:
    cp      'a'             ; check for letters in the range of 'a' to 'z'.
    ret     c
    cp      '{'
    ret     nc
    and     $5F             ; convert it if found.
    ret

;-----------------------------------------------------------------------------
; Routine to get a line of input. We must check to see if the
; user is in (BATCH) mode. If so, then read the input from file
; ($$$.SUB). At the end, reset to console input.
;-----------------------------------------------------------------------------
_getinp:
    ld      a,(_batch)       ; if =0, then use console input.
    or      a
    jp      z,_getinp1

    ; Use the submit file ($$$.sub) which is prepared by a
    ; SUBMIT run. It must be on drive (A) and it will be deleted
    ; if and error occures (like eof).
    ld      a,(_cdrive)      ; select drive 0 if need be.
    or      a
    ld      a,0             ; always use drive A for submit.
    call    nz,_dsksel      ; select it if required.
    ld      de,_batchfcb
    call    _open           ; look for it.
    jp      z,_getinp1      ; if not there, use normal input.
    ld      a,(_batchfcb+15) ; get last record number+1.
    dec     a
    ld      (_batchfcb+32),a
    ld      de,_batchfcb
    call    _rdrec          ; read last record.
    jp      nz,_getinp1     ; quit on end of file.

    ; Move this record into input buffer.
    ld      de,_inbuff+1
    ld      hl,TBUFF        ; data was read into buffer here.
    ld      b,128           ; all 128 characters may be used.
    call    _hl2de          ; (hl) to (de), (B) bytes.
    ld      hl,_batchfcb+14
    ld      (hl),0          ; zero out the 's2' byte.
    inc     hl              ; and decrement the record count.
    dec     (hl)
    ld      de,_batchfcb    ; close the batch file now.
    call    _close
    jp      z,_getinp1      ; quit on an error.
    ld      A,(_cdrive)     ; re-select previous drive if need be.
    or      A
    call    nz,_dsksel      ; don't do needless selects.

    ; Print line just read on console.
    ld      hl,_inbuff+2
    call    _pline2
    call    _chkcon         ; check console, quit on a key.
    jp      z,_getinp2      ; jump if no key is pressed.

    ; Terminate the submit job on any keyboard input. Delete this
    ; file such that it is not re-started and jump to normal keyboard
    ; input section.
    call    _delbatch       ; delete the batch file.
    jp      _cmmnd1         ; and restart command input.

    ; Get here for normal keyboard input. Delete the submit file
    ; incase there was one.
_getinp1:
    call    _delbatch       ; delete file ($$$.sub).
    call    _setcdrv        ; reset active disk.
    ld      c,10            ; get line from console device.
    ld      de,_inbuff
    call    ENTRY
    call    _movecd         ; reset current drive (again).

    ; Convert input line to upper case.
_getinp2:
    ld      hl,_inbuff+1
    ld      b,(hl)          ; (B)=character counter.
_getinp3:
    inc     hl
    ld      a,b             ; end of the line?
    or      a
    jp      z,_getinp4
    ld      a,(hl)          ; convert to upper case.
    call    _upper
    ld      (hl),a
    dec     b               ; adjust character count.
    jp      _getinp3
_getinp4:
    ld      (hl),a          ; add trailing null.
    ld      hl,_inbuff+2
    ld      (_inpoint),hl   ; reset input line pointer.
    ret

;-----------------------------------------------------------------------------
; Routine to check the console for a key pressed. The zero
; flag is set is none, else the character is returned in (A).
;-----------------------------------------------------------------------------
_chkcon:
    ld      c,11            ; check console.
    call    ENTRY
    or      A
    ret     Z               ; return if nothing.
    ld      c,1             ; else get character.
    call    ENTRY
    or      A               ; clear zero flag and return.
    ret

;-----------------------------------------------------------------------------
; Routine to get the currently active drive number.
;-----------------------------------------------------------------------------
_getdsk:
    ld      c,25
    jp      ENTRY

;-----------------------------------------------------------------------------
; Set the standard DMA address.
;-----------------------------------------------------------------------------
_stddma:
    ld      de,TBUFF

;-----------------------------------------------------------------------------
; Routine to set the DMA address to (de).
;-----------------------------------------------------------------------------
_dmaset:
    ld      c,26
    jp      ENTRY

;-----------------------------------------------------------------------------
; Delete the batch file created by SUBMIT.
;-----------------------------------------------------------------------------
_delbatch:
    ld      hl,_batch       ; is batch active?
    ld      a,(hl)
    or      a
    ret     z
    ld      (hl),0          ; yes, de-activate it.
    xor     a
    call    _dsksel         ; select drive 0 for sure.
    ld      de,_batchfcb    ; and delete this file.
    call    _delete
    ld      a,(_cdrive)     ; reset current drive.
    jp      _dsksel

;-----------------------------------------------------------------------------
; Check to two strings at (PATTRN1) and (PATTRN2). They must be
; the same or we halt....
;-----------------------------------------------------------------------------
_verify:
    ld      de,_pattrn1     ; these are the serial number bytes.
    ld      hl,PATTRN2      ; ditto, but how could they be different?
    ld      b,6             ; 6 bytes each.
.1: ld      a,(de)
    cp      (hl)
    jp      nz,_halt        ; jump to halt routine.
    inc     de
    inc     hl
    dec     b
    jp      nz,.1
    ret

;-----------------------------------------------------------------------------
; Print back file name with a '?' to indicate a syntax error.
;-----------------------------------------------------------------------------
_synerr:
    call    _crlf           ; end current line.
    ld      hl,(_namepnt)   ; this points to name in error.
_synerr1:
    ld      a,(hl)          ; print it until a space or null is found.
    cp      ' '
    jp      z,_synerr2
    or      a
    jp      z,_synerr2
    push    hl
    call    _print
    pop     hl
    inc     hl
    jp      _synerr1
_synerr2:
    ld      a,'?'           ; add trailing '?'.
    call    _print
    call    _crlf
    call    _delbatch       ; delete any batch file.
    jp      _cmmnd1         ; and restart from console input.

;-----------------------------------------------------------------------------
; Check character at (de) for legal command input. Note that the
; zero flag is set if the character is a delimiter.
;-----------------------------------------------------------------------------
_check:
    ld      a,(de)
    or      a
    ret     z
    cp      ' '             ; control characters are not legal here.
    jp      c,_synerr
    ret     z               ; check for valid delimiter.
    cp      '='
    ret     z
    cp      '_'
    ret     z
    cp      '.'
    ret     z
    cp      ':'
    ret     z
    cp      ';'
    ret     z
    cp      '<'
    ret     z
    cp      '>'
    ret     z
    ret

;-----------------------------------------------------------------------------
; Get the next non-blank character from (de).
;-----------------------------------------------------------------------------
_nonblank:
    ld      a,(de)
    or      a               ; string ends with a null.
    ret     z
    cp      ' '
    ret     nz
    inc     de
    jp      _nonblank

;-----------------------------------------------------------------------------
; Add (hl)=(hl)+(A)
;-----------------------------------------------------------------------------
_addhl:
    add     a,l
    ld      l,a
    ret     nc              ; take care of any carry.
    inc     h
    ret

;-----------------------------------------------------------------------------
; Convert the first name in (_fcb).
;-----------------------------------------------------------------------------
_convfst:
    ld      a,0
    ; fall into _convert

;-----------------------------------------------------------------------------
; Format a file name (convert * to '?', etc.). On return,
; (A)=0 is an unambigeous name was specified. Enter with (A) equal to
; the position within the fcb for the name (either 0 or 16).
;-----------------------------------------------------------------------------
_convert:
    ld      hl,_fcb
    call    _addhl
    push    hl
    push    hl
    xor     a
    ld      (_chgdrv),a     ; initialize drive change flag.
    ld      hl,(_inpoint)   ; set (hl) as pointer into input line.
    ex      de,hl
    call    _nonblank       ; get next non-blank character.
    ex      de,hl
    ld      (_namepnt),hl   ; save pointer here for any error message.
    ex      de,hl
    pop     hl
    ld      a,(de)          ; get first character.
    or      a
    jp      z,_convrt1
    SBC     a,'A'-1         ; might be a drive name, convert to binary.
    ld      b,a             ; and save.
    inc     de              ; check next character for a ':'.
    ld      a,(de)
    cp      ':'
    jp      z,_convrt2
    dec     de              ; nope, move pointer back to the start of the line.
_convrt1:
    ld      a,(_cdrive)
    ld      (hl),a
    jp      _convrt3
_convrt2:
    ld      a,b
    ld      (_chgdrv),a     ; set change in drives flag.
    ld      (hl),b
    inc     de

    ; Convert the basic file name.
_convrt3:
    ld      b,$08
_convrt4:
    call    _check
    jp      z,_convrt8
    inc     hl
    cp      '*'             ; note that an '*' will fill the remaining
    jp      nz,_convrt5     ; field with '?'.
    ld      (hl),'?'
    jp      _convrt6
_convrt5:
    ld      (hl),a
    inc     de
_convrt6:
    dec     b
    jp      nz,_convrt4
_convrt7:
    call    _check          ; get next delimiter.
    jp      z,_getext
    inc     de
    jp      _convrt7
_convrt8:
    inc     hl              ; blank fill the file name.
    ld      (hl),' '
    dec     b
    jp      nz,_convrt8

    ; Get the extension and convert it.
_getext:
    ld      b,$03
    cp      '.'
    jp      nz,_getext5
    inc     de
_getext1:
    call    _check
    jp      z,_getext5
    inc     hl
    cp      '*'
    jp      nz,_getext2
    ld      (hl),'?'
    jp      _getext3
_getext2:
    ld      (hl),a
    inc     de
_getext3:
    dec     b
    jp      nz,_getext1
_getext4:
    call    _check
    jp      z,_getext6
    inc     de
    jp      _getext4
_getext5:
    inc     hl
    ld      (hl),' '
    dec     b
    jp      nz,_getext5
_getext6:
    ld      b,3
_getext7:
    inc     hl
    ld      (hl),0
    dec     b
    jp      nz,_getext7
    ex      de,hl
    ld      (_inpoint),hl   ; save input line pointer.
    pop     hl

    ; Check to see if this is an ambiguous file name specification.
    ; Set the (A) register to non zero if it is.
    ld      bc,11           ; set name length.
_getext8:
    inc     hl
    ld      a,(hl)
    cp      '?'             ; any question marks?
    jp      nz,_getext9
    inc     b               ; count them.
_getext9:
    dec     c
    jp      nz,_getext8
    ld      a,b
    or      a
    ret

;-----------------------------------------------------------------------------
; CP/M command table. Note commands can be either 3 or 4 characters long.
;-----------------------------------------------------------------------------
_numcmds: equ     6             ; number of commands
_cmdtbl:
    defb    'DIR ','ERA ','TYPE','SAVE'
    defb    'REN ','USER'

;-----------------------------------------------------------------------------
; The following six bytes must agree with those at (PATTRN2)
; or cp/m will HALT. Why?
;-----------------------------------------------------------------------------
_pattrn1:
    defb    0,22,0,0,0,0    ; (* serial number bytes *).

;-----------------------------------------------------------------------------
; Search the command table for a match with what has just been entered.
; If a match is found, then we jump to the proper section.
; Else jump to (_unknown).
; On return, the (C) register is set to the command number that matched
; (or _numcmds+1 if no match).
;-----------------------------------------------------------------------------
_search:
    ld      hl,_cmdtbl
    ld      c,0
_search1:
    ld      a,c
    cp      _numcmds        ; this commands exists.
    ret     nc
    ld      de,_fcb+1       ; check this one.
    ld      b,4             ; max command length.
_search2:
    ld      a,(de)
    cp      (hl)
    jp      nz,_search3     ; not a match.
    inc     de
    inc     hl
    dec     b
    jp      nz,_search2
    ld      a,(de)          ; allow a 3 character command to match.
    cp      ' '
    jp      nz,_search4
    ld      a,c             ; set return register for this command.
    ret
_search3:
    inc     hl
    dec     b
    jp      nz,_search3
_search4:
    inc     c
    jp      _search1

;-----------------------------------------------------------------------------
; Set the input buffer to empty and then start the command processor (CCP).
;-----------------------------------------------------------------------------
_clearbuf:
    xor     a
    ld      (_inbuff+1),a   ; second byte is actual length.

;-----------------------------------------------------------------------------
; CCP - Console Command Processor
;-----------------------------------------------------------------------------
_command:
    ld      sp,_ccpstack    ; setup stack area.
    push    bc              ; note that (C) should be equal to:
    ld      a,c             ; (uuuudddd) where 'uuuu' is the user number
    rra                     ; and 'dddd' is the drive number.
    rra
    rra
    rra
    and     $0F             ; isolate the user number.
    ld      e,a
    call    _getsetuc       ; and set it.
    call    _resdsk         ; reset the disk system.
    ld      (_batch),a      ; clear batch mode flag.
    pop     bc
    ld      a,c
    and     $0F             ; isolate the drive number.
    ld      (_cdrive),a     ; and save.
    call    _dsksel         ; ...and select.
    ld      a,(_inbuff+1)
    or      a               ; anything in input buffer already?
    jp      nz,_cmmnd2      ; yes, we just process it.

;-----------------------------------------------------------------------------
; Entry point to get a command line from the console.
;-----------------------------------------------------------------------------
_cmmnd1:
    ld      sp,_ccpstack    ; set stack straight.
    call    _crlf           ; start a new line on the screen.
    call    _getdsk         ; get current drive.
    add     a,'A'
    call    _print          ; print current drive.
    ld      a,'>'
    call    _print          ; and add prompt.
    call    _getinp         ; get line from user.

    ; Process command line here.
_cmmnd2:
    ld      de,TBUFF
    call    _dmaset         ; set standard DMA address.
    call    _getdsk
    ld      (_cdrive),a     ; set current drive.
    call    _convfst        ; convert name typed in.
    call    nz,_synerr      ; wild cards are not allowed.
    ld      a,(_chgdrv)     ; if a change in drives was indicated,
    or      a               ; then treat this as an unknown command
    jp      nz,_unknown     ; which gets executed.
    call    _search         ; else search command table for a match.

    ; Note that an unknown command returns with (A) pointing to the last address
    ; in our table which is (_unknown).
    ld      hl,_cmdadr      ; now, look thru our address table for command (A).
    ld      e,a             ; set (de) to command number.
    ld      d,0
    add     hl,de
    add     hl,de           ; (hl)=(_cmdadr)+2*(command number).
    ld      a,(hl)          ; now pick out this address.
    inc     hl
    ld      h,(hl)
    ld      l,a
    jp      (hl)            ; now execute it.

; CP/M command address table.
_cmdadr:
    defw    _direct,_erase,_type,_save
    defw    _rename,_user,_unknown

; Halt the system. Reason for this is unknown at present.
_halt:
    ld      hl,$76F3        ; 'DI HLT' instructions.
    ld      (CBASE),hl
    ld      hl,CBASE
    jp      (hl)

;-----------------------------------------------------------------------------
; Read error while TYPEing a file.
;-----------------------------------------------------------------------------
_rderror:
    ld      bc,_rderr
    jp      _pline
_rderr:
    defb    'Read error',0

;-----------------------------------------------------------------------------
; Required file was not located.
;-----------------------------------------------------------------------------
_none:
    ld      bc,_nofile
    jp      _pline
_nofile:
    defb    'No file',0

;-----------------------------------------------------------------------------
; Decode a command of the form 'A>filename number{ filename}.
; Note that a drive specifier is not allowed on the first file
; name. On return, the number is in register (A). Any error
; causes 'filename?' to be printed and the command is aborted.
;-----------------------------------------------------------------------------
_decode:
    call    _convfst        ; convert filename.
    ld      a,(_chgdrv)     ; do not allow a drive to be specified.
    or      a
    jp      nz,_synerr
    ld      hl,_fcb+1       ; convert number now.
    ld      bc,11           ; (B)=sum register, (C)=max digit count.
_decode1:
    ld      a,(hl)
    cp      ' '             ; a space terminates the numeral.
    jp      z,_decode3
    inc     hl
    SUB     '0'             ; make binary from ascii.
    cp      10              ; legal digit?
    jp      nc,_synerr
    ld      d,a             ; yes, save it in (D).
    ld      a,b             ; compute (B)=(B)*10 and check for overflow.
    and     $E0
    jp      nz,_synerr
    ld      a,b
    rlca
    rlca
    rlca                    ; (A)=(B)*8
    add     a,b             ; .......*9
    jp      c,_synerr
    add     a,b             ; .......*10
    jp      c,_synerr
    add     a,d             ; add in new digit now.
    jp      c,_synerr
    ld      b,a             ; and save result.
    dec     c               ; only look at 11 digits.
    jp      nz,_decode1
    ret
_decode3:
    ld      A,(hl)          ; spaces must follow (why?).
    cp      ' '
    jp      nz,_synerr
    inc     hl
    dec     c
    jp      nz,_decode3
    ld      a,b             ; set (A)=the numeric value entered.
    ret

;-----------------------------------------------------------------------------
; Move 3 bytes from (HL) to (DE).
; Note that there is only one reference to this at (A2D5h).
;-----------------------------------------------------------------------------
_move3:
    ld      b,3

;-----------------------------------------------------------------------------
; Move (B) bytes from (hl) to (de).
;-----------------------------------------------------------------------------
_hl2de:
    ld      a,(hl)
    ld      (de),a
    inc     hl
    inc     de
    dec     b
    jp      nz,_hl2de
    ret

;-----------------------------------------------------------------------------
; Compute (hl)=(TBUFF)+(A)+(C) and get the byte that's here.
;-----------------------------------------------------------------------------
_extract:
    ld      hl,TBUFF
    add     a,c
    call    _addhl
    ld      a,(hl)
    ret

;-----------------------------------------------------------------------------
; Check drive specified. If it means a change, then the new
; drive will be selected. In any case, the drive byte of the
; fcb will be set to null (means use current drive).
;-----------------------------------------------------------------------------
_dselect:
    xor     a               ; null out first byte of fcb.
    ld      (_fcb),a
    ld      a,(_chgdrv)     ; a drive change indicated?
    or      a
    ret     z
    dec     a               ; yes, is it the same as the current drive?
    ld      hl,_cdrive
    cp      (hl)
    ret     z
    jp      _dsksel         ; no. Select it then.

;-----------------------------------------------------------------------------
; Check the drive selection and reset it to the previous
; drive if it was changed for the preceeding command.
;-----------------------------------------------------------------------------
_resetdr:
    ld      a,(_chgdrv)     ; drive change indicated?
    or      a
    ret     z
    dec     a               ; yes, was it a different drive?
    ld      hl,_cdrive
    cp      (hl)
    ret     z
    ld      a,(_cdrive)     ; yes, re-select our old drive.
    jp      _dsksel

;-----------------------------------------------------------------------------
; DIRECTORY COMMAND
;-----------------------------------------------------------------------------
_direct:
    call    _convfst        ; convert file name.
    call    _dselect        ; select indicated drive.
    ld      hl,_fcb+1       ; was any file indicated?
    ld      a,(hl)
    cp      ' '
    jp      nz,_direct2
    ld      b,11            ; no. Fill field with '?' - same as *.*.
_direct1:
    ld      (hl),'?'
    inc     hl
    dec     b
    jp      nz,_direct1
_direct2:
    ld      e,0             ; set initial cursor position.
    push    de
    call    _srchfcb        ; get first file name.
    call    z,_none         ; none found at all?
_direct3:
    jp      z,_direct9      ; terminate if no more names.
    ld      a,(_rtncode)    ; get file's position in segment (0-3).
    rrca
    rrca
    rrca
    and     $60             ; (A)=position*32
    ld      c,a
    ld      a,10
    call    _extract        ; extract the tenth entry in fcb.
    rla                     ; check system file status bit.
    jp      c,_direct8      ; we don't list them.
    pop     de
    ld      a,e             ; bump name count.
    inc     e
    push    de
    and     $03             ; at end of line?
    push    af
    jp      nz,_direct4
    call    _crlf           ; yes, end this line and start another.
    push    bc
    call    _getdsk         ; start line with ('A:').
    pop     bc
    add     a,'A'
    call    _printb
    ld      a,':'
    call    _printb
    jp      _direct5
_direct4:
    call    _space          ; add seperator between file names.
    ld      A,':'
    call    _printb
_direct5:
    call    _space
    ld      B,1             ; 'extract' each file name character at a time.
_direct6:
    ld      a,b
    call    _extract
    and     $7F             ; strip bit 7 (status bit).
    cp      ' '             ; are we at the end of the name?
    jp      nz,_drect65
    pop     af              ; yes, don't print spaces at the end of a line.
    push    af
    cp      3
    jp      nz,_drect63
    ld      a,9             ; first check for no extension.
    call    _extract
    and     $7F
    cp      ' '
    jp      z,_direct7      ; don't print spaces.
_drect63:
    ld      a,' '           ; else print them.
_drect65:
    call    _printb
    inc     b               ; bump to next character psoition.
    ld      a,b
    cp      12              ; end of the name?
    jp      nc,_direct7
    cp      9               ; nope, starting extension?
    jp      nz,_direct6
    call    _space          ; yes, add seperating space.
    jp      _direct6
_direct7:
    pop     af              ; get the next file name.
_direct8:
    call    _chkcon         ; first check console, quit on anything.
    jp      nz,_direct9
    call    _srchnxt        ; get next name.
    jp      _direct3        ; and continue with our list.
_direct9:
    pop     de              ; restore the stack and return to command level.
    jp      _getback

;-----------------------------------------------------------------------------
; ERASE COMMAND
;-----------------------------------------------------------------------------
_erase:
    call    _convfst        ; convert file name.
    cp      11              ; was '*.*' entered?
    jp      nz,_erase1
    ld      bc,_yesno       ; yes, ask for confirmation.
    call    _pline
    call    _getinp
    ld      hl,_inbuff+1
    dec     (hl)            ; must be exactly 'y'.
    jp      nz,_cmmnd1
    inc     hl
    ld      a,(hl)
    cp      'Y'
    jp      nz,_cmmnd1
    inc     hl
    ld      (_inpoint),hl   ; save input line pointer.
_erase1:
    call    _dselect        ; select desired disk.
    ld      de,_fcb
    call    _delete         ; delete the file.
    inc     a
    call    z,_none         ; not there?
    jp      _getback        ; return to command level now.
_yesno:
    defb    'All (y/n)?',0

;-----------------------------------------------------------------------------
; TYPE COMMAND
;-----------------------------------------------------------------------------
_type:
    call    _convfst        ; convert file name.
    jp      nz,_synerr      ; wild cards not allowed.
    call    _dselect        ; select indicated drive.
    call    _openfcb        ; open the file.
    jp      z,_type5        ; not there?
    call    _crlf           ; ok, start a new line on the screen.
    ld      hl,_nbytes      ; initialize byte counter.
    ld      (hl),0FFH       ; set to read first sector.
_type1:
    ld      hl,_nbytes
    ld      a,(hl)          ; have we written the entire sector?
    cp      128
    jp      c,_type3
    push    hl              ; yes, read in the next one.
    call    _readfcb
    pop     hl
    jp      nz,_type4       ; end or error?
    xor     a               ; ok, clear byte counter.
    ld      (hl),a
_type3:
    inc     (hl)            ; count this byte.
    ld      hl,TBUFF        ; and get the (A)th one from the buffer (TBUFF).
    call    _addhl
    ld      a,(hl)
    cp      CTRL_Z          ; end of file mark?
    jp      z,_getback
    call    _print          ; no, print it.
    call    _chkcon         ; check console, quit if anything ready.
    jp      nz,_getback
    jp      _type1

    ; Get here on an end of file or read error.
_type4:
    dec     a               ; read error?
    jp      z,_getback
    call    _rderror        ; yes, print message.
_type5:
    call    _resetdr        ; and reset proper drive
    jp      _synerr         ; now print file name with problem.

;-----------------------------------------------------------------------------
; SAVE COMMAND
;-----------------------------------------------------------------------------
_save:
    call    _decode         ; get numeric number that follows SAVE.
    push    af              ; save number of pages to write.
    call    _convfst        ; convert file name.
    jp      nz,_synerr      ; wild cards not allowed.
    call    _dselect        ; select specified drive.
    ld      de,_fcb         ; now delete this file.
    push    de
    call    _delete
    pop     de
    call    _create         ; and create it again.
    jp      z,_save3        ; can't create?
    xor     a               ; clear record number byte.
    ld      (_fcb+32),A
    pop     af              ; convert pages to sectors.
    ld      L,A
    ld      H,0
    add     hl,hl           ; (hl)=number of sectors to write.
    ld      de,TBASE        ; and we start from here.
_save1:
    ld      A,H             ; done yet?
    or      L
    jp      z,_save2
    dec     hl              ; nope, count this and compute the start
    push    hl              ; of the next 128 byte sector.
    ld      hl,128
    add     hl,de
    push    hl              ; save it and set the transfer address.
    call    _dmaset
    ld      de,_fcb         ; write out this sector now.
    call    _wrtrec
    pop     de              ; reset (de) to the start of the last sector.
    pop     hl              ; restore sector count.
    jp      nz,_save3       ; write error?
    jp      _save1

    ; Get here after writing all of the file.
_save2:
    ld      de,_fcb         ; now close the file.
    call    _close
    inc     A               ; did it close ok?
    jp      nz,_save4

    ; Print out error message (no space).
_save3:
    ld      bc,_nospace
    call    _pline
_save4:
    call    _stddma         ; reset the standard DMA address.
    jp      _getback

_nospace:
    defb    'No space',0

;-----------------------------------------------------------------------------
; RENAME COMMAND
;-----------------------------------------------------------------------------
_rename:
    call    _convfst        ; convert first file name.
    jp      nz,_synerr      ; wild cards not allowed.
    ld      a,(_chgdrv)     ; remember any change in drives specified.
    push    af
    call    _dselect        ; and select this drive.
    call    _srchfcb        ; is this file present?
    jp      nz,_rename6     ; yes, print error message.
    ld      hl,_fcb         ; yes, move this name into second slot.
    ld      de,_fcb+16
    ld      b,16
    call    _hl2de
    ld      hl,(_inpoint)   ; get input pointer.
    ex      de,hl
    call    _nonblank       ; get next non blank character.
    cp      '='             ; only allow an '=' or '_' seperator.
    jp      z,_rename1
    cp      '_'
    jp      nz,_rename5
_rename1:
    ex      de,hl
    inc     hl              ; ok, skip seperator.
    ld      (_inpoint),hl   ; save input line pointer.
    call    _convfst        ; convert this second file name now.
    jp      nz,_rename5     ; again, no wild cards.
    pop     af              ; if a drive was specified, then it
    ld      b,a             ; must be the same as before.
    ld      hl,_chgdrv
    ld      a,(hl)
    or      a
    jp      z,_rename2
    cp      b
    ld      (hl),b
    jp      nz,_rename5     ; they were different, error.
_rename2:
    ld      (hl),b          ; reset as per the first file specification.
    xor     a
    ld      (_fcb),a        ; clear the drive byte of the fcb.
    call    _srchfcb        ; and go look for second file.
    jp      z,_rename4      ; doesn't exist?
    ld      de,_fcb
    call    _renam          ; ok, rename the file.
    jp      _getback

    ; Process rename errors here.
_rename4:
    call    _none           ; file not there.
    jp      _getback
_rename5:
    call    _resetdr        ; bad command format.
    jp      _synerr
_rename6:
    ld      bc,_exists      ; destination file already exists.
    call    _pline
    jp      _getback
_exists:
    defb    'File exists',0

;-----------------------------------------------------------------------------
; USER COMMAND
;-----------------------------------------------------------------------------
_user:
    call    _decode         ; get numeric value following command.
    cp      16              ; legal user number?
    jp      nc,_synerr
    ld      e,a             ; yes but is there anything else?
    ld      a,(_fcb+1)
    cp      ' '
    jp      z,_synerr       ; yes, that is not allowed.
    call    _getsetuc       ; ok, set user code.
    jp      _getback1

;-----------------------------------------------------------------------------
; TRANSIENT PROGRAM COMMAND
;-----------------------------------------------------------------------------
_unknown:
    call    _verify         ; check for valid system (why?).
    ld      a,(_fcb+1)      ; anything to execute?
    cp      ' '
    jp      nz,_unkwn1
    ld      a,(_chgdrv)     ; nope, only a drive change?
    or      a
    jp      z,_getback1     ; neither???
    dec     a
    ld      (_cdrive),a     ; ok, store new drive.
    call    _movecd         ; set (TDRIVE) also.
    call    _dsksel         ; and select this drive.
    jp      _getback1       ; then return.

    ; Here a file name was typed. Prepare to execute it.
_unkwn1:
    ld      de,_fcb+9       ; an extension specified?
    ld      a,(de)
    cp      ' '
    jp      nz,_synerr      ; yes, not allowed.
    push    de
    call    _dselect        ; select specified drive.
    pop     de
    ld      hl,_comfile     ; set the extension to 'COM'.
    call    _move3
    call    _openfcb        ; and open this file.
    jp      z,_unkwn9       ; not present?

    ; Load in the program.
    ld      hl,TBASE        ; store the program starting here.
.load_loop:
    push    hl
    ex      de,hl
    call    _dmaset         ; set transfer address.
    ld      de,_fcb         ; and read the next record.
    call    _rdrec
    jp      nz,_unkwn4      ; end of file or read error?
    pop     hl              ; nope, bump pointer for next sector.
    ld      de,128
    add     hl,de
    ld      de,CBASE        ; enough room for the whole file?
    ld      a,l
    sub     e
    ld      a,h
    sbc     a,d
    jp      nc,_unkwn0      ; no, it can't fit.
    jp      .load_loop

    ; Get here after finished reading.
_unkwn4:
    pop     hl
    dec     a               ; normal end of file?
    jp      nz,_unkwn0
    call    _resetdr        ; yes, reset previous drive.
    call    _convfst        ; convert the first file name that follows
    ld      hl,_chgdrv      ; command name.
    push    hl
    ld      a,(hl)          ; set drive code in default fcb.
    ld      (_fcb),a
    ld      a,16            ; put second name 16 bytes later.
    call    _convert        ; convert second file name.
    pop     hl
    ld      a,(hl)          ; and set the drive for this second file.
    ld      (_fcb+16),a
    xor     a               ; clear record byte in fcb.
    ld      (_fcb+32),a
    ld      de,TFCB         ; move it into place at(005Ch).
    ld      hl,_fcb
    ld      b,33
    call    _hl2de
    ld      hl,_inbuff+2    ; now move the remainder of the input
_unkwn5:
    ld      a,(hl)          ; line down to (0080h). Look for a non blank.
    or      a               ; or a null.
    jp      z,_unkwn6
    cp      ' '
    jp      z,_unkwn6
    inc     hl
    jp      _unkwn5

    ; Do the line move now. It ends in a null byte.
_unkwn6:
    ld      b,0             ; keep a character count.
    ld      de,TBUFF+1      ; data gets put here.
_unkwn7:
    ld      a,(hl)          ; move it now.
    ld      (de),a
    or      a
    jp      z,_unkwn8
    inc     b
    inc     hl
    inc     de
    jp      _unkwn7
_unkwn8:
    ld      a,b             ; now store the character count.
    ld      (TBUFF),a
    call    _crlf           ; clean up the screen.
    call    _stddma         ; set standard transfer address.
    call    _setcdrv        ; reset current drive.
    call    TBASE           ; and execute the program.

    ; Transient programs return here (or reboot).
    ld      sp,_batch       ; set stack first off.
    call    _movecd         ; move current drive into place (TDRIVE).
    call    _dsksel         ; and reselect it.
    jp      _cmmnd1         ; back to command mode.

    ; Get here if some error occured.
_unkwn9:
    call    _resetdr        ; inproper format.
    jp      _synerr
_unkwn0:
    ld      bc,_badload     ; read error or won't fit.
    call    _pline
    jp      _getback

_badload:
    defb    'Bad load',0
_comfile:
    defb    'COM'           ; command file extension.

;-----------------------------------------------------------------------------
; Get here to return to command level. We will reset the
; previous active drive and then either return to command
; level directly or print error message and then return.
;-----------------------------------------------------------------------------
_getback:
    call    _resetdr        ; reset previous drive.
_getback1:
    call    _convfst        ; convert first name in (_fcb).
    ld      a,(_fcb+1)      ; if this was just a drive change request,
    sub     ' '             ; make sure it was valid.
    ld      hl,_chgdrv
    or      (hl)
    jp      nz,_synerr
    jp      _cmmnd1         ; ok, return to command level.

;-----------------------------------------------------------------------------
; CCP stack area.
;-----------------------------------------------------------------------------
    defb    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
_ccpstack: equ    $             ; end of CCP stack area.

;-----------------------------------------------------------------------------
; Batch (or SUBMIT) processing information storage.
;-----------------------------------------------------------------------------
_batch:    defb 0               ; batch mode flag (0=not active).
_batchfcb: defb 0,'$$$     SUB',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;-----------------------------------------------------------------------------
; File control block setup by the CCP.
;-----------------------------------------------------------------------------
_fcb:      defb 0,'           ',0,0,0,0,0,'           ',0,0,0,0,0
_rtncode:  defb 0               ; status returned from bdos call.
_cdrive:   defb 0               ; currently active drive.
_chgdrv:   defb 0               ; change in drives flag (0=no change).
_nbytes:   defw 0               ; byte counter used by TYPE.

    ; Room for expansion?
    defb    0,0,0,0,0,0,0,0,0,0,0,0,0
