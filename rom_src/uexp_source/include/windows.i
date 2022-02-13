; window structure
 STRUCTURE window,0
      BYTE win_flags    ; window attributes
      BYTE win_color    ; text color (foreground*16+background)
      BYTE win_bcolor   ; border color
      BYTE win_x        ; x position (column)
      BYTE win_y        ; y position (line)
      BYTE win_w        ; width of interior
      BYTE win_h        ; height of interior
      WORD win_title    ; pointer to title string
 ENDSTRUCT window

; window flag bits
WA_BORDER = 0           ; window has a border
WA_TITLE  = 1           ; window has a title
WA_CENTER = 2           ; title is centered
WA_SCROLL = 3           ; scroll enabled

