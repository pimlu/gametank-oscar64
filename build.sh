#!/bin/sh
set -e

cd "$(dirname "$0")"


usage() {
    echo "usage: ./build.sh <game-name>"
    echo "Available games:"
    ls src/games | xargs -I % echo "    %"
    echo "This will create a 2MB ROM, gametank.bin, in the project directory."
    exit 1
}


if [ "$#" -ne 1 ]; then
    echo "Expected exactly one argument."
    echo
    usage
fi

if ! [ -d "src/games/$1" ]; then
    echo "Invalid game '$1'."
    echo
    usage
fi


oscar64 -i=src/ -tf=bin -xz -tm=c64 -n -O3 -pp "src/games/$1/main.cpp" -o=output.bin

python3 asm2gametank.py output.asm gametank.bin
