; Based on V1.0 of micro-expander ROM

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




;-----------------------------------------------------------------------------
; strlen - String length
;  in: hl = string (null-terminated)
; out:  a = number of characters in string
;-----------------------------------------------------------------------------
strlen:
    push  de
    ld    d,h
    ld    e,l
    xor   a
    dec   hl
_strlen_loop:
    inc   hl
    cp    (hl)
    jr    nz,_strlen_loop
    sbc   hl,de
    ld    a,l
    ex    de,hl
    pop   de
    ret

;-----------------------------------------------------------------------------
; strcmp - Compare strings
;  in: hl = string 1 (null terminated)
;      de = string 2 (null terminated)
; out: Z  = strings equal
;      NZ = not equal
;-----------------------------------------------------------------------------
strcmp:
    ld   a,(de)          ; get char from string 2
    inc  de
    cp  (hl)             ; compare to char in string 1
    inc  hl
    ret  nz              ; return NZ if not equal
    or   a
    jr   nz,strcmp       ; loop until end of strings
    ret                  ; return Z

;-----------------------------------------------------------------------------
; Print null-terminated string
; in: HL = text ending with NULL
;-----------------------------------------------------------------------------
prtstr:
    ld   a,(hl)
    inc  hl
    or   a
    ret  z
    call PRNCHR
    jr   prtstr

;-----------------------------------------------------------------------------
; Check for argument in current statement
;  in: HL = text pointer
; out: NZ = argument present
;       Z = end of statement
;-----------------------------------------------------------------------------
chkarg:
    push hl                   ; save BASIC text pointer
_chkarg_next_char:
    ld   a,(hl)               ; get char
    inc  hl
    cp   ' '                  ; skip spaces
    jr   z,_chkarg_next_char
    cp   ':'                  ; Z if end of statement
    jr   z,_chkarg_done       ; return Z if end of statement
    or   a                    ; Z if end of line
_chkarg_done:
    pop  hl                   ; restore BASIC text pointer
    ret

;-----------------------------------------------------------------------------
; Get next character, skipping spaces
;  in: HL = text pointer
; out: NZ, A = next non-space char, HL = address of char in text
;      Z,  A = 0, HL = end of text
;-----------------------------------------------------------------------------
get_next:                       ; starting at next location
    inc     hl
get_arg:                        ; starting at current location
    ld      a,(hl)
    or      a
    ret     z                   ; return Z if NULL
    cp      ' '
    ret     nz                  ; return NZ if not SPACE
    jr      get_next


;-----------------------------------------------------------------------------
; CD - Change directory
;
; Syntax:
; CD "dirname"  = add 'subdir' to path
; CD "/path"    = set path to '/path'
; CD ""         = no operation
; CD            = show path
;-----------------------------------------------------------------------------
ST_CD:
    push   hl                    ; push BASIC text pointer
    ld     c,a
    call   usb__ready            ; check for USB disk (may reset path to root!)
    jr     nz,.do_error
    ld     a,c
    or     a                     ; any args?
    jr     nz,.change_dir        ; yes,
.show_path:
    ld     hl,PathName
    call   PRINTSTR              ; print path
    call   PRNCRLF
    jr     .done
.change_dir:
    pop    hl                    ; pop BASIC text pointer
    call   EVAL                  ; evaluate expression
    push   hl                    ; push BASIC text pointer
    call   TSTSTR                ; type mismatch error if not string
    call   GETLEN                ; get string and its length
    jr     z,.open               ; if null string then open current directory
    inc    hl
    inc    hl                    ; skip to string text pointer
    ld     b,(hl)
    inc    hl
    ld     h,(hl)                ; hl = string text
    ld     l,b
    call   dos__set_path         ; update path (out: de = end of old path)
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
    ex     de,hl                 ; hl = end of old path
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

;-----------------------------------------------------------------------------
; LOAD
;-----------------------------------------------------------------------------
; LOAD "filename"        load BASIC program, binary executable
; LOAD "filename",12345  load file as raw binary to address 12345
; LOAD "filename",*a     load data into numeric array a
;
;  in: hl = BASIC text pointer
;
; out: hl = BASIC text pointer
;       z = loaded OK, a = filetype
;
; file type detection
; -------------------
; BAS:   file starts with CAQ BASIC header (32 bytes)
; CAQ:   file starts with CAQ ARRAY header (19 bytes)
; BIN:   file starts with BINARY header (4 bytes $C9,$C3,nnnn = ret, jp nnnn)
; TXT:   file starts with 7 bit ASCII character
; else:  unknown type (raw binary)
;-----------------------------------------------------------------------------
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
    call    GETVAR                ; get array (out: bc = address, de = length)
    ld      (FORFLG),a            ; clear array flag
    jp      nz,ERROR_FC           ; FC Error if array not found
    call    TSTNUM                ; TM error if not numeric
_stl_array_parms:
    push    hl                    ; push BASIC text pointer
    ld      h,b
    ld      l,c                   ; hl = address
    ld      c,(hl)
    ld      b,0                   ; bc = index
    add     hl,bc
    add     hl,bc
    inc     hl                    ; hl = array data
    ld      (BINSTART),hl
    dec     de
    dec     de                    ; subtract array header to get data length
    dec     de
    ld      (BINLEN),de
    ld      a,1<<DF_ARRAY
    ld      (DOSFLAGS),a          ; set 'loading to array' flag
    pop     hl                    ; pop text pointer
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
    cp      $BF                   ; cp a instruction?
    jp      nz,_stl_raw
    call    usb__read_byte        ; yes, read 2nd byte from file
    jp      nz,_stl_read_error
    cp      $DA                   ; jp c instruction?
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
    ld      hl,(BINSTART)         ; hl = address
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
    ld      hl,(BINSTART)         ; hl = array data address
    ld      de,(BINLEN)           ; de = array data length
    jr      _stl_read_len         ; read file into array

    ; loading BASIC program
_stl_basprog:
    call    st_read_sync          ; read 2nd CAQ sync sequence
    jr      nz,_stl_bad_file
    ld      hl,(BASTART)          ; hl = start of BASIC program
    ld      de,$ffff              ; de = read to end of file
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
    ; hl = load address
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
    call    _show_error           ; print DOS error message (a = error code)
    call    usb__close_file       ; close file (if opened)
    ld      e,FC_ERR              ; Function Call error
_stl_do_error:
    pop     hl                    ; restore BASIC text pointer
    jp      DO_ERROR              ; return to BASIC with error code in e
_stl_done:
    call    usb__close_file       ; close file
    ld      a,(FILETYPE)
    cp      a                     ; z = OK
    pop     hl                    ; restore BASIC text pointer
    ret

;-----------------------------------------------------------------------------
; Print DOS error message
;
;  in: a = error code
;-----------------------------------------------------------------------------
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

no_376_msg:         db "no CH376", 0
no_disk_msg:        db "no USB", 0
no_mount_msg:       db "no disk", 0
bad_name_msg:       db "invalid name", 0
no_file_msg:        db "file not found", 0
file_empty_msg:     db "file empty", 0
bad_file_msg:       db "filetype mismatch", 0
no_addr_msg:        db "no load address", 0
read_error_msg:     db "read error", 0
write_error_msg:    db "write error", 0
create_error_msg:   db "file create error", 0
open_dir_error_msg: db "directory not found", 0
path_too_long_msg:  db "path too long", 0
unknown_error_msg:  db "disk error $", 0

;-----------------------------------------------------------------------------
; Read CAQ Sync Sequence
;
; CAQ BASIC header is 12x$FF, $00, 6 bytes filename, 12x$FF, $00.
; This subroutine reads and checks the sync sequence 12x$FF, $00.
;
; out: z = OK
;     nz = bad header
;
; uses: a, b
;-----------------------------------------------------------------------------
st_read_sync:
    ld      b,12
_st_read_caq_lp1
    call    usb__read_byte
    ret     nz
    inc     a
    ret     nz                    ; nz if not $FF
    djnz    _st_read_caq_lp1
    call    usb__read_byte
    ret     nz
    and     a                     ; z if $00
    ret

;-----------------------------------------------------------------------------
; Initialize BASIC Program
;
; Resets variables, arrays, string space etc.
; Updates nextline pointers to match location of BASIC program in RAM
;-----------------------------------------------------------------------------
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
    ld      de,(BASTART)       ; de = start of BASIC program
_ibl_next_line:
    ld      h,d
    ld      l,e                ; hl = de
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
    ex      de,hl              ; hl = current line, de = next line
    ld      (hl),e
    inc     hl                 ; set address of next line
    ld      (hl),d
    jr      _ibl_next_line
_ibl_done:
    ret

;-----------------------------------------------------------------------------
; SAVE "filename" (,address,length)
;
;  SAVE "filename"             save BASIC program
;  SAVE "filename",addr,len    save binary data
;-----------------------------------------------------------------------------
ST_SAVE:
    xor     a
    ld      (DOSFLAGS),a        ; clear all flags
    call    dos__getfilename    ; filename -> FileName
    jr      z,ST_SAVEFILE
    push    hl                  ; push BASIC text pointer
    ld      e,a                 ; e = error code
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
    call    GETVAR              ; bc = array address, de = array length
    ld      (FORFLG),a          ; clear flag
    jp      nz,ERROR_FC         ; report FC Error if array not found
    call    TSTNUM              ; TM error if not numeric
    call    get_next
    cp      'a'
    jr      c,_sts_array
    inc     hl                  ; skip 2nd letter of array name ???
_sts_array:
    push    hl
    ld      h,b
    ld      l,c                 ; hl = address
    ld      c,(hl)
    ld      b,0                 ; bc = index
    add     hl,bc
    add     hl,bc
    inc     hl                  ; hl = array data
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
    rst     $08                 ; CHKNXT - skip ',' (syntax error if not ',')
    db      ','
    call    GETNUM              ; get length
    call    DEINT               ; convert to 16 bit integer
    ld      (BINLEN),de         ; store length

    ; create new file
_sts_open:
    push    hl                  ; push BASIC text pointer
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
    ld      de,(BASTART)        ; de = start of BASIC program
    ld      hl,(BASEND)         ; hl = end of BASIC program
    or      a
    sbc     hl,de
    ex      de,hl               ; hl = start, de = length of BASIC program
    jr      _sts_write_data

    ; saving BINARY
_sts_binary:
    ld      a,$c9               ; write $c9 = ret
    call    usb__write_byte
    jr      nz,_sts_write_error
    ld      a,$c3               ; write $c3 = jp
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

    ; save data (hl = address, de = length)
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
    call    _show_error         ; show DOS error message (a = error code)
    ld      e,FC_ERR
_sts_error:
    pop     hl
    jp      DO_ERROR            ; return to BASIC with error code in e
_sts_done:
    pop     hl                  ; restore BASIC text pointer
    ret

_array_name:
    db      "######"

;-----------------------------------------------------------------------------
; Write CAQ Sync Sequence  12x$FF, $00
; uses: a, b
;-----------------------------------------------------------------------------
st_write_sync:
    ld      b,12
.write_caq_loop:
    ld      a,$FF
    call    usb__write_byte     ; write $FF
    ret     nz                  ; return if error
    djnz    .write_caq_loop
    ld      a,$00
    jp      usb__write_byte     ; write $00

;-----------------------------------------------------------------------------
; Disk Directory Listing
;
; Display directory listing of all files, or only those which match
; the wildcard pattern.
;
; Listing includes details such as file size, volume label etc.
;
; DIR "wildcard"   selective directory listing
; DIR              listing all files
;-----------------------------------------------------------------------------
ST_DIR:
    push    hl                ; push text pointer
    xor     a
    ld      (FileName),a      ; wildcard string = NULL
    call    chkarg            ; is wildcard argument present?
    jr      z,.st_dir_go      ; if no wildcard then show all files
    call    dos__getfilename  ; wildcard -> FileName
    ex      (sp),hl           ; update text pointer on stack
.st_dir_go:
    call    usb__ready        ; check for USB disk (may reset path to root!)
    jr      nz,.error
    call    PRINTSTR          ; print path
    call    PRNCRLF
    call    dos__directory    ; display directory listing
    jr      z,.st_dir_done    ; if successful listing then done
.error:
    call    _show_error       ; else show error message (a = error code)
    ld      e,FC_ERR
    pop     hl
    jp      DO_ERROR          ; return to BASIC with FC error
.st_dir_done:
    pop     hl                ; pop text pointer
    ret

;------------------------------------------------------------------------------
; Read and Display Directory
;
; Reads all filenames in directory, printing only those names that match the
; wildcard pattern.
;
; in: FILENAME = wildcard string (null string for all files)
;
; out: z = OK, nz = no disk
;
; uses: a, bc, de, hl
;------------------------------------------------------------------------------
dos__directory:
    ld      a,$0D
    call    PRNCHR                  ; print CR
    call    usb__open_dir           ; open '*' for all files in directory
    ret     nz                      ; abort if error (disk not present?)
    ld      a,22
    ld      (LISTCNT),a             ; set initial number of lines per page
.dir_loop:
    ld      a,CH376_CMD_RD_USB_DATA
    out     (CH376_CONTROL_PORT),a  ; command: read USB data
    ld      c,CH376_DATA_PORT
    in      a,(c)                   ; a = number of bytes in CH376 buffer
    cp      32                      ; must be 32 bytes!
    ret     nz
    ld      b,a
    ld      hl,-32
    add     hl,sp                   ; allocate 32 bytes on stack
    ld      sp,hl
    push    hl
    inir                            ; read directory info onto stack
    pop     hl
    ld      de,FileName             ; de = wildcard pattern
    call    usb__wildcard           ; z if filename matches wildcard
    call    z,dos__prtDirInfo       ; display file info (type, size)
    ld      hl,32
    add     hl,sp                   ; clean up stack
    ld      sp,hl
    ld      a,CH376_CMD_FILE_ENUM_GO
    out     (CH376_CONTROL_PORT),a  ; command: read next filename
    call    usb__wait_int           ; wait until done
.dir_next:
    cp      CH376_INT_DISK_READ     ; more entries?
    jp      z,.dir_loop             ; yes, get next entry
    cp      CH376_ERR_MISS_FILE     ; z if end of file list, else nz
    ret

_dir_msg:
    db      "<DIR>",0

;-----------------------------------------------------------------------------
; Print File Info
;
; in: hl = file info structure (32 bytes)
;
; if directory then print "<dir>"
; if file then print size in Bytes, kB or MB
;-----------------------------------------------------------------------------
dos__prtDirInfo:
    ld      b,8                     ; 8 characters in filename
.dir_name:
    ld      a,(hl)                  ; get next char of filename
    inc     hl
.dir_prt_name:
    call    PRNCHR1                 ; print filename char, with pause if end of screen
    djnz    .dir_name
    ld      a," "                   ; space between name and extension
    call    PRNCHR
    ld      b,3                     ; 3 characters in extension
.dir_ext:
    ld      a,(hl)                  ; get next char of extension
    inc     hl
    call    PRNCHR                  ; print extn char
    djnz    .dir_ext
    ld      a,(hl)                  ; get file attribute byte
    inc     hl
    and     ATTR_DIRECTORY          ; directory bit set?
    jr      nz,.dir_folder
    ld      a,' '                   ; print ' '
    call    PRNCHR
    ld      bc,16                   ; DIR_FileSize-DIR_NTres
    add     hl,bc                   ; skip to file size
.dir_file_size:
    ld      e,(hl)
    inc     hl                      ; de = size 15:0
    ld      d,(hl)
    inc     hl
    ld      c,(hl)
    inc     hl                      ; bc = size 31:16
    ld      b,(hl)

    ld      a,b
    or      c
    jr      nz,.kbytes
    ld      a,d
    cp      high(10000)
    jr      c,.bytes
    jr      nz,.kbytes              ; <10000 bytes?
    ld      a,e
    cp      low(10000)
    jr      nc,.kbytes              ; no,
.bytes:
    ld      h,d
    ld      l,e                     ; hl = file size 0-9999 bytes
    jr      .print_bytes
.kbytes:
    ld      l,d
    ld      h,c                     ; c, hl = size / 256
    ld      c,b
    ld      b,'k'                   ; b = 'k' (kbytes)
    ld      a,d
    and     3
    or      e
    ld      e,a                     ; e = zero if size is multiple of 1 kilobyte
    srl     c
    rr      h
    rr      l
    srl     c                       ; c,hl = size / 1024
    rr      h
    rr      l
    ld      a,c
    or      a
    jr      nz,.dir_MB
    ld      a,h
    cp      high(1000)
    jr      c,.dir_round            ; <1000kB?
    jr      nz,.dir_MB
    ld      a,l
    cp      low(1000)
    jr      c,.dir_round            ; yes
.dir_MB:
    ld      a,h
    and     3
    or      l                       ; e = 0 if size is multiple of 1 megabyte
    or      e
    ld      e,a

    ld      b,'M'                   ; 'M' after number

    ld      l,h
    ld      h,c
    srl     h
    rr      l                       ; hl = kB / 1024
    srl     h
    srl     l
.dir_round:
    ld      a,h
    or      l                       ; if 0 kB/MB then round up
    jr      z,.round_up
    inc     e
    dec     e
    jr      z,.print_kB_MB          ; if exact kB or MB then don't round up
.round_up:
    inc     hl                      ; filesize + 1
.print_kB_MB:
    ld      a,3                     ; 3 digit number with leading spaces
    call    print_integer           ; print hl as 16 bit number
    ld      a,b
    call    PRNCHR                  ; print 'k', or 'M'
    jr      .dir_tab
.print_bytes:
    ld      a,4                     ; 4 digit number with leading spaces
    call    print_integer           ; print hl as 16 bit number
    ld      a,' '
    call    PRNCHR                  ; print ' '
    jr      .dir_tab
.dir_folder:
    ld      hl,_dir_msg             ; print "<dir>"
    call    PRINTSTR
.dir_tab:
    ld      a,(CURCOL)
    cp      19
    ret     z                       ; if reached center of screen then return
    jr      nc,.tab_right           ; if on right side then fill to end of line
    ld      a,' '
    call    PRNCHR                  ; print " "
    jr      .dir_tab
.tab_right:
    ld      a,(CURCOL)
    cp      0
    ret     z                       ; reached end of line?
    ld      a,' '
    call    PRNCHR                  ; no, print " "
    jr      .tab_right

;-----------------------------------------------------------------------------
; Print Integer as Decimal with leading spaces
;
;   in: hl = 16 bit Integer
;        a = number of chars to print
;-----------------------------------------------------------------------------
print_integer:
    push     bc
    push     af
    call     INT2STR
    ld       hl,FPSTR+1
    call     strlen
    pop      bc
    ld       c,a
    ld       a,b
    SUB      c
    jr       z,.prtnum
    ld       b,a
.lead_space:
    ld       a," "
    call     PRNCHR        ; print leading space
    djnz     .lead_space
.prtnum:
    ld       a,(hl)        ; get next digit
    inc      hl
    or       a             ; return when NULL reached
    jr       z,.done
    call     PRNCHR        ; print digit
    jr       .prtnum
.done:
    pop      bc
    ret

;-----------------------------------------------------------------------------
; Delete File
;-----------------------------------------------------------------------------
ST_DEL:
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

;-----------------------------------------------------------------------------
; Set Path
;    In:    hl = string to add to path (NOT null-terminated!)
;            a = string length
;
;   out:    de = original end of path
;            z = OK
;           nz = path too long
;
; path with no leading '/' is added to existing path
;         with leading '/' replaces existing path
;        ".." = removes last subdir from path
;-----------------------------------------------------------------------------
dos__set_path:
    push   bc
    ld     c,a               ; c = string length
    ld     de,PathName
    ld     a,(de)
    cp     '/'               ; does current path start with '/'?
    jr     z,.gotpath
    call   usb__root         ; no, create root path
.gotpath:
    inc    de                ; de = 2nd char in pathname (after '/')
    ld     b,PathSize-1     ; b = max number of chars in pathname (less leading '/')
    ld     a,(hl)
    cp     '/'               ; does string start with '/'?
    jr     z,.rootdir        ; yes, replace entire path
    jr     .path_end         ; no, goto end of path
.path_end_loop:
    inc    de                ; advance de towards end of path
    dec    b
    jr     z,.fail           ; fail if path full
.path_end:
    ld     a,(de)
    or     a
    jr     nz,.path_end_loop

    ; at end-of-path
    ld     a,'.'             ; does string start with '.' ?
    cp     (hl)
    jr     nz,.subdir        ; no

    ; "." or ".."
    inc    hl
    cp     (hl)              ; ".." ?
    jr     nz,.ok            ; no, staying in current directory so quit
.dotdot:
    dec    de
    ld     a,(de)
    cp     '/'               ; back to last '/'
    jr     nz,.dotdot
    ld     a,e
    cp     low(PathName)     ; at root?
    jr     nz,.trim
    inc    de                ; yes, leave root '/' in
.trim:
    xor    a
    ld     (de),a            ; NULL terminate pathname
    jr     .ok               ; return OK
.rootdir:
    push   de                ; push end-of-path
    jr     .nextc            ; skip '/' in string, then copy to path
.subdir:
    push   de                ; push end-of-path before adding '/'
    ld     a,e
    cp     low(PathName)+1   ; at root?
    jr     z,.copypath       ; yes,
    ld     a,'/'
    ld     (de),a            ; add '/' separator
    inc    de
    dec    b
    jr     z,.undo           ; if path full then undo
.copypath:
    ld     a,(hl)            ; get next string char
    call   dos__char         ; convert to MSDOS
    ld     (de),a            ; store char in pathname
    inc    de
    dec    b
    jr     z,.undo           ; if path full then undo and fail
.nextc:
    inc    hl
    dec    c
    jr     nz,.copypath      ; until end of string
.nullend:
    xor    a
    ld     (de),a            ; NULL terminate pathname
    jr     .copied

    ; if path full then undo add
.undo:
    pop    de                ; pop original end-of-path
.fail:
    xor    a
    ld     (de),a            ; remove added subdir from path
    inc    a                 ; return nz
    jr     .done
.copied:
    pop    de                ; de = original end-of-path
.ok:
    cp     a                 ; return z
.done:
    pop    bc
    ret

;-----------------------------------------------------------------------------
; Get Filename
;
; Get Filename argument from BASIC text or command line.
; May be literal string, or an expression that evaluates to a string
; eg. LOAD "filename"
;     SAVE left$(a$,11)
;
; in:  hl = BASIC text pointer
;
; out: Uppercase filename in FileName, 1-12 chars null-terminated
;      hl = BASIC text pointer
;       z = OK
;      nz = error, a = $08 null string, $18 not a string
;
; uses: bc,de
;-----------------------------------------------------------------------------
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
    ld      b,a               ; b = string length
    inc     hl
    inc     hl                ; skip to string text pointer
    ld      a,(hl)
    inc     hl
    ld      h,(hl)
    ld      l,a               ; hl = string text pointer
    ld      de,FileName       ; de = filename buffer (13 bytes)
.copy_str:
    ld      a,(hl)            ; get string char
    call    to_upper         ; 'a-z' -> 'A-Z'
    cp      '='
    jr      nz,.dos_char
    ld      a,'~'             ; convert '=' to '~'
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

;-----------------------------------------------------------------------------
; Determine File Type from Extension
;
; Examines extension to determine filetype eg. "name.BIN" is binary
;
;  out: a = file type:-
;             0       bad name
;          FT_NONE    no extension
;          FT_OTHER   unknown extension
;          FT_TXT     ASCII text
;          FT_BIN     binary code/data
;          FT_BAS     BASIC program
;          FT_CAQ     tape file
;-----------------------------------------------------------------------------
dos__getfiletype:
    push  hl
    ld    hl,FIleName
    ld    b,-1             ; b = position of '.' in filename
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
    ld    de,extn_list     ; de = list of extension names
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

;-----------------------------------------------------------------------------
; Convert FAT filename to DOS filename
;
; eg. "NAME    EXT" -> "NAME.EXT",0
;
;   in: hl = FAT filename (11 chars)
;       de = DOS filename string (13 chars)
;
; NOTE: source and destination can be the same string, but
;       string must have space for 13 chars.
;-----------------------------------------------------------------------------
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
    ld    a,(hl)       ; a = 1st extn char
    cp    " "
    jr    z,.end       ; if " " then no extn
    ex    de,hl
    ld    (hl),"."     ; add separator
    ex    de,hl
    inc   de
    ld    b,3          ; 3 chars in extn
.extn:
    inc   hl
    ld    c,(hl)       ; c = next extn char
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

;-----------------------------------------------------------------------------
; Convert character to MSDOS equivalent
;
;  Input:  a = char
; Output:  a = MDOS compatible char
;
; converts:-
;     lowercase to uppercase
;     '=' -> '~' (in case we cannot type '~' on the keyboard!)
;-----------------------------------------------------------------------------
dos__char:
    call    to_upper
    cp      '='
    ret     nz             ; convert '=' to '~'
    ld      a, '~'
    ret
