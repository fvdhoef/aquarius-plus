;-----------------------------------------------------------------------------
; crt0.asm
;-----------------------------------------------------------------------------
    .module crt0

;-----------------------------------------------------------------------------
; Entry point
;-----------------------------------------------------------------------------
    .area   _HEADER (ABS)
	.org    0x100
    .globl  _main
_entry_start::
    call    gsinit      ; Initialize global variables
    call    _main       ; Call main
    jp      _exit
_entry_end::

;-----------------------------------------------------------------------------
; Ordering of segments for the linker.
;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
; _clock()
;-----------------------------------------------------------------------------
__clock::
    ret
  
;-----------------------------------------------------------------------------
; exit()
;-----------------------------------------------------------------------------
_exit::
    jp      0
  
;-----------------------------------------------------------------------------
; Initialization
;-----------------------------------------------------------------------------
    .area   _GSINIT
    .globl  l__INITIALIZER
    .globl  l__DATA
    .globl  s__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__DATA
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
