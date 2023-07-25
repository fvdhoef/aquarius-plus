# Aquarius<sup>+</sup>

# Memory map

| Z80 Memory address | Description | Reg Hex/Int | Initial Register Value                                 |
| ------------------ | ----------- | ----------- | ------------------------------------------------------ |
| $0000 - $3FFF      | Bank 0      | $F0 / 240   | 192 = ROM Page 0 + [READ ONLY + OVERLAY](#overlay-ram) |
| $4000 - $7FFF      | Bank 1      | $F1 / 241   | 33 = RAM Page 33                                       |
| $8000 - $BFFF      | Bank 2      | $F2 / 242   | 34 = RAM Page 34                                       |
| $C000 - $FFFF      | Bank 3      | $F3 / 243   | 19 = Cartridge Port Page 19                            |

## Overlay RAM

When setting the **_Overlay RAM_** bit of the **BANK**x registers, the memory at offset $3000-$3FFF for that BANK is replaced with the functions listed in the chart below. This is not limited to BANK0, although the default code for existing Aquarius software expects it to be there. But why would anyone want to change this? As an example, screen update code could be rewritten to update BANK3 (starting at $C000) with the OVERLAY bit set, placing an access point to CHARRAM at $F000 ($C000 + $3000), COLRAM at $F400, and BASIC RAM at $F800. Since the BASIC interpreter expects its variables and code to start at $3800, this is likely not possible with BASIC -- you're pulling the rug out from under your BASIC interpreter -- but it's possible using assembly/machine code. Also, the OVERLAY bit can be set in any/all of the banks simlutaneously, which would allow a programmer to update the border character at $3000, $7000, $B000, and/or $F000.

| Offset        | Description     |
| ------------- | --------------- |
| $3000 - $33FF | CHARRAM (1KB)   |
| $3400 - $37FF | COLRAM (1KB)    |
| $3800 - $3FFF | BASIC RAM (2KB) |

# Banking

## Banked Memory Pointers

|           Page | Description                                   |
| -------------: | --------------------------------------------- |
|              0 | System ROM memory (16KB)                      |
|           1-15 | -                                             |
|          16-19 | Cartridge port (data via scrambling register) |
| [20](#page-20) | [Video RAM](#page-20)                         |
| [21](#page-21) | [Character RAM](#page-21)                     |
|          22-31 | -                                             |
|          32-63 | RAM (512KB)                                   |

### Page 20

Video RAM used by tile / bitmap / sprite engine.

| Offset        | Description                       |
| ------------- | --------------------------------- |
| $0000 - $1F3F | 8KB Bitmap RAM                    |
| $2000 - $23E7 | 1KB Bitmap color RAM (40x25)      |
| $0000 - $0FFF | 4KB Tile map 64x32                |
| $0000 - $3FFF | 16KB Tile data (max 512 patterns) |

As seen in above table, the address ranges overlap. Since bitmap mode and tile mode are mutual exclusive, this doesn't pose a problem. The amount of available tile data that can be used for sprites is dependent on the selected mode.

**_Bitmap mode_** uses the _bitmap RAM_ (@ $0000). It uses 1 bit per pixel to determine if a pixel is set or clear. The actual color index is determined by the _bitmap color RAM_ (@ $2000), which specifies the foreground/background color index per 8x8 pixels (similar to text color RAM). This color index is used to lookup the actual color in the tile/bitmap palette.

**_Tile mode_** uses the _tile map_ (@ $3000) to determine which 8x8 pattern to display. This pattern is identified by an index which specifies a base address in video RAM: `vram_address = tile_idx * 32`. Each tile pattern uses 4 bits per pixel to specify the color index for each pixel. This color index is used to lookup the actual color in the tile/bitmap palette.

**_Sprites_** use the parameters set via the sprite IO registers. The sprites use the same tile map data format as the tile mode, but the color index lookup is performed using the sprite palette instead.

### Page 21

| Offset        | Description       |
| ------------- | ----------------- |
| $0000 - $07FF | 2KB Character RAM |
| $0800 - $3FFF | -                 |

The character RAM is used by the text mode character generator to display text on the screen. The characters can be redefined by writing to the character RAM.

# Video registers

The current line number is reflected in the **_VLINE_** register. Internal line numbers range from 0-261. Line numbers above 255 are reflected in this register as 255.

The **_IRQMASK_** register determines which video events generate an interrupt.  
The **_IRQSTAT_** indicates pending interrupts when read. When writing a 1 to a corresponding bit the event is cleared.

**_Text mode_** and **_sprites_** can be enabled simultaneously with either **_tile map mode_** or **_bitmap mode_**.

The **_tile map scroll registers_** determine which part of the **_tile map_** is visible on the screen. They determine the offset within the tile map for the upper left corner of the screen.

**_Sprites_** can be set partially off-screen by using negative values. For example, a sprite positioned at X-position 508, will show 4 pixels on the left side of the screen.

### Graphics mode

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
        <td align="center" colspan="1">-</td>
        <td align="center" colspan="1">Priority</td>
        <td align="center" colspan="2">Palette</td>
        <td align="center" colspan="1">-</td>
        <td align="center" colspan="1">V-flip</td>
        <td align="center" colspan="1">H-flip</td>
        <td align="center" colspan="1">Tile index (8)</td>
    </tr>
</table>

If the **_Priority_** bit is set, the tile is displayed in front of sprites (if sprite priority bit is not set as well).  
If **_V-flip_** bit is set, the sprite is vertically mirrored.  
If **_H-flip_** bit is set, the sprite is horizontally mirrored.

## Tile data

512 tile pattern entries. Each pattern is 32 bytes in size. Each pattern is 8x8 pixels of 4 bits per pixel. Each byte contains 2 pixels, the upper 4 bits contains the palette index of the left pixel, the lower 4 bits the palette index of the right pixel.

## Video Line Register

The current line number is reflected in the **_VLINE_** register. Internal line numbers range from 0-261. Line numbers above 255 are reflected in this register as 255.

The **_IRQMASK_** register determines which video events generate an interrupt.  
The **_IRQSTAT_** indicates pending interrupts when read. When writing a 1 to a corresponding bit the event is cleared.

# IO map

<table>
    <tr>
        <th>IO reg</th>
        <th>Name</th>
        <th>Description</th>
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
        <td>$00-$DF</td>
        <td>-</td>
        <td colspan="1">-</td>
        <td colspan="8" align="center">-</td>
    </tr>
    <tr>
        <td>$E0/224</td>
        <td>VCTRL</td>
        <td colspan="1">Video control register</td>
        <td colspan="3" align="center">-</td>
        <td colspan="1" align="center">Text priority</td>
        <td colspan="1" align="center">Sprites enable</td>
        <td colspan="2" align="center">Graphics mode</td>
        <td colspan="1" align="center">Text enable</td>
    </tr>
    <tr>
        <td>$E1/225</td>
        <td>VSCRX_L</td>
        <td colspan="1">Tilemap horizontal scroll</td>
        <td colspan="8" align="center">Tile map X-scroll (7:0)</td>    
    </tr>
    <tr>
        <td>$E2/226</td>
        <td>VSCRX_H</td>
        <td colspan="1">Tilemap horizontal scroll</td>
        <td colspan="7" align="center">-</td>
        <td colspan="1" align="center">Tile map X-scroll (8)</td>
    </tr>
    <tr>
        <td>$E3/227</td>
        <td>VSCRY</td>
        <td colspan="1">Tilemap vertical scroll</td>
        <td colspan="8" align="center">Tile map Y-scroll</td>
    </tr>
    <tr>
        <td>$E4/228</td>
        <td>VSPRSEL</td>
        <td colspan="1">Sprite select</td>
        <td colspan="2" align="center">-</td>
        <td colspan="6" align="center">Sprite selection (0-63)</td>
    </tr>
    <tr>
        <td>$E5/229</td>
        <td>VSPRX_L</td>
        <td colspan="1">Sprite X-position</td>
        <td colspan="8" align="center">Sprite X-position (7:0)</td>
    </tr>
    <tr>
        <td>$E6/230</td>
        <td>VSPRX_H</td>
        <td colspan="1">Sprite X-position</td>
        <td colspan="7" align="center">-</td>
        <td colspan="1" align="center">Sprite X-position (8)</td>
    </tr>
    <tr>
        <td>$E7/231</td>
        <td>VSPRY</td>
        <td colspan="1">Sprite Y-position</td>
        <td colspan="8" align="center">Sprite Y-position</td>
    </tr>
    <tr>
        <td>$E8/232</td>
        <td>VSPRIDX</td>
        <td colspan="1">Sprite tile index</td>
        <td colspan="8" align="center">Sprite tile index (7:0)</td>
    </tr>
    <tr>
        <td>$E9/233</td>
        <td>VSPRATTR</td>
        <td colspan="1">Sprite attributes</td>
        <td align="center" colspan="1">Enable</td>
        <td align="center" colspan="1">Priority</td>
        <td align="center" colspan="2">Palette</td>
        <td align="center" colspan="1">Height: 16</td>
        <td align="center" colspan="1">V-flip</td>
        <td align="center" colspan="1">H-flip</td>
        <td align="center" colspan="1">Tile index (8)</td>
    </tr>
    <tr>
        <td>$EA/234</td>
        <td>VPALSEL</td>
        <td colspan="1">Palette entry selection</td>
        <td align="center" colspan="1">-</td>
        <td align="center" colspan="2">Palette</td>
        <td align="center" colspan="4">Entry</td>
        <td align="center" colspan="1">0:GB, 1:R</td>
    </tr>
    <tr>
        <td>$EB/235</td>
        <td>VPALDATA (GB)</td>
        <td colspan="1">Palette entry G/B</td>
        <td align="center" colspan="4">Green</td>
        <td align="center" colspan="4">Blue</td>
    </tr>
    <tr>
        <td>$EB/235</td>
        <td>VPALDATA (R)</td>
        <td colspan="1">Palette entry R</td>
        <td align="center" colspan="4">-</td>
        <td align="center" colspan="4">Red</td>
    </tr>
    <tr>
        <td>$EC/236</td>
        <td>VLINE</td>
        <td colspan="1">Current video line</td>
        <td align="center" colspan="8">Current line number</td>
    </tr>
    <tr>
        <td>$ED/237</td>
        <td>VIRQLINE</td>
        <td colspan="1">Video line at which to generate interrupt</td>
        <td align="center" colspan="8">Line number at which to generate IRQ</td>
    </tr>
    <tr>
        <td>$EE/238</td>
        <td>IRQMASK</td>
        <td colspan="1">Interrupt masking register</td>
        <td align="center" colspan="6">-</td>
        <td align="center" colspan="1">Line</td>
        <td align="center" colspan="1">V-blank</td>
    </tr>
    <tr>
        <td>$EF/239</td>
        <td>IRQSTAT</td>
        <td colspan="1">Interrupt status register</td>
        <td align="center" colspan="6">-</td>
        <td align="center" colspan="1">Line</td>
        <td align="center" colspan="1">V-blank</td>
    </tr>
    <tr>
        <td>$F0/240</td>
        <td>BANK0</td>
        <td colspan="1">Bank 0 ($0000-$3FFF) page</td>
        <td colspan="1" align="center">Read only</td>
        <td colspan="1" align="center">Overlay RAM</td>
        <td colspan="6" align="center">Page</td>
    </tr>
    <tr>
        <td>$F1/241</td>
        <td>BANK1</td>
        <td colspan="1">Bank 1 ($4000-$7FFF) page</td>
        <td colspan="1" align="center">Read only</td>
        <td colspan="1" align="center">Overlay RAM</td>
        <td colspan="6" align="center">Page</td>
    </tr>
    <tr>
        <td>$F2/242</td>
        <td>BANK2</td>
        <td colspan="1">Bank 2 ($8000-$BFFF) page</td>
        <td colspan="1" align="center">Read only</td>
        <td colspan="1" align="center">Overlay RAM</td>
        <td colspan="6" align="center">Page</td>
    </tr>
    <tr>
        <td>$F3/243</td>
        <td>BANK3</td>
        <td colspan="1">Bank 3 ($C000-$FFFF) page</td>
        <td colspan="1" align="center">Read only</td>
        <td colspan="1" align="center">Overlay RAM</td>
        <td colspan="6" align="center">Page</td>
    </tr>
    <tr>
        <td>$F4/244</td>
        <td>ESPCTRL</td>
        <td colspan="1">ESP32 control/status</td>
        <td colspan="1" align="center">Transmit break</td>
        <td colspan="2" align="center">-</td>
        <td colspan="1" align="center">RX FIFO overflow</td>
        <td colspan="1" align="center">RX framing error</td>
        <td colspan="1" align="center">RX break received</td>
        <td colspan="1" align="center">TX busy</td>
        <td colspan="1" align="center">RX FIFO non-empty</td>
    </tr>
    <tr>
        <td>$F5/245</td>
        <td>ESPDATA</td>
        <td colspan="1">ESP32 data</td>
        <td colspan="8" align="center">FIFO read / write</td>
    </tr>
    <tr>
        <td>$F6/246</td>
        <td>PSG1DATA</td>
        <td colspan="1">PSG1: AY-3-8910</td>
        <td colspan="8" align="center">Data</td>
    </tr>
    <tr>
        <td>$F7/247 W</td>
        <td>PSG1ADDR</td>
        <td colspan="1">PSG1: AY-3-8910</td>
        <td colspan="4" align="center">-</td>
        <td colspan="4" align="center">Address</td>
    </tr>
    <tr>
        <td>$F8/248</td>
        <td>PSG1DATA</td>
        <td colspan="1">PSG2: AY-3-8910</td>
        <td colspan="8" align="center">Data</td>
    </tr>
    <tr>
        <td>$F9/249 W</td>
        <td>PSG1ADDR</td>
        <td colspan="1">PSG2: AY-3-8910</td>
        <td colspan="4" align="center">-</td>
        <td colspan="4" align="center">Address</td>
    </tr>
    <tr>
        <td>$FA/250</td>
        <td>-</td>
        <td colspan="1">-</td>
        <td align="center" colspan="8">-</td>
    </tr>
    <tr>
        <td>$FB/251</td>
        <td>SYSCTRL</td>
        <td colspan="1">System control register</td>
        <td align="center" colspan="6">-</td>
        <td align="center" colspan="1">Disable PSGs</td>
        <td align="center" colspan="1">Disable regs $E0-$F5, $F8-$F9</td>
    </tr>
    <tr>
        <td>$FC/252 R</td>
        <td>CASSETTE</td>
        <td colspan="1">Cassette interface</td>
        <td align="center" colspan="7">-</td>
        <td align="center" colspan="1">Cassette input</td>
    </tr>
    <tr>
        <td>$FC/252 W</td>
        <td>CASSETTE</td>
        <td colspan="1">Cassette interface</td>
        <td align="center" colspan="7">-</td>
        <td align="center" colspan="1">Cassette/sound output</td>
    </tr>
    <tr>
        <td>$FD/253 R</td>
        <td>VSYNC</td>
        <td colspan="1">V-sync indication</td>
        <td align="center" colspan="7">-</td>
        <td align="center" colspan="1">V-sync signal (0: in v-sync)</td>
    </tr>
    <tr>
        <td>$FD/253 W</td>
        <td>CPM</td>
        <td colspan="1">CP/M memory remapping</td>
        <td align="center" colspan="7">-</td>
        <td align="center" colspan="1">Enable CP/M remapping</td>
    </tr>
    <tr>
        <td>$FE/254 R</td>
        <td>PRINTER</td>
        <td colspan="1">Serial printer</td>
        <td align="center" colspan="7">-</td>
        <td align="center" colspan="1">Clear to send status</td>
    </tr>
    <tr>
        <td>$FE/254 W</td>
        <td>PRINTER</td>
        <td colspan="1">Serial printer</td>
        <td align="center" colspan="7">-</td>
        <td align="center" colspan="1">Serial printer output</td>
    </tr>
    <tr>
        <td>$FF/255 R</td>
        <td>KEYBOARD</td>
        <td colspan="1">Keyboard</td>
        <td align="center" colspan="2">-</td>
        <td align="center" colspan="6">Keyboard</td>
    </tr>
    <tr>
        <td>$FF/255 W</td>
        <td>SCRAMBLE</td>
        <td colspan="1">Scramble</td>
        <td align="center" colspan="8">External bus scramble value (XOR) - not present in Aq+</td>
    </tr>
</table>

# ESP32 interface (ESPCTRL/ESPDATA)

The Aq+ has an interface to communicate with an ESP32 module. This interface is used for file storage and network connectivity.

Communication is done via 2 IO registers: ESPCTRL and ESPDATA.

Writing a 1 to _Transmit break_ will transmit a BREAK condition to indicate the start of a new command.

## Commands

| Value | Function | Description                                | Category                    |
| ----: | -------- | ------------------------------------------ | --------------------------- |
|   $01 | RESET    | Indicate to ESP that system has been reset | General                     |
|   $10 | OPEN     | Open / create file                         | File                        |
|   $11 | CLOSE    | Close open file                            | File                        |
|   $12 | READ     | Read from file                             | File                        |
|   $13 | WRITE    | Write to file                              | File                        |
|   $14 | SEEK     | Move read/write pointer                    | File                        |
|   $15 | TELL     | Get current read/write                     | File                        |
|   $16 | OPENDIR  | Open directory                             | Directory                   |
|   $17 | CLOSEDIR | Close open directory                       | Directory                   |
|   $18 | READDIR  | Read from directory                        | Directory                   |
|   $19 | DELETE   | Remove file or directory                   | File / Directory management |
|   $1A | RENAME   | Rename / move file or directory            | File / Directory management |
|   $1B | MKDIR    | Create directory                           | File / Directory management |
|   $1C | CHDIR    | Change directory                           | File / Directory management |
|   $1D | STAT     | Get file status                            | File / Directory management |
|   $1E | GETCWD   | Get current working directory              | File / Directory management |
|   $1F | CLOSEALL | Close any open file/directory descriptor   | File / Directory            |

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
