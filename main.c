// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "hud.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/music/the_journey_begins.c"

#include "pix/characters_data.c"
#include "pix/characters_map.c"
#include "pix/overworld_anim_gbc_data.c"
#include "pix/overworld_anim_gbc_map.c"
#include "pix/overworld_gbc_data.c"
#include "pix/overworld_gbc_map.c"
#include "pix/win_gbc_data.c"

#include "pix/overworld_gbc_pal.c"
#define bkgPalette overworld_gbc_pal

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HEIGHT (8)
#define WIDTH (10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH (34)
#define TRANSPARENT (RGB(12, 25, 0))

#define SHEET_START (97)
// width in 16x16 blocks
#define SHEET_WIDTH (17)
#define ANIM_START (233)
#define ANIM_WIDTH (4)
#define CHARACTERS_START (267)

#include "level.c"

void load_map(const unsigned int background[], const unsigned int sprites[]);

UINT8 used_sprites;
UINT8 counter;
UINT8 anim_counter;

UINT8 level_x;
UINT8 level_y;
Level *current_level;

typedef struct {
    UINT8 x; // position
    UINT8 y;
    UINT8 direction;
    UINT8 palette;
    UINT8 sprite; // sprite character section
    // maybe completely remove this and do this in animation tick
    UINT8 sprite_index;
} Character;

Character player;

void change_level(){
    current_level = &level[level_y][level_x];
    load_map(current_level->background, current_level->sprites);
}

// character spritesheet must be 4 16x16 blocks wide ... always
void render_character(const Character *chrctr) {
    UINT8 base = chrctr->sprite * 4 * 4 + chrctr->direction * 4;
    set_sprite_tile(chrctr->sprite_index,
                    CHARACTERS_START + characters_map[base]);
    move_sprite(chrctr->sprite_index, 8 + (chrctr->x) * 16,
                16 + (chrctr->y) * 16);
    set_sprite_prop(chrctr->sprite_index, chrctr->palette);
    set_sprite_tile(chrctr->sprite_index + 1,
                    CHARACTERS_START + characters_map[base + 2]);
    move_sprite(chrctr->sprite_index + 1, 8 + (chrctr->x) * 16 + 8,
                16 + (chrctr->y) * 16);
    set_sprite_prop(chrctr->sprite_index + 1, chrctr->palette);
}

void move_character(Character *chrctr, const INT8 x, const INT8 y,
                    const unsigned int *collision) {
    if(chrctr->x == 0 && x < 0){
        level_x--;
        change_level();
        chrctr->x += WIDTH;
    }
    if(chrctr->x >= WIDTH-1 && x > 0){
        level_x++;
        change_level();
        chrctr->x -= WIDTH;
    }
    if(chrctr->y == 0 && y < 0){
        level_y--;
        change_level();
        chrctr->y += HEIGHT;
    }
    if(chrctr->y >= HEIGHT-1 && y > 0){
        level_y++;
        change_level();
        chrctr->y -= HEIGHT;
    }
    UINT8 index = (chrctr->y + y) * WIDTH + (chrctr->x + x);
    if ((collision[index / 8] & (1 << (index % 8))) == 0) {
        chrctr->x += x;
        chrctr->y += y;
        render_character(chrctr);
    }
}

void load_map(const unsigned int background[], const unsigned int sprites[]) {
    UINT8 y;
    UINT8 x;
    int index;
    UINT8 i;
    // tmx
    UINT16 tile;
    // loaded spritesheet
    UINT8 palette;
    unsigned char tiles[4];

    // reset all sprites used_sprites
    for (i = 0; i < used_sprites; ++i)
        move_sprite(used_sprites, 0, 0);

    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            // load background
            tile = background[(y * WIDTH) + x] - 2;
            palette = tile / (SPRITEWIDTH / 2);
            index = tile * 4;
            // set color (GBC only)
            VBK_REG = 1;
            // each row has own palette
            tiles[0] = tiles[1] = tiles[2] = tiles[3] = palette;
            set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);
            VBK_REG = 0;
            // set tiles
            tiles[0] = SHEET_START + overworld_gbc_map[index];
            tiles[1] = SHEET_START + overworld_gbc_map[index + 2];
            tiles[2] = SHEET_START + overworld_gbc_map[index + 1];
            tiles[3] = SHEET_START + overworld_gbc_map[index + 3];
            set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);

            // load sprites
            tile = sprites[(y * WIDTH) + x];
            if (tile != 0) {
                tile -= 2;
                palette = tile / (SPRITEWIDTH / 2);
                index = tile * 4;
                set_sprite_tile(used_sprites,
                                SHEET_START + overworld_gbc_map[index]);
                move_sprite(used_sprites, 8 + x * 16, 16 + y * 16);
                set_sprite_prop(used_sprites, palette);
                set_sprite_tile(used_sprites + 1,
                                SHEET_START + overworld_gbc_map[index + 2]);
                move_sprite(used_sprites + 1, 8 + x * 16 + 8, 16 + y * 16);
                set_sprite_prop(used_sprites + 1, palette);
                used_sprites += 2;
            }
        }
    }
}

// index of tile in spritesheet; index of tile in animation sheet
// 16x16 block indices
/* void replace_tile(const UINT8 index, const UINT8 indexa){
        set_bkg_data(SHEET_START + overworld_gbc_map[index *
4],4,&overworld_anim_gbc_data[overworld_anim_gbc_map[indexa * 4]*16]);
} */

#define replace_tile(index, indexa)                                            \
    (set_bkg_data(                                                             \
        SHEET_START + overworld_gbc_map[(index)*4], 4,                         \
        &overworld_anim_gbc_data[overworld_anim_gbc_map[(indexa)*4] * 16]))

inline void tick_animate() {
    // ANIM_START
    replace_tile(1, anim_counter);

    replace_tile(2, anim_counter + ANIM_WIDTH);
    replace_tile(SHEET_WIDTH * 4, anim_counter + 2 * ANIM_WIDTH);

    anim_counter = (anim_counter + 1) % ANIM_WIDTH;
}

void timer_isr() {
    tick_music();
    if (counter % 8 == 0) {
        tick_animate();
    }
    counter++;
    counter %= 20;
}

void main() {
    level_x = 1;
    level_y = 0;
    current_level = &level[level_y][level_x];
    HIDE_BKG;
    HIDE_WIN;
    HIDE_SPRITES;
    SPRITES_8x16;
    used_sprites = 0;
    counter = 0;
    anim_counter = 0;

    BGP_REG = 0xE1; // 11100001
    OBP0_REG = 0xE1;

    init_hud();
    init_music(&the_journey_begins);

    player.x = 2;
    player.y = 3;
    player.sprite = 1;
    player.direction = 1;
    player.palette = 4;
    player.sprite_index = 38;

    render_character(&player);
    move_character(&player, 1, 0, current_level->collision);
    move_character(&player, 1, 0, current_level->collision);

    cgb_compatibility();
    set_bkg_palette(0, 5, bkgPalette[0]);
    set_sprite_palette(0, 5, bkgPalette[0]);

    // load tileset
    set_bkg_data(SHEET_START, 136, overworld_gbc_data);
    set_sprite_data(SHEET_START, 136, overworld_gbc_data);
    set_win_data(WIN_START, 96, win_gbc_data);
    set_sprite_data(CHARACTERS_START, 29, characters_data);
    load_map(current_level->background, current_level->sprites);

    // init_hud();
    draw_hud(2, 42);

    // render_character(&player);

    SHOW_BKG;
    SHOW_WIN;
    SHOW_SPRITES;
    DISPLAY_ON;
    // reset();

    // set_sprite_tile(2, SHEET_START + overworld_gbc_map[20]);

    // configure interrupt
    TIMA_REG = TMA_REG = 0x1A;
    TAC_REG = 0x4 | 0x0; // 4096 Hz
    // enable timer interrupt
    disable_interrupts();
    add_TIM(timer_isr);
    enable_interrupts();
    set_interrupts(TIM_IFLAG);

    while (1) {

        switch (joypad()) {
        case J_RIGHT: // If joypad() is equal to RIGHT
            move_character(&player, 1, 0, current_level->collision);
            delay(100);
            break;
        case J_LEFT: // If joypad() is equal to LEFT
            move_character(&player, -1, 0, current_level->collision);
            delay(100);
            break;
        case J_UP: // If joypad() is equal to UP
            move_character(&player, 0, -1, current_level->collision);
            delay(100);
            break;
        case J_DOWN: // If joypad() is equal to DOWN
            move_character(&player, 0, 1, current_level->collision);
            delay(100);
            break;
        default:
            break;
        }
    }
}
