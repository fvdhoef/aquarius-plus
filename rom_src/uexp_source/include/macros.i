; start a structure definition
; eg. STRUCTURE mystruct,0
STRUCTURE MACRO name,offset
`name`_offset EQU offset
count     = offset
          ENDM

; allocate 1 byte in structure
; eg. BYTE char1
BYTE      MACRO name
name      EQU   count
count     = count+1
          ENDM

; allocate 2 bytes in structure
; eg. WORD int1
WORD      MACRO name
name      EQU count
count     = count+2
          ENDM

; allocate 3 bytes in structure
; typically used to define jump table addresses
; eg. VECT my_code
VECTOR    MACRO name
name      EQU count
count     = count+3
          ENDM

; allocate 4 bytes in structure
; eg. LONG longint1
LONG      MACRO name
name      EQU count
count     = count+4
          ENDM

; allocate multiple bytes
; typically used to embed a structure inside another
; eg. STRUCT filename,11
STRUCT    MACRO name,size
name      EQU   count
count     = count+size
          ENDM

; finish defining structure
; eg. ENDSTRUCT mystruct
ENDSTRUCT MACRO name
`name`.size EQU count-`name`_offset
          ENDM

