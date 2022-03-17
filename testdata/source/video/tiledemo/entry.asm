
	org $38E1

    ; Header and BASIC stub
	db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db "AQPLUS"
	db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp main
