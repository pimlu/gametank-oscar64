#!/usr/bin/env python3
"""Does this song work on the GameTank? Plays the whole converted song in the emulator (see
test_firmware.py for what that needs) and checks the output against the model, byte for byte,
through the end and on round the loop. Fails if the firmware ever had to wait for a late tick
(the song is too busy for the ring buffer at this frame rate), lost an IRQ, or ran out of
cycles.

usage: check_song.py song.fur|song.vgm [--rate d0] [--rom game.bin] [--loops 1] [--wav out.wav]
--rom: a ROM built by ../../build.sh to play it in; default: gametank.bin, which has to be a game
that plays music (./build.sh furnace, or polyfish to test at its frame rate)."""
import argparse
import os
import sys
import tempfile
import time

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import test_firmware  # noqa: E402
import vgm2gts  # noqa: E402

ap = argparse.ArgumentParser()
ap.add_argument('song')
ap.add_argument('--check-tools', action='store_true', help='only check that the emulator is there (exit status 77 if not)')
ap.add_argument('--rate', default='d0')
ap.add_argument('--wav', help='keep the recording of the emulated GameTank here')
ap.add_argument('--rom', help='ROM to play it in (default: the furnace player)')
ap.add_argument('--loops', type=int, default=1, help='rounds of the loop to check after the first pass')
a = ap.parse_args()
if a.check_tools:
    if not test_firmware.find_gtaudio():
        print('skipped: needs gtaudio too')
        sys.exit(test_firmware.SKIPPED)
    sys.exit(0)
test_firmware.need_gtaudio()

with tempfile.TemporaryDirectory() as tmp:
    is_fur = a.song.lower().endswith('.fur')
    vgm = vgm2gts.fur_to_vgm(a.song, tmp) if is_fur else a.song
    song = vgm2gts.convert(vgm, int(a.rate, 16), vgm2gts.fur_wavetables(a.song, tmp) if is_fur else ())
    t0 = time.time()
    got, expected, info = test_firmware.run_song(song, a.rom or os.path.join(test_firmware.REPO, 'gametank.bin'), tmp, 'song',
                                             tail_ticks=30, loops=a.loops)
    print('emulated %.1f s in %.1f s; start delay %d samples, irq avg %.0f max %d of %d cycles'
          % (len(expected) / test_firmware.gts.sample_rate(song.rate_reg), time.time() - t0, info['delay'],
             info['irq_avg'], info['irq_max'], info['budget']))
    if a.wav:
        vgm2gts.write_wav(a.wav, got[info['delay']:len(expected)], test_firmware.gts.sample_rate(song.rate_reg))
    try:
        test_firmware.check(got, expected, info)
    except AssertionError as e:
        sys.exit('FAIL: %s' % e)
    print('ok: all %d samples match the model; no late ticks, no lost IRQs' % len(expected))
