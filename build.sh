#!/bin/sh
set -e

oscar64 -tf=bin -tm=c64 -n -O2 -pp src/system/boot.cpp -o=output.bin

python3 asm2gametank.py output.asm gametank.bin
