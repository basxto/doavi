Dessert on a Vegan Island 

Made for [Major Jam: Isolation](https://itch.io/jam/major-jam-isolation)

It’s a play on [Vegan on a Desert Island](https://gitlab.com/voadi/voadi)

Target platform is GameBoy and GameBoy Color.



## Build
This needs [GNU Make](https://www.gnu.org/software/make/), [GBDK-2020](https://github.com/Zal0/gbdk-2020) 4.0.3 and [SDCC](https://sourceforge.net/projects/sdcc/) 4.1.0.
Some tools need python 3 and `PyPNG` installed.
```
git clone --recursive https://github.com/basxto/doavi.git
cd doavi
make -C dev/lzcomp
make

```

## Play with
This needs an emulator, by default it’s configured to use gambatte via retroarch
```
make run
```

## Optional dependencies

[GameBoy Online][] for exporting it to itch.io with an emulator written in JavaScript.

gb2png.py (have to create repository yet) to convert the ROM into a png, where you can see the tiles as well as glitchy noise, that is actually code.

## ROM/RAM testing
ROM/RAM testing is unfinished and not working.
Images, maps and strings get placed on specific positions in the ROM, but switching ROMs does not work.

## Licensing

All code is put under [MIT](license.md) license by [basxto][].
“[The Journey Begins GameBoy Remix](dev/gbdk-music/music/the_journey_begins.c)” is based on “[The Journey Begins][]” by [Igor Gundarev][], which is licensed under [cc0][], and therefore is also licensed under [cc0][].

[overworld_a_gbc.png](pix/overworld_a_gbc.png) and [overworld_b_gbc.png](pix/overworld_b_gbc.png) are both from “[Zoria Tileset][]” by [DragonDePlatino][], which is licensed under [cc-by 4.0][]

[ghost_gbc.png](pix/ghost_gbc.png) just got the colors fixed and is from “[Tiny 16: Basic][]” by [Lanea Zimmerman][], which is licensed under [cc-by 3.0][]

[muffin_gbc.png](pix/muffin_gbc.png) is made by [basxto][] and licensed under [cc0][]

[angry_toast_gbc.png](pix/angry_toast_gbc.png) is from [voadi][] by [Nathan Manske][] and licensed under [cc-by-sa 4.0][]

[bush.png](pix/bush.png) is an improved (more repitition) version of the bushes from “[Zoria Tileset][]” by [DragonDePlatino][], which is licensed under [cc-by 4.0][]

[dialog_photos.png](pix/dialog_photos.png) are from [voadi][] by [Alex Gleason][] and licensed under [cc-by-sa 4.0][]

[house_wood_round.png](pix/house_wood_round.png) are a havily modified version of the houses from [voadi][] by [Alex Gleason][] and licensed under [cc-by-sa 4.0][], with inspiration from [Rpg House][] from [Angel][], which is licensed under [cc0][]

[win_gbc.png](pix/win_gbc.png) is [squont8][] by [basxto][] and licensed under [cc0][], with some menu tiles in the first row from [voadi][] by [Alex Gleason][] and licensed under [cc0][]

[collision.png](pix/collision.png) is made by [basxto][] and licensed under [cc0][]

[overworld_anim_gbc.png](pix/overworld_anim_gbc.png) the flowers are from [voadi][] by [Alex Gleason][] and licensed under [cc-by-sa 4.0][], the rest is from “[Zoria Tileset][]” by [DragonDePlatino][], which is licensed under [cc-by 4.0][]

[The Journey Begins]: https://opengameart.org/content/the-journey-begins
[Zoria Tileset]: https://opengameart.org/content/zoria-tileset
[Tiny 16: Basic]: https://opengameart.org/content/tiny-16-basic
[Rpg House]: https://opengameart.org/content/rpg-house
[voadi]: https://voadi.com/
[squont8]: https://opengameart.org/content/squont8

[basxto]: https://github.com/basxto
[Igor Gundarev]: https://opengameart.org/users/igor-gundarev
[DragonDePlatino]: https://opengameart.org/users/dragondeplatino
[Lanea Zimmerman]: https://opengameart.org/users/sharm
[SILTOCYN]: https://siltocyn.itch.io/
[Alex Gleason]: https://alexgleason.me/
[Angel]: https://opengameart.org/users/angel
[Nathan Manske]: https://gitlab.com/nmanske

[cc0]: https://creativecommons.org/publicdomain/zero/1.0/deed
[cc-by 3.0]: https://creativecommons.org/licenses/by/4.0/
[cc-by 4.0]: https://creativecommons.org/licenses/by/4.0/
[cc-by-sa 4.0]: https://creativecommons.org/licenses/by-sa/4.0/

[GameBoy Online]: https://github.com/taisel/GameBoy-Online

## After jam stretch goals:
* [ ] Support items that can be actively used
* [ ] Allow environment destruction (digging, gras cutting)
* [ ] Support houses and caves
* [ ] Implement 20min of story
* [ ] Fill up ROM
* [ ] Compress data and fill up ROM even more