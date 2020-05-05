#include "map.h"
#include "main.h"
#include "hud.h"
#include "level.h"
#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/sound.h"

#include "strings.c"

extern const unsigned char overworld_a_gbc_map[];
extern const unsigned char overworld_b_gbc_map[];

// defined in main.c
extern UINT8 *current_background;
extern const unsigned char *current_map;

void teleport_to(const INT8 lx, const INT8 ly, const INT8 px, const INT8 py) {
    sg->level_x = lx;
    sg->level_y = ly;
    sg->player.x = px;
    sg->player.y = py;
    wait_vbl_done();
    render_character(&(sg->player));
    change_level();
}

UINT8 move_player(const INT8 x, const INT8 y) {
    if (move_character(&(sg->player), x, y) == 1) {
        blinger(0x00 | note_d, 4, 0x00, 0, 0x00 | note_a);
        return 1;
    }

    // leaving the beach if bottle is not collected
    if((sg->collectable & 0x2) == 0 && sg->level_y == 4 && sg->player.y == 0){
        dialog(strlen(text_youdontw), text_youdontw, strlen(text_narrator),
               text_narrator, 0);
        sg->player.direction = 0;
        sg->player.y++;
        return 1;
    }

    UINT8 tile =
        current_background[(sg->player.y * WIDTH) + sg->player.x];

    // trigger stuff

    // cave entrance
    if(tile == 21){
        if(sg->level_x == 4){
            sg->player.direction = 0;
            teleport_to(0, 6, 2, 1);
        }else{
            sg->player.direction = 0;
            teleport_to(1, 6, 7, 1);
        }
    }

    //  house entrance
    if (tile == 34 + 10) {
        if(sg->level_y == 1 && sg->level_x == 1)
            if(sg->player.x > 5)
                teleport_to(0, 5, 5, 6);
            else
                teleport_to(1, 5, 4, 6);
        else if(sg->level_y == 1 && sg->level_x == 0)
            teleport_to(2, 5, 6, 6);
        else if(sg->level_y == 2 && sg->level_x == 0)
            teleport_to(3, 5, 3, 6);
    }

    // player stepped into the doorway
    if(sg->level_y == 5 && sg->player.y == 7){
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

    // player stepped onto the stairs
    if (sg->level_y == 6 && sg->player.y == 0) {
        if(sg->level_x == 0)
            teleport_to(4, 2, 5, 3);
        else
            teleport_to(5, 2, 5, 3);
    }
    return 0;
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
    tile = current_background[(y * WIDTH) + x];
    // write_num(8, 1, 3, tile);
    // sign
    if (tile == 18) {
        if (sg->level_x == 1 && sg->level_y == 1) {
            dialog(strlen(text_welcomet), text_welcomet, strlen(text_sign),
                   text_sign, 1);
        } else {
            dialog(strlen(text_hellowor), text_hellowor, strlen(text_sign),
                   text_sign, 1);
        }
        draw_hud(sg->lives, sg->tpaper);
    }
    // bottle
    if (tile == 34 && current_map == overworld_b_gbc_map) {
        if(sg->collectable & 0x2){
            dialog(strlen(text_thisbott), text_thisbott, strlen(text_narrator),
               text_narrator, 0);
        }else{
            dialog(strlen(text_ohlookth), text_ohlookth, strlen(text_narrator),
               text_narrator, 0);
            dialog(strlen(text_pleasefi), text_pleasefi, strlen(text_letter),
                text_letter, 4);
            sg->collectable |= 0x2;
        }
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 26) {
        if(x == 5 && y == 2 && (sg->collectable & (1<<2)) == 0){
            screen_shake();
            // spawn ghost
            sg->character[0].x = 4;
            sg->character[0].y = 2;
            if(sg->player.x == 4 && sg->player.y == 2){
                sg->character[0].x = 6;
                sg->character[0].y = 2;
            }
            sg->character[0].sprite = 2;
            sg->character[0].direction = 0;
            sg->character[0].palette = 3;
            sg->character[0].offset_x = 0;
            sg->character[0].offset_y = 0;

            render_character(&(sg->character[0]));
            sg->collectable |= (1<<2);
        }
        dialog(strlen(text_somebody), text_somebody, strlen(text_grave),
               text_grave, 2);
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 30 && current_map == overworld_a_gbc_map) {
        dialog(strlen(text_burnever), text_burnever, strlen(text_flame),
               text_flame, 3);
        draw_hud(sg->lives, sg->tpaper);
        // reset();
    }
    if (tile == 32) {
        if (!(sg->collectable & 0x1) && sg->level_x == 1 && sg->level_y == 0) {
            incject_map(2, 2, 30);
            sg->collectable |= 0x1;
            sg->tpaper++;
            draw_hud(sg->lives, sg->tpaper);
            blinger(0x05 | note_a, 4, 0x05 | note_b, 5, 0x04 | note_e);
        }
    }
    // cut grass
    if (tile == 16) {
        incject_map(x, y, 17-2);
        current_background[(y * WIDTH) + x] = current_map[17-2];
    }
    // move stone
    if (tile == 27) {
        if(is_free(x + (x - sg->player.x),y + (y - sg->player.y)) == 1){
            incject_map_palette(x, y, 0);
            incject_map(x, y, 0);
            current_background[(y * WIDTH) + x] = 2;
            x += (x - sg->player.x);
            y += (y - sg->player.y);
            incject_map_palette(x, y, 3);
            incject_map(x, y, 27-2);
            current_background[(y * WIDTH) + x] = 27;
        }
    }
}