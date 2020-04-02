Dessert on a Vegan Island 

Made for [Major Jam: Isolation](https://itch.io/jam/major-jam-isolation)

It’s a play on [Vegan on a Desert Island](https://gitlab.com/voadi/voadi)

Target platform is GameBoy and GameBoy Color.



## Build
This needs [GNU Make](https://www.gnu.org/software/make/) and [SDCC](https://sourceforge.net/projects/sdcc/).
Some tools need python 3 and `PyPNG` installed.
```
git clone --recursive https://github.com/basxto/doavi.git
cd doavi
make gbdk-n && make

```

## Play with
This needs an emulator, by default it’s configured to use gambatte via retroarch
```
make run EMU="vbam"
```