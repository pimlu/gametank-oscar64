#!/bin/sh
# Build a game and put a Furnace song into the ROM for it to play.
set -e
# (no cd: the song and --wav paths are relative to where you are)
DIR="$(cd "$(dirname "$0")" && pwd)"

usage() {
    cat <<USAGE
usage: ./build_with_song.sh <game-name> <song.fur | song.vgm> [options]

Builds the game like ./build.sh, then converts the song and writes it into gametank.bin
(next to this script).
Games that call music_init()/music_pump() play it: polyfish, and furnace (which does nothing
else). Without a song in the ROM they are simply silent.

The song has to be for the PC Engine / TurboGrafx-16 chip, using wavetables and noise only
(no PCM samples, no hardware LFO). Furnace comes with some in demos/pce/.
  song.fur  needs the furnace command line on your PATH
  song.vgm  what Furnace's "file > export > VGM" writes; no furnace needed

options:
  --once       play the song once instead of looping
  --rate ff    13983 Hz instead of 22233 Hz: more ACP time to spare, more aliasing
  --wav FILE   also write exactly what the GameTank will output to a wav, to listen to it

Needs python3 with numpy, and whatever ./build.sh needs.
USAGE
    exit 1
}

[ "$#" -ge 2 ] || usage
GAME="$1"
SONG="$2"
shift 2

[ -d "$DIR/src/games/$GAME" ] || { echo "Invalid game '$GAME'."; echo; usage; }
[ -f "$SONG" ] || { echo "No such file: $SONG"; echo; usage; }
python3 -c 'import numpy' 2>/dev/null || { echo "python3 with numpy is needed (pip install numpy)."; exit 1; }
case "$SONG" in
    *.fur|*.FUR)
        command -v furnace >/dev/null || {
            echo "'furnace' is not on your PATH. Either add it, or open the song in Furnace, use"
            echo "file > export > VGM, and pass the .vgm instead."
            exit 1
        } ;;
esac

LOG="$(mktemp)"
if ! "$DIR/build.sh" "$GAME" > "$LOG" 2>&1 || grep -q ") : error " "$LOG"; then
    grep -E "error|Error" "$LOG" || cat "$LOG"
    rm -f "$LOG"
    echo "build failed"
    exit 1
fi
rm -f "$LOG"

python3 "$DIR/music/vgm2gts.py" "$SONG" --rom "$DIR/gametank.bin" "$@"
