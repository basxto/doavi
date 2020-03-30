// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "hud.h"

#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/music/the_journey_begins.c"

#include "pix/characters_data.c"
#include "pix/overworld_a_gbc_data.c"
#include "pix/overworld_anim_gbc_data.c"
#include "pix/overworld_b_gbc_data.c"
#include "pix/win_gbc_data.c"

#include "pix/characters_map.c"
#include "pix/overworld_a_gbc_map.c"
#include "pix/overworld_anim_gbc_map.c"
#include "pix/overworld_b_gbc_map.c"

#include "pix/overworld_a_gbc_pal.c"
#include "pix/overworld_b_gbc_pal.c"
#define bkgPalette overworld_b_gbc_pal

#include "strings.c"

// all maps are 10 tiles (16x16) wide and 9 tiles high
#define HEIGHT (8)
#define WIDTH (10)
// tile (8x8) width of our sprite
#define SPRITEWIDTH (16)
#define TRANSPARENT (RGB(12, 25, 0))

#define CHARACTERS_START (0)
#define SHEET_START (128)
// width in 16x16 blocks
#define SHEET_WIDTH (8)
#define ANIM_WIDTH (4)

#include "level.c"

void load_map(const unsigned int background[], const unsigned int sprites[]);

UINT8 used_sprites;
UINT8 counter;
UINT8 anim_counter;

Level *current_level;
const unsigned char *current_map;

typedef struct {
    UINT8 x; // position
    UINT8 y;
    UINT8 direction;
    UINT8 palette;
    UINT8 sprite; // sprite character section
    // maybe completely remove this and do this in animation tick
    UINT8 sprite_index;
} Character;

typedef struct {
    char magic;
    UINT8 level_x;
    UINT8 level_y;
    UINT8 lives;
    UINT8 tpaper;
    // each bit for one
    UINT8 collectable;
    Character player;
} Savegame;
// Savegame noram;
Savegame *sg;

void change_level() {
    current_level = &level[sg->level_y][sg->level_x];
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

UINT8 move_character(Character *chrctr, const INT8 x, const INT8 y,
                     const unsigned int *collision) {
    if (chrctr->x == 0 && x < 0) {
        sg->level_x--;
        chrctr->x += WIDTH + x;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    if (chrctr->x == WIDTH - 1 && x > 0) {
        sg->level_x++;
        chrctr->x += -WIDTH + x;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    if (chrctr->y == 0 && y < 0) {
        sg->level_y--;
        chrctr->y += HEIGHT + y;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    if (chrctr->y == HEIGHT - 1 && y > 0) {
        sg->level_y++;
        chrctr->y += -HEIGHT + y;
        wait_vbl_done();
        render_character(&(sg->player));
        change_level();
        return 0;
    }
    UINT8 index = (chrctr->y + y) * WIDTH + (chrctr->x + x);
    if ((collision[index / 8] & (1 << (index % 8))) == 0) {
        chrctr->x += x;
        chrctr->y += y;
        render_character(&(sg->player));
        return 0;
    } else {
        return 1;
    }
}

void incject_map(UINT8 x, UINT8 y, UINT16 index) {
    unsigned char tiles[4];
    index *= 4;
    tiles[0] = SHEET_START + current_map[index];
    tiles[1] = SHEET_START + current_map[index + 2];
    tiles[2] = SHEET_START + current_map[index + 1];
    tiles[3] = SHEET_START + current_map[index + 3];
    set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);
}

void load_map(const unsigned int background[], const unsigned int sprites[]) {
    UINT8 y;
    UINT8 x;
    UINT16 index;
    UINT8 i;
    // tmx
    UINT16 tile;
    // loaded spritesheet
    UINT8 palette;
    unsigned char tiles[4];

    DISPLAY_OFF;
    // load spritesheet
    if (sg->level_y > 1) {
        current_map = overworld_b_gbc_map;
        set_bkg_data(SHEET_START, sizeof(overworld_b_gbc_data) / 16,
                     overworld_b_gbc_data);
        set_bkg_palette(0, 6, overworld_b_gbc_pal[0]);
        set_sprite_palette(0, 6, overworld_b_gbc_pal[0]);
        BGP_REG = 0xE4; // 11100100
    } else {
        current_map = overworld_a_gbc_map;
        set_bkg_data(SHEET_START, sizeof(overworld_a_gbc_data) / 16,
                     overworld_a_gbc_data);
        set_bkg_palette(0, 6, overworld_a_gbc_pal[0]);
        set_sprite_palette(0, 6, overworld_a_gbc_pal[0]);
        BGP_REG = 0xE1;
    }

    // reset all sprites used_sprites
    for (i = 0; i < used_sprites; ++i)
        move_sprite(used_sprites, 0, 0);

    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            // load background
            tile = background[(y * WIDTH) + x] - 2;
            index = tile * 4;
            // set color (GBC only)
            VBK_REG = 1;
            // each row has own palette
            palette = tile / (SPRITEWIDTH / 2);
            if (current_map == overworld_a_gbc_map && palette == 3 &&
                (tile % (SPRITEWIDTH / 2)) >= 4) {
                // last row has two palletes
                palette = 4;
            }
            tiles[0] = tiles[1] = tiles[2] = tiles[3] = palette;
            set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);
            VBK_REG = 0;
            // set tiles
            tiles[0] = SHEET_START + current_map[index];
            tiles[1] = SHEET_START + current_map[index + 2];
            tiles[2] = SHEET_START + current_map[index + 1];
            tiles[3] = SHEET_START + current_map[index + 3];
            set_bkg_tiles(x * 2, y * 2, 2, 2, tiles);

            // load sprites
            tile = sprites[(y * WIDTH) + x];
            if (tile != 0) {
                tile -= 2;
                palette = tile / (SPRITEWIDTH / 2);
                index = tile * 4;
                set_sprite_tile(used_sprites, SHEET_START + current_map[index]);
                move_sprite(used_sprites, 8 + x * 16, 16 + y * 16);
                set_sprite_prop(used_sprites, palette);
                set_sprite_tile(used_sprites + 1,
                                SHEET_START + current_map[index + 2]);
                move_sprite(used_sprites + 1, 8 + x * 16 + 8, 16 + y * 16);
                set_sprite_prop(used_sprites + 1, palette);
                used_sprites += 2;
            }
        }
    }

    // map scripting
    if (!(sg->collectable & 0x1) && sg->level_x == 2 && sg->level_y == 1) {
        incject_map(7, 3, 29);
    }
    DISPLAY_ON;
}

void interact() {
    UINT8 x = sg->player.x;
    UINT8 y = sg->player.y;
    UINT8 tile;
    switch (sg->player.direction) {
    case 0:
        y++;
        break;
    case 1:
        y--;
        break;
    case 2:
        x--;
        break;
    case 3:
        x++;
        break;
    }
    tile = current_level->background[(y * WIDTH) + x];
    write_num(8, 1, 3, tile);
    if (tile == 18) {
        if (sg->level_x == 0 && sg->level_y == 0) {
            dialog(sizeof(text_whatsup) - 1, text_whatsup,
                   sizeof(text_sign) - 1, text_sign, 1);
        } else {
            dialog(sizeof(text_hellowor) - 1, text_hellowor,
                   sizeof(text_sign) - 1, text_sign, 1);
        }
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 26) {
        dialog(sizeof(text_somebody) - 1, text_somebody, sizeof(text_grave) - 1,
               text_grave, 2);
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 30) {
        dialog(sizeof(text_burnever) - 1, text_burnever, sizeof(text_flame) - 1,
               text_flame, 3);
        draw_hud(sg->lives, sg->tpaper);
        reset();
    }
    if (tile == 32) {
        if (!(sg->collectable & 0x1) && sg->level_x == 2 && sg->level_y == 1) {
            incject_map(7, 3, 30);
            sg->collectable |= 0x1;
            sg->tpaper++;
            draw_hud(sg->lives, sg->tpaper);
        }
    }
}

// index of tile in spritesheet; index of tile in animation sheet
// 16x16 block indices
#define replace_tile(index, indexa, counter)                                   \
    (set_bkg_data(                                                             \
        SHEET_START + current_map[(index)*4], 4,                               \
        &overworld_anim_gbc_data                                               \
            [overworld_anim_gbc_map[((indexa)*ANIM_WIDTH + (counter)) * 4] *   \
             16]))

// for compressed tiles
#define replace_subtile(index, indexa, counter, offset)                        \
    (set_bkg_data(                                                             \
        SHEET_START + current_map[(index)*4 + offset], 1,                      \
        &overworld_anim_gbc_data                                               \
            [overworld_anim_gbc_map[((indexa)*ANIM_WIDTH + (counter)) * 4 +    \
                                    offset] *                                  \
             16]))

inline void tick_animate() {
    if (current_map == overworld_a_gbc_map) {
        replace_tile(1, 0, anim_counter);
        replace_tile(2, 1, anim_counter);
        replace_tile(SHEET_WIDTH * 3 + 4, 2, anim_counter);
    }
    if (current_map == overworld_b_gbc_map) {
        replace_tile(SHEET_WIDTH * 3 + 7, 3, anim_counter);
        replace_tile(SHEET_WIDTH * 3 + 3, 4, anim_counter);
        // shore waves
        replace_subtile(SHEET_WIDTH * 3 + 4, 5, anim_counter, 0);
        replace_subtile(SHEET_WIDTH * 3 + 4, 5, anim_counter, 2);
        replace_subtile(SHEET_WIDTH * 3 + 5, 6, anim_counter, 0);
        replace_subtile(SHEET_WIDTH * 3 + 5, 6, anim_counter, 1);
        replace_subtile(SHEET_WIDTH * 3 + 6, 7, anim_counter, 2);
        replace_subtile(SHEET_WIDTH * 3 + 6, 7, anim_counter, 3);
    }

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
    sg = (Savegame *)0xa000;
    // load savegame
    ENABLE_RAM_MBC1;
    if (sg->magic != 'V') {
        sg->level_x = 1;
        sg->level_y = 0;

        sg->player.x = 2;
        sg->player.y = 3;
        sg->player.sprite = 1;
        sg->player.direction = 0;
        sg->player.palette = 4;
        sg->player.sprite_index = 38;

        sg->lives = 5;
        sg->tpaper = 0;

        sg->collectable = 0;

        sg->magic = 'V';
    }
    current_level = &level[sg->level_y][sg->level_x];
    HIDE_BKG;
    HIDE_WIN;
    HIDE_SPRITES;
    DISPLAY_OFF;
    SPRITES_8x16;
    used_sprites = 0;
    counter = 0;
    anim_counter = 0;

    BGP_REG = 0xE1; // 11100001
    OBP0_REG = 0xE1;

    init_hud();
    init_music(&the_journey_begins);

    render_character(&(sg->player));

    cgb_compatibility();
    set_bkg_palette(0, 6, bkgPalette[0]);
    set_sprite_palette(0, 6, bkgPalette[0]);

    // load tilesets
    set_win_data(WIN_START, sizeof(win_gbc_data) / 16, win_gbc_data);
    set_sprite_data(CHARACTERS_START, sizeof(characters_data) / 16,
                    characters_data);
    load_map(current_level->background, current_level->sprites);

    // init_hud();
    draw_hud(sg->lives, sg->tpaper);

    // render_character(&(sg->player));

    SHOW_BKG;
    SHOW_WIN;
    SHOW_SPRITES;
    DISPLAY_ON;
    // reset();

    // set_sprite_tile(2, SHEET_START + current_map[20]);

    // configure interrupt
    TIMA_REG = TMA_REG = 0x1A;
    TAC_REG = 0x4 | 0x0; // 4096 Hz
    // enable timer interrupt
    disable_interrupts();
    add_TIM(timer_isr);
    enable_interrupts();
    // gbdk needs VBL iflag
    set_interrupts(VBL_IFLAG | TIM_IFLAG);

    while (1) {

        switch (joypad()) {
        case J_RIGHT: // If joypad() is equal to RIGHT
            sg->player.direction = 3;
            if (move_character(&(sg->player), 1, 0, current_level->collision) ==
                1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_LEFT: // If joypad() is equal to LEFT
            sg->player.direction = 2;
            if (move_character(&(sg->player), -1, 0,
                               current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_UP: // If joypad() is equal to UP
            sg->player.direction = 1;
            if (move_character(&(sg->player), 0, -1,
                               current_level->collision) == 1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_DOWN: // If joypad() is equal to DOWN
            sg->player.direction = 0;
            if (move_character(&(sg->player), 0, 1, current_level->collision) ==
                1)
                render_character(&(sg->player));
            delay(100);
            break;
        case J_A: // If joypad() is equal to DOWN
            interact();
            delay(100);
            break;
        default:
            break;
        }
    }
}
