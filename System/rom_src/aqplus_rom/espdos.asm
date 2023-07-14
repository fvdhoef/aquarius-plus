;-----------------------------------------------------------------------------
; espdos.asm
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; LOAD
;
; LOAD "filename"        Load BASIC program
; LOAD "filename",12345  Load file as raw binary to address 12345
; LOAD "filename",*a     Load data into numeric array a
;-----------------------------------------------------------------------------
ST_LOAD:
    ; Close any open files
    call    esp_close_all

    ; Get string parameter with path
    call    get_string_parameter

    ; Check for second parameter
    call    get_arg
    cp      ','
    jp      nz, load_basic_program  ; No parameter -> load as basic program
    call    get_next
    cp      $AA                     ; Token for '*'
    jr      z, .array               ; Array parameter -> load as array

    ; Load as binary to address
    call    FRMNUM                  ; Get number
    call    FRCINT                  ; Convert to 16 bit integer
    ld      (BINSTART), de
    jp      load_binary

    ; Load into array
.array:
    call    get_array_argument
    jp      load_caq_array

;-----------------------------------------------------------------------------
; Get array argument
;-----------------------------------------------------------------------------
get_array_argument:
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
    pop     hl                  ; Pop text pointer

    ret

;-----------------------------------------------------------------------------
; SAVE
;
; SAVE "filename"             Save BASIC program
; SAVE "filename",addr,len    Save binary data
; SAVE "filename",*a          Save numeric array a
;-----------------------------------------------------------------------------
ST_SAVE:
    ; Close any open files
    call    esp_close_all

    ; Get string parameter with path
    call    get_string_parameter

    ; Check for second parameter
    call    get_arg
    cp      ','
    jp      nz, save_basic_program
    call    get_next
    cp      $AA                     ; Token for '*'
    jr      z, .array               ; Array parameter -> save array

    ; Save binary data
    
    ; Get first parameter: address
    call    FRMNUM                  ; Get number
    call    FRCINT                  ; Convert to 16 bit integer
    ld      (BINSTART), de

    ; Expect comma
    call    get_arg
    cp      ','
    jp      nz, MOERR
    inc     hl

    ; Get second parameter: length
    call    FRMNUM                  ; Get number
    call    FRCINT                  ; Convert to 16 bit integer
    ld      (BINLEN), de
    jp      save_binary

    ; Save array
.array:
    call    get_array_argument
    jp      save_caq_array

;-----------------------------------------------------------------------------
; DEL - Delete file/directory
;-----------------------------------------------------------------------------
ST_DEL:
    ; Get string parameter
    call    get_string_parameter

    ; Save BASIC text pointer
    push    hl

    ; Issue ESP command
    ld      a, ESPCMD_DELETE
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
    call    esp_get_byte
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
; MKDIR - Create directory
;-----------------------------------------------------------------------------
ST_MKDIR:
    ; Get string parameter
    call    get_string_parameter

    ; Save BASIC text pointer
    push    hl

    ; Issue ESP command
    ld      a, ESPCMD_MKDIR
    call    esp_cmd
    call    esp_send_pathbuf
    call    esp_get_result

    ; Restore BASIC text pointer
    pop     hl
    ret

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
    call    esp_send_byte
    call    esp_get_byte

    cp      ERR_EOF
    jp      z, .done
    or      a
    jp      p, .ok2
    pop     hl              ; Restore BASIC text pointer
    jp      esp_error

.ok2:
    ;-- Date -----------------------------------------------------------------
    call    esp_get_byte
    ld      (.tmp0), a
    call    esp_get_byte
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
    call    esp_get_byte
    ld      (.tmp0), a
    call    esp_get_byte
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
    call    esp_get_byte
    bit     0, a
    jr      z, .no_dir

    ;-- Directory ------------------------------------------------------------
    ld      hl, .str_dir
    call    STROUT

    ; Skip length bytes
    call    esp_get_byte
    call    esp_get_byte
    call    esp_get_byte
    call    esp_get_byte

    jr      .get_filename

    ;-- Regular file: file size ----------------------------------------------
.no_dir:
    ; aaaaaaaa bbbbbbbb cccccccc dddddddd

    call    esp_get_byte
    ld      (.tmp0), a
    call    esp_get_byte
    ld      (.tmp1), a
    call    esp_get_byte
    ld      (.tmp2), a
    call    esp_get_byte
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
    call    esp_get_byte
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
    call    esp_send_byte
    call    esp_get_byte
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
    cp      -ERR_NOT_EMPTY
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
    dw .msg_err_not_empty     ; -8: Not empty

.msg_err_not_found:     db "Not found",0
.msg_err_too_many_open: db "Too many open",0
.msg_err_param:         db "Invalid param",0
.msg_err_eof:           db "EOF",0
.msg_err_exists:        db "Already exists",0
.msg_err_other:         db "Unknown error",0
.msg_err_no_disk:       db "No disk",0
.msg_err_not_empty:     db "Not empty",0

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
    ld      hl, PATHBUF
    ld      d, 0
    ld      a, (PATHLEN)
    inc     a               ; Include zero termination
    ld      e, a
    jp      esp_send_bytes

;-----------------------------------------------------------------------------
; Get first result byte, and jump to error handler if it was an error
;-----------------------------------------------------------------------------
esp_get_result:
    call    esp_get_byte
    or      a
    jp      m, esp_error
    ret

;-----------------------------------------------------------------------------
; Close any open file/directory descriptor
;
; Clobbered registers: A
;-----------------------------------------------------------------------------
esp_close_all:
    ld      a, ESPCMD_CLOSEALL
    call    esp_cmd
    jp      esp_get_result

;-----------------------------------------------------------------------------
; Open file in PATHBUF
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
esp_open:
    ld      a, ESPCMD_OPEN
    call    esp_cmd
    ld      a, FO_RDONLY
    call    esp_send_byte
    call    esp_send_pathbuf
    jp      esp_get_result

;-----------------------------------------------------------------------------
; Create file in PATHBUF
;-----------------------------------------------------------------------------
esp_create:
    ld      a, ESPCMD_OPEN
    call    esp_cmd
    ld      a, FO_WRONLY | FO_CREATE | FO_TRUNC
    call    esp_send_byte
    call    esp_send_pathbuf
    jp      esp_get_result

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
    call    esp_get_result

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
    ret

;-----------------------------------------------------------------------------
; Send bytes
; Input:  HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;-----------------------------------------------------------------------------
esp_send_bytes:
    push    de

.loop:
    ; Done sending? (DE=0)
    ld      a, d
    or      a, e
    jr      z, .done

    ld      a, (hl)
    call    esp_send_byte
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de
    ret

;-----------------------------------------------------------------------------
; Write bytes
; Input:  HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
esp_write_bytes:
    ld      a, ESPCMD_WRITE
    call    esp_cmd

    ; Send file descriptor
    xor     a
    call    esp_send_byte

    ; Send write size
    ld      a, e
    call    esp_send_byte
    ld      a, d
    call    esp_send_byte

    ; Send bytes
    call    esp_send_bytes

    ; Get result
    call    esp_get_result

    ; Get number of bytes actual written
    call    esp_get_byte
    ld      e, a
    call    esp_get_byte
    ld      d, a

    ret

;-----------------------------------------------------------------------------
; Seek
; Input:  DE: offset
;
; Clobbered registers: A, DE
;-----------------------------------------------------------------------------
esp_seek:
    ld      a, ESPCMD_SEEK
    call    esp_cmd

    ; Send file descriptor
    xor     a
    call    esp_send_byte

    ; Send offset
    ld      a, e
    call    esp_send_byte
    ld      a, d
    call    esp_send_byte
    xor     a
    call    esp_send_byte
    call    esp_send_byte

    ; Get result
    call    esp_get_result
    ret

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

;-----------------------------------------------------------------------------
; Load binary data in PATHBUF into BINSTART;
;
; Clobbered registers: A, DE
;-----------------------------------------------------------------------------
load_binary:
    push    hl

    ; Load file into memory
    call    esp_open
    ld      hl, (BINSTART)
    ld      de, $FFFF
    call    esp_read_bytes
    call    esp_close_all

    pop     hl
    ret

;-----------------------------------------------------------------------------
; Load CAQ array file in PATHBUF into BINSTART (BINLEN length)
;-----------------------------------------------------------------------------
load_caq_array:
    push    hl

    ; Open file
    call    esp_open

    ; Check CAQ header
    call    check_sync_bytes    ; Sync bytes
    ld      de, 6               ; Check that filename is '######'
    ld      hl, TMPBUF
    call    esp_read_bytes
    ld      c, 6
    ld      hl, TMPBUF
.loop:
    ld      a, (hl)
    cp      '#'
    jp      nz, err_bad_file
    inc     hl
    dec     c
    jr      nz, .loop

    ; Load data into array
    ld      hl, (BINSTART)
    ld      de, (BINLEN)
    call    esp_read_bytes

    ; Close file
    call    esp_close_all

    pop     hl
    ret

;-----------------------------------------------------------------------------
; Load CAQ/BAS file in PATHBUF
;-----------------------------------------------------------------------------
load_basic_program:
    push    hl

    ; Open file
    call    esp_open            

    ; Check CAQ header
    call    check_sync_bytes    ; Sync bytes
    ld      de, 6               ; Skip filename
    ld      hl, TMPBUF
    call    esp_read_bytes
    call    check_sync_bytes    ; Sync bytes
    
    ; Load actual program
    ld      hl, (TXTTAB)
    ld      de, $FFFF
    call    esp_read_bytes

    ; Close file
    call    esp_close_all

    ; Back up to last line of BASIC program
.loop:
    dec     hl
    xor     a
    cp      (hl)
    jr      z, .loop
    inc     hl

    ; Skip past 3 zeros at end of BASIC program
    inc     hl
    inc     hl
    inc     hl

    ; Set end of BASIC program
    ld      (VARTAB), hl

    ; Initialize BASIC program
    call    init_basic_program

    pop     hl
    ret

sync_bytes:
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00

;-----------------------------------------------------------------------------
; Save basic program
;-----------------------------------------------------------------------------
save_basic_program:
    push    hl

    ; Create file
    call    esp_create

    ; Write CAQ header
    ld      hl, sync_bytes      ; Sync bytes
    ld      de, 13
    call    esp_write_bytes
    ld      hl, .caq_filename   ; Filename
    ld      de, 6
    call    esp_write_bytes
    ld      hl, sync_bytes      ; Sync bytes
    ld      de, 13
    call    esp_write_bytes

    ; Write BASIC data
    ld      de, (TXTTAB)            ; DE = start of BASIC program
    ld      hl, (VARTAB)            ; HL = end of BASIC program
    sbc     hl, de
    ex      de, hl                  ; HL = start, DE = length of BASIC program
    call    esp_write_bytes

    ; Close file
    call    esp_close_all

    pop     hl
    ret

.caq_filename: db "BASPRG"

;-----------------------------------------------------------------------------
; Save array
;-----------------------------------------------------------------------------
save_caq_array:
    push    hl

    ; Create file
    call    esp_create

    ; Write CAQ header
    ld      hl, sync_bytes      ; Sync bytes
    ld      de, 13
    call    esp_write_bytes
    ld      hl, .array_filename ; Filename
    ld      de, 6
    call    esp_write_bytes

    ; Write array data
    ld      hl, (BINSTART)
    ld      de, (BINLEN)
    call    esp_write_bytes

    ; Close file
    call    esp_close_all

    pop     hl
    ret

.array_filename: db "######"

;-----------------------------------------------------------------------------
; Save binary
;-----------------------------------------------------------------------------
save_binary:
    push    hl

    ; Create file
    call    esp_create

    ; Write binary data
    ld      hl, (BINSTART)
    ld      de, (BINLEN)
    call    esp_write_bytes

    ; Close file
    call    esp_close_all

    pop     hl
    ret

;-----------------------------------------------------------------------------
; Run file
;-----------------------------------------------------------------------------
run_file:
    ; Close any open files
    call    esp_close_all

    ; Get string parameter with path
    call    get_string_parameter

    ; Check for .ROM extension
    ld      a, (PATHLEN)
    cp      a, 5
    jr      c, .load_basic      ; Too short to have ROM extension
    sub     a, 3
    ld      d, PATHBUF >> 8
    ld      e, a
    ld      hl, .romext
    call    .cmp
    jr      z, load_rom

.load_basic:
    call    load_basic_program
    jp      RUNC

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

.romext: db ".ROM",0

;-----------------------------------------------------------------------------
; Load ROM file
;-----------------------------------------------------------------------------
load_rom:
    ; Open file
    call    esp_open

    ; Map RAM in bank3
    ld      a, 35
    out     (IO_BANK3), a

    ; Load file
    ld      hl, $C000
    ld      de, $4000
    call    esp_read_bytes

    ; Check length
    ld      a, d
    cp      $20         ; 8KB ROM?
    jr      nz, .ok

    ; 8KB ROM, duplicate data to second 8KB area
    ld      hl, $C000
    ld      de, $E000
    ld      bc, $2000
    ldir
.ok:
    call    esp_close_all

descramble_rom:
    ; Determine scramble value
    xor     a
    ld      hl, $E003
    ld      b, 12
.loop:
    add     a, (hl)
    inc     hl
    add     a, b
    dec     b
    jr      nz, .loop
    xor     (hl)
    ld      b, a

    ; Descramble ROM
    ld      hl, $C000
    ld      de, $4000
.loop2:
    ld      a, b
    xor     (hl)
    ld      (hl), a

    inc     hl
    dec     de
    ld      a, d
    or      e
    jr      nz, .loop2

    ; Reinit banks
    ld      a, 33
    out     (IO_BANK1), a
    ld      a, 34
    out     (IO_BANK2), a

    ; Bank3 -> readonly
    ld      a, 35 | BANK_READONLY
    out     (IO_BANK3), a

    ; Reinit stack pointer
    ld      sp, $38A0

    ; Start ROM
    jp      $E010
