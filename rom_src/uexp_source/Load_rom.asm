;---------------------------------------------------------
;                     Load ROM
;---------------------------------------------------------
;  For use with Aquarius MicroExpander only!
;
; - Catalog USB disk, list ROM files
; - User selects ROM file with keys 0-9, A-Z
; - Load ROM file to RAM at $8000-BFFF, reconfigure RAM as
;   ROM at $C000, reboot.

; 2017-04-16 V0.1 quit on ^C or RTN
; 2017 05-09 V0.2 using standard file requester
;                 load file into low RAM, then copy to $8000
; 2017-05-23 V0.3 clear ROM out of low mem after copying
; 2017-06-12 V1.0  bumped to release version

;simulate      = 1  ; don't activate ROM

_MAXFILES      = 36
_RAM           = $3A00        ; 16K bytes for array, ROM file

Load_ROM:
    LD      IX,RomWindow
    CALL    OpenWindow
    LD      HL,_RAM           ; array of file infos
    LD      B,_MAXFILES
    LD      DE,rompat         ; "*.ROM"
    CALL    RequestFile       ; select file, A = file number
    RET     NZ                ; quit if no file selected
.open_rom:
    ld      hl,_RAM
    CALL    IndexArray        ; HL = filename
    call    usb__open_read    ; open ROM file
    jr      z,.load_file
.nofile:
    ld      hl,_nofile_msg    ; can't open file
.show_error:
    ld      ix,FileErrorWindow
    call    OpenWindow
    call    WinPrtStr         ; show error message
    call    wait_key          ; wait for any key, then quit
    ret
.load_file:
    ld      hl,_RAM            ; load file into RAM
    ld      de,$4000           ; maximum file length is 16k Bytes
    call    usb__read_bytes    ; Read file into RAM
    push    af
    call    usb__close_file    ; close file
    pop     af
    ld      hl,_readerr_msg
    jr      nz,.show_error     ; quit if read error
    ld      hl,$4000
    or      a
    sbc     hl,de
    jr      z,.16k             ; if 16k - length = 0 then 16k ROM
    sbc     hl,de
    jr      z,.8k              ; if 16k - (length*2) <> 0 then error
    ld      hl,_badfile_msg
    jr      .show_error
.8k:
    ld      hl,_RAM
    ld      de,_RAM+$2000
    ld      bc,$2000
    ldir                       ; copy 8k ROM to upper half of 16k ROM
.16k:
    ld      hl,_RAM
    ld      de,$8000
    ld      bc,$4000
    ldir                       ; copy ROM to $8000-BFFF
    ld      hl,_RAM
    ld      de,_RAM+1
    ld      bc,$4000-1
    ld      (hl),0             ; remove ROM image from low RAM
    ldir
    ld      hl,.switcher
    ld      de,_RAM
    ld      bc,3
    ldir                       ; copy switcher code to RAM
    jp      _RAM               ; execute code from RAM

; ROM activation code. Must be in RAM because our ROM
; will disappear.
.switcher:
    out     ($00),a            ; configure upper 16k RAM as ROM at $C000
.error:
 ifdef simulate
    ret
 endif
    rst     0                  ; restart CPU, boot loaded ROM


rompat:
    db    "*.ROM",0
_nofile_msg:
    db CR,CR," Cannot open file",0
_readerr_msg:
    db CR,CR," Read error",0
_badfile_msg:
    db CR,CR,"Invalid ROM size (not 8k/16k)",0

;-------------------------------------------
;  Convert 11 char filename to name - extn
;-------------------------------------------
;
;  in: hl-> filename+space+extn 11 chars
; out: hl-> filename+null 1-8 chars + null
;
TrimName:
    push   bc
    push   hl
    ld     b,8
_tn_getchr:
    ld     a,(hl)
    cp     " "        ; " " = end of name
    jr     z,_tn_done
    inc    hl
_tn_next:
    djnz   _tn_getchr ; 8 chars = end of name
_tn_done:
    ld     (hl),0     ; null end of name
    pop    hl
    pop    bc
    ret

RomWindow:
      db   (1<<WA_BORDER)|(1<<WA_TITLE)|(1<<WA_CENTER) ; attributes
      db   WHITE*16+BLACK               ; text colors
      db   BLACK*16+RED                 ; border colors
      db   1,2,38,22                    ; x,y,w,h
      dw   rom_title                    ; title
rom_title:
      db   " ROM Loader ",0


FileErrorWindow:
      db   (1<<WA_BORDER)|(1<<WA_TITLE) ; attributes
      db   WHITE*16+DKBLUE              ; text colors
      db   BLACK*16+MAGENTA             ; border colors
      db   6,4,31,6                     ; x,y,w,h
      dw   file_error_title             ; title
file_error_title:
      db   " File Error ",0


