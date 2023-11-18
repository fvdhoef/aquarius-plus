;-----------------------------------------------------------------------------
; aqds.asm
;-----------------------------------------------------------------------------
    include "regs.inc"

PAGE_LAUNCHSTATE    equ 40
PGM_PATH            equ $FE80
PGM_ARG             equ $FF00

;-----------------------------------------------------------------------------
; Header and BASIC stub
;-----------------------------------------------------------------------------
    org     $38E1
    defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    "AQPLUS"
    defb    $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    defb    $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp      _entry

;-----------------------------------------------------------------------------
; _entry
;-----------------------------------------------------------------------------
_entry:
    ; Disable interrupts
    di

    ; Save registers
    push    af
    push    bc
    push    de
    push    hl

    ; Save stack pointer
    ld      (_stackp),sp

    ; Save IO register state
    in      a,(IO_VCTRL)
    ld      (_vctrl),a
    in      a,(IO_BANK0)
    ld      (_bank0),a
    in      a,(IO_BANK1)
    ld      (_bank1),a
    in      a,(IO_BANK2)
    ld      (_bank2),a
    in      a,(IO_BANK3)
    ld      (_bank3),a
    in      a,(IO_SYSCTRL)
    ld      (_sysctrl),a
    in      a,(IO_IRQMASK)
    ld      (_irqmask),a

    ; Enable turbo mode
    ld      a,$4
    out     (IO_SYSCTRL),a

    ; Save important memory state
    ld      a,PAGE_LAUNCHSTATE
    out     (IO_BANK3),a
    ld      de,$C000

    ; Save overlay RAM
    ld      hl,$3800
    ld      bc,$800
    ldir

    ; Save text RAM
    ld      a,$60       ; 80-columns mode, text page
    out     (IO_VCTRL),a
    ld      hl,$3000
    ld      bc,$800
    ldir
    ld      a,$E0       ; 80-columns mode, color page
    out     (IO_VCTRL),a
    ld      hl,$3000
    ld      bc,$800
    ldir

    ; Save character RAM
    ld      a,21
    out     (IO_BANK2)
    ld      hl,$3000
    ld      bc,$800
    ldir
    ld      a,(_bank2)
    out     (IO_BANK2)

    ; Save palette
    ex      de,hl
    ld      a,0
    ld      c,IO_VPALDATA
.1: out     (IO_VPALSEL),a
    in      b,(c)
    ld      (hl),b
    inc     hl
    inc     a
    cp      64
    jr      nz,.1

    ; Load default character set
    ld      a,0
    out     (IO_BANK2)
    ld      a,21
    out     (IO_BANK3)
    ld      hl,$B000
    ld      de,$C000
    ld      bc,$800
    ldir
    ld      a,(_bank2)
    out     (IO_BANK2),a

    jp      _main


    org     $4000
;-----------------------------------------------------------------------------
; Pre-launch state
;-----------------------------------------------------------------------------
_stackp:    defw    0
_vctrl:     defb    0
_bank0:     defb    0
_bank1:     defb    0
_bank2:     defb    0
_bank3:     defb    0
_sysctrl:   defb    0
_irqmask:   defb    0

;-----------------------------------------------------------------------------
; ret_basic
;-----------------------------------------------------------------------------
ret_basic:
    ; Restore RAM state
    ld      a,PAGE_LAUNCHSTATE
    out     (IO_BANK3),a
    ld      hl,$C000

    ; Save overlay RAM
    ld      de,$3800
    ld      bc,$800
    ldir

    ; Restore text RAM
    ld      a,$60       ; 80-columns mode, text page
    out     (IO_VCTRL),a
    ld      de,$3000
    ld      bc,$800
    ldir
    ld      a,$E0       ; 80-columns mode, color page
    out     (IO_VCTRL),a
    ld      de,$3000
    ld      bc,$800
    ldir

    ; Restore character RAM
    ld      a,21
    out     (IO_BANK2)
    ld      de,$3000
    ld      bc,$800
    ldir

    ; Restore palette
    ld      a,0
    ld      c,IO_VPALDATA
.1: out     (IO_VPALSEL),a
    ld      b,(hl)
    out     (c),b
    inc     hl
    inc     a
    cp      64
    jr      nz,.1

    ; Restore IO register state
    ld      a,(_vctrl)
    out     (IO_VCTRL),a
    ld      a,(_bank0)
    out     (IO_BANK0),a
    ld      a,(_bank1)
    out     (IO_BANK1),a
    ld      a,(_bank2)
    out     (IO_BANK2),a
    ld      a,(_bank3)
    out     (IO_BANK3),a
    ld      a,(_sysctrl)
    out     (IO_SYSCTRL),a
    ld      a,(_irqmask)
    out     (IO_IRQMASK),a
  
    ; Restore stack pointer
    ld      sp,(_stackp)

    ; Restore registers
    pop     hl
    pop     de
    pop     bc
    pop     af
    ret

;-----------------------------------------------------------------------------
; _main
;-----------------------------------------------------------------------------
_main:
    ; Map memory in bank3 with overlay activated
    ld      a,$40 | 44
    out     (IO_BANK3),a

    ; Clear RAM from $F800-$FFFF
    xor     a
    ld      hl,$F800
    ld      (hl),a
    ld      de,$F801
    ld      bc,$7FF
    ldir

    ; Load OS
    ld      hl,_os_start
    ld      de,$F800
    ld      bc,_os_end - _os_start
    ldir

    ; Jump to OS entry
    jp      _os_entry

;-----------------------------------------------------------------------------
; Start of OS
;-----------------------------------------------------------------------------
_os_start:
    phase   $F800

    ; Jump table
    jp      _go_basic       ; $F800
    jp      _run_program    ; $F803
    jp      _go_fileman     ; $F806

;-----------------------------------------------------------------------------
; _os_entry
;-----------------------------------------------------------------------------
_os_entry:
    ; Put stack at end of RAM
    ld      sp,$0

    ; Init banks
    ld      a,41
    out     (IO_BANK0),a
    inc     a
    out     (IO_BANK1),a
    inc     a
    out     (IO_BANK2),a

    ; Clear RAM
    xor     a
    ld      hl,0
    ld      (hl),a
    ld      de,1
    ld      bc,$F000-1
    ldir

    ; Load initial program
    ld      hl,.initial_program
    ld      de,PGM_PATH
    ld      bc,.initial_program_end - .initial_program
    ldir
    ld      hl,.initial_program_arg
    ld      de,PGM_ARG
    ld      bc,.initial_program_arg_end - .initial_program_arg
    ldir
    jp      _run_program

.initial_program:   defb "/aqds/editor.bin",0
.initial_program_end:

.initial_program_arg:   defb "/blaat.txt",0
.initial_program_arg_end:

;-----------------------------------------------------------------------------
; _go_fileman
;-----------------------------------------------------------------------------
_go_fileman:
    ; Load file manager
    ld      hl,.initial_program
    ld      de,PGM_PATH
    ld      bc,.initial_program_end - .initial_program
    ldir
    jp      _run_program

.initial_program:   defb "/aqds/fileman.bin",0
.initial_program_end:

;-----------------------------------------------------------------------------
; _run_program
;-----------------------------------------------------------------------------
_run_program:
    call    _esp_closeall
    call    _esp_openfile
    or      a
    jp      nz,_go_basic

    ; Load file to $100
    xor     a
    ld      hl,$100
    ld      de,$F000-$100
    call    _esp_read_bytes
    call    _esp_closeall

    ; Jump to program
    jp      $100

;-----------------------------------------------------------------------------
; _go_basic
;-----------------------------------------------------------------------------
_go_basic:
    ; Restore bank configuration
    ld      a,$C0
    out     (IO_BANK0),a
    ld      a,33
    out     (IO_BANK1),a
    inc     a
    out     (IO_BANK2),a

    ; Jump to return-to-BASIC routine
    jp      ret_basic

;-----------------------------------------------------------------------------
; Issue command to ESP
;-----------------------------------------------------------------------------
_esp_cmd:
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
    jp      _esp_send_byte

;-----------------------------------------------------------------------------
; Wait for data from ESP
;-----------------------------------------------------------------------------
_esp_get_byte:
.wait:
    in      a,(IO_ESPCTRL)
    and     a,1
    jr      z,.wait
    in      a,(IO_ESPDATA)
    ret

;-----------------------------------------------------------------------------
; Write data to ESP
;-----------------------------------------------------------------------------
_esp_send_byte:
    push    a

.wait:
    in      a,(IO_ESPCTRL)
    and     a,2
    jr      nz,.wait

    pop     a
    out     (IO_ESPDATA),a
    ret

;-----------------------------------------------------------------------------
; _esp_closeall
;-----------------------------------------------------------------------------
_esp_closeall:
    ld      a,ESPCMD_CLOSEALL
    call    _esp_cmd
    call    _esp_get_byte
    ret

;-----------------------------------------------------------------------------
; _esp_openfile
;-----------------------------------------------------------------------------
_esp_openfile:
    ld      a,ESPCMD_OPEN
    call    _esp_cmd
    ld      a,FO_RDWR           ; Flags (read-only)
    call    _esp_send_byte
    ld      hl,PGM_PATH
.1: ld      a,(hl)
    call    _esp_send_byte
    inc     hl
    or      a
    jr      nz,.1
    jp      _esp_get_byte

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
_esp_read_bytes:
    ld      b,a
    ld      a,ESPCMD_READ
    call    _esp_cmd
    ld      a,b

    ; Send file descriptor
    call    _esp_send_byte

    ; Send read size
    ld      a,e
    call    _esp_send_byte
    ld      a,d
    call    _esp_send_byte

    ; Get result
    call    _esp_get_byte
    or      a
    ret     nz

    ; Get number of bytes actual read
    call    _esp_get_byte
    ld      e,a
    call    _esp_get_byte
    ld      d,a

    push    de

.loop:
    ; Done reading? (DE=0)
    ld      a,d
    or      a,e
    jr      z,.done

    call    _esp_get_byte
    ld      (hl),a
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de

    xor     a
    ret

;-----------------------------------------------------------------------------
; End of OS
;-----------------------------------------------------------------------------
_end:
    dephase
_os_end:
