;-----------------------------------------------------------------------------
; Open file in HL
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
esp_open:
    ld      a, ESPCMD_OPEN
    call    esp_cmd
    ld      a, FO_RDONLY
    call    esp_send_byte
    call    strlen
    inc     de
    call    esp_send_bytes
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
; Close any open file/directory descriptor
;
; Clobbered registers: A
;-----------------------------------------------------------------------------
esp_close_all:
    ld      a, ESPCMD_CLOSEALL
    call    esp_cmd
    jp      esp_get_result

;-----------------------------------------------------------------------------
; Issue command to ESP
;
; Clobbered registers: none
;-----------------------------------------------------------------------------
esp_cmd:
    ; Reset FIFOs and issue start of command
    push    a
    ld      a, $83
    out     (IO_ESPCTRL), a
    pop     a

    ; Issue command
    jp      esp_send_byte

;-----------------------------------------------------------------------------
; Write data to ESP
;
; Clobbered registers: none
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
; Get first result byte, and jump to error handler if it was an error
;-----------------------------------------------------------------------------
esp_get_result:
    call    esp_get_byte
    or      a
    jp      m, esp_error
    ret

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
; Get length of string
; Input:  HL: string
; Output: DE: length
;-----------------------------------------------------------------------------
strlen:
    push    hl

    xor     a, a
	ld      b, a
	ld      c, a
	cpir
    ld      hl, -1
    sbc     hl, bc
    ex      de, hl

    pop     hl
    ret

;-----------------------------------------------------------------------------
; esp_error
;-----------------------------------------------------------------------------
esp_error:
    rst     0
