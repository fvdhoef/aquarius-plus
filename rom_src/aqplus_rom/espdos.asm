;-----------------------------------------------------------------------------
; espdos.asm
;-----------------------------------------------------------------------------

FILETYPE_BAS: equ 1
FILETYPE_CAQ: equ 2

;-----------------------------------------------------------------------------
; LOAD
;-----------------------------------------------------------------------------
ST_LOAD:
    ; Close any open files
    call    esp_close_all

    ; Get string parameter
    call    get_string_parameter

    ; Determine file type based on file extension
    push    hl
    call    path_get_filetype
    ld      (FILETYPE), a
    pop     hl

    ; Check for second parameter
    call    get_arg
    cp      ','
    jr      nz, .noarg
    call    get_next
    cp      $AA                 ; Token for '*'
    jr      z, .array
    jr      .addr

    ; No argument
.noarg:
    ; Only .BAS and .CAQ files can be loaded without argument
    ld      a, (FILETYPE)
    cp      FILETYPE_BAS
    jp      z, load_basic_program
    cp      FILETYPE_CAQ
    jp      z, load_basic_program

    ; Other filetypes need operand
    jp      MOERR


    ; Load into array
.array:
    ; Skip '*' token
    inc     hl

    ; Get pointer to array variable
    ld      a, 1
    ld      (SUBFLG), a         ; Set array flag
    call    PTRGET              ; Get array (out: BC = pointer to number of dimensions, DE = next array entry)
    ld      (SUBFLG), a         ; Clear array flag
    jp      nz, FCERR           ; FC Error if array not found
    call    CHKNUM              ; TM error if not numeric

    ; Get start address and length of array
    push    hl                  ; Push BASIC text pointer
    ld      h, b
    ld      l, c                ; HL = address
    ld      c, (hl)
    ld      b, 0                ; BC = index
    add     hl, bc
    add     hl, bc
    inc     hl                  ; HL = array data
    ld      (BINSTART), hl
    dec     de
    dec     de                  ; Subtract array header to get data length
    dec     de
    ld      (BINLEN), de
    ; ld      a, 1<<DF_ARRAY
    ; ld      (DOSFLAGS), a     ; Set 'loading to array' flag
    pop     hl                  ; Pop text pointer
    jr      .start

    ; Load to address
.addr:
    call    FRMNUM              ; Get number
    call    FRCINT              ; Convert to 16 bit integer
    ld      (BINSTART), de
    ; ld      a, 1<<DF_ADDR
    ; ld      (DOSFLAGS), a     ; Load address specified

.start:
    push    hl                  ; Push BASIC text pointer

    ; Close any open files
    call    esp_close_all

    ; Issue ESP command
    ld      a, ESPCMD_OPEN
    call    esp_cmd
    ld      a, FO_RDONLY
    out     (IO_ESPDATA), a
    call    esp_send_pathbuf
    call    esp_get_result


;-----------------------------------------------------------------------------
; Load CAQ/BAS file in PATHBUF
;-----------------------------------------------------------------------------
load_basic_program:
    push    hl

    ; Load file into memory
    call    esp_open
    call    check_sync_bytes
    ld      de, 6
    ld      hl, TMPBUF
    call    esp_read_bytes
    call    check_sync_bytes
    ld      hl, (TXTTAB)
    ld      de, $FFFF
    call    esp_read_bytes
    call    esp_close_all

    ; Back up to last line of BASIC program
.loop:
    dec     hl
    xor     a
    cp      (hl)
    jr      z, .loop
    inc     hl

    ; Forward past 3 zeros = end of BASIC program
    inc     hl
    inc     hl
    inc     hl

    ; Set end of BASIC program
    ld      (VARTAB), hl

    ; Clear variables etc. and update line addresses
    call    init_basic_program

    pop     hl
    ret




    ; ld      de, 1               ; 1 byte to read
    ; ld      hl, FILETYPE
    ; call    esp_read_bytes      ; Read 1st byte from file into FILETYPE

    ; ld      de, 0
    ; call    esp_seek

    ; ld      hl, PATHBUF
    ; call    STROUT

    pop     hl
    ret

;-----------------------------------------------------------------------------
; SAVE
;-----------------------------------------------------------------------------
ST_SAVE:
    ret

;-----------------------------------------------------------------------------
; DEL - Delete file/directory
;-----------------------------------------------------------------------------
ST_DEL:
    ; Get string parameter
    call    get_string_parameter

    ; Save BASIC text pointer
    push    hl

    ; Issue ESP command
    ld      a, ESPCMD_UNLINK
    call    esp_cmd
    call    esp_send_pathbuf
    call    esp_get_result

    ; Restore BASIC text pointer
    pop     hl
    ret

;-----------------------------------------------------------------------------
; CD - Change directory
;
; No argument -> Show current directory
; With argument -> Change current directory
;-----------------------------------------------------------------------------
ST_CD:
    ; Push BASIC text pointer
    push    hl

    ; Argument given?
    or      a
    jr      nz, .change_dir     ; Yes

    ; -- No argument -> show current path ------------------------------------
.show_path:
    ld      a, ESPCMD_GETCWD
    call    esp_cmd
    call    esp_get_result

    ; Print current working directory
.print_cwd:
    call    esp_get_data
    or      a
    jr      z, .print_done
    call    TTYCHR
    jr      .print_cwd
.print_done:
    call    CRDO

.done:
    ; Restore BASIC text pointer
    pop     hl
    ret

    ; -- Argument given -> change directory ----------------------------------
.change_dir:
    ; Restore BASIC text pointer
    pop     hl

    ; Get string parameter
    call    get_string_parameter

    ; Save BASIC text pointer
    push    hl

    ; Issue ESP command
    ld      a, ESPCMD_CHDIR
    call    esp_cmd
    call    esp_send_pathbuf
    call    esp_get_result
    jr      .done

;-----------------------------------------------------------------------------
; DIR - Directory listing
;
; No argument -> List current directory
; With argument -> List given path
;-----------------------------------------------------------------------------
ST_DIR:
    .tmp0: equ FILNAM+1
    .tmp1: equ FILNAM+2
    .tmp2: equ FILNAM+3
    .tmp3: equ FILNAM+4

    ; Preserve BASIC text pointer
    push    hl

    ; Argument given?
    or      a
    jr      nz, .witharg     ; Yes

    xor     a
    ld      (PATHLEN), a
    ld      (PATHBUF), a
    jr      .esp_command

.witharg:
    pop     hl                  ; Pop BASIC text pointer
    call    get_string_parameter
    push    hl

.esp_command:
    call    esp_close_all

    ; Issue ESP command
    ld      a, ESPCMD_OPENDIR
    call    esp_cmd
    call    esp_send_pathbuf
    call    esp_get_result

    ; Set initial number of lines per page
    ld      a, 24
    ld      (CNTOFL), a

.next_entry:
    ; Read entry
    ld      a, ESPCMD_READDIR
    call    esp_cmd
    xor     a
    out     (IO_ESPDATA), a
    call    esp_get_data

    cp      ERR_EOF
    jp      z, .done
    or      a
    jp      p, .ok2
    pop     hl              ; Restore BASIC text pointer
    jp      esp_error

.ok2:
    ;-- Date -----------------------------------------------------------------
    call    esp_get_data
    ld      (.tmp0), a
    call    esp_get_data
    ld      (.tmp1), a

    ; Extract year
    srl     a
    add     80
    call    out_number_2digits

    ld      a, '-'
    call    TTYCHR

    ; Extract month
    ld      a, (.tmp1)
    rra                     ; Lowest bit in carry
    ld      a, (.tmp0)
    rra
    srl     a
    srl     a
    srl     a
    srl     a
    call    out_number_2digits

    ld      a, '-'
    call    TTYCHR

    ; Extract day
    ld      a, (.tmp0)
    and     $1F
    call    out_number_2digits

    ld      a, ' '
    call    TTYCHR

    ;-- Time -----------------------------------------------------------------
    ; Get time (hhhhhmmm mmmsssss)
    call    esp_get_data
    ld      (.tmp0), a
    call    esp_get_data
    ld      (.tmp1), a

    ; Hours
    srl     a
    srl     a
    srl     a
    call    out_number_2digits

    ld      a, ':'
    call    TTYCHR

    ; Minutes
    ld      a, (.tmp1)
    and     $07
    ld      c, a
    ld      a, (.tmp0)
    srl     c
    rra
    srl     c
    rra
    srl     c
    rra
    srl     c
    rra
    srl     c
    rra
    call    out_number_2digits

    ;-- Attributes -----------------------------------------------------------
    call    esp_get_data
    bit     0, a
    jr      z, .no_dir

    ;-- Directory ------------------------------------------------------------
    ld      hl, .str_dir
    call    STROUT

    ; Skip length bytes
    call    esp_get_data
    call    esp_get_data
    call    esp_get_data
    call    esp_get_data

    jr      .get_filename

    ;-- Regular file: file size ----------------------------------------------
.no_dir:
    ; aaaaaaaa bbbbbbbb cccccccc dddddddd

    call    esp_get_data
    ld      (.tmp0), a
    call    esp_get_data
    ld      (.tmp1), a
    call    esp_get_data
    ld      (.tmp2), a
    call    esp_get_data
    ld      (.tmp3), a

    ; Megabytes range?
    or      a
    jr      nz, .mb
    ld      a, (.tmp2)
    and     $F0
    jr      nz, .mb

    ; Kilobytes range?
    ld      a, (.tmp2)
    or      a
    jr      nz, .kb
    ld      a, (.tmp1)
    and     $FC
    jr      nz, .kb

    ; Bytes range (aaaaaaaa bbbbbbbb ccccccCC DDDDDDDD)
.bytes:
    ld      a, (.tmp1)
    ld      h, a
    ld      a, (.tmp0)
    ld      l, a
    call    out_number_4digits
    ld      a, 'B'
    call    TTYCHR
    jr      .get_filename

    ; Kilobytes range: aaaaaaaa bbbbBBBB CCCCCCcc dddddddd
.kb:
    ld      a, (.tmp2)
    and     a, $0F
    ld      h, a
    ld      a, (.tmp1)
    ld      l, a
    srl     h
    rr      l
    srl     h
    rr      l
    call    out_number_4digits
    ld      a, 'K'
    call    TTYCHR
    jr      .get_filename

    ; Megabytes range: AAAAAAAA BBBBbbbb cccccccc dddddddd
.mb:
    ld      a, (.tmp3)
    ld      h, a
    ld      a, (.tmp2)
    ld      l, a
    srl     h
    rr      l
    srl     h
    rr      l
    srl     h
    rr      l
    srl     h
    rr      l
    call    out_number_4digits
    ld      a, 'M'
    call    TTYCHR
    jr      .get_filename

    ;-- Filename -------------------------------------------------------------
.get_filename:
    ld      a, ' '
    call    TTYCHR

.filename:
    call    esp_get_data
    or      a
    jr      z, .name_done
    call    TTYCHR
    jr      .filename

.name_done:
    call    CRDO
    jp      .next_entry

.str_dir: db " <DIR>",0

.done:
    ; Close directory
    ld      a, ESPCMD_CLOSEDIR
    call    esp_cmd
    xor     a
    out     (IO_ESPDATA), a
    call    esp_get_data
    or      a
    jp      m, esp_error

    pop     hl      ; Restore BASIC text pointer
    ret

;-----------------------------------------------------------------------------
; esp_error
;-----------------------------------------------------------------------------
esp_error:
    neg
    dec     a
    cp      -ERR_NO_DISK
    jr      c, .ok
    ld      a, -ERR_OTHER - 1

.ok:
    ld      hl, .error_msgs
    add     a,a
    add     l
    ld      l, a
    ld      a, h
    adc     a, 0
    ld      h, a
    ld      a, (hl)
    inc     hl
    ld      h, (hl)
    ld      l, a

    ; Print error message
    ld      a, '?'
    OUTCHR
    jp      ERRFN1

.error_msgs:
    dw .msg_err_not_found     ; -1: File / directory not found
    dw .msg_err_too_many_open ; -2: Too many open files / directories
    dw .msg_err_param         ; -3: Invalid parameter
    dw .msg_err_eof           ; -4: End of file / directory
    dw .msg_err_exists        ; -5: File already exists
    dw .msg_err_other         ; -6: Other error
    dw .msg_err_no_disk       ; -7: No disk

.msg_err_not_found:     db "Not found",0
.msg_err_too_many_open: db "Too many open",0
.msg_err_param:         db "Invalid param",0
.msg_err_eof:           db "EOF",0
.msg_err_exists:        db "Already exists",0
.msg_err_other:         db "Unknown error",0
.msg_err_no_disk:       db "No disk",0

;-----------------------------------------------------------------------------
; Bad file error
;-----------------------------------------------------------------------------
err_bad_file:
    ld      hl, .msg_bad_file

    ; Print error message
    ld      a, '?'
    OUTCHR
    jp      ERRFN1

.msg_bad_file:       db "Bad file",0


;-----------------------------------------------------------------------------
; Issue command to ESP
;-----------------------------------------------------------------------------
esp_cmd:
    ; Reset FIFOs and issue start of command
    push    a
    ld      a, $83
    out     (IO_ESPCTRL), a
    pop     a

    ; Issue command
    out     (IO_ESPDATA), a
    ret

;-----------------------------------------------------------------------------
; Wait for data from ESP
;-----------------------------------------------------------------------------
esp_get_data:
.wait:
    in      a, (IO_ESPCTRL)
    and     a, 1
    jr      z, .wait
    in      a, (IO_ESPDATA)
    ret

;-----------------------------------------------------------------------------
; Output 2 number digit in A
;-----------------------------------------------------------------------------
out_number_2digits:
    cp      100
    jr      c, .l0
    sub     a, 100
    jr      out_number_2digits
.l0:
    ld      c, 0
.l1:
    inc     c
    sub     a, 10
    jr      nc, .l1
    add     a, 10
    push    a

    ld      a, c
    add     '0'-1
    call    TTYCHR
    pop     a
    add     '0'
    call    TTYCHR
    ret

;-----------------------------------------------------------------------------
; Output 4 number digit in HL
;-----------------------------------------------------------------------------
out_number_4digits:
    ld      d, 1

    ld      bc, -10000
    call    .num1
    ld      bc, -1000
    call    .num1
    ld      bc, -100
    call    .num1
    ld      c, -10
    call    .num1

    ld      d, 0
    ld      c, -1
.num1:
    ld      a, -1
.num2:
    inc     a
    add     hl, bc
    jr      c, .num2
    sbc     hl, bc

    or      a
    jr      z, .zero

    ld      d, 0

.normal:
    add     '0'
    call    TTYCHR
    ret

.zero:
    bit     0, d
    jr      z, .normal
    ld      a, ' '
    call    TTYCHR
    ret

;-----------------------------------------------------------------------------
; Initialize BASIC Program
;
; Resets variables, arrays, string space etc.
; Updates nextline pointers to match location of BASIC program in RAM
;-----------------------------------------------------------------------------
init_basic_program:
    ; Set next statement to start of program
    ld      hl, (TXTTAB)
    dec     hl
    ld      (SAVTXT), hl

    ; Set DATPTR to start of program
    ld      (DATPTR), hl

    ; Clear string space
    ld      hl, (MEMSIZ)
    ld      (FRETOP), hl

    ; Clear simple variables
    ld      hl, (VARTAB)
    ld      (ARYTAB), hl

    ; Clear array table
    ld      (STREND), hl

    ; Clear string buffer
    ld      hl, TEMPPT + 2
    ld      (TEMPPT), hl

    ; Set CONTinue position to 0
    xor     a
    ld      l, a
    ld      h, a
    ld      (OLDTXT), hl

    ; Clear locator flag
    ld      (SUBFLG), a

    ; Clear array pointer???
    ld      (VARNAM), hl

    ; Fix up next line addresses in loaded BASIC program
.link_lines:
    ld      de, (TXTTAB)        ; DE = start of BASIC program
.next_line:
    ld      h, d
    ld      l, e                ; HL = DE
    ld      a, (hl)
    inc     hl                  ; Test nextline address
    or      (hl)
    jr      z, .init_done       ; If $0000 then done
    inc     hl
    inc     hl                  ; Skip line number
    inc     hl
    xor     a                   ; End of line = $00
.find_eol:
    cp      (hl)                ; Search for end of line
    inc     hl
    jr      nz, .find_eol
    ex      de, hl              ; HL = current line, DE = next line
    ld      (hl), e
    inc     hl                  ; Set address of next line
    ld      (hl), d
    jr      .next_line
.init_done:
    ret

;-----------------------------------------------------------------------------
; Get string parameter and store it in PATHLEN/PATHBUF
;-----------------------------------------------------------------------------
get_string_parameter:
    ; Evaluate expression
    call    FRMEVL

    ; Save BASIC text pointer
    push    hl

    ; Get string length (this will check for string type)
    call    LEN1

    ; Save length
    ld      (PATHLEN), a

    ; Get string pointer into HL
    inc     hl
    inc     hl
    ld      b, (hl)
    inc     hl
    ld      h, (hl)
    ld      l, b

    ; Copy string to PATHBUF
    ld      de, PATHBUF
    ld      b, 0
    ld      c, a
    or      a
    jr      z, .copy_done
    ldir
.copy_done:

    ; Zero-terminate string
    xor     a
    ld      (de), a

    ; Restore BASIC text pointer
    pop     hl
    ret

;-----------------------------------------------------------------------------
; Send PATHBUF including zero termination to ESPDATA
;-----------------------------------------------------------------------------
esp_send_pathbuf:
    ld      a, (PATHLEN)
    inc     a               ; Include zero termination
    ld      b, a
    ld      hl, PATHBUF
    ld      c, IO_ESPDATA
    otir
    ret

;-----------------------------------------------------------------------------
; Get first result byte, and jump to error handler if it was an error
;-----------------------------------------------------------------------------
esp_get_result:
    call    esp_get_data
    or      a
    jp      m, esp_error
    ret

;-----------------------------------------------------------------------------
; Close any open file/directory descriptor
;-----------------------------------------------------------------------------
esp_close_all:
    ld      a, ESPCMD_CLOSEALL
    call    esp_cmd
    jp      esp_get_result

;-----------------------------------------------------------------------------
; Open file in PATHBUF
;-----------------------------------------------------------------------------
esp_open:
    ld      a, ESPCMD_OPEN
    call    esp_cmd
    ld      a, FO_RDONLY
    out     (IO_ESPDATA), a
    call    esp_send_pathbuf
    jp      esp_get_result

;-----------------------------------------------------------------------------
; Read bytes
; Input:  HL: destination address
;         DE: number of bytes to read
; Output: HL: next address (start address if no bytes read)
;         DE: number of bytes actually read
;-----------------------------------------------------------------------------
esp_read_bytes:
    ld      a, ESPCMD_READ
    call    esp_cmd

    ; Send file descriptor
    xor     a
    out     (IO_ESPDATA), a

    ; Send read size
    ld      a, e
    out     (IO_ESPDATA), a
    ld      a, d
    out     (IO_ESPDATA), a

    ; Get result
    call    esp_get_result

    ; Get number of bytes actual read
    call    esp_get_data
    ld      e, a
    call    esp_get_data
    ld      d, a

    push    de

.loop:
    ; Done reading? (DE=0)
    ld      a, d
    or      a, e
    jr      z, .done

    call    esp_get_data
    ld      (hl), a
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de
    ret

;-----------------------------------------------------------------------------
; Seek
; Input:  DE: offset
;-----------------------------------------------------------------------------
esp_seek:
    ld      a, ESPCMD_SEEK
    call    esp_cmd

    ; Send file descriptor
    xor     a
    out     (IO_ESPDATA), a

    ; Send offset
    ld      a, e
    out     (IO_ESPDATA), a
    ld      a, d
    out     (IO_ESPDATA), a
    xor     a
    out     (IO_ESPDATA), a
    out     (IO_ESPDATA), a

    ; Get result
    call    esp_get_result
    ret

;-----------------------------------------------------------------------------
; Get filetype based on extension of file in PATHBUF
;-----------------------------------------------------------------------------
path_get_filetype:
    ld      a, (PATHLEN)
    cp      a, 5
    jr      nc, .ok
.unknown:
    xor     a
    ret

.ok:
    ; Pointer to extension
    sub     a, 3
    ld      h, PATHBUF >> 8
    ld      l, a

    ; Check for dot
    ld      a, (hl)
    cp      '.'
    jr      nz, .unknown
    inc     hl

    ; Search for extension in list
    ld      de, .known_ext
    jr      .search
.skip:
    or      a
    jr      z, .next
    ld      a, (de)
    inc     de
    jr      .skip
.next:
    inc     de
.search:
    ld      a, (de)
    or      a
    jr      z, .unknown     ; End of extensions list
    push    hl
    ex      de, hl
    call    .cmp
    ex      de, hl
    pop     hl
    jr      nz, .skip       ; No match, check next entry

    ; Get file type from entry
    ld      a, (de)
    ret

.cmp:
    ld      a, (de)         ; Get char from string 2
    call    to_upper
    inc     de
    cp      (hl)            ; Compare to char in string 1
    inc     hl
    ret     nz              ; Return NZ if not equal
    or      a
    jr      nz, .cmp        ; Loop until end of strings
    ret                     ; Return Z

.known_ext:
    db "BAS", 0, FILETYPE_BAS
    db "CAQ", 0, FILETYPE_CAQ
    db 0

;-----------------------------------------------------------------------------
; Get next character, skipping spaces
;  in: HL = text pointer
; out: NZ, A = next non-space char, HL = address of char in text
;      Z,  A = 0, HL = end of text
;-----------------------------------------------------------------------------
get_next:                   ; Starting at next location
    inc     hl
get_arg:                    ; Starting at current location
    ld      a, (hl)
    or      a
    ret     z               ; Return Z if NULL
    cp      ' '
    ret     nz              ; Return NZ if not SPACE
    jr      get_next

;-----------------------------------------------------------------------------
; Check for sync sequence (12x$FF, 1x$00)
;-----------------------------------------------------------------------------
check_sync_bytes:
    ; Read 13 bytes into TMPBUF
    ld      de, 13
    ld      hl, TMPBUF
    call    esp_read_bytes
    ld      a, e
    cp      13
    jp      nz, err_bad_file

    ; Check for 12x$FF
    ld      c, 12
    ld      hl, TMPBUF
.loop:
    ld      a, (hl)
    cp      $FF
    jp      nz, err_bad_file
    inc     hl
    dec     c
    jr      nz, .loop

    ; Check for $00
    ld      a, (hl)
    or      a
    jp      nz, err_bad_file
    ret
