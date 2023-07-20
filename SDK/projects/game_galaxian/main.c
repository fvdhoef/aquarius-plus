/*
Galaxians
Based on the code by Steven Hugg
8bitworkshop.com, https://8bitworkshop.com/v3.10.1/?file=shoot2.c&platform=galaxian-scramble

Adapted for Aquarius+
by Sean P. Harrington, sph@1stage.com

Abstract -
This is an exercise in transcribing and porting code from a legacy hardware game system to the Aquarius+ platform. The game being modeled is Galaxian, as implemented on Scramble hardware, featuring a Z80 processor, 2-bit graphics, and sound.

Version History -
2023-07-20  v0.01  - Begin the documentation / commenting of code. - SPH

*/

#include <aqplus.h>                                         // Include the master header file for Aquarius+ code. It is in ../../include.

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned char sbyte;

byte __at (0x4800) vram[32][32];

struct {
  byte scroll;
  byte attrib;
} __at (0x5000) vcolumns[32];

struct {
  byte xpos;
  byte code;
  byte color;
  byte ypos;
} __at (0x5040) vsprites[8];

struct {
  byte unused1;
  byte xpos;
  byte unused2;
  byte ypos;
} __at (0x5060) vmissiles[8];

byte __at (0x6801) enable_irq;
byte __at (0x6804) enable_stars;
byte __at (0x6808) missile_width;
byte __at (0x6809) missile_offset;
byte __at (0x7000) watchdog;
volatile byte __at (0x8100) input0;
volatile byte __at (0x8101) input1;
volatile byte __at (0x8102) input2;

#define LEFT1 !(input0 & 0x20)                          // Left button is the value of the input0 matrix & bit 6, inverted
#define RIGHT1 !(input0 & 0x10)
#define FIRE1 !(input0 & 0x8)

#define ENEMIES_PER_ROW 8
#define ENEMY_ROWS 4
#define MAX_IN_FORMATION (ENEMIES_PER_ROW*ENEMY_ROWS)
#define MAX_ATTACKERS 6

FormationEnemy formation[MAX_IN_FORMATION];
AttackingEnemy attackers[MAX_ATTACKERS];
Missile missiles[8];

static uint8_t formation_offset_x;
static char formation_direction;
static uint8_t current_row;
static uint8_t player_x;
const byte player_y = 232;
static uint8_t player_exploding;
uint8_t enemy_exploding;
uint8_t enemies_left;
uint16_t player_score;
uint16_t framecount;

// Set up SOUND DEVICES
inline void set8910a(byte rega, byte dataa) {
  IO_PSG1ADDR = rega;
  IO_PSG1DATA = dataa;
}

inline void set8910b(byte regb, byte datab) {
  IO_PSG2ADDR = regb;
  IO_PSG2DATA = datab;
}

typedef enum {
  AY_PITCH_A_LO, AY_PITCH_A_HI,
  AY_PITCH_B_LO, AY_PITCH_B_HI,
  AY_PITCH_C_LO, AY_PITCH_C_HI,
  AY_NOISE_PERIOD,
  AY_ENABLE,
  AY_ENV_VOL_A,
  AY_ENV_VOL_B,
  AY_ENV_VOL_C,
  AY_ENV_PERI_LO, AY_ENV_PERI_HI,
  AY_ENV_SHAPE
} AY8910Register;

// Line beneath this was from the old code. A vestige? Error?
// void main();

void start() __naked {
__asm
	LD      SP,#0x4800
        EI
; copy initialized data to RAM
        LD    BC, #l__INITIALIZER
        LD    A, B
        LD    DE, #s__INITIALIZED
        LD    HL, #s__INITIALIZER
        LDIR
  	JP    _main
; padding to get to offset 0x66
  	.ds   0x66 - (. - _start)
__endasm;
}

volatile byte video_framecount; // actual framecount

// starts at address 0x66
void rst_66() __interrupt {
  video_framecount++;
}

#define LOCHAR 0x30
#define HICHAR 0xff

#define CHAR(ch) (ch-LOCHAR)

#define BLANK 0x10

void memset_safe(void* _dest, char ch, word size) {
  byte* dest = _dest;
  while (size--) {
    *dest++ = ch;
  }
}

void clrscr() {
  memset_safe(vram, BLANK, sizeof(vram));
}

void reset_video_framecount() __critical {
  video_framecount = 0;
}

byte getchar(byte x, byte y) {
  return vram[29-x][y];
}

void putchar(byte x, byte y, byte ch) {
  vram[29-x][y] = ch;
}

void clrobjs() {
  byte i;
  memset_safe(vcolumns, 0, 0x100);
  for (i=0; i<8; i++)
    vsprites[i].ypos = 64;
}

void putstring(byte x, byte y, const char* string) {
  while (*string) {
    putchar(x++, y, CHAR(*string++));
  }
}

char in_rect(byte x, byte y, byte x0, byte y0, byte w, byte h) {
  return ((byte)(x-x0) < w && (byte)(y-y0) < h); // unsigned
}

void draw_bcd_word(byte x, byte y, word bcd) {
  byte j;
  x += 3;
  for (j=0; j<4; j++) {
    putchar(x, y, CHAR('0'+(bcd&0xf)));
    x--;
    bcd >>= 4;
  }
}

// add two 16-bit BCD values
word bcd_add(word a, word b) __naked {
  a; b; // to avoid warning
__asm
 	push	ix
 	ld	ix,#0
	add	ix,sp
 	ld	a,4 (ix)
 	add	a, 6 (ix)
	daa
	ld	c,a
 	ld	a,5 (ix)
 	adc	a, 7 (ix)
	daa
 	ld	b,a
 	ld	l, c
 	ld	h, b
	pop	ix
 	ret
__endasm;
}

// https://en.wikipedia.org/wiki/Linear-feedback_shift_register#Galois_LFSRs
static word lfsr = 1;
word rand() {
  byte lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
  lfsr >>= 1;                /* Shift register */
  if (lsb) {                 /* If the output bit is 1, apply toggle mask. */
    lfsr ^= 0xB400u;
  }
  return lfsr;
}

// GAME CODE

typedef struct {
  byte shape;
} FormationEnemy;

// should be power of 2 length
typedef struct {
  byte findex;
  byte shape;
  word x;
  word y;
  byte dir;
  byte returning;
} AttackingEnemy;

typedef struct {
  signed char dx;
  byte xpos;
  signed char dy;
  byte ypos;
} Missile;

void add_score(word bcd) {
  player_score = bcd_add(player_score, bcd);
  draw_bcd_word(0, 1, player_score);
  putchar(4, 1, CHAR('0'));
}

void setup_formation() {
  byte i;
  memset(formation, 0, sizeof(formation));
  memset(attackers, 0, sizeof(attackers));
  memset(missiles, 0, sizeof(missiles));
  for (i=0; i<MAX_IN_FORMATION; i++) {
    byte flagship = i < ENEMIES_PER_ROW;
    formation[i].shape = flagship ? 0x43 : 0x43;
  }
  enemies_left = MAX_IN_FORMATION;
}

void draw_row(byte row) {
  byte i;
  byte y = 4 + row * 2;
  vcolumns[y].attrib = 0x2;
  vcolumns[y].scroll = formation_offset_x;
  for (i=0; i<ENEMIES_PER_ROW; i++) {
    byte x = i * 3;
    byte shape = formation[i + row*ENEMIES_PER_ROW].shape;
    if (shape) {
      putchar(x, y, shape);
      putchar(x+1, y, shape-2);
    } else {
      putchar(x, y, BLANK);
      putchar(x+1, y, BLANK);
    }
  }
}

void draw_next_row() {
  draw_row(current_row);
  if (++current_row == ENEMY_ROWS) {
    current_row = 0;
    formation_offset_x += formation_direction;
    if (formation_offset_x == 40) {
      formation_direction = -1;
    }
    else if (formation_offset_x == 0) {
      formation_direction = 1;
    }
  }
}

#define FLIPX 0x40
#define FLIPY 0x80
#define FLIPXY 0xc0

const byte DIR_TO_CODE[32] = {
  0, 1, 2, 3, 4, 5, 6, 6,
  6|FLIPXY, 6|FLIPXY, 5|FLIPXY, 4|FLIPXY, 3|FLIPXY, 2|FLIPXY, 1|FLIPXY, 0|FLIPXY,
  0|FLIPX, 1|FLIPX, 2|FLIPX, 3|FLIPX, 4|FLIPX, 5|FLIPX, 6|FLIPX, 6|FLIPX,
  6|FLIPY, 6|FLIPY, 5|FLIPY, 4|FLIPY, 3|FLIPY, 2|FLIPY, 1|FLIPY, 0|FLIPY,
};

const byte SINTBL[32] = {
  0, 25, 49, 71, 90, 106, 117, 125,
  127, 125, 117, 106, 90, 71, 49, 25,
  0, -25, -49, -71, -90, -106, -117, -125,
  -127, -125, -117, -106, -90, -71, -49, -25,
};

signed char isin(byte dir) {
  return SINTBL[dir & 31];
}

signed char icos(byte dir) {
  return isin(dir+8);
}

#define FORMATION_X0 18
#define FORMATION_Y0 27
#define FORMATION_XSPACE 24
#define FORMATION_YSPACE 16

byte get_attacker_x(byte formation_index) {
  byte column = (formation_index % ENEMIES_PER_ROW);
  return FORMATION_XSPACE*column + FORMATION_X0 + formation_offset_x;
}

byte get_attacker_y(byte formation_index) {
  byte row = formation_index / ENEMIES_PER_ROW;
  return FORMATION_YSPACE*row + FORMATION_Y0;
}

void draw_attacker(byte i) {
  AttackingEnemy* a = &attackers[i];
  if (a->findex) {
    byte code = DIR_TO_CODE[a->dir & 31];
    vsprites[i].code = code + a->shape + 14;
    vsprites[i].xpos = a->x >> 8;
    vsprites[i].ypos = a->y >> 8;
    vsprites[i].color = 2;
  } else {
    vsprites[i].ypos = 255; // offscreen
  }
}

void draw_attackers() {
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    draw_attacker(i);
  }
}

void return_attacker(AttackingEnemy* a) {
  byte fi = a->findex-1;
  byte destx = get_attacker_x(fi);
  byte desty = get_attacker_y(fi);
  byte ydist = desty - (a->y >> 8);
  // are we close to our formation slot?
  if (ydist == 0) {
    // convert back to formation enemy
    formation[fi].shape = a->shape;
    a->findex = 0;
  } else {
    a->dir = (ydist + 16) & 31;
    a->x = destx << 8;
    a->y += 128;
  }
}

void fly_attacker(AttackingEnemy* a) {
  a->x += isin(a->dir) * 2;
  a->y += icos(a->dir) * 2;
  if ((a->y >> 8) == 0) {
    a->returning = 1;
  }
}

void move_attackers() {
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex) {
      if (a->returning)
        return_attacker(a);
      else
        fly_attacker(a);
    }
  }
}

void think_attackers() {
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex) {
      // rotate?
      byte x = a->x >> 8;
      byte y = a->y >> 8;
      // don't shoot missiles after player exploded
      if (y < 128 || player_exploding) {
        if (x < 112) {
          a->dir++;
        } else {
          a->dir--;
        }
      } else {
        // lower half of screen
        // shoot a missile?
        if (missiles[i].ypos == 0) {
          missiles[i].ypos = 245-y;
          missiles[i].xpos = x+8;
          missiles[i].dy = -2;
        }
      }
    }
  }
}

void formation_to_attacker(byte formation_index) {
  byte i;
  // out of bounds? return
  if (formation_index >= MAX_IN_FORMATION)
    return;
  // nobody in formation? return
  if (!formation[formation_index].shape)
    return;
  // find an empty attacker slot
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex == 0) {
      a->x = get_attacker_x(formation_index) << 8;
      a->y = get_attacker_y(formation_index) << 8;
      a->shape = formation[formation_index].shape;
      a->findex = formation_index+1;
      a->dir = 0;
      a->returning = 0;
      formation[formation_index].shape = 0;
      break;
    }
  }
}

// Draws the player's ship on the screen
void draw_player() {
  vcolumns[29].attrib = 1;                                      // Activate the video columns attribute for the left half of the player sprite (???)
  vcolumns[30].attrib = 1;                                      // Activate the video columns attribute for the right half of the player sprite (???)
  vram[30][29] = 0x60;                                          // Set the tile index of the upper left quarter of the player sprite
  vram[31][29] = 0x62;                                          // Set the tile index of the lower left quarter of the player sprite
  vram[30][30] = 0x61;                                          // Set the tile index of the upper right quarter of the player sprite
  vram[31][30] = 0x63;                                          // Set the tile index of the lower right quarter of the player sprite
}

// Checks control input to move player or fire missile
void move_player() {
  if (LEFT1 && player_x > 16) player_x--;                       // If the left move button is active and player x coord is greater than 16, move one position left
  if (RIGHT1 && player_x < 224) player_x++;                     // If the right move button is active and player x coord is less than 224, move one position right
  if (FIRE1 && missiles[7].ypos == 0) {                         // If the fire button is active and at least one missile is unfired (y pos equal to 0)...
    missiles[7].ypos = 252-player_y;                            // ...set the y pos of the missile to just above the player (must be multiple of missile speed, below)...
    missiles[7].xpos = player_x+8;                              // ...and set the x pos of the missle to the middle of the player's sprite (x + 8)...
    missiles[7].dy = 4;                                         // ...and set the missile speed.
  }
  vcolumns[29].scroll = player_x;                               // (???) Update the video columns to draw the left half of the player sprite (???)
  vcolumns[30].scroll = player_x;                               // (???) Update the video columns to draw the right half of the player sprite (???)
}

// Update the position of the missiles, (???) and move unused missiles to the top of the array (???)
void move_missiles() {
  byte i;                                                       // Iterator i to regulate the for loop
  for (i=0; i<8; i++) {                                         // Go through each item in the missiles array...
    if (missiles[i].ypos) {                                     // ...if it's y pos is NOT zero...
      if ((byte)(missiles[i].ypos += missiles[i].dy) < 4) {     // ...and if the y pos incremented by the missile speed is less than four (near the top of screen)...
        missiles[i].xpos = 0xff;                                // ...move the missile's x pos to off screen...
        missiles[i].ypos = 0;                                   // ...and set it's y pos to zero.
      }
    }
  }
  memcpy(vmissiles, missiles, sizeof(missiles));                // Copy all "shadow missiles" to video memory
}

// Set enemy as ready to "blow up". There is only one enemy explosion shown at a time on the screen. A new enemy explosion resets this process and moves sprite 6 to the new location.
void blowup_at(byte x, byte y) {                                // Take in the x and y position of the destroyed alien.
  vsprites[6].color = 1;                                        // Set the sprite 6 color (palette???) to 1.
  vsprites[6].code = 28;                                        // Set the sprite 6 code (tile number) to 28, the first frame of the alien exploding animation.
  vsprites[6].xpos = x;                                         // Set the sprite 6 x pos to the former position of the alien.
  vsprites[6].ypos = y;                                         // Set the sprite 6 y pos to the former position of the alien.
  enemy_exploding = 1;                                          // Set the enemy explode (frame) counter to 1.
}

// Animate an enemy explosion.
void animate_enemy_explosion() {
  if (enemy_exploding) {                                        // If the enemy is exploding...
    vsprites[6].code = 28 + enemy_exploding++;                  // ...set the sprite 6 code (tile number) to 28 plus the current "frame", and increment that frame counter.
    if (enemy_exploding > 4)                                    // If the enemy frame counter is greater than four frames...
      enemy_exploding = 0;                                      // ...hide the explosion.
  }
}

// Animate a player explosion. This animation is in a 16 tile grid (4x4), centered on the last player location.
void animate_player_explosion() {
  byte z = player_exploding;                                    // Set variable z to the value (frame) of the player explosion
  if (z <= 5) {                                                 // If there are still animation frames left (<5)...
    if (z == 5) {                                               // ...and if we are on the final frame (5) of player explosion animation...
      memset_safe(&vram[29][28], BLANK, 4);                     // ...set the left column of four tiles to blank...
      memset_safe(&vram[30][28], BLANK, 4);                     // ...then set the middle left column of four tiles to blank...
      memset_safe(&vram[31][28], BLANK, 4);                     // ...then set the middle right column of four four tiles to blank...
      memset_safe(&vram[0][28], BLANK, 4);                      // ...and then set the right column of four tiles to blank.
    } else {                                                    // Otherwise, we're going to continue animation the player explosion.
      z = 0xb0 + (z<<4);                                        // Multiply our frame value (z) by 10, and add 176 to that, to turn z into a tile reference number.
      vcolumns[28].scroll = player_x;                           // Shift the offset of v column 28 to the player x pos (???).
      vcolumns[31].scroll = player_x;                           // Shift the offset of v column 31 to the player x pos (???).
      vcolumns[28].attrib = 2;                                  // Set the visibiliy (???) of v column 28 to "exploding" (???).
      vcolumns[29].attrib = 2;                                  // Set the visibiliy (???) of v column 29 to "exploding" (???).
      vcolumns[30].attrib = 2;                                  // Set the visibiliy (???) of v column 30 to "exploding" (???).
      vcolumns[31].attrib = 2;                                  // Set the visibiliy (???) of v column 31 to "exploding" (???).
      // The explosion animation takes up a 16 tile grid (4x4)
      vram[29][28] = z+0x0;                                     // Set the left column row 1 to tile reference plus 0 offset.
      vram[29][29] = z+0x1;                                     // Set the left column row 1 to tile reference plus 1 offset.
      vram[29][30] = z+0x4;                                     // Set the left column row 1 to tile reference plus 4 offset.
      vram[29][31] = z+0x5;                                     // Set the left column row 1 to tile reference plus 5 offset.
      vram[30][28] = z+0x2;                                     // Set the middle left column row 2 to tile reference plus 2 offset.
      vram[30][29] = z+0x3;                                     // Set the middle left column row 2 to tile reference plus 3 offset.
      vram[30][30] = z+0x6;                                     // Set the middle left column row 2 to tile reference plus 6 offset.
      vram[30][31] = z+0x7;                                     // Set the middle left column row 2 to tile reference plus 7 offset.
      vram[31][28] = z+0x8;                                     // Set the middle right column row 3 to tile reference plus 8 offset.
      vram[31][29] = z+0x9;                                     // Set the middle right column row 3 to tile reference plus 9 offset.
      vram[31][30] = z+0xc;                                     // Set the middle right column row 3 to tile reference plus c offset.
      vram[31][31] = z+0xd;                                     // Set the middle right column row 3 to tile reference plus d offset.
      vram[0][28]  = z+0xa;                                     // Set the right column row 4 to tile reference plus a offset.
      vram[0][29]  = z+0xb;                                     // Set the right column row 4 to tile reference plus b offset.
      vram[0][30]  = z+0xe;                                     // Set the right column row 4 to tile reference plus e offset.
      vram[0][31]  = z+0xf;                                     // Set the right column row 4 to tile reference plus f offset.
    }
  }
}

void hide_player_missile() {
  missiles[7].ypos = 0;
  missiles[7].xpos = 0xff;
}

void does_player_shoot_formation() {
  byte mx = missiles[7].xpos;
  byte my = 255 - missiles[7].ypos; // missiles are Y-backwards
  signed char row = (my - FORMATION_Y0) / FORMATION_YSPACE;
  if (row >= 0 && row < ENEMY_ROWS) {
    // ok if unsigned (in fact, must be due to range)
    byte xoffset = mx - FORMATION_X0 - formation_offset_x;
    byte column = xoffset / FORMATION_XSPACE;
    byte localx = xoffset - column * FORMATION_XSPACE;
    if (column < ENEMIES_PER_ROW && localx < 16) {
      char index = column + row * ENEMIES_PER_ROW;
      if (formation[index].shape) {
        formation[index].shape = 0;
        enemies_left--;
        blowup_at(get_attacker_x(index), get_attacker_y(index));
        hide_player_missile();
        add_score(2);
      }
    }
  }
}

void does_player_shoot_attacker() {
  byte mx = missiles[7].xpos;
  byte my = 255 - missiles[7].ypos; // missiles are Y-backwards
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex && in_rect(mx, my, a->x >> 8, a->y >> 8, 16, 16)) {
      blowup_at(a->x >> 8, a->y >> 8);
      a->findex = 0;
      enemies_left--;
      hide_player_missile();
      add_score(5);
      break;
    }
  }
}

void does_missile_hit_player() {
  byte i;
  if (player_exploding)
    return;
  for (i=0; i<MAX_ATTACKERS; i++) {
    if (missiles[i].dy && 
        in_rect(missiles[i].xpos, 255-missiles[i].ypos, 
                player_x, player_y, 16, 16)) {
      player_exploding = 1;
      break;
    }
  }
}

void new_attack_wave() {
  byte i = rand();
  byte j;
  // find a random slot that has an enemy
  for (j=0; j<MAX_IN_FORMATION; j++) {
    i = (i+1) & (MAX_IN_FORMATION-1);
    // anyone there?
    if (formation[i].shape) {
      formation_to_attacker(i);
      formation_to_attacker(i+1);
      formation_to_attacker(i+ENEMIES_PER_ROW);
      formation_to_attacker(i+ENEMIES_PER_ROW+1);
      break;
    }
  }
}

void new_player_ship() {
  player_exploding = 0;
  draw_player();
  player_x = 112;
}

void set_sounds() {
  byte i;                                                             // Create an incrementer i for loops below.
  byte enable = 0;                                                    // Create an enable byte to store bit flags for channel activation.

  // Play missile fire sound.
  if (missiles[7].ypos) {                                             // If missle 7's y pos is 1 or greater...
    // set8910a(AY_PITCH_A_LO, missiles[7].ypos);                     // (old code)
    pt3play_ayregs.tone_a = missiles[7].ypos;                         // ...set tone channel A to missile 7 y pos...
    // set8910a(AY_ENV_VOL_A, 15-(missiles[7].ypos>>4));              // (old code)
    pt3play_ayregs.ampl_a = 15-(missiles[7].ypos>>4);                 // ...and set volume of channel A to y pos divided by 10.
    enable |= 0x1;                                                    // Set bit 1 to enable CH A.
  }

  // Play enemy explosion sound.
  if (enemy_exploding) {
    // set8910a(AY_PITCH_B_HI, enemy_exploding);
    pt3play_ayregs.tone_a = enemy_exploding;
    // set8910a(AY_ENV_VOL_B, 15);
    pt3play_ayregs.ampl_b = 15;
    enable |= 0x2;
  }

  // Play player explosion sound.
  if (player_exploding && player_exploding < 15) {                    // If player is exploding and it's less than 15...
    // set8910a(AY_ENV_VOL_C, 15-player_exploding);                   // (old code)
    pt3play_ayregs.ampl_c = 15 - player_exploding;                    // ...set the volume of channel C to 15 subtract the player exploding value.
    enable |= 0x4 << 3;                                               // Set bit 3 to enable CH C.
  }

  // set8910a(AY_ENABLE, ~enable);                                    // (old code)
  pt3play_ayregs.mixer = ~enable;                                     // Set mixer flags the bitwise inverse of enable to turn on and off Channels A, B, C

  // Play diving sounds for spaceships on second AY.
  enable = 0;                                                         // Reset our enable flag to zero.
  for (i=0; i<3; i++) {                                               // Loop three times to update up to three attacker dive sounds.
    byte y = attackers[i].y >> 8;                                     // Take the y position (word) of attacker i and divide it's value by 256 (???)
    if (y >= 0x80) {                                                  // If this new y value is greater than or equal to 128...
      // set8910b(AY_PITCH_A_LO+i, y);                                // (old code)
      pt3play_ayregs.tone_a 
      // set8910b(AY_ENV_VOL_A+i, 7);

      enable |= 1<<i;
    }
  }
  // set8910b(AY_ENABLE, ~enable);
}

void wait_for_frame() {
  while (((video_framecount^framecount)&3) == 0);
}

void play_round() {
  byte end_timer = 255;
  player_score = 0;
  add_score(0);
  putstring(0, 0, "PLAYER 1");
  setup_formation();
  formation_direction = 1;
  missile_width = 4;
  missile_offset = 0;
  reset_video_framecount();
  framecount = 0;
  new_player_ship();
  while (end_timer) {
    enable_irq = 0;
    enable_irq = 1;
    if (player_exploding) {                                   // If player exploding frame is 1 or greater...
      if ((framecount & 7) == 1) {                            // ...and if framecount is a multiple of 7 (???)...
        animate_player_explosion();                           // ...update the player explosion animation frame.
        if (++player_exploding > 32 && enemies_left) {        // If the player exploding frame is greater than 32 and there is at least one enemy left...
          new_player_ship();                                  // ...initiate a new player ship to be created.
        }
      }
    } else {
      if ((framecount & 0x7f) == 0 && enemies_left > 8) {     // If framecount is a multiple of 128 (???), and there are more than eight enemies...
        new_attack_wave();                                    // ...do an attack run.
      }
      move_player();
      does_missile_hit_player();
    }
    if ((framecount & 3) == 0) animate_enemy_explosion();
    move_attackers();
    move_missiles();
    does_player_shoot_formation();
    does_player_shoot_attacker();
    draw_next_row();
    draw_attackers();
    if ((framecount & 0xf) == 0) think_attackers();
    set_sounds();
    framecount++;
    watchdog++;
    if (!enemies_left) end_timer--;
    putchar(12,0,video_framecount&3);
    putchar(13,0,framecount&3);
    putchar(14,0,(video_framecount^framecount)&3);
    wait_for_frame();
  }
  enable_irq = 0;
}

/* 
// Original 8bitworkshop main loop
void main() {
  clrscr();                                         // Clears the screen
  clrobjs();                                        // Resets all the sprites, setting them to Y pos 64
  enable_stars = 0xff;                              // Turns on the starfield generator flag
  enable_irq = 0;                                   // Turns off the IRQ flag
  play_round();                                     // Plays a round of the game (see play_round function above)
  main();                                           // Loops back to main? Maybe this is how Galaxian hardware handles looping?
}

 */

// Tetris main loop
int main(void) {
    uint8_t iobank3_old = IO_BANK3;                 // Stores the current iobank3 value so that it can be reset at end of main loop.

    init();                                         // Perform initialization function (to be created for Galaxian).

    extern const unsigned char __21_2F_pt3[];       // Create a char array for the filename of the "21_2F.pt3" file in the assets folder of Tetris. Change for Galaxian.
    pt3play_init(__21_2F_pt3);                      // Initialize the pt3player to play the "21_2F.pt3" song file.

    while (!quit) {                                 // This is the LOOP in the main loop. Variable quit doesn't do anything (yet), so game plays until system reset.
        play_round();                               // Used to be play_marathon in Tetris; changed for Galaxian
    }

    IO_BANK3 = iobank3_old;                         // Restore original iobank3 value
    IO_VCTRL = VCTRL_TEXT_EN;                       // Set the IO_VCTRL (io registers for Video Control) to enable text mode.

    return 0;                                       // If this code were reachable (no funciton to read quit yet), it would exit the game.
}
