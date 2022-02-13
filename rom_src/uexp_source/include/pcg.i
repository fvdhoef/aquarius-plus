;====================================================================
;                      PCG Definitions
;====================================================================
; 2016-02-12 started
;
;-------------------------------------------------------------------
;                         Screen RAM
;-------------------------------------------------------------------
; 1k for characters followed by 1k for color attributes.
; 1000 visible characters on screen, leaving 24 unused bytes at end.
; First character in screen also sets the border character and color.
; The first row (40 bytes), and first & last columns of each row are
; normally filled with spaces, giving an effective character matrix
; of 38 columns x 24 rows.

CHRRAM   = $3000 ; 12288           start of character RAM
;          $33E7 ; 13287           end of character RAM
                 ;                 24 unused bytes
COLRAM   = $3400 ; 13312           Start of colour RAM
;          $37E7 ; 14311           end of color RAM
                 ;                 24 unused bytes

;               ---------------------------
;                    CONTROL REGISTERS
;               ---------------------------
;
PCG_UNLOCK  = $37F8 ; 14328  unlock
PCG_LOCK    = $37F9 ; 14329  lock
PCG_MODE    = $37FA ; 14330  set display mode 0 = normal, 1 = 320x200
PCG_DBANK   = $37FB ; 14331  set display bank 0-7
PCG_WBANK   = $37FC ; 14332  set write bank 0-7
PCG_CHRSET  = $37FD ; 14333  load character set 0-63
PCG_CHAR    = $37FE ; 14334  select character 0-255 to write
PCG_PATT    = $37FF ; 14335  write pattern to character
;