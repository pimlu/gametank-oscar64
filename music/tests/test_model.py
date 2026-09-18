#!/usr/bin/env python3
"""Tests of the converter and the model that need nothing but python3 and numpy: no emulator, no
furnace, no song. The PC Engine register logs are made up here, written the way Furnace writes
them, and the expected values are worked out independently of the code under test.

usage: test_model.py [-k substring]
"""
import argparse
import os
import struct
import sys
import tempfile

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import gts  # noqa: E402
import vgm2gts  # noqa: E402

RATE_REG = 0xD0
RATE = 315e6 / 88 / 161          # $D0 -> a timer period of 161 main clock cycles
TRI = [min(2 * i, 63 - 2 * i) for i in range(32)]
SAW = list(range(32))


class Vgm:
    """a HuC6280 register log, one tick = 735 samples at 44100 Hz = 1/60 s"""

    def __init__(self):
        self.data = bytearray()
        self.samples = 0
        self.loop_at = None

    def write(self, ch, reg, value):
        self.data += bytes([0xB9, 0, ch, 0xB9, reg, value])

    def upload(self, ch, wave, rotate=0, volume=31):
        # what Furnace's updateWave() does: DDA on and off again restarts the wave, then the 32
        # samples, rotated by where it thinks playback was, then the channel goes back on
        self.write(ch, 4, 0x5F)
        self.write(ch, 4, 0x1F)
        for i in range(32):
            self.write(ch, 6, wave[(i + rotate) & 31])
        self.write(ch, 4, 0x80 | volume)

    def period(self, ch, period):
        self.write(ch, 2, period & 0xFF)
        self.write(ch, 3, period >> 8)

    def tick(self, n=1):
        for _ in range(n):
            self.data += b'\x62'
            self.samples += 735

    def loop_here(self):
        self.loop_at = (len(self.data), self.samples)

    def save(self, path):
        hdr = bytearray(0x100)
        hdr[0:4] = b'Vgm '
        struct.pack_into('<I', hdr, 0x08, 0x171)
        struct.pack_into('<I', hdr, 0x18, self.samples)
        struct.pack_into('<I', hdr, 0x24, 60)
        struct.pack_into('<I', hdr, 0x34, 0x100 - 0x34)
        struct.pack_into('<I', hdr, 0xA4, 3579545)
        if self.loop_at:
            struct.pack_into('<I', hdr, 0x1C, 0x100 + self.loop_at[0] - 0x1C)
            struct.pack_into('<I', hdr, 0x20, self.samples - self.loop_at[1])
        open(path, 'wb').write(bytes(hdr) + bytes(self.data) + b'\x66')
        return path


def states_of(vgm, tmp):
    writes, total, tick_hz, loop_time = vgm2gts.parse_vgm(vgm.save(os.path.join(tmp, 't.vgm')))
    return vgm2gts.chip_states(writes, total, tick_hz), loop_time


def commands(song, loops=0):
    names = {gts.CMD_INC: 'inc', gts.CMD_GAIN: 'gain', gts.CMD_WAVE: 'wave', gts.CMD_NOISE: 'noise', gts.CMD_PHASE: 'phase'}
    return [[(names[t], ch, v) for t, ch, v in tick] for tick in gts.parse_ticks(song.stream, song.loop_offset, loops)]


def expected_inc(period):
    # the PCE steps through its wave at clock / period samples per second
    return int(round(3579545 / period / RATE * 65536))


def t_note(tmp):
    """one note: pitch, volume, and a wave that Furnace uploaded rotated"""
    v = Vgm()
    v.write(0, 1, 0xFF)
    v.write(0, 5, 0xFF)
    v.period(0, 508)                 # 220.2 Hz
    v.upload(0, TRI, rotate=5)
    v.tick(3)
    states, _ = states_of(v, tmp)
    song = vgm2gts.build(states, RATE_REG, 60, amp=2.0, known_waves=[TRI])
    assert song.waves == [TRI], 'the wave should come out unrotated'
    first = dict(((n, ch), val) for n, ch, val in commands(song)[0])
    assert first[('wave', 0)] == 0
    assert first[('inc', 0)] == expected_inc(508), first
    assert first[('gain', 0)] == 512, first          # full volume * amp 2.0 * 256
    # the PCE restarted the wave at sample 5 of the real wave, +1 for switching the channel on
    assert first[('phase', 0)] == 6, first
    assert commands(song)[1:] == [[], []], 'nothing changes after the first tick'
    assert song.stream[-1] == gts.CMD_END and song.loop_offset is None


def t_unknown_wave_rotations(tmp):
    """without the song's wavetables, rotations of one wave still count as one wave"""
    v = Vgm()
    v.write(0, 1, 0xFF); v.write(0, 5, 0xFF); v.period(0, 400)
    v.upload(0, SAW, rotate=3)
    v.tick()
    v.upload(0, SAW, rotate=11)
    v.tick()
    v.upload(1, SAW, rotate=20); v.write(1, 5, 0xFF); v.period(1, 300)
    v.tick()
    states, _ = states_of(v, tmp)
    song = vgm2gts.build(states, RATE_REG, 60, amp=2.0)
    assert len(song.waves) == 1, song.waves


def t_mono_gain(tmp):
    """volume, pan and master volume are 1.5 / 3 / 3 dB steps; mono = mean of left and right"""
    def gain_for(volume, pan, master):
        v = Vgm()
        v.write(0, 1, master); v.write(0, 5, pan); v.period(0, 400)
        v.upload(0, TRI, volume=volume)
        v.tick()
        states, _ = states_of(v, tmp)
        song = vgm2gts.build(states, RATE_REG, 60, amp=1.0, known_waves=[TRI])
        return dict(((n, ch), val) for n, ch, val in commands(song)[0]).get(('gain', 0), 0)

    db = lambda x: 10 ** (-x / 20)
    assert gain_for(31, 0xFF, 0xFF) == 256
    # hard left: half in mono. (Pan 0 is 30 steps = -45 dB, not quite off: only 31 steps are.)
    assert gain_for(31, 0xF0, 0xFF) == round(256 * (1 + db(45.0)) / 2)
    assert gain_for(29, 0xFF, 0xFF) == round(256 * db(3.0))             # 2 volume steps
    assert gain_for(31, 0xEE, 0xFF) == round(256 * db(3.0))             # 1 pan step on both sides
    assert gain_for(31, 0xFE, 0xDD) == round(256 * db(6.0) * (1 + db(3.0)) / 2)
    assert gain_for(0, 0xFF, 0xFF) == 0                                 # 31 steps: silence
    assert gain_for(1, 0xEE, 0xFF) == 0                                 # 30 + 2 steps
    assert gain_for(31, 0x00, 0xFF) == round(256 * db(45.0))


def t_silence_is_cheap(tmp):
    """pitch changes of a silent channel aren't sent; it catches up when it is audible again"""
    v = Vgm()
    v.write(0, 1, 0xFF); v.write(0, 5, 0xFF); v.period(0, 400)
    v.upload(0, TRI)
    v.tick()
    v.write(0, 4, 0x80)              # volume 0
    v.tick()
    for p in (390, 380, 370):
        v.period(0, p)
        v.tick()
    v.write(0, 4, 0x9F)
    v.tick()
    states, _ = states_of(v, tmp)
    cmds = commands(vgm2gts.build(states, RATE_REG, 60, amp=2.0, known_waves=[TRI]))
    assert cmds[1] == [('gain', 0, 0)], cmds[1]
    assert cmds[2:5] == [[], [], []], cmds[2:5]
    back = dict(((n, ch), val) for n, ch, val in cmds[5])
    assert back[('inc', 0)] == expected_inc(370) and back[('gain', 0)] == 512, back


def t_noise(tmp):
    """noise rate, and less gain when the PCE clocks its noise faster than we can sample it"""
    def first_tick(nf):
        v = Vgm()
        v.write(4, 1, 0xFF); v.write(4, 5, 0xFF); v.period(4, 400)
        v.upload(4, TRI)
        v.write(4, 7, 0x80 | nf)
        v.tick()
        states, _ = states_of(v, tmp)
        return dict(((n, ch), val) for n, ch, val in commands(vgm2gts.build(states, RATE_REG, 60, amp=2.0))[0])

    slow = first_tick(0)             # 3579545 / (64 * 31) = 1804 Hz
    assert slow[('noise', 4)] == round(1804.2 / RATE * 65536), slow
    assert slow[('gain', 4)] == 512
    assert ('wave', 4) not in slow and ('inc', 4) not in slow
    fast = first_tick(31)            # 111861 Hz, five times our sample rate
    assert fast[('noise', 4)] == 65535, fast
    assert fast[('gain', 4)] == round(512 * (RATE / 111860.8) ** 0.5), fast


def audible_states(ticks):
    st = [dict(inc=0, gain=0, wave=0, noise=0) for _ in range(6)]
    out = []
    for tick in ticks:
        for name, ch, val in tick:
            if name != 'phase':
                st[ch][name] = val
        out.append([dict(c) if c['gain'] else None for c in st])
    return out


def t_loop(tmp):
    """a song with a loop point: every round has to be the same as the first"""
    v = Vgm()
    v.write(0, 1, 0xFF)
    for ch in range(3):
        v.write(ch, 5, 0xFF)
    v.period(0, 500); v.upload(0, TRI)                 # intro: channel 0 only
    v.tick(4)
    v.loop_here()
    v.period(1, 250); v.upload(1, SAW, volume=20)      # the loop tick itself starts a note
    v.tick(2)
    v.period(0, 450)
    v.tick(3)
    v.write(1, 4, 0)                                   # ends differently from how the loop starts
    v.period(2, 100); v.upload(2, SAW)
    v.period(0, 333)
    v.tick(2)
    states, loop_time = states_of(v, tmp)
    assert loop_time == 4 * 735
    song = vgm2gts.build(states, RATE_REG, 60, amp=2.0, known_waves=[TRI, SAW], loop_tick=4)
    assert song.loop_offset is not None and gts.CMD_END not in (song.stream[-1],)
    once = commands(song)
    n = len(once)
    assert n == 11 + 1, n                              # 11 ticks + the one standing in for tick 4
    st = audible_states(commands(song, loops=2))
    round_len = n - 5
    for r in (1, 2):
        for i in range(round_len):
            # tick 4's stand-in is the last tick of a pass, so a round runs from tick 5
            assert st[5 + i] == st[5 + r * round_len + i], (r, i, st[5 + i], st[5 + r * round_len + i])
    assert st[4] == st[n - 1], 'the stand-in tick has to leave things as tick 4 did'
    # and no loop when asked not to
    song = vgm2gts.build(states, RATE_REG, 60, amp=2.0, known_waves=[TRI, SAW])
    assert song.loop_offset is None and song.stream[-1] == gts.CMD_END


def t_rom_layout(tmp):
    """header, 16K banks with a length each, loop pointer"""
    rng = np.random.default_rng(5)
    song = gts.Song(rate_reg=RATE_REG, waves=[TRI, SAW])
    mark = None
    while len(song.stream) < 40000:
        song.inc(int(rng.integers(0, 6)), int(rng.integers(0, 1 << 21)))
        song.tick(int(rng.integers(0, 2)))
        if mark is None and len(song.stream) > 20000:
            mark = song.mark()
    song.loop_to(mark)
    banks = gts.song_banks(song)
    assert all(len(b) == 0x4000 for b in banks)
    b0 = banks[0]
    assert b0[:4] == b'GTS1' and b0[4] == 0xD0 and b0[5] == 2
    n, f = struct.unpack_from('<HB', b0, 6)
    assert abs(n + f / 256 - RATE / 60) < 1 / 256, (n, f)
    assert list(b0[16:16 + 64]) == TRI + SAW
    stream = b''
    for b in banks[1:-1]:
        length = struct.unpack_from('<H', b, 0)[0]
        assert 0 < length <= 0x3FFE
        stream += b[2:2 + length]
    assert stream == bytes(song.stream)
    assert struct.unpack_from('<H', banks[-1], 0)[0] == 0, 'a bank of length 0 ends the stream'
    loop_bank, loop_addr = struct.unpack_from('<BH', b0, 9)
    assert loop_bank == 2 and banks[loop_bank][loop_addr - 0x8000:][:16] == bytes(song.stream[mark:mark + 16])

    with open(os.path.join(tmp, 'rom.bin'), 'wb') as fh:
        fh.write(b'\xAA' * (128 * 0x4000))
    gts.patch_rom(os.path.join(tmp, 'rom.bin'), os.path.join(tmp, 'out.bin'), song)
    rom = open(os.path.join(tmp, 'out.bin'), 'rb').read()
    assert rom[:len(banks) * 0x4000] == b''.join(banks) and set(rom[len(banks) * 0x4000:]) == {0xAA}


def t_model(tmp):
    """render(): table arithmetic, tick lengths, pitch"""
    # gain 256 = one DAC count per wave step, wave value v stands for v - 15.5, rounded half up
    assert gts.render_table(SAW, 256) == [v - 16 if v < 16 else v - 15 for v in SAW]
    assert gts.render_table(SAW, 0) == [0] * 32
    assert gts.render_table(SAW, 512)[31] == 31 and gts.render_table(SAW, 512)[0] == -31
    assert max(gts.render_table(SAW, gts.MAX_GAIN)) <= 127
    assert gts.render_table(SAW, 300, noise=True)[:2] == [-18, 18]      # 15.5 * 300 / 256 = 18.16

    song = gts.Song(rate_reg=RATE_REG, waves=[[0] * 16 + [31] * 16])
    song.inc(0, song.hz_to_inc(1000)); song.wave(0, 0); song.gain(0, 256)
    song.tick(599)
    song.end()
    out = gts.render(song).astype(int) - 128
    assert abs(len(out) - 10 * RATE) <= 1, (len(out), 10 * RATE)         # 600 ticks are 10 seconds
    assert set(out.tolist()) == {-16, 16}
    rising = np.count_nonzero((out[1:] > 0) & (out[:-1] < 0))
    assert abs(rising - 10000) <= 1, rising                              # 1000 Hz for 10 seconds

    # waits: FE n covers n + 1 ticks, and long ones are split correctly
    for extra in (0, 1, 255, 256, 257, 600):
        s = gts.Song(waves=[SAW])
        s.tick(extra)
        s.end()
        assert len(gts.parse_ticks(s.stream)) == extra + 1, extra


TESTS = [t_note, t_unknown_wave_rotations, t_mono_gain, t_silence_is_cheap, t_noise, t_loop, t_rom_layout, t_model]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-k', default='', help='only tests whose name contains this')
    a = ap.parse_args()
    failed = 0
    for t in TESTS:
        name = t.__name__[2:]
        if a.k not in name:
            continue
        with tempfile.TemporaryDirectory() as tmp:
            try:
                t(tmp)
                print('ok    %s' % name)
            except AssertionError as e:
                failed += 1
                print('FAIL  %-24s %s' % (name, e))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
