;******************************************************************************
;                      CH376 USB Driver for Z80 CPU
;******************************************************************************
; Based on code for the MZ800 by Michal Hucík http://www.8bit.8u.cz
;
;   Date      Ver                     Changes                         Author
; 2015-10-4  V0.00 started                                         Bruce Abbott
; 2015-10-24 V0.01 it works! (reading binary files only)
; 2015-10-25 V0.02 usb_read_bytes returns end address + 1 in HL, length in DE
; 2015-11-11 V0.03 usb_dir returns NZ if no disk, wildcard filter
; 2015-12-11 V0.04 increased mount timeout
; 2016-01-01 V0.05 write_bytes
; 2016-01-10 V0.06 add '/' to begining of filename
;                  add usb_seek
; 2016-01-12 V0.07 read_byte
; 2016-01-18 V0.08 add usb_delete, usb_close_file, usb_set_filename
; 2016-02-19 V0.09 bugfix: usb_file_exist sometimes failed when file existed!
; 2016-04-07 V0.10 changed I/O address to suit micro-expander
; 2017-03-03       added documentation for read byte, write byte
; 2017-03-08 V0.11 usb_dir fills array with directory info
;                  usb_wild_card (moved to here from dos.asm)
; 2017-04-07 V0.12 usb_set_path
;                  usb_dir now returns CH376 interrupt code
; 2017-04-10 V0.13 usb_MSDOS convert filename char to MSDOS compatible
; 2017-04-20 V0.14 check exists, set usb mode.
; 2017-04-23 V0.15 usb_get_path
; 2017-04-24 V0.16 usb_open_path
; 2017-05-01 V0.17 usb_delete opens path before deleting file
; 2017-06-08 V0.18 sort_dir: subdirectories before files
; 2017-06-12 V1.0  bumped to release version
;
; usb_sort:
;   sort filename array
;   input:  A = sort options  0 = directories before files only (no other sorting)
;           C = number of filenames in array
;
; usb_ready:
;   check for disk, mount if present
;   output:  Z = OK
;           NZ = error, A = error code (1 = no CH376, 2 = no disk, 3 = mount fail)
;
; usb_open_read:
;   open file or directory for reading
;   input:  HL = address of filename (null-terminated string)
;   output:  Z = OK
;           NZ = fail, A = error code
;
; usb_read_byte:
;    read byte from open file
;    Input: none
;   Output:  Z = successful read, byte returned in A
;           NZ = fail, A = error code
;
; usb_read_bytes:
;    read data from open file and store in RAM
;    Input: HL = address where data will be stored
;           DE = number of bytes to read (-1 = load any file up to 65535 bytes)
;   output: HL = actual end address + 1
;           DE = number of bytes actually read
;            Z = OK
;           NZ = fail, A = error code
;
; usb_open_write:
;   open file for writing. create new file if none exists.
;   input:  HL = address of filename (null-terminated string)
;   output:  Z = OK
;           NZ = fail, A = error code
;
; usb_write_byte:
;    write byte to file (opened with usb_open_write)
;    Input:  A = byte
;   Output:  Z = successful write
;           NZ = fail, A = error code
;
; usb_write_bytes:
;    write data from RAM to file opened for writing
;    Input: HL = address of data in RAM
;           DE = number of bytes to write
;   output:  Z = OK
;           NZ = fail, A = error code
;
; usb_seek:
;    seek into open file
;    input: DE = byte offset from beginning of file (0-65535)
;   output:  Z = OK
;           NZ = fail, A = error code
;
; usb_delete:
;    delete file
;   input:  HL = filename
;  output:   Z = OK
;           NZ = fail, A = error code
;
; usb_open_dir:
;    open current directory  (if not found then open root directory)
;  out: z = directory opened
;      nz = error
;
; usb_dir:
;   read directory into array, 16 bytes per entry (DIR_filename:11, DIR_attr:1, DIR_filesize:4)
;   input:  HL = address of array
;            B = number of elements in array
;           DE = wildcard pattern ("*" for all files)
;   output:  C = number of matching files found
;            Z = OK
;           NZ = fail, A = error code
;
; usb_wildcard:
;   pattern match filename against wildcard pattern
;   input:  HL-> filename 11 chars (spaces bwteen name and extn)
;           DE-> pattern (null-terminated string)
;  output:   Z = match
;           NZ = no match
;
; usb_set_filename:
;   send filename to CH376
;    input: HL = filename
;   output:  A = CH376 interrupt code
;
;
; I/O ports
CH376_DATA_PORT         equ     $40     ; change this to match your hardware!
CH376_CONTROL_PORT      equ     CH376_DATA_PORT+1 ; A0 = high

; commands
CH376_CMD_SET_USB_SPEED equ     $04     ; set USB device speed (send 0 for 12Mbps, 2 for 1.5Mbps)
CH376_CMD_CHECK_EXIST   equ     $06     ; check if file exists
CH376_CMD_SET_FILE_SIZE equ     $0D     ; set file size
CH376_CMD_SET_USB_MODE  equ     $15     ; set USB mode
CH376_CMD_GET_STATUS    equ     $22     ; get status
CH376_CMD_RD_USB_DATA   equ     $27     ; read data from USB
CH376_CMD_WR_REQ_DATA   equ     $2D     ; write data to USB
CH376_CMD_SET_FILE_NAME equ     $2F     ; set name of file to open, read etc.
CH376_CMD_DISK_CONNECT  equ     $30     ; check if USB drive is plugged in
CH376_CMD_DISK_MOUNT    equ     $31     ; mount disk
CH376_CMD_FILE_OPEN     equ     $32     ; open file
CH376_CMD_FILE_ENUM_GO  equ     $33     ; get next file info
CH376_CMD_FILE_CREATE   equ     $34     ; create new file
CH376_CMD_FILE_ERASE    equ     $35     ; delete file
CH376_CMD_FILE_CLOSE    equ     $36     ; close opened file
CH376_CMD_BYTE_LOCATE   equ     $39     ; seek into file
CH376_CMD_BYTE_READ     equ     $3A     ; start reading bytes
CH376_CMD_BYTE_RD_GO    equ     $3B     ; continue reading bytes
CH376_CMD_BYTE_WRITE    equ     $3C     ; start writing bytes
CH376_CMD_BYTE_WR_GO    equ     $3D     ; continue writing bytes
; status codes
CH376_INT_SUCCESS       equ     $14     ; command executed OK
CH376_INT_DISK_READ     equ     $1D     ; read again (more bytes to read)
CH376_INT_DISK_WRITE    equ     $1E     ; write again (more bytes to write)
CH376_ERR_OPEN_DIR      equ     $41     ; is directory, not file
CH376_ERR_MISS_FILE     equ     $42     ; file not found

 structure FAT_DIR_INFO,0
    STRUCT DIR_Name,11          ; $00 0
     BYTE  DIR_Attr             ; $0B 11
     BYTE  DIR_NTRes            ; $0C 12
     BYTE  DIR_CrtTimeTenth     ; $0D 13
     WORD  DIR_CrtTime          ; $0E 14
     WORD  DIR_CrtDate          ; $10 16
     WORD  DIR_LstAccDate       ; $12 18
     WORD  DIR_FstClusHI        ; $14 20
     WORD  DIR_WrtTime          ; $16 22
     WORD  DIR_WrtDate          ; $18 24
     WORD  DIR_FstClusLO        ; $1A 26
     LONG  DIR_FileSize         ; $1C 28
 endstruct FAT_DIR_INFO         ; $20 32

; attribute masks
ATTR_READ_ONLY      EQU $01
ATTR_HIDDEN         EQU $02
ATTR_SYSTEM         EQU $04
ATTR_VOLUME_ID      EQU $08
ATTR_DIRECTORY      EQU $10
ATTR_ARCHIVE        EQU $20
ATTR_LONG_NAME      EQU (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
ATTR_LONG_NAME_MASK EQU (ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_ARCHIVE)
; attribute bits
ATTR_B_READ_ONLY    EQU 0
ATTR_B_HIDDEN       EQU 1
ATTR_B_SYSTEM       EQU 2
ATTR_B_VOLUME_ID    EQU 3
ATTR_B_DIRECTORY    EQU 4
ATTR_B_ARCHIVE      EQU 5


;------------------------------------------------------------------------------
;     Get Pointer to Path string
;------------------------------------------------------------------------------
;
usb__get_path:
        LD     HL,PathName
        RET

;------------------------------------------------------------------------------
;     create root path
;------------------------------------------------------------------------------
usb__root:
        LD   A,'/'
        LD   (PathName),A
        XOR  A
        LD   (PathName+1),A
        RET

;--------------------------------------------------------------
;            Open all subdirectory levels in path
;--------------------------------------------------------------
;    in: PathName = path eg. "/",0
;                            "/subdir1/subdir2/subdir3",0
;   out:     Z = OK
;           NZ = failed to open directory, A = error code
;
usb__open_path:
        PUSH   HL
        CALL   usb__ready               ; check for USB drive
        JR     NZ,.done                 ; abort if no drive
        LD     HL,PathName
        LD     A,CH376_CMD_SET_FILE_NAME
        OUT    (CH376_CONTROL_PORT),A   ; command: set file name (root dir)
        LD     A,'/'
        JR     .start                   ; start with '/' (root dir)
.next_level:
        LD     A,(HL)
        OR     A                        ; if NULL then end of path
        JR     Z,.done
        LD     A,CH376_CMD_SET_FILE_NAME
        OUT    (CH376_CONTROL_PORT),A   ; command: set file name (subdirectory)
.send_name:
        INC    HL
        LD     A,(HL)                   ; get next char of directory name
        CP     "/"
        JR     Z,.open_dir
        OR     A                        ; terminate name on '/' or NULL
        JR     Z,.open_dir
        CALL   UpperCase                ; convert 'a-z' to 'A-Z'
.start  OUT    (CH376_DATA_PORT),A      ; send char to CH376
        JR     .send_name               ; next char
.open_dir:
        XOR    A
        OUT    (CH376_DATA_PORT),A      ; send NULL char (end of name)
        LD     A,CH376_CMD_FILE_OPEN
        OUT    (CH376_CONTROL_PORT),A   ; command: open file/directory
        CALL   usb__wait_int
        CP     CH376_ERR_OPEN_DIR       ; opened directory?
        JR     Z,.next_level            ; yes, do next level.  no, error
.done:  POP    HL
        RET


;-----------------------------------------------------
;           Open Current Directory
;-----------------------------------------------------
;  out: z = directory opened
;      nz = error
;
; if current directory won't open then reset to root.
;
usb__open_dir:
    ld      hl,_usb_star
    CALL    usb__open_read          ; open "*" = request all files in current directory
    CP      CH376_INT_DISK_READ     ; opened directory?
    RET     Z                       ; yes, ret z
    CP      CH376_ERR_MISS_FILE     ; no, directory missing?
    RET     NZ                      ; no, quit with disk error
    Call    usb__root               ; yes, set path to root
    ld      hl,_usb_star
    CALL    usb__open_read          ; try to open root directory
    CP      CH376_INT_DISK_READ
    RET                             ; z = OK, nz = error


;------------------------------------------------------------------------------
;                      Test if File Exists
;------------------------------------------------------------------------------
; Input:    HL = filename
;
; Output:    Z = file exists
;           NZ = file not exist or is directory, A = error code
;
usb__file_exist:
        CALL    usb__open_read          ; try to open file
        JR      Z,.close
        CP      CH376_ERR_OPEN_DIR      ; error, file is directory?
        JR      NZ,.done                ; no, quit
.close: PUSH    AF
        CALL    usb__close_file         ; close file
        POP     AF
.done:  CP      CH376_INT_SUCCESS       ; Z if file exists, else NZ
        RET

;------------------------------------------------------------------------------
;                        Open File for Writing
;------------------------------------------------------------------------------
; If file doesn't exist then creates and opens new file.
; If file does exist then opens it and sets size to 1.
;
; WARNING: overwrites existing file!
;
; Input:    HL = filename
;
; Output:    Z = success
;           NZ = fail, A = error code
;
usb__open_write:
        CALL    usb__open_read          ; try to open existing file
        JR      Z,.file_exists
        CP      CH376_ERR_MISS_FILE     ; error = file missing?
        RET     NZ                      ; no, some other error so abort
        LD      A,CH376_CMD_FILE_CREATE
        OUT     (CH376_CONTROL_PORT),A  ; command: create new file
        JP      usb__wait_int           ; and return
; file exists, set size to 1 byte (forgets existing data in file)
.file_exists:
        LD      A,CH376_CMD_SET_FILE_SIZE
        OUT     (CH376_CONTROL_PORT),A  ; command: set file size
        LD      A,$68
        OUT     (CH376_DATA_PORT),A     ; select file size variable in CH376
        LD      A,1
        OUT     (CH376_DATA_PORT),A     ; file size = 1
        XOR     A
        OUT     (CH376_DATA_PORT),A
        OUT     (CH376_DATA_PORT),A     ; zero out higher bytes of file size
        OUT     (CH376_DATA_PORT),A
        RET

;------------------------------------------------------------------------------
;    Write Bytes from Memory to open File
;------------------------------------------------------------------------------
;   in: HL = address of source data
;       DE = number of bytes to write
;
;  out: Z if successful
;       HL = next address
;
usb__write_bytes:
        PUSH    BC
        LD      A,CH376_CMD_BYTE_WRITE
        OUT     (CH376_CONTROL_PORT),A     ; send command 'byte write'
        LD      C,CH376_DATA_PORT
        OUT     (C),E                      ; send data length lower byte
        OUT     (C),D                      ; send data length upper byte
.loop:  CALL    usb__wait_int              ; wait for response
        JR      Z,.done                    ; return Z if finished writing
        CP      CH376_INT_DISK_WRITE       ; more bytes to write?
        RET     NZ                         ; no, error so return NZ
        LD      A,CH376_CMD_WR_REQ_DATA
        OUT     (CH376_CONTROL_PORT),A     ; send command 'write request'
        IN      B,(C)                      ; B = number of bytes requested
        JR      Z,.next                    ; skip if no bytes to transfer
        OTIR                               ; output data (1-255 bytes)
.next:  LD      A,CH376_CMD_BYTE_WR_GO
        OUT     (CH376_CONTROL_PORT),A     ; send command 'write go'
        JR      .loop                      ; do next transfer
.done:  POP     BC
        RET

;------------------------------------------------------------------------------
;    Write Byte in A to File
;------------------------------------------------------------------------------
;  in: A = Byte
;
; out: Z if successful
;
usb__write_byte:
        PUSH    BC
        LD      B,A                        ; B = byte
        LD      A,CH376_CMD_BYTE_WRITE
        OUT     (CH376_CONTROL_PORT),A     ; send command 'byte write'
        LD      C,CH376_DATA_PORT
        LD      A,1
        OUT     (C),A                      ; send data length = 1 byte
        XOR     A
        OUT     (C),A                      ; send data length upper byte
usb_write_byte_loop:
        CALL    usb__wait_int              ; wait for response
        CP      CH376_INT_DISK_WRITE
        JR      NZ,usb_write_byte_end      ; return error if not requesting byte
        LD      A,CH376_CMD_WR_REQ_DATA
        OUT     (CH376_CONTROL_PORT),A     ; send command 'write request'
        IN      A,(C)                      ; A = number of bytes requested
        CP      1
        JR      NZ,usb_write_byte_end      ; return error if no byte requested
        OUT     (C),B                      ; send the byte
usb_write_byte_go:
        LD      A,CH376_CMD_BYTE_WR_GO
        OUT     (CH376_CONTROL_PORT),A     ; send command 'write go' (flush buffers)
        CALL    usb__wait_int              ; wait until command executed
usb_write_byte_end:
        POP     BC
        RET

;--------------------------------------------------------------------
;                          Close File
;--------------------------------------------------------------------
;
usb__close_file:
        LD      A,CH376_CMD_FILE_CLOSE
        OUT     (CH376_CONTROL_PORT),A
        LD      A,1
        OUT     (CH376_DATA_PORT),A
        JP      usb__wait_int

;------------------------------------------------------------------------------
;                      Open a File or Directory
;------------------------------------------------------------------------------
; Input:   HL = filename (null-terminated)
;
; Output:   Z = OK
;          NZ = fail, A = error code
;                         $1D (INT_DISK_READ) too many subdirectories
;                         $41 (ERR_OPEN_DIR) 'filename'is a directory
;                         $42 (CH376_ERR_MISS_FILE) file not found
;
usb__open_read:
        PUSH    HL                      ; save filename pointer
        CALL    usb__open_path          ; enter current directory
        POP     HL                      ; restore filename pointer
        RET     NZ
        CALL    usb__set_filename       ; send filename to CH376
        RET     NZ                      ; abort if error
        LD      A,CH376_CMD_FILE_OPEN
        OUT     (CH376_CONTROL_PORT),A  ; command: open file
        JP      usb__wait_int

;------------------------------------------------------------------------------
;                       Set File Name
;------------------------------------------------------------------------------
;  Input:  HL = filename
; Output:   Z = OK
;          NZ = error, A = error code
;
usb__set_filename:
        PUSH    HL
        CALL    usb__ready              ; check for USB drive
        JR      NZ,.done                ; abort if error
        LD      A,CH376_CMD_SET_FILE_NAME
        OUT     (CH376_CONTROL_PORT), A ; command: set file name
.send_name:
        LD      A,(HL)
        CALL    dos__char               ; convert char to MSDOS equivalent
.send_char:
        OUT     (CH376_DATA_PORT),A     ; send filename char to CH376
        INC     HL                      ; next char
        OR      A
        JR      NZ,.send_name           ; until end of name
.done:  POP     HL
        RET


;------------------------------------------------------------------------------
;               Read Bytes from File into RAM
;------------------------------------------------------------------------------
; Input:  HL = destination address
;         DE = number of bytes to read
;
; Output: HL = next address (start address if no bytes read)
;         DE = number of bytes actually read
;          Z = successful read
;         NZ = error reading file
;          A = status code
;
usb__read_bytes:
        PUSH    BC
        PUSH    HL
        LD      A,CH376_CMD_BYTE_READ
        OUT     (CH376_CONTROL_PORT),A  ; command: read bytes
        LD      C,CH376_DATA_PORT
        OUT     (C),E
        OUT     (C),D                   ; send number of bytes to read
usb_read_loop:
        CALL    usb__wait_int           ; wait until command executed
        LD      E,A                     ; E = status
        LD      A,CH376_CMD_RD_USB_DATA
        OUT     (CH376_CONTROL_PORT),A  ; command: read USB data
        IN      B,(C)                   ; B = number of bytes in this block
        JR      Z,usb_read_next         ; number of bytes > 0?
        INIR                            ; yes, read data block into RAM
usb_read_next:
        LD      A,E
        CP      CH376_INT_SUCCESS       ; file read success?
        JR      Z,usb_read_end          ; yes, return
        CP      CH376_INT_DISK_READ     ; more bytes to read?
        JR      NZ,usb_read_end         ; no, return
        LD      A,CH376_CMD_BYTE_RD_GO
        OUT     (CH376_CONTROL_PORT),A  ; command: read more bytes
        JR      usb_read_loop           ; loop back to read next block
usb_read_end:
        POP     DE                      ; DE = start address
        PUSH    HL                      ; save HL = end address + 1
        OR      A
        SBC     HL,DE                   ; HL = end + 1 - start
        EX      DE,HL                   ; DE = number of bytes actually read
        POP     HL                      ; restore HL = end address + 1
        POP     BC
        CP      CH376_INT_SUCCESS
        RET


;------------------------------------------------------------------------------
;                   Read 1 Byte from File into A
;------------------------------------------------------------------------------
;
; Output:  Z = successful read, byte returned in A
;         NZ = error reading byte, A = status code
;
usb__read_byte:
        LD      A,CH376_CMD_BYTE_READ
        OUT     (CH376_CONTROL_PORT),A  ; command: read bytes
        LD      A,1
        OUT     (CH376_DATA_PORT),A     ; number of bytes to read = 1
        XOR     A
        OUT     (CH376_DATA_PORT),A
usb_readbyte_loop:
        CALL    usb__wait_int           ; wait until command executed
        CP      CH376_INT_DISK_READ
        JR      NZ,usb_readbyte_end     ; quit if no byte available
        LD      A,CH376_CMD_RD_USB_DATA
        OUT     (CH376_CONTROL_PORT),A  ; command: read USB data
        IN      A,(CH376_DATA_PORT)     ; get number of bytes available
        CP      1
        JR      NZ,usb_readbyte_end     ; if not 1 byte available then quit
        IN      A,(CH376_DATA_PORT)     ; read byte into A
        PUSH    AF
        LD      A,CH376_CMD_BYTE_RD_GO
        OUT     (CH376_CONTROL_PORT),A  ; command: read more bytes (for next read)
        CALL    usb__wait_int           ; wait until command executed
        POP     AF
        CP      A                       ; return Z with byte in A
usb_readbyte_end:
        RET

;------------------------------------------------------------------------------
;                        Delete File
;------------------------------------------------------------------------------
; Input:  HL = filename string
;
; Output:  Z = OK
;         NZ = fail, A = error code
;
usb__delete:
        CALL    usb__open_read
        RET     NZ
        LD      A,CH376_CMD_FILE_ERASE
        OUT     (CH376_CONTROL_PORT),A  ; command: erase file
        JR      usb__wait_int

;------------------------------------------------------------------------------
;                        Seek Into Open File
;------------------------------------------------------------------------------
; Input:  DE = number of bytes to skip (max 65535 bytes)
;
; Output:  Z = OK
;         NZ = fail, A = error code
;
usb__seek:
        LD      A,CH376_CMD_BYTE_LOCATE
        OUT     (CH376_CONTROL_PORT),A  ; command: byte locate
        LD      A,E
        OUT     (CH376_DATA_PORT),A     ; send offset low byte
        LD      A,D
        OUT     (CH376_DATA_PORT),A     ;     ''     high byte
        XOR     A
        OUT     (CH376_DATA_PORT),A     ; zero bits 31-16
        OUT     (CH376_DATA_PORT),A
; falls into...

;------------------------------------------------------------------------------
;                   Wait for Interrupt and Read Status
;------------------------------------------------------------------------------
; output:  Z = success
;         NZ = fail, A = error code
;
usb__wait_int:
        PUSH    BC
        LD      BC,0                    ; wait counter = 65536
.wait_int_loop:
        IN      A,(CH376_CONTROL_PORT)  ; command: read status register
        RLA                             ; interrupt bit set?
        JR      NC,.wait_int_end        ; yes,
        DEC     BC                      ; no, counter-1
        LD      A,B
        OR      C
        JR      NZ,.wait_int_loop       ; loop until timeout
     ifdef debug
        ld      a,"?"
        ld      ($3000),a
     endif
.wait_int_end:
        LD      A,CH376_CMD_GET_STATUS
        OUT     (CH376_CONTROL_PORT),A  ; command: get status
        NOP
        IN      A,(CH376_DATA_PORT)     ; read status byte
        CP      CH376_INT_SUCCESS       ; test return code
        POP     BC
        RET


;---------------------------------------------------------------------
;                     Check if CH376 Exists
;---------------------------------------------------------------------
;  out: Z = CH376 exists
;      NZ = not detected, A = error code 1 (no CH376)
;
usb__check_exists:
        LD      B,10
.retry: LD      A,CH376_CMD_CHECK_EXIST
        OUT     (CH376_CONTROL_PORT),A  ; command: check CH376 exists
        LD      A,$1A
        OUT     (CH376_DATA_PORT),A     ; send test byte
        EX      (SP),HL
        EX      (SP),HL                 ; delay ~10us
        IN      A,(CH376_DATA_PORT)
        CP      $E5                     ; byte inverted?
        RET     Z
  ifdef debug
        ld      a,"1"+10
        sub     b
        ld      ($3001),a
  endif
        DJNZ    .retry
        LD      A,1                     ; error code = no CH376
        OR      A                       ; NZ
        RET

;---------------------------------------------------------------------
;                         Set USB Mode
;---------------------------------------------------------------------
;  out: Z = OK
;      NZ = failed to enter USB mode, A = error code 2 (no USB)
;
usb__set_usb_mode:
        LD      B,10
.retry: LD      A,CH376_CMD_SET_USB_MODE
        OUT     (CH376_CONTROL_PORT),A  ; command: set USB mode
        LD      A,6
        OUT     (CH376_DATA_PORT),A     ; mode 6
        EX      (SP),HL
        EX      (SP),HL
        EX      (SP),HL                 ; delay ~20us
        EX      (SP),HL
        IN      A,(CH376_DATA_PORT)
        CP      $51                     ; status = $51?
        RET     Z
  ifdef debug
        ld      a,"1"+10
        sub     b
        ld      ($3002),a
  endif
        DJNZ    .retry
        LD      A,2                     ; error code 2 = no USB
        OR      A                       ; NZ
        RET


;-------------------------------------------------------------------
;               is USB drive Ready to access?
;-------------------------------------------------------------------
; Check for presense of CH376 and USB drive.
; If so then mount drive.
;
; Output:  Z = OK
;         NZ = error, A = error code
;                          1 = no CH376
;                          2 = no USB
;                          3 = no disk (mount failure)
;
usb__ready:
        PUSH    BC
        ld      a,(PathName)
        cp      '/'                     ; if no path then set to '/',0
        call    nz,usb__root
        call    usb__check_exists       ; CH376 hardware present?
        jr      nz,.done
        ld      b,3                     ; retry count for mount
        ld      c,1                     ; C = flag, 1 = before set_usb_mode
.mount:
        LD      B,5
.mountloop:
        CALL    usb__mount              ; try to mount disk
        JR      z,.done                 ; return OK if mounted
     ifdef debug
        ld      a,'5'
        sub     b
        ld      ($3003),a
     endif
        call    usb__root               ; may be different disk so reset path
        DJNZ    .mountloop
; mount failed,
     ifdef debug
        ld      a,"S"
        ld      ($3004),a
     endif
        DEC     C                       ; already tried set_usb_mode ?
        JR      NZ,.done                ; yes, fail
        call    usb__set_usb_mode       ; put CH376 into USB mode
        JR      Z,.mount                ; if successful then try to mount disk
.done:  POP     BC
        RET

;------------------------------------------------------------------------------
;                            Mount USB Disk
;------------------------------------------------------------------------------
; output:  Z = mounted
;         NZ = not mounted
;          A = CH376 interrupt code
;
usb__mount:
        LD      A,CH376_CMD_DISK_MOUNT
        OUT     (CH376_CONTROL_PORT),A  ; command: mount disk
        JP      usb__wait_int           ; wait until done


  STRUCTURE FileInfo,0
    STRUCT  FI_NAME,11
    BYTE    FI_ATTR
    LONG    FI_SIZE
  ENDSTRUCT FileInfo


;-----------------------------------------------------------------------------
;                  Get Disk Directory with Wildcard filter
;-----------------------------------------------------------------------------
;  in: HL = filename array (16 bytes per name)
;       B = number of elements in array
;      DE-> wildcard pattern
;
;  out: C = number of files that match wildcard
;       A = return code:-
;              CH376_ERR_MISS_FILE = got all files in directory
;              CH376_INT_DISK_READ = may be more files in directory
;              anything else       = disk error
;       Z = OK, NZ = disk error
;
usb__dir:
    PUSH    HL                      ; save array address
    LD      C,0                     ; C = count matching files
    LD      HL,_usb_star            ; filename = "*" (read directory)
    CALL    usb__open_read          ; open directory
    POP     HL
    PUSH    HL
    JR      .next_entry             ; start reading directory
.dir_loop:
    LD      A,CH376_CMD_RD_USB_DATA
    OUT     (CH376_CONTROL_PORT),A  ; command: read USB data (directory entry)
    IN      A,(CH376_DATA_PORT)     ; A = number of bytes in CH376 buffer (should be 32)
    OR      A                       ; if bytes = 0 then read next entry
    JR      Z,.next_entry
; read DIR_name, DIR_attr, DIR_filesize from FAT_DIR_INFO buffer in CH376
    PUSH    HL                      ; save element array pointer
    PUSH    BC                      ; save array size, file count
    LD      B,12                    ; B = 11 bytes filename, 1 byte file attributes
.read_name_attr:
    IN      A,(CH376_DATA_PORT)     ; get next filename char
    LD      (HL),A                  ; store it in array
    INC     HL
    DJNZ    .read_name_attr
    LD      B,32-12-4               ; B = bytes to absorb
    LD      C,A                     ; C = attributes
.absorb_bytes:
    IN      A,(CH376_DATA_PORT)     ; absorb bytes until filesize
    DJNZ    .absorb_bytes
    LD      B,4
.read_size:
    IN      A,(CH376_DATA_PORT)     ; get next size byte
    LD      (HL),A                  ; store filesize byte in array
    INC     HL
    DJNZ    .read_size
    BIT     ATTR_B_DIRECTORY,C      ; if subdirectory then don't filter it
    POP     BC                      ; restore array size, file count
    POP     HL                      ; restore array pointer
    JR      NZ,.subdir
    CALL    usb__wildcard           ; wildcard pattern matches file?
    JR      Z,.gotfile              ; yes,
.killname:
    LD      (HL),0                  ; no, kill filename
    JR      .read_next
.subdir:
    LD      A,(HL)                  ; get 1st char of filename
    CP      '.'                     ; directory name starts with '.'?
    JR      NZ,.gotfile             ; no,
    INC     HL
    LD      A,(HL)                  ; get 2nd char
    DEC     HL
    CP      '.'                     ; ".." ?
    JR      NZ,.killname            ; no, kill filename
    JR      .gotfile                ; yes, got directory ".."
.gotfile:
    PUSH    BC
    LD      BC,16
    ADD     HL,BC                   ; advance to next element in array
    POP     BC                      ; restore array size, count
    INC     C                       ; count+1
.read_next:
    LD      A,CH376_CMD_FILE_ENUM_GO
    OUT     (CH376_CONTROL_PORT),A  ; command: read next filename
    CALL    usb__wait_int           ; wait until done
.next_entry:
    CP      CH376_INT_DISK_READ     ; more files in directory?
    JR      NZ,.dir_end             ; no,
    LD      A,C
    CP      B                       ; yes, filename array full?
    JR      C,.dir_loop             ; no, get next entry
    CP      A
    JR      .done                   ; yes, ret Z = OK
.dir_end:
    CP      CH376_ERR_MISS_FILE     ; Z if got all files, else disk error
.done:
    POP     HL
    RET

_usb_star:
   db  "*",0


;---------------------------------------------------------------------
;                      sort directory array
;---------------------------------------------------------------------
;   in: HL = array
;        B = number of files in array
;        A = sort options    0 = directories before files
;                            1 = sort by name (not implemented!)
;                            2 = sort by size (not implemented!)
;
usb__sort:
    push  ix
    push  de
    push  bc
    push  hl
    pop   ix                     ; IX = array
    dec   b                      ; B = number of compares to do
    jr    z,.done                ; if no compares needed then done
    ld    c,0                    ; C  = no swaps
.sort:
    push  bc                     ; save number of files, swapflag
    push  ix                     ; save array address
.compare:
    ld    de,FileInfo.size       ; DE = size of array entry
    bit   ATTR_B_DIRECTORY,(ix+DIR_Attr)
    jr    nz,.skip               ; if dir then skip
    bit   ATTR_B_DIRECTORY,(ix+DIR_Attr+FileInfo.size)
    jr    nz,.swap               ; if next is dir then swap
.skip:
    add   ix,de                  ; else skip to next entry
    jr    .next
.swap:
    ld    a,(ix+0)
    ld    d,(ix+FileInfo.size)
    ld    (ix++0),d              ; current entry <-> next entry
    ld    (ix+FileInfo.size),a
    inc   ix
    dec   e                      ; next byte
    jr    nz,.swap
    set   0,c                    ; 1 or more swaps occurred
.next:
    djnz  .compare               ; compare next entries until end of list
.end:
    pop   ix                     ; restore array address
    bit   0,c                    ; any swaps?
    pop   bc                     ; restore number of files, swapflag
    jr    nz,.sort               ; if any swaps then continue sorting
.done:
    pop   bc
    pop   de
    pop   ix
    ret



;---------------------------------------------------------------------
;                     Wildcard Pattern Match
;---------------------------------------------------------------------
; Pattern match CH376 filename. Filename is 11 characters long, padded
; with spaces to 8 name characters plus 3 extension characters.
;
; example filenames:-     "NAME       "
;                         "NAME    TXT"
;                         "FILENAMETXT"
;
; pattern string is up to 12 characters long, not padded.
;
; example:- "F?LE*.TX?"
;
; pattern matching characters:-
; '?'  = match any single character
; '*'  = match all characters to end of name or extension
; '.'  = separator between name and extension
; NULL = match all files
;
;---------------------------------------------------------------------
; in:  HL = filename 11 chars (8 name + 3 extension)
;      DE = wildcard pattern string
;
;out:   Z = match, NZ = no match
;
usb__wildcard:
    push hl
    push de
    push bc
    ld   b,0              ; B = char position in filename
    ld   a,(de)           ; get 1st pattern char
    or   a
    jr   z,.wcd_done      ; if null string then done
    jr   .wcd_start
.wcd_next_pat:
    inc  de               ; next pattern
.wcd_next_char:
    inc  hl               ; next filename char
    inc  b
    ld   a,b
    cp   11
    jr   z,.wcd_done      ; if end of filename then it's a match
.wcd_get_pat:
    ld   a,(de)           ; a = pattern char
    or   a                ; end of pattern string?
    jr   z,.wcd_endpat
.wcd_start:
    cp   '*'              ; '*'  = match all chars
    jr   z,.wcd_star
    cp   '?'              ; '?'  = match any char at current position
    jr   z,.wcd_next_pat
    CP   '.'              ; '.'  = finish checking name and start extn
    jr   z,.wcd_dot
    cp   (hl)             ; else = compare pattern char to filename char
    jr   z,.wcd_next_pat  ; if match then test next char
    jr   .wcd_done        ; else return no match
;
; end of pattern, check that rest of filename is spaces
.wcd_endpat:
    ld   a,' '            ; <SPACE> in filename?
    cp   (hl)
    jr   z,.wcd_next_char ; yes, continue checking
    jr   .wcd_done        ; no, return no match
;
; '*' = match all chars to end of name or extn
.wcd_star:
    inc  de
    ld   a,(de)
    dec  de
    or   a                ; '*' is last char in pattern?
    jr   z,.wcd_done      ; yes, return match
    ld   a,b
    cp   7                ; at end of name?
    jr   z,.wcd_next_pat  ; yes, cancel '*' and do next pattern char
    jr   .wcd_next_char   ; else continue '*' with next filename char
;
; '.' = finish name part and start checking extn part
.wcd_dot:
    inc  de               ; point to next pattern char
    ld   a,(de)
.wcd_to_extn:
    ld   a,b
    cp   8                ; reached start of extn part?
    jr   z,.wcd_get_pat   ; if yes then start checking extn
    jr   nc,.wcd_done     ; if past start of extn then fail
; still in name so...
    ld   a,(hl)           ; get current name char
    inc  hl
    inc  b                ; advance to next name char
    cp   ' '
    jr   z,.wcd_to_extn   ; if <SPACE> then continue, else fail
;
; return Z = match, NZ = no match
.wcd_done:
    pop  bc
    pop  de
    pop  hl
    ret

