#include "../dev/gbdk-music/sound.h"
#include "../dev/gbdk-music/music.h"

// Vegan On A Desert Island!?
// https://voadi.com
// by CosmicGem released under some kind of creative commons license?

const Pattern_frame cosmicgem_voadi_pattern[][16] = {

    //  |-------PULSE-2-------|
    //  |------NOTE-----| |VI-|
    {
        {note(3,note_f)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_fis), 0x81},
        {note(2,note_fis), 0x81},
        {0xFF            , 0x8F},
        {note(3,note_f)  , 0x81},
        {note(3,note_fis), 0x81},
        {note(3,note_a)  , 0x81},
        {note(3,note_f)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_fis), 0x81},
        {note(2,note_fis), 0x81},
        {0xFF            , 0x8F},
        {note(3,note_f)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_f)  , 0x81},
    },
    //  |------- WAVE---------|
    {
        {note(0,note_d)  , w7|0x3},
        {0xFF            , w7|0x3},
        {note(0,note_a)  , w7|0x3},
        {note(0,note_d)  , w7|0x3},
        {0xFF            , w0|0x3},
        {note(0,note_fis), w7|0x3},
        {0xFF            , w0|0x3},
        {note(0,note_a)  , w7|0x3},
        {note(0,note_d)  , w7|0x3},
        {0xFF            , w7|0x3},
        {note(0,note_a)  , w7|0x3},
        {note(0,note_d)  , w7|0x3},
        {0xFF            , w0|0x3},
        {0xFF            , w7|0x3},
        {0xFF            , w7|0x3},
        {note(0,note_d)  , w7|0x3}
    },
    //  |-------NOISE---------|
    {//86 A4
        {0xFF            ,0xA4},
        {0xFF            ,0xFF},
        {0xFF            ,0x86},
        {0xFF            ,0xA4},
        {0xFF            ,0x84},
        {0xFF            ,0xFF},
        {0xFF            ,0x86},
        {0xFF            ,0xA4},
        {0xFF            ,0x84},
        {0xFF            ,0xFF},
        {0xFF            ,0x86},
        {0xFF            ,0xA4},
        {0xFF            ,0x84},
        {0xFF            ,0xFF},
        {0xFF            ,0xFF},
        {0xFF            ,0xFF}
    },
    //  |-------PULSE-1-------|
    {
        {note(3,note_d)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_d)  , 0x81},
        {note(2,note_a)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_d)  , 0x81},
        {note(3,note_d)  , 0x81},
        {note(3,note_fis), 0x81},
        {note(3,note_d)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_d)  , 0x81},
        {note(2,note_a)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_b)  , 0x81},
        {0xFF            , 0x8F},
        {note(3,note_e)  , 0x81},
    }
};

const Song_frame cosmicgem_voadi_arrangement[] = {
//   P1  P2  W   N
    { 3,  0,  1,  2}
};

const Instrument cosmicgem_voadi_instruments[] = {
    //NR2     NR1   NR4   NR0/NR3
    // pulse
    {0x00   , 0x90, 0x80, 0x00},
    {0x02   , 0x50, 0xC0, 0x00},
    {0x07   , 0x90, 0xC0, 0x00},
    // wave
    {0x00   , 0xF0, 0x80, 0x80},
    // noise
    {0x00 |2,   23, 0xC0, 0x10 | 0x04 | 0x03},
    {0x00 |3,   28, 0xC0, 0x60 | 0x04 | 0x03},
    {0x00 |5,   30, 0xC0, 0x20 | 0x04 | 0x03}
};

Song cosmicgem_voadi={16,1,5,
    cosmicgem_voadi_pattern[0],
    cosmicgem_voadi_arrangement,
    cosmicgem_voadi_instruments
};