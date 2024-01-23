
;-----------------------------------------------------------------------------
; esp_open (path in hl)
;-----------------------------------------------------------------------------
esp_open:
    ld      a,ESPCMD_OPEN
    call    esp_cmd
    ld      a,FO_RDONLY
    call    esp_send_byte
    call    esp_send_string
    call    esp_get_byte
    or      a
    ret

;-----------------------------------------------------------------------------
; esp_close
;-----------------------------------------------------------------------------
esp_close:
    ld      a,ESPCMD_CLOSE
    call    esp_cmd
    xor     a
    call    esp_send_byte
    call    esp_get_byte
    ret

;-----------------------------------------------------------------------------
; Issue command to ESP
;-----------------------------------------------------------------------------
esp_cmd:
    push    a

    ; Drain RX FIFO
.drain:
    in      a,(IO_ESPCTRL)
    and     a,1
    jr      z,.done
    in      a,(IO_ESPDATA)
    jr      .drain
.done:

    ; Issue start of command
    ld      a,$80
    out     (IO_ESPCTRL),a

    ; Issue command
    pop     a
    jp      esp_send_byte

;-----------------------------------------------------------------------------
; Wait for data from ESP
;-----------------------------------------------------------------------------
esp_get_byte:
.wait:
    in      a,(IO_ESPCTRL)
    and     a,1
    jr      z,.wait
    in      a,(IO_ESPDATA)
    ret

;-----------------------------------------------------------------------------
; Write data to ESP
;-----------------------------------------------------------------------------
esp_send_byte:
    push    a

.wait:
    in      a,(IO_ESPCTRL)
    and     a,2
    jr      nz,.wait

    pop     a
    out     (IO_ESPDATA),a
    ret

;-----------------------------------------------------------------------------
; Get bytes
; Input:  HL: destination address
;         DE: number of bytes to read
;-----------------------------------------------------------------------------
esp_get_bytes:
.loop:
    ; Done reading? (DE=0)
    ld      a,d
    or      a,e
    ret     z

    call    esp_get_byte
    ld      (hl),a
    inc     hl
    dec     de
    jr      .loop

;-----------------------------------------------------------------------------
; Send string
; Input:  HL: address of zero-terminated string
;-----------------------------------------------------------------------------
esp_send_string:
.1: ld      a,(hl)
    inc     hl
    call    esp_send_byte
    or      a
    jr      nz,.1
    ret

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
    ld      a,ESPCMD_READ
    call    esp_cmd

    ; Send file descriptor
    xor     a
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
