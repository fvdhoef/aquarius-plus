#include <aqplus.h>

extern const unsigned char ingarden_pt3[];

int main(void) {
    printf("Playing PT3...");

    // Initialize PT3 library
    pt3play_init(ingarden_pt3);

    while (1) {
        // Wait for end-of-frame (line 216)
        video_wait_eof();

        // Play 1 frame of PT3 data. Break out of loop if end-of-song.
        if (pt3play_play())
            break;
    }

    // Mute any remaining sound
    pt3play_mute();

    printf("done.\n");
    return 0;
}
