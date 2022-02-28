; Based on V1.0 of micro-expander ROM
; Based on code for the MZ800 by Michal Huc�k http://www.8bit.8u.cz

; I/O ports
CH376_DATA_PORT:         equ     $40     ; change this to match your hardware!
CH376_CONTROL_PORT:      equ     CH376_DATA_PORT+1 ; A0 = high

; commands
CH376_CMD_CHECK_EXIST:   equ     $06     ; check if file exists
CH376_CMD_SET_FILE_SIZE: equ     $0D     ; set file size
CH376_CMD_SET_USB_MODE:  equ     $15     ; set USB mode
CH376_CMD_GET_STATUS:    equ     $22     ; get status
CH376_CMD_RD_USB_DATA:   equ     $27     ; read data from USB
CH376_CMD_WR_REQ_DATA:   equ     $2D     ; write data to USB
CH376_CMD_SET_FILE_NAME: equ     $2F     ; set name of file to open, read etc.
CH376_CMD_DISK_MOUNT:    equ     $31     ; mount disk
CH376_CMD_FILE_OPEN:     equ     $32     ; open file
CH376_CMD_FILE_ENUM_GO:  equ     $33     ; get next file info
CH376_CMD_FILE_CREATE:   equ     $34     ; create new file
CH376_CMD_FILE_ERASE:    equ     $35     ; delete file
CH376_CMD_FILE_CLOSE:    equ     $36     ; close opened file
CH376_CMD_BYTE_LOCATE:   equ     $39     ; seek into file
CH376_CMD_BYTE_READ:     equ     $3A     ; start reading bytes
CH376_CMD_BYTE_RD_GO:    equ     $3B     ; continue reading bytes
CH376_CMD_BYTE_WRITE:    equ     $3C     ; start writing bytes
CH376_CMD_BYTE_WR_GO:    equ     $3D     ; continue writing bytes

; status codes
CH376_INT_SUCCESS:       equ     $14     ; command executed OK
CH376_INT_DISK_READ:     equ     $1D     ; read again (more bytes to read)
CH376_INT_DISK_WRITE:    equ     $1E     ; write again (more bytes to write)
CH376_ERR_OPEN_DIR:      equ     $41     ; is directory, not file
CH376_ERR_MISS_FILE:     equ     $42     ; file not found

ATTR_DIRECTORY:          equ $10
ATTR_B_DIRECTORY:        equ 4

usb_init:
    call    usb__check_exists   ; CH376 present?
    jr      nz, .no_ch376
    call    usb__set_usb_mode   ; yes, set USB mode
.no_ch376:
    call    usb__root           ; root directory
    ret


;-----------------------------------------------------------------------------
; Create root path
;-----------------------------------------------------------------------------
usb__root:
    ld   a, '/'
    ld   (PathName), a
    xor  a
    ld   (PathName + 1), a
    ret

;-----------------------------------------------------------------------------
; Open all subdirectory levels in path
;    in: PathName = path eg. "/",0
;                            "/subdir1/subdir2/subdir3",0
;   out:     z = OK
;           nz = failed to open directory, a = error code
;-----------------------------------------------------------------------------
usb__open_path:
    push   hl
    call   usb__ready               ; check for USB drive
    jr     nz,.done                 ; abort if no drive
    ld     hl,PathName
    ld     a,CH376_CMD_SET_FILE_NAME
    out    (CH376_CONTROL_PORT),a   ; command: set file name (root dir)
    ld     a,'/'
    jr     .start                   ; start with '/' (root dir)
.next_level:
    ld     a,(hl)
    or     a                        ; if NULL then end of path
    jr     z,.done
    ld     a,CH376_CMD_SET_FILE_NAME
    out    (CH376_CONTROL_PORT),a   ; command: set file name (subdirectory)
.send_name:
    inc    hl
    ld     a,(hl)                   ; get next char of directory name
    cp     "/"
    jr     z,.open_dir
    or     a                        ; terminate name on '/' or NULL
    jr     z,.open_dir
    call   to_upper                ; convert 'a-z' to 'A-Z'
.start:
    out    (CH376_DATA_PORT),a      ; send char to CH376
    jr     .send_name               ; next char
.open_dir:
    xor    a
    out    (CH376_DATA_PORT),a      ; send NULL char (end of name)
    ld     a,CH376_CMD_FILE_OPEN
    out    (CH376_CONTROL_PORT),a   ; command: open file/directory
    call   usb__wait_int
    cp     CH376_ERR_OPEN_DIR       ; opened directory?
    jr     z,.next_level            ; yes, do next level.  no, error
.done:
    pop    hl
    ret

;-----------------------------------------------------------------------------
; Open file for writing
; If file doesn't exist then creates and opens new file.
; If file does exist then opens it and sets size to 1.
;
; WARNING: overwrites existing file!
;
; Input:    hl = filename
;
; Output:    z = success
;           nz = fail, a = error code
;-----------------------------------------------------------------------------
usb__open_write:
    call    usb__open_read          ; try to open existing file
    jr      z,.file_exists
    cp      CH376_ERR_MISS_FILE     ; error = file missing?
    ret     nz                      ; no, some other error so abort
    ld      a,CH376_CMD_FILE_CREATE
    out     (CH376_CONTROL_PORT),a  ; command: create new file
    jp      usb__wait_int           ; and return

    ; file exists, set size to 1 byte (forgets existing data in file)
.file_exists:
    ld      a,CH376_CMD_SET_FILE_SIZE
    out     (CH376_CONTROL_PORT),a  ; command: set file size
    ld      a,$68
    out     (CH376_DATA_PORT),a     ; select file size variable in CH376
    ld      a,1
    out     (CH376_DATA_PORT),a     ; file size = 1
    xor     a
    out     (CH376_DATA_PORT),a
    out     (CH376_DATA_PORT),a     ; zero out higher bytes of file size
    out     (CH376_DATA_PORT),a
    ret

;-----------------------------------------------------------------------------
; Write bytes from memory to open file
;   in: hl = address of source data
;       de = number of bytes to write
;
;  out: z if successful
;       hl = next address
;-----------------------------------------------------------------------------
usb__write_bytes:
    push    bc
    ld      a,CH376_CMD_BYTE_WRITE
    out     (CH376_CONTROL_PORT),a     ; send command 'byte write'
    ld      c,CH376_DATA_PORT
    out     (c),e                      ; send data length lower byte
    out     (c),d                      ; send data length upper byte
.loop:
    call    usb__wait_int              ; wait for response
    jr      z, .write_bytes_done       ; return z if finished writing
    cp      CH376_INT_DISK_WRITE       ; more bytes to write?
    ret     nz                         ; no, error so return nz
    ld      a,CH376_CMD_WR_REQ_DATA
    out     (CH376_CONTROL_PORT),a     ; send command 'write request'
    in      b,(c)                      ; b = number of bytes requested
    jr      z,.next                    ; skip if no bytes to transfer
    otir                               ; output data (1-255 bytes)
.next:
    ld      a,CH376_CMD_BYTE_WR_GO
    out     (CH376_CONTROL_PORT),a     ; send command 'write go'
    jr      .loop                      ; do next transfer
.write_bytes_done:
    pop     bc
    ret

;-----------------------------------------------------------------------------
; Write byte in a to File
;  in: a = Byte
;
; out: z if successful
;-----------------------------------------------------------------------------
usb__write_byte:
    push    bc
    ld      b,a                        ; b = byte
    ld      a,CH376_CMD_BYTE_WRITE
    out     (CH376_CONTROL_PORT),a     ; send command 'byte write'
    ld      c,CH376_DATA_PORT
    ld      a,1
    out     (c),a                      ; send data length = 1 byte
    xor     a
    out     (c),a                      ; send data length upper byte

    call    usb__wait_int              ; wait for response
    cp      CH376_INT_DISK_WRITE
    jr      nz, .end                   ; return error if not requesting byte
    ld      a,CH376_CMD_WR_REQ_DATA
    out     (CH376_CONTROL_PORT),a     ; send command 'write request'
    in      a,(c)                      ; a = number of bytes requested
    cp      1
    jr      nz, .end                   ; return error if no byte requested
    out     (c),b                      ; send the byte

    ld      a,CH376_CMD_BYTE_WR_GO
    out     (CH376_CONTROL_PORT),a     ; send command 'write go' (flush buffers)
    call    usb__wait_int              ; wait until command executed

.end:
    pop     bc
    ret

;-----------------------------------------------------------------------------
; Close file
;-----------------------------------------------------------------------------
usb__close_file:
    ld      a,CH376_CMD_FILE_CLOSE
    out     (CH376_CONTROL_PORT),a
    ld      a,1
    out     (CH376_DATA_PORT),a
    jp      usb__wait_int

;-----------------------------------------------------------------------------
; Open a File or Directory
; Input:  hl = filename (null-terminated)
; Output: z = OK
;         nz = fail, a = error code
;                         $1D (INT_DISK_READ) too many subdirectories
;                         $41 (ERR_OPEN_DIR) 'filename'is a directory
;                         $42 (CH376_ERR_MISS_FILE) file not found
;-----------------------------------------------------------------------------
usb__open_read:
    push    hl                      ; save filename pointer
    call    usb__open_path          ; enter current directory
    pop     hl                      ; restore filename pointer
    ret     nz
    call    usb__set_filename       ; send filename to CH376
    ret     nz                      ; abort if error
    ld      a,CH376_CMD_FILE_OPEN
    out     (CH376_CONTROL_PORT),a  ; command: open file
    jp      usb__wait_int

;-----------------------------------------------------------------------------
; Set file name
;  Input:  hl = filename
; Output:   z = OK
;          nz = error, a = error code
;-----------------------------------------------------------------------------
usb__set_filename:
    push    hl
    call    usb__ready              ; check for USB drive
    jr      nz,.set_filename_done   ; abort if error
    ld      a,CH376_CMD_SET_FILE_NAME
    out     (CH376_CONTROL_PORT),a  ; command: set file name
.set_filename_send_name:
    ld      a,(hl)
.send_char:
    out     (CH376_DATA_PORT),a     ; send filename char to CH376
    inc     hl                      ; next char
    or      a
    jr      nz,.set_filename_send_name   ; until end of name
.set_filename_done:
    pop     hl
    ret

;-----------------------------------------------------------------------------
; Read bytes from file into RAM
; Input:  hl = destination address
;         de = number of bytes to read
; Output: hl = next address (start address if no bytes read)
;         de = number of bytes actually read
;          z = successful read
;         nz = error reading file
;          a = status code
;-----------------------------------------------------------------------------
usb__read_bytes:
    push    bc
    push    hl
    ld      a,CH376_CMD_BYTE_READ
    out     (CH376_CONTROL_PORT),a  ; command: read bytes
    ld      c,CH376_DATA_PORT
    out     (c),e
    out     (c),d                   ; send number of bytes to read

usb_read_loop:
    call    usb__wait_int           ; wait until command executed
    ld      e,a                     ; e = status
    ld      a,CH376_CMD_RD_USB_DATA
    out     (CH376_CONTROL_PORT),a  ; command: read USB data
    in      b,(c)                   ; b = number of bytes in this block
    jr      z,usb_read_next         ; number of bytes > 0?
    inir                            ; yes, read data block into RAM

usb_read_next:
    ld      a,e
    cp      CH376_INT_SUCCESS       ; file read success?
    jr      z,usb_read_end          ; yes, return
    cp      CH376_INT_DISK_READ     ; more bytes to read?
    jr      nz,usb_read_end         ; no, return
    ld      a,CH376_CMD_BYTE_RD_GO
    out     (CH376_CONTROL_PORT),a  ; command: read more bytes
    jr      usb_read_loop           ; loop back to read next block

usb_read_end:
    pop     de                      ; de = start address
    push    hl                      ; save hl = end address + 1
    or      a
    sbc     hl,de                   ; hl = end + 1 - start
    ex      de,hl                   ; de = number of bytes actually read
    pop     hl                      ; restore hl = end address + 1
    pop     bc
    cp      CH376_INT_SUCCESS
    ret

;-----------------------------------------------------------------------------
; Read 1 Byte from File into a
; Output:  z = successful read, byte returned in a
;         nz = error reading byte, a = status code
;-----------------------------------------------------------------------------
usb__read_byte:
    ld      a,CH376_CMD_BYTE_READ
    out     (CH376_CONTROL_PORT),a  ; command: read bytes
    ld      a,1
    out     (CH376_DATA_PORT),a     ; number of bytes to read = 1
    xor     a
    out     (CH376_DATA_PORT),a

usb_readbyte_loop:
    call    usb__wait_int           ; wait until command executed
    cp      CH376_INT_DISK_READ
    jr      nz,usb_readbyte_end     ; quit if no byte available
    ld      a,CH376_CMD_RD_USB_DATA
    out     (CH376_CONTROL_PORT),a  ; command: read USB data
    in      a,(CH376_DATA_PORT)     ; get number of bytes available
    cp      1
    jr      nz,usb_readbyte_end     ; if not 1 byte available then quit
    in      a,(CH376_DATA_PORT)     ; read byte into a
    push    af
    ld      a,CH376_CMD_BYTE_RD_GO
    out     (CH376_CONTROL_PORT),a  ; command: read more bytes (for next read)
    call    usb__wait_int           ; wait until command executed
    pop     af
    cp      a                       ; return z with byte in a

usb_readbyte_end:
    ret

;-----------------------------------------------------------------------------
; Seek Into Open File
; Input:  de = number of bytes to skip (max 65535 bytes)
;
; Output:  z = OK
;         nz = fail, a = error code
;-----------------------------------------------------------------------------
usb__seek:
    ld      a,CH376_CMD_BYTE_LOCATE
    out     (CH376_CONTROL_PORT),a  ; command: byte locate
    ld      a,e
    out     (CH376_DATA_PORT),a     ; send offset low byte
    ld      a,d
    out     (CH376_DATA_PORT),a     ;     ''     high byte
    xor     a
    out     (CH376_DATA_PORT),a     ; zero bits 31-16
    out     (CH376_DATA_PORT),a

    ; falls into usb__wait_int

;-----------------------------------------------------------------------------
; Wait for interrupt and read status
; output:  z = success
;         nz = fail, a = error code
;-----------------------------------------------------------------------------
usb__wait_int:
    push    bc
    ld      bc,0                    ; wait counter = 65536
.wait_int_loop:
    in      a,(CH376_CONTROL_PORT)  ; command: read status register
    rla                             ; interrupt bit set?
    jr      NC,.wait_int_end        ; yes,
    dec     bc                      ; no, counter-1
    ld      a,b
    or      c
    jr      nz,.wait_int_loop       ; loop until timeout
.wait_int_end:
    ld      a,CH376_CMD_GET_STATUS
    out     (CH376_CONTROL_PORT),a  ; command: get status
    nop
    in      a,(CH376_DATA_PORT)     ; read status byte
    cp      CH376_INT_SUCCESS       ; test return code
    pop     bc
    ret

;---------------------------------------------------------------------
; Check if CH376 Exists
;  out: z = CH376 exists
;      nz = not detected, a = error code 1 (no CH376)
;---------------------------------------------------------------------
usb__check_exists:
    ld      b, 10
.retry:
    ld      a, CH376_CMD_CHECK_EXIST
    out     (CH376_CONTROL_PORT), a ; command: check CH376 exists
    ld      a, $1A
    out     (CH376_DATA_PORT), a    ; send test byte
    ex      (sp), hl
    ex      (sp), hl                ; delay ~10us
    in      a, (CH376_DATA_PORT)
    cp      $E5                     ; byte inverted?
    ret     z
    djnz    .retry
    ld      a, 1                    ; error code = no CH376
    or      a                       ; nz
    ret

;---------------------------------------------------------------------
; Set USB Mode
;  out: z = OK
;      nz = failed to enter USB mode, a = error code 2 (no USB)
;---------------------------------------------------------------------
usb__set_usb_mode:
    ld      b, 10
.set_mode_retry:
    ld      a, CH376_CMD_SET_USB_MODE
    out     (CH376_CONTROL_PORT), a ; command: set USB mode
    ld      a, 6
    out     (CH376_DATA_PORT), a    ; mode 6
    ex      (sp), hl
    ex      (sp), hl
    ex      (sp), hl                ; delay ~20us
    ex      (sp), hl
    in      a, (CH376_DATA_PORT)
    cp      $51                     ; status = $51?
    ret     z
    djnz    .set_mode_retry
    ld      a, 2                    ; error code 2 = no USB
    or      a                       ; nz
    ret

;-----------------------------------------------------------------------------
; Is USB drive Ready to access?
; Check for presense of CH376 and USB drive.
; If so then mount drive.
;
; Output:  z = OK
;         nz = error, a = error code
;                          1 = no CH376
;                          2 = no USB
;                          3 = no disk (mount failure)
;-----------------------------------------------------------------------------
usb__ready:
    push    bc
    ld      a,(PathName)
    cp      '/'                     ; if no path then set to '/',0
    call    nz,usb__root
    call    usb__check_exists       ; CH376 hardware present?
    jr      nz,.ready_done
    ld      b,3                     ; retry count for mount
    ld      c,1                     ; c = flag, 1 = before set_usb_mode
.mount:
    ld      b,5
.mountloop:
    call    usb__mount              ; try to mount disk
    jr      z,.ready_done           ; return OK if mounted
    call    usb__root               ; may be different disk so reset path
    djnz    .mountloop

    ; mount failed,
    dec     c                       ; already tried set_usb_mode ?
    jr      nz,.ready_done          ; yes, fail
    call    usb__set_usb_mode       ; put CH376 into USB mode
    jr      z,.mount                ; if successful then try to mount disk
.ready_done:
    pop     bc
    ret

;-----------------------------------------------------------------------------
; Mount USB Disk
; output:  z = mounted
;         nz = not mounted
;          a = CH376 interrupt code
;-----------------------------------------------------------------------------
usb__mount:
    ld      a,CH376_CMD_DISK_MOUNT
    out     (CH376_CONTROL_PORT),a  ; command: mount disk
    jp      usb__wait_int           ; wait until done

_usb_star: db "*", 0
