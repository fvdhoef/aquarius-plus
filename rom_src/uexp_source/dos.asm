;====================================================================
;                  Aquarius USB DOS Commands
;====================================================================
;
;changes:
; 2015-11-11 V0.00 Extracted from babasicV0.2f
; 2016-01-02 V0.01 Save file implemented.
;                  File type analysis.
; 2016-01-11 V0.02 Improved file type analysis.
;                  Using IY register to reference system variables.
;                  Changed name from 'usb_dos' to 'dos' in preparation
;                  for handling different types of storage media.
; 2016-01-18 V0.03 KILL, CAT, DIR.
;                  ST_LOAD returns filetype if successful.
; 2016-01-20 V0.04 getfilename now returns error code offset rather than
;                  returning to BASIC prompt if filename is invalid.
; 2016-01-27 V0.05 Load/save numeric array.
; 2016-01-31 V0.06 internal Init_BASIC routine
; 2016-02-06 V0.07 trim trailing zeros off BASIC (CAQ) file after loading
; 2016-02-19 V0.08 close file after reading
; 2016-02-21 V0.09 DIR supports wildcards
; 2017-01-19 V0.10 string operands evaluated, eg. LEFT$(A$,11)
; 2017-01-22 V0.11 CAT prints 3 filenames per line
; 2017-03-04 V0.12 binary header is now $BF,$DA,addr
; 2017-03-05 V0.13 removed PRNTAB
; 2017-03-13 V0.14 incresed max filename chars from 11 to 12 (8+"."+3)
;                  moved get_next, getarg, chk_arg to strings.asm
; 2017-04-05 V0.15 strip header off binary file
; 2017-04-24 V0.16 ST_CD change directory
; 2017-04-26 V0.17 CD without arg shows current directory
; 2017-05-01 V0.18 KILL error returns to BASIC with FC error
; 2017-05-13 V0.19 CD checks for disk ready (resets path to root if disk changed)
;                  move valid path chacks to usb_ready
; 2017-06-01 V0.20 CD to a file (invalid) now removes the filename from the path
; 2017-06-12 V1.0  bumped to release version

; file types
FT_NONE  equ $01  ; no file extension (type determined from file header)
FT_TXT   equ $20  ; .TXT ASCII text file (no header)
FT_OTHER equ $80  ; .??? unknown file type (raw binary, no header)
FT_BIN   equ $BF  ; .BIN binary (starts with $BF,$DA,load_addr if executable)
FT_BAS   equ $fe  ; .BAS tokenized BASIC (has CAQ header same as .CAQ)
FT_CAQ   equ $ff  ; .CAQ BASIC program or numeric array

; bits in dosflags
DF_ADDR   = 0      ; set = address specified
DF_ARRAY  = 7      ; set = numeric array

;--------------------------------------------------------------------
;                        Change Directory
;--------------------------------------------------------------------
; CD "dirname"  = add 'subdir' to path
; CD "/path"    = set path to '/path'
; CD ""         = no operation
; CD            = show path
;
ST_CD:
    push   hl                    ; push BASIC text pointer
    ld     c,a
    call   usb__ready            ; check for USB disk (may reset path to root!)
    jr     nz,.do_error
    ld     a,c
    OR     A                     ; any args?
    JR     NZ,.change_dir        ; yes,
.show_path:
    LD     HL,PathName
    call   PRINTSTR              ; print path
    call   PRNCRLF
    jr     .done
.change_dir:
    pop    hl                    ; pop BASIC text pointer
    CALL   EVAL                  ; evaluate expression
    PUSH   HL                    ; push BASIC text pointer
    CALL   TSTSTR                ; type mismatch error if not string
    CALL   GETLEN                ; get string and its length
    JR     Z,.open               ; if null string then open current directory
    inc    hl
    inc    hl                    ; skip to string text pointer
    ld     b,(hl)
    inc    hl
    ld     h,(hl)                ; hl = string text
    ld     l,b
    call   dos__set_path         ; update path (out: DE = end of old path)
    jr     z,.open
    ld     a,ERROR_PATH_LEN
    jr     .do_error             ; path too long
.open:
    ld     hl,PathName
    call   usb__open_path        ; try to open directory
    jr     z,.done               ; if opened OK then done
    cp     CH376_ERR_MISS_FILE   ; directory missing?
    jr     z,.undo
    cp     CH376_INT_SUCCESS     ; 'directory' is actually a file?
    jr     nz,.do_error          ; no, disk error
.undo:
    ex     de,hl                 ; HL = end of old path
    ld     (hl),0                ; remove subdirectory from path
    ld     a,ERROR_NO_DIR        ; error = missing directory
.do_error:
    call   _show_error           ; print error message
    ld     e,FC_ERR
    pop    hl
    jp     DO_ERROR              ; return to BASIC with FC error
.done:
    pop    hl                    ; restore BASIC text pointer
    ret

;--------------------------------------------------------------------
;                             LOAD
;--------------------------------------------------------------------
;
;  LOAD "filename"        load BASIC program, binary executable
;  LOAD "filename",12345  load file as raw binary to address 12345
;  LOAD "filename",*A     load data into numeric array A
;
;  in: HL = BASIC text pointer
;
; out: HL = BASIC text pointer
;       Z = loaded OK, A = filetype
;
; file type detection
; -------------------
; BAS:   file starts with CAQ BASIC header (32 bytes)
; CAQ:   file starts with CAQ ARRAY header (19 bytes)
; BIN:   file starts with BINARY header (4 bytes $C9,$C3,nnnn = RET, JP nnnn)
; TXT:   file starts with 7 bit ASCII character
; else:  unknown type (raw binary)
;
ST_LOAD:
    call    dos__getfilename      ; filename -> FileName
    jp      z,ST_LOADFILE         ; good filename?
    push    hl                    ; push BASIC text pointer
    ld      e,a
    cp      FC_ERR                ; if Function Call error then show DOS error
    jp      nz,_stl_do_error      ; else show BASIC error code
    ld      a,ERROR_BAD_NAME
    jp      _stl_show_error       ; break with bad filename error
; load file with filename in FileName
ST_LOADFILE:
    xor     a
    ld      (FILETYPE),a          ; filetype unknown
    ld      (DOSFLAGS),a          ; clear all DOS flags
_stl_getarg:
    call    get_arg               ; get next non-space character
    cp      ','
    jr      nz,_stl_start         ; if not ',' then no arg
    call    get_next
    cp      $aa                   ; token for '*'
    jr      nz,_stl_addr
_stl_arg_array:
    inc     hl                    ; skip '*' token
    ld      a,1
    ld      (FORFLG),a            ; set array flag
    call    GETVAR                ; get array (out: BC = address, DE = length)
    ld      (FORFLG),a            ; clear array flag
    jp      nz,ERROR_FC           ; FC Error if array not found
    call    TSTNUM                ; TM error if not numeric
_stl_array_parms:
    push    hl                    ; push BASIC text pointer
    ld      h,b
    ld      l,c                   ; HL = address
    ld      c,(hl)
    ld      b,0                   ; BC = index
    add     hl,bc
    add     hl,bc
    inc     hl                    ; HL = array data
    ld      (BINSTART),hl
    dec     de
    dec     de                    ; subtract array header to get data length
    dec     de
    ld      (BINLEN),de
    ld      a,1<<DF_ARRAY
    ld      (DOSFLAGS),a          ; set 'loading to array' flag
    pop     hl                    ; POP text pointer
    jr      _stl_start
_stl_addr:
    call    GETNUM                ; get number
    call    DEINT                 ; convert to 16 bit integer
    ld      (BINSTART),de
    ld      a,1<<DF_ADDR
    ld      (DOSFLAGS),a          ; load address specified
_stl_start:
    push    hl                    ; >>>> push BASIC text pointer
    ld      hl,FileName
    call    usb__open_read        ; try to open file
    jp      nz,_stl_no_file
    ld      de,1                  ; 1 byte to read
    ld      hl,FILETYPE
    call    usb__read_bytes       ; read 1st byte from file into FILETYPE
    jp      nz,_stl_show_error
    ld      de,0                  ; rewind back to start of file
    call    usb__seek
    call    dos__getfiletype      ; get filetype from extn  (eg. "name.BAS")
    or      a
    jp      nz,_stl_type
    ld      a,ERROR_BAD_FILE      ; 0 = bad name
    jp      _stl_show_error
_stl_type
    cp      FT_NONE               ; file type in extn?
    jr      nz,_stl_parse_type
    ld      a,(FILETYPE)          ; no, use type from file
    cp      $80                   ; ASCII text?
    jr      nc,_stl_parse_type
    ld      a,FT_TXT              ; yes, type is TXT
_stl_parse_type:
    ld      (FILETYPE),a
    cp      FT_TXT                ; TXT ?
    jr      z,_stl_txt
    cp      FT_CAQ                ; CAQ ?
    jr      z,_stl_caq
    cp      FT_BAS                ; BAS ?
    jr      z,_stl_bas
    cp      FT_BIN                ; BIN ?
    jr      z,_stl_bin
; unknown filetype
    ld      a,(DOSFLAGS)
    bit     DF_ADDR,a             ; address specified?
    jr      nz,_stl_load_bin
    bit     DF_ARRAY,a            ; no, loading to array?
    jr      nz,_stl_bas
    jp      _stl_no_addr
; TXT
_stl_txt:
    ld      a,(DOSFLAGS)
    bit     DF_ADDR,a             ; address specified?
    jr      nz,_stl_load_bin      ; yes, load text to address
; view text here ???
    jp      _stl_no_addr          ; no, error
; BIN
_stl_bin:
    call    usb__read_byte        ; read 1st byte from file
    jp      nz,_stl_read_error
    cp      $BF                   ; CP A instruction?
    jp      nz,_stl_raw
    call    usb__read_byte        ; yes, read 2nd byte from file
    jp      nz,_stl_read_error
    cp      $DA                   ; JP C instruction?
    jp      nz,_stl_raw
; binary with header
    ld      a,(DOSFLAGS)
    bit     DF_ADDR,a             ; yes, address specified by user?
    jr      nz,_stl_load_bin
    ld      hl,BINSTART
    ld      de,2
    call    usb__read_bytes       ; no, read load address
    jp      nz,_stl_read_error
    dec     e
    dec     e                     ; got 2 bytes?
    jr      z,_stl_load_bin       ; yes,
    jp      _stl_no_addr          ; no, error
; raw binary (no header)
_stl_raw:
    ld      a,(DOSFLAGS)
    bit     DF_ADDR,a             ; address specified by user?
    jp      z,_stl_no_addr        ; no, error
; load binary file to address
_stl_load_bin:
    ld      de,0
    call    usb__seek             ; rewind to start of file
    ld      a,FT_BIN
    ld      (FILETYPE),a          ; force type to BIN
    ld      hl,(BINSTART)         ; HL = address
    jr      _stl_read             ; read file into RAM

; BASIC program or array, has CAQ header
_stl_caq:
_stl_bas:
    ld      a,(DOSFLAGS)
    bit     DF_ADDR,a             ; address specified?
    jr      nz,_stl_load_bin      ; yes, load as raw binary
    call    st_read_sync          ; no, read 1st CAQ sync sequence
    jr      nz,_stl_bad_file
    ld      hl,FileName
    ld      de,6                  ; read internal tape name
    call    usb__read_bytes
    jr      nz,_stl_bad_file
    ld      a,(DOSFLAGS)
    bit     DF_ARRAY,a            ; loading into array?
    jr      z,_stl_basprog
; Loading array
    ld      hl,FileName
    ld      b,6                   ; 6 chars in name
    ld      a,'#'                 ; all chars should be '#'
_stl_array_id:
    cp      (hl)
    jr      nz,_stl_bad_file      ; if not '#' then bad tape name
    djnz    _stl_array_id
    ld      hl,(BINSTART)         ; HL = array data address
    ld      de,(BINLEN)           ; DE = array data length
    jr      _stl_read_len         ; read file into array
; loading BASIC program
_stl_basprog:
    call    st_read_sync          ; read 2nd CAQ sync sequence
    jr      nz,_stl_bad_file
    ld      hl,(BASTART)          ; HL = start of BASIC program
    ld      de,$ffff              ; DE = read to end of file
    call    usb__read_bytes       ; read BASIC program into RAM
    jr      nz,_stl_read_error
_stl_bas_end:
    dec     hl
    xor     a
    cp      (hl)                  ; back up to last line of BASIC program
    jr      z,_stl_bas_end
    inc     hl
    inc     hl
    inc     hl                    ; forward past 3 zeros = end of BASIC program
    inc     hl
    ld      (BASEND),hl           ; set end of BASIC program
    call    Init_BASIC            ; clear variables etc. and update line addresses
    ld      a,FT_BAS
    ld      (FILETYPE),a          ; filetype is BASIC
    jr      _stl_done
; read file into RAM
; HL = load address
_stl_read:
    ld      de,$ffff              ; set length to max (will read to end of file)
_stl_read_len:
    call    usb__read_bytes       ; read file into RAM
    jr      z,_stl_done           ; if good load then done
_stl_read_error:
    ld      a,ERROR_READ_FAIL     ; disk error while reading
    jr     _stl_show_error
_stl_no_file:
    ld      a,ERROR_NO_FILE       ; file not found
    jr      _stl_show_error
_stl_bad_file:
    ld      a,ERROR_BAD_FILE      ; file type incompatible with load method
    jr      _stl_show_error
_stl_no_addr:
    ld      a,ERROR_NO_ADDR       ; no load address specified
_stl_show_error:
    call    _show_error           ; print DOS error message (A = error code)
    call    usb__close_file       ; close file (if opened)
    ld      e,FC_ERR              ; Function Call error
_stl_do_error:
    pop     hl                    ; restore BASIC text pointer
    jp      DO_ERROR              ; return to BASIC with error code in E
_stl_done:
    call    usb__close_file       ; close file
    ld      a,(FILETYPE)
    cp      a                     ; Z = OK
    pop     hl                    ; restore BASIC text pointer
    ret

;-------------------------------------------------
;           Print DOS error message
;-------------------------------------------------
;
;  in: A = error code
;
ERROR_NO_CH376    equ   1 ; CH376 not responding
ERROR_NO_USB      equ   2 ; not in USB mode
ERROR_MOUNT_FAIL  equ   3 ; drive mount failed
ERROR_BAD_NAME    equ   4 ; bad name
ERROR_NO_FILE     equ   5 ; no file
ERROR_FILE_EMPTY  equ   6 ; file empty
ERROR_BAD_FILE    equ   7 ; file header mismatch
ERROR_NO_ADDR     equ   8 ; no load address in binary file
ERROR_READ_FAIL   equ   9 ; read error
ERROR_WRITE_FAIL  equ  10 ; write error
ERROR_CREATE_FAIL equ  11 ; can't create file
ERROR_NO_DIR      equ  12 ; can't open directory
ERROR_PATH_LEN    equ  13 ; path too long

ERROR_UNKNOWN     equ  14 ; other disk error

_show_error:
    cp      ERROR_UNKNOWN        ; known error?
    jr      c,.index             ; yes,
    push    af                   ; no, push error code
    ld      hl,unknown_error_msg
    call    prtstr               ; print "disk error $"
    pop     af                   ; pop error code
    call    printhex
    jp      PRNCRLF
.index:
    ld      hl,_error_messages
    dec     a
    add     a,a
    add     l
    ld      l,a
    ld      a,h
    adc     0
    ld      h,a                  ; index into error message list
    ld      a,(hl)
    inc     hl
    ld      h,(hl)               ; hl = error message
    ld      l,a
    call    prtstr               ; print error message
    jp      PRNCRLF

_error_messages:
    dw      no_376_msg           ; 1
    dw      no_disk_msg          ; 2
    dw      no_mount_msg         ; 3
    dw      bad_name_msg         ; 4
    dw      no_file_msg          ; 5
    dw      file_empty_msg       ; 6
    dw      bad_file_msg         ; 7
    dw      no_addr_msg          ; 8
    dw      read_error_msg       ; 9
    dw      write_error_msg      ;10
    dw      create_error_msg     ;11
    dw      open_dir_error_msg   ;12
    dw      path_too_long_msg    ;13

no_376_msg:
    db      "no CH376",0
no_disk_msg:
    db      "no USB",0
no_mount_msg:
    db      "no disk",0
bad_name_msg:
    db      "invalid name",0
no_file_msg:
    db      "file not found",0
file_empty_msg
    db      "file empty",0
bad_file_msg:
    db      "filetype mismatch",0
no_addr_msg:
    db      "no load address",0
read_error_msg:
    db      "read error",0
write_error_msg:
    db      "write error",0
create_error_msg:
    db      "file create error",0
open_dir_error_msg:
    db      "directory not found",0
path_too_long_msg:
    db      "path too long",0
unknown_error_msg:
    db      "disk error $",0

;--------------------------------------------------------------------
;                  Read CAQ Sync Sequence
;--------------------------------------------------------------------
; CAQ BASIC header is 12x$FF, $00, 6 bytes filename, 12x$FF, $00.
; This subroutine reads and checks the sync sequence 12x$FF, $00.
;
; out: Z = OK
;     NZ = bad header
;
; uses: A, B
;
st_read_sync:
    ld      b,12
_st_read_caq_lp1
    call    usb__read_byte
    ret     nz
    inc     a
    ret     nz                    ; NZ if not $FF
    djnz    _st_read_caq_lp1
    call    usb__read_byte
    ret     nz
    and     a                     ; Z if $00
    ret


;--------------------------------------------------------------------
;                   Initialize BASIC Program
;--------------------------------------------------------------------
; Resets variables, arrays, string space etc.
; Updates nextline pointers to match location of BASIC program in RAM
;
Init_BASIC:
        ld      hl,(BASTART)
        dec     hl
        ld      (TMPSTAT),hl       ; set next statement to start of program
        ld      (RESTORE),hl       ; set RESTORE to start of program
        ld      hl,(RAMTOP)
        ld      (FRETOP),hl        ; clear string space
        ld      hl,(BASEND)
        ld      (ARYTAB),hl        ; clear simple variables
        ld      (ARYEND),hl        ; clear array table
        ld      hl,STRBUF+2
        ld      (STRBUF),hl        ; clear string buffer
        xor     a
        ld      l,a
        ld      h,a
        ld      (CONTPOS),hl       ; set CONTinue position to 0
        ld      (FORFLG),a         ; clear locator flag
        ld      ($38de),hl         ; clear array pointer???
_link_lines:
        ld      de,(BASTART)       ; DE = start of BASIC program
_ibl_next_line:
        ld      h,d
        ld      l,e                ; HL = DE
        ld      a,(hl)
        inc     hl                 ; test nextline address
        or      (hl)
        jr      z,_ibl_done        ; if $0000 then done
        inc     hl
        inc     hl                 ; skip line number
        inc     hl
        xor     a                  ; end of line = $00
_ibl_find_eol:
        cp      (hl)               ; search for end of line
        inc     hl
        jr      nz,_ibl_find_eol
        ex      de,hl              ; HL = current line, DE = next line
        ld      (hl),e
        inc     hl                 ; set address of next line
        ld      (hl),d
        jr      _ibl_next_line
_ibl_done:
        ret


;--------------------------------------------------------------------
;                SAVE "filename" (,address,length)
;--------------------------------------------------------------------
;  SAVE "filename"             save BASIC program
;  SAVE "filename",addr,len    save binary data
;
ST_SAVE:
    xor     a
    ld      (DOSFLAGS),a        ; clear all flags
    call    dos__getfilename    ; filename -> FileName
    jr      z,ST_SAVEFILE
    push    hl                  ; push BASIC text pointer
    ld      e,a                 ; E = error code
    cp      FC_ERR
    jp      nz,_sts_error       ; if not FC error then show BASIC error code
    ld      a,ERROR_BAD_NAME
    jp      DO_ERROR            ; bad filename, quit to BASIC
; save with filename in FileName
ST_SAVEFILE:
    call    get_arg             ; get current char (skipping spaces)
    cp      ','
    jr      nz,_sts_open        ; if not ',' then no args so saving BASIC program
    call    get_next
    cp      $aa                 ; '*' token?
    jr      nz,_sts_num         ; no, parse binary address & length
    inc     hl                  ; yes, skip token
    ld      a,1
    ld      (FORFLG),a          ; flag = array
    call    GETVAR              ; BC = array address, DE = array length
    ld      (FORFLG),a          ; clear flag
    jp      nz,ERROR_FC         ; report FC Error if array not found
    call    TSTNUM              ; TM error if not numeric
    call    get_next
    cp      'A'
    jr      c,_sts_array
    inc     hl                  ; skip 2nd letter of array name ???
_sts_array:
    push    hl
    ld      h,b
    ld      l,c                 ; HL = address
    ld      c,(hl)
    ld      b,0                 ; BC = index
    add     hl,bc
    add     hl,bc
    inc     hl                  ; HL = array data
    ld      (BINSTART),hl
    dec     de
    dec     de                  ; subtract array header to get data length
    dec     de
    ld      (BINLEN),de
    ld      a,1<<DF_ARRAY
    ld      (DOSFLAGS),a        ; flag saving array
    pop     hl
    jr      _sts_open
; parse address, length
_sts_num:
    call    GETNUM              ; get address
    call    DEINT               ; convert to 16 bit integer
    ld      (BINSTART),de       ; set address
    ld      a,1<<DF_ADDR
    ld      (DOSFLAGS),a        ; flag load address present
    call    get_arg             ; get next char from text, skipping spaces
    CHKNXT  ","                 ; skip ',' (syntax error if not ',')
    call    GETNUM              ; get length
    call    DEINT               ; convert to 16 bit integer
    ld      (BINLEN),de         ; store length
; create new file
_sts_open:
    push    hl                  ; PUSH BASIC text pointer
    ld      hl,FileName
    call    usb__open_write     ; create/open new file
    jr      nz,_sts_open_error
    ld      a,(DOSFLAGS)
    bit     DF_ADDR,a
    jr      nz,_sts_binary
; saving BASIC program or array
    call    st_write_sync       ; write caq sync 12 x $FF, $00
    jr      nz,_sts_write_error
    ld      a,(DOSFLAGS)
    bit     DF_ARRAY,a          ; saving array?
    jr      z,_sts_bas
; saving array
    ld      hl,_array_name      ; "######"
    ld      de,6
    call    usb__write_bytes
    jr      nz,_sts_write_error
    jr      _sts_binary         ; write array
; saving BASIC program
_sts_bas:
    ld      hl,FileName
    ld      de,6                ; write 1st 6 chars of filename
    call    usb__write_bytes
    jr      nz,_sts_write_error
    call    st_write_sync       ; write 2nd caq sync $FFx12,$00
    jr      nz,_sts_write_error
    ld      de,(BASTART)        ; DE = start of BASIC program
    ld      hl,(BASEND)         ; HL = end of BASIC program
    or      a
    sbc     hl,de
    ex      de,hl               ; HL = start, DE = length of BASIC program
    jr      _sts_write_data
; saving BINARY
_sts_binary:
    ld      a,$c9               ; write $c9 = RET
    call    usb__write_byte
    jr      nz,_sts_write_error
    ld      a,$c3               ; write $c3 = JP
    call    usb__write_byte
    jr      nz,_sts_write_error
    ld      hl,(BINSTART)       ; hl = binary load address
    ld      a,l
    call    usb__write_byte
    jr      nz,_sts_write_error
    ld      a,h                 ; write binary load address
    call    usb__write_byte
    jr      nz,_sts_write_error
    ld      de,(BINLEN)
; save data (HL = address, DE = length)
_sts_write_data:
    call    usb__write_bytes    ; write data block to file
    push    af
    call    usb__close_file     ; close file
    pop     af
    jr      z,_sts_done         ; if wrote OK then done
; error while writing
_sts_write_error:
    ld      a,ERROR_WRITE_FAIL
    jr      _sts_show_error
; error opening file
_sts_open_error:
    ld      a,ERROR_CREATE_FAIL
_sts_show_error:
    call    _show_error         ; show DOS error message (A = error code)
    ld      e,FC_ERR
_sts_error:
    pop     hl
    jp      DO_ERROR            ; return to BASIC with error code in E
_sts_done:
    pop     hl                  ; restore BASIC text pointer
    ret

_array_name:
    db      "######"


;--------------------------------------------------------------------
;             Write CAQ Sync Sequence  12x$FF, $00
;--------------------------------------------------------------------
; uses: A, B
;
st_write_sync:
    ld      b,12
.write_caq_loop:
    ld      a,$FF
    call    usb__write_byte     ; write $FF
    ret     nz                  ; return if error
    djnz    .write_caq_loop
    ld      a,$00
    jp      usb__write_byte     ; write $00


;--------------------------------------------------------------------
;                      Catalog Disk
;--------------------------------------------------------------------
;
; Minimalist directory listing (shows all filenames)
;
ST_CAT:
    push    hl                      ; save BASIC text pointer
    LD      A,$0D                   ; print carriage return
    CALL    PRNCHR1
    ld      a,23
    ld      (LISTCNT),a             ; set initial number of lines per page
.cat_disk:
    call    usb__ready              ; check for USB disk
    jr      nz,.disk_error
    call    usb__open_dir           ; open '*' for all files in directory
    jr      z,.cat_loop
; usb_ready or open "*" failed
.disk_error:
    call    _show_error             ; show error code
    pop     hl
    ld      e,FC_ERR
    jp      DO_ERROR                ; return to BASIC with FC error
.cat_loop:
    LD      A,CH376_CMD_RD_USB_DATA
    OUT     (CH376_CONTROL_PORT),A  ; command: read USB data (directory entry)
    IN      A,(CH376_DATA_PORT)     ; A = number of bytes in CH376 buffer
    OR      A                       ; if bytes = 0 then read next entry
    JR      Z,.cat_next
    LD      HL,-16
    ADD     HL,SP                   ; allocate 16 bytes on stack
    LD      SP,HL
    LD      B,12                    ; B = 11 bytes filename, 1 byte file attributes
    LD      C,CH376_DATA_PORT
    INIR                            ; get filename, attributes
    LD      B,16
.absorb_bytes:
    IN      A,(CH376_DATA_PORT)     ; absorb bytes until filesize
    DJNZ    .absorb_bytes
    LD      B,4                     ; B = 4 bytes file size
.read_size:
    INIR                            ; get file size
    LD      BC,-5
    ADD     HL,BC                   ; HL = attributes
    LD      A,(HL)                  ; get attributes
    LD      BC,-11                  ; HL = filename
    ADD     HL,BC
    LD      C,A                     ; C = attributes
    LD      B,8                     ; 8 chars in file name
    BIT     ATTR_B_DIRECTORY,C
    JR      Z,.cat_name
    LD      A,'<'
    CALL    PRNCHR1                 ; print '<' in front of directory name
.cat_name:
    LD      A,(HL)
    INC     HL
    CALL    PRNCHR1                 ; print name char
    DJNZ    .cat_name
    BIT     ATTR_B_DIRECTORY,C
    JR      NZ,.extn
    LD      A," "                   ; print ' ' between file name and extension
.separator:
    CALL    PRNCHR
.extn:
    LD      A,(HL)
    INC     HL
    CALL    PRNCHR                  ; print 1st extn char
    LD      A,(HL)
    INC     HL
    CALL    PRNCHR                  ; print 2nd extn char
    LD      A,(HL)
    CP      ' '
    JR      NZ,.last                ; if 3rd extn char is SPACE
    BIT     ATTR_B_DIRECTORY,C      ; and name is directory then
    JR      Z,.last
    LD      A,'>'                   ; replace with '>'
.last:
    CALL    PRNCHR                  ; print 3rd extn char
    LD      HL,16
    ADD     HL,SP                   ; clean up stack
    LD      SP,HL
    LD      A,(CURCOL)
    AND     A                       ; if column = 0 then already on next line
    JR      Z,.cat_go
    LD      A," "                   ; else padding space after filename
    CALL    PRNCHR
.cat_go:
    LD      A,CH376_CMD_FILE_ENUM_GO
    OUT     (CH376_CONTROL_PORT),A  ; command: read next filename
    CALL    usb__wait_int           ; wait until done
.cat_next:
    CP      CH376_INT_DISK_READ     ; more entries?
    JR      Z,.cat_loop             ; yes, get next entry
.cat_done:
    pop     hl                      ; restore BASIC text pointer
    RET

;--------------------------------------------------------------------
;                   Disk Directory Listing
;--------------------------------------------------------------------
; Display directory listing of all files, or only those which match
; the wildcard pattern.
;
; Listing includes details such as file size, volume label etc.
;
; DIR "wildcard"   selective directory listing
; DIR              listing all files
;
ST_DIR:
    push    hl                ; PUSH text pointer
    xor     a
    ld      (FileName),a      ; wildcard string = NULL
    call    chkarg            ; is wildcard argument present?
    jr      z,.st_dir_go      ; if no wildcard then show all files
    call    dos__getfilename  ; wildcard -> FileName
    ex      (sp),hl           ; update text pointer on stack
.st_dir_go:
    call    usb_ready         ; check for USB disk (may reset path to root!)
    jr      nz,.error
    call    PRINTSTR          ; print path
    call    PRNCRLF
    call    dos__directory    ; display directory listing
    jr      z,.st_dir_done    ; if successful listing then done
.error:
    call    _show_error       ; else show error message (A = error code)
    ld      e,FC_ERR
    pop     hl
    jp      DO_ERROR          ; return to BASIC with FC error
.st_dir_done:
    pop     hl                ; POP text pointer
    ret

;-----------------------------------------
;      FAT Directory Info structure
;-----------------------------------------
; structure FAT_DIR_INFO
;    STRUCT DIR_Name,11;         ; $00 0
;     UINT8 DIR_Attr;            ; $0B 11
;     UINT8 DIR_NTRes;           ; $0C 12
;     UINT8 DIR_CrtTimeTenth;    ; $0D 13
;    UINT16 DIR_CrtTime;         ; $0E 14
;    UINT16 DIR_CrtDate;         ; $10 16
;    UINT16 DIR_LstAccDate;      ; $12 18
;    UINT16 DIR_FstClusHI;       ; $14 20
;    UINT16 DIR_WrtTime;         ; $16 22
;    UINT16 DIR_WrtDate;         ; $18 24
;    UINT16 DIR_FstClusLO;       ; $1A 26
;    UINT32 DIR_FileSize;        ; $1C 28
; endstruct FAT_DIR_INFO;        ; $20 32

;------------------------------------------------------------------------------
;                     Read and Display Directory
;------------------------------------------------------------------------------
; Reads all filenames in directory, printing only those names that match the
; wildcard pattern.
;
; in: FILENAME = wildcard string (null string for all files)
;
; out: Z = OK, NZ = no disk
;
; uses: A, BC, DE, HL
;
dos__directory:
        LD      A,$0D
        CALL    PRNCHR                  ; print CR
        CALL    usb__open_dir           ; open '*' for all files in directory
        RET     NZ                      ; abort if error (disk not present?)
        ld      a,22
        ld      (LISTCNT),a             ; set initial number of lines per page
.dir_loop:
        LD      A,CH376_CMD_RD_USB_DATA
        OUT     (CH376_CONTROL_PORT),A  ; command: read USB data
        LD      C,CH376_DATA_PORT
        IN      A,(C)                   ; A = number of bytes in CH376 buffer
        CP      32                      ; must be 32 bytes!
        RET     NZ
        LD      B,A
        LD      HL,-32
        ADD     HL,SP                   ; allocate 32 bytes on stack
        LD      SP,HL
        PUSH    HL
        INIR                            ; read directory info onto stack
        POP     HL
        ld      DE,FileName             ; DE = wildcard pattern
        call    usb__wildcard           ; Z if filename matches wildcard
        call    z,dos__prtDirInfo       ; display file info (type, size)
        LD      HL,32
        ADD     HL,SP                   ; clean up stack
        LD      SP,HL
        LD      A,CH376_CMD_FILE_ENUM_GO
        OUT     (CH376_CONTROL_PORT),A  ; command: read next filename
        CALL    usb__wait_int           ; wait until done
.dir_next:
        CP      CH376_INT_DISK_READ     ; more entries?
        JP      Z,.dir_loop             ; yes, get next entry
        CP      CH376_ERR_MISS_FILE     ; Z if end of file list, else NZ
        RET

_dir_msg:
        db      "<dir>",0


;--------------------------------------------------------------------
;                      Print File Info
;--------------------------------------------------------------------
; in: HL = file info structure (32 bytes)
;
; if directory then print "<dir>"
; if file then print size in Bytes, kB or MB
;
dos__prtDirInfo:
        LD      B,8                     ; 8 characters in filename
.dir_name:
        LD      A,(HL)                  ; get next char of filename
        INC     HL
.dir_prt_name:
        call    PRNCHR1                 ; print filename char, with pause if end of screen
        DJNZ    .dir_name
        LD      A," "                   ; space between name and extension
        call    PRNCHR
        LD      B,3                     ; 3 characters in extension
.dir_ext:
        LD      A,(HL)                  ; get next char of extension
        INC     HL
        call    PRNCHR                  ; print extn char
        DJNZ    .dir_ext
        LD      A,(HL)                  ; get file attribute byte
        INC     HL
        AND     ATTR_DIRECTORY          ; directory bit set?
        JR      NZ,.dir_folder
        LD      A,' '                   ; print ' '
        CALL    PRNCHR
        LD      BC,16                   ; DIR_FileSize-DIR_NTres
        ADD     HL,BC                   ; skip to file size
.dir_file_size:
        LD      E,(HL)
        INC     HL                      ; DE = size 15:0
        LD      D,(HL)
        INC     HL
        LD      C,(HL)
        INC     HL                      ; BC = size 31:16
        LD      B,(HL)

        LD      A,B
        OR      C
        JR      NZ,.kbytes
        LD      A,D
        CP      high(10000)
        JR      C,.bytes
        JR      NZ,.kbytes              ; <10000 bytes?
        LD      A,E
        CP      low(10000)
        JR      NC,.kbytes              ; no,
.bytes:
        LD      H,D
        LD      L,E                     ; HL = file size 0-9999 bytes
        JR      .print_bytes
.kbytes:
        LD      L,D
        LD      H,C                     ; C, HL = size / 256
        LD      C,B
        LD      B,'k'                   ; B = 'k' (kbytes)
        LD      A,D
        AND     3
        OR      E
        LD      E,A                     ; E = zero if size is multiple of 1 kilobyte
        SRL     C
        RR      H
        RR      L
        SRL     C                       ; C,HL = size / 1024
        RR      H
        RR      L
        LD      A,C
        OR      A
        JR      NZ,.dir_MB
        LD      A,H
        CP      high(1000)
        JR      C,.dir_round            ; <1000kB?
        JR      NZ,.dir_MB
        LD      A,L
        CP      low(1000)
        JR      C,.dir_round            ; yes
.dir_MB:
        LD      A,H
        AND     3
        OR      L                       ; E = 0 if size is multiple of 1 megabyte
        OR      E
        LD      E,A

        LD      B,'M'                   ; 'M' after number

        LD      L,H
        LD      H,C
        SRL     H
        RR      L                       ; HL = kB / 1024
        SRL     H
        SRL     L
.dir_round:
        LD      A,H
        OR      L                       ; if 0 kB/MB then round up
        JR      Z,.round_up
        INC     E
        DEC     E
        JR      Z,.print_kB_MB          ; if exact kB or MB then don't round up
.round_up:
        INC     HL                      ; filesize + 1
.print_kB_MB:
        LD      A,3                     ; 3 digit number with leading spaces
        CALL    print_integer           ; print HL as 16 bit number
        LD      A,B
        CALL    PRNCHR                  ; print 'k', or 'M'
        JR      .dir_tab
.print_bytes:
        LD      A,4                     ; 4 digit number with leading spaces
        CALL    print_integer           ; print HL as 16 bit number
        LD      A,' '
        CALL    PRNCHR                  ; print ' '
        JR      .dir_tab
.dir_folder:
        LD      HL,_dir_msg             ; print "<dir>"
        call    PRINTSTR
.dir_tab:
        LD      A,(CURCOL)
        CP      19
        RET     Z                       ; if reached center of screen then return
        JR      NC,.tab_right           ; if on right side then fill to end of line
        LD      A,' '
        CALL    PRNCHR                  ; print " "
        JR      .dir_tab
.tab_right:
        LD      A,(CURCOL)
        CP      0
        RET     Z                       ; reached end of line?
        LD      A,' '
        CALL    PRNCHR                  ; no, print " "
        JR      .tab_right

;--------------------------------------------------------
;  Print Integer as Decimal with leading spaces
;--------------------------------------------------------
;   in: HL = 16 bit Integer
;        A = number of chars to print
;
print_integer:
       PUSH     BC
       PUSH     AF
       CALL     INT2STR
       LD       HL,FPSTR+1
       CALL     strlen
       POP      BC
       LD       C,A
       LD       A,B
       SUB      C
       JR       Z,.prtnum
       LD       B,A
.lead_space:
       LD       A," "
       CALL     PRNCHR        ; print leading space
       DJNZ     .lead_space
.prtnum:
       LD       A,(HL)        ; get next digit
       INC      HL
       OR       A             ; return when NULL reached
       JR       Z,.done
       CALL     PRNCHR        ; print digit
       JR       .prtnum
.done:
       POP      BC
       RET

;--------------------------------------------------------------------
;                        Delete File
;--------------------------------------------------------------------
;
ST_KILL:
    call   dos__getfilename  ; filename -> FileName
    push   hl                ; push BASIC text pointer
    jr     z,.goodname
    ld     e,a
    ld     a,ERROR_BAD_NAME
    jr     .do_error
.goodname:
    ld     hl,FileName
    call   usb__delete       ; delete file
    jr     z,.done
    ld     e,FC_ERR
    ld     a,ERROR_NO_FILE
.do_error:
    call   _show_error       ; print error message
    pop    hl                ; pop BASIC text pointer
    jp     DO_ERROR
.done:
    pop    hl                ; pop BASIC text pointer
    ret


;----------------------------------------------------------------
;                         Set Path
;----------------------------------------------------------------
;
;    In:    HL = string to add to path (NOT null-terminated!)
;            A = string length
;
;   out:    DE = original end of path
;            Z = OK
;           NZ = path too long
;
; path with no leading '/' is added to existing path
;         with leading '/' replaces existing path
;        ".." = removes last subdir from path
;
dos__set_path:
        PUSH   BC
        LD     C,A               ; C = string length
        LD     DE,PathName
        LD     A,(DE)
        CP     '/'               ; does current path start with '/'?
        JR     Z,.gotpath
        CALL   usb__root         ; no, create root path
.gotpath:
        INC    DE                ; DE = 2nd char in pathname (after '/')
        LD     B,path.size-1     ; B = max number of chars in pathname (less leading '/')
        LD     A,(HL)
        CP     '/'               ; does string start with '/'?
        JR     Z,.rootdir        ; yes, replace entire path
        JR     .path_end         ; no, goto end of path
.path_end_loop:
        INC    DE                ; advance DE towards end of path
        DEC    B
        JR     Z,.fail           ; fail if path full
.path_end:
        LD     A,(DE)
        OR     A
        JR     NZ,.path_end_loop
; at end-of-path
        LD     A,'.'             ; does string start with '.' ?
        CP     (HL)
        JR     NZ,.subdir        ; no
; "." or ".."
        INC    HL
        CP     (HL)              ; ".." ?
        JR     NZ,.ok            ; no, staying in current directory so quit
.dotdot:
        DEC    DE
        LD     A,(DE)
        CP     '/'               ; back to last '/'
        JR     NZ,.dotdot
        LD     A,E
        CP     low(PathName)     ; at root?
        JR     NZ,.trim
        INC    DE                ; yes, leave root '/' in
.trim:  XOR    A
        LD     (DE),A            ; NULL terminate pathname
        JR     .ok               ; return OK
.rootdir:
        PUSH   DE                ; push end-of-path
        JR     .nextc            ; skip '/' in string, then copy to path
.subdir:
        PUSH   DE                ; push end-of-path before adding '/'
        LD     A,E
        CP     low(PathName)+1   ; at root?
        JR     Z,.copypath       ; yes,
        LD     A,'/'
        LD     (DE),A            ; add '/' separator
        INC    DE
        DEC    B
        JR     Z,.undo           ; if path full then undo
.copypath:
        LD     A,(HL)            ; get next string char
        CALL   dos__char         ; convert to MSDOS
        LD     (DE),A            ; store char in pathname
        INC    DE
        DEC    B
        JR     Z,.undo           ; if path full then undo and fail
.nextc: INC    HL
        DEC    C
        JR     NZ,.copypath      ; until end of string
.nullend:
        XOR    A
        LD     (DE),A            ; NULL terminate pathname
        JR     .copied
; if path full then undo add
.undo:  POP    DE                ; pop original end-of-path
.fail:  XOR    A
        LD     (DE),A            ; remove added subdir from path
        INC    A                 ; return NZ
        JR     .done
.copied:
        POP    DE                ; DE = original end-of-path
.ok     CP     A                 ; return Z
.done:  POP    BC
        RET


;--------------------------------------------------------------------
;                        Get Filename
;--------------------------------------------------------------------
; Get Filename argument from BASIC text or command line.
; May be literal string, or an expression that evaluates to a string
; eg. LOAD "filename"
;     SAVE left$(A$,11)
;
; in:  HL = BASIC text pointer
;
; out: Uppercase filename in FileName, 1-12 chars null-terminated
;      HL = BASIC text pointer
;       z = OK
;      nz = error, A = $08 null string, $18 not a string
;
; uses: BC,DE
;
dos__getfilename:
    call    EVAL              ; evaluate expression
    push    hl                ; save BASIC text pointer
    ld      a,(VALTYP)        ; get type
    dec     a
    jr      nz,.type_mismatch
    call    GETLEN            ; get string and its length
    jr      z,.null_str       ; if empty string then return
    cp      12
    jr      c,.string         ; trim to 12 chars max
    ld      a,12
.string:
    ld      b,a               ; B = string length
    inc     hl
    inc     hl                ; skip to string text pointer
    ld      a,(hl)
    inc     hl
    ld      h,(hl)
    ld      l,a               ; hl = string text pointer
    ld      de,FileName       ; de = filename buffer (13 bytes)
.copy_str:
    ld      a,(hl)            ; get string char
    CALL    UpperCase         ; 'a-z' -> 'A-Z'
    CP      '='
    jr      nz,.dos_char
    LD      A,'~'             ; convert '=' to '~'
.dos_char:
    ld      (de),a            ; copy char to filename
    inc     hl
    inc     de
    djnz    .copy_str         ; loop back to copy next char
    jr      .got_name         ; done
.null_str:
    ld      a,$08             ; function code error
    jr      .done
.type_mismatch:
    ld      a,$18             ; type mismatch error
    jr      .done
.got_name:
    xor     a                 ; no error
    ld      (de),a            ; terminate filename
.done:
    pop     hl                ; restore BASIC text pointer
    or      a                 ; test error code
    ret


;--------------------------------------------------------------------
;                 Determine File Type from Extension
;--------------------------------------------------------------------
; Examines extension to determine filetype eg. "name.BIN" is binary
;
;  out: A = file type:-
;             0       bad name
;          FT_NONE    no extension
;          FT_OTHER   unknown extension
;          FT_TXT     ASCII text
;          FT_BIN     binary code/data
;          FT_BAS     BASIC program
;          FT_CAQ     tape file
;
dos__getfiletype:
    push  hl
    ld    hl,FIleName
    ld    b,-1             ; B = position of '.' in filename
_gft_find_dot
    inc   b
    ld    a,b
    cp    9                ; error if name > 8 charcters long
    jr    nc,_gft_error
    ld    a,(hl)           ; get next char in filename
    inc   hl
    cp    "."              ; is it a '.'?
    jr    z,_gft_got_dot
    or    a                ; end of string?
    jr    z,_gft_no_extn
    jr    _gft_find_dot    ; continue searching for '.'
_gft_got_dot:
    ld    a,b
    or    a                ; error if no name
    jr    z,_gft_error
    ld    a,(hl)
    or    a                ; if '.' is last char then no extn
    jr    z,_gft_no_extn
    ld    de,extn_list     ; DE = list of extension names
    jr    _gft_search
_gft_skip:
    or    a
    jr    z,_gft_next
    ld    a,(de)
    inc   de               ; skip extn name in list
    jr    _gft_skip
_gft_next:
    inc   de               ; skip filetype in list
_gft_search:
    ld    a,(de)
    or    a                ; end of filetypes list?
    jr    z,_gft_other
    push  hl
    call  strcmp           ; compare extn to name in list
    pop   hl
    jr    nz,_gft_skip     ; if no match then keep searching
_gft_got_extn:
    ld    a,(de)           ; get filetype
    jr    _gft_done
_gft_other:
    ld    a,FT_OTHER       ; unknown filetype
    jr    _gft_done
_gft_no_extn:
    ld    a,FT_NONE        ; no extn (eg. "name", "name.")
    jr    _gft_done
_gft_error:
    xor   a                ; 0 = bad name
_gft_done:
    pop   hl
    ret

extn_list:
    db   "TXT",0,FT_TXT    ; ASCII text
    db   "BIN",0,FT_BIN    ; binary code/data
    db   "BAS",0,FT_BAS    ; BASIC program
    db   "CAQ",0,FT_CAQ    ; tape file (BASIC, Array, ???)
    db   0

;----------------------------------------------------------
;      Convert FAT filename to DOS filename
;----------------------------------------------------------
;
; eg. "NAME    EXT" -> "NAME.EXT",0
;
;   in: HL = FAT filename (11 chars)
;       DE = DOS filename string (13 chars)
;
; NOTE: source and destination can be the same string, but
;       string must have space for 13 chars.
;
dos__name:
   push  bc
   push  de
   push  hl
   ld    b,8
.getname:
   ld    a,(hl)       ; get name char
   inc   hl
   cp    " "          ; don't copy spaces
   jr    z,.next
   ld    (de),a       ; store name char
   inc   de
.next:
   djnz  .getname
   ld    a,(hl)       ; A = 1st extn char
   cp    " "
   jr    z,.end       ; if " " then no extn
   ex    de,hl
   ld    (hl),"."     ; add separator
   ex    de,hl
   inc   de
   ld    b,3          ; 3 chars in extn
.extn:
   inc   hl
   ld    c,(hl)       ; C = next extn char
   ld    (de),a       ; store current extn char
   inc   de
   dec   b
   jr    z,.end       ; if done 3 chars then end of extn
   ld    a,c
   cp    ' '          ; if space then end of extn
   jr    nz,.extn
.end:
   xor   a
   ld    (de),a       ; NULL end of DOS filename string
.done:
   pop   hl
   pop   de
   pop   bc
   ret

;------------------------------------------------------------------------------
;              Convert Character to MSDOS equivalent
;------------------------------------------------------------------------------
;  Input:  A = char
; Output:  A = MDOS compatible char
;
; converts:-
;     lowercase to upppercase
;     '=' -> '~' (in case we cannot type '~' on the keyboard!)
;
dos__char:
        CP      'a'
        JR      C,.uppercase
        CP      'z'+1          ; convert lowercase to uppercase
        JR      NC,.uppercase
        AND     $5f
.uppercase:
        CP      '='
        RET     NZ             ; convert '=' to '~'
        LD      A,'~'
        RET




