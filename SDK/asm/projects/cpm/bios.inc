    include "regs.inc"

;-----------------------------------------------------------------------------
; BIOS jump table
;-----------------------------------------------------------------------------
BOOT:       jp  _boot
WBOOT:      jp  _wboot
CONST:      jp  _const
CONIN:      jp  _conin
CONOUT:     jp  termemu_out
LIST:       jp  _list
PUNCH:      jp  _punch
READER:     jp  _reader
HOME:       jp  _home
SELDSK:     jp  _seldsk
SETTRK:     jp  _settrk
SETSEC:     jp  _setsec
SETDMA:     jp  _setdma
READ:       jp  _read
WRITE:      jp  _write
PRSTAT:     jp  _prstat
SECTRN:     jp  _sectran
SETDISK:    jp  _setdisk
GETDISK:    jp  _getdisk

;-----------------------------------------------------------------------------
; Variables used by BIOS
;-----------------------------------------------------------------------------
_dma_addr:  defw 0
_keyval:    defb 0

_diskfd:    defb 0
_diskno:    defb 0
_prev_disk: defb 0
_track:     defb 0
_sector:    defb 0

;-----------------------------------------------------------------------------
; Disk image paths
;-----------------------------------------------------------------------------
MAX_DISKS:  equ 4

_path_a:    defb    "/CPM/DISK-A.DSK",0
            defs    64-($-_path_a)
_path_b:    defs    64
_path_c:    defs    64
_path_d:    defs    64

;-----------------------------------------------------------------------------
; Disk parameter header
;-----------------------------------------------------------------------------
_dph_disk0:
    defw    _xlt_skew6  ; Address of XLT
    defw    0,0,0       ; Scratch area
    defw    _dirbuf     ; Address of Directory operation buffer
    defw    _dpb_ibm8in ; Address of Disk parameter block
    defw    _chk00      ; Address of a scratch pad area used for software check for changed disks. This address is different for each DPH.
    defw    _all00      ; Address of a scratch pad area used by the BDOS to keep disk storage allocation information. This address is different for each DPH.
_dph_disk1:
    defw    _xlt_skew6  ; Address of XLT
    defw    0,0,0       ; Scratch area
    defw    _dirbuf     ; Address of Directory operation buffer
    defw    _dpb_ibm8in ; Address of Disk parameter block
    defw    _chk01      ; Address of a scratch pad area used for software check for changed disks. This address is different for each DPH.
    defw    _all01      ; Address of a scratch pad area used by the BDOS to keep disk storage allocation information. This address is different for each DPH.
_dph_disk2:
    defw    _xlt_skew6  ; Address of XLT
    defw    0,0,0       ; Scratch area
    defw    _dirbuf     ; Address of Directory operation buffer
    defw    _dpb_ibm8in ; Address of Disk parameter block
    defw    _chk02      ; Address of a scratch pad area used for software check for changed disks. This address is different for each DPH.
    defw    _all02      ; Address of a scratch pad area used by the BDOS to keep disk storage allocation information. This address is different for each DPH.
_dph_disk3:
    defw    _xlt_skew6  ; Address of XLT
    defw    0,0,0       ; Scratch area
    defw    _dirbuf     ; Address of Directory operation buffer
    defw    _dpb_ibm8in ; Address of Disk parameter block
    defw    _chk03      ; Address of a scratch pad area used for software check for changed disks. This address is different for each DPH.
    defw    _all03      ; Address of a scratch pad area used by the BDOS to keep disk storage allocation information. This address is different for each DPH.

;-----------------------------------------------------------------------------
; Skew of 6 as used by Single Sided 8" floppy format
;-----------------------------------------------------------------------------
_xlt_skew6:
    defb     1,  7, 13, 19  ; sectors  1,  2,  3,  4
    defb    25,  5, 11, 17  ; sectors  5,  6,  7,  6
    defb    23,  3,  9, 15  ; sectors  9, 10, 11, 12
    defb    21,  2,  8, 14  ; sectors 13, 14, 15, 16
    defb    20, 26,  6, 12  ; sectors 17, 18, 19, 20
    defb    18, 24,  4, 10  ; sectors 21, 22, 23, 24
    defb    16, 22          ; sectors 25, 26

;-----------------------------------------------------------------------------
; Disk parameter block for original CP/M format
; (Single Sided 8" floppy, 256256 bytes)
;-----------------------------------------------------------------------------
_dpb_ibm8in:
    defw    26      ; SPT: total number of sectors per track. 
    defb    3       ; BSH: data allocation block shift factor, determined by the data block allocation size.
    defb    7       ; BLM: data allocation block mask ((2^BSH)-1).
    defb    0       ; EXM: extent mask, determined by the data block allocation size and the number of disk blocks.
    defw    242     ; DSM: determines the total storage capacity of the disk drive.
    defw    63      ; DRM: determines the total number of directory entries that can be stored on this drive.
    defb    192     ; AL0: reserved directory blocks
    defb    0       ; AL1: reserved directory blocks
    defw    16      ; CKS: size of the directory check vector
    defw    2       ; OFF: number of reserved tracks at the beginning of the (logical) disk

_dirbuf:    defs    128     ; Scratch directory area
_all00:     defs    31      ; Allocation vector 0
_all01:     defs    31      ; Allocation vector 1
_all02:     defs    31      ; Allocation vector 2
_all03:     defs    31      ; Allocation vector 3
_chk00:     defs    16      ; Check vector 0
_chk01:     defs    16      ; Check vector 1
_chk02:     defs    16      ; Check vector 2
_chk03:     defs    16      ; Check vector 3

;-----------------------------------------------------------------------------
; Initialize
;-----------------------------------------------------------------------------
_init:
    ; Set video mode to 80-columns mode text with remapped border character
    ld      a,$61
    out     (IO_VCTRL),a

    ; Close any open descriptor
    ld      a,ESPCMD_CLOSEALL
    call    esp_cmd
    call    esp_get_byte

    ; Enable keyboard repeat
    ld      a,ESPCMD_KEYMODE
    call    esp_cmd
    ld      a,7
    call    esp_send_byte
    call    esp_get_byte

    ret

_filename: defb "cpm2-1.dsk",0

;-----------------------------------------------------------------------------
; Send string in HL
;-----------------------------------------------------------------------------
_esp_send_string:
    ld      a,(hl)
    call    esp_send_byte
    or      a
    ret     z
    inc     hl
    jr      _esp_send_string

;-----------------------------------------------------------------------------
; Get image filename for given disk
;
; C  = disk drive (0-0-(MAX_DISKS-1))
; HL = zero-terminated string (max 63 characters + 1)
;-----------------------------------------------------------------------------
_getdisk:
    ; Check disk drive range
    ld      hl,0
    ld      a,c
    ld      (_diskno),a
    cp      MAX_DISKS       ; must be between 0-(MAX_DISKS-1)
    ret     nc

    ; Calculate address of filename buffer
    ld      a,c
    ld      l,a
    ld      h,0
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    ld      de,_path_a
    add     hl,de
    scf                     ; Set carry to indicate success
    ret

;-----------------------------------------------------------------------------
; Set image filename for given disk
;
; C  = disk drive (0-0-(MAX_DISKS-1))
; HL = zero-terminated string (max 63 characters + 1)
;-----------------------------------------------------------------------------
_setdisk:
    push    hl
    call    _getdisk
    ex      de,hl
    pop     hl
    ret     nc

    ; Copy 63 bytes and zero terminate
    ld      bc,63
    ldir
    xor     a
    ld      (de),a
    ret

;-----------------------------------------------------------------------------
; Copy CP/M to page 36 so it can be reloaded by _load_cpm.
;-----------------------------------------------------------------------------
_save_cpm:
    ld      a,36
    out     (IO_BANK0),a
    ld      de,$0
    ld      hl,ccp
    ld      bc,bios-ccp
    ldir
    ld      a,32
    out     (IO_BANK0),a
    ret

;-----------------------------------------------------------------------------
; Load CP/M from page 36
;-----------------------------------------------------------------------------
_load_cpm:
    ld      a,36
    out     (IO_BANK0),a
    ld      de,ccp
    ld      hl,0
    ld      bc,bios-ccp
    ldir
    ld      a,32
    out     (IO_BANK0),a
    ret

;-----------------------------------------------------------------------------
; BOOT - Cold start routine
;-----------------------------------------------------------------------------
_boot:
    ld      sp,$0           ; Setup a stack
    call    _save_cpm       ; Save CP/M for later reloading by warm boot

    ; Clear user memory
    ld      hl,$0
    ld      bc,ccp
    ld      a,0
    call    memset

    call    _init           ; Initialize
    ld      bc,.bootstr     ; Boot message
    call    PRTMESG
    jr      _go_cpm

.bootstr: defb "CP/M 2.2 for Aquarius+ (v1.0)",13,10,"$"

;-----------------------------------------------------------------------------
; WBOOT - Warm boot - reload command processor
;
; Reloads the command processor and (on some systems) the BDOS as well.
; How it does this is implementation-dependent; it may use the reserved tracks
; of a floppy disc or extra memory.
;-----------------------------------------------------------------------------
_wboot:
    ld      sp,$0           ; Setup a stack
    call    _load_cpm       ; Load CP/M
    call    _init           ; Initialize

;-----------------------------------------------------------------------------
; Jump to CPM
;-----------------------------------------------------------------------------
_go_cpm:
    ; Set up warm boot entry point
    ld      a,$C3
    ld      (0),a
    ld      hl,WBOOT
    ld      (1),hl

    ; Set up BDOS entry point
    ld      (5),a
    ld      hl,BDOS
    ld      (6),hl

    ; Set default DMA address
    ld      bc,$80
    call    SETDMA

    ; Get current disk number and jump to CCP
    ld      a,(4)
    ld      c,a
    jp      CCP

;-----------------------------------------------------------------------------
; CONST - Console status
;
; Returns its status in A; 0 if no character is ready, $FF if one is.
;-----------------------------------------------------------------------------
_const:
    ; First check _keyval buffer
    ld      a,(_keyval)
    or      a
    jr      nz,.haschar

    ; Check KEYBUF IO port
    in      a,(IO_KEYBUF)
    or      a
    ret     z   ; No key

    ; Save value in _keyval buffer
    ld      (_keyval),a

.haschar:
    ld      a,$FF
    ret

;-----------------------------------------------------------------------------
; CONIN - Console input
;
; Wait until the keyboard is ready to provide a character, and return it in A.
;-----------------------------------------------------------------------------
_conin:
    call    _const
    or      a
    jr      z,_conin
    ld      a,(_keyval)
    ld      b,a
    xor     a
    ld      (_keyval),a
    ld      a,b
    ret

;-----------------------------------------------------------------------------
; LIST - Printer output
;
; Write the character in C to the printer. If the printer isn't ready,
; wait until it is.
;-----------------------------------------------------------------------------
_list:
    ret

;-----------------------------------------------------------------------------
; LISTST - Status of list device
;
; Return status of current printer device.
; Returns A=0 (not ready) or A=$FF (ready).
;-----------------------------------------------------------------------------
_prstat:
    xor     a
    ret

;-----------------------------------------------------------------------------
; PUNCH - Paper tape punch output
;
; Write the character in C to the "paper tape punch" - or whatever the current
; auxiliary device is. If the device isn't ready, wait until it is.
;-----------------------------------------------------------------------------
_punch:
    ret

;-----------------------------------------------------------------------------
; READER - Paper tape reader input
;
; Read a character from the "paper tape reader" - or whatever the current
; auxiliary device is. If the device isn't ready, wait until it is. The
; character will be returned in A. If this device isn't implemented, return
; character 26 (^Z).
;-----------------------------------------------------------------------------
_reader:
    ld      a,26
    ret

;-----------------------------------------------------------------------------
; SETDMA - Set DMA address
;
; The next disc operation will read its data from (or write its data to)
; the address given in BC.
;-----------------------------------------------------------------------------
_setdma:
    ld      (_dma_addr),bc
    ret

;-----------------------------------------------------------------------------
; memset
;-----------------------------------------------------------------------------
memset:
    inc     c
    inc     b
    jr      .start
.repeat:
    ld      (hl),a
    inc     hl
.start:
    dec     c
    jr      nz,.repeat
    dec     b
    jr      nz,.repeat
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
; _home
;-----------------------------------------------------------------------------
_home:
    ld      c,0
    call    SETTRK
    ret

;-----------------------------------------------------------------------------
; _seldsk
;
; C = disk drive (0:A, 1:B, ..., 15:P)
;
; Returns:
; HL = Address of 16-byte Disk Parameter Header (DPH)
; On error, HL = $0000
;-----------------------------------------------------------------------------
_seldsk:
    ; Select disk given by register c
    ld      a,c
    ld      (_diskno),a
    cp      MAX_DISKS       ; must be between 0-(MAX_DISKS-1)
    jr      nc,.err

    ; Close file descriptor
    ld      a,ESPCMD_CLOSE
    call    esp_cmd
    ld      a,(_diskfd)
    call    esp_send_byte
    call    esp_get_byte

    ; Open disk image
    ld      a,ESPCMD_OPEN
    call    esp_cmd
    ld      a,FO_RDWR           ; Flags (read-only)
    call    esp_send_byte

    ; Get filename of disk image
    ld      a,(_diskno)
    ld      l,a
    ld      h,0
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    ld      de,_path_a
    add     hl,de
    call    _esp_send_string
    call    esp_get_byte
    or      a
    jr      nz,.err
    ld      (_diskfd),a

    ; Compute proper disk parameter header address (hl = _dph_disk0 + _diskno * 8)
    ld      a,(_diskno)
    ld      l,a
    ld      h,0
    add     hl,hl
    add     hl,hl
    add     hl,hl
    add     hl,hl
    ld      de,_dph_disk0
    add     hl,de

    ; Store current disk number, so we can restore it if a next disk selection fails
    ld      a,(_diskno)
    ld      (_prev_disk),a

    ret

.err:
    ; Restore disk number
    ld      a,(_prev_disk)
    ld      (4),a

    ; Report error (hl=0)
    ld      hl,0
    ret

;-----------------------------------------------------------------------------
; _settrk
;
; BC = track number for subsequent disk accesses on the currently selected
;      drive. The sector number in BC is the same as the number returned
;      from the SECTRAN entry point. You can choose to seek the selected
;      track at this time or delay the seek until the next read or write
;      actually occurs. Register BC can take on values in the range 0-76
;      corresponding to valid track numbers for standard floppy disk drives
;      and 0-65535 for nonstandard disk subsystems.
;-----------------------------------------------------------------------------
_settrk:
    ld      a,c
    ld      (_track),a
    ret

;-----------------------------------------------------------------------------
; _setsec
;
; BC = sector number (1-26) for subsequent disk accesses on the currently;
;      selected drive. The sector number in BC is the same as the number
;      returned from the SECTRAN entry point. You can choose to send this
;      information to the controller at this point or delay sector selection
;      until a read or write operation occurs.
;-----------------------------------------------------------------------------
_setsec:
    ld      a,c
    ld      (_sector),a
    ret

;-----------------------------------------------------------------------------
; _sectran   Logical-to-physical sector translation
;
; BC = logical sector number
; DE = translate table address
;
; HL = resulting sector number
;-----------------------------------------------------------------------------
_sectran:
    ld      a,d
    or      a,e
    jr      z,.notran

    ex      de,hl
    add     hl,bc
    ld      l,(hl)
    ld      h,0
    ret

.notran:
    ld      h,b
    ld      l,c
    inc     hl
    ret

;-----------------------------------------------------------------------------
; _read
;-----------------------------------------------------------------------------
_read:
    call    _seek_disk
    ld      a,(_diskfd)
    ld      hl,(_dma_addr)
    ld      de,128
    call    _esp_read_bytes
    xor     a
    ret

;-----------------------------------------------------------------------------
; _write
;-----------------------------------------------------------------------------
_write:
    call    _seek_disk
    ld      a,(_diskfd)
    ld      hl,(_dma_addr)
    ld      de,128
    call    _esp_write_bytes
    xor     a
    ret

;-----------------------------------------------------------------------------
; _seek_disk
;-----------------------------------------------------------------------------
_seek_disk:
    ld      d,0
    ld      a,(_track)
    ld      e,a
    ld      bc,26
    call    _mul16
    ld      d,0
    ld      a,(_sector)
    dec     a
    ld      e,a
    add     hl,de
    ex      de,hl
    ld      a,(_diskfd)
    jp      _seek

;-----------------------------------------------------------------------------
; This routine performs the operation DEHL=BC*DE
;-----------------------------------------------------------------------------
_mul16:
    ld hl,0
    ld a,16
.1: add hl,hl
    rl e
    rl d
    jp nc,.2
    add hl,bc
    jp nc,.2
    inc de      ; This instruction (with the jump) is like an "ADC DE,0"
.2: dec a
    jp nz,.1
    ret

;-----------------------------------------------------------------------------
; Seek to record DE (file descriptor in A)
;-----------------------------------------------------------------------------
_seek:
    ld      b,a
    ld      a,ESPCMD_SEEK
    call    esp_cmd
    ld      a,b
    call    esp_send_byte   ; File descriptor

    ; a = (de << 7) & 0xFF
    ld      a,e
    rrca
    and     $80
    call    esp_send_byte

    ; a = (de >> 1) & 0xFF
    ld      a,d
    rra                     ; Shift lowest bit in B into carry
    ld      a,e
    rra
    call    esp_send_byte

    ; a = (de >> 9) & 0xFF
    ld      a,d
    rra
    and     $7F
    call    esp_send_byte

    xor     a
    call    esp_send_byte

    call    esp_get_byte
    ret

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
    call    esp_cmd
    ld      a,b

    ; Send file descriptor
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

;-----------------------------------------------------------------------------
; Send bytes
; Input:  HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;-----------------------------------------------------------------------------
_esp_send_bytes:
    push    de

.loop:
    ; Done sending? (DE=0)
    ld      a,d
    or      a,e
    jr      z,.done

    ld      a,(hl)
    call    esp_send_byte
    inc     hl
    dec     de
    jr      .loop

.done:
    pop     de
    ret

;-----------------------------------------------------------------------------
; Write bytes
; Input:  A: file descriptor
;         HL: source address
;         DE: number of bytes to write
; Output: HL: next address
;         DE: number of bytes actually written
;
; Clobbered registers: A, HL, DE
;-----------------------------------------------------------------------------
_esp_write_bytes:
    ld      b,a
    ld      a,ESPCMD_WRITE
    call    esp_cmd
    ld      a,b

    ; Send file descriptor
    call    esp_send_byte

    ; Send write size
    ld      a,e
    call    esp_send_byte
    ld      a,d
    call    esp_send_byte

    ; Send bytes
    call    _esp_send_bytes

    ; Get result
    call    esp_get_byte
    or      a
    ret     nz

    ; Get number of bytes actual written
    call    esp_get_byte
    ld      e,a
    call    esp_get_byte
    ld      d,a

    xor     a
    ret


    include "termemu.inc"
