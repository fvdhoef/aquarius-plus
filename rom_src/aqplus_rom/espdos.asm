;-----------------------------------------------------------------------------
; espdos.asm
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; DEL - Delete file/directory
;-----------------------------------------------------------------------------
ST_DEL:
    call    FRMEVL              ; Evaluate expression
    push    hl                  ; Push BASIC text pointer

    ; Get string argument (HL = string text, A = string length)
    call    CHKSTR              ; Type mismatch error if not string
    call    LEN1                ; Get string and its length
    inc     hl
    inc     hl                  ; Skip to string text pointer
    ld      b, (hl)
    inc     hl
    ld      h, (hl)             ; HL = string text
    ld      l, b

    ; Issue ESP command
    push    a
    ld      a, ESP_UNLINK
    call    esp_cmd
    pop     a
    or      a
    jr      z, .zeroterm        ; Empty string
    ld      b, a
    ld      c, IO_ESPDATA
    otir
.zeroterm:
    xor     a
    out     (IO_ESPDATA), a

    ; Get result
    call    esp_get_data
    or      a
    jp      p, .done
    pop     hl              ; Restore BASIC text pointer
    jp      esp_error
 
.done:
    pop     hl                  ; Restore BASIC text pointer
    ret

;-----------------------------------------------------------------------------
; CD - Change directory
;
; No argument -> Show current directory
; With argument -> Change current directory
;-----------------------------------------------------------------------------
ST_CD:
    push    hl                  ; Push BASIC text pointer

    ; Argument given?
    or      a
    jr      nz, .change_dir     ; Yes

    ; -- No argument -> show current path ------------------------------------
.show_path:
    ld      a, ESP_GETCWD
    call    esp_cmd

    ; Get result
    call    esp_get_data
    or      a
    jp      p, .print_cwd
    pop     hl                  ; Restore BASIC text pointer
    jp      esp_error
.print_cwd:
    call    esp_get_data
    or      a
    jr      z, .print_done
    call    TTYCHR
    jr      .print_cwd
.print_done:
    call    CRDO

.done:
    pop     hl                  ; Restore BASIC text pointer
    ret

    ; -- Argument given -> change directory ----------------------------------
.change_dir:
    pop     hl                  ; Pop BASIC text pointer
    call    FRMEVL              ; Evaluate expression
    push    hl                  ; Push BASIC text pointer

    ; Get string argument (HL = string text, A = string length)
    call    CHKSTR              ; Type mismatch error if not string
    call    LEN1                ; Get string and its length
    inc     hl
    inc     hl                  ; Skip to string text pointer
    ld      b, (hl)
    inc     hl
    ld      h, (hl)             ; HL = string text
    ld      l, b

    ; Issue ESP command
    push    a
    ld      a, ESP_CHDIR
    call    esp_cmd
    pop     a
    or      a
    jr      z, .zeroterm        ; Empty string
    ld      b, a
    ld      c, IO_ESPDATA
    otir
.zeroterm:
    xor     a
    out     (IO_ESPDATA), a

    ; Get result
    call    esp_get_data
    or      a
    jp      p, .done
    pop     hl              ; Restore BASIC text pointer
    jp      esp_error

;-----------------------------------------------------------------------------
; DIR - Directory listing
;
; No argument -> List current directory
; With argument -> List given path
;-----------------------------------------------------------------------------
ST_DIR:
    .dd:   equ FILNAM+0
    .tmp0: equ FILNAM+1
    .tmp1: equ FILNAM+2
    .tmp2: equ FILNAM+3
    .tmp3: equ FILNAM+4

    ; Preserve BASIC text pointer
    push    hl

    ; Argument given?
    or      a
    jr      nz, .witharg     ; Yes

    ; Issue ESP command
    ld      a, ESP_OPENDIR
    call    esp_cmd
    jr      .zeroterm

.witharg:
    pop     hl                  ; Pop BASIC text pointer
    call    FRMEVL              ; Evaluate expression
    push    hl                  ; Push BASIC text pointer

    ; Get string argument (HL = string text, A = string length)
    call    CHKSTR              ; Type mismatch error if not string
    call    LEN1                ; Get string and its length
    inc     hl
    inc     hl                  ; Skip to string text pointer
    ld      b, (hl)
    inc     hl
    ld      h, (hl)             ; HL = string text
    ld      l, b

    ; Issue ESP command
    push    a
    ld      a, ESP_OPENDIR
    call    esp_cmd
    pop     a
    or      a
    jr      z, .zeroterm        ; Empty string
    ld      b, a
    ld      c, IO_ESPDATA
    otir
.zeroterm:
    xor     a
    out     (IO_ESPDATA), a
    call    esp_get_data
    or      a
    jp      p, .ok
    pop     hl              ; Restore BASIC text pointer
    jp      esp_error
.ok:
    ld      (.dd), a        ; Keep track of directory descriptor

    ld      a, 22
    ld      (CNTOFL), a     ; Set initial number of lines per page

.next_entry:
    ; Read entry
    ld      a, ESP_READDIR
    call    esp_cmd
    ld      a, (.dd)
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
    ld      a, ESP_CLOSEDIR
    call    esp_cmd
    ld      a, (.dd)
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
    push    hl
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
    call    STROUT

    pop     hl
    jp      CRDO

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
; Issue command to ESP
;-----------------------------------------------------------------------------
esp_cmd:
    ; Start of command
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
