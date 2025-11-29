#!/bin/sh
set -e

oscar64 -tf=bin -xz -tm=c64 -n -O3 -pp src/system/boot.cpp -o=output.bin

python3 asm2gametank.py output.asm gametank.bin
