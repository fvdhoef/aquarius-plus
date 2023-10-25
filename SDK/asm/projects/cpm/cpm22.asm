;
; CP/M version 2.2
; Reconstructed from memory image on February 27, 1981 by Clark A. Calkins
;

; Interesting links:
; - https://obsolescence.wixsite.com/obsolescence/cpm-internals
; - http://48k.ca/zmac.html
; - http://cpmarchives.classiccmp.org
; - http://www.retroarchive.org/cpm/

; Set memory limit here. This is the amount of contiguous
; ram starting from 0000. CP/M will reside at the end of this space.
MEM:    equ     62              ; for a 62k system
IOBYTE: equ     3               ; i/o definition byte.
TDRIVE: equ     4               ; current drive name and user number.
ENTRY:  equ     5               ; entry point for the cp/m bdos.
TFCB:   equ     $5C             ; default file control block.
TBUFF:  equ     $80             ; i/o buffer and command line storage.
TBASE:  equ     $100            ; transient program storage area.

; Set control character equates.
CTRL_C: equ     $03             ; control-c
CTRL_E: equ     $05             ; control-e
BS:     equ     $08             ; backspace
TAB:    equ     $09             ; tab
LF:     equ     $0A             ; line feed
FF:     equ     $0C             ; form feed
CR:     equ     $0D             ; carriage return
CTRL_P: equ     $10             ; control-p
CTRL_R: equ     $12             ; control-r
CTRL_S: equ     $13             ; control-s
CTRL_U: equ     $15             ; control-u
CTRL_X: equ     $18             ; control-x
CTRL_Z: equ     $1A             ; control-z (end-of-file mark)
DEL:    equ     $7F             ; rubout

    ; Set origin for CP/M
    org     (MEM - 7) * 1024

    assert $ == $DC00

ccp:
    include "ccp.inc"

    assert $ == $E400

; Note that the following six bytes must match those at
; (PATTRN1) or cp/m will HALT. Why?
PATTRN2:    defb    0,22,0,0,0,0    ; (* serial number bytes *).

bdos:
    include "bdos.inc"

    assert $ <= $F200
    ds $F200 - $

bios:
    include "bios.inc"
