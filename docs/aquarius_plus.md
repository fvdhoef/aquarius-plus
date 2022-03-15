# Aquarius<sup>+</sup>

# Memory map

| Memory address | Description |
| -------------- | ----------- |
| $0000 - $3FFF  | Bank 0      |
| $4000 - $7FFF  | Bank 1      |
| $8000 - $BFFF  | Bank 2      |
| $C000 - $FFFF  | Bank 3      |

# IO map

| IO address | Name     | Description                                                      |
| ---------- | -------- | ---------------------------------------------------------------- |
| $00 - $DF  |          | -                                                                |
| $E0        | VCTRL    | Video: Control register                                          |
| $E1        | VSCRX_L  | Video: Tilemap horizontal scroll (7:0)                           |
| $E2        | VSCRX_H  | Video: Tilemap horizontal scroll (8)                             |
| $E3        | VSCRY    | Video: Tilemap vertical scroll                                   |
| $E4        | VSPRX_L  | Video: Sprite X-position (7:0)                                   |
| $E5        | VSPRX_H  | Video: Sprite X-position (8)                                     |
| $E6        | VSPRY    | Video: Sprite Y-position                                         |
| $E7        | VSPRIDX  | Video: Sprite tile index (7:0)                                   |
| $E8        | VSPRATTR | Video: Sprite attributes                                         |
| $E9        | VPALTXT  | Video: Text palette                                              |
| $EA        | VPALTILE | Video: Tile/bitmap palette                                       |
| $EB        | VPALSPR  | Video: Sprite palette                                            |
| $EC        | VLINE    | Video: Current line                                              |
| $ED        | VIRQLINE | Video: Line at which to generate interrupt                       |
| $EE        | IRQMASK  | Interrupt masking register                                       |
| $EF        | IRQSTAT  | Interrupt status register                                        |
| $F0        | BANK0    | Bank 0 ($0000-$3FFF) page                                        |
| $F1        | BANK1    | Bank 1 ($4000-$7FFF) page                                        |
| $F2        | BANK2    | Bank 2 ($8000-$BFFF) page                                        |
| $F3        | BANK3    | Bank 3 ($C000-$FFFF) page                                        |
| $F4        | ESPCTRL  | ESP32 control/status                                             |
| $F5        | ESPDATA  | ESP32 data                                                       |
| $F6        | PSG1DATA | PSG1: AY-3-8910 R: read from PSG, W: write to PSG                |
| $F7        | PSG1ADDR | PSG1: AY-3-8910 R: read from PSG, W: write to latch address      |
| $F8        | PSG2DATA | PSG2: AY-3-8910 R: read from PSG, W: write to PSG                |
| $F9        | PSG2ADDR | PSG2: AY-3-8910 R: read from PSG, W: write to latch address      |
| $FA        |          | -                                                                |
| $FB        | SYSCTRL  | RW<0>: Disable access to extended registers                      |
| $FB        | SYSCTRL  | RW<1>: Disable PSGs (as if mini-expander isn't available)        |
| $FC        |          | W<0>: Cassette/Sound output<br/>R<0>: Cassette input             |
| $FD        |          | W<0>: CP/M mode remapping<br/>R<0>: V-sync signal (0: in v-sync) |
| $FE        |          | W<0>: Serial printer (1200bps)<br/>R<0>: Clear to send status    |
| $FF        |          | W<7:0>: External bus scramble value (XOR)<br/>R<5:0>: Keyboard   |

## Banking

### Banking registers

<table>
    <tr>
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
        <td colspan="1" align="center">Read only</td>
        <td colspan="1" align="center">Overlay RAM</td>
        <td colspan="6" align="center">Page</td>
    </tr>
</table>

When setting the **_Overlay RAM_** bit, $3000-$3FFF is replaced with:

| Address       | Description      |
| ------------- | ---------------- |
| $3000 - $33FF | Screen RAM (1KB) |
| $3400 - $37FF | Color RAM (1KB)  |
| $3800 - $3FFF | Basic RAM (2KB)  |

### Banked memory map

|  Page | Description                                |
| ----: | ------------------------------------------ |
|  0-15 | Flash memory (256KB)                       |
|    16 | External bus $0000 - $3FFF                 |
|    17 | External bus $4000 - $7FFF                 |
|    18 | External bus $8000 - $BFFF                 |
|    19 | External bus $C000 - $FFFF (Cartridge ROM) |
|    20 | Video RAM                                  |
|    21 | Character RAM                              |
| 22-31 | -                                          |
| 32-63 | RAM (512KB)                                |

### Page 4

Video RAM used by tile / bitmap / sprite engine.

| Address       | Description                       |
| ------------- | --------------------------------- |
| $0000 - $1F3F | 8KB Bitmap RAM                    |
| $2000 - $23E7 | 1KB Bitmap color RAM (40x25)      |
| $0000 - $0FFF | 4KB Tile map 64x32                |
| $0000 - $3FFF | 16KB Tile data (max 512 patterns) |

As seen in above table, the address ranges overlap. Since bitmap mode and tile mode are mutual exclusive, this doesn't pose a problem. The amount of available tile data that can be used for sprites is dependent on the selected mode.

**_Tile mode_** uses the _tile map_ (@ $3000) to determine which 8x8 pattern to display. This pattern is identified by an index which specifies a base address in video RAM: `vram_address = tile_idx * 32`. Each tile pattern uses 4 bits per pixel to specify the color index for each pixel. This color index is used to lookup the actual color in the tile/bitmap palette.

**_Bitmap mode_** uses the _bitmap RAM_ (@ $0000). It uses 1 bit per pixel to determine if a pixel is set or clear. The actual color index is determined by the _bitmap color RAM_ (@ $2000), which specifies the foreground/background color index per 8x8 pixels (similar to text color RAM). This color index is used to lookup the actual color in the tile/bitmap palette.

**_Sprites_** use the parameters set via the sprite IO registers. The sprites use the same tile map data format as the tile mode, but the color index lookup is performed using the sprite palette instead.

### Page 5

| Address       | Description       |
| ------------- | ----------------- |
| $0000 - $07FF | 2KB Character RAM |
| $0800 - $3FFF | -                 |

The character RAM is used by the text mode character generator to display text on the screen. The characters can be redefined by writing to the character RAM.

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
        <td>$E0</td>
        <td>VCTRL</td>
        <td colspan="4" align="center">-</td>
        <td colspan="1" align="center">Sprites enable</td>
        <td colspan="2" align="center">Tile map / bitmap mode</td>
        <td colspan="1" align="center">Text enable</td>
    </tr>
    <tr>
        <td>$E1</td>
        <td>VSCRX_L</td>
        <td colspan="8" align="center">Tile map X-scroll (7:0)</td>
    </tr>
    <tr>
        <td>$E2</td>
        <td>VSCRX_H</td>
        <td colspan="7" align="center">-</td>
        <td colspan="1" align="center">Tile map X-scroll (8)</td>
    </tr>
    <tr>
        <td>$E3</td>
        <td>VSCRY</td>
        <td colspan="8" align="center">Tile map Y-scroll</td>
    </tr>
    <tr>
        <td>$E4</td>
        <td>VSPRX_L</td>
        <td colspan="8" align="center">Sprite X-position (7:0)</td>
    </tr>
    <tr>
        <td>$E5</td>
        <td>VSPRX_H</td>
        <td colspan="8" align="center">Sprite X-position (8)</td>
    </tr>
    <tr>
        <td>$E6</td>
        <td>VSPRY</td>
        <td colspan="8" align="center">Sprite Y-position</td>
    </tr>
    <tr>
        <td>$E7</td>
        <td>VSPRIDX</td>
        <td colspan="8" align="center">Sprite tile index (7:0)</td>
    </tr>
    <tr>
        <td>$E8</td>
        <td>VSPRATTR</td>
        <td align="center" colspan="1">Enable</td>
        <td align="center" colspan="2">-</td>
        <td align="center" colspan="1">Priority</td>
        <td align="center" colspan="1">Palette</td>
        <td align="center" colspan="1">V-flip</td>
        <td align="center" colspan="1">H-flip</td>
        <td align="center" colspan="1">Tile index (8)</td>
    </tr>
    <tr>
        <td>$E9</td>
        <td>VPALTXT</td>
        <td align="center" colspan="4">Green</td>
        <td align="center" colspan="4">Blue</td>
    </tr>
    <tr>
        <td>$E9</td>
        <td>VPALTXT</td>
        <td align="center" colspan="4">-</td>
        <td align="center" colspan="4">Red</td>
    </tr>
    <tr>
        <td>$EA</td>
        <td>VPALTILE</td>
        <td align="center" colspan="4">Green</td>
        <td align="center" colspan="4">Blue</td>
    </tr>
    <tr>
        <td>$EA</td>
        <td>VPALTILE</td>
        <td align="center" colspan="4">-</td>
        <td align="center" colspan="4">Red</td>
    </tr>
    <tr>
        <td>$EB</td>
        <td>VPALSPR</td>
        <td align="center" colspan="4">Green</td>
        <td align="center" colspan="4">Blue</td>
    </tr>
    <tr>
        <td>$EB</td>
        <td>VPALSPR</td>
        <td align="center" colspan="4">-</td>
        <td align="center" colspan="4">Red</td>
    </tr>
    <tr>
        <td>$EC</td>
        <td>VLINE</td>
        <td align="center" colspan="8">Current line number</td>
    </tr>
    <tr>
        <td>$ED</td>
        <td>VIRQLINE</td>
        <td align="center" colspan="8">Line number at which to generate IRQ</td>
    </tr>
    <tr>
        <td>$EE</td>
        <td>IRQMASK</td>
        <td align="center" colspan="6">-</td>
        <td align="center" colspan="1">Line</td>
        <td align="center" colspan="1">VSync</td>
    </tr>
    <tr>
        <td>$EF</td>
        <td>IRQSTAT</td>
        <td align="center" colspan="6">-</td>
        <td align="center" colspan="1">Line</td>
        <td align="center" colspan="1">VSync</td>
    </tr>
</table>

For registers $E4-$E8, A11-A8 determines which sprite to read or write. (eg. use a OUT (C),A instruction with B containing palette index)  
For registers $E9-$EB, A11-A8 determines which palette entry to read or write. (eg. use a OUT (C),A instruction with B containing palette index)

The current line number is reflected in the **_VLINE_** register. Internal line number range from 0-261. Line numbers above 255 are reflected in this register as 255.

The **_IRQMASK_** register determines which video events generate an interrupt.  
The **_IRQSTAT_** indicates pending interrupts when read. When writing a 1 to a corresponding bit the event is cleared.

**_Text mode_** and **_sprites_** can be enabled simultaneously with either **_tile map mode_** or **_bitmap mode_**.

The **_tile map scroll registers_** determine which part of the **_tile map_** is visible on the screen. They determine the offset within the tile map for the upper left corner of the screen.

**_Sprites_** can be set partially off-screen by using negative values. For example, a sprite positioned at X-position 508, will show 4 pixels on the left side of the screen.

**Implementation detail**: when writing to the lower byte of a palette entry, the entry itself isn’t yet updated, but the value written is stored in a latch. When writing the upper byte the value is combined with the value of the latch and the complete 12-bit value is written.

Similarly writing to VSCRX_L or VSPRX_L will store the value in a latch. Only when writing to VSCRX_H or VSPRX_H, the latched value is combined with written value and the complete scroll register is updated.


### Tile map / bitmap mode

| Value | Description       |
| ----: | ----------------- |
|     0 | Disabled          |
|     1 | 64x32 tilemap     |
|     2 | 320x200 bitmapped |
|     3 | -                 |

# Video RAM data

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

If the **_Priority_** bit is set, the tile is displayed in front of sprites (if sprite priority bit is not set as well).  
If the **_Palette_** bit is set, the sprite palette is used instead of the tile palette.  
If **_V-flip_** bit is set, the sprite is vertically mirrored.  
If **_H-flip_** bit is set, the sprite is horizontally mirrored.

## Tile data

512 tile pattern entries. Each pattern is 32 bytes in size. Each pattern is 8x8 pixels of 4 bits per pixel. Each byte contains 2 pixels, the upper 4 bits contains the palette index of the left pixel, the lower 4 bits the palette index of the right pixel.

# ESP32 interface

The Aq+ has an interface defined to communicate with an ESP32 module. This module is used for file storage and network connectivity.

Communication is done via 2 IO registers:

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
        <td>$F4</td>
        <td>ESPCTRL</td>
        <td colspan="1" align="center">Start frame</td>
        <td colspan="5" align="center">-</td>
        <td colspan="1" align="center">TX FIFO full</td>
        <td colspan="1" align="center">RX FIFO non-empty</td>
    </tr>
    <tr>
        <td>$F5</td>
        <td>ESPDATA</td>
        <td colspan="8" align="center">FIFO read / write</td>
    </tr>
</table>

Writing a 1 to _Start frame_ will generate a framing sequence.

Write a 1 to either _TX FIFO full_ or _RX FIFO non-empty_ will flush the respective FIFO buffer.

## Commands

| Value | Function | Description                              | Category                    |
| ----: | -------- | ---------------------------------------- | --------------------------- |
|   $01 | RESET    | Reset ESP                                | General                     |
|   $10 | OPEN     | Open / create file                       | File                        |
|   $11 | CLOSE    | Close open file                          | File                        |
|   $12 | READ     | Read from file                           | File                        |
|   $13 | WRITE    | Write to file                            | File                        |
|   $14 | SEEK     | Move read/write pointer                  | File                        |
|   $15 | TELL     | Get current read/write                   | File                        |
|   $16 | OPENDIR  | Open directory                           | Directory                   |
|   $17 | CLOSEDIR | Close open directory                     | Directory                   |
|   $18 | READDIR  | Read from directory                      | Directory                   |
|   $19 | DELETE   | Remove file or directory                 | File / Directory management |
|   $1A | RENAME   | Rename / move file or directory          | File / Directory management |
|   $1B | MKDIR    | Create directory                         | File / Directory management |
|   $1C | CHDIR    | Change directory                         | File / Directory management |
|   $1D | STAT     | Get file status                          | File / Directory management |
|   $1E | GETCWD   | Get current working directory            | File / Directory management |
|   $1F | CLOSEALL | Close any open file/directory descriptor | File / Directory            |

TODO: WiFi management commands

## Error codes

| Value | Function          | Description                       |
| ----: | ----------------- | --------------------------------- |
|    -1 | ERR_NOT_FOUND     | File / directory not found        |
|    -2 | ERR_TOO_MANY_OPEN | Too many open files / directories |
|    -3 | ERR_PARAM         | Invalid parameter                 |
|    -4 | ERR_EOF           | End of file / directory           |
|    -5 | ERR_EXISTS        | File already exists               |
|    -6 | ERR_OTHER         | Other error                       |
|    -7 | ERR_NO_DISK       | No disk                           |
|    -8 | ERR_NOT_EMPTY     | Not empty                         |

### RESET

| Offset | Value |
| ------ | ----- |
| 0      | $01   |

### OPEN

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $10                  |
| 1      | Flags                |
| 2-n    | Zero-terminated path |

#### Response

| Offset | Value                             |
| ------ | --------------------------------- |
| 0      | File descriptor / Error code (<0) |

### CLOSE

#### Request

| Offset | Value           |
| ------ | --------------- |
| 0      | $11             |
| 1      | File descriptor |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### READ

#### Request

| Offset | Value                   |
| ------ | ----------------------- |
| 0      | $12                     |
| 1      | File descriptor         |
| 2-3    | Length to read (16-bit) |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
| 1-2    | Length read                    |
| 3-n    | Data bytes                     |

### WRITE

#### Request

| Offset | Value                    |
| ------ | ------------------------ |
| 0      | $13                      |
| 1      | File descriptor          |
| 2-3    | Length to write (16-bit) |
| 4-n    | Data bytes               |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
| 1-2    | Length written                 |

### SEEK

#### Request

| Offset | Value           |
| ------ | --------------- |
| 0      | $14             |
| 1      | File descriptor |
| 2-5    | Seek offset     |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### TELL

#### Request

| Offset | Value           |
| ------ | --------------- |
| 0      | $15             |
| 1      | File descriptor |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
| 1-4    | Current offset                 |

### OPENDIR

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $16                  |
| 1-n    | Zero-terminated path |

#### Response

| Offset | Value                                  |
| ------ | -------------------------------------- |
| 0      | Directory descriptor / error code (<0) |

### CLOSEDIR

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $17                  |
| 1      | Directory descriptor |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### READDIR

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $18                  |
| 1      | Directory descriptor |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
| 1-2    | Date                           |
| 3-4    | Time                           |
| 5      | Attribute                      |
| 6-9    | File size                      |
| 10-n   | Zero terminated filename       |

### DELETE

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $19                  |
| 1-n    | Zero-terminated path |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### RENAME

#### Request

| Offset  | Value                    |
| ------- | ------------------------ |
| 0       | $1A                      |
| 1-n     | Old zero-terminated path |
| (n+1)-m | New zero-terminated path |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### MKDIR

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $1B                  |
| 1-n    | Zero-terminated path |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### CHDIR

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $1C                  |
| 1-n    | Zero-terminated path |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |

### STAT

#### Request

| Offset | Value                |
| ------ | -------------------- |
| 0      | $1D                  |
| 1-n    | Zero-terminated path |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
| 1-2    | Date                           |
| 3-4    | Time                           |
| 5      | Attribute                      |
| 6-9    | File size                      |

### GETCWD

#### Request

| Offset | Value |
| ------ | ----- |
| 0      | $1E   |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
| 1-n    | Zero-terminated path           |

### CLOSEALL

#### Request

| Offset | Value |
| ------ | ----- |
| 0      | $1F   |

#### Response

| Offset | Value                          |
| ------ | ------------------------------ |
| 0      | 0 on success / error code (<0) |
