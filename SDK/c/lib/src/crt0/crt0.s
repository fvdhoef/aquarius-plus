    .module crt0
    .globl  _main

    .globl  l__INITIALIZER
    .globl  l__DATA
    .globl  s__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__DATA

    .area   _HEADER (ABS)

    ; CAQ header and BASIC stub
	.org    0x38E1
	.db     0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00
    .ascii  "AQPLUS"
	.db     0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00
    .db     0x0E,0x39,0x0A,0x00,0xDA
    .ascii  "14608:"
    .db     0x80,0x00,0x00,0x00

    ; Stack at the top of memory.
    ;  ld  sp,#0xffff        
    ;  I will use the Basic stack, so the program can return to basic!
    push    hl

    ; Initialise global variables
    call    gsinit
    call    _main

    pop     hl
    jp      _exit

    ; Ordering of segments for the linker.
    .area   _HOME
    .area   _CODE
    .area	_INITIALIZER
    .area   _GSINIT
    .area   _GSFINAL
    .area	_INITIALIZED
    .area   _DATA
    .area	_BSEG
    .area   _BSS
    .area   _HEAP

    .area   _CODE
__clock::
    ret
  
_exit::
    ret
  
    .area   _GSINIT
gsinit::
    ; Default-initialized global variables.
    ld      bc, #l__DATA
    ld      a, b
    or      a, c
    jr      z, zeroed_data
    ld      hl, #s__DATA
    ld      (hl), #0x00
    dec     bc
    ld      a, b
    or      a, c
    jr      z, zeroed_data
    ld      e, l
    ld      d, h
    inc     de
    ldir
zeroed_data:

	; Explicitly initialized global variables.
	ld      bc, #l__INITIALIZER
	ld      a, b
	or      a, c
	jr      z, gsinit_next
	ld      de, #s__INITIALIZED
	ld      hl, #s__INITIALIZER
	ldir

gsinit_next:

    .area   _GSFINAL
    ret
