;=======================================================
;                  File Requester
;=======================================================
;
; Read Directory into array supplied by caller. show
; file list, user selects file with keys 0-9,a-z.
;
; Shows only files matching wildcard filter. Hides
; file extension if wildcard is "*.xxx".
;
; Enter subdir by selecting it, return to previous level
; with key '0'. RTN = quit without selecting file.
;
; Returns file number 1-36 in A. 0 = no file. Caller
; can index array to get filename, attributes, size.
;
; NOTE:
;  Maximum 36 files/dirs displayed in directory (but no
;  restiction on number of files not matching wildcard).
;
; 2017-04-22 V0.02 hide wildcard "*."
;                  moved DotName to dos.asm
; 2017-04-24 V0.03 FileSelector
; 2017-05-09 V0.04 Adjusted help message position
;                  Changed name to FileRequester
; 2017-06-11 V0.05 trim spaces off directory name
;                  directories first then files
; 2017-06-12 V1.0  bumped to release version
;
; ------------------  Functions --------------------
;
;RequestFile: show file list, user selects file.
;  in:  IX = window
;       HL = filename array
;       DE = wildcard
;        A = number of elements in array (35 max!)
;
; out:   C = number of files in list
;        Z = file selected, A = file number
;       NZ = no file selected, A = key pressed
;
;
;ListFiles: DIR disk and collect filenames in array
;     in: IX = window
;         HL = array
;         DE = wildcard
;          A = number of elements in array (35 max!)
;
;    out:  C = number of files shown
;          Z = a file was selected, A = file number 0-35
;         NZ = no file selected, A = key pressed
;
;
;ShowList: Show list of filenames
;     in: IX = window
;         HL = array
;         DE = wildcard
;
;    out:  A = number of files shown
;
;
;SelectFile: key 0-9,A-Z = file number 0-35
;     in:  A = number of files in list (36 max.!)
;
;    out:  Z = file selected, A = file number
;         NZ = no file selected, A = key pressed
;
;
;IndexArray: point to filename in array
;     in: HL = array of filenames (16 bytes per name)
;          A = index 0-35 (max)
;
;    out: HL = filename in array
;

;-------------------------------------------------------
;                   File Requester
;-------------------------------------------------------
;  Show file list, user selects file.
;
;  in:  IX = window
;       HL = filename array
;       DE = wildcard
;        A = number of elements in array (36 max!)
;
; out:   Z = file selected, A = file number, C = number of files listed
;       NZ = no file selected, A = key pressed (0 = no files listed)
;
; uses: BC
;
RequestFile:
.list_files:
        PUSH DE
        PUSH HL
        CALL ListFiles         ; show file list
        LD   C,A               ; C = number of files listed
        CP   1
        JR   C,.done           ; quit c, nz if no files
        CALL SelectFile        ; user selects file in list
        JR   NZ,.done          ; quit if no file selected (A = key pressed)
        CALL IndexArray
        LD   DE,11
        EX   DE,HL
        ADD  HL,DE
        BIT  ATTR_B_DIRECTORY,(HL) ; directory?
        EX   DE,HL
        JR   Z,.done           ; if not directory then done
        LD   D,H
        LD   E,L
        CALL dos__name         ; convert FAT name to DOS name (eg. "dir.extn")
        CALL strlen
        CALL dos__set_path     ; add directory to path
        POP  HL
        POP  DE
        JR   .list_files
.done:
        POP  HL
        POP  DE
        RET

;-------------------------------------------------------
;      Dir disk and List files matching Wildcard
;-------------------------------------------------------
;   in: ix = window
;       hl = filename array
;       de = wildcard string
;
;  out: A = number of files found
;
ListFiles:
    push   hl                      ; push array
    push   de                      ; push wildcard
    call   clearwindow
    ld     de,256*7+10             ; cursor x,y
    call   WinSetCursor
    call   WinPrtMsg
    db     "Reading Directory...",0
    pop    de                      ; pop wildcard
    ld     b,36                    ; up to 36 files in list
    call   usb__dir                ; dir disk to filename array
    pop    hl                      ; pop array
    jr     nz,.disk_error          ; abort if disk error
    ld     b,c
    xor    a                       ; directories first, then files
    call   usb__sort               ; sort filename array
    ld     a,c
    or     a                       ; any files to list?
    jp     nz,ShowList             ; yes, list files
.no_files:
    push   de                      ; push wildcard
    call   clearwindow
    ld     de,256*10+10            ; cursor x,y
    call   WinSetCursor
    ld     hl,.no_msg              ; show "no"
    call   WinPrtStr
    pop    hl                      ; pop wildcard to HL
    ld     a,(hl)
    or     a
    jr     z,.show_filesmsg
    cp     '*'                     ; skip "*"
    jr     nz,.show_wild
    inc    hl
    ld     a,(hl)
    cp     '.'                     ; skip "."
    jr     nz,.show_wild
    inc    hl
    ld     a,(hl)
    cp     '*'                     ; skip "*"
    jr     z,.show_filesmsg
.show_wild:
    call   WinPrtStr               ; print wild card string
.show_filesmsg:
    ld     hl,.files_msg
    jr     .abort                  ; "files found"
.disk_error:
    call   clearwindow
    ld     de,256*14+10            ; cursor x,y
    call   WinSetCursor
    ld     hl,.disk_msg            ; "disk error"
.abort:
    call   WinPrtStr               ; print final message
    call   wait_key
    xor    a                       ; A = 0 (no files)
    ret

.no_msg:
    db     "No ",0
.files_msg:
    db     " files found",0
.disk_msg:
    db     "Disk error",0


;----------------------------------------------------
;             List names in Array
;----------------------------------------------------
;  in: IX = window
;      HL = filename array
;       A = number of names to list
;
; out:  A = number of names listed
;
ShowList:
    ld     c,a
    or     a
    ret    z                      ; abort if no names!
    push   hl
    call   clearwindow
    ld     b,0                    ; B = current file number
.display_next:
    ld     d,3                    ; x = left side of screen
    ld     e,b                    ; y = file number
    ld     a,c
    cp     21                     ; more then 20 files?
    jr     c,.small
    ld     a,b
    cp     18                     ; if file number >=18  then
    jr     c,.setxy
    sub    18
    ld     e,a                    ;     y = file number -18
    ld     d,22                   ;     x = right side of screen
    jr     .setxy
.small:
    ld     a,b
    cp     10                     ; if file number >=10  then
    jr     c,.dblspace
    sub    10
    ld     e,a                    ;     y = file number -10
    ld     d,19                   ;     x = 19 (screen right)
.dblspace:
    sla    e                      ; y*2 = double line spacing
.setxy:
    inc    e
    call   WinSetCursor           ; cursor to x,y
    ld     a,b
    add    "0"
    cp     "0"+10
    jr     c,.prt_key             ; convert number to ASCII key 0-9,A-Z
    add    7
.prt_key:
    call   WinPrtChr              ; print key
    push   hl                     ; push file array pointer
    ld     de,11
    ex     de,hl
    add    hl,de                  ; HL = attribute byte
    bit    ATTR_B_DIRECTORY,(HL)  ; directory?
    ex     de,hl
    jr     z,.showfile            ; no, show file name
    ld     a,'<'
    call  WinPrtChr
    push   bc
    ld     b,11                   ; B = number of chars to print
.showdir:
    ld     a,(hl)
    inc    hl
    cp     ' '
    jr     z,.dirend
    call   WinPrtChr              ; show dirname char
    djnz   .showdir
.dirend:
    ld     a,'>'
    call   WinPrtChr
    pop    bc
    jr     .printed
.showfile:
    ld     a," "
    call   WinPrtChr              ; print ' '
    ld     a,8
    call   WinPrtChrs             ; print filename without extn
.printed:
    pop    hl                     ; pop file array pointer
    ld     de,16
    add    hl,de                  ; next filename
    inc    b
    ld     a,b                    ; until all names printed
    cp     c
    jr     nz,.display_next
    ld     de,256*2+21            ; cursor x,y
    call   WinSetCursor
    ld     hl,_fs_help
    call   WinPrtStr              ; show key help message
    pop    hl
    ld     a,c                    ; A = number of names listed
    ret

;----------------------------------------------------
;              Select File from List
;----------------------------------------------------
;
; - File is labeled with digits 0-9, letters A-Z. user
;   presses digit/letter corresponding to chosen file.
;
; - any key below "0" exits without choosing file.
;
;  in:  A = number of files in list (36 max.!)
;
; out:  z = file selected, A = file number (0-35)
;      nz = no file selected, A = key pressed (eg. RTN, CTRL-C)
;
SelectFile:
    ld     b,a
.waitkey:
    call   Key_Check             ; wait for key press (no click)
    jr     z,.waitkey
    call   UpperCase
    cp     "0"
    jr     c,.done               ; ret NZ if key < "0"
    sub    "0"
    cp     10                    ; convert 0-9 to number
    jr     c,.gotkey
    cp     17                    ; ignore keys between 9 and A
    jr     c,.waitkey
    sub    7                     ; convert A-Z to number
    cp     b
    jr     nc,.waitkey           ; ignore keys higher than those on list
.gotkey:
    cp     a                     ; z
.done:
    ret

;------------------------------------------------------------
;            Point to filename in array
;------------------------------------------------------------
;
;  in: HL = array of filenames (16 bytes per name)
;       A = index number
; out: HL = filename in array
;
IndexArray:
   push   de
   ld     e,a
   ld     d,0                   ; DE = file number
   ex     de,hl
   add    hl,hl
   add    hl,hl
   add    hl,hl                 ; DE = file number * 16
   add    hl,hl
   ex     de,hl
   add    hl,de                 ; HL = array[number]
   pop    de
   ret

_fs_help:
   db    "0-Z = select file      RTN = quit",0

