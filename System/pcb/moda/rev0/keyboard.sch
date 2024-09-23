(kicad_sch
	(version 20231120)
	(generator "eeschema")
	(generator_version "8.0")
	(uuid "fcc9ff58-5316-43d1-9ceb-32a90e974622")
	(paper "A4")
	(title_block
		(title "Keyboard")
		(date "2021-09-14")
		(rev "1")
		(company "Designed by Frank van den Hoef in 2021")
	)
	(lib_symbols
		(symbol "4xxx:4017"
			(pin_names
				(offset 1.016)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "U"
				(at -7.62 16.51 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "4017"
				(at -7.62 -19.05 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "http://www.intersil.com/content/dam/Intersil/documents/cd40/cd4017bms-22bms.pdf"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Johnson Counter ( 10 outputs )"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_locked" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "CNT CNT10"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "DIP?16*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "4017_1_0"
				(pin output line
					(at 12.7 0 180)
					(length 5.08)
					(name "Q5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 2.54 180)
					(length 5.08)
					(name "Q4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "10"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 -10.16 180)
					(length 5.08)
					(name "Q9"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "11"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 -15.24 180)
					(length 5.08)
					(name "Cout"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "12"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input inverted
					(at -12.7 10.16 0)
					(length 5.08)
					(name "CKEN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "13"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input clock
					(at -12.7 12.7 0)
					(length 5.08)
					(name "CLK"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "14"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 5.08 0)
					(length 5.08)
					(name "Reset"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "15"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 20.32 270)
					(length 5.08)
					(name "VDD"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "16"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 10.16 180)
					(length 5.08)
					(name "Q1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 12.7 180)
					(length 5.08)
					(name "Q0"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 7.62 180)
					(length 5.08)
					(name "Q2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 -2.54 180)
					(length 5.08)
					(name "Q6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 -5.08 180)
					(length 5.08)
					(name "Q7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 5.08 180)
					(length 5.08)
					(name "Q3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 -22.86 90)
					(length 5.08)
					(name "VSS"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 -7.62 180)
					(length 5.08)
					(name "Q8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "9"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(symbol "4017_1_1"
				(rectangle
					(start -7.62 15.24)
					(end 7.62 -17.78)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
			)
		)
		(symbol "74xx:74LS165"
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "U"
				(at -7.62 19.05 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "74LS165"
				(at -7.62 -21.59 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "https://www.ti.com/lit/ds/symlink/sn74ls165a.pdf"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Shift Register 8-bit, parallel load"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "TTL SR SR8"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "DIP?16* SO*16*3.9x9.9mm*P1.27mm* SSOP*16*5.3x6.2mm*P0.65mm* TSSOP*16*4.4x5mm*P0.65*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "74LS165_1_0"
				(pin input line
					(at -12.7 -10.16 0)
					(length 5.08)
					(name "~{PL}"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 15.24 0)
					(length 5.08)
					(name "DS"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "10"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 12.7 0)
					(length 5.08)
					(name "D0"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "11"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 10.16 0)
					(length 5.08)
					(name "D1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "12"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 7.62 0)
					(length 5.08)
					(name "D2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "13"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 5.08 0)
					(length 5.08)
					(name "D3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "14"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 -17.78 0)
					(length 5.08)
					(name "~{CE}"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "15"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 22.86 270)
					(length 5.08)
					(name "VCC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "16"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 -15.24 0)
					(length 5.08)
					(name "CP"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 2.54 0)
					(length 5.08)
					(name "D4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 0 0)
					(length 5.08)
					(name "D5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 -2.54 0)
					(length 5.08)
					(name "D6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 -5.08 0)
					(length 5.08)
					(name "D7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 12.7 180)
					(length 5.08)
					(name "~{Q7}"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 -25.4 90)
					(length 5.08)
					(name "GND"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 15.24 180)
					(length 5.08)
					(name "Q7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "9"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(symbol "74LS165_1_1"
				(rectangle
					(start -7.62 17.78)
					(end 7.62 -20.32)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
			)
		)
		(symbol "Connector_Generic:Conn_02x04_Odd_Even"
			(pin_names
				(offset 1.016) hide)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "J"
				(at 1.27 5.08 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "Conn_02x04_Odd_Even"
				(at 1.27 -7.62 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Generic connector, double row, 02x04, odd/even pin numbering scheme (row 1 odd numbers, row 2 even numbers), script generated (kicad-library-utils/schlib/autogen/connector/)"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "connector"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "Connector*:*_2x??_*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "Conn_02x04_Odd_Even_1_1"
				(rectangle
					(start -1.27 -4.953)
					(end 0 -5.207)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -2.413)
					(end 0 -2.667)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 0.127)
					(end 0 -0.127)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 2.667)
					(end 0 2.413)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 3.81)
					(end 3.81 -6.35)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
				(rectangle
					(start 3.81 -4.953)
					(end 2.54 -5.207)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start 3.81 -2.413)
					(end 2.54 -2.667)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start 3.81 0.127)
					(end 2.54 -0.127)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start 3.81 2.667)
					(end 2.54 2.413)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -5.08 2.54 0)
					(length 3.81)
					(name "Pin_1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 7.62 2.54 180)
					(length 3.81)
					(name "Pin_2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 0 0)
					(length 3.81)
					(name "Pin_3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 7.62 0 180)
					(length 3.81)
					(name "Pin_4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -2.54 0)
					(length 3.81)
					(name "Pin_5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 7.62 -2.54 180)
					(length 3.81)
					(name "Pin_6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -5.08 0)
					(length 3.81)
					(name "Pin_7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 7.62 -5.08 180)
					(length 3.81)
					(name "Pin_8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
		(symbol "Device:C"
			(pin_numbers hide)
			(pin_names
				(offset 0.254)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "C"
				(at 0.635 2.54 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "C"
				(at 0.635 -2.54 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" ""
				(at 0.9652 -3.81 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Unpolarized capacitor"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "cap capacitor"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "C_*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "C_0_1"
				(polyline
					(pts
						(xy -2.032 -0.762) (xy 2.032 -0.762)
					)
					(stroke
						(width 0.508)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -2.032 0.762) (xy 2.032 0.762)
					)
					(stroke
						(width 0.508)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "C_1_1"
				(pin passive line
					(at 0 3.81 270)
					(length 2.794)
					(name "~"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 0 -3.81 90)
					(length 2.794)
					(name "~"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
		(symbol "Device:D_Small"
			(pin_numbers hide)
			(pin_names
				(offset 0.254) hide)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "D"
				(at -1.27 2.032 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "D_Small"
				(at -3.81 -2.032 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" ""
				(at 0 0 90)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 90)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Diode, small symbol"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Sim.Device" "D"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Sim.Pins" "1=K 2=A"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "diode"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "TO-???* *_Diode_* *SingleDiode* D_*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "D_Small_0_1"
				(polyline
					(pts
						(xy -0.762 -1.016) (xy -0.762 1.016)
					)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -0.762 0) (xy 0.762 0)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0.762 -1.016) (xy -0.762 0) (xy 0.762 1.016) (xy 0.762 -1.016)
					)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "D_Small_1_1"
				(pin passive line
					(at -2.54 0 0)
					(length 1.778)
					(name "K"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 2.54 0 180)
					(length 1.778)
					(name "A"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
		(symbol "Device:R"
			(pin_numbers hide)
			(pin_names
				(offset 0)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "R"
				(at 2.032 0 90)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "R"
				(at 0 0 90)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at -1.778 0 90)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Resistor"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "R res resistor"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "R_*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "R_0_1"
				(rectangle
					(start -1.016 -2.54)
					(end 1.016 2.54)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "R_1_1"
				(pin passive line
					(at 0 3.81 270)
					(length 1.27)
					(name "~"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 0 -3.81 90)
					(length 1.27)
					(name "~"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
		(symbol "Graphic:Logo_Open_Hardware_Small"
			(exclude_from_sim no)
			(in_bom no)
			(on_board no)
			(property "Reference" "#SYM"
				(at 0 6.985 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Value" "Logo_Open_Hardware_Small"
				(at 0 -5.715 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Open Hardware logo, small"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Sim.Enable" "0"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "Logo"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "Logo_Open_Hardware_Small_0_1"
				(polyline
					(pts
						(xy 3.3528 -4.3434) (xy 3.302 -4.318) (xy 3.175 -4.2418) (xy 2.9972 -4.1148) (xy 2.7686 -3.9624)
						(xy 2.54 -3.81) (xy 2.3622 -3.7084) (xy 2.2352 -3.6068) (xy 2.1844 -3.5814) (xy 2.159 -3.6068)
						(xy 2.0574 -3.6576) (xy 1.905 -3.7338) (xy 1.8034 -3.7846) (xy 1.6764 -3.8354) (xy 1.6002 -3.8354)
						(xy 1.6002 -3.8354) (xy 1.5494 -3.7338) (xy 1.4732 -3.5306) (xy 1.3462 -3.302) (xy 1.2446 -3.0226)
						(xy 1.1176 -2.7178) (xy 0.9652 -2.413) (xy 0.8636 -2.1082) (xy 0.7366 -1.8288) (xy 0.6604 -1.6256)
						(xy 0.6096 -1.4732) (xy 0.5842 -1.397) (xy 0.5842 -1.397) (xy 0.6604 -1.3208) (xy 0.7874 -1.2446)
						(xy 1.0414 -1.016) (xy 1.2954 -0.6858) (xy 1.4478 -0.3302) (xy 1.524 0.0762) (xy 1.4732 0.4572)
						(xy 1.3208 0.8128) (xy 1.0668 1.143) (xy 0.762 1.3716) (xy 0.4064 1.524) (xy 0 1.5748) (xy -0.381 1.5494)
						(xy -0.7366 1.397) (xy -1.0668 1.143) (xy -1.2192 0.9906) (xy -1.397 0.6604) (xy -1.524 0.3048)
						(xy -1.524 0.2286) (xy -1.4986 -0.1778) (xy -1.397 -0.5334) (xy -1.1938 -0.8636) (xy -0.9144 -1.143)
						(xy -0.8636 -1.1684) (xy -0.7366 -1.27) (xy -0.635 -1.3462) (xy -0.5842 -1.397) (xy -1.0668 -2.5908)
						(xy -1.143 -2.794) (xy -1.2954 -3.1242) (xy -1.397 -3.4036) (xy -1.4986 -3.6322) (xy -1.5748 -3.7846)
						(xy -1.6002 -3.8354) (xy -1.6002 -3.8354) (xy -1.651 -3.8354) (xy -1.7272 -3.81) (xy -1.905 -3.7338)
						(xy -2.0066 -3.683) (xy -2.1336 -3.6068) (xy -2.2098 -3.5814) (xy -2.2606 -3.6068) (xy -2.3622 -3.683)
						(xy -2.54 -3.81) (xy -2.7686 -3.9624) (xy -2.9718 -4.0894) (xy -3.1496 -4.2164) (xy -3.302 -4.318)
						(xy -3.3528 -4.3434) (xy -3.3782 -4.3434) (xy -3.429 -4.318) (xy -3.5306 -4.2164) (xy -3.7084 -4.064)
						(xy -3.937 -3.8354) (xy -3.9624 -3.81) (xy -4.1656 -3.6068) (xy -4.318 -3.4544) (xy -4.4196 -3.3274)
						(xy -4.445 -3.2766) (xy -4.445 -3.2766) (xy -4.4196 -3.2258) (xy -4.318 -3.0734) (xy -4.2164 -2.8956)
						(xy -4.064 -2.667) (xy -3.6576 -2.0828) (xy -3.8862 -1.5494) (xy -3.937 -1.3716) (xy -4.0386 -1.1684)
						(xy -4.0894 -1.0414) (xy -4.1148 -0.9652) (xy -4.191 -0.9398) (xy -4.318 -0.9144) (xy -4.5466 -0.8636)
						(xy -4.8006 -0.8128) (xy -5.0546 -0.7874) (xy -5.2578 -0.7366) (xy -5.4356 -0.7112) (xy -5.5118 -0.6858)
						(xy -5.5118 -0.6858) (xy -5.5372 -0.635) (xy -5.5372 -0.5588) (xy -5.5372 -0.4318) (xy -5.5626 -0.2286)
						(xy -5.5626 0.0762) (xy -5.5626 0.127) (xy -5.5372 0.4064) (xy -5.5372 0.635) (xy -5.5372 0.762)
						(xy -5.5372 0.8382) (xy -5.5372 0.8382) (xy -5.461 0.8382) (xy -5.3086 0.889) (xy -5.08 0.9144)
						(xy -4.826 0.9652) (xy -4.8006 0.9906) (xy -4.5466 1.0414) (xy -4.318 1.0668) (xy -4.1656 1.1176)
						(xy -4.0894 1.143) (xy -4.0894 1.143) (xy -4.0386 1.2446) (xy -3.9624 1.4224) (xy -3.8608 1.6256)
						(xy -3.7846 1.8288) (xy -3.7084 2.0066) (xy -3.6576 2.159) (xy -3.6322 2.2098) (xy -3.6322 2.2098)
						(xy -3.683 2.286) (xy -3.7592 2.413) (xy -3.8862 2.5908) (xy -4.064 2.8194) (xy -4.064 2.8448)
						(xy -4.2164 3.0734) (xy -4.3434 3.2512) (xy -4.4196 3.3782) (xy -4.445 3.4544) (xy -4.445 3.4544)
						(xy -4.3942 3.5052) (xy -4.2926 3.6322) (xy -4.1148 3.81) (xy -3.937 4.0132) (xy -3.8608 4.064)
						(xy -3.6576 4.2926) (xy -3.5052 4.4196) (xy -3.4036 4.4958) (xy -3.3528 4.5212) (xy -3.3528 4.5212)
						(xy -3.302 4.4704) (xy -3.1496 4.3688) (xy -2.9718 4.2418) (xy -2.7432 4.0894) (xy -2.7178 4.0894)
						(xy -2.4892 3.937) (xy -2.3114 3.81) (xy -2.1844 3.7084) (xy -2.1336 3.683) (xy -2.1082 3.683)
						(xy -2.032 3.7084) (xy -1.8542 3.7592) (xy -1.6764 3.8354) (xy -1.4732 3.937) (xy -1.27 4.0132)
						(xy -1.143 4.064) (xy -1.0668 4.1148) (xy -1.0668 4.1148) (xy -1.0414 4.191) (xy -1.016 4.3434)
						(xy -0.9652 4.572) (xy -0.9144 4.8514) (xy -0.889 4.9022) (xy -0.8382 5.1562) (xy -0.8128 5.3848)
						(xy -0.7874 5.5372) (xy -0.762 5.588) (xy -0.7112 5.6134) (xy -0.5842 5.6134) (xy -0.4064 5.6134)
						(xy -0.1524 5.6134) (xy 0.0762 5.6134) (xy 0.3302 5.6134) (xy 0.5334 5.6134) (xy 0.6858 5.588)
						(xy 0.7366 5.588) (xy 0.7366 5.588) (xy 0.762 5.5118) (xy 0.8128 5.334) (xy 0.8382 5.1054) (xy 0.9144 4.826)
						(xy 0.9144 4.7752) (xy 0.9652 4.5212) (xy 1.016 4.2926) (xy 1.0414 4.1402) (xy 1.0668 4.0894)
						(xy 1.0668 4.0894) (xy 1.1938 4.0386) (xy 1.3716 3.9624) (xy 1.5748 3.8608) (xy 2.0828 3.6576)
						(xy 2.7178 4.0894) (xy 2.7686 4.1402) (xy 2.9972 4.2926) (xy 3.175 4.4196) (xy 3.302 4.4958) (xy 3.3782 4.5212)
						(xy 3.3782 4.5212) (xy 3.429 4.4704) (xy 3.556 4.3434) (xy 3.7338 4.191) (xy 3.9116 3.9878) (xy 4.064 3.8354)
						(xy 4.2418 3.6576) (xy 4.3434 3.556) (xy 4.4196 3.4798) (xy 4.4196 3.429) (xy 4.4196 3.4036) (xy 4.3942 3.3274)
						(xy 4.2926 3.2004) (xy 4.1656 2.9972) (xy 4.0132 2.794) (xy 3.8862 2.5908) (xy 3.7592 2.3876)
						(xy 3.6576 2.2352) (xy 3.6322 2.159) (xy 3.6322 2.1336) (xy 3.683 2.0066) (xy 3.7592 1.8288) (xy 3.8608 1.6002)
						(xy 4.064 1.1176) (xy 4.3942 1.0414) (xy 4.5974 1.016) (xy 4.8768 0.9652) (xy 5.1308 0.9144) (xy 5.5372 0.8382)
						(xy 5.5626 -0.6604) (xy 5.4864 -0.6858) (xy 5.4356 -0.6858) (xy 5.2832 -0.7366) (xy 5.0546 -0.762)
						(xy 4.8006 -0.8128) (xy 4.5974 -0.8636) (xy 4.3688 -0.9144) (xy 4.2164 -0.9398) (xy 4.1402 -0.9398)
						(xy 4.1148 -0.9652) (xy 4.064 -1.0668) (xy 3.9878 -1.2446) (xy 3.9116 -1.4478) (xy 3.81 -1.651)
						(xy 3.7338 -1.8542) (xy 3.683 -2.0066) (xy 3.6576 -2.0828) (xy 3.683 -2.1336) (xy 3.7846 -2.2606)
						(xy 3.8862 -2.4638) (xy 4.0386 -2.667) (xy 4.191 -2.8956) (xy 4.318 -3.0734) (xy 4.3942 -3.2004)
						(xy 4.445 -3.2766) (xy 4.4196 -3.3274) (xy 4.3434 -3.429) (xy 4.1656 -3.5814) (xy 3.937 -3.8354)
						(xy 3.8862 -3.8608) (xy 3.683 -4.064) (xy 3.5306 -4.2164) (xy 3.4036 -4.318) (xy 3.3528 -4.3434)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type outline)
					)
				)
			)
		)
		(symbol "Mechanical:MountingHole"
			(pin_names
				(offset 1.016)
			)
			(exclude_from_sim yes)
			(in_bom no)
			(on_board yes)
			(property "Reference" "H"
				(at 0 5.08 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "MountingHole"
				(at 0 3.175 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Mounting Hole without connection"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "mounting hole"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_fp_filters" "MountingHole*"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "MountingHole_0_1"
				(circle
					(center 0 0)
					(radius 1.27)
					(stroke
						(width 1.27)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
		)
		(symbol "Switch:SW_Push_45deg"
			(pin_numbers hide)
			(pin_names
				(offset 1.016) hide)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "SW"
				(at 3.048 1.016 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "SW_Push_45deg"
				(at 0 -3.81 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" "~"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Push button switch, normally open, two pins, 45° tilted"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "switch normally-open pushbutton push-button"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "SW_Push_45deg_0_1"
				(circle
					(center -1.1684 1.1684)
					(radius 0.508)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -0.508 2.54) (xy 2.54 -0.508)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 1.016 1.016) (xy 2.032 2.032)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -2.54 2.54) (xy -1.524 1.524) (xy -1.524 1.524)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 1.524 -1.524) (xy 2.54 -2.54) (xy 2.54 -2.54) (xy 2.54 -2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(circle
					(center 1.143 -1.1938)
					(radius 0.508)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -2.54 2.54 0)
					(length 0)
					(name "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 2.54 -2.54 180)
					(length 0)
					(name "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
		(symbol "power:+5V"
			(power)
			(pin_numbers hide)
			(pin_names
				(offset 0) hide)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "#PWR"
				(at 0 -3.81 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Value" "+5V"
				(at 0 3.556 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Power symbol creates a global label with name \"+5V\""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "global power"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "+5V_0_1"
				(polyline
					(pts
						(xy -0.762 1.27) (xy 0 2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 0) (xy 0 2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 2.54) (xy 0.762 1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "+5V_1_1"
				(pin power_in line
					(at 0 0 90)
					(length 0)
					(name "~"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
		(symbol "power:GND"
			(power)
			(pin_numbers hide)
			(pin_names
				(offset 0) hide)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "#PWR"
				(at 0 -6.35 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Value" "GND"
				(at 0 -3.81 0)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "Description" "Power symbol creates a global label with name \"GND\" , ground"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(property "ki_keywords" "global power"
				(at 0 0 0)
				(effects
					(font
						(size 1.27 1.27)
					)
					(hide yes)
				)
			)
			(symbol "GND_0_1"
				(polyline
					(pts
						(xy 0 0) (xy 0 -1.27) (xy 1.27 -1.27) (xy 0 -2.54) (xy -1.27 -1.27) (xy 0 -1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "GND_1_1"
				(pin power_in line
					(at 0 0 270)
					(length 0)
					(name "~"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
		)
	)
	(junction
		(at 238.76 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "0037ffda-621d-48da-abbc-23b7758afff2")
	)
	(junction
		(at 247.65 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "030c88d9-8857-49b0-a20d-a6e4a9cd0242")
	)
	(junction
		(at 218.44 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "053fbbab-3108-423f-81b7-f6fe127d7b16")
	)
	(junction
		(at 179.07 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "06727f4f-4445-4c7f-bf3d-ca82a29ff5e3")
	)
	(junction
		(at 127 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "0853fc28-c097-49a4-99ff-c974097fb683")
	)
	(junction
		(at 133.35 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "09782cd2-7f6a-43f1-a1c2-37ee1affd272")
	)
	(junction
		(at 81.28 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "0d472a4d-0db9-47cf-ab7c-ee30d8d53fd9")
	)
	(junction
		(at 48.26 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "122f40e7-7b2e-4297-b634-ae7046f27ed3")
	)
	(junction
		(at 118.11 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "13be92fd-3a7b-4fbf-8d23-71ca6c1c9fa1")
	)
	(junction
		(at 148.59 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "151c6f7f-21bf-4893-af37-ecaaf76b9fba")
	)
	(junction
		(at 55.88 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "18445a96-85e2-4886-9c03-b1b102cd56a8")
	)
	(junction
		(at 102.87 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1a012946-1a60-4233-adb2-24e3d0e08314")
	)
	(junction
		(at 40.64 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1ad49221-252e-4a4f-b0c6-d1e7021890f4")
	)
	(junction
		(at 133.35 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1b5e8765-f022-4e78-9c53-19452fa834b2")
	)
	(junction
		(at 64.77 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1c7bac30-28e2-4259-9d82-45e83d42c5b2")
	)
	(junction
		(at 142.24 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1f2742ee-dc33-444d-b393-69258c48b23f")
	)
	(junction
		(at 118.11 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1f544d88-d39d-41ef-bf53-93e32868c55d")
	)
	(junction
		(at 247.65 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1f89e11e-10ac-4925-8efa-45685e334fb0")
	)
	(junction
		(at 224.79 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "2095d9ad-8aa8-44d0-8ff8-60ba41d23047")
	)
	(junction
		(at 72.39 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "20c984d1-f976-49db-8875-c494de826591")
	)
	(junction
		(at 194.31 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "267844c9-27e3-4240-bdeb-26c68457b4e3")
	)
	(junction
		(at 209.55 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "2de70893-ce4f-4cf7-ab92-266dae7bb90a")
	)
	(junction
		(at 232.41 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "2ecb4e44-239f-4798-b53e-2e6ba3c0eaee")
	)
	(junction
		(at 59.69 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "33337713-a36f-4a0d-84f8-6ef3a2c1bd5c")
	)
	(junction
		(at 111.76 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "35dfa9c7-e0c0-4438-b313-906f1887bfd3")
	)
	(junction
		(at 58.42 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "385ca09e-af0e-4c01-b7f5-3b575d5fdaca")
	)
	(junction
		(at 127 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "3bb2c1d0-9a7c-45bc-852e-7d8eaa4fc2c9")
	)
	(junction
		(at 148.59 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "3bb8abc2-fb64-4ee5-8819-fba3468771a5")
	)
	(junction
		(at 133.35 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "411f9cc3-26cd-462d-9b13-23ebd5e8f84a")
	)
	(junction
		(at 232.41 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "42cc71e3-5cd1-4827-bd41-8724cae445f0")
	)
	(junction
		(at 60.96 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "43328151-3b2c-47ce-9448-28db365f74f0")
	)
	(junction
		(at 157.48 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "44bab60a-0743-4da5-bff3-cecc9311b347")
	)
	(junction
		(at 238.76 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4ba1ef1d-cdbc-4746-932e-90990779fce7")
	)
	(junction
		(at 66.04 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4c975453-130c-4434-8037-630996a04286")
	)
	(junction
		(at 57.15 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4dcab3a1-887e-4018-932f-d2e8cab896bc")
	)
	(junction
		(at 96.52 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4df40092-1f7c-40a9-b69a-90908bd72f59")
	)
	(junction
		(at 203.2 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4e231e0a-0bdb-4b8d-98ba-f7c1bf5378d8")
	)
	(junction
		(at 179.07 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4f1c3e6e-9122-4920-9d2d-b74ccb4d2b15")
	)
	(junction
		(at 118.11 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "5091a605-8c2b-483b-a24d-d8dc87c752ab")
	)
	(junction
		(at 40.64 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "512db071-0fb3-4261-bec1-8094500a220e")
	)
	(junction
		(at 194.31 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "519420a1-e727-4cf3-ba76-b04afea64493")
	)
	(junction
		(at 218.44 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "51aa5c5f-6b09-457b-97af-c7a467271aee")
	)
	(junction
		(at 96.52 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "537045d9-0148-4468-94cd-ebc705e3549a")
	)
	(junction
		(at 142.24 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "565d3a9c-d06e-4539-b272-9a49aa459eca")
	)
	(junction
		(at 96.52 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "570eb38e-7a61-4000-a78a-a86f1d62569c")
	)
	(junction
		(at 172.72 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "5cb3d195-3b76-4042-ba36-f56924d64e88")
	)
	(junction
		(at 163.83 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "5dad2ca3-181f-47d4-b297-33dacd852606")
	)
	(junction
		(at 66.04 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "5de25a61-4370-4749-955a-2a8ebabe5795")
	)
	(junction
		(at 187.96 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "5e88ad63-6088-4f00-8f4c-5bf1f3d89127")
	)
	(junction
		(at 232.41 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "658b2b34-26fe-4a58-aef2-90a414bbebec")
	)
	(junction
		(at 72.39 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "662ca542-08f1-4a1a-a87d-b15dab3dccd2")
	)
	(junction
		(at 224.79 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "66e301d7-574a-4759-950e-08b090448022")
	)
	(junction
		(at 179.07 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "67198aee-13c8-4413-9b69-2afbb5163b3c")
	)
	(junction
		(at 194.31 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "677f7d24-8e82-4e31-843e-48db074e1923")
	)
	(junction
		(at 63.5 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "6840bf16-f9ce-4388-b0fc-f1f8b5ce7c71")
	)
	(junction
		(at 96.52 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "6d549e16-f620-4bb3-b65f-20e1f09b47a1")
	)
	(junction
		(at 96.52 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "70323982-88c5-4670-bdc0-c66526dc1236")
	)
	(junction
		(at 163.83 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "7347a0ff-4548-4a3a-a6ef-611114a87201")
	)
	(junction
		(at 81.28 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "75a5b33d-ad28-45a6-872d-cc9d1f84e346")
	)
	(junction
		(at 118.11 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "77793cb5-577e-48fe-8eb8-2a6b4da56db6")
	)
	(junction
		(at 62.23 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "797ae6cc-ce1f-4f63-9fb9-bd747f6803e0")
	)
	(junction
		(at 218.44 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "7b789afc-c27c-4223-8d23-40f94d34aaea")
	)
	(junction
		(at 111.76 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "7c500a8f-a3b3-4315-9aaf-677ecf5ad0b3")
	)
	(junction
		(at 247.65 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "7e572e77-b284-402a-bc07-5b99e2b97f01")
	)
	(junction
		(at 87.63 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "817d49cb-8733-4f14-b0f8-2dc8a3c9b7bc")
	)
	(junction
		(at 40.64 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "82de7456-a9c4-4a1f-be95-436a18122a8c")
	)
	(junction
		(at 238.76 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "84013f61-28d6-445b-8857-dde3f132edf0")
	)
	(junction
		(at 40.64 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "84a26157-5d21-41b8-af65-da23a4bb0b31")
	)
	(junction
		(at 157.48 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "86dc8985-707c-4667-bab5-fc135888f15f")
	)
	(junction
		(at 238.76 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "87b1f173-cd98-4f2f-a4b4-a48eab5a7e16")
	)
	(junction
		(at 247.65 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "894a200d-3b84-450c-9687-c451e1131586")
	)
	(junction
		(at 194.31 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "8d6ceb0b-6ac0-494a-92f2-bbb4f3a1d2c4")
	)
	(junction
		(at 81.28 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "90f1c9da-8112-4ace-beb3-049a7e064089")
	)
	(junction
		(at 247.65 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "91b2d459-3316-4578-9cc2-d7b4d3a749dc")
	)
	(junction
		(at 102.87 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "91c87226-f186-498d-a9e6-5fe581c71ddb")
	)
	(junction
		(at 224.79 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "92814367-9bea-47af-a184-9c75a1fbac9d")
	)
	(junction
		(at 102.87 43.18)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "95bcb6ab-9646-4cfe-aec1-dbc80d732c62")
	)
	(junction
		(at 187.96 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "987495f2-8b77-4290-b671-966fadcd74dd")
	)
	(junction
		(at 172.72 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "99b8d691-7afa-4b7d-ae36-f1ea52d18210")
	)
	(junction
		(at 87.63 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "9a4f54c8-6e3c-49bb-b8b1-4a60eba96215")
	)
	(junction
		(at 148.59 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "9bb005a8-f3e2-413d-ba7c-03534bfee560")
	)
	(junction
		(at 163.83 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "9cf4da88-32ed-4bb4-bc90-a49751e24c05")
	)
	(junction
		(at 203.2 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "9d5186cc-e013-4168-9764-4f5cae46af4a")
	)
	(junction
		(at 209.55 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a057ef3a-8292-4451-970f-bacf99b5cefd")
	)
	(junction
		(at 209.55 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a07f466e-08a6-466b-982b-f215584b0d7b")
	)
	(junction
		(at 81.28 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a33980b4-af79-49f6-9ec0-2a6984225359")
	)
	(junction
		(at 262.89 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a480cd41-cb7e-40ff-8606-93420d026f0f")
	)
	(junction
		(at 209.55 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a521831a-3f5a-40a6-92a1-499f52a070fb")
	)
	(junction
		(at 157.48 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a55f0c42-c658-459e-b0d4-21075f331a51")
	)
	(junction
		(at 218.44 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "a8b593da-306b-4e72-b60c-9ff49c153abe")
	)
	(junction
		(at 148.59 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "aaa52091-a921-4f6a-8939-9a143227682a")
	)
	(junction
		(at 203.2 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "aeee6276-d4d6-42ac-bd6e-57200aa05b09")
	)
	(junction
		(at 262.89 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "af0985cb-f765-4fe6-9925-baf7af9a51be")
	)
	(junction
		(at 48.26 102.87)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "aff363b1-6824-4e8a-bd84-a74efa0377e4")
	)
	(junction
		(at 66.04 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "b53bb5fc-554f-4bef-81ed-b19d6ca7ddbc")
	)
	(junction
		(at 48.26 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "b7ad666e-fe87-4347-bf06-a078b8152ffc")
	)
	(junction
		(at 187.96 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "bb05cbe0-e89e-4be6-b360-a60a40d96400")
	)
	(junction
		(at 179.07 82.55)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "c119bb42-5210-4f5e-8784-565d7a4b9258")
	)
	(junction
		(at 127 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "cd6cf71e-324e-4ad1-a6a5-c07925a980bc")
	)
	(junction
		(at 218.44 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d2be198a-cada-4946-bde2-e5fdf6df7e63")
	)
	(junction
		(at 72.39 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d2febfc5-35be-40fd-8ada-1c2ce26838ff")
	)
	(junction
		(at 224.79 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d4f0f986-2083-4724-b237-94267d2ed887")
	)
	(junction
		(at 187.96 25.4)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d6274078-d42e-401d-85c4-be659e33b96d")
	)
	(junction
		(at 111.76 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d77a9672-1be4-4807-98d4-a9ab5701f209")
	)
	(junction
		(at 142.24 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d89c7a9f-8a8c-4713-817a-64b4bc9d5689")
	)
	(junction
		(at 232.41 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d9607b84-8189-4e77-8f8a-1c523fb8f7d8")
	)
	(junction
		(at 157.48 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "d97c9772-2aaf-4e77-b404-579655d99204")
	)
	(junction
		(at 163.83 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "db043327-071b-4215-b251-990645c9e211")
	)
	(junction
		(at 87.63 63.5)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "dcc639e6-511f-4fe1-bf07-4ec56b7cf2ba")
	)
	(junction
		(at 87.63 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "ddbb4276-3944-4e5c-a53a-bbfb0ebdf64b")
	)
	(junction
		(at 262.89 90.17)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "ea118f01-0e7a-4b20-82d2-cf35a194de84")
	)
	(junction
		(at 127 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "ee6e1403-6a0c-4892-9117-61122a99bd48")
	)
	(junction
		(at 172.72 50.8)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "ef563c55-638c-4745-941f-59680112f11c")
	)
	(junction
		(at 102.87 62.23)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "ef81e907-2eea-4393-a894-77f0279b3eae")
	)
	(junction
		(at 157.48 31.75)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "f1eb69db-086f-4ab7-bc53-552e1f4241a2")
	)
	(junction
		(at 48.26 44.45)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "f2e6960c-59a0-4a7c-92d3-50a4ce977bba")
	)
	(junction
		(at 133.35 101.6)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "f50cf600-45e7-4122-aed3-ad1d9b92d941")
	)
	(junction
		(at 66.04 69.85)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "f6a07eac-3590-4201-8a7e-12780019a6ba")
	)
	(junction
		(at 203.2 104.14)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "f6a379c5-5a39-495a-a372-505c88afdf3b")
	)
	(junction
		(at 72.39 81.28)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "fd5e6df0-4e56-41d0-b580-8b976f699d78")
	)
	(no_connect
		(at 52.07 176.53)
		(uuid "0e2d00bc-ac11-4d6f-85b0-0213eaae921c")
	)
	(no_connect
		(at 52.07 171.45)
		(uuid "85e56baa-b05d-4e73-84f1-02094fce96e7")
	)
	(no_connect
		(at 96.52 151.13)
		(uuid "9c78b835-6a1f-4461-a35d-98440803145d")
	)
	(no_connect
		(at 52.07 168.91)
		(uuid "ec5c8877-081b-4e51-b9ee-71a1b494fd43")
	)
	(wire
		(pts
			(xy 264.16 110.49) (xy 262.89 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "008442d2-4b8f-4f01-9dbe-36c4e9927e0b")
	)
	(wire
		(pts
			(xy 127 69.85) (xy 127 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "01717fe8-5782-425a-a3a6-fa791e51ba02")
	)
	(wire
		(pts
			(xy 194.31 81.28) (xy 224.79 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "017ebaaa-4325-4647-9a79-ce2e5fdf1a04")
	)
	(wire
		(pts
			(xy 102.87 124.46) (xy 62.23 124.46)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "01f98d55-8bb6-47c5-95cb-cf566219f4f4")
	)
	(wire
		(pts
			(xy 102.87 100.33) (xy 102.87 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0239f7eb-4c54-42a9-8f16-6b605bbfd2b8")
	)
	(wire
		(pts
			(xy 82.55 69.85) (xy 81.28 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0264f744-d7e6-44d6-ba74-63d9df741283")
	)
	(wire
		(pts
			(xy 72.39 43.18) (xy 102.87 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0276758e-2eb8-46e6-8974-9c2592bdcce2")
	)
	(wire
		(pts
			(xy 194.31 43.18) (xy 194.31 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "027a3aeb-24da-409b-882e-604eb88700c1")
	)
	(wire
		(pts
			(xy 238.76 82.55) (xy 261.62 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "02a97d8e-e41e-4412-bace-86203ada334e")
	)
	(wire
		(pts
			(xy 163.83 125.73) (xy 163.83 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "02e96d6f-fec1-4a74-8ac7-8422e674b67b")
	)
	(wire
		(pts
			(xy 172.72 31.75) (xy 172.72 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "04455f56-6e26-4f19-8fba-d1d4c8d86cdb")
	)
	(wire
		(pts
			(xy 72.39 43.18) (xy 72.39 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "04da2661-090a-4667-bce6-308ef8cf37b3")
	)
	(wire
		(pts
			(xy 187.96 25.4) (xy 187.96 22.86)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "05f7f29e-5321-4ddc-ac9a-c3ac574fa930")
	)
	(wire
		(pts
			(xy 87.63 41.91) (xy 87.63 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0737f761-1154-45c0-a1f5-dca26d7328e2")
	)
	(wire
		(pts
			(xy 224.79 43.18) (xy 254 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "07bcf568-d664-4491-8339-33022e9bd2a1")
	)
	(wire
		(pts
			(xy 66.04 110.49) (xy 66.04 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "07e0a81c-3659-4dc7-92e0-00ce44f24800")
	)
	(wire
		(pts
			(xy 203.2 31.75) (xy 203.2 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "09298890-dc48-4d17-a6fa-ff620fc41759")
	)
	(wire
		(pts
			(xy 219.71 110.49) (xy 218.44 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0aa5b414-954b-4280-a0bc-847fccc65f92")
	)
	(wire
		(pts
			(xy 157.48 25.4) (xy 172.72 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0affc126-3820-4bac-99b2-c5a471b31350")
	)
	(wire
		(pts
			(xy 60.96 63.5) (xy 60.96 125.73)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0be0274e-00cc-46f6-a721-f40f312f70cb")
	)
	(wire
		(pts
			(xy 58.42 128.27) (xy 224.79 128.27)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0c224ddd-1c36-4263-9d50-53db2891ad2c")
	)
	(wire
		(pts
			(xy 40.64 81.28) (xy 59.69 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0c899a17-c2c9-4c78-9d31-a26814ed0d78")
	)
	(wire
		(pts
			(xy 96.52 31.75) (xy 96.52 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0ce262ae-a66e-440d-85ed-d65128d5de9d")
	)
	(wire
		(pts
			(xy 157.48 22.86) (xy 157.48 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0de8ee3f-7132-428b-941d-4b2e798dc28b")
	)
	(wire
		(pts
			(xy 72.39 121.92) (xy 64.77 121.92)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0e11b977-1ee2-4acb-bf59-99a66461af95")
	)
	(wire
		(pts
			(xy 148.59 102.87) (xy 179.07 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0e668b1b-c8fc-4207-a223-a4c69e3982bb")
	)
	(wire
		(pts
			(xy 142.24 50.8) (xy 143.51 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "100c39b2-25c5-45cc-a8d2-4e4cc82b47c8")
	)
	(wire
		(pts
			(xy 111.76 50.8) (xy 111.76 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "10668519-774a-4ce2-9872-91bdc0730e99")
	)
	(wire
		(pts
			(xy 57.15 129.54) (xy 238.76 129.54)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "109b8f90-bb8e-40fc-8c03-21542e9d57dc")
	)
	(wire
		(pts
			(xy 142.24 50.8) (xy 142.24 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1200f9e1-f6b5-4a93-bad0-da89e0f1912f")
	)
	(wire
		(pts
			(xy 118.11 63.5) (xy 118.11 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "12b01fc5-5d9f-486c-82f1-4e8d5d1e3eee")
	)
	(wire
		(pts
			(xy 40.64 101.6) (xy 35.56 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1367efe9-6476-472b-9e16-1185383d3d26")
	)
	(wire
		(pts
			(xy 36.83 64.77) (xy 35.56 64.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "139686f6-3c5a-46a5-b825-a7d80b710a42")
	)
	(wire
		(pts
			(xy 97.79 110.49) (xy 96.52 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "14017022-c3c7-4b44-bfb5-91c219aa273e")
	)
	(wire
		(pts
			(xy 102.87 62.23) (xy 133.35 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1477054d-b76b-4178-8574-0090ae8615b4")
	)
	(wire
		(pts
			(xy 194.31 81.28) (xy 194.31 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "149a7819-7222-44d5-8cff-85ca1692dd75")
	)
	(wire
		(pts
			(xy 203.2 25.4) (xy 203.2 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "14d09593-5ca4-4e30-a001-239c07cd99b9")
	)
	(wire
		(pts
			(xy 96.52 31.75) (xy 96.52 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "157c47a7-e108-4843-a0a2-540c1e0fa1f2")
	)
	(wire
		(pts
			(xy 209.55 102.87) (xy 238.76 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "17fc6ed7-fb3b-41be-8f0a-cd821485cb5e")
	)
	(wire
		(pts
			(xy 269.24 63.5) (xy 269.24 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "19674363-cccc-4fdf-a6bd-fb7b270287bc")
	)
	(wire
		(pts
			(xy 66.04 25.4) (xy 66.04 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1aff716f-cab2-4c31-b6f0-81d3f5094ed8")
	)
	(wire
		(pts
			(xy 35.56 43.18) (xy 40.64 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1b904620-26b1-419a-abee-5321a9fc84a0")
	)
	(wire
		(pts
			(xy 203.2 110.49) (xy 203.2 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1c05aa7e-af81-4862-82c6-49d9681c1a02")
	)
	(wire
		(pts
			(xy 172.72 50.8) (xy 173.99 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1c659453-ca56-449b-9b19-0582e02c458f")
	)
	(wire
		(pts
			(xy 58.42 82.55) (xy 58.42 128.27)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1d12628a-bcfd-4a60-b079-dac0658033c6")
	)
	(wire
		(pts
			(xy 247.65 25.4) (xy 262.89 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1d45c328-0760-48b4-8429-19d453e24f33")
	)
	(wire
		(pts
			(xy 64.77 43.18) (xy 72.39 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1de4c342-070d-45a5-90f4-62437fab64dc")
	)
	(wire
		(pts
			(xy 163.83 81.28) (xy 163.83 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "20062456-24ae-48c9-8231-1ac889b0f474")
	)
	(wire
		(pts
			(xy 260.35 123.19) (xy 260.35 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "21023032-88fe-40d1-aad6-4de1cf57096f")
	)
	(wire
		(pts
			(xy 157.48 69.85) (xy 157.48 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2174a8e6-ceb9-4ad1-8628-48f547b2fa11")
	)
	(wire
		(pts
			(xy 264.16 50.8) (xy 262.89 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "218a2911-0dc0-4e1d-8134-9bb50b300c0a")
	)
	(wire
		(pts
			(xy 269.24 101.6) (xy 269.24 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2281762b-3af7-485d-bdaa-8b8d8836bd55")
	)
	(wire
		(pts
			(xy 142.24 31.75) (xy 142.24 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "22e8cc8e-0fc2-43f0-92b3-940a8e7611ae")
	)
	(wire
		(pts
			(xy 60.96 125.73) (xy 163.83 125.73)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "23d1479a-05dc-4988-b0d1-1082ea4cc004")
	)
	(wire
		(pts
			(xy 66.04 31.75) (xy 66.04 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2409b6e8-adc1-4293-a77e-619e9c86e32b")
	)
	(wire
		(pts
			(xy 232.41 69.85) (xy 233.68 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "24c7a100-7724-42b8-b772-161376ceddbd")
	)
	(wire
		(pts
			(xy 87.63 102.87) (xy 118.11 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2506bed9-25bd-4f27-8543-4950c615e719")
	)
	(wire
		(pts
			(xy 59.69 81.28) (xy 59.69 127)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2629cb09-a4c6-4fe4-a50b-cc1ca0e1b4eb")
	)
	(wire
		(pts
			(xy 187.96 69.85) (xy 189.23 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "267e79b6-d765-41ab-bd1d-9567acd26f90")
	)
	(wire
		(pts
			(xy 172.72 50.8) (xy 172.72 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "26925c80-2299-4d76-b2e5-9f59e102f99a")
	)
	(wire
		(pts
			(xy 127 22.86) (xy 127 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "27740eb5-9656-4414-a23b-ed3531655e84")
	)
	(wire
		(pts
			(xy 111.76 69.85) (xy 111.76 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "28324a4c-f9fd-477b-9b51-302bcf8b11e1")
	)
	(wire
		(pts
			(xy 232.41 69.85) (xy 232.41 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "290d6451-b21e-4bd7-a0d5-1224f98fe06f")
	)
	(wire
		(pts
			(xy 96.52 69.85) (xy 96.52 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "29f3736b-cbb8-4684-ab40-7edec5cc35ad")
	)
	(wire
		(pts
			(xy 36.83 104.14) (xy 35.56 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2a2b558b-b5f8-4c25-92e5-1b7e9f70a9ee")
	)
	(wire
		(pts
			(xy 97.79 90.17) (xy 96.52 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2ae52918-c424-461d-bbab-a440c11b75bd")
	)
	(wire
		(pts
			(xy 238.76 102.87) (xy 261.62 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2cb2e26d-897e-47f8-ac64-bf28cc55bf77")
	)
	(wire
		(pts
			(xy 102.87 101.6) (xy 133.35 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2d0a3ece-6431-4c40-bee4-0c41c8256d92")
	)
	(wire
		(pts
			(xy 224.79 128.27) (xy 224.79 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2de09910-b434-43b8-b266-3367730265c5")
	)
	(wire
		(pts
			(xy 261.62 82.55) (xy 261.62 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2e76b2be-f422-4668-a896-cc04d95abee9")
	)
	(wire
		(pts
			(xy 218.44 31.75) (xy 219.71 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2e9efa9f-28c6-471a-a29f-41584edc5553")
	)
	(wire
		(pts
			(xy 254 101.6) (xy 254 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2fe2b183-3238-4d13-9beb-fe51ae7549e8")
	)
	(wire
		(pts
			(xy 63.5 123.19) (xy 87.63 123.19)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "2fea4f2f-b878-4c6c-9bd9-f66a9b7d3ab3")
	)
	(wire
		(pts
			(xy 209.55 102.87) (xy 209.55 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "306ffaf4-3e1b-4024-85de-a3940a53b693")
	)
	(wire
		(pts
			(xy 55.88 102.87) (xy 55.88 130.81)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "31836c7a-73c3-40b0-91de-9bb9da451c22")
	)
	(wire
		(pts
			(xy 127 50.8) (xy 127 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "32f906ad-512c-4086-a833-c6530b953128")
	)
	(wire
		(pts
			(xy 36.83 102.87) (xy 48.26 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "33816dc6-6169-4176-83e0-edfb0e2a8c64")
	)
	(wire
		(pts
			(xy 238.76 82.55) (xy 238.76 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3416bea4-b29b-451d-ba8d-939ab18b9fd1")
	)
	(wire
		(pts
			(xy 218.44 69.85) (xy 218.44 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "34597109-9958-47a1-89b7-21c61049c748")
	)
	(wire
		(pts
			(xy 189.23 90.17) (xy 187.96 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "34934e52-ecf7-4315-96ea-5db6e8f18231")
	)
	(wire
		(pts
			(xy 81.28 50.8) (xy 81.28 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "35e69439-f5ff-4373-a398-32c88ede0e68")
	)
	(wire
		(pts
			(xy 232.41 31.75) (xy 233.68 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "39bd26aa-ac96-4aff-9eca-a6a9f5af065c")
	)
	(wire
		(pts
			(xy 102.87 120.65) (xy 102.87 124.46)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3a018983-964f-49c4-9aba-20de2a9501e6")
	)
	(wire
		(pts
			(xy 232.41 90.17) (xy 232.41 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3a0643c1-df68-40f8-b167-da9135f84f9c")
	)
	(wire
		(pts
			(xy 163.83 101.6) (xy 163.83 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3a3edd0e-522c-425f-b5d6-0989f607cb3c")
	)
	(wire
		(pts
			(xy 57.15 101.6) (xy 57.15 129.54)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3b41aea4-ceb4-4c83-9bac-47caea360c55")
	)
	(wire
		(pts
			(xy 82.55 50.8) (xy 81.28 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3b9a439c-8232-4cd9-a088-b9bbd0a144a4")
	)
	(wire
		(pts
			(xy 187.96 31.75) (xy 187.96 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3bf12899-ef62-482e-8dfa-ce546a27f14d")
	)
	(wire
		(pts
			(xy 48.26 102.87) (xy 55.88 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3c5c4801-27e5-4fc3-8da0-e4f319164da1")
	)
	(wire
		(pts
			(xy 72.39 62.23) (xy 102.87 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3d559351-e6dc-4c0b-afb1-41ca1a9334b0")
	)
	(wire
		(pts
			(xy 203.2 104.14) (xy 218.44 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3d7ad9d0-a729-4886-8d5a-4a1f1a049558")
	)
	(wire
		(pts
			(xy 209.55 44.45) (xy 238.76 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3f565247-5f99-4504-985a-e0bd2e6c47c4")
	)
	(wire
		(pts
			(xy 133.35 81.28) (xy 133.35 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "405e6c28-29f0-4443-aba0-b933c4d6ebcf")
	)
	(wire
		(pts
			(xy 179.07 63.5) (xy 209.55 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "40ea22d5-b87d-45bd-8f07-d96555c653e8")
	)
	(wire
		(pts
			(xy 82.55 31.75) (xy 81.28 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "416bfd63-c735-43c8-aac1-8bff382634d5")
	)
	(wire
		(pts
			(xy 58.42 82.55) (xy 87.63 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "42f9dd4e-161a-4c85-8a15-ff72e79d3c4e")
	)
	(wire
		(pts
			(xy 209.55 63.5) (xy 238.76 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "437e1d4f-d998-4dc2-9914-a84b6ea831d5")
	)
	(wire
		(pts
			(xy 113.03 50.8) (xy 111.76 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4440d7d7-1a96-49e7-9f36-b17788d512ab")
	)
	(wire
		(pts
			(xy 187.96 50.8) (xy 187.96 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "47ce59ca-614f-4936-b7b3-d5d8e43e2e9d")
	)
	(wire
		(pts
			(xy 247.65 69.85) (xy 248.92 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "48327cb3-021c-40e7-8c13-4f4ac388b514")
	)
	(wire
		(pts
			(xy 66.04 22.86) (xy 66.04 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4a14b293-ee7a-40da-baec-93f50bc44592")
	)
	(wire
		(pts
			(xy 72.39 62.23) (xy 72.39 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4b3542a6-ccc2-46e0-b287-c18e4c5b3252")
	)
	(wire
		(pts
			(xy 66.04 104.14) (xy 81.28 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4b905c03-6fe1-4d35-a016-43426cb0fe5c")
	)
	(wire
		(pts
			(xy 261.62 121.92) (xy 269.24 121.92)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4c6ef9d9-d1b0-4ffe-9e74-2828a79cf1e2")
	)
	(wire
		(pts
			(xy 157.48 90.17) (xy 158.75 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4c97ad62-a089-4959-b1ae-b78a342e53aa")
	)
	(wire
		(pts
			(xy 233.68 110.49) (xy 232.41 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4d1198d9-35be-459a-952e-ec8baac7ef27")
	)
	(wire
		(pts
			(xy 247.65 25.4) (xy 247.65 22.86)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4e6c41fc-3d7b-4b06-9d0b-cf2ce1f20e30")
	)
	(wire
		(pts
			(xy 63.5 44.45) (xy 48.26 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4e8b44d6-1177-4498-bed4-0525c8bc7ac3")
	)
	(wire
		(pts
			(xy 66.04 25.4) (xy 81.28 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "503a5291-c729-4e28-b7aa-056bf7897bec")
	)
	(wire
		(pts
			(xy 232.41 50.8) (xy 232.41 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "519c6832-6964-4ede-82e7-f2bf13b5991f")
	)
	(wire
		(pts
			(xy 127 69.85) (xy 128.27 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5301ee5c-59ac-48cc-ae4d-dfbb1d0b443d")
	)
	(wire
		(pts
			(xy 247.65 31.75) (xy 248.92 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5454a7db-f311-4d37-a201-69aebdf4d905")
	)
	(wire
		(pts
			(xy 82.55 110.49) (xy 81.28 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "578989db-1946-4a9d-b766-9a870b8e7cca")
	)
	(wire
		(pts
			(xy 157.48 31.75) (xy 157.48 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5793cb31-92e9-4504-b3cf-ade4555839b4")
	)
	(wire
		(pts
			(xy 148.59 102.87) (xy 148.59 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "589f2b59-10bb-473e-9f08-84d015286c51")
	)
	(wire
		(pts
			(xy 238.76 63.5) (xy 238.76 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "597784f4-9ac0-494f-afe2-4eca4cd72cfc")
	)
	(wire
		(pts
			(xy 204.47 110.49) (xy 203.2 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "59ba64e3-0f5c-431c-a8af-c79eecec9823")
	)
	(wire
		(pts
			(xy 63.5 44.45) (xy 63.5 123.19)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5a746c34-90f0-46cf-9f14-64a16113a7aa")
	)
	(wire
		(pts
			(xy 36.83 82.55) (xy 48.26 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5ac99574-3b1c-4317-b4b3-5efda14d92aa")
	)
	(wire
		(pts
			(xy 57.15 101.6) (xy 40.64 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5cb8e1fb-0f8d-45e8-ace3-e8a5061c05c9")
	)
	(wire
		(pts
			(xy 203.2 69.85) (xy 204.47 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5d14642b-190a-4d25-9c83-842c5202d28d")
	)
	(wire
		(pts
			(xy 203.2 31.75) (xy 204.47 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5d824aea-3a6b-42a3-8bf4-ff11fb9e195f")
	)
	(wire
		(pts
			(xy 87.63 82.55) (xy 118.11 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5d87a427-03d9-4838-b801-bb5f9229d639")
	)
	(wire
		(pts
			(xy 203.2 50.8) (xy 204.47 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5e23f5dc-ec9c-4572-96fb-5cc93ad220a4")
	)
	(wire
		(pts
			(xy 81.28 25.4) (xy 81.28 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5f38d957-e235-49b9-bfae-af4b27096ad7")
	)
	(wire
		(pts
			(xy 157.48 50.8) (xy 158.75 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "603b352f-35d3-4d54-82ba-2ecd417e7781")
	)
	(wire
		(pts
			(xy 127 31.75) (xy 128.27 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "60d193bf-a334-40b4-9c10-8aad036018ae")
	)
	(wire
		(pts
			(xy 118.11 102.87) (xy 148.59 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "618fe1fa-7cbb-44c3-b619-b5b8c799a935")
	)
	(wire
		(pts
			(xy 148.59 44.45) (xy 179.07 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6372390f-8a48-4770-a8b3-461894855cf9")
	)
	(wire
		(pts
			(xy 179.07 82.55) (xy 179.07 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "64ff4dd1-6bb3-44c2-a128-57d806fd129e")
	)
	(wire
		(pts
			(xy 81.28 110.49) (xy 81.28 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "68eb3fd4-3179-486b-b305-eac2377a0338")
	)
	(wire
		(pts
			(xy 72.39 100.33) (xy 72.39 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6ac0f008-4d8a-49c5-9380-1c7c723c59d1")
	)
	(wire
		(pts
			(xy 163.83 81.28) (xy 194.31 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6b0cabef-a323-4031-819e-053b7938f8b6")
	)
	(wire
		(pts
			(xy 102.87 43.18) (xy 133.35 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6ba42cd6-db27-4697-8871-19a0d0aaaf26")
	)
	(wire
		(pts
			(xy 261.62 102.87) (xy 261.62 121.92)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6bc136ff-14c6-451d-b9ab-4737d51ad9d4")
	)
	(wire
		(pts
			(xy 264.16 31.75) (xy 262.89 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6e2596d1-44fa-4940-a502-57cade1a8074")
	)
	(wire
		(pts
			(xy 87.63 80.01) (xy 87.63 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6e7d9f15-a9fc-4019-9920-d78b8e7a58c6")
	)
	(wire
		(pts
			(xy 224.79 62.23) (xy 224.79 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6f13364a-4988-4ba7-b404-c7317cbd4d29")
	)
	(wire
		(pts
			(xy 254 130.81) (xy 254 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6f6f1e1d-0ffb-49d8-83a8-2a34a9273b9a")
	)
	(wire
		(pts
			(xy 118.11 82.55) (xy 148.59 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6fcd5baf-4815-4ac1-acb5-c3d0c4bfa7f1")
	)
	(wire
		(pts
			(xy 218.44 50.8) (xy 218.44 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "703ef167-c7b6-457f-9a5f-6dc725d71ccf")
	)
	(wire
		(pts
			(xy 55.88 102.87) (xy 87.63 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "70801080-8885-4062-98d3-eebce7ba556f")
	)
	(wire
		(pts
			(xy 96.52 90.17) (xy 96.52 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "70f5db8a-cb9b-4179-9043-f566b8b3f503")
	)
	(wire
		(pts
			(xy 224.79 62.23) (xy 254 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "70ff9c94-939a-4025-b7c8-4d86928a7855")
	)
	(wire
		(pts
			(xy 67.31 110.49) (xy 66.04 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "73636f17-f94f-4b3c-9616-b718dcb9f589")
	)
	(wire
		(pts
			(xy 157.48 25.4) (xy 157.48 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "73a029fa-46aa-475e-8916-ba192ba70112")
	)
	(wire
		(pts
			(xy 40.64 64.77) (xy 40.64 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "74108400-76bd-4063-ae82-9264283c7ea5")
	)
	(wire
		(pts
			(xy 72.39 101.6) (xy 102.87 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "75faf6d5-2ae6-45dd-95a7-74680d172dfb")
	)
	(wire
		(pts
			(xy 194.31 62.23) (xy 194.31 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "78d5b922-3324-4981-beb6-97ce8f5e0e42")
	)
	(wire
		(pts
			(xy 96.52 25.4) (xy 96.52 22.86)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7b97396b-cfa3-4f16-95de-d90574086a85")
	)
	(wire
		(pts
			(xy 40.64 83.82) (xy 40.64 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7c756b92-3a98-4ee4-b32a-e65fa2e32531")
	)
	(wire
		(pts
			(xy 157.48 104.14) (xy 203.2 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7c8fd99b-17ee-4b51-bf70-c0024ec3ad7c")
	)
	(wire
		(pts
			(xy 72.39 81.28) (xy 72.39 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7cc7fdeb-b8bf-4ba1-8d79-87729b481bea")
	)
	(wire
		(pts
			(xy 163.83 43.18) (xy 194.31 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7d8e8763-aa96-4ad9-acc5-f893cb0222a8")
	)
	(wire
		(pts
			(xy 232.41 110.49) (xy 232.41 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7ee1673c-a49e-41b6-8a19-c3024d1c49f3")
	)
	(wire
		(pts
			(xy 66.04 50.8) (xy 66.04 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7fe45458-9f24-4443-86eb-a6dcbbabde99")
	)
	(wire
		(pts
			(xy 40.64 45.72) (xy 40.64 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "81ae495c-f706-4fd1-a35d-9e0c7aedeb22")
	)
	(wire
		(pts
			(xy 269.24 121.92) (xy 269.24 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "83561539-f7b0-4ddf-adc2-d5c1a747829d")
	)
	(wire
		(pts
			(xy 81.28 69.85) (xy 81.28 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "83b7b2af-9239-4cc1-9bb7-ca451b067b05")
	)
	(wire
		(pts
			(xy 194.31 101.6) (xy 224.79 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "83ef2fb3-0eea-4fd0-96b9-c2300e9ab4d8")
	)
	(wire
		(pts
			(xy 218.44 25.4) (xy 218.44 22.86)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "856a5615-1e8f-4059-a117-5df4e136dffe")
	)
	(wire
		(pts
			(xy 82.55 90.17) (xy 81.28 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "85da36c4-f329-42bc-849e-3950efa2dbeb")
	)
	(wire
		(pts
			(xy 247.65 110.49) (xy 247.65 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "87cec3b1-b500-4dae-bbbc-bdc97a7bc103")
	)
	(wire
		(pts
			(xy 72.39 81.28) (xy 102.87 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "88d7d365-a54c-4dfe-b84d-caaf630f6da4")
	)
	(wire
		(pts
			(xy 254 81.28) (xy 254 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "892d1cc5-27ba-4e92-a12d-9f9479874266")
	)
	(wire
		(pts
			(xy 218.44 104.14) (xy 232.41 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8b1a6b41-8d31-4f0b-b191-dd4de0d3173e")
	)
	(wire
		(pts
			(xy 203.2 50.8) (xy 203.2 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8b43ac86-671c-4e30-8fcc-8f3692ed8e26")
	)
	(wire
		(pts
			(xy 35.56 62.23) (xy 40.64 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8bc65719-10e9-46cb-916b-f388edc1333f")
	)
	(wire
		(pts
			(xy 269.24 44.45) (xy 269.24 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8c515ec6-209c-4a5c-bb53-829a15dac4cb")
	)
	(wire
		(pts
			(xy 127 90.17) (xy 128.27 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8c6fa04f-442a-415c-9c5f-1936f6978e52")
	)
	(wire
		(pts
			(xy 59.69 81.28) (xy 72.39 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8d3c5fd9-0f74-4c99-acd8-cea772ba55a0")
	)
	(wire
		(pts
			(xy 261.62 101.6) (xy 269.24 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8dade74a-dc13-4294-916f-8688d7ff2104")
	)
	(wire
		(pts
			(xy 87.63 100.33) (xy 87.63 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8f364dbd-89aa-4819-85cc-70bfd89eb889")
	)
	(wire
		(pts
			(xy 232.41 104.14) (xy 247.65 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "907a1963-f85c-4c85-9b30-34f63d6cda22")
	)
	(wire
		(pts
			(xy 111.76 31.75) (xy 111.76 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9186ad08-385e-4b30-9c3f-01617ebdb7a1")
	)
	(wire
		(pts
			(xy 238.76 102.87) (xy 238.76 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "91c25c31-85d7-42b8-8720-0d7fc79aa793")
	)
	(wire
		(pts
			(xy 172.72 90.17) (xy 173.99 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "927f9eaa-74ac-430c-ab57-b590a77d2ac0")
	)
	(wire
		(pts
			(xy 148.59 82.55) (xy 179.07 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "939dd271-0cad-4da8-b130-0cc2235d08a6")
	)
	(wire
		(pts
			(xy 127 50.8) (xy 128.27 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "945e981b-7ff5-4f8b-950e-7fd1d03bf824")
	)
	(wire
		(pts
			(xy 72.39 120.65) (xy 72.39 121.92)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "95aced58-21e4-4867-8838-0f8e795e31ab")
	)
	(wire
		(pts
			(xy 163.83 43.18) (xy 163.83 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "95fb066b-db69-4759-a616-93a331be1d33")
	)
	(wire
		(pts
			(xy 40.64 43.18) (xy 64.77 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "96a502ae-3c72-464d-871c-2a2a0da53e86")
	)
	(wire
		(pts
			(xy 72.39 101.6) (xy 57.15 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "96d19ba7-a276-45b1-b4b9-dc262421b7b9")
	)
	(wire
		(pts
			(xy 142.24 31.75) (xy 143.51 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "98143cf9-a266-4593-af67-8c94d6066bb9")
	)
	(wire
		(pts
			(xy 262.89 90.17) (xy 264.16 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "986a7b24-99ae-4621-a3ed-cbff01b9e9cb")
	)
	(wire
		(pts
			(xy 48.26 83.82) (xy 48.26 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "98b47ca9-0772-4893-8f88-9f2bf127d266")
	)
	(wire
		(pts
			(xy 133.35 62.23) (xy 163.83 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "99da66de-277c-419d-a3ea-7f03b6982f70")
	)
	(wire
		(pts
			(xy 48.26 63.5) (xy 36.83 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9afead27-7b59-4376-8331-7dadf086b2ad")
	)
	(wire
		(pts
			(xy 113.03 31.75) (xy 111.76 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9b2e8422-0c10-4ca0-81ac-efade3767f4b")
	)
	(wire
		(pts
			(xy 133.35 101.6) (xy 163.83 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9b45bdee-d319-48f1-b494-0b03cb55bfc4")
	)
	(wire
		(pts
			(xy 48.26 82.55) (xy 58.42 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9c97e57e-e72e-4d08-b4d4-1cd79a7f4b56")
	)
	(wire
		(pts
			(xy 62.23 62.23) (xy 72.39 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9d417cca-9798-46f1-80ab-3718bcc834fd")
	)
	(wire
		(pts
			(xy 247.65 104.14) (xy 260.35 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9d4dbdaa-3368-4c75-9cf4-4acc45f62dc4")
	)
	(wire
		(pts
			(xy 67.31 50.8) (xy 66.04 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9d61325c-a788-4b29-8c51-1e01c83712f9")
	)
	(wire
		(pts
			(xy 81.28 104.14) (xy 96.52 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9ee0bebe-2110-4523-8051-e92709c3deef")
	)
	(wire
		(pts
			(xy 179.07 102.87) (xy 209.55 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9f3f9614-32ed-4690-9b49-c6d6be3608a0")
	)
	(wire
		(pts
			(xy 40.64 104.14) (xy 40.64 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9fb275a0-4dfd-407d-8bc2-aef32f8ef6f1")
	)
	(wire
		(pts
			(xy 96.52 104.14) (xy 157.48 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9fbad939-2ce1-4da9-8063-fc9f8a474c83")
	)
	(wire
		(pts
			(xy 111.76 25.4) (xy 111.76 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a0530e2d-a677-4f0c-b6dc-c167346c5389")
	)
	(wire
		(pts
			(xy 209.55 44.45) (xy 209.55 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a3a52da6-d710-4703-85f6-a757621e2bc8")
	)
	(wire
		(pts
			(xy 187.96 25.4) (xy 203.2 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a4266bd8-6856-49aa-9a42-9c39a6034334")
	)
	(wire
		(pts
			(xy 209.55 63.5) (xy 209.55 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a4c4c1dc-101f-494b-88bf-9c6bd8ac3cb5")
	)
	(wire
		(pts
			(xy 87.63 123.19) (xy 87.63 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a5055de0-c258-4a66-b886-e51349372507")
	)
	(wire
		(pts
			(xy 238.76 44.45) (xy 238.76 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a5cdbd16-b38b-4d4e-8f8a-148e31137714")
	)
	(wire
		(pts
			(xy 219.71 90.17) (xy 218.44 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a5dbe626-cce0-46bd-8e7f-6cd6b45156c4")
	)
	(wire
		(pts
			(xy 247.65 50.8) (xy 247.65 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a772f698-95c9-43e8-bb16-3a6ad1b59867")
	)
	(wire
		(pts
			(xy 133.35 81.28) (xy 163.83 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a855fb09-dd96-4dbd-af18-802ebcff3e71")
	)
	(wire
		(pts
			(xy 157.48 31.75) (xy 158.75 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ab2e2a04-6746-46dd-b263-f3825bbc64b0")
	)
	(wire
		(pts
			(xy 172.72 69.85) (xy 173.99 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ab840cb3-5832-4966-bdc5-efd07eea733f")
	)
	(wire
		(pts
			(xy 118.11 63.5) (xy 148.59 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "acf44f6d-9059-4301-a294-16500ab38558")
	)
	(wire
		(pts
			(xy 262.89 31.75) (xy 262.89 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ad078bc5-53cf-49ac-9c0e-a3fa5d88af06")
	)
	(wire
		(pts
			(xy 87.63 44.45) (xy 118.11 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ade2db26-7e8c-406f-bd89-967fb8d6f776")
	)
	(wire
		(pts
			(xy 118.11 44.45) (xy 148.59 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ae3ec81d-7acd-46d4-9b39-5eae9e950e48")
	)
	(wire
		(pts
			(xy 97.79 31.75) (xy 96.52 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "af573d28-5f32-4f52-92a8-355b3e0c93f4")
	)
	(wire
		(pts
			(xy 102.87 43.18) (xy 102.87 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "af953e7e-3470-4764-b686-fd190bd381fb")
	)
	(wire
		(pts
			(xy 148.59 44.45) (xy 148.59 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "afb071ea-5f1c-4405-8549-7a5ea5b8ee0e")
	)
	(wire
		(pts
			(xy 224.79 101.6) (xy 224.79 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b0a3c773-7e26-458c-82a2-79cdddffed21")
	)
	(wire
		(pts
			(xy 232.41 31.75) (xy 232.41 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b0c1b22b-688e-49b1-b123-614b52e9869b")
	)
	(wire
		(pts
			(xy 172.72 31.75) (xy 173.99 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b1244f81-9d52-497a-9bec-08651e73ad06")
	)
	(wire
		(pts
			(xy 127 31.75) (xy 127 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b13d5779-7706-4f92-9fe9-92e829ae2ba7")
	)
	(wire
		(pts
			(xy 118.11 102.87) (xy 118.11 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b272dfbe-368e-48dd-866b-fb0af3b64d8c")
	)
	(wire
		(pts
			(xy 97.79 69.85) (xy 96.52 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b34c6dd2-a9f8-4866-83db-006a4c3d9f00")
	)
	(wire
		(pts
			(xy 194.31 101.6) (xy 194.31 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b421ba34-48dd-467a-8c7f-e1c448f5e00f")
	)
	(wire
		(pts
			(xy 224.79 101.6) (xy 254 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b4231e3a-b2fa-4757-a18e-1d561b1aaabc")
	)
	(wire
		(pts
			(xy 67.31 90.17) (xy 66.04 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b42e35c2-e7b3-49df-b15c-a07ea5c322be")
	)
	(wire
		(pts
			(xy 163.83 101.6) (xy 194.31 101.6)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b4b2dd49-dad5-42b7-b1b8-ee5ba97c6de3")
	)
	(wire
		(pts
			(xy 40.64 62.23) (xy 62.23 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b5743c94-4d3b-4c68-8bcb-b450b30c66b8")
	)
	(wire
		(pts
			(xy 133.35 43.18) (xy 133.35 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b5842382-ee39-4ab6-b20e-56f4e68b56cd")
	)
	(wire
		(pts
			(xy 67.31 69.85) (xy 66.04 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b7345224-0aca-4657-a62d-091ed27b43b6")
	)
	(wire
		(pts
			(xy 148.59 63.5) (xy 179.07 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b84eafc6-fb91-4702-85d1-e158c3f457bc")
	)
	(wire
		(pts
			(xy 59.69 127) (xy 209.55 127)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b8d501e6-d4e0-4114-a9b0-76b7844cb90a")
	)
	(wire
		(pts
			(xy 218.44 110.49) (xy 218.44 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b95a01aa-a778-461d-bd23-7c801b2a8856")
	)
	(wire
		(pts
			(xy 233.68 90.17) (xy 232.41 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b9efd96e-df0d-4bb2-9cee-1a7aed6e3bb2")
	)
	(wire
		(pts
			(xy 179.07 102.87) (xy 179.07 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "baebfcee-ba8c-41b3-8f42-5fd4c46c2a97")
	)
	(wire
		(pts
			(xy 102.87 81.28) (xy 133.35 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bb1f6547-b2fe-4a75-bcbe-b038d5a7d460")
	)
	(wire
		(pts
			(xy 187.96 90.17) (xy 187.96 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bb8de0ae-28d1-456a-a4d6-b197915eea42")
	)
	(wire
		(pts
			(xy 247.65 69.85) (xy 247.65 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bc91767a-8c61-4f80-bca0-dc94c055b860")
	)
	(wire
		(pts
			(xy 97.79 50.8) (xy 96.52 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bd72bbd7-6ab8-4b60-880e-96cfc2ee865f")
	)
	(wire
		(pts
			(xy 36.83 44.45) (xy 36.83 45.72)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bdc0cb19-d671-4079-8947-c31ebe0d8222")
	)
	(wire
		(pts
			(xy 232.41 25.4) (xy 218.44 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bf7b06a0-1813-4577-bd7f-a027eecb5318")
	)
	(wire
		(pts
			(xy 278.13 123.19) (xy 260.35 123.19)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c01301fe-b177-4a64-8305-4985c0f1da6e")
	)
	(wire
		(pts
			(xy 102.87 80.01) (xy 102.87 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c047a7b6-6259-46db-b882-119a9f5f5be9")
	)
	(wire
		(pts
			(xy 142.24 25.4) (xy 142.24 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c1230c60-1112-4d82-8991-d1abc69702bb")
	)
	(wire
		(pts
			(xy 157.48 50.8) (xy 157.48 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c18686d1-77fa-43e6-8065-b9b5d88bab78")
	)
	(wire
		(pts
			(xy 203.2 90.17) (xy 204.47 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c197389b-8bf8-47e2-8d8a-c1a00b234db3")
	)
	(wire
		(pts
			(xy 127 25.4) (xy 127 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c214cbd9-96d0-4986-9408-039c925c39b3")
	)
	(wire
		(pts
			(xy 48.26 104.14) (xy 48.26 102.87)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c2ab64eb-7cb4-4385-a59b-9a89df676e8d")
	)
	(wire
		(pts
			(xy 113.03 90.17) (xy 111.76 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c3754992-95b7-4339-81d9-1a521c3660cb")
	)
	(wire
		(pts
			(xy 158.75 110.49) (xy 157.48 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c38f3a21-338b-483a-8cbc-ab4500ba32e1")
	)
	(wire
		(pts
			(xy 87.63 60.96) (xy 87.63 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c45f602c-6fec-4014-a425-7b0e9f72063f")
	)
	(wire
		(pts
			(xy 209.55 127) (xy 209.55 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c473be19-7948-498d-b4ed-89fecfbabf37")
	)
	(wire
		(pts
			(xy 187.96 50.8) (xy 189.23 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c5050546-a289-4406-8f52-09179733dc20")
	)
	(wire
		(pts
			(xy 238.76 63.5) (xy 269.24 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c5dac9cb-9918-4112-ab45-5bbb7ff9e51e")
	)
	(wire
		(pts
			(xy 254 43.18) (xy 254 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c619861d-48a3-42b1-bc0f-47ee902d760d")
	)
	(wire
		(pts
			(xy 142.24 90.17) (xy 143.51 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cb01891a-6125-4ae1-a662-0a264fe2457c")
	)
	(wire
		(pts
			(xy 142.24 69.85) (xy 143.51 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cb5f18f7-8542-416e-afd9-272a936d26f8")
	)
	(wire
		(pts
			(xy 179.07 63.5) (xy 179.07 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cc50c151-4e76-417c-9bc0-d8b3536c44d8")
	)
	(wire
		(pts
			(xy 262.89 50.8) (xy 262.89 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cd0c40f6-2af3-4810-87a8-331eeeb4754c")
	)
	(wire
		(pts
			(xy 194.31 43.18) (xy 224.79 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cdb44bf7-637e-416e-bab5-070c9ef03509")
	)
	(wire
		(pts
			(xy 248.92 110.49) (xy 247.65 110.49)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cedb481f-3a32-4bc7-a4e1-f061e6a98db1")
	)
	(wire
		(pts
			(xy 247.65 90.17) (xy 247.65 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d09c762f-0aeb-48f6-bc4d-44f6951f8c2c")
	)
	(wire
		(pts
			(xy 142.24 69.85) (xy 142.24 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d1b94f69-66fd-4a8f-a1e7-a140d8a7bcc9")
	)
	(wire
		(pts
			(xy 224.79 81.28) (xy 224.79 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d258c358-0b8e-4dac-8677-c5cf0493efb5")
	)
	(wire
		(pts
			(xy 248.92 90.17) (xy 247.65 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d32b23ca-3cd7-4f41-af8a-ba9f75dfc9ba")
	)
	(wire
		(pts
			(xy 247.65 50.8) (xy 248.92 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d43af13d-342d-4646-8b00-d39b3c8656d1")
	)
	(wire
		(pts
			(xy 209.55 82.55) (xy 238.76 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d4b6a2ec-a81f-4e5d-930c-73597b85ca07")
	)
	(wire
		(pts
			(xy 36.83 83.82) (xy 35.56 83.82)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d4cda11b-ac1f-45c9-94ea-740e0cc4abd5")
	)
	(wire
		(pts
			(xy 218.44 90.17) (xy 218.44 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d74b4a26-97d4-49e2-92c7-cc0930e05d24")
	)
	(wire
		(pts
			(xy 163.83 62.23) (xy 194.31 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d7c01f0a-da8a-4142-8d2d-51b8d574d763")
	)
	(wire
		(pts
			(xy 209.55 82.55) (xy 209.55 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d8105ded-f8c6-4a44-bc20-a3c76f9b8fdc")
	)
	(wire
		(pts
			(xy 238.76 129.54) (xy 238.76 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d8b1ebc9-a7ab-4aec-9094-8f528f43e7d7")
	)
	(wire
		(pts
			(xy 63.5 44.45) (xy 87.63 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d9512cba-36ad-4126-bb0a-3d18071f71ff")
	)
	(wire
		(pts
			(xy 36.83 82.55) (xy 36.83 83.82)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "da21089c-57bf-4306-8b19-29b63dca421e")
	)
	(wire
		(pts
			(xy 96.52 25.4) (xy 111.76 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "da9991e1-464c-4e24-bd2b-7b95f88153c5")
	)
	(wire
		(pts
			(xy 66.04 69.85) (xy 66.04 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "db025c6d-eebb-493a-bae6-a2f9816c3962")
	)
	(wire
		(pts
			(xy 67.31 31.75) (xy 66.04 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "db09a1a7-802c-4064-933a-6891b1cd3b5d")
	)
	(wire
		(pts
			(xy 148.59 63.5) (xy 148.59 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "dbbfa617-f5cf-42a5-ae71-eb9d77d88edd")
	)
	(wire
		(pts
			(xy 48.26 44.45) (xy 36.83 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "dc2acf4c-e94c-4015-9fad-cd7462e4eab7")
	)
	(wire
		(pts
			(xy 179.07 82.55) (xy 209.55 82.55)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "dcea2fe0-99f6-4a10-a2bf-d601088790da")
	)
	(wire
		(pts
			(xy 247.65 31.75) (xy 247.65 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "dd496093-c937-411b-8cc3-85ca127f7642")
	)
	(wire
		(pts
			(xy 87.63 63.5) (xy 118.11 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "dd7f15b8-4985-49f1-8b21-74d91b009203")
	)
	(wire
		(pts
			(xy 64.77 121.92) (xy 64.77 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "deb792b9-4e4c-4122-b6d0-c02d19f6661b")
	)
	(wire
		(pts
			(xy 48.26 45.72) (xy 48.26 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e15137f2-149c-4e13-ae52-a6c1fe6037c3")
	)
	(wire
		(pts
			(xy 55.88 130.81) (xy 254 130.81)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e1cabfa7-1b2e-4ef7-971e-c5391e049118")
	)
	(wire
		(pts
			(xy 133.35 43.18) (xy 163.83 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e207c88b-af5a-4178-bb28-e1351494c35b")
	)
	(wire
		(pts
			(xy 133.35 101.6) (xy 133.35 100.33)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e237e52d-e6a1-4cf5-9d3b-d99a6253c0b9")
	)
	(wire
		(pts
			(xy 81.28 31.75) (xy 81.28 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e29a87c7-9059-4594-93da-aafa3a109497")
	)
	(wire
		(pts
			(xy 157.48 69.85) (xy 158.75 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e343c358-2e2d-4380-8efb-a570cdeb75da")
	)
	(wire
		(pts
			(xy 254 62.23) (xy 254 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e355b5e6-af3c-4a01-93d2-abfa14c2d057")
	)
	(wire
		(pts
			(xy 278.13 22.86) (xy 278.13 123.19)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e58ba72f-7138-4d32-8962-6c4e53d6d458")
	)
	(wire
		(pts
			(xy 224.79 43.18) (xy 224.79 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e630d733-45f5-461f-85f2-aca2236c439d")
	)
	(wire
		(pts
			(xy 238.76 44.45) (xy 269.24 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e80c1c6b-a12d-42bb-8130-978b52d1b8c5")
	)
	(wire
		(pts
			(xy 218.44 31.75) (xy 218.44 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e8f57621-0090-4549-93e8-848930acc668")
	)
	(wire
		(pts
			(xy 60.96 63.5) (xy 48.26 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e90493c6-f313-40a3-a661-aaa4d9b6b644")
	)
	(wire
		(pts
			(xy 203.2 69.85) (xy 203.2 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e97edc52-ae71-4797-8a30-e157a24b4571")
	)
	(wire
		(pts
			(xy 102.87 60.96) (xy 102.87 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ea666d93-0444-4beb-8690-3e9210faa907")
	)
	(wire
		(pts
			(xy 133.35 62.23) (xy 133.35 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ea875c9d-29a3-4ec3-b1b0-2f32817b298f")
	)
	(wire
		(pts
			(xy 224.79 81.28) (xy 254 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "eb9571df-0fe1-4361-9e18-67c993a9b3bb")
	)
	(wire
		(pts
			(xy 118.11 82.55) (xy 118.11 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ec510a7c-ea02-4987-8f85-eef558df708d")
	)
	(wire
		(pts
			(xy 62.23 124.46) (xy 62.23 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ec555c24-5d13-46b7-982f-228875f6424e")
	)
	(wire
		(pts
			(xy 118.11 44.45) (xy 118.11 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ed3396ea-f3cb-4220-8d76-0c9b043a8850")
	)
	(wire
		(pts
			(xy 36.83 45.72) (xy 35.56 45.72)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ef98bf5b-4598-4da7-9260-0da1b0267967")
	)
	(wire
		(pts
			(xy 36.83 102.87) (xy 36.83 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f012fed7-0974-4f40-866f-b874560e6ff1")
	)
	(wire
		(pts
			(xy 157.48 110.49) (xy 157.48 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f17f7bcf-a586-4c0f-afef-52a067dfe56a")
	)
	(wire
		(pts
			(xy 163.83 62.23) (xy 163.83 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f2f5c1c3-0ef0-4fa5-9d19-9e16d328d2c7")
	)
	(wire
		(pts
			(xy 262.89 110.49) (xy 262.89 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f36c12e6-9ec7-48de-970a-6cb2cb818120")
	)
	(wire
		(pts
			(xy 218.44 50.8) (xy 219.71 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f3b606c8-aead-4c6d-ab32-f909fa99c582")
	)
	(wire
		(pts
			(xy 36.83 63.5) (xy 36.83 64.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f414335a-1bde-4124-93b2-54059c4b4ad8")
	)
	(wire
		(pts
			(xy 113.03 69.85) (xy 111.76 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f4473bc2-57b6-4e6f-b8a6-132ca77c5412")
	)
	(wire
		(pts
			(xy 218.44 69.85) (xy 219.71 69.85)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f58c9082-3aee-4c1c-8972-2f24680fa3ba")
	)
	(wire
		(pts
			(xy 60.96 63.5) (xy 87.63 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f59cc689-ae71-42a6-8016-7e841255ad29")
	)
	(wire
		(pts
			(xy 35.56 81.28) (xy 40.64 81.28)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f5a5126f-a9e5-4993-a1ed-5a243cb1cb99")
	)
	(wire
		(pts
			(xy 179.07 44.45) (xy 179.07 41.91)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f6b8bce6-c521-4f07-9cb9-7f8fec7af702")
	)
	(wire
		(pts
			(xy 127 25.4) (xy 142.24 25.4)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f6d0e536-c6dd-4dee-a6b0-9255f0c37d18")
	)
	(wire
		(pts
			(xy 48.26 64.77) (xy 48.26 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f6ee6fe3-7899-4296-95b6-869cf8237bcc")
	)
	(wire
		(pts
			(xy 172.72 69.85) (xy 172.72 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f832c20b-ffac-4f73-9099-3dd9c5c56cde")
	)
	(wire
		(pts
			(xy 172.72 25.4) (xy 172.72 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f8d0e8c8-822c-4539-ad1f-caa7c499fd4d")
	)
	(wire
		(pts
			(xy 187.96 31.75) (xy 189.23 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f9b2435c-d881-4437-b749-22f0a0e03d26")
	)
	(wire
		(pts
			(xy 262.89 25.4) (xy 262.89 31.75)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fa131907-97b8-4127-b11a-e7ba21edaf60")
	)
	(wire
		(pts
			(xy 148.59 82.55) (xy 148.59 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fa3f84af-3a7a-4bda-9154-0e3b6e76981c")
	)
	(wire
		(pts
			(xy 194.31 62.23) (xy 224.79 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fc4e802f-9b56-43cf-9152-c6f8b063ca7e")
	)
	(wire
		(pts
			(xy 96.52 110.49) (xy 96.52 104.14)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fc72f9fa-6f94-41aa-8d43-5653ee6b82bc")
	)
	(wire
		(pts
			(xy 187.96 69.85) (xy 187.96 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fe2db663-7f89-4e54-b4fb-c4e81b0d35f9")
	)
	(wire
		(pts
			(xy 232.41 50.8) (xy 233.68 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fe59020e-da23-486f-9e25-6df660e98520")
	)
	(wire
		(pts
			(xy 179.07 44.45) (xy 209.55 44.45)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ffb210f3-d7b5-47e0-b40e-b9626a44f1ff")
	)
	(text "open source\n  hardware"
		(exclude_from_sim no)
		(at 271.78 180.975 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "eab43f83-dc19-4469-b77a-a9e42551c20c")
	)
	(global_label "DOUT"
		(shape output)
		(at 96.52 148.59 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "08319e59-820c-4be7-958d-229170df721d")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 96.52 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "DOUT"
		(shape input)
		(at 185.42 157.48 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "0d440459-1f95-48f4-8d79-6525238edc7a")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 185.42 157.48 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "NEXTCOL"
		(shape output)
		(at 172.72 154.94 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "1983b805-e917-44cf-a567-b0c12ae2bee1")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 172.72 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW0"
		(shape output)
		(at 35.56 43.18 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "1ad7040f-8451-407d-b95e-b97144031918")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 43.18 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "GND"
		(shape output)
		(at 172.72 160.02 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "1b60d851-efc0-4d59-b289-98bc7bfabdaa")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 172.72 160.02 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "NEXTCOL"
		(shape input)
		(at 26.67 148.59 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "1de80a50-7ea6-43de-9c02-f7f9427e164c")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 26.67 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "GND"
		(shape input)
		(at 26.67 151.13 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "414758be-3c33-43d6-8bb9-9cd419339cf3")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 26.67 151.13 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW3"
		(shape input)
		(at 71.12 158.75 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "452d319f-4b4e-40a7-9de7-3d6f755d8afa")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 158.75 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW2"
		(shape output)
		(at 35.56 62.23 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "46a404dd-c1ad-451d-8ff5-c422956c3562")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 62.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL7"
		(shape input)
		(at 278.13 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "49db6def-7751-4b2b-bb11-e9a9b74e6497")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 278.13 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW6"
		(shape input)
		(at 71.12 166.37 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "4ce4d20e-0df9-4f94-a57d-8cbe7880cbf1")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 166.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW4"
		(shape output)
		(at 35.56 81.28 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "4d603c27-b917-472b-992d-1812c7e8d039")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 81.28 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL3"
		(shape input)
		(at 157.48 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "4fe8d3ad-d65f-40f3-8f0e-7e1fe9057f64")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 157.48 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "GND"
		(shape input)
		(at 71.12 181.61 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "51a007a6-825a-47be-adeb-2bda0168a2a6")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 181.61 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL4"
		(shape input)
		(at 187.96 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "60b02826-4bf8-46b3-932b-68b99ad968ab")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 187.96 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "GND"
		(shape input)
		(at 71.12 148.59 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "638b282d-fdc4-4ec2-809c-5ae9dfa9b4c9")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "+5V"
		(shape output)
		(at 172.72 152.4 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "646ddf90-34a6-49c2-a7bb-e202b5932879")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 172.72 152.4 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "LOAD#"
		(shape input)
		(at 71.12 173.99 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "65482684-be7f-4de7-a2ce-04902b6bb55f")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 173.99 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL1"
		(shape output)
		(at 52.07 151.13 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "65576d89-a4f2-4adc-991e-ea918bf21650")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 151.13 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL4"
		(shape output)
		(at 52.07 158.75 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "66d42aa5-e3fa-4995-843d-5e731b9e8520")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 158.75 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW4"
		(shape input)
		(at 71.12 161.29 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "69f411ef-f8cf-4d35-ad75-dabff5ed90a8")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW5"
		(shape output)
		(at 35.56 83.82 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "6c72bd80-56e3-470e-b7f2-806b15ecbfa4")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 83.82 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL6"
		(shape output)
		(at 52.07 163.83 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "6e799713-f1ef-4d2e-9c05-585d84318746")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 163.83 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW2"
		(shape input)
		(at 71.12 156.21 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "74006b9c-dc1f-456d-bea1-1fd82824eb15")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 156.21 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW3"
		(shape output)
		(at 35.56 64.77 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "76545c6f-04fc-46b7-978d-4424eb7609c0")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 64.77 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL0"
		(shape input)
		(at 66.04 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "7800fd0a-b928-4c50-bcce-0c507b9edc2e")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 66.04 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "SHCLK"
		(shape output)
		(at 172.72 157.48 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "79166368-6079-4d51-92e1-7c7212166139")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 172.72 157.48 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW5"
		(shape input)
		(at 71.12 163.83 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "86345efa-73d1-4aec-84ad-a1955e8839e4")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 163.83 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "SHCLK"
		(shape input)
		(at 71.12 179.07 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "8a5be3c5-229c-4f8e-9e79-6991125a05f6")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 179.07 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL6"
		(shape input)
		(at 247.65 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "92a182de-40e5-4d9e-86df-89b5f53d1fd5")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 247.65 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "FIRSTCOL"
		(shape output)
		(at 185.42 152.4 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "95f89328-3a63-41b4-ab84-c08d48502d6f")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 185.42 152.4 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW7"
		(shape input)
		(at 71.12 168.91 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "a8129142-085c-41d6-8a22-ef0b7c5a57cf")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 168.91 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL2"
		(shape input)
		(at 127 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "b330c7dd-0d80-4f6a-84bd-b642adf62a66")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 127 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "FIRSTCOL"
		(shape input)
		(at 26.67 156.21 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "b4515cd1-d613-43ed-b2ea-589bf7988cb3")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 26.67 156.21 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "GND"
		(shape output)
		(at 185.42 160.02 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "b7b8f45e-b960-4d2d-9555-e11631799ab7")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 185.42 160.02 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL2"
		(shape output)
		(at 52.07 153.67 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "bab052a5-0452-483f-a4a1-57f51809a9d8")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 153.67 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW0"
		(shape input)
		(at 71.12 151.13 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "bb6fcb8d-7455-4d06-910f-e1968ced1f7b")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 151.13 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL5"
		(shape input)
		(at 218.44 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "bd28d675-7b3c-42fc-936e-bda380685973")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 218.44 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW7"
		(shape output)
		(at 35.56 104.14 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "bde606d2-1ff2-4096-9ff1-6d74ade9e458")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 104.14 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL7"
		(shape output)
		(at 52.07 166.37 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "bf0ab71a-9d84-4ff8-91e0-c3c3e3957988")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 166.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW1"
		(shape input)
		(at 71.12 153.67 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "c8ea4a9a-ec9a-4278-bb3c-4f586e85b134")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 71.12 153.67 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL3"
		(shape output)
		(at 52.07 156.21 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "ce6187e7-bef0-4b7d-a70e-a1d8b3780e6c")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 156.21 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "LOAD#"
		(shape output)
		(at 185.42 154.94 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "d062a813-dc2d-43c9-bab6-c0b72733102d")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 185.42 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW6"
		(shape output)
		(at 35.56 101.6 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "d38c032b-a706-4261-8c98-fe315a87042f")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 101.6 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "ROW1"
		(shape output)
		(at 35.56 45.72 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right)
		)
		(uuid "d92103c9-9de0-4516-b9f6-cfc4a51c70bd")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 35.56 45.72 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL0"
		(shape output)
		(at 52.07 148.59 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "e0f13dfc-a3a1-4581-b730-73281db0b7c2")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL1"
		(shape input)
		(at 96.52 22.86 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "e76127f3-c828-4ffb-aa4a-8b3a67f4609b")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 96.52 22.86 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(global_label "COL5"
		(shape output)
		(at 52.07 161.29 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left)
		)
		(uuid "f8dd0055-457b-478f-a788-c7ece3d19184")
		(property "Intersheetrefs" "${INTERSHEET_REFS}"
			(at 52.07 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 69.85 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de23676")
		(property "Reference" "SW1"
			(at 69.85 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "ESC"
			(at 69.85 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 69.85 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 69.85 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 69.85 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "841b01bc-15d4-43a1-b2e2-33a1d27dd719")
		)
		(pin "1"
			(uuid "f22abb53-231c-4115-816c-390b99918efb")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 85.09 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de253c5")
		(property "Reference" "SW2"
			(at 85.09 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "1"
			(at 85.09 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 85.09 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 85.09 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 85.09 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "bf31e2d6-f3f9-432a-94fa-b6b69ac7aaf2")
		)
		(pin "2"
			(uuid "63ade359-45af-4124-b380-e0c1687c8e9c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 100.33 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de25903")
		(property "Reference" "SW3"
			(at 100.33 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "2"
			(at 100.33 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 100.33 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 100.33 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 100.33 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "51b04571-56ba-40fa-93bc-faf9148ad56e")
		)
		(pin "1"
			(uuid "3313023f-84ff-48d7-9478-085d5d002c24")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 115.57 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de25ef6")
		(property "Reference" "SW4"
			(at 115.57 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "3"
			(at 115.57 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 115.57 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 115.57 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 115.57 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "0ce7a7ab-459f-4ce8-bde0-3d3874588095")
		)
		(pin "2"
			(uuid "c87a032b-6ea8-4542-b498-0b432b7e10a6")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 130.81 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de26659")
		(property "Reference" "SW5"
			(at 130.81 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "4"
			(at 130.81 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 130.81 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 130.81 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 130.81 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "93b4b271-4ad1-447e-a4d6-11944462a189")
		)
		(pin "2"
			(uuid "02bf65ea-0eb0-462c-8dd5-9f1254eba687")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 146.05 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de26bb7")
		(property "Reference" "SW6"
			(at 146.05 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "5"
			(at 146.05 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 146.05 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 146.05 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 146.05 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "a3f288bd-f1e8-492b-b921-c593cdb83b38")
		)
		(pin "2"
			(uuid "303f3d05-4237-43ef-a7a6-38663d8c4eca")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 161.29 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de27236")
		(property "Reference" "SW7"
			(at 161.29 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "6"
			(at 161.29 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 161.29 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 161.29 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 161.29 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "48802449-f26b-45ca-86dd-1a5490c7df37")
		)
		(pin "2"
			(uuid "2657c735-557e-47b9-abe3-3db83e4369c6")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 176.53 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de2778f")
		(property "Reference" "SW8"
			(at 176.53 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "7"
			(at 176.53 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 176.53 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 176.53 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 176.53 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "4ff5d002-e20d-4636-87c1-102eb20c2b51")
		)
		(pin "1"
			(uuid "06481891-73c8-42b7-8853-f95d2a0ec8a0")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW8")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 191.77 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de30592")
		(property "Reference" "SW9"
			(at 191.77 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "8"
			(at 191.77 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 191.77 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 191.77 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 191.77 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "d2802910-d2d7-476a-ba73-fd657bf02285")
		)
		(pin "1"
			(uuid "8657adac-9a1b-40b7-9d61-1ae9da421343")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW9")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 207.01 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de30598")
		(property "Reference" "SW10"
			(at 207.01 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "9"
			(at 207.01 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 207.01 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 207.01 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 207.01 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "bde76ae8-821a-459b-9971-39dc4c186093")
		)
		(pin "2"
			(uuid "e0aaa0ab-b49a-4ab0-b485-e39199d23516")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW10")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 222.25 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de3059e")
		(property "Reference" "SW11"
			(at 222.25 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "0"
			(at 222.25 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 222.25 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 222.25 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 222.25 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "fc0f1fb2-120a-4d38-8ee1-a4fa1a194968")
		)
		(pin "2"
			(uuid "3e5b76cd-a3fd-4739-ba0c-1ea99fb32148")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW11")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 236.22 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de305a4")
		(property "Reference" "SW12"
			(at 236.22 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "-"
			(at 236.22 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 236.22 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 236.22 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 236.22 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "eecf2c1a-129d-40ff-95fd-1a7c784d414c")
		)
		(pin "1"
			(uuid "5e4f7fe8-4b4a-44e1-88e2-8aeda21833e9")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW12")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 251.46 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de305aa")
		(property "Reference" "SW13"
			(at 251.46 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "="
			(at 251.46 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 251.46 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 251.46 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 251.46 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "981c4463-6239-4bfb-a8f9-a956c238d7a4")
		)
		(pin "2"
			(uuid "86afe789-084e-4a75-b7c4-99abdc664158")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW13")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 266.7 34.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005de305b0")
		(property "Reference" "SW14"
			(at 266.7 27.1526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "BS"
			(at 266.7 29.464 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_2.00u_PCB"
			(at 266.7 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 34.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "cf154405-69a5-4eae-8105-491184c1381e")
		)
		(pin "1"
			(uuid "0ec1d9cb-5790-45cf-99d4-b9fc822b987d")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW14")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 69.85 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7957")
		(property "Reference" "SW15"
			(at 69.85 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "TAB"
			(at 69.85 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.50u_PCB"
			(at 69.85 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 69.85 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 69.85 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "a7e4722c-8f67-4cfb-aa10-287c7d8bfa6b")
		)
		(pin "2"
			(uuid "728551f2-1f56-42da-8afb-1eb321c205b0")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW15")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 85.09 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb795d")
		(property "Reference" "SW16"
			(at 85.09 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "Q"
			(at 85.09 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 85.09 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 85.09 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 85.09 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "a0583cea-cc73-493d-91ef-2d61d584af90")
		)
		(pin "1"
			(uuid "bbf0ccb3-1ba8-4188-9bf0-9086957cb67c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW16")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 100.33 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7963")
		(property "Reference" "SW17"
			(at 100.33 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "W"
			(at 100.33 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 100.33 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 100.33 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 100.33 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "421bcf1b-baf9-478c-9271-482fefc284b0")
		)
		(pin "1"
			(uuid "959c32c4-10de-42a2-81a2-635df4c19402")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW17")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 115.57 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7969")
		(property "Reference" "SW18"
			(at 115.57 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "E"
			(at 115.57 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 115.57 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 115.57 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 115.57 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "206da7ae-2d8b-4bd6-a60f-08c176d34b17")
		)
		(pin "2"
			(uuid "91681a26-c8a4-4ea9-888d-4a2b47c1ecc6")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW18")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 146.05 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb796f")
		(property "Reference" "SW20"
			(at 146.05 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "T"
			(at 146.05 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 146.05 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 146.05 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 146.05 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7cfe88c9-ea36-4cb4-b942-5c056463b5d9")
		)
		(pin "2"
			(uuid "958ac817-4741-426b-854f-48fdd3a2873e")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW20")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 161.29 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7975")
		(property "Reference" "SW21"
			(at 161.29 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "Y"
			(at 161.29 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 161.29 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 161.29 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 161.29 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "64301e93-7c49-43c7-9b8a-cc0f13153bd6")
		)
		(pin "2"
			(uuid "5106b756-72af-4bbe-bc19-f49093f895d2")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW21")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 176.53 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb797b")
		(property "Reference" "SW22"
			(at 176.53 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "U"
			(at 176.53 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 176.53 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 176.53 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 176.53 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "ce8b5135-5a1a-4d07-9f14-46c10966bfaa")
		)
		(pin "1"
			(uuid "eda57259-c5b1-4f75-bad3-06bdb3ccbd56")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW22")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 191.77 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7981")
		(property "Reference" "SW23"
			(at 191.77 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "I"
			(at 191.77 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 191.77 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 191.77 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 191.77 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "cd45842e-75bb-45fa-92d6-774c492ffc98")
		)
		(pin "2"
			(uuid "95de4978-57f3-442d-b510-8a9f02d08cee")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW23")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 207.01 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7987")
		(property "Reference" "SW24"
			(at 207.01 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "O"
			(at 207.01 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 207.01 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 207.01 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 207.01 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "eefdf45e-d17a-4db4-8bb6-174147b29c3c")
		)
		(pin "2"
			(uuid "d41540b2-2442-4120-bd4a-83f9d2956dab")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW24")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 222.25 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb798d")
		(property "Reference" "SW25"
			(at 222.25 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "P"
			(at 222.25 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 222.25 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 222.25 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 222.25 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "010d0f6a-0bd4-441a-b7f0-be7f3c52eaf4")
		)
		(pin "2"
			(uuid "2b745930-cf94-4733-9ce3-810dc4a4602f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW25")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 236.22 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7993")
		(property "Reference" "SW26"
			(at 236.22 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "["
			(at 236.22 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 236.22 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 236.22 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 236.22 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "a9fb1c8b-571b-446c-a87d-41897f204d07")
		)
		(pin "1"
			(uuid "6e19b3aa-cad4-48e2-ac23-6c652aad8db1")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW26")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 251.46 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb7999")
		(property "Reference" "SW27"
			(at 251.46 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "]"
			(at 251.46 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 251.46 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 251.46 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 251.46 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "040f54c0-8f13-4fec-bb23-80724551aa2c")
		)
		(pin "2"
			(uuid "f4146208-9825-4490-9dd1-8e180c87a945")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW27")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 266.7 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb799f")
		(property "Reference" "SW28"
			(at 266.7 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "\\"
			(at 266.7 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.50u_PCB"
			(at 266.7 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "4517bce3-643e-4e21-b192-694fd64966e5")
		)
		(pin "2"
			(uuid "28149cbb-76fe-45c7-a715-19ad3a260bea")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW28")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 130.81 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005deb79a5")
		(property "Reference" "SW19"
			(at 130.81 46.2026 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "R"
			(at 130.81 48.514 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 130.81 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 130.81 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 130.81 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "d4f5c87e-5e54-4150-82af-419e4df94f40")
		)
		(pin "1"
			(uuid "fef16638-c024-499b-8e47-4498c3a3198f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW19")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 69.85 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf926")
		(property "Reference" "SW29"
			(at 69.85 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "HYPER"
			(at 69.85 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.75u_PCB"
			(at 69.85 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 69.85 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 69.85 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "88ebdd8f-1c7e-4ddf-94da-a35df7596e1f")
		)
		(pin "2"
			(uuid "337708ae-1cec-4cd6-8eba-231d6d419cdd")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW29")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 85.09 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf92c")
		(property "Reference" "SW30"
			(at 85.09 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "A"
			(at 85.09 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 85.09 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 85.09 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 85.09 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "ddaaf9ed-d17c-48f3-8e0d-61c6fef58fc8")
		)
		(pin "2"
			(uuid "8efb09ea-c2c0-4c37-a7c5-43f7f6087a26")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW30")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 100.33 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf932")
		(property "Reference" "SW31"
			(at 100.33 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "S"
			(at 100.33 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 100.33 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 100.33 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 100.33 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "cb4a8e62-d201-4531-b5dd-91e97b9c7dc8")
		)
		(pin "2"
			(uuid "524815c8-cc37-46d5-9b35-18bbbf55728f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW31")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 115.57 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf938")
		(property "Reference" "SW32"
			(at 115.57 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "D"
			(at 115.57 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 115.57 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 115.57 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 115.57 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "c56d370e-5e5f-4206-8563-63922dccb797")
		)
		(pin "2"
			(uuid "e3fd0324-dd74-4871-af47-a6024592392c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW32")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 146.05 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf93e")
		(property "Reference" "SW34"
			(at 146.05 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "G"
			(at 146.05 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 146.05 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 146.05 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 146.05 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "17af88fb-5ee6-4e67-905f-eff39549d9b9")
		)
		(pin "1"
			(uuid "0fed67ad-ae3d-4a60-b69e-dacf96873769")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW34")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 161.29 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf944")
		(property "Reference" "SW35"
			(at 161.29 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "H"
			(at 161.29 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 161.29 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 161.29 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 161.29 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "b5a8ccfe-753d-487f-a2e5-d9228b302af6")
		)
		(pin "2"
			(uuid "9d41239b-a398-4bbd-9458-70715e4014c0")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW35")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 176.53 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf94a")
		(property "Reference" "SW36"
			(at 176.53 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "J"
			(at 176.53 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 176.53 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 176.53 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 176.53 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "71ca59e1-7e03-497c-963f-1119668d3d9d")
		)
		(pin "1"
			(uuid "fbb9c594-fda2-4707-9f11-ba89802bd840")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW36")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 191.77 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf950")
		(property "Reference" "SW37"
			(at 191.77 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "K"
			(at 191.77 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 191.77 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 191.77 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 191.77 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "eea2d6a1-9004-4c93-8f47-7123bf31a715")
		)
		(pin "1"
			(uuid "066fee70-cc48-4c2a-8b68-333198bdcb82")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW37")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 207.01 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf956")
		(property "Reference" "SW38"
			(at 207.01 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "L"
			(at 207.01 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 207.01 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 207.01 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 207.01 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "80c4f29f-2baf-4c77-ba35-bb7409b106b6")
		)
		(pin "2"
			(uuid "af081a1f-7d2a-4f31-8288-62d3b949e421")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW38")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 222.25 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf95c")
		(property "Reference" "SW39"
			(at 222.25 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" ";"
			(at 222.25 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 222.25 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 222.25 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 222.25 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "8d67d2ec-c572-43fe-9ec2-667fd4db0116")
		)
		(pin "1"
			(uuid "96a01f93-49a3-4fe7-a446-9557763da8ba")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW39")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 236.22 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf962")
		(property "Reference" "SW40"
			(at 236.22 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "'"
			(at 236.22 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 236.22 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 236.22 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 236.22 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "341e245a-dd05-458c-841e-9d1817a8b58d")
		)
		(pin "2"
			(uuid "feeb643f-6186-4165-9b80-87540c8e4f38")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW40")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 251.46 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf96e")
		(property "Reference" "SW41"
			(at 251.46 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "ENTER"
			(at 251.46 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_2.25u_PCB"
			(at 251.46 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 251.46 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 251.46 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "cbe3866f-cda9-4ba0-84b5-54e64c30483b")
		)
		(pin "1"
			(uuid "3ef379a0-f21f-4b9d-8228-ce1800d4960c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW41")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 130.81 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005debf974")
		(property "Reference" "SW33"
			(at 130.81 65.2526 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "F"
			(at 130.81 67.564 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 130.81 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 130.81 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 130.81 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "3fb7da0e-92ec-4a49-8d95-b91bc096bb5c")
		)
		(pin "1"
			(uuid "a83f0648-7c3e-4afc-ae6f-4f05c58f83c0")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW33")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 69.85 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42b4")
		(property "Reference" "SW42"
			(at 69.85 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "SHIFT"
			(at 69.85 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_2.00u_PCB"
			(at 69.85 87.9094 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 69.85 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 69.85 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "1c468d48-db17-46d1-882b-7bdb795cfef4")
		)
		(pin "1"
			(uuid "24a32309-9c26-448a-b228-aaf0fe0cf707")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW42")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 85.09 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42ba")
		(property "Reference" "SW43"
			(at 85.09 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "Z"
			(at 85.09 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 85.09 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 85.09 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 85.09 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "d09e1de3-ffdd-41eb-a2ee-0413345372e7")
		)
		(pin "1"
			(uuid "b55ff62d-8d44-490a-a4eb-c5f02b3cdfdc")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW43")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 100.33 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42c0")
		(property "Reference" "SW44"
			(at 100.33 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "X"
			(at 100.33 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 100.33 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 100.33 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 100.33 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "12ee5470-38db-4b1e-9b6d-f98c72859b42")
		)
		(pin "2"
			(uuid "2decdd4d-6180-4d49-9a6a-74f90f5c322a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW44")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 115.57 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42c6")
		(property "Reference" "SW45"
			(at 115.57 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "C"
			(at 115.57 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 115.57 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 115.57 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 115.57 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "2d441bcc-d2b7-4f6b-a1ea-c43e87a3ffd1")
		)
		(pin "2"
			(uuid "a6ca791f-c2af-4446-b582-a6909a986d0e")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW45")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 146.05 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42cc")
		(property "Reference" "SW47"
			(at 146.05 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "B"
			(at 146.05 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 146.05 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 146.05 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 146.05 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "1d849fe4-5870-43f9-9c33-651c8b433d22")
		)
		(pin "1"
			(uuid "352cb7b9-6fc5-4102-8e26-27d02b890cf1")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW47")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 161.29 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42d2")
		(property "Reference" "SW48"
			(at 161.29 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "N"
			(at 161.29 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 161.29 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 161.29 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 161.29 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "5ed6193c-0e81-4e4b-918c-f2ef24e3da28")
		)
		(pin "2"
			(uuid "2fa07792-433d-4e9d-94f0-4c9945b86c00")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW48")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 176.53 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42d8")
		(property "Reference" "SW49"
			(at 176.53 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "M"
			(at 176.53 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 176.53 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 176.53 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 176.53 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "ce3f9e7c-4dd7-4f01-9b52-a8a9ebc20c8f")
		)
		(pin "1"
			(uuid "c2d8f032-0aea-4739-a66f-73e95e51226d")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW49")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 191.77 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42de")
		(property "Reference" "SW50"
			(at 191.77 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" ","
			(at 191.77 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 191.77 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 191.77 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 191.77 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "838f2a72-1072-4f73-9cc4-b0eb6d4ea7e5")
		)
		(pin "2"
			(uuid "1d9c5c53-1bd6-4365-931e-ac01b3b561a4")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW50")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 207.01 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42e4")
		(property "Reference" "SW51"
			(at 207.01 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "."
			(at 207.01 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 207.01 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 207.01 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 207.01 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7e16a841-06b6-48c4-98e3-f1d0452b33cf")
		)
		(pin "2"
			(uuid "0c551c94-b2eb-4e17-bfbe-797600fe8395")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW51")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 222.25 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42ea")
		(property "Reference" "SW52"
			(at 222.25 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "/"
			(at 222.25 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 222.25 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 222.25 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 222.25 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "203d412a-dfe6-45a1-9f59-7e60db89a240")
		)
		(pin "2"
			(uuid "0fc241fc-9998-4d26-bd9e-579195b92139")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW52")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 236.22 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec42f0")
		(property "Reference" "SW53"
			(at 236.22 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "SHIFT"
			(at 236.22 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 236.22 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 236.22 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 236.22 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "7c4a7958-b947-45b4-9ba3-869891959fdd")
		)
		(pin "1"
			(uuid "367dc419-ee8d-418e-a00c-aff1561de499")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW53")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 130.81 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec4302")
		(property "Reference" "SW46"
			(at 130.81 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "V"
			(at 130.81 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 130.81 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 130.81 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 130.81 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "decd7488-1a5d-4094-84a9-48876960102a")
		)
		(pin "1"
			(uuid "dad17788-1604-44e6-9c00-4dc29c2eed32")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW46")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 69.85 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec8fdd")
		(property "Reference" "SW56"
			(at 69.85 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "CTRL"
			(at 69.85 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.25u_PCB"
			(at 69.85 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 69.85 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 69.85 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "b9ee8b66-d09d-4a6a-a316-78a820dbab0f")
		)
		(pin "1"
			(uuid "c3b5848f-1e64-45fa-9548-3eef2cb95e21")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW56")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 85.09 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec8fe3")
		(property "Reference" "SW57"
			(at 85.09 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "SUPER"
			(at 85.09 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.25u_PCB"
			(at 85.09 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 85.09 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 85.09 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "1eded671-0dcc-43d6-ac57-d8150328911a")
		)
		(pin "1"
			(uuid "1e3d2f86-afd1-46e3-983f-0195aad1ef27")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW57")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 100.33 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec8fe9")
		(property "Reference" "SW58"
			(at 100.33 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "ALT"
			(at 100.33 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.25u_PCB"
			(at 100.33 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 100.33 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 100.33 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "9b0e3af6-f255-4a39-bd71-32f0ba698444")
		)
		(pin "1"
			(uuid "3ccf423f-ddf7-4818-b1ab-6af1cd8c2d30")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW58")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 161.29 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec8fef")
		(property "Reference" "SW59"
			(at 161.29 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "SPACE"
			(at 161.29 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_6.25u_PCB"
			(at 161.29 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 161.29 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 161.29 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "ccd8331d-9229-4a31-9395-d56f99eac2b7")
		)
		(pin "1"
			(uuid "39829bec-cf16-4adf-9b6a-9c42de5f0f35")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW59")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 207.01 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005dec8ffb")
		(property "Reference" "SW60"
			(at 207.01 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "FN"
			(at 207.01 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 207.01 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 207.01 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 207.01 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "346e6816-bd3f-4217-bf0b-99fea63e3d2e")
		)
		(pin "2"
			(uuid "f648c3f9-f571-49fb-bcef-925d9f1d0ffa")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW60")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 251.46 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005def0cbb")
		(property "Reference" "SW54"
			(at 251.46 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "UP"
			(at 251.46 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 251.46 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 251.46 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 251.46 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "b59a49af-6c35-4dfb-8610-189e9d3d968f")
		)
		(pin "1"
			(uuid "41febe8f-bda4-4bf8-939c-bd5170783d48")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW54")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 251.46 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005def159c")
		(property "Reference" "SW63"
			(at 251.46 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "DOWN"
			(at 251.46 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 251.46 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 251.46 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 251.46 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "423df335-8863-4004-9422-dcf276a67240")
		)
		(pin "1"
			(uuid "de411cf1-baad-432b-9b01-78f522bdbdd3")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW63")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 236.22 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005def1e0d")
		(property "Reference" "SW62"
			(at 236.22 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "LEFT"
			(at 236.22 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 236.22 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 236.22 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 236.22 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "4647a3d7-9818-48cc-b6d0-e6e5b045a3eb")
		)
		(pin "1"
			(uuid "5daca7ff-649f-486f-8a9f-0303a30d2dc9")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW62")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 266.7 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005def24ea")
		(property "Reference" "SW64"
			(at 266.7 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "RIGHT"
			(at 266.7 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 266.7 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "6dd2cd08-8109-4fa6-8676-5a2ccf72f117")
		)
		(pin "2"
			(uuid "92e52207-31da-40d1-b31c-eee151c2c123")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW64")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 222.25 113.03 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005df0ccc4")
		(property "Reference" "SW61"
			(at 222.25 105.8926 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "CTRL"
			(at 222.25 108.204 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 222.25 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 222.25 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 222.25 113.03 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "17400650-8b94-48a7-9484-4e5071f7ec57")
		)
		(pin "2"
			(uuid "f6d5f81c-647d-42f3-9445-7426928e7d2f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW61")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push_45deg")
		(at 266.7 92.71 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005df58132")
		(property "Reference" "SW55"
			(at 266.7 85.5726 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "DELETE"
			(at 266.7 87.884 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_Keyboard:SW_Cherry_MX1A_1.00u_PCB"
			(at 266.7 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 92.71 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "0680aa5a-fcbf-45b4-82e0-d414b2b2ff68")
		)
		(pin "1"
			(uuid "e10fd325-62f7-4467-b190-9d9847aff6b7")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "SW55")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 266.7 148.59 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005e12d541")
		(property "Reference" "H10"
			(at 269.24 147.4216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 269.24 149.733 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 266.7 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H10")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 266.7 154.94 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005e12d547")
		(property "Reference" "H11"
			(at 269.24 153.7716 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 269.24 156.083 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 266.7 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H11")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 266.7 161.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005e12d54d")
		(property "Reference" "H12"
			(at 269.24 160.1216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 269.24 162.433 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 266.7 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H12")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 203.2 148.59 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005e9ffce5")
		(property "Reference" "H1"
			(at 205.74 147.4216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 205.74 149.733 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 203.2 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 203.2 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 203.2 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 203.2 154.94 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea00f5b")
		(property "Reference" "H3"
			(at 205.74 153.7716 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 205.74 156.083 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 203.2 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 203.2 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 203.2 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 203.2 161.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea0126f")
		(property "Reference" "H5"
			(at 205.74 160.1216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 205.74 162.433 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 203.2 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 203.2 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 203.2 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 223.52 148.59 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea02548")
		(property "Reference" "H2"
			(at 226.06 147.4216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 226.06 149.733 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 223.52 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 223.52 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 223.52 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 223.52 154.94 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea0254e")
		(property "Reference" "H4"
			(at 226.06 153.7716 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 226.06 156.083 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 223.52 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 223.52 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 223.52 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 223.52 161.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea02554")
		(property "Reference" "H6"
			(at 226.06 160.1216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 226.06 162.433 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 223.52 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 223.52 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 223.52 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 245.11 148.59 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea4b0da")
		(property "Reference" "H7"
			(at 247.65 147.4216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 247.65 149.733 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 245.11 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 245.11 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 245.11 148.59 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 245.11 154.94 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea4b4e2")
		(property "Reference" "H8"
			(at 247.65 153.7716 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 247.65 156.083 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 245.11 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 245.11 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 245.11 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H8")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 245.11 161.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00005ea4b7b6")
		(property "Reference" "H9"
			(at 247.65 160.1216 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 247.65 162.433 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "MountingHole:MountingHole_3.2mm_M3"
			(at 245.11 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 245.11 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 245.11 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H9")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 72.39 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612201d5")
		(property "Reference" "D1"
			(at 74.168 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 74.168 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 72.39 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 72.39 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 72.39 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "c9584dc4-3f9d-4b1d-8ced-e58da2c10908")
		)
		(pin "2"
			(uuid "4bf4ef7f-31fe-4205-a43e-9632fd1ff900")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Graphic:Logo_Open_Hardware_Small")
		(at 278.13 172.085 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006122cc87")
		(property "Reference" "LOGO2"
			(at 278.13 165.1 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "Logo_Open_Hardware_Small"
			(at 278.13 177.8 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Footprint" "Symbol:OSHW-Logo_11.4x12mm_SilkScreen"
			(at 278.13 172.085 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 278.13 172.085 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 278.13 172.085 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "LOGO2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 87.63 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061282df8")
		(property "Reference" "D2"
			(at 89.408 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 89.408 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 87.63 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 87.63 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 87.63 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "1198c3b6-0191-4f96-ba7b-af06c844a7b5")
		)
		(pin "2"
			(uuid "06ba2675-4d8d-4b39-8383-dff7a6517aef")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 102.87 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006128336a")
		(property "Reference" "D3"
			(at 104.648 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 104.648 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 102.87 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 102.87 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 102.87 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "3d258ac9-ece1-4429-ba08-fd45844014ea")
		)
		(pin "1"
			(uuid "8082915c-a619-4365-be6d-061cdbcc2ca6")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 118.11 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061283b1d")
		(property "Reference" "D4"
			(at 119.888 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 119.888 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 118.11 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 118.11 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 118.11 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "5762fced-bc33-4628-aaa1-0074b1bab385")
		)
		(pin "2"
			(uuid "8bc62b62-1fb8-4cf8-a022-5e30a946e7c5")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 133.35 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006128426a")
		(property "Reference" "D5"
			(at 135.128 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 135.128 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 133.35 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 133.35 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 133.35 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "f6645359-43f8-42fc-b9db-c0f15a814652")
		)
		(pin "2"
			(uuid "2b74c80e-0561-43b1-a034-2457bc85b4c1")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 148.59 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612848b3")
		(property "Reference" "D6"
			(at 150.368 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 150.368 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 148.59 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 148.59 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 148.59 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "cfb23fb4-731f-4285-8ffd-3212902f9744")
		)
		(pin "2"
			(uuid "26241b3c-de1d-4d55-ac8f-6a428ccb7d05")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 163.83 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061284da8")
		(property "Reference" "D7"
			(at 165.608 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 165.608 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 163.83 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 163.83 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 163.83 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "2a019925-f46e-4465-9b91-fbf1c35fb828")
		)
		(pin "1"
			(uuid "7353d561-08ec-4348-ae6c-247147583e52")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 179.07 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612851cb")
		(property "Reference" "D8"
			(at 180.848 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 180.848 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 179.07 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 179.07 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 179.07 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "31a33ca5-4cfe-47db-b44d-c68f7048e559")
		)
		(pin "1"
			(uuid "09181097-ea7f-4d62-819f-21bd911ec925")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D8")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 194.31 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061285657")
		(property "Reference" "D9"
			(at 196.088 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 196.088 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 194.31 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 194.31 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 194.31 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "cbdd673a-1769-4ec4-85c0-00792c3a8306")
		)
		(pin "1"
			(uuid "28872267-9178-4be0-8a6c-30937adac632")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D9")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 209.55 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061285a16")
		(property "Reference" "D10"
			(at 211.328 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 211.328 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 209.55 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 209.55 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 209.55 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7a30df94-9a4f-4f8c-b11c-de7dd56d418e")
		)
		(pin "2"
			(uuid "f161f578-c260-4e67-9ec9-3abac32cfc94")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D10")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 224.79 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061286205")
		(property "Reference" "D11"
			(at 226.568 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 226.568 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 224.79 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 224.79 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 224.79 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "47a320cd-3ca0-4c7d-a22c-a13ebfefba55")
		)
		(pin "1"
			(uuid "5c465d28-ccbb-48b3-a79e-b55ae7429c57")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D11")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 238.76 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061286736")
		(property "Reference" "D12"
			(at 240.538 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 240.538 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 238.76 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 238.76 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 238.76 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "4badcf7e-7220-4b11-a305-d7d327ca987c")
		)
		(pin "1"
			(uuid "c21109ee-9ef8-43ba-a53a-cc7e5e4133cc")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D12")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 254 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061286c53")
		(property "Reference" "D13"
			(at 255.778 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 255.778 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 254 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 254 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 254 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "c1179d90-0638-4f24-befe-9c997bda3436")
		)
		(pin "1"
			(uuid "cd877ead-10ae-4e23-b248-dde46bc65a0b")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D13")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 269.24 39.37 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061286f59")
		(property "Reference" "D14"
			(at 271.018 38.2016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 271.018 40.513 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 269.24 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 269.24 39.37 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 269.24 39.37 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "b9bf0628-d7f4-4264-bc87-3dcc8a03875d")
		)
		(pin "2"
			(uuid "e4551619-d95f-4a49-9c33-d21cd6fff1be")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D14")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 72.39 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3204")
		(property "Reference" "D15"
			(at 74.168 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 74.168 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 72.39 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 72.39 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 72.39 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "cb2b8ae2-7c3c-429e-9be9-173559cc1825")
		)
		(pin "2"
			(uuid "8412914d-4650-4b14-b5c3-f62b26cf4a10")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D15")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 87.63 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e320a")
		(property "Reference" "D16"
			(at 89.408 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 89.408 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 87.63 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 87.63 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 87.63 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "b31d5ec5-f9f2-44b6-8748-2f432b02bd67")
		)
		(pin "1"
			(uuid "cc5ed8e1-3b94-41a9-9fb0-ac76589c9f10")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D16")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 102.87 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3210")
		(property "Reference" "D17"
			(at 104.648 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 104.648 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 102.87 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 102.87 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 102.87 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "0a4d97d8-949a-4e29-be0f-7aed71b813be")
		)
		(pin "2"
			(uuid "20bb08ec-63ba-49da-b588-e52522c1db7a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D17")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 118.11 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3216")
		(property "Reference" "D18"
			(at 119.888 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 119.888 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 118.11 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 118.11 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 118.11 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "133d78f7-7226-4ae6-a75b-1a0244b4f2e6")
		)
		(pin "1"
			(uuid "3d31752e-d1e3-481c-8cdf-dc34066feeab")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D18")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 133.35 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e321c")
		(property "Reference" "D19"
			(at 135.128 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 135.128 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 133.35 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 133.35 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 133.35 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "825056d3-b5e5-4f46-8871-5c146cd6773a")
		)
		(pin "1"
			(uuid "4074d08a-1edd-4f10-b13b-2c291a29e6b2")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D19")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 148.59 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3222")
		(property "Reference" "D20"
			(at 150.368 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 150.368 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 148.59 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 148.59 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 148.59 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "89074a95-0f1e-4611-9105-475584be3c14")
		)
		(pin "2"
			(uuid "976cc9c4-02c0-4a12-ad50-3f8381f409c6")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D20")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 163.83 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3228")
		(property "Reference" "D21"
			(at 165.608 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 165.608 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 163.83 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 163.83 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 163.83 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7cfc2f4d-18c3-4f23-a8e8-57c69a26a614")
		)
		(pin "2"
			(uuid "400fbe8f-afa1-4e31-85b8-8987a973334f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D21")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 179.07 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e322e")
		(property "Reference" "D22"
			(at 180.848 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 180.848 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 179.07 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 179.07 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 179.07 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "28978ce4-f4bf-47eb-9137-8749e58abbda")
		)
		(pin "1"
			(uuid "30632880-6df8-4773-bacc-85504f3c4094")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D22")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 194.31 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3234")
		(property "Reference" "D23"
			(at 196.088 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 196.088 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 194.31 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 194.31 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 194.31 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "10d5f53d-4cb8-40f1-abe0-6ace3f88fd6b")
		)
		(pin "2"
			(uuid "582e2093-77c2-4849-b3b4-d7894fe3b182")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D23")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 209.55 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e323a")
		(property "Reference" "D24"
			(at 211.328 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 211.328 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 209.55 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 209.55 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 209.55 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "b9fccd68-66fb-445f-bc69-2bf9bab97ded")
		)
		(pin "2"
			(uuid "0159a34c-20b1-4878-86dc-c44fc2b10257")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D24")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 224.79 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3240")
		(property "Reference" "D25"
			(at 226.568 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 226.568 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 224.79 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 224.79 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 224.79 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "b436c51f-8151-4426-8c28-a925cec64fae")
		)
		(pin "1"
			(uuid "80077f04-6459-4e64-a24c-2a81eac636cd")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D25")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 238.76 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3246")
		(property "Reference" "D26"
			(at 240.538 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 240.538 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 238.76 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 238.76 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 238.76 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "a8624e45-8d0b-4ff3-ab6f-7815bcbd1336")
		)
		(pin "1"
			(uuid "509dac4f-8247-4ba9-a813-ddca8ff79abb")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D26")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 254 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e324c")
		(property "Reference" "D27"
			(at 255.778 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 255.778 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 254 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 254 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 254 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "9e29f5aa-8bed-4ccc-adee-3ad4eee847fd")
		)
		(pin "2"
			(uuid "3607c5e2-63f0-49eb-bddc-c2b6fa0ca263")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D27")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 269.24 58.42 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612e3252")
		(property "Reference" "D28"
			(at 271.018 57.2516 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 271.018 59.563 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 269.24 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 269.24 58.42 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 269.24 58.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "8a42aa61-4596-4000-9b4e-53d0c7eda171")
		)
		(pin "1"
			(uuid "1802c243-8968-46a3-b34d-64be40a2406d")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D28")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 72.39 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff17e")
		(property "Reference" "D29"
			(at 74.168 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 74.168 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 72.39 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 72.39 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 72.39 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "f9a65586-29bf-4887-9d34-1fa2887f04a9")
		)
		(pin "1"
			(uuid "2656f3ab-fafc-43a8-93bc-0cc57a1b7a27")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D29")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 87.63 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff184")
		(property "Reference" "D30"
			(at 89.408 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 89.408 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 87.63 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 87.63 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 87.63 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "ee998f1a-6a2d-4d31-9539-a629a2c3e2e0")
		)
		(pin "2"
			(uuid "e046bf7a-1df2-4545-a54c-3919bd18453a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D30")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 102.87 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff18a")
		(property "Reference" "D31"
			(at 104.648 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 104.648 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 102.87 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 102.87 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 102.87 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "56a73aa1-17f3-49fc-a935-9d9ae2b6f20a")
		)
		(pin "2"
			(uuid "9b8b8d4b-d775-4720-bf60-5733664f3161")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D31")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 118.11 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff190")
		(property "Reference" "D32"
			(at 119.888 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 119.888 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 118.11 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 118.11 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 118.11 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "1e499f96-cbf3-4944-be78-48ad7af8a41e")
		)
		(pin "2"
			(uuid "48ed8245-e275-48dd-81e9-e0718cf1f1aa")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D32")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 133.35 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff196")
		(property "Reference" "D33"
			(at 135.128 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 135.128 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 133.35 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 133.35 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 133.35 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "72b8b7f5-c5c9-4f04-b340-44e37a194d2a")
		)
		(pin "1"
			(uuid "aa7e24cb-5612-464a-92e8-b9db18997347")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D33")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 148.59 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff19c")
		(property "Reference" "D34"
			(at 150.368 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 150.368 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 148.59 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 148.59 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 148.59 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "b305f957-749a-4770-ae96-266f3b19fe6f")
		)
		(pin "2"
			(uuid "60846a80-7ef8-4ab8-b650-95850d7f5111")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D34")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 163.83 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1a2")
		(property "Reference" "D35"
			(at 165.608 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 165.608 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 163.83 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 163.83 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 163.83 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "797f6731-261e-4107-ab00-bd3038e7a6af")
		)
		(pin "1"
			(uuid "300f1924-f53a-4bcb-aff6-c65ba189a059")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D35")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 179.07 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1a8")
		(property "Reference" "D36"
			(at 180.848 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 180.848 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 179.07 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 179.07 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 179.07 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "38cf45a5-eb8b-41c8-a5a1-721bb1e29d65")
		)
		(pin "2"
			(uuid "d63b6e66-23c5-46a2-90bd-cf5922f7ac2b")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D36")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 194.31 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1ae")
		(property "Reference" "D37"
			(at 196.088 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 196.088 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 194.31 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 194.31 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 194.31 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "9bed7a39-7de2-4e76-9159-058ce816f12d")
		)
		(pin "1"
			(uuid "5ab52e3f-f3dd-40bf-8257-f3365729142f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D37")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 209.55 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1b4")
		(property "Reference" "D38"
			(at 211.328 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 211.328 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 209.55 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 209.55 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 209.55 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "ca6cb3d6-f655-4c2c-b6c3-79c7a9a6ec5f")
		)
		(pin "2"
			(uuid "33904392-b8bd-4db3-9773-f85e9509e4f8")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D38")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 224.79 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1ba")
		(property "Reference" "D39"
			(at 226.568 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 226.568 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 224.79 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 224.79 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 224.79 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "6bd09d09-c5d4-4c53-8568-7384c5ab5769")
		)
		(pin "2"
			(uuid "85c8790a-9115-4aea-b264-a314b6ecc3b9")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D39")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 238.76 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1c0")
		(property "Reference" "D40"
			(at 240.538 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 240.538 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 238.76 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 238.76 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 238.76 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "6db7aceb-47fa-4eba-9f07-194dcd46d248")
		)
		(pin "1"
			(uuid "62e09fbd-2f96-43e8-becb-61e15a3bdb62")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D40")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 254 77.47 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000612ff1c6")
		(property "Reference" "D41"
			(at 255.778 76.3016 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 255.778 78.613 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 254 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 254 77.47 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 254 77.47 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "f3c396cf-db8f-4457-a6a0-74d247613300")
		)
		(pin "2"
			(uuid "30cf5514-98c7-4835-9a41-e4c9f88b6929")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D41")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 72.39 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a794")
		(property "Reference" "D42"
			(at 74.168 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 74.168 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 72.39 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 72.39 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 72.39 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "4b53e510-fb23-45a1-960f-e21746db8949")
		)
		(pin "1"
			(uuid "76b2301f-51cd-4af7-be10-d5ad9d45112d")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D42")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 87.63 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a79a")
		(property "Reference" "D43"
			(at 89.408 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 89.408 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 87.63 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 87.63 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 87.63 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "7b0236b6-6c02-4979-9c21-78b79aea2816")
		)
		(pin "1"
			(uuid "2933dda8-ae7d-4ae5-b674-b523e53b98ab")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D43")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 102.87 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7a0")
		(property "Reference" "D44"
			(at 104.648 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 104.648 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 102.87 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 102.87 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 102.87 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "fc1a8976-a1e9-4e01-9729-9817688f45ed")
		)
		(pin "1"
			(uuid "07a9a9d9-69db-433b-bc81-f0e6953025a9")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D44")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 118.11 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7a6")
		(property "Reference" "D45"
			(at 119.888 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 119.888 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 118.11 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 118.11 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 118.11 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "df15523b-2fc2-47e8-8dcd-d0e202ead1d1")
		)
		(pin "2"
			(uuid "abeb118c-c745-43d5-8045-6a743cd3d147")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D45")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 133.35 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7ac")
		(property "Reference" "D46"
			(at 135.128 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 135.128 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 133.35 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 133.35 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 133.35 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "130f0bad-2a61-4574-b960-922d9b9c8896")
		)
		(pin "2"
			(uuid "4bba5af8-9254-45f3-a743-fae933602bdb")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D46")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 148.59 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7b2")
		(property "Reference" "D47"
			(at 150.368 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 150.368 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 148.59 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 148.59 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 148.59 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "02cd716b-9440-4c7a-9384-f2346dd33ce4")
		)
		(pin "2"
			(uuid "bc2d8ab4-1f3e-4f0f-bbee-08ac580351f7")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D47")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 163.83 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7b8")
		(property "Reference" "D48"
			(at 165.608 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 165.608 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 163.83 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 163.83 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 163.83 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "9006470b-7db5-49f6-912c-0ee5ae414a52")
		)
		(pin "2"
			(uuid "6d4b37dd-fd19-4a63-8b45-b8044b11305c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D48")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 179.07 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7be")
		(property "Reference" "D49"
			(at 180.848 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 180.848 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 179.07 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 179.07 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 179.07 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "ba6ed906-d385-420b-8457-28e43c7d931a")
		)
		(pin "2"
			(uuid "ce9cf6a1-521e-4daf-aa59-9bb4fe6fd2cb")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D49")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 194.31 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7c4")
		(property "Reference" "D50"
			(at 196.088 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 196.088 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 194.31 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 194.31 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 194.31 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "cef14999-a888-4f4b-bfcb-36af72574e05")
		)
		(pin "2"
			(uuid "7563c838-217b-47af-9116-26cf31672577")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D50")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 209.55 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7ca")
		(property "Reference" "D51"
			(at 211.328 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 211.328 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 209.55 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 209.55 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 209.55 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "07b3a313-5f21-45c8-ad23-4bd7f2c60075")
		)
		(pin "2"
			(uuid "01bf4427-9dac-4932-b1fe-ff33d58a8efc")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D51")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 224.79 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7d0")
		(property "Reference" "D52"
			(at 226.568 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 226.568 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 224.79 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 224.79 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 224.79 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "2dad181c-afdc-4145-bcd0-a8fafe9dc42d")
		)
		(pin "1"
			(uuid "754c607f-485a-44bc-8873-57a0c239f958")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D52")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 238.76 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7d6")
		(property "Reference" "D53"
			(at 240.538 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 240.538 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 238.76 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 238.76 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 238.76 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "81f74a4f-c675-485d-83eb-39927f6a4d24")
		)
		(pin "2"
			(uuid "3c64e28a-d114-484d-a866-e856e7b63ea1")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D53")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 254 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7dc")
		(property "Reference" "D54"
			(at 255.778 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 255.778 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 254 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 254 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 254 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "71929123-f8f2-4020-b3d3-ea08b0eaf3e1")
		)
		(pin "2"
			(uuid "4f3c16c0-4798-4c0c-b1ff-801b73f55dca")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D54")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 269.24 97.79 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006131a7e2")
		(property "Reference" "D55"
			(at 271.018 96.6216 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 271.018 98.933 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 269.24 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 269.24 97.79 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 269.24 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "93db0310-13d6-44d4-9022-c3920a9f7812")
		)
		(pin "2"
			(uuid "c01d1247-9c09-41e2-beba-332b0b0c8bf2")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D55")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 72.39 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000613384f2")
		(property "Reference" "D56"
			(at 74.168 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 74.168 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 72.39 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 72.39 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 72.39 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7e3b012e-cad5-4489-8004-963ff5706bf7")
		)
		(pin "2"
			(uuid "17bd8e26-8552-41b0-ba11-f81e276984ba")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D56")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 87.63 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000613384f8")
		(property "Reference" "D57"
			(at 89.408 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 89.408 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 87.63 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 87.63 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 87.63 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "d6bf1cfc-a234-4bb3-ae4a-1b3a3a44f5fa")
		)
		(pin "1"
			(uuid "66f7c04e-46bf-496d-a4c8-a21b1fd20f7a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D57")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 102.87 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000613384fe")
		(property "Reference" "D58"
			(at 104.648 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 104.648 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 102.87 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 102.87 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 102.87 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "b2eaa3a2-98d3-40be-af07-c944a1854660")
		)
		(pin "1"
			(uuid "83179c53-1994-42b6-8f0e-91d758cda369")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D58")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 163.83 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061338516")
		(property "Reference" "D59"
			(at 165.608 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 165.608 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 163.83 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 163.83 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 163.83 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "b40cd0c9-0fca-486e-b680-58577ba34b5d")
		)
		(pin "1"
			(uuid "ecad1891-0809-4c7f-a0f0-b56f42136d7c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D59")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 209.55 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061338528")
		(property "Reference" "D60"
			(at 211.328 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 211.328 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 209.55 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 209.55 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 209.55 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "618501d9-df47-4211-bec0-5bf07e5d513d")
		)
		(pin "2"
			(uuid "f78443bd-5eff-4538-be7f-74f2a3f866c5")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D60")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 224.79 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006133852e")
		(property "Reference" "D61"
			(at 226.568 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 226.568 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 224.79 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 224.79 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 224.79 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "2367c794-3e26-4c18-8e86-f6c27279ac5e")
		)
		(pin "2"
			(uuid "b04798df-bbae-4e2c-8d87-5c51367f1157")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D61")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 238.76 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061338534")
		(property "Reference" "D62"
			(at 240.538 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 240.538 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 238.76 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 238.76 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 238.76 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "8e02648d-8d69-4926-a08f-4e093d10258b")
		)
		(pin "2"
			(uuid "3eacaf08-980a-4bce-966f-cf347376556e")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D62")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 254 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006133853a")
		(property "Reference" "D63"
			(at 255.778 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 255.778 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 254 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 254 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 254 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "63914180-464f-41e6-9ed3-db6cc92a96c7")
		)
		(pin "2"
			(uuid "47de4a91-9591-4562-bb2e-014142c2333e")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D63")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Small")
		(at 269.24 118.11 90)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061338540")
		(property "Reference" "D64"
			(at 271.018 116.9416 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "1N4148"
			(at 271.018 119.253 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal"
			(at 269.24 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 269.24 118.11 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 269.24 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "3d075bcd-8f83-482b-a97a-cc2e02ca5914")
		)
		(pin "1"
			(uuid "de5fd638-41fa-4453-a4be-4103afb4f25a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "D64")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 40.64 49.53 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006144386c")
		(property "Reference" "R1"
			(at 42.418 48.3616 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 42.418 50.673 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 38.862 49.53 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 40.64 49.53 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 49.53 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "be0423fe-1f92-4115-bbb4-fed64aa5d60f")
		)
		(pin "2"
			(uuid "f33b968b-622f-4415-b4fb-067fe82bdc51")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 40.64 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061443e41")
		(property "Reference" "#PWR09"
			(at 40.64 59.69 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 40.767 57.7342 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 40.64 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 40.64 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "e4b7bcc0-4c9d-42b6-80f9-6eeebd570b1c")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR09")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "6502-computer:RETRO65-LOGO")
		(at 195.58 172.72 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061443e91")
		(property "Reference" "LOGO1"
			(at 195.58 185.4962 0)
			(effects
				(font
					(size 1.524 1.524)
				)
				(hide yes)
			)
		)
		(property "Value" "RETRO65-LOGO"
			(at 195.58 159.9438 0)
			(effects
				(font
					(size 1.524 1.524)
				)
				(hide yes)
			)
		)
		(property "Footprint" "6502-computer:RETRO65-LOGO"
			(at 195.58 172.72 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 195.58 172.72 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 195.58 172.72 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "LOGO1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 48.26 49.53 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006148e6ba")
		(property "Reference" "R2"
			(at 50.038 48.3616 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 50.038 50.673 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 46.482 49.53 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 48.26 49.53 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 49.53 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "fa7c39a7-cfd6-423d-be12-9673023071ad")
		)
		(pin "2"
			(uuid "c1d379f9-c4a7-4812-adb2-780444cecd3a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 48.26 53.34 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006148e6c0")
		(property "Reference" "#PWR010"
			(at 48.26 59.69 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 48.387 57.7342 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 48.26 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 48.26 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 53.34 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "61bba1cd-d5c7-4993-a41a-e2aa67fdcf8f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR010")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 40.64 68.58 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614a68c6")
		(property "Reference" "R3"
			(at 42.418 67.4116 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 42.418 69.723 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 38.862 68.58 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 40.64 68.58 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 68.58 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "00507ba8-bd41-4551-b2c3-cb0e4ebd1f45")
		)
		(pin "2"
			(uuid "e013eda9-6932-43d0-a1fe-2d2caed3919a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 40.64 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614a68cc")
		(property "Reference" "#PWR011"
			(at 40.64 78.74 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 40.767 76.7842 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 40.64 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 40.64 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7ffcbadb-1968-402c-bf01-b4545f1a7e2b")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR011")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 48.26 68.58 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614becdc")
		(property "Reference" "R4"
			(at 50.038 67.4116 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 50.038 69.723 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 46.482 68.58 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 48.26 68.58 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 68.58 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "57d064b4-d662-4e5d-b7ee-b1b48eb077dd")
		)
		(pin "1"
			(uuid "5d7b4695-ddac-41c2-adc7-1e7736d78d7a")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 48.26 72.39 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614bece2")
		(property "Reference" "#PWR012"
			(at 48.26 78.74 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 48.387 76.7842 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 48.26 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 48.26 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 72.39 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "e6ca0bd6-418a-43aa-89ef-a7ccafaa7f27")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR012")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 40.64 87.63 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bb8")
		(property "Reference" "R5"
			(at 42.418 86.4616 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 42.418 88.773 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 38.862 87.63 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 40.64 87.63 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 87.63 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "8547725a-6c22-4c0b-b8de-760f5ac40fd8")
		)
		(pin "2"
			(uuid "b20bfc79-8828-4ebe-868b-fc67123c237f")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 40.64 91.44 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bbe")
		(property "Reference" "#PWR013"
			(at 40.64 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 40.767 95.8342 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 40.64 91.44 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 40.64 91.44 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 91.44 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "3a8bf934-5717-448e-b051-c9bd2dc3d37e")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR013")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 48.26 87.63 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bc4")
		(property "Reference" "R6"
			(at 50.038 86.4616 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 50.038 88.773 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 46.482 87.63 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 48.26 87.63 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 87.63 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "2"
			(uuid "55ccd141-6719-4e22-8e18-f98943a40d65")
		)
		(pin "1"
			(uuid "44da4131-069b-45ac-810b-7b1b38c2acc8")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 48.26 91.44 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bca")
		(property "Reference" "#PWR014"
			(at 48.26 97.79 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 48.387 95.8342 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 48.26 91.44 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 48.26 91.44 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 91.44 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "5a03f67f-4956-450e-b27f-11922b70a3b1")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR014")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 40.64 107.95 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bd0")
		(property "Reference" "R7"
			(at 42.418 106.7816 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 42.418 109.093 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 38.862 107.95 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 40.64 107.95 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 107.95 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "5ce09d12-2164-48f3-83d1-4b9184693790")
		)
		(pin "2"
			(uuid "c02fcefc-0451-4d09-856e-3a938f823f18")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 40.64 111.76 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bd6")
		(property "Reference" "#PWR015"
			(at 40.64 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 40.767 116.1542 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 40.64 111.76 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 40.64 111.76 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 40.64 111.76 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "9bc88690-dfe0-42b8-b9c8-7ee457359993")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR015")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 48.26 107.95 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8bdc")
		(property "Reference" "R8"
			(at 50.038 106.7816 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 50.038 109.093 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"
			(at 46.482 107.95 90)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 48.26 107.95 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 107.95 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "27d9a43a-e1bb-4f6d-84bc-128c3c50bcbf")
		)
		(pin "2"
			(uuid "286c8b49-7a56-469a-a337-942dac5791b7")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "R8")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 48.26 111.76 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000614d8be2")
		(property "Reference" "#PWR016"
			(at 48.26 118.11 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 48.387 116.1542 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 48.26 111.76 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 48.26 111.76 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 48.26 111.76 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "7887d14e-805d-42c2-8800-bce08ff1dcc4")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR016")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 83.82 189.23 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000618e1e83")
		(property "Reference" "#PWR08"
			(at 83.82 195.58 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 83.947 193.6242 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 83.82 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 83.82 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 83.82 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "757d2627-3206-4464-9e5f-99ce046a12af")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR08")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "74xx:74LS165")
		(at 83.82 163.83 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006191a6d4")
		(property "Reference" "U2"
			(at 85.09 142.24 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "74HCT165"
			(at 85.09 144.78 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Package_DIP:DIP-16_W7.62mm"
			(at 83.82 163.83 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 83.82 163.83 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 83.82 163.83 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "9"
			(uuid "fedec0cb-f086-42ef-84ce-99ea1738c714")
		)
		(pin "15"
			(uuid "e0934747-b287-4643-85c1-f7fdf12e4a52")
		)
		(pin "10"
			(uuid "f5abdcb6-4048-485b-8ce3-81197a29e037")
		)
		(pin "12"
			(uuid "7d417f15-c764-4b71-852e-bbc6b63dde33")
		)
		(pin "14"
			(uuid "a84421e6-43a8-4757-ab73-990c8936eba4")
		)
		(pin "16"
			(uuid "e034c545-9015-4a90-aceb-4be30b522d2b")
		)
		(pin "2"
			(uuid "6dcc3442-b752-411d-aaa0-a45d1849a0f5")
		)
		(pin "7"
			(uuid "eabd895b-cef5-4476-883f-bf9998cebe75")
		)
		(pin "8"
			(uuid "ca189a4d-ea9d-4ac7-8958-9393c270614e")
		)
		(pin "3"
			(uuid "22bd7214-234f-4c0d-83fa-acb573bb4bcb")
		)
		(pin "4"
			(uuid "2bf5acd7-9d3d-463e-9422-889b05387fb1")
		)
		(pin "5"
			(uuid "f2a48904-47b2-49a5-b274-ef47f59cf95b")
		)
		(pin "6"
			(uuid "41c51593-d17e-43c1-8dc4-4d77d5326281")
		)
		(pin "11"
			(uuid "04342ef6-9ac6-4b65-b7ae-23ebcb8b9a99")
		)
		(pin "13"
			(uuid "3a2e0216-3236-4a48-8583-ca4e34f5bd65")
		)
		(pin "1"
			(uuid "12f60310-b3ca-456c-bf73-c986182d0e37")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "U2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 83.82 140.97 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000619b136b")
		(property "Reference" "#PWR02"
			(at 83.82 144.78 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "+5V"
			(at 84.201 136.5758 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 83.82 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 83.82 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 83.82 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "9d3e2488-9601-487e-aa64-722b2723f901")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR02")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "4xxx:4017")
		(at 39.37 161.29 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000619d121e")
		(property "Reference" "U1"
			(at 43.18 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "4017"
			(at 44.45 143.51 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Package_DIP:DIP-16_W7.62mm"
			(at 39.37 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 39.37 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 39.37 161.29 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "5"
			(uuid "3ff80f6e-d56a-4347-9a84-2a7b38e55ffd")
		)
		(pin "1"
			(uuid "4d667204-00d1-4df9-a302-36583a1e25b6")
		)
		(pin "11"
			(uuid "e2f90d66-62a5-47ae-84bd-1a00efd2ce04")
		)
		(pin "12"
			(uuid "90e78831-6707-417d-87b2-3447d585ea62")
		)
		(pin "13"
			(uuid "2a1a47d9-b187-4f05-837a-96773014c972")
		)
		(pin "14"
			(uuid "729df277-1fc5-4490-bf91-320b1d30c324")
		)
		(pin "16"
			(uuid "85373c5e-97c5-4a01-8c27-6af8ce451ff8")
		)
		(pin "2"
			(uuid "3af9aa39-e148-4656-8980-e889ecc6b836")
		)
		(pin "3"
			(uuid "510df14c-5fb3-49e7-aa2e-438718346bce")
		)
		(pin "15"
			(uuid "cca2e049-b459-4b03-bce0-46258c3f2c15")
		)
		(pin "10"
			(uuid "b08b7705-c0d5-41bb-a0b4-26fed772abe1")
		)
		(pin "4"
			(uuid "310b6b11-f6a5-4fbf-9542-11609cbb5ac6")
		)
		(pin "8"
			(uuid "c7215729-21eb-4a22-b1e4-240ad95e5aa1")
		)
		(pin "7"
			(uuid "aec60fec-2a74-4514-b755-32656b586ec2")
		)
		(pin "6"
			(uuid "9ea0f2ac-c077-4674-8ba5-9cc0cae6695f")
		)
		(pin "9"
			(uuid "71aaa0c2-d9fa-466d-adb0-ebcc91bc79bf")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "U1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 39.37 184.15 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000619d2ffc")
		(property "Reference" "#PWR07"
			(at 39.37 190.5 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 39.497 188.5442 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 39.37 184.15 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 39.37 184.15 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 39.37 184.15 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "53f25696-9f7c-4864-9360-ec3b2bab7289")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR07")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 39.37 140.97 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-0000619d3785")
		(property "Reference" "#PWR01"
			(at 39.37 144.78 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "+5V"
			(at 39.751 136.5758 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 39.37 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 39.37 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 39.37 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "5e02b346-4eff-44f1-9696-2b583d7a0205")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR01")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 20.32 179.07 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a38930")
		(property "Reference" "C1"
			(at 23.241 177.9016 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "100nF"
			(at 23.241 180.213 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm"
			(at 21.2852 182.88 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 20.32 179.07 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 20.32 179.07 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "f37a3160-0d53-4a72-b673-e876cd8683ca")
		)
		(pin "2"
			(uuid "e8ed9f2a-5511-429d-ba9d-f6cb5d678edc")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "C1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 99.06 185.42 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a390ef")
		(property "Reference" "C2"
			(at 101.981 184.2516 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "100nF"
			(at 101.981 186.563 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm"
			(at 100.0252 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 99.06 185.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 99.06 185.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "f895e569-1410-494a-9818-ab8e8f20bdf4")
		)
		(pin "2"
			(uuid "63b00dbc-ead0-412d-83ab-5b24bf0df524")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "C2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 20.32 182.88 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a40549")
		(property "Reference" "#PWR05"
			(at 20.32 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 20.447 187.2742 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 20.32 182.88 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 20.32 182.88 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 20.32 182.88 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "2da3123f-7f07-43fb-a30c-6d693a44aec8")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR05")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 99.06 189.23 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a40827")
		(property "Reference" "#PWR06"
			(at 99.06 195.58 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "GND"
			(at 99.187 193.6242 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 99.06 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 99.06 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 99.06 189.23 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "e203f770-ed9a-49d9-a9e5-cb170ca8d356")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR06")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_02x04_Odd_Even")
		(at 177.8 154.94 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a46343")
		(property "Reference" "J1"
			(at 179.07 146.8882 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "Conn_02x04_Odd_Even"
			(at 179.07 149.1996 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical"
			(at 177.8 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 177.8 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 177.8 154.94 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "813cf6c0-c1f3-40c8-b730-11e178045dd2")
		)
		(pin "7"
			(uuid "d641c9b9-2bec-48aa-ba7e-3e00a7a08059")
		)
		(pin "8"
			(uuid "e3209aee-3981-4973-922d-09d761784b7f")
		)
		(pin "3"
			(uuid "0fdf8509-3645-45bd-82bd-fcda1c39551c")
		)
		(pin "6"
			(uuid "ea9bc160-4797-4638-98da-59821545b007")
		)
		(pin "4"
			(uuid "00c81e6d-7e94-4b55-a1e2-90a14de8fa72")
		)
		(pin "5"
			(uuid "1ee28542-ed94-47b4-af5b-c5386397bc99")
		)
		(pin "2"
			(uuid "ccf97f91-3be1-44e1-ae91-d6619167df93")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "J1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 20.32 175.26 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a484c8")
		(property "Reference" "#PWR03"
			(at 20.32 179.07 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "+5V"
			(at 20.701 170.8658 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 20.32 175.26 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 20.32 175.26 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 20.32 175.26 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "dd6f0fb0-4d40-447d-a8f2-9be36088831e")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR03")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 99.06 181.61 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061a48ab3")
		(property "Reference" "#PWR04"
			(at 99.06 185.42 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Value" "+5V"
			(at 99.441 177.2158 0)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 99.06 181.61 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" ""
			(at 99.06 181.61 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 99.06 181.61 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(pin "1"
			(uuid "bcb2e521-82e0-45f6-b6f8-a4f84314aea3")
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "#PWR04")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 266.7 134.62 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061b769ba")
		(property "Reference" "H13"
			(at 269.24 133.4516 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 269.24 135.763 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "6502-computer:MountingHole_3.2mm_M3_minimal"
			(at 266.7 134.62 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 134.62 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 134.62 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H13")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Mechanical:MountingHole")
		(at 266.7 140.97 0)
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000061b76e9e")
		(property "Reference" "H14"
			(at 269.24 139.8016 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "MountingHole"
			(at 269.24 142.113 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "6502-computer:MountingHole_3.2mm_M3_minimal"
			(at 266.7 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Datasheet" "~"
			(at 266.7 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(property "Description" ""
			(at 266.7 140.97 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
		(instances
			(project ""
				(path "/fcc9ff58-5316-43d1-9ceb-32a90e974622"
					(reference "H14")
					(unit 1)
				)
			)
		)
	)
	(sheet_instances
		(path "/"
			(page "1")
		)
	)
)
