#!/usr/bin/env python3
"""Firmware tests: the ACP firmware (src/audio/acp_fw.c), running in an emulator, has to
reproduce the model in music/gts.py byte for byte.

Each test is a small synthetic GTS song. It is patched into the ROM built from the `furnace`
game, run in gtaudio, and the recorded DAC stream is compared with gts.render(). Also checked:
no lost ACP IRQs, the firmware never waited for a late tick, the IRQ handler fits the sample
period, and power-on RAM contents don't matter.

Needs gtaudio, the headless GameTank emulator that records the DAC
(gametank-audio-emulator, see GTAUDIO_URL below): $GTAUDIO, or on the PATH, or built in a
gametank-audio-emulator checkout next to this repository. No song and no furnace needed.

usage: test_firmware.py [-k substring] [--keep] [--no-build]       (leaves gametank.bin = furnace)
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import wave

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
MUSIC = os.path.dirname(HERE)
REPO = os.path.dirname(MUSIC)
sys.path.insert(0, MUSIC)
import gts  # noqa: E402

GTAUDIO_URL = 'https://github.com/pimlu/gametank-audio-emulator'
SKIPPED = 77     # exit status: couldn't run for lack of a tool, as opposed to failed


def find_gtaudio():
    """$GTAUDIO, the PATH, or gametank-audio-emulator/build/gtaudio next to this repository (or
    next to a directory this repository is nested in)"""
    if os.environ.get('GTAUDIO'):
        return os.environ['GTAUDIO'] if os.path.exists(os.environ['GTAUDIO']) else None
    found = shutil.which('gtaudio')
    d = REPO
    for _ in range(3):
        d = os.path.dirname(d)
        candidate = os.path.join(d, 'gametank-audio-emulator', 'build', 'gtaudio')
        if not found and os.path.exists(candidate):
            found = candidate
    return found


def need_gtaudio():
    path = find_gtaudio()
    if not path:
        print('skipped: gtaudio not found. It is the headless GameTank emulator these tests run the firmware in:\n'
              '  ' + GTAUDIO_URL + ' (make), then set $GTAUDIO, put it on the\n'
              '  PATH, or keep that checkout next to this repository.')
        sys.exit(SKIPPED)
    return path

RATE_22K, RATE_14K = 0xD0, 0xFF

SAW = list(range(32))
SQUARE = [0] * 16 + [31] * 16
SINE = [int(round(15.5 + 15.5 * np.sin(2 * np.pi * (i + 0.5) / 32))) for i in range(32)]
TRI = [min(2 * i, 63 - 2 * i) for i in range(32)]
LOPSIDED = [0, 31, 1, 30, 15, 16, 7, 24] * 4
WAVES = [SAW, SQUARE, SINE, TRI, LOPSIDED]


def new_song(rate=RATE_22K, waves=WAVES):
    return gts.Song(rate_reg=rate, waves=[list(w) for w in waves])


# --- the songs ---------------------------------------------------------------------------------

def t_one_tone(rate=RATE_22K):
    s = new_song(rate)
    s.inc(0, s.hz_to_inc(440)); s.wave(0, 0); s.gain(0, 512)
    s.tick(29)
    s.end()
    return s


def t_one_tone_14k():
    return t_one_tone(RATE_14K)


def t_each_channel():
    s = new_song()
    for ch in range(6):
        s.inc(ch, s.hz_to_inc(110 * (ch + 1))); s.wave(ch, ch % len(WAVES)); s.gain(ch, 300 + 100 * ch)
        s.tick(5)
        s.gain(ch, 0)
    s.tick()
    s.end()
    return s


def t_gain_envelope():
    s = new_song()
    s.inc(1, s.hz_to_inc(330)); s.wave(1, 2)
    for g in list(range(0, 2080, 97)) + [2080, 1, 2, 255, 256, 257, 0]:
        s.gain(1, g)
        s.tick()
    s.end()
    return s


def t_wave_sequence():
    s = new_song()
    s.inc(3, s.hz_to_inc(262)); s.gain(3, 700)
    for i in range(40):
        s.wave(3, i % len(WAVES))
        s.tick(i % 3)
    s.end()
    return s


def t_vibrato():
    s = new_song()
    s.wave(2, 3); s.gain(2, 900)
    for i in range(90):
        s.inc(2, s.hz_to_inc(440 * 2 ** (np.sin(i / 5) / 24)))
        s.tick()
    s.end()
    return s


def t_extreme_pitch():
    s = new_song()
    s.wave(0, 0); s.gain(0, 600)
    for inc in (1, 255, 256, 65535, 65536, 0x1FFFFF, 0x200000, 0xFFFFFF, 0):
        s.inc(0, inc)
        s.tick(2)
    s.end()
    return s


def t_noise():
    s = new_song()
    s.wave(4, 1); s.inc(4, s.hz_to_inc(200)); s.gain(4, 500)
    s.wave(5, 2); s.inc(5, s.hz_to_inc(300)); s.gain(5, 400)
    s.tick(3)
    for n in (1, 1000, 30000, 65535):
        s.noise(4, n)
        s.tick(4)
        s.noise(5, 65535 - n); s.gain(5, 250)
        s.tick(4)
        s.noise(4, 0)          # back to the wavetable, phase index left at the last noise bit
        s.tick(2)
    s.noise(5, 0); s.gain(4, 0)
    s.tick(3)
    s.end()
    return s


def t_set_phase():
    s = new_song()
    for ch in range(6):
        s.inc(ch, s.hz_to_inc(100 + 37 * ch)); s.wave(ch, ch % len(WAVES)); s.gain(ch, 300)
    s.tick(2)
    for i in range(20):
        for ch in range(6):
            if (i + ch) % 3 == 0:
                s.phase(ch, (7 * i + ch) & 31)
        if i == 5:
            s.noise(4, 40000)      # phase set during noise: index gets overwritten right away
        if i == 9:
            s.noise(4, 0); s.phase(4, 9)
        if i % 4 == 0:
            s.inc(1, s.hz_to_inc(200 + i)); s.phase(1, 0); s.wave(1, i % len(WAVES)); s.gain(1, 200 + i)
        s.tick(i % 2)
    s.end()
    return s


def t_chord(rate=RATE_22K):
    s = new_song(rate)
    for ch, hz in enumerate((65.4, 130.8, 164.8, 196.0, 261.6, 329.6)):
        s.inc(ch, s.hz_to_inc(hz)); s.wave(ch, ch % len(WAVES)); s.gain(ch, 330)
    s.tick(59)
    s.end()
    return s


def t_chord_14k():
    return t_chord(RATE_14K)


def t_long_waits():
    s = new_song()
    s.inc(0, s.hz_to_inc(500)); s.wave(0, 1); s.gain(0, 400)
    s.tick(300)
    s.gain(0, 0)
    s.tick(256)
    s.gain(0, 800)
    s.tick(255)
    s.end()
    return s


def random_song(seed, ticks, rate=RATE_22K, density=0.5, end=True):
    rng = np.random.default_rng(seed)
    s = new_song(rate)
    for _ in range(ticks):
        for ch in range(6):
            if rng.random() < density:
                s.inc(ch, int(rng.integers(0, 1 << 21)))
            if rng.random() < density:
                s.gain(ch, int(rng.integers(0, 700)))
            if rng.random() < density:
                s.wave(ch, int(rng.integers(0, len(WAVES))))
            if rng.random() < density / 2:
                s.phase(ch, int(rng.integers(0, 32)))
            if ch >= 4 and rng.random() < density / 2:
                s.noise(ch, int(rng.integers(0, 65536)) if rng.random() < 0.7 else 0)
        s.tick(int(rng.integers(0, 3)) if rng.random() < 0.3 else 0)
    if end:
        s.end()
    return s


def t_random_busy():
    # every channel changing everything nearly every tick: worst case for the main loop
    return random_song(1, 240, density=0.9)


def t_random_14k():
    return random_song(2, 240, RATE_14K)


def t_bank_crossing():
    # > 16K of stream, so the pump has to move on to a second ROM bank
    s = random_song(3, 900, density=0.8)
    assert len(s.stream) > 2 * gts.BANK_SIZE - 4 or len(s.stream) > gts.BANK_SIZE, len(s.stream)
    return s


def t_loop():
    s = new_song()
    s.inc(0, s.hz_to_inc(220)); s.wave(0, 3); s.gain(0, 500)
    s.tick(4)
    start = s.mark()
    for i in range(6):
        s.inc(1, s.hz_to_inc(330 + 40 * i)); s.wave(1, i % len(WAVES)); s.gain(1, 100 + 60 * i)
        s.tick(i % 3)
    s.gain(0, 300 + len(s.stream))     # the state at the end need not match the loop point for this test
    s.tick()
    s.loop_to(start)
    return s


def t_loop_across_banks():
    # the loop point is in the second stream bank, the end in the third
    s = random_song(4, 900, density=0.8, end=False)
    ticks_at = [i for i in range(len(s.stream)) if i > gts.BANK_SIZE + 500 and s.stream[i - 1] == gts.CMD_TICK]
    assert len(s.stream) > 2 * gts.BANK_SIZE
    # a byte that follows an FF isn't necessarily the start of a tick; pick one that parses
    for off in ticks_at:
        try:
            gts.parse_ticks(s.stream, off, 1)
        except (AssertionError, ValueError, IndexError):
            continue
        s.loop_to(off)
        return s
    raise AssertionError('no loop point found')


TESTS = [t_one_tone, t_one_tone_14k, t_each_channel, t_gain_envelope, t_wave_sequence, t_vibrato,
         t_extreme_pitch, t_noise, t_set_phase, t_chord, t_chord_14k, t_long_waits, t_random_busy, t_random_14k,
         t_bank_crossing, t_loop, t_loop_across_banks]


# --- running them ------------------------------------------------------------------------------

def run_song(song, base_rom, workdir, name, seed=1, tail_ticks=6, loops=2):
    """-> (dac bytes recorded, expected bytes, info dict). A looping song is checked for the
    first pass and `loops` more rounds of the loop."""
    rom = os.path.join(workdir, name + '.bin')
    wav = os.path.join(workdir, name + '.wav')
    rep = os.path.join(workdir, name + '.json')
    aram = os.path.join(workdir, name + '.aram')
    gts.patch_rom(base_rom, rom, song)

    model = gts.render(song, tail_ticks=tail_ticks, loops=loops)
    rate = gts.sample_rate(song.rate_reg)
    seconds = len(model) / rate + 0.25          # + start-up delay (the ring is prefilled: short)
    r = subprocess.run([need_gtaudio(), rom, '-s', '%.3f' % seconds, '-o', wav, '--report', rep,
                        '--dump-aram', aram, '--seed', str(seed)], capture_output=True, text=True)
    if r.returncode != 0:
        raise AssertionError('gtaudio exit status %d: %s' % (r.returncode, r.stderr.strip()))
    with wave.open(wav, 'rb') as w:
        assert w.getsampwidth() == 1 and w.getnchannels() == 1
        got = np.frombuffer(w.readframes(w.getnframes()), dtype=np.uint8)
    report = json.load(open(rep))
    ram = open(aram, 'rb').read()
    info = {
        'delay': ram[0xA5] | ram[0xA6] << 8,
        'stall': ram[0xA3] | ram[0xA4] << 8,
        'irqs_missed': report['irqs_missed'],
        'irq_max': report['irq_cycles_max'],
        'irq_avg': report['irq_cycles_avg'],
        'budget': report['acp_cycles_per_sample'],
        'stream_bytes': len(song.stream),
    }
    expected = np.concatenate([np.full(info['delay'], 0x80, dtype=np.uint8), model])
    return got, expected, info


def check(got, expected, info):
    assert info['stall'] == 0, 'firmware waited %d samples for late ticks' % info['stall']
    assert info['irqs_missed'] == 0, '%d ACP IRQs lost' % info['irqs_missed']
    assert info['irq_max'] < info['budget'], 'IRQ handler too slow: %d of %d' % (info['irq_max'], info['budget'])
    assert len(got) >= len(expected), 'recording too short: %d < %d' % (len(got), len(expected))
    got = got[:len(expected)]
    if not np.array_equal(got, expected):
        bad = np.flatnonzero(got != expected)
        i = int(bad[0])
        raise AssertionError('%d of %d samples differ, first at %d (%d into the song): got %s, expected %s'
                             % (len(bad), len(expected), i, i - info['delay'],
                                got[i:i + 8].tolist(), expected[i:i + 8].tolist()))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-k', default='', help='only tests whose name contains this')
    ap.add_argument('--keep', action='store_true', help='keep the work directory (ROMs, wavs, reports)')
    ap.add_argument('--no-build', action='store_true', help="don't rebuild the furnace ROM first")
    a = ap.parse_args()
    need_gtaudio()

    if not a.no_build:
        r = subprocess.run([os.path.join(REPO, 'build.sh'), 'furnace'], capture_output=True, text=True)
        if r.returncode != 0 or 'error' in (r.stdout + r.stderr).lower():
            sys.exit('build failed:\n' + '\n'.join(l for l in (r.stdout + r.stderr).splitlines() if 'error' in l.lower()))
    base_rom = os.path.join(REPO, 'gametank.bin')

    workdir = tempfile.mkdtemp(prefix='gts_tests_')
    failed = 0
    for t in TESTS:
        name = t.__name__[2:]
        if a.k not in name:
            continue
        try:
            song = t()
            got, expected, info = run_song(song, base_rom, workdir, name)
            check(got, expected, info)
            # power-on RAM contents must not matter
            got2, _, _ = run_song(song, base_rom, workdir, name + '_seed2', seed=2)
            assert np.array_equal(got, got2), 'output depends on power-on RAM contents'
            print('ok    %-16s %6d samples, %5d stream bytes, start delay %3d, irq avg %.0f max %d of %d cycles'
                  % (name, len(expected), info['stream_bytes'], info['delay'], info['irq_avg'], info['irq_max'], info['budget']))
        except AssertionError as e:
            failed += 1
            print('FAIL  %-16s %s' % (name, e))
    if a.keep or failed:
        print('work directory:', workdir)
    else:
        subprocess.run(['rm', '-rf', workdir])
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
