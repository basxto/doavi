#include "map.h"
#include "main.h"
#include "hud.h"
#include "level.h"
#include "../dev/gbdk-music/music.h"
#include "../dev/gbdk-music/sound.h"
#include "utils.h"

#include "../build/strings.h"

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];
extern const unsigned char inside_wood_house_map[];

// defined in main.c
extern uint8_t *current_background;
extern const unsigned char *current_map;

void teleport_to(const int8_t lx, const int8_t ly, const int8_t px, const int8_t py) {
    level_x = lx;
    level_y = ly;
    character[0].x = px;
    character[0].y = py;
    change_level();
}

uint8_t move_player(const int8_t x, const int8_t y) {
    uint8_t tile =
        current_background[U8(U8(character[0].y * WIDTH) + character[0].x)];

    if (move_character(0, x, y) == 1) {
        // little cheat
        // jump from tree
        if(tile == 12 || tile == 13){
            if(move_character(0, x*2, y*2) == 0){
                return 0;
            }
        }
        // otherwise boing
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
    }

    // leaving the beach if bottle is not collected
    if((progress[0] & PRGRS_BTL) == 0 && level_y == 4 && character[0].y == 0){
        dialog(text_stay_beach, text_narrator, 0);
        character[0].direction = 0;
        character[0].y++;
        return 1;
    }
    tile =
        current_background[(character[0].y * WIDTH) + character[0].x];

    // trigger stuff

    // cave entrance
    if(tile == 21 && current_map == overworld_a_gbc_map){
        if(level_x == 4){
            character[0].direction = 0;
            teleport_to(0, 6, 2, 1);
        }else{
            character[0].direction = 0;
            teleport_to(1, 6, 7, 1);
        }
    }

    //  house entrance
    if (tile == U8(46 + 10)) {
        if(level_y == 1 && level_x == 1)
            if(character[0].x > U8(5))
                teleport_to(0, 5, 5, 6);
            else
                teleport_to(1, 5, 4, 6);
        else if(level_y == 1 && level_x == 0)
            teleport_to(2, 5, 6, 6);
        else if(level_y == 2 && level_x == 0)
            teleport_to(3, 5, 3, 6);
        else // glitch house
            teleport_to(4, 5, 4, 6);
    }

    // player stepped into the doorway
    if(level_y == 5 && character[0].y == 7){
        if (level_x == 0) {
            teleport_to(1, 1, 7, 5);
        }
        else if (level_x == 1) {
            teleport_to(1, 1, 2, 6);
        }
        else if (level_x == 2) {
            teleport_to(0, 1, 4, 6);
        }
        else if (level_x == 3) {
            teleport_to(0, 2, 5, 6);
        }
        else if (level_x == 4) {
            teleport_to(4, 1, 4, 6);
        }
    }
    // player goes through the back door
    if(level_y == 5 && character[0].y == 2){
        teleport_to(1, 0, 7, 6);
    }

    // player goes back into the house
    if(level_x == 1 && level_y == 0 && character[0].y == 7){
        teleport_to(0, 5, 7, 3);
    }

    // player stepped onto the stairs
    if (level_y == 6 && character[0].y == 0) {
        if(level_x == 0)
            teleport_to(4, 2, 5, 3);
        else
            teleport_to(5, 2, 5, 3);
    }

    return 0;
}


void interact() {
    uint8_t x = character[0].x;
    uint8_t y = character[0].y;
    uint8_t tile;
    switch (character[0].direction) {
    case 0:
        y++;
        break;
    case 2:
        y--;
        break;
    case 1:
        x--;
        break;
    case 3:
        x++;
        break;
    }
    tile = current_background[U8(y * WIDTH + x)];
    // write_num(8, 1, 3, tile);
    // sign
    if (tile == 18) {
        if (level_x == 1 && level_y == 1) {
            dialog(text_village, text_sign, 0);
        } else {
            dialog(text_hello_world, text_sign, 0);
        }
    }
    // bottle
    if (tile == 34 && current_map == overworld_b_gbc_map) {
        if(progress[0] & PRGRS_BTL){
            dialog(text_empty_bottle, text_narrator, 0);
        }else{
            dialog(text_bottle_post, text_narrator, 0);
            dialog(text_shekiro_1, text_letter, 0);
            progress[0] |= PRGRS_BTL;
        }
    }
    if(current_map == inside_wood_house_map) {
        // cabinet
        if (tile == 11) {
            dialog(text_empty_cabinet, text_narrator, 0);
        }
        // cupboard
        if (tile == 12 || tile == 13) {
            dialog(text_empty_cupboard, text_narrator, 0);
        }
        // chair
        if (tile == 24) {
            dialog(text_sit, text_narrator, 0);
        }
        // plant
        if (tile == 16) {
            dialog(text_plant, text_narrator, 0);
        }
        // barrel
        if (tile == 8) {
            dialog(text_na_barrel, text_narrator, 0);
        }
    }
    if(current_map == overworld_a_gbc_map) {
        // grave
        if (tile == 26) {
            if(x == 5 && y == 2 && (progress[0] & PRGRS_GHOST) == 0){
                screen_shake();
                // spawn ghost
                character[1].x = 4;
                character[1].y = 2;
                if(character[0].x == 4 && character[0].y == 2){
                    character[1].x = 6;
                }
                character[1].sprite = 2;
                character[1].direction = 7<<2;//ghost bottom
                character[1].palette = 3<<4 | 3;

                render_character(1);
                // ghost visible
                SET_PRGRS_GHOST(0x1);
                //progress[0] & (0x1<<4));
            }
            dialog(text_dead, text_grave, 0);
        }
        if (tile == 30 && current_map == overworld_a_gbc_map) {
            dialog(text_dialog4, text_flame, 0);
            // reset();
        }
    }
    if (current_map == overworld_a_gbc_map || current_map == overworld_b_gbc_map) {
        // chest
        if (tile == 31 || tile == 20) {
            // we don't have to check the status
            // since chests would be a different tile  otherwise
            chest |= current_chest;
            switch(current_chest){
                case (1<<(1-1)):
                item[1] = ITEM_POWER;
                dialog(text_found_power, text_narrator, 0);
                break;
                case (1<<(2-1)):
                item[0] = ITEM_SWORD;
                dialog(text_found_sword, text_narrator, 0);
                break;
                case (1<<(3-1)):
                item[2] = ITEM_FLINT;
                dialog(text_found_flint, text_narrator, 0);
                break;
                default:
                ++tpaper;
            }
            incject_map(x, y, tile-2);
            blinger(0x05 | note_a, 4, 0x05 | note_b, 5, 0x04 | note_e);
        }
        // enflame
        if (tile == 32 && get_selected_item() == ITEM_FLINT) {
            flame |= current_flame;
            incject_map(x, y, tile-1);
        }
        // cut grass
        if (tile == 16) {
            if(get_selected_item() == ITEM_SWORD){
                incject_map(x, y, 17-2);
                incject_collision(x, y, FALSE);
                current_background[(y * WIDTH) + x] = current_map[17-2];
            } else
                dialog(text_na_grass, text_narrator, 0);

        }
        // move stone
        if (tile == 27) {
            if(get_selected_item() == ITEM_POWER){
                if(is_free(x + (x - character[0].x),y + (y - character[0].y)) == 1){
                    incject_map_palette(x, y, 2);
                    incject_map(x, y, (current_map == overworld_a_gbc_map? 20 : 2));

                    incject_collision(x, y, FALSE);
                    current_background[(y * WIDTH) + x] = 2;
                    x += (x - character[0].x);
                    y += (y - character[0].y);
                    incject_map_palette(x, y, 3);
                    incject_map(x, y, 27-2);
                    incject_collision(x, y, TRUE);
                    current_background[(y * WIDTH) + x] = 27;
                }
            } else
                dialog(text_na_rock, text_narrator, 0);
        }
    }
    // we assume there can't be multiple characters at one place
    uint8_t i;
    // 4 enemies
    for(i = 4; i != 0; --i){
        if(character[i].sprite != 0xFF && character[i].x == x && character[i].y == y)
            break;
    }
    // if we have a match
    if(i != 0){
        // turn to main character
        uint8_t stance = character[i].direction/4;
        // mirror direction
        uint8_t direction = U8(character[0].direction + 2)%4;
        character[i].direction = stance + direction;
        render_character(i);
        // talk
        if(level_y == 0){
            // ghost
            if(level_x == 0){
                dialog(text_boohoo, text_ghost, 5);
            }
            if(level_x == 4){
                if(i == 1){
                    dialog(text_stranded, text_t0, 2);
                }else{
                    screen_wobble();
                    if(IS_PRGRS_TIME(0)){
                        SET_PRGRS_TIME(1);
                        dialog(text_the_past, text_t1, 2);
                    }else{
                        SET_PRGRS_TIME(0);
                        dialog(text_the_present, text_t1, 2);
                    }
                }
            }
        }
        if(level_y == 5){
            dialog(text_marvin, text_rachel, 4);
        }
    }
    draw_hud(lives, tpaper);
}