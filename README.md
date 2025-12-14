## GameTank Oscar64 SDK

This repository contains a primitive SDK to compile GameTank games with the [Oscar64 compiler](https://github.com/drmortalwombat/oscar64).


### Example project

There is an example game `hellodvd` in the `src/games` directory. It bounces a box around the screen like a DVD screensaver.

To build it, first [install Oscar64](https://github.com/drmortalwombat/oscar64/blob/main/oscar64.md#installation-and-usage) and then run: `./build.sh hellodvd`

### Making your own game

Simply add another folder to the `src/games` directory and copy paste hellodvd's `main.c` to get started. Then build it with: `./build.sh <folder name>`

### Oscar64 quirks

Oscar64 has some quirks (that maybe are just because I didn't fully set things up properly lol).

#### Data / BSS memory

When you declare a variable, you can either initialize it, like:
```c
uint8_t foo = 123;
```
Or leave it uninitialized, like:
```c
uint8_t foo;
```

If you initialize `foo` to anything (including 0), even though it's not const, Oscar64 will place it on the data section, meaning it goes into ROM and cannot be modified. Bad stuff will happen if you try to write to foo.

If you leave `foo` uninitialized, Oscar64 will place it in the bss section, meaning it is writable, but it will start as 0. You'll need to initialize it yourself in an `init()` function. There is an example of this initializing `dx`/`dy` in hellodvd.

#### ROM banks

When you write code, you should always specify which ROM bank it goes into. If you don't know what that means, that's fine. Just paste this after your includes (but before your actual code):
```
#pragma code(code63)
#pragma data(data63)
#pragma bss(bss)
```

Technical details: Oscar64 supports up to 64 ROM banks. I only set up two of them, 62 and 63. 63 is the fixed memory bank (`0xC000 - 0xFFFF`), and 62 is the banked memory I load up by default (`0x8000 - 0xBFFF`).
