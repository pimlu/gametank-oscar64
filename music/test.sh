#!/bin/sh
# Tests of the music player. Each part says so and is skipped if what it needs isn't there.
#
#   ./test.sh                     model and converter (python3 + numpy)
#                                 firmware vs model in the emulator (+ gtaudio, see tests/test_firmware.py)
#   ./test.sh song.fur|song.vgm   ... and with a song of yours (there is none in this repository):
#                                 the whole song in the emulator, in the furnace player and in polyfish
#                                 how close it is to furnace's own render (.fur only; + furnace, scipy)
#
# Rebuilds gametank.bin several times; what is left is a game without a song.
cd "$(dirname "$0")"
SONG="$1"
FAILED=0
SKIPPED=0

run() {
    echo "== $1"
    shift
    "$@"
    status=$?
    if [ $status -eq 77 ]; then
        SKIPPED=$((SKIPPED + 1))
    elif [ $status -ne 0 ]; then
        FAILED=$((FAILED + 1))
    fi
}

build() {
    ../build.sh "$1" > /dev/null 2>&1 || { echo "building $1 failed, run ./build.sh $1"; return 1; }
}

play_in() {
    python3 tests/check_song.py "$SONG" --check-tools || return $?
    build "$1" && python3 tests/check_song.py "$SONG" --loops "$2"
}

run "model and converter" python3 tests/test_model.py
run "firmware vs model, synthetic streams" python3 tests/test_firmware.py
if [ -n "$SONG" ]; then
    [ -f "$SONG" ] || { echo "No such file: $SONG"; exit 1; }
    run "$SONG in the furnace player: through the end and once round the loop" play_in furnace 1
    run "$SONG in polyfish: its real frame rate, the pump called from a banked ROM" play_in polyfish 0
    case "$SONG" in
        *.fur|*.FUR) run "$SONG: model vs furnace's render" python3 tests/compare_furnace.py "$SONG" ;;
        *) echo "== model vs furnace's render: skipped, needs the .fur" ;;
    esac
else
    echo "== no song given: skipping the whole-song tests (./test.sh song.fur)"
fi

[ $SKIPPED -eq 0 ] || echo "$SKIPPED part(s) skipped"
if [ $FAILED -ne 0 ]; then
    echo "$FAILED part(s) FAILED"
    exit 1
fi
echo "All tests passed!"
