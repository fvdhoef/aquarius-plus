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

| IO address| Description                                                      |
|-----------|------------------------------------------------------------------|
| $00 - $7D | Unallocated                                                      |
| $7E       | Modem (?)                                                        |
| $7F       | Modem (?)                                                        |
| $80 - $E7 | Unallocated                                                      |
| $E8 - $EA | Floppy disk interface (?)                                        |
| $EB - $EF | Unallocated                                                      |
| $F0 - $F5 | Unallocated                                                      |
| $F6       | AY-3-8910 R: read from PSG, W: write to PSG                      |
| $F7       | AY-3-8910 R: read from PSG, W: write to latch address            |
| $F8 - $FB | Unallocated                                                      |
| $FC       | W<0>: Casette/Sound output<br/>R<0>: Casette input               |
| $FD       | W<0>: CP/M mode remapping<br/>R<0>: V-sync signal (0: in v-sync) |
| $FE       | W<0>: Serial printer (1200bps)<br/>R<0>: Clear to send status    |
| $FF       | W<7:0>: External bus scramble value (XOR)<br/>R<5:0>: Keyboard   |
