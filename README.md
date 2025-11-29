## GameTank Oscar64 SDK

This repository contains a simple SDK to compile GameTank games with the [Oscar64 compiler](https://github.com/drmortalwombat/oscar64).


### Example project

There is an example game `hellodvd` in the `src/games` directory. It bounces a box around the screen like a DVD screensaver.

To build it, first [install Oscar64](https://github.com/drmortalwombat/oscar64/blob/main/oscar64.md#installation-and-usage) and then run: `./build.sh hellodvd`

### Making your own game

Simply add another folder to the `src/games` directory and copy paste hellodvd's `main.cpp` to get started. Then build it with: `./build.sh <folder name>`
