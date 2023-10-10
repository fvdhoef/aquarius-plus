
;
;**************************************************************
;*
;*        B I O S   J U M P   T A B L E
;*
;**************************************************************
;
BOOT:   JP      _boot
WBOOT:  JP      _wboot
CONST:  JP      _const
CONIN:  JP      _conin
CONOUT: JP      _conout
LIST:   JP      _list
PUNCH:  JP      _punch
READER: JP      _reader
HOME:   JP      _home
SELDSK: JP      _seldsk
SETTRK: JP      _settrk
SETSEC: JP      _setsec
SETDMA: JP      _setdma
READ:   JP      _read
WRITE:  JP      _write
PRSTAT: JP      _prstat
SECTRN: JP      _sectrn

_memset:
    inc     c
    inc     b
    jr      .start
.repeat:
    ld      (hl), a
    inc     hl
.start:
    dec     c
    jr      nz, .repeat
    dec     b
    jr      nz, .repeat
    ret

_boot:
    ; Setup a stack
    ld      sp, $0

    ; Clear user memory
    ld      hl, $0
    ld      bc, $DC00
    ld      a, 0
    call    _memset

    ; Setup low storage area
    ld  de, $0
    ld  hl, __low_storage_values
    ld  bc, __low_storage_values_end - __low_storage_values
    ldir

.loop:
    jr      .loop



__low_storage_values:
    jp      $F200   ; Jump to BIOS
    db      0       ; IO byte
    db      0       ; DSK byte (current default drive)
    jp      $E400   ; Jump to BDOS
__low_storage_values_end:



_wboot:      jr _wboot
_const:      jr _const
_conin:      jr _conin
_conout:     jr _conout
_list:       jr _list
_punch:      jr _punch
_reader:     jr _reader
_home:       jr _home
_seldsk:     jr _seldsk
_settrk:     jr _settrk
_setsec:     jr _setsec
_setdma:     jr _setdma
_read:       jr _read
_write:      jr _write
_prstat:     jr _prstat
_sectrn:     jr _sectrn
