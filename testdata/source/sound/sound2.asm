; PROJECT: SOUND2
; ASM COMPATIBILITY: TASM (e.g., TASM -80 -b sound2.asm sound2.caq)

; James' Cassette Compatible Init (CLOAD > RUN)
LOADER:

	.org $38E1 
	.db	$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF 
	.db	$00 

	; Can replace with custom SIX character identifier
	.db	"SOUND2"
	
	.db	$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF 
	.db	$00 
	.db	$25,$39,$0a,$00 
	.db	$8e 

	; Do not edit this line
	.db	" For Aquarius S2 (do not edit)" 

	.db	00 
	.db	$2d, $39, $14, $00 
	.db	$42, $B0, $30 
	.db	00 
	.db	$49, $39, $1e, $00 
	.db	$94, $20, "14340", $2C, "088" 
	.db	$3A 
	.db	$94, $20, "14341", $2C, "057" 
	.db	00 
	.db	(TERMINATE & 255) 
	.db	(TERMINATE >> 8) 
	.db	$28, $00 
	.db	$42, $B0,$B5,$28,$30,$29,$3A,$80 
	.db	$00 
	.db	$00,$00	
	
	; Play sounds
	call	PLAYSOUNDS
	
; Start Main Program
MAIN: 

	; Add Custom Code Here - Press RETURN to Exit
	call	INPUT
	jr		nz,	MAIN
	
	; Clear Screen and Exit
	call	$1e45
	
; Basic Input Handler
INPUT:

	; Short Pause
	ld		bc,$3000
	call	$1d4b

	; Check for RETURN key
	ld		bc, $feff
	in		a,(c)
	bit		3, a
	
	ret

; Wait 500ms
WAITASEC:
	ld		bc,$BD74
	call	$1d4b
	ret

; Play sounds
PLAYSOUNDS:

	; Start fresh
	; CH-ABC0 off
	ld  a,7
	out ($F7),a
	ld  a,63
	out ($F6),a
	; CH-ABC1 off
	ld  a,7
	out ($F9),a
	ld  a,63
	out ($F8),a
	; C2 in CH-A0
	ld	a,0
	out ($F7),a
	ld	a,45
	out ($F6),a
	ld  a,1
	out ($F7),a
	ld  a,4
	out ($F6),a
	; E2 in CH-B0
	ld	a,2
	out ($F7),a
	ld	a,80
	out ($F6),a
	ld  a,3
	out ($F7),a
	ld  a,3
	out ($F6),a
	; G2 in CH-C0
	ld  a,4
	out ($F7),a
	ld	a,201
	out ($F6),a
	ld  a,5
	out ($F7),a
	ld  a,2
	out ($F6),a
	; Third octave notes
	; C3 in CH-A1
	ld	a,0
	out ($F9),a
	ld	a,22
	out ($F8),a
	ld  a,1
	out ($F9),a
	ld  a,2
	out ($F8),a
	; E3 in CH-B1
	ld	a,2
	out ($F9),a
	ld	a,168
	out ($F8),a
	ld  a,3
	out ($F9),a
	ld  a,1
	out ($F8),a
	; G3 in CH-C1
	ld  a,4
	out ($F9),a
	ld	a,101
	out ($F8),a
	ld  a,5
	out ($F9),a
	ld  a,1
	out ($F8),a
	; CH-ABC0 to Volume 15
	ld  a,8
	out ($F7),a
	ld  a,15
	out ($F6),a
	ld  a,9
	out ($F7),a
	ld  a,15
	out ($F6),a
	ld  a,10
	out ($F7),a
	ld  a,15
	out ($F6),a
	; CH-ABC1 to Volume 15
	ld  a,8
	out ($F9),a
	ld  a,15
	out ($F8),a
	ld  a,9
	out ($F9),a
	ld  a,15
	out ($F8),a
	ld  a,10
	out ($F9),a
	ld  a,15
	out ($F8),a
	; Play the notes with a pause between
	; CH-A0 on
	ld  a,7
	out ($F7),a
	ld  a,62
	out ($F6),a
	call WAITASEC
	; CH-AB0 on
	ld  a,7
	out ($F7),a
	ld  a,60
	out ($F6),a
	call WAITASEC
	; CH-ABC0 on
	ld  a,7
	out ($F7),a
	ld  a,56
	out ($F6),a
	call WAITASEC
	; Third octave
	; CH-A1 on
	ld  a,7
	out ($F9),a
	ld  a,62
	out ($F8),a
	call WAITASEC
	; CH-AB1 on
	ld  a,7
	out ($F9),a
	ld  a,60
	out ($F8),a
	call WAITASEC
	; CH-ABC1 on
	ld  a,7
	out ($F9),a
	ld  a,56
	out ($F8),a
	call WAITASEC
	call WAITASEC
	call WAITASEC
	call WAITASEC
	; CH-ABC0 off
	ld  a,7
	out ($F7),a
	ld  a,63
	out ($F6),a
	; CH-ABC1 off
	ld  a,7
	out ($F9),a
	ld  a,63
	out ($F8),a
	; CH-ABC0 to Volume 0
	ld  a,8
	out ($F7),a
	ld  a,0
	out ($F6),a
	ld  a,9
	out ($F7),a
	ld  a,0
	out ($F6),a
	ld  a,10
	out ($F7),a
	ld  a,0
	out ($F6),a
	; CH-ABC1 to Volume 0
	ld  a,8
	out ($F9),a
	ld  a,0
	out ($F8),a
	ld  a,9
	out ($F9),a
	ld  a,0
	out ($F8),a
	ld  a,10
	out ($F9),a
	ld  a,0
	out ($F8),a
	ret

; Conclude ML Routine
TERMINATE:
	
	.db $00,$00,$00,$00,$00,$00,$00,$00 
	.db $00,$00,$00,$00,$00,$00,$00,$00 
	.db $00,$00,$00,$00,$00,$00,$00,$00 
	.end
