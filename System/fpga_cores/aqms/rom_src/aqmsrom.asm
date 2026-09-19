;-----------------------------------------------------------------------------
; IO ports
;-----------------------------------------------------------------------------
IO_ESPCTRL          equ $10
IO_ESPDATA          equ $11
IO_VDPDATA          equ $BE
IO_VDPCTRL          equ $BF
IO_JOY1             equ $C0
IO_JOY2             equ $C1

JOY1_UP             equ 0
JOY1_DOWN           equ 1
JOY1_LEFT           equ 2
JOY1_RIGHT          equ 3
JOY1_TL             equ 4
JOY1_TR             equ 5

RAMSEL              equ $FFFC
BANK0               equ $FFFD
BANK1               equ $FFFE
BANK2               equ $FFFF

;-----------------------------------------------------------------------------
; ESP32 commands
;-----------------------------------------------------------------------------
ESPCMD_RESET        equ $01     ; Reset ESP
ESPCMD_VERSION      equ $02     ; Get version string
ESPCMD_GETDATETIME  equ $03     ; Get current date/time
ESPCMD_KEYMODE      equ $08     ; Set keyboard buffer mode
ESPCMD_GETMOUSE     equ $0C     ; Get mouse state
ESPCMD_OPEN         equ $10     ; Open / create file
ESPCMD_CLOSE        equ $11     ; Close open file
ESPCMD_READ         equ $12     ; Read from file
ESPCMD_WRITE        equ $13     ; Write to file
ESPCMD_SEEK         equ $14     ; Move read/write pointer
ESPCMD_TELL         equ $15     ; Get current read/write
ESPCMD_OPENDIR      equ $16     ; Open directory
ESPCMD_CLOSEDIR     equ $17     ; Close open directory
ESPCMD_READDIR      equ $18     ; Read from directory
ESPCMD_DELETE       equ $19     ; Remove file or directory
ESPCMD_RENAME       equ $1A     ; Rename / move file or directory
ESPCMD_MKDIR        equ $1B     ; Create directory
ESPCMD_CHDIR        equ $1C     ; Change directory
ESPCMD_STAT         equ $1D     ; Get file status
ESPCMD_GETCWD       equ $1E     ; Get current working directory
ESPCMD_CLOSEALL     equ $1F     ; Close any open file/directory descriptor
ESPCMD_OPENDIR83    equ $20     ; Open directory in 8.3 mode

ERR_NOT_FOUND       equ -1      ; File / directory not found
ERR_TOO_MANY_OPEN   equ -2      ; Too many open files / directories
ERR_PARAM           equ -3      ; Invalid parameter
ERR_EOF             equ -4      ; End of file / directory
ERR_EXISTS          equ -5      ; File already exists
ERR_OTHER           equ -6      ; Other error
ERR_NO_DISK         equ -7      ; No disk
ERR_NOT_EMPTY       equ -8      ; Not empty

FO_RDONLY           equ $00     ; Open for reading only
FO_WRONLY           equ $01     ; Open for writing only
FO_RDWR             equ $02     ; Open for reading and writing
FO_ACCMODE          equ $03     ; Mask for above modes
FO_APPEND           equ $04     ; Append mode
FO_CREATE           equ $08     ; Create if non-existant
FO_TRUNC            equ $10     ; Truncate to zero length
FO_EXCL             equ $20     ; Error if already exists

LINES_PER_PAGE      equ 22

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
_cursor_x           equ $C000
_cursor_y           equ $C001
_file_page          equ $C002               ; 8-bit
_selected_entry     equ $C003               ; 8-bit
_prev_joy           equ $C004
_joy                equ $C005
_is_last_page       equ $C006
_page_lines         equ $C007
_dirent_namelen     equ $C008
_dirent             equ $C009
_dirent_date        equ _dirent      + 0    ; 16-bit
_dirent_time        equ _dirent_date + 2    ; 16-bit
_dirent_attr        equ _dirent_time + 2    ;  8-bit
_dirent_size        equ _dirent_attr + 1    ; 32-bit
_dirent_name        equ _dirent_size + 4    ; max 255 bytes
_dirent_end         equ _dirent_name + 256

    org     0

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
_entry:
    ; Disable interrupts
    di
    im      1

    ; Set stack pointer
    ld      sp,$DF00

    ; Clear RAM
    ld      hl,$C000
    ld      de,$C001
    ld      bc,$1FFF
    ld      (hl),$00
    ldir

    ; Configure bank registers
    xor     a
    ld      (BANK0),a

    ; Indicate reset to ESP
    ld      a,ESPCMD_RESET
    call    esp_cmd

    jp      _main

;-----------------------------------------------------------------------------
; Interrupt handler
;-----------------------------------------------------------------------------
;     org     $38
; _irq_handler:
;     ex      af,af'
;     exx

;     in      a,(IO_VDPCTRL)
;     bit     7,a
;     jr      z,.line_irq

;     ; V-sync interrupt
;     jr      .exit

;     ; Line interrupt
; .line_irq:

; .exit:
;     exx
;     ex      af,af'
;     ei
;     reti

;-----------------------------------------------------------------------------
; Main program
;-----------------------------------------------------------------------------
_main:
    ; Init system
    call    _init

    ; Show game selection menu
    call    _clear_screen
    call    _draw_title
    call    _game_select

    ; Load game
    call    _hide_sprite
    call    _clear_screen
    call    _draw_title
    jp      _load_game

;-----------------------------------------------------------------------------
; Put selected entry in _dirent
;-----------------------------------------------------------------------------
_select_entry:
    ; Close any open descriptors and open directory
    call    esp_close_all
    call    esp_opendir

    ; Skip LINES_PER_PAGE * _file_page file entries
    ld      a,(_file_page)
    ld      b,a
    inc     b
.loop2:
    dec     b
    jr      z,.skip_done
    ld      c,LINES_PER_PAGE
    call    .skip_entries
    jr      .loop2
.skip_done:

    ; Skip _selected_entry file entries
    ld      a,(_selected_entry)
    ld      c,a
    call    .skip_entries

    ; Read the directory entry
    call    esp_readdir

    call    esp_close_all
    ret

    ; Skip C entries
.skip_entries:
    ld      a,c
    or      a
    ret     z
.loop:
    push    bc
    call    esp_readdir
    pop     bc
    dec     c
    ret     z
    jr      .loop

;-----------------------------------------------------------------------------
; Load game
;-----------------------------------------------------------------------------
_load_game:
    call    _select_entry

    ; Print filename
    xor     a
    ld      (_cursor_x),a
    ld      a,8
    ld      (_cursor_y),a
    call    _setaddr
    ld      hl,.loading_str
    call    _puts
    call    _draw_filename

    ; Load file into external RAM to emulate ROM
    call    _load_file
    call    esp_close_all

    ; Load stub into RAM and jump to it
    ld      de,$D000
    ld      hl,_stub
    ld      bc,_stub_end - _stub
    ldir
    jp      $D000

.loading_str:  defb "Loading:",10,10,0

;-----------------------------------------------------------------------------
; Load file into RAM
;-----------------------------------------------------------------------------
_load_file:
    call    esp_close_all

    ; Open file in _dirent_name
    ld      a,ESPCMD_OPEN
    call    esp_cmd
    ld      a,FO_RDONLY
    call    esp_send_byte
    ld      hl,_dirent_name
.1: ld      a,(hl)
    inc     hl
    call    esp_send_byte
    or      a
    jr      nz,.1
    call    esp_get_byte

    ; If the image has a 512 byte header, skip it
    ld      a,(_dirent_size+1)
    bit     1,a
    jr      z,.2
    ld      a,ESPCMD_SEEK
    call    esp_cmd
    xor     a
    call    esp_send_byte
    call    esp_send_byte
    ld      a,2
    call    esp_send_byte
    xor     a
    call    esp_send_byte
    call    esp_send_byte
    call    esp_get_byte
.2:
    ; Map bank1 to first page
    xor     a
    ld      (BANK1),a

.3: ld      hl,$4000
    ld      de,$4000
    call    esp_read_bytes
    ld      a,d
    cp      a,$40
    jr      nz,.done

    ld      a,'.'
    call    _putchar

    ld      a,(BANK1)
    inc     a
    ld      (BANK1),a
    jr      .3

.done:
    ret

;-----------------------------------------------------------------------------
; Draw title
;-----------------------------------------------------------------------------
_draw_title:
    xor     a
    ld      (_cursor_x),a
    ld      (_cursor_y),a
    call    _setaddr
    ld      hl,.str
    call    _puts
    ret

.str:  defb "<<<< Aquarius Master System >>>>",0

;-----------------------------------------------------------------------------
; Game selection
;-----------------------------------------------------------------------------
_game_select:
    ; Draw screen
    call    _draw_screen

    ; Process input
.input:
    call    _update_joy
    ld      a,(_joy)
    bit     JOY1_RIGHT,a
    jr      nz,.next_page
    bit     JOY1_LEFT,a
    jr      nz,.prev_page
    bit     JOY1_UP,a
    jr      nz,.prev_entry
    bit     JOY1_DOWN,a
    jr      nz,.next_entry
    bit     JOY1_TL,a
    jr      nz,.dir_up
    bit     JOY1_TR,a
    jr      nz,.select
    jr      .input

.next_page:
    call    _next_page
    jr      .input
.prev_page:
    call    _prev_page
    jr      .input
.prev_entry:
    call    _prev_entry
    jr      .input
.next_entry:
    call    _next_entry
    jr      .input
.dir_up:
    call    _dir_up
    jr      .input
.select:
    ld      a,(_page_lines)
    or      a
    jr      z,.input
    call    _select_entry
    ld      a,(_dirent_attr)
    bit     0,a
    ret     z       ; Normal file, load it
    call    _chdir
    jr      .input

;-----------------------------------------------------------------------------
; Change directory
;-----------------------------------------------------------------------------
_chdir:
    ; Directory -> change directory
    ld      a,ESPCMD_CHDIR
    call    esp_cmd

    ld      hl,_dirent_name
.1: ld      a,(hl)
    inc     hl
    call    esp_send_byte
    or      a
    jr      nz,.1
    call    esp_get_byte

    ; Reset cursor
    xor     a
    ld      (_file_page),a
    ld      (_selected_entry),a

    ; Update screen
    call    _draw_screen
    ret

;-----------------------------------------------------------------------------
; Directory up
;-----------------------------------------------------------------------------
_dir_up:
    ; Directory up
    ld      a,ESPCMD_CHDIR
    call    esp_cmd
    ld      a,'.'
    call    esp_send_byte
    call    esp_send_byte
    xor     a
    call    esp_send_byte
    call    esp_get_byte

    ; Reset cursor
    xor     a
    ld      (_file_page),a
    ld      (_selected_entry),a

    ; Update screen
    call    _draw_screen
    ret

;-----------------------------------------------------------------------------
; Previous entry
;-----------------------------------------------------------------------------
_prev_entry:
    ld      a,(_selected_entry)
    or      a
    jr      z,.prev_page
    dec     a
    ld      (_selected_entry),a
    jp      _update_sprite

.prev_page:
    call    _prev_page
    ret     z
    ld      a,LINES_PER_PAGE-1
    ld      (_selected_entry),a
    jp      _update_sprite

;-----------------------------------------------------------------------------
; Next entry
;-----------------------------------------------------------------------------
_next_entry:
    ld      a,(_selected_entry)
    cp      a,LINES_PER_PAGE-1
    jr      nc,.next_page
    inc     a

    ld      b,a
    ld      a,(_page_lines)
    cp      b
    ret     c
    ld      a,b
    ld      (_selected_entry),a
    jp      _update_sprite

.next_page:
    call    _next_page
    ret     nz
    xor     a
    ld      (_selected_entry),a
    jp      _update_sprite

;-----------------------------------------------------------------------------
; Go to previous page (result: z = no change, at first page)
;-----------------------------------------------------------------------------
_prev_page:
    ld      a,(_file_page)
    or      a
    ret     z
    dec     a
    ld      (_file_page),a
    call    _draw_screen
    ld      a,1
    or      a
    ret

;-----------------------------------------------------------------------------
; Go to next page (result: nz = no change, at last page)
;-----------------------------------------------------------------------------
_next_page:
    ld      a,(_is_last_page)
    or      a
    ret     nz                  ; At last page
    ld      a,(_file_page)
    inc     a
    ld      (_file_page),a
    call    _draw_screen
    xor     a
    ret

;-----------------------------------------------------------------------------
; Update joystick state
;-----------------------------------------------------------------------------
_update_joy:
    in      a,(IO_JOY1)
    xor     $FF
    ld      b,a

    ld      a,(_prev_joy)
    xor     $FF
    and     b
    ld      (_joy),a

    ld      a,b
    ld      (_prev_joy),a
    ret

;-----------------------------------------------------------------------------
; Draw screen
;-----------------------------------------------------------------------------
_draw_screen:
    ; Home cursor
    ld      a,2
    ld      (_cursor_y),a
    xor     a
    ld      (_cursor_x),a
    call    _setaddr

    ; Close any open descriptors and open directory
    call    esp_close_all
    call    esp_opendir

    ; Skip LINES_PER_PAGE * _file_page file entries
    ld      a,(_file_page)
    ld      b,a
    inc     b
.loop2:
    dec     b
    jr      z,.skip_done
    ld      c,LINES_PER_PAGE
.loop:
    push    bc
    call    esp_readdir
    pop     bc
    jr      nz,.end_of_dir
    dec     c
    jr      nz,.loop
    jr      .loop2
.skip_done:

    xor     a
    ld      (_page_lines),a

    ; Draw entries
.next:
    ld      a,(_cursor_y)
    cp      24
    jr      nc,.done2
    
    call    esp_readdir
    jr      nz,.done

    ld      a,(_page_lines)
    inc     a
    ld      (_page_lines),a

    ld      a,' '
    call    _putchar
    call    _draw_filename
    jr      .next

.done2:
    call    esp_readdir
.done:
    jr      nz,.end_of_dir
    xor     a
    ld      (_is_last_page),a
    jr      .fill_screen
.end_of_dir:
    ld      a,1
    ld      (_is_last_page),a
    jr      .fill_screen

.fill_screen:
    ld      a,(_cursor_y)
    cp      24
    jr      nc,.done3
    call    _pad
    jr      .fill_screen

.done3:
    ld      a,(_page_lines)
    or      a
    jr      z,.nofiles
    call    _update_sprite
    ret

.nofiles:
    ; No files found
    ld      a,4
    ld      (_cursor_x),a
    ld      a,8
    ld      (_cursor_y),a
    call    _setaddr
    ld      hl,.nofiles_str
    call    _puts
    call    _hide_sprite
    ret

.nofiles_str:  defb "No .SMS ROM files found",0

_pad:
    ld      a,' '
    call    _putchar
    ld      a,(_cursor_x)
    or      a
    jr      nz,_pad
    ret

;-----------------------------------------------------------------------------
; Draw and pad filename
;-----------------------------------------------------------------------------
_draw_filename:
    ; Print '[' if directory
    ld      a,(_dirent_attr)
    bit     0,a
    jr      z,.1
    ld      a,'['
    call    _putchar
.1:
    ; Print file name
    ld      hl,_dirent_name
    ld      a,(_dirent_namelen)
    ld      c,a
.2: ld      a,(hl)
    inc     hl
    call    _putchar
    dec     c
    jr      z,.3
    ld      a,(_cursor_x)
    or      a
    ret     z
    jr      .2
.3:
    ; Print ']' if directory
    ld      a,(_dirent_attr)
    bit     0,a
    jr      z,.4
    ld      a,']'
    call    _putchar
.4:
    ; Pad if not already on the new line
    ld      a,(_cursor_x)
    or      a
    ret     z
    jp      _pad

;-----------------------------------------------------------------------------
; Initialization
;-----------------------------------------------------------------------------
_init:
    ; Read VDP control
    in      a,(IO_VDPCTRL)

    ; Load VDP settings
    ld      c,IO_VDPCTRL
    ld      b,.vdp_settings_end - .vdp_settings
    ld      hl,.vdp_settings
    otir

    ; Load palette
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$C0
    out     (IO_VDPCTRL),a
    ld      c,IO_VDPDATA
    ld      b,.palette_end - .palette
    ld      hl,.palette
    otir
    ld      b,.palette_end - .palette
    ld      hl,.palette
    otir

    ; Load patterns
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$40
    out     (IO_VDPCTRL),a

    ld      d,(font_end-font_start)/32
    ld      c,IO_VDPDATA
    ld      hl,font_start
.l: ld      b,32
    otir
    dec     d
    jr      nz,.l

    ret

.vdp_settings:
    defb    $06,$80, $C0,$81, $FF,$82, $FF,$83, $FF,$84, $FF,$85, $00,$86, $00,$87, $00,$88, $00,$89
.vdp_settings_end:

.palette:
    defb    $00,$03,$0C,$0F,$30,$33,$3C,$3F,$2A,$28,$33,$21,$1F,$1C,$02,$15
.palette_end:

    include "font.asm"

;-----------------------------------------------------------------------------
; Update 'cursor' sprite
;-----------------------------------------------------------------------------
_update_sprite:
    ld      a,(_page_lines)
    ld      b,a
    ld      a,(_selected_entry)
    cp      b
    jr      c,.ok

    ld      a,b
    sub     a,1
    ld      (_selected_entry),a

.ok:
    ; Y
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$40|$3F
    out     (IO_VDPCTRL),a
    ld      a,(_selected_entry)
    add     a,a
    add     a,a
    add     a,a
    add     a,6+8
    out     (IO_VDPDATA),a
    ld      a,$D0               ; Terminate sprite list
    out     (IO_VDPDATA),a

    ; X/Tile
    ld      a,$80
    out     (IO_VDPCTRL),a
    ld      a,$40|$3F
    out     (IO_VDPCTRL),a
    xor     a                   ; X
    out     (IO_VDPDATA),a
    ld      a,'>'-32            ; Tile index
    out     (IO_VDPDATA),a

    ret

;-----------------------------------------------------------------------------
; Hide sprites
;-----------------------------------------------------------------------------
_hide_sprite:
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$40|$3F
    out     (IO_VDPCTRL),a
    ld      a,$D0               ; Terminate sprite list
    out     (IO_VDPDATA),a
    ret

;-----------------------------------------------------------------------------
; Clear screen
;-----------------------------------------------------------------------------
_clear_screen:
    ; Map is located at $3800

    ; vaddr = $3800
    xor     a
    out     (IO_VDPCTRL),a
    ld      a,$40|$38
    out     (IO_VDPCTRL),a

    ; write 28x32x2 zeroes
    xor     a
    ld      c,28
.1: ld      b,32
.2: out     (IO_VDPDATA)
    out     (IO_VDPDATA)
    dec     b
    jr      nz,.2
    dec     c
    jr      nz,.1

    ret

;-----------------------------------------------------------------------------
; Set addr
;-----------------------------------------------------------------------------
_setaddr:
    ; _cursor_y * 64
    ld      h,0
    ld      a,(_cursor_y)
    ld      l,a
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl

    ; += _cursor_x * 2
    ld      a,(_cursor_x)
    add     a,a
    add     a,l
    out     (IO_VDPCTRL),a

    ld      a,h
    add     a,$40|$38
    out     (IO_VDPCTRL),a
    ret

;-----------------------------------------------------------------------------
; Put char
;-----------------------------------------------------------------------------
_putchar:
    cp      10
    jp      z,_newline
    sub     a,32
    cp      a,95
    jr      c,.1
    xor     a
.1: out     (IO_VDPDATA)
    xor     a
    out     (IO_VDPDATA)

    ; Increment cursor X
    ld      a,(_cursor_x)
    inc     a
    ld      (_cursor_x),a
    cp      32
    ret     c
    xor     a
    ld      (_cursor_x),a

    ; Increment cursor Y
    ld      a,(_cursor_y)
    inc     a
    ld      (_cursor_y),a

    ret

;-----------------------------------------------------------------------------
; New line
;-----------------------------------------------------------------------------
_newline:
    push    hl
    xor     a
    ld      (_cursor_x),a
    ld      a,(_cursor_y)
    inc     a
    ld      (_cursor_y),a
    call    _setaddr
    pop     hl
    ret

;-----------------------------------------------------------------------------
; Put string
;-----------------------------------------------------------------------------
_puts:
    ld      a,(hl)
    or      a
    ret     z
    call    _putchar
    inc     hl
    jr      _puts

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
; Send string in HL to ESP
;-----------------------------------------------------------------------------
esp_send_str:
    ld      a,(hl)
    call    esp_send_byte
    or      a
    ret     z
    inc     hl
    jr      esp_send_str

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
; Close any open file/directory descriptor
;
; Clobbered registers: A
;-----------------------------------------------------------------------------
esp_close_all:
    ld      a,ESPCMD_CLOSEALL
    call    esp_cmd
    jp      esp_get_byte

;-----------------------------------------------------------------------------
; Open directory
;-----------------------------------------------------------------------------
esp_opendir:
    ld      a,ESPCMD_OPENDIR
    call    esp_cmd
    xor     a
    call    esp_send_byte
    jp      esp_get_byte

;-----------------------------------------------------------------------------
; Read directory entry
;-----------------------------------------------------------------------------
esp_readdir:
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

    ; Read filename
    ld      c,0
.1: call    esp_get_byte
    ld      (hl),a
    inc     hl
    or      a
    jr      z,.2
    inc     c
    jr      .1
.2: ld      a,c
    ld      (_dirent_namelen),a

    ; Directory?
    ld      a,(_dirent_attr)
    bit     0,a
    jr      nz,.ok

    ; Regular file

    ; Skip if filename shorter than 5 characters
    ld      a,(_dirent_namelen)
    cp      5
    jr      c,esp_readdir

    ; Check extension to be .sms
    sub     a,4
    ld      (_dirent_namelen),a
    ld      hl,_dirent_name
    ld      b,0
    ld      c,a
    add     hl,bc
    ex      de,hl
    ld      hl,.ext
.cmp:
    ld      a,(de)
    call    _to_upper
    inc     de
    cp      (hl)
    inc     hl
    jr      nz,esp_readdir      ; Skip if mismatch
    or      a
    jr      nz,.cmp

.ok:
    xor     a
    ret

.ext:   defb ".SMS",0

;-----------------------------------------------------------------------------
; _to_upper
;-----------------------------------------------------------------------------
_to_upper:
    cp      'a'
    ret     c
    cp      'z'+1
    ret     nc
    sub     $20
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
    ; call    esp_get_result

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
    ret

;-----------------------------------------------------------------------------
; Stub used for starting game
;-----------------------------------------------------------------------------
_stub:
    phase   $D000

    xor     a
    ld      (BANK0),a
    inc     a
    ld      (BANK1),a
    inc     a
    ld      (BANK2),a
    xor     a
    ld      (RAMSEL),a
    jp      0

    dephase
_stub_end:
