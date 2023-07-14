#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "regs.h"
#include "file_io.h"

unsigned long ticks = 0;

struct blinker {
    bool lightOn;               // Blink status; start ON
    uint8_t onTime;             // Ticks to stay on
    uint8_t offTime;            // Ticks to stay off
    uint8_t onChr;              // Char when on
    uint8_t offChr;             // Char when off
    uint8_t onCol;              // Color setting when on
    uint8_t offCol;             // Color setting when off
    uint8_t *chrLoc;            // Location of character
    uint8_t *colLoc;            // Location of color toggle
    unsigned long startCycle;   // Last ticks start of cycle
};

struct blinker towerLight;

struct blinker moveJoyToStart[25];

const char joyString[] = "Move Control Pad to Start";

static uint8_t get_joystick(void) {
    IO_PSG1ADDR = 14;
    return IO_PSG1DATA;
}

void updateBlinker(struct blinker this) {
    if (((this.startCycle + this.onTime) > ticks) && this.lightOn) {
        *this.chrLoc = (uint8_t) this.offChr;
        *this.colLoc = (uint8_t) this.offCol;
        this.lightOn = false;
    } else if (((this.startCycle + this.onTime + this.offTime) > ticks) && !this.lightOn) {
        *this.chrLoc = (uint8_t) this.onChr;
        *this.colLoc = (uint8_t) this.onCol;
        this.lightOn = true;
    } else {
        this.startCycle = ticks;
    }
}

static void clear_screen() {
    uint8_t *tram = (uint8_t *)0x3000;
    uint8_t *cram = (uint8_t *)0x3400;

    // Clear screen
    for (uint16_t i = 0; i < 1000; i++) {
        tram[i] = ' ';
        cram[i] = 0x06;
    }
    printf("ticks = %u\n", ticks);
}

static void update_screen() {
    updateBlinker(towerLight);
    for (uint8_t i = 0; i < 25; i++) {
        updateBlinker(moveJoyToStart[i]);
    }
}

unsigned long the_ticks() {
    return ticks;
}

static inline void wait_vsync(void) {
    ticks++;
    IO_VIRQLINE = 216;
    IO_IRQSTAT  = 2;
    while ((IO_IRQSTAT & 2) == 0) {
    }
}

bool init(void) {
    // Load in CHR and COL data
    int8_t fd = open("pt_ss.scr", FO_RDONLY);
    if (fd < 0) {
        return false;
    }
    read(fd, (void *)0x3000, 0x3800);
    close(fd);

    // Init Tower blinker
    towerLight.lightOn    = 1;                          // Initial blink status
    towerLight.onTime     = 20;                         // Ticks
    towerLight.offTime    = 10;                         // Ticks
    towerLight.onChr      = 34;                         // Char when on
    towerLight.offChr     = 34;                         // Char when off
    towerLight.onCol      = 31;                         // Color setting when on
    towerLight.offCol     = 255;                        // Color setting when off
    *towerLight.chrLoc    = (uint16_t *) 12641;                      // Location of character
    *towerLight.colLoc    = (uint16_t *) 13665;                      // Location of color toggle
    towerLight.startCycle = ticks;                      // Start the blink cycle

    // Init Press Control Pad blinker
    for (uint8_t i=0; i<25; i++) {
        moveJoyToStart[i].lightOn    = 1;
        moveJoyToStart[i].onTime     = 90;
        moveJoyToStart[i].offTime    = 70;
        moveJoyToStart[i].onChr      = joyString[i];
        moveJoyToStart[i].offChr     = (uint8_t) " ";
        moveJoyToStart[i].onCol      = 118;
        moveJoyToStart[i].offCol     = 102;
        *moveJoyToStart[i].chrLoc    = 12694 + i;
        *moveJoyToStart[i].colLoc    = 13718 + i;
        moveJoyToStart[i].lightOn    = ticks;
    }

    return true;
}

int main(void) {
    if (!init()) {
        return 1;
    }

    // Main Loop
    while (1) {
        wait_vsync();
        update_screen();
        uint8_t joyval = ~get_joystick();

        // Control Pad DOWN
        if (joyval & (1 << 0)) {
            clear_screen();
            return 0;
        }

        // Control Pad RIGHT
        if (joyval & (1 << 1)) {
            //clear_screen();
            //return 0;
        }

        // Control Pad UP
        if (joyval & (1 << 2)) {
            //updateBlinker(towerLight);
            //clear_screen();
            //return 0;
        }

        // Control Pad LEFT
        /*
        if (joyval & (1 << 3)) {
            clear_screen();
            return 0;
        }
        */
    }
}
