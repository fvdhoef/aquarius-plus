; #asm
    include "/sdk/asm/inc/basic_stub.inc"
main:
    jp      _main
; #endasm


; #include "inc.h"

; // Test include file

; // Hello there

; /* Multi line
;          comment */

; #asm
label:
    jr      label
; #endasm

; // 0x4387
; // 1234
; // '\\'123
; // "Hello world!"

; int a = 80;
_a:
    defw 80
; char b = 'A';
_b:
    defb 65

; dinges() {
_dinges:
; //    return 6;
; }
.func_exit:
    ret

; main(    ) {
_main:
;     dinges();
    call    _dinges
;     a = 5 + (2 * 3);
; }
.func_exit:
    ret
