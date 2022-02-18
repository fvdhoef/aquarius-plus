# Aquarius<sup>+</sup>

# Memory map

| Memory address | Description |
|----------------|-------------|
| $0000 - $3FFF  | Bank 0      |
| $4000 - $7FFF  | Bank 1      |
| $8000 - $BFFF  | Bank 2      |
| $C000 - $FFFF  | Bank 3      |

# IO map

| IO address | Description                                                       |
|------------|-------------------------------------------------------------------|
| $00 - $DF  | -                                                                 |
| $E0        | Video: VCTRL                                                      |
| $E1        | Video: SCRX_L - Tilemap horizontal scroll (7:0)                   |
| $E2        | Video: SCRX_H - Tilemap horizontal scroll (8)                     |
| $E3        | Video: SCRY - Tilemap vertical scroll                             |
| $E4        | Video: SPRX_L - Sprite X-position (7:0)                           |
| $E5        | Video: SPRX_H - Sprite X-position (8)                             |
| $E6        | Video: SPRY - Sprite Y-position                                   |
| $E7        | Video: SPRIDX - Sprite tile index (7:0)                           |
| $E8        | Video: SPRATTR - Sprite attributes                                |
| $E9        | Video: PALTXT - Text palette                                      |
| $EA        | Video: PALTILE - Tile/bitmap palette                              |
| $EB        | Video: PALSPR - Sprite palette                                    |
| $EC        | Video: LINE - Current line                                        |
| $ED        | Video: IRQLINE - Line at which to generate interrupt              |
| $EE        | IRQMASK - Interrupt masking register                              |
| $EF        | IRQSTAT - Interrupt status register                               |
| $F0        | Bank 0 ($0000-$3FFF) page                                         |
| $F1        | Bank 1 ($4000-$7FFF) page                                         |
| $F2        | Bank 2 ($8000-$BFFF) page                                         |
| $F3        | Bank 3 ($C000-$FFFF) page                                         |
| $F4        | ESPCTL - ESP32 control/status                                     |
| $F5        | ESPDAT - ESP32 data                                               |
| $F6        | AY-3-8910 (left/both) R: read from PSG, W: write to PSG           |
| $F7        | AY-3-8910 (left/both) R: read from PSG, W: write to latch address |
| $F8        | AY-3-8910 (right) R: read from PSG, W: write to PSG               |
| $F9        | AY-3-8910 (right) R: read from PSG, W: write to latch address     |
| $FA        | Audio control<br/>RW<0>: Send first PSG to both channels          |
| $FB        | System control<br/>RW<0>: Disable access to extended registers    |
| $FC        | W<0>: Cassette/Sound output<br/>R<0>: Cassette input              |
| $FD        | W<0>: CP/M mode remapping<br/>R<0>: V-sync signal (0: in v-sync)  |
| $FE        | W<0>: Serial printer (1200bps)<br/>R<0>: Clear to send status     |
| $FF        | W<7:0>: External bus scramble value (XOR)<br/>R<5:0>: Keyboard    |

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

When setting the ***Overlay RAM*** bit, $3000-$3FFF is replaced with:

| Address       | Description                                  |
|---------------|----------------------------------------------|
| $3000 - $33FF | Screen RAM (1KB)                             |
| $3400 - $37FF | Color RAM (1KB)                              |
| $3800 - $3FFF | Basic RAM (2KB)                              |

### Banked memory map

| Page  | Description                                |
|------:|--------------------------------------------|
| 0     | External bus $0000 - $3FFF                 |
| 1     | External bus $4000 - $7FFF                 |
| 2     | External bus $8000 - $BFFF                 |
| 3     | External bus $C000 - $FFFF (Cartridge ROM) |
| 4     | Video RAM                                  |
| 5     | Character RAM                              |
| 6-15  | -                                          |
| 16-31 | Flash memory (256KB)                       |
| 32-63 | RAM (512KB)                                |

### Page 4

Video RAM used by tile / bitmap / sprite engine.

| Address       | Description                       |
|---------------|-----------------------------------|
| $0000 - $1BFF | 8KB Bitmap RAM                    |
| $2000 - $23BF | 1KB Bitmap color RAM (40x24)      |
| $0000 - $0FFF | 4KB Tile map 64x32                |
| $0000 - $3FFF | 16KB Tile data (max 512 patterns) |

As seen in above table, the address ranges overlap. Since bitmap mode and tile mode are mutual exclusive, this doesn't pose a problem. The amount of available tile data that can be used for sprites is dependent on the selected mode.

***Tile mode*** uses the *tile map* (@ $3000) to determine which 8x8 pattern to display. This pattern is identified by an index which specifies a base address in video RAM: `vram_address = tile_idx * 32`. Each tile pattern uses 4 bits per pixel to specify the color index for each pixel. This color index is used to lookup the actual color in the tile/bitmap palette.

***Bitmap mode*** uses the *bitmap RAM* (@ $0000). It uses 1 bit per pixel to determine if a pixel is set or clear. The actual color index is determined by the *bitmap color RAM* (@ $2000), which specifies the foreground/background color index per 8x8 pixels (similar to text color RAM). This color index is used to lookup the actual color in the tile/bitmap palette.

***Sprites*** use the parameters set via the sprite IO registers. The sprites use the same tile map data format as the tile mode, but the color index lookup is performed using the sprite palette instead.

### Page 5

| Address       | Description                   |
|---------------|-------------------------------|
| $0000 - $07FF | 2KB Character RAM             |
| $0800 - $3FFF | -                             |

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
        <td>SCRX_L</td>
        <td colspan="8" align="center">Tile map X-scroll (7:0)</td>
    </tr>
    <tr>
        <td>$E2</td>
        <td>SCRX_H</td>
        <td colspan="7" align="center">-</td>
        <td colspan="1" align="center">Tile map X-scroll (8)</td>
    </tr>
    <tr>
        <td>$E3</td>
        <td>SCRY</td>
        <td colspan="8" align="center">Tile map Y-scroll</td>
    </tr>
    <tr>
        <td>$E4</td>
        <td>SPRX_L</td>
        <td colspan="8" align="center">Sprite X-position (7:0)</td>
    </tr>
    <tr>
        <td>$E5</td>
        <td>SPRX_H</td>
        <td colspan="8" align="center">Sprite X-position (8)</td>
    </tr>
    <tr>
        <td>$E6</td>
        <td>SPRY</td>
        <td colspan="8" align="center">Sprite Y-position</td>
    </tr>
    <tr>
        <td>$E7</td>
        <td>SPRIDX</td>
        <td colspan="8" align="center">Sprite tile index (7:0)</td>
    </tr>
    <tr>
        <td>$E8</td>
        <td>SPRATTR</td>
        <td align="center" colspan="3">-</td>
        <td align="center" colspan="1">Priority</td>
        <td align="center" colspan="1">Palette</td>
        <td align="center" colspan="1">V-flip</td>
        <td align="center" colspan="1">H-flip</td>
        <td align="center" colspan="1">Tile index (8)</td>
    </tr>
    <tr>
        <td>$E9</td>
        <td>PALTXT</td>
        <td align="center" colspan="2">-</td>
        <td align="center" colspan="2">B</td>
        <td align="center" colspan="2">G</td>
        <td align="center" colspan="2">R</td>
    </tr>
    <tr>
        <td>$EA</td>
        <td>PALTILE</td>
        <td align="center" colspan="2">-</td>
        <td align="center" colspan="2">B</td>
        <td align="center" colspan="2">G</td>
        <td align="center" colspan="2">R</td>
    </tr>
    <tr>
        <td>$EB</td>
        <td>PALSPR</td>
        <td align="center" colspan="2">-</td>
        <td align="center" colspan="2">B</td>
        <td align="center" colspan="2">G</td>
        <td align="center" colspan="2">R</td>
    </tr>
    <tr>
        <td>$EC</td>
        <td>LINE</td>
        <td align="center" colspan="8">Current line number</td>
    </tr>
    <tr>
        <td>$ED</td>
        <td>IRQLINE</td>
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

For registers $E4-$E8, the A register determines which sprite to read or write.  
For registers $E9-$EB, the A register determines which palette entry to read or write.

The ***IRQMASK*** register determines which video events generate an interrupt.  
The ***IRQSTAT*** indicates pending interrupts when read. When writing a 1 to a corresponding bit the event is cleared.


***Text mode*** and ***sprites*** can be enabled simultaneously with either ***tile map mode*** or ***bitmap mode***.

The ***tile map scroll registers*** determine which part of the ***tile map*** is visible on the screen. They determine the offset within the tile map for the upper left corner of the screen.

***Sprites*** can be set partially off-screen by using negative values. For example, a sprite positioned at X-position 508, will show 4 pixels on the left side of the screen.


### Tile map / bitmap mode

| Value | Description          |
|------:|----------------------|
| 0     | Disabled             |
| 1     | 64x32 tilemap        |
| 2     | 320x192 bitmapped    |
| 3     | -                    |

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

If priority bit is set, the tile is displayed in front of sprites (if sprite priority bit is not set).

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
        <td>ESPCTL</td>
        <td colspan="1" align="center">Start frame</td>
        <td colspan="5" align="center">-</td>
        <td colspan="1" align="center">TX FIFO full</td>
        <td colspan="1" align="center">RX FIFO non-empty</td>
    </tr>
    <tr>
        <td>$F5</td>
        <td>ESPDAT</td>
        <td colspan="8" align="center">FIFO read / write</td>
    </tr>
</table>

Writing a 1 to *Start frame* will generate a framing sequence.

Write a 1 to either *TX FIFO full* or *RX FIFO non-empty* will flush the respective FIFO buffer.

## Commands

| Value | Function | Description                     | Category                    |
|------:|----------|---------------------------------|-----------------------------|
| $10   | OPEN     | Open / create file              | File                        |
| $11   | CLOSE    | Close open file                 | File                        |
| $12   | READ     | Read from file                  | File                        |
| $13   | WRITE    | Write to file                   | File                        |
| $14   | SEEK     | Move read/write pointer         | File                        |
| $15   | TELL     | Get current read/write          | File                        |
| $16   | OPENDIR  | Open directory                  | Directory                   |
| $17   | CLOSEDIR | Close open directory            | Directory                   |
| $18   | READDIR  | Read from directory             | Directory                   |
| $19   | UNLINK   | Remove file or directory        | File / Directory management |
| $1A   | RENAME   | Rename / move file or directory | File / Directory management |
| $1B   | MKDIR    | Create directory                | File / Directory management |
| $1C   | CHDIR    | Change directory                | File / Directory management |
| $1D   | STAT     | Get file status                 | File / Directory management |

TODO: WiFi management commands

### OPEN

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $10                  |
| 1      | Flags                |
| 2-n    | Zero-terminated path |

#### Response
| Offset | Value                             |
|--------|-----------------------------------|
| 0      | File descriptor / Error code (<0) |

### CLOSE

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $11                  |
| 1-n    | File descriptor      |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### READ

#### Request
| Offset | Value                   |
|--------|-------------------------|
| 0      | $12                     |
| 1-2    | Length to read (16-bit) |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |
| 1-2    | Length read                    |
| 3-n    | Data bytes                     |

### WRITE
#### Request
| Offset | Value                    |
|--------|--------------------------|
| 0      | $13                      |
| 1-2    | Length to write (16-bit) |
| 3-n    | Data bytes               |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |
| 1-2    | Length written                 |

### SEEK
#### Request
| Offset | Value                       |
|--------|-----------------------------|
| 0      | $14                         |
| 1-4    | Seek offset                 |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### TELL
#### Request
| Offset | Value                       |
|--------|-----------------------------|
| 0      | $15                         |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |
| 1-4    | Current offset                 |

### OPENDIR

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $16                  |
| 1-n    | Zero-terminated path |

#### Response
| Offset | Value                                  |
|--------|----------------------------------------|
| 0      | Directory descriptor / error code (<0) |

### CLOSEDIR

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $17                  |
| 1      | Directory descriptor |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### READDIR

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $18                  |
| 1      | Directory descriptor |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |
| 1-4    | File size                      |
| 5-6    | Date                           |
| 7-8    | Time                           |
| 9      | Attribute                      |
| 10-n   | Zero terminated filename       |

### UNLINK

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $19                  |
| 1-n    | Zero-terminated path |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### RENAME

#### Request
| Offset  | Value                    |
|---------|--------------------------|
| 0       | $1A                      |
| 1-n     | Old zero-terminated path |
| (n+1)-m | New zero-terminated path |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### MKDIR

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $1B                  |
| 1-n    | Zero-terminated path |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### CHDIR

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $1C                  |
| 1-n    | Zero-terminated path |

#### Response
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |

### STAT

#### Request
| Offset | Value                |
|--------|----------------------|
| 0      | $1D                  |
| 1-n    | Zero-terminated path |

#### Response (same as readdir)
| Offset | Value                          |
|--------|--------------------------------|
| 0      | 0 on success / error code (<0) |
| 1-4    | File size                      |
| 5-6    | Date                           |
| 7-8    | Time                           |
| 9      | Attribute                      |
| 10-n   | Zero terminated filename       |
