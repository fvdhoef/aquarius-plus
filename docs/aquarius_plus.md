# Aquarius<sup>+</sup>

# Memory map

| Memory address | Description            |
|----------------|------------------------|
| $0000 - $1FFF  | System ROM             |
| $2000 - $2FFF  | Extended system ROM    |
| $3000 - $03FF  | Screen RAM             |
| $3400 - $07FF  | Color RAM              |
| $3800 - $3FFF  | System RAM             |
| $4000 - $7FFF  | Bank 1                 |
| $8000 - $BFFF  | Bank 2                 |
| $C000 - $FFFF  | Cartridge ROM / Bank 3 |

# IO map

| IO address | Description                                                      |
|------------|------------------------------------------------------------------|
| $00 - $7D  | Unallocated                                                      |
| $7E        | Modem (?)                                                        |
| $7F        | Modem (?)                                                        |
| $80 - $E7  | Unallocated                                                      |
| $E8 - $EA  | Floppy disk interface (?)                                        |
| $EB - $EF  | Unallocated                                                      |
| $F0        | Video: VCTRL                                                     |
| $F1        | Video: SCRX_L                                                    |
| $F2        | Video: SCRX_H                                                    |
| $F3        | Video: SCRY                                                      |
| $F4        | Video:                                                           |
| $F5        | Video:                                                           |
| $F6        | AY-3-8910 R: read from PSG, W: write to PSG                      |
| $F7        | AY-3-8910 R: read from PSG, W: write to latch address            |
| $F8        | Bank 1 ($0000-$3FFF) page                                        |
| $F9        | Bank 2 ($4000-$7FFF) page                                        |
| $FA        | Bank 3 ($8000-$BFFF) page                                        |
| $FB        | Bank 4 ($C000-$FFFF) page                                        |
| $FC        | W<0>: Cassette/Sound output<br/>R<0>: Cassette input             |
| $FD        | W<0>: CP/M mode remapping<br/>R<0>: V-sync signal (0: in v-sync) |
| $FE        | W<0>: Serial printer (1200bps)<br/>R<0>: Clear to send status    |
| $FF        | W<7:0>: External bus scramble value (XOR)<br/>R<5:0>: Keyboard   |

# Video registers (IO space)

<table>
	<tr>
		<th>Addr</th>
		<th>Name</th>
		<th>Bit&nbsp;7</th>
		<th>Bit&nbsp;6</th>
		<th>Bit&nbsp;5 </th>
		<th>Bit&nbsp;4</th>
		<th>Bit&nbsp;3 </th>
		<th>Bit&nbsp;2</th>
		<th>Bit&nbsp;1 </th>
		<th>Bit&nbsp;0</th>
	</tr>
    <tr>
        <td>$F0</td>
        <td>VCTRL</td>
        <td colspan="5" align="center">-</td>
        <td colspan="1" align="center">Sprites enable</td>
        <td colspan="1" align="center">Tile map enable</td>
        <td colspan="1" align="center">Text enable</td>
    </tr>
    <tr>
        <td>$F1</td>
        <td>SCRX_L</td>
        <td colspan="8" align="center">Tile map X-scroll (7:0)</td>
    </tr>
    <tr>
        <td>$F2</td>
        <td>SCRX_H</td>
        <td colspan="7" align="center">-</td>
        <td colspan="1" align="center">Tile map X-scroll (8)</td>
    </tr>
    <tr>
        <td>$F3</td>
        <td>SCRY</td>
        <td colspan="8" align="center">Tile map Y-scroll</td>
    </tr>
</table>

# Video memory

| Address       | Description              |
|---------------|--------------------------|
| $0000 - $07FF | Character RAM            |
| $0800 - $083F | Sprite Y-position        |
| $0840 - $087F | Sprite X-position (7:0)  |
| $0880 - $08BF | Sprite X-position (8)    |
| $08C0 - $08FF | Sprite tile index (7:0)  |
| $0900 - $093F | Sprite attributes        |
| $0940 - $094F | Character palette        |
| $0950 - $095F | Tile palette             |
| $0960 - $096F | Sprite palette           |
| $0A40 - $0FFF | -                        |
| $1000 - $1FFF | Tile map 64x32           |
| $2000 - $3FFF | -                        |
| $4000 - $7FFF | Tile data (512 patterns) |

## Sprite attributes
64 entries, each with the following format:

<table>
	<tr>
		<th>Bit&nbsp;7</th>
		<th>Bit&nbsp;6</th>
		<th>Bit&nbsp;5</th>
		<th>Bit&nbsp;4</th>
		<th>Bit&nbsp;3</th>
		<th>Bit&nbsp;2</th>
		<th>Bit&nbsp;1</th>
		<th>Bit&nbsp;0</th>
	</tr>
	<tr>
		<td align="center" colspan="3">-</td>
		<td align="center" colspan="1">Priority</td>
		<td align="center" colspan="1">Palette</td>
		<td align="center" colspan="1">V-flip</td>
		<td align="center" colspan="1">H-flip</td>
		<td align="center" colspan="1">Tile index (8)</td>
	</tr>
</table>

## Palettes
Each palette has 16 entries, each with the following format:

<table>
	<tr>
		<th>Bit&nbsp;7</th>
		<th>Bit&nbsp;6</th>
		<th>Bit&nbsp;5</th>
		<th>Bit&nbsp;4</th>
		<th>Bit&nbsp;3</th>
		<th>Bit&nbsp;2</th>
		<th>Bit&nbsp;1</th>
		<th>Bit&nbsp;0</th>
	</tr>
	<tr>
		<td align="center" colspan="2">-</td>
		<td align="center" colspan="2">B</td>
		<td align="center" colspan="2">G</td>
		<td align="center" colspan="2">R</td>
	</tr>
</table>


## Tile map
64x32 entries, each with the following format:

<table>
	<tr>
		<th>Bit&nbsp;7</th>
		<th>Bit&nbsp;6</th>
		<th>Bit&nbsp;5</th>
		<th>Bit&nbsp;4</th>
		<th>Bit&nbsp;3</th>
		<th>Bit&nbsp;2</th>
		<th>Bit&nbsp;1</th>
		<th>Bit&nbsp;0</th>
	</tr>
	<tr>
		<td align="center" colspan="8">Tile index (7:0)</td>
	</tr>
	<tr>
		<td align="center" colspan="3">-</td>
		<td align="center" colspan="1">Priority</td>
		<td align="center" colspan="1">Palette</td>
		<td align="center" colspan="1">V-flip</td>
		<td align="center" colspan="1">H-flip</td>
		<td align="center" colspan="1">Tile index (8)</td>
	</tr>
</table>

If priority bit is set, the tile is displayed in front of sprites (if sprite priority bit is not set).

## Tile data

512 tile pattern entries. Each pattern is 32 bytes in size. Each pattern is 8x8 pixels of 4 bits per pixel. Each byte contains 2 pixels, the upper 4 bits contains the palette index of the left pixel, the lower 4 bits the palette index of the right pixel.
