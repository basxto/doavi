#include "map.h"
#include "main.h"
#include "hud.h"
#include "level.h"
#include "dev/gbdk-music/music.h"
#include "dev/gbdk-music/sound.h"

#include "strings.c"

// defined in main.c
extern Level *current_level;
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

UINT8 move_player(const INT8 x, const INT8 y, const UINT8 *collision) {
    if (move_character(&(sg->player), x, y, collision) == 1) {
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
        current_level->background[(sg->player.y * WIDTH) + sg->player.x];

    // trigger stuff

    //  house entrance
    if (tile == 34 + 10) {
        teleport_to(0, 5, 5, 6);
    }

    // player stepped into the doorway
    if (sg->level_x == 0 && sg->level_y == 5 && sg->player.y == 7) {
        teleport_to(1, 1, 7, 5);
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
    tile = current_level->background[(y * WIDTH) + x];
    // write_num(8, 1, 3, tile);
    // s
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
    if (tile == 34) {
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
        screen_shake();
        dialog(strlen(text_somebody), text_somebody, strlen(text_grave),
               text_grave, 2);
        draw_hud(sg->lives, sg->tpaper);
    }
    if (tile == 30) {
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
}