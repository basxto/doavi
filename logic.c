#include "map.h"
#include "main.h"
#include "hud.h"
#include "level.h"
#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/sound.h"
#include "utils.h"

#include "strings.h"

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];

// defined in main.c
extern UINT8 *current_background;
extern const unsigned char *current_map;

void teleport_to(const INT8 lx, const INT8 ly, const INT8 px, const INT8 py) {
    sg->level_x = lx;
    sg->level_y = ly;
    sg->character[0].x = px;
    sg->character[0].y = py;
    change_level();
}

UINT8 move_player(const INT8 x, const INT8 y) {
    if (move_character(0, x, y) == 1) {
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
    }

    // leaving the beach if bottle is not collected
    if((sg->progress[1] & PRGRS_BTL) == 0 && sg->level_y == 4 && sg->character[0].y == 0){
        dialog(text_stay_beach, text_narrator, 0);
        sg->character[0].direction = 0;
        sg->character[0].y++;
        return 1;
    }

    UINT8 tile =
        current_background[(sg->character[0].y * WIDTH) + sg->character[0].x];

    // trigger stuff

    // cave entrance
    if(tile == 21 && current_map == overworld_a_gbc_map){
        if(sg->level_x == 4){
            sg->character[0].direction = 0;
            teleport_to(0, 6, 2, 1);
        }else{
            sg->character[0].direction = 0;
            teleport_to(1, 6, 7, 1);
        }
    }

    //  house entrance
    if (tile == $(34 + 10)) {
        if(sg->level_y == 1 && sg->level_x == 1)
            if(sg->character[0].x > $(5))
                teleport_to(0, 5, 5, 6);
            else
                teleport_to(1, 5, 4, 6);
        else if(sg->level_y == 1 && sg->level_x == 0)
            teleport_to(2, 5, 6, 6);
        else if(sg->level_y == 2 && sg->level_x == 0)
            teleport_to(3, 5, 3, 6);
    }

    // player stepped into the doorway
    if(sg->level_y == 5 && sg->character[0].y == 7){
        if (sg->level_x == 0) {
            teleport_to(1, 1, 7, 5);
        }
        else if (sg->level_x == 1) {
            teleport_to(1, 1, 2, 6);
        }
        else if (sg->level_x == 2) {
            teleport_to(0, 1, 4, 6);
        }
        else if (sg->level_x == 3) {
            teleport_to(0, 2, 5, 6);
        }
    }
    // player goes through the back door
    if(sg->level_y == 5 && sg->character[0].y == 2){
        teleport_to(1, 0, 7, 6);
    }

    // player goes back into the house
    if(sg->level_x == 1 && sg->level_y == 0 && sg->character[0].y == 7){
        teleport_to(0, 5, 7, 3);
    }

    // player stepped onto the stairs
    if (sg->level_y == 6 && sg->character[0].y == 0) {
        if(sg->level_x == 0)
            teleport_to(4, 2, 5, 3);
        else
            teleport_to(5, 2, 5, 3);
    }

    return 0;
}


void interact() {
    UINT8 x = sg->character[0].x;
    UINT8 y = sg->character[0].y;
    UINT8 tile;
    switch (sg->character[0].direction) {
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
    tile = current_background[(y * WIDTH) + x];
    // write_num(8, 1, 3, tile);
    // sign
    if (tile == 18) {
        if (sg->level_x == 1 && sg->level_y == 1) {
            dialog(text_village, text_sign, 1);
        } else {
            dialog(text_hello_world, text_sign, 1);
        }
        draw_hud(sg->lives, sg->tpaper);
    }
    // bottle
    if (tile == 34 && current_map == overworld_b_gbc_map) {
        if(sg->progress[1] & PRGRS_BTL){
            dialog(text_empty_bottle, text_narrator, 0);
        }else{
            dialog(text_bottle_post, text_narrator, 0);
            dialog(text_shekiro_1, text_letter, 4);
            sg->progress[1] |= PRGRS_BTL;
        }
        draw_hud(sg->lives, sg->tpaper);
    }
    if(current_map == overworld_a_gbc_map) {
        // grave
        if (tile == 26) {
            if(x == 5 && y == 2 && (sg->progress[0] & PRGRS_GHOST) == 0){
                screen_shake();
                // spawn ghost
                sg->character[1].x = 4;
                sg->character[1].y = 2;
                if(sg->character[0].x == 4 && sg->character[0].y == 2){
                    sg->character[1].x = 6;
                }
                sg->character[1].sprite = 2;
                sg->character[1].direction = 7<<2;//ghost bottom
                sg->character[1].palette = 3<<4 | 3;

                render_character(1);
                // ghost visible
                SET_PRGRS_GHOST(0x1);
                //sg->progress[0] & (0x1<<4));
            }
            dialog(text_dead, text_grave, 2);
            draw_hud(sg->lives, sg->tpaper);
        }
        if (tile == 30 && current_map == overworld_a_gbc_map) {
            dialog(text_dialog4, text_flame, 3);
            draw_hud(sg->lives, sg->tpaper);
            // reset();
        }
    }
    if (current_map == overworld_a_gbc_map || current_map == overworld_b_gbc_map) {
        // chest
        if (tile == 32 || tile == 20) {
            // we don't have to check the status
            // since chests would be a different tile  otherwise
            _Bool update = 0;
            if(sg->level_x == 1 && sg->level_y == 0)
                if (!(sg->chest & 0x1) && y == 2) {
                    sg->chest |= 0x1;
                    ++sg->tpaper;
                    update = 1;
                }else if(!(sg->chest & 1<<1)){
                    sg->chest |= 1<<1;
                    ++sg->tpaper;
                    update = 1;
                }
            if(!(sg->chest & 1<<2) && sg->level_x == 2 && sg->level_y == 0){
                sg->chest |= 1<<2;
                ++sg->tpaper;
                update = 1;
            }
            if(update){
                incject_map(x, y, tile-2);
                draw_hud(sg->lives, sg->tpaper);
                blinger(0x05 | note_a, 4, 0x05 | note_b, 5, 0x04 | note_e);
            }
        }
        // cut grass
        if (tile == 16) {
            incject_map(x, y, 17-2);
            incject_collision(x, y, FALSE);
            current_background[(y * WIDTH) + x] = current_map[17-2];
        }
        // move stone
        if (tile == 27) {
            if(is_free(x + (x - sg->character[0].x),y + (y - sg->character[0].y)) == 1){
                incject_map_palette(x, y, 2);
                incject_map(x, y, (current_map == overworld_a_gbc_map? 20 : 2));

                incject_collision(x, y, FALSE);
                current_background[(y * WIDTH) + x] = 2;
                x += (x - sg->character[0].x);
                y += (y - sg->character[0].y);
                incject_map_palette(x, y, 3);
                incject_map(x, y, 27-2);
                incject_collision(x, y, TRUE);
                current_background[(y * WIDTH) + x] = 27;
            }
        }
    }
}