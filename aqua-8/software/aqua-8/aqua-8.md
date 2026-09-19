Aqua-8

Drawing acceleration registers:

| Register   | Width | Description                                                 |
| ---------- | ----: | ----------------------------------------------------------- |
| POSX       |    16 | X position (only drawing when <192)                         |
| POSY       |    16 | Y position (only drawing when <160)                         |
| POSX1616   |    32 | X position (lower 16-bit discarded)                         |
| POSY1616   |    32 | Y position (lower 16-bit discarded)                         |
| FLAGS      |       | 0:x-incr, 1:y-incr                                          |
| COLOR1BPP  |     4 | Color to use for 1bpp operations                            |
| COLORREMAP |  16x5 | Color remapping (bit 4: transparency, bit 3-0: color index) |
| DRAW1BPP   |     8 | Draw 8 pixels at 1bpp                                       |
| DRAW4BPP   |    32 | Draw 8 pixels at 4bpp                                       |
