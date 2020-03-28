#include "music.h"
#include "sound.h"

// enable/disable channels
#define WAVE
#define PULSE
#define NOISE

UINT8 music_counter;

typedef struct {
    // 0x14 is notes[1][0x04]
    // note_e is 0x04
    // 0x10 & note_e is also possible
    // 0xFF is keep note
    UINT8 pulse_note;
    // 0x78 is volume 7 and 7th instrument
    // instrument F is NOP
    UINT8 pulse_vi;
    UINT8 wave_note;
    UINT8 wave_vi;
    // noise instruments have fixed notes
    UINT8 noise_vi;
} Pattern_frame;

typedef struct {
    UINT8 pulse_pattern;
    UINT8 wave_pattern;
    UINT8 noise_pattern;
} Song_frame;

#define pattern_size (7)
#define song_size (2)

// roughly based on Vegan On A Desert Island!? by CosmicGem
const Pattern_frame pattern[][pattern_size] = {
    {//5snare 4bass 3hihat
        {0x20 | note_d  , 0xD6, 0x20 | note_f   , 0xF2, 0xC4},
        {0x20 | note_fis, 0xD6, 0x00            , 0x02, 0xFF},
        {0x20 | note_a  , 0xD6, 0x20 | note_fis , 0xF2, 0xC3},
        {0xFF           , 0x06, 0x00            , 0x02, 0xFF},
        {0x20 | note_d  , 0xD7, 0x10 | note_fis , 0xF2, 0xC5},
        {0xFF           , 0xDF, 0xFF            , 0xFF, 0xFF},
        {0xFF           , 0xDF, 0xFF            , 0xFF, 0xC5}
    },
    {
        {0x20 | note_fis, 0xD6, 0x20 | note_f   , 0xF2, 0xFF},
        {0x20 | note_a  , 0xD6, 0x00            , 0x02, 0xFF},
        {0xFF           , 0x06, 0x20 | note_fis , 0xF2, 0xC3},
        {0x20 | note_a  , 0xD6, 0x00            , 0x02, 0xFF},
        {0x20 | note_d  , 0xD6, 0x20 | note_a   , 0xF2, 0xC5},
        {0x20 | note_d  , 0xD7, 0x20 | note_f   , 0xF2, 0xFF},
        {0xFF           , 0xDF, 0xFF            , 0xFF, 0xC5}
    }};

const Song_frame song[] = {{0, 0, 0}, {1, 1, 1}};

void init_music() { music_counter = 0; }

void tick_music() {
    UINT8 pttrn_frame = music_counter % pattern_size;
    UINT8 pttrn = song[music_counter / pattern_size].pulse_pattern;
    const Pattern_frame *pat = &(pattern[pttrn][pttrn_frame]);

#ifdef PULSE
    // instrument
    switch ((pat->pulse_vi & 0x0F)) {
    case 7:
        NR21_REG = 0x50;                          // 50% duty
        NR22_REG = 0x02 | (pat->pulse_vi & 0xF0); // volume envelope
        break;
    case 6:
        NR21_REG = 0x90;                          // 75% duty
        NR22_REG = 0x07 | (pat->pulse_vi & 0xF0); // volume envelope
        break;
    }

    if (pat->pulse_note != 0xFF) {
        NR24_REG = 0xC0 | note2int_hi(pat->pulse_note); // msb
        NR23_REG = note2int_lo(pat->pulse_note);
    }
#endif

#ifdef WAVE
    pttrn = song[music_counter / pattern_size].wave_pattern;
    pat = &(pattern[pttrn][pttrn_frame]);

    if ((pat->wave_vi & 0x0F) == 2) {
        NR30_REG = 0x0;  // off
        NR30_REG = 0x80; // on
        NR32_REG = 0x20; // max volume
        NR31_REG = 0xE0; // sound length
    } else {
        NR30_REG = 0x0; // off
    }

    if (pat->wave_note != 0xFF) {
        NR34_REG = 0xC0 | note2int_hi(pat->wave_note); // msb
        NR33_REG = note2int_lo(pat->wave_note);
    }
#endif

#ifdef NOISE
    pttrn = song[music_counter / pattern_size].noise_pattern;
    pat = &(pattern[pttrn][pttrn_frame]);
    // NR41 sound length
    NR44_REG = 0x80;
    NR42_REG = 0x07 | (pat->noise_vi & 0xF0);
    switch (pat->noise_vi & 0x0F) {
    case 0x03: // hihat
        // Env. Start: 10
        // Env. Down/Up: 0
        // Nev. Length: 2
        // Sound Size: 23
        NR41_REG = 23;
        // start | down/up | length
        NR42_REG = 0xA0 | 0x00 | 2;
        NR43_REG = 0x10 | 0x04 | 0x03;
        break;
    case 0x04: // bass
        NR41_REG = 28;
        NR42_REG = 0xF0 | 0x00 | 3;
        NR43_REG = 0x60 | 0x04 | 0x03;
        break;
    case 0x05: // snare
        NR41_REG = 30;
        NR42_REG = 0x80 | 0x00 | 5;
        NR43_REG = 0x20 | 0x04 | 0x03;
        break;
    }
#endif

    music_counter++;
    music_counter %= (pattern_size * song_size);
}
