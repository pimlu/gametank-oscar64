#!/usr/bin/env python3
"""Convert a Furnace PC Engine song to GTS, and optionally put it into a GameTank ROM.
(../build_with_song.sh does this after building a game.)

    vgm2gts.py song.fur --rom gametank.bin      put it into a ROM that ./build.sh made
    vgm2gts.py song.vgm --wav preview.wav       what the GameTank will play, exactly

A .fur needs the furnace command line on the PATH; a .vgm (Furnace: file > export > VGM) doesn't.

The input is the HuC6280 register log, so every Furnace effect and macro is already applied; we
only translate what the chip was told to do each tick into what our synth needs to be told:
period -> phase increment, volume/pan/master -> one mono gain, wave RAM uploads -> wave index,
noise frequency -> LFSR rate. Not supported (warned about, silent): DDA/PCM samples, the LFO.
"""
import argparse
import collections
import gzip
import os
import re
import struct
import subprocess
import sys
import tempfile
import wave

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gts  # noqa: E402

PCE_CLOCK = 3579545.0
VGM_RATE = 44100


def parse_vgm(path):
    """-> ([(time in 1/44100 s, reg, value)], total time, tick rate, loop time or None) for the
    HuC6280"""
    d = open(path, 'rb').read()
    if d[:2] == b'\x1f\x8b':      # .vgz
        d = gzip.decompress(d)
    if d[:4] != b'Vgm ':
        sys.exit('%s: not a vgm file' % os.path.basename(path))
    if len(d) < 0xA8 or struct.unpack_from('<I', d, 0xA4)[0] == 0:
        sys.exit('%s: this is not a PC Engine / TurboGrafx-16 song (no HuC6280 in the vgm). '
                 'Only that chip is supported.' % os.path.basename(path))
    tick_hz = struct.unpack_from('<I', d, 0x24)[0] or 60
    p = 0x34 + struct.unpack_from('<I', d, 0x34)[0]
    loop_pos = struct.unpack_from('<I', d, 0x1C)[0]
    loop_pos = loop_pos + 0x1C if loop_pos else None
    loop_time = None
    t = 0
    out = []
    while p < len(d):
        if p == loop_pos:
            loop_time = t
        c = d[p]
        if c == 0xB9:
            out.append((t, d[p + 1], d[p + 2])); p += 3
        elif c == 0x61:
            t += struct.unpack_from('<H', d, p + 1)[0]; p += 3
        elif c == 0x62:
            t += 735; p += 1
        elif c == 0x63:
            t += 882; p += 1
        elif c == 0x66:
            break
        elif 0x70 <= c <= 0x7F:
            t += (c & 15) + 1; p += 1
        elif c in (0x90, 0x91, 0x95):
            p += 5
        elif c == 0x92:
            p += 6
        elif c == 0x93:
            p += 11
        elif c == 0x94:
            p += 2
        elif c == 0x67:
            p += 7 + struct.unpack_from('<I', d, p + 3)[0]
        else:
            sys.exit('%s: unknown vgm command %02x at %d' % (path, c, p))
    return out, t, tick_hz, loop_time


def db_gain(steps):
    """HuC6280: 1.5 dB per attenuation step, 31 steps or more is silence"""
    return 0.0 if steps >= 0x1F else 10 ** (-1.5 * steps / 20)


class Chan:
    def __init__(self):
        self.period = 0
        self.ctl = 0
        self.pan = 0xFF
        self.wave = [0] * 32
        self.widx = 0
        self.noise = 0
        self.restarted = False  # this tick: the wave position was reset to 0 (happens on upload)
        self.bumps = 0          # this tick: off->on transitions, each advances the wave by a sample


# What one PCE channel is doing during a tick.
#   audible:   enabled, not DDA, volume above silence
#   gain:      mono gain 0..1 (average of the left and right outputs)
#   noise:     noise frequency register value, or None if it plays its wave
#   running:   the wave position advances (it is frozen while the channel is disabled)
#   restarted, bumps: see Chan
State = collections.namedtuple('State', 'audible gain period wave noise running restarted bumps')


def chip_states(writes, total, tick_hz):
    """Replay the register writes. -> per tick, per channel, a State"""
    chans = [Chan() for _ in range(6)]
    master = 0xFF
    sel = 0
    warned = set()
    per_tick = VGM_RATE / tick_hz
    n_ticks = int(round(total / per_tick))
    states = []

    def snapshot():
        st = []
        for i, ch in enumerate(chans):
            on = bool(ch.ctl & 0x80)
            if on and (ch.ctl & 0x40):
                if 'dda' not in warned:
                    warned.add('dda'); print('warning: DDA (PCM sample) playback is not supported, those notes are dropped')
                on = False
            g = 0.0
            for side in (4, 0):
                att = (0x1F - (ch.ctl & 0x1F)) + 2 * (0xF - ((ch.pan >> side) & 0xF)) + 2 * (0xF - ((master >> side) & 0xF))
                g += db_gain(att) / 2
            noise = ch.noise & 0x1F if (i >= 4 and ch.noise & 0x80) else None
            st.append(State(audible=on and g > 0, gain=g, period=ch.period, wave=tuple(ch.wave), noise=noise,
                            running=on, restarted=ch.restarted, bumps=ch.bumps))
            ch.restarted = False
            ch.bumps = 0
        return st

    wi = 0
    for tick in range(n_ticks):
        # writes that happen at the start of this tick
        limit = (tick + 0.5) * per_tick
        while wi < len(writes) and writes[wi][0] < limit:
            _, reg, v = writes[wi]; wi += 1
            if reg == 0:
                sel = v & 7
            elif reg == 1:
                master = v
            elif reg == 9:
                if v & 3 and 'lfo' not in warned:
                    warned.add('lfo'); print('warning: the hardware LFO is not supported')
            elif sel < 6 and reg in (2, 3, 4, 5, 6, 7):
                ch = chans[sel]
                if reg == 2:
                    ch.period = (ch.period & 0xF00) | v
                elif reg == 3:
                    ch.period = (ch.period & 0xFF) | ((v & 0xF) << 8)
                elif reg == 4:
                    if (ch.ctl & 0x40) and not (v & 0x40):
                        ch.widx = 0
                        ch.restarted = True
                        ch.bumps = 0
                    if not (ch.ctl & 0x80) and (v & 0x80) and not (v & 0x40):
                        ch.bumps += 1
                    ch.ctl = v
                elif reg == 5:
                    ch.pan = v
                elif reg == 6:
                    if not (ch.ctl & 0x40) and not (ch.ctl & 0x80):
                        ch.wave[ch.widx] = v & 0x1F
                        ch.widx = (ch.widx + 1) & 31
                elif reg == 7:
                    ch.noise = v
        states.append(snapshot())
    return states


class WaveBook:
    """Furnace uploads waves rotated ("anti-click": a PCE wave upload restarts the wave, so
    Furnace rotates the data by where playback would have been, to make it carry on seamlessly).
    Our synth swaps tables without touching the phase, which is what that trick is after, so we
    want the unrotated wave: look it up among the song's wavetables. For a wave that isn't one
    of those (wave synth, vgm input) the first rotation we see stands for all of them; the only
    effect is a phase offset.

    The rotation also tells us where in the wave the PCE is right after the upload. Furnace
    resets its prediction to 0 on a new note, i.e. notes restart the wave, which matters when
    channels play in unison or a kick lands on a bass note. build() follows the PCE's wave
    position and resyncs our oscillator (SET_PHASE) when they disagree."""

    def __init__(self, known=()):
        self.canonical = {}
        for w in known:
            self.add(tuple(w))

    def add(self, w):
        for r in range(32):
            self.canonical.setdefault(w[r:] + w[:r], (w, r))

    def lookup(self, uploaded):
        """-> (wave, rotations): uploaded[i] == wave[(i + r) & 31] for each r in rotations"""
        if uploaded not in self.canonical:
            self.add(uploaded)
        w, r = self.canonical[uploaded]
        period = next(p for p in (1, 2, 4, 8, 16, 32) if w[p:] + w[:p] == w)
        return w, [(r + k * period) & 31 for k in range(32 // period)]


def build(states, rate_reg, tick_hz, amp, known_waves=(), loop_tick=None):
    """amp: DAC counts per wave step for a channel at full volume.
    loop_tick: play forever, going back to this tick after the last one."""
    song = gts.Song(rate_reg=rate_reg, tick_hz=tick_hz)
    rate = gts.sample_rate(rate_reg)
    wave_index = {}
    book = WaveBook(known_waves)
    last = [dict(inc=0, gain=0, wave=None, noise=0) for _ in range(6)]
    pending_ticks = 0
    first = True
    # Wave positions (as 5.16 phases) at the start of each tick: our oscillators', tracked like
    # gts.render does, and the PCE's, following pce_psg.cpp: frozen while the channel is
    # disabled, running during noise, back to 0 when an upload clears DDA, one step forward
    # whenever the channel gets enabled. While a note sounds both move by the same increment,
    # so they only part ways at such events, or while we weren't listening: we don't send pitch
    # changes to a silent channel, and noise mode scribbles over our oscillator's index.
    tick_n, tick_f = gts.tick_params(rate_reg, tick_hz)
    frac = 0
    phase = [0] * 6
    phase_known = [True] * 6
    # The PCE's position has to stay right for minutes (drums never re-upload their wave), so
    # no rounded increments here: count master clock cycles like the chip does. A wave step
    # happens when the down counter runs out, and it is reloaded with the period of that moment.
    pce_idx = [0] * 6
    pce_counter = [4096.0] * 6
    clocks_per_tick = PCE_CLOCK / tick_hz

    def to_inc(period):
        return min((1 << 21) - 1, int(round(PCE_CLOCK / (period or 4096) / rate * 65536)))

    def distance(a, b):
        return min((a - b) & 31, (b - a) & 31)

    def flush():
        nonlocal pending_ticks
        if pending_ticks:
            song.tick(pending_ticks - 1)
            pending_ticks = 0

    if loop_tick is not None and not 0 <= loop_tick < len(states) - 1:
        loop_tick = None
    loop_state = None

    for tick_index, st in enumerate(states):
        cmds = []
        resync = [None] * 6
        frac += tick_f
        n = tick_n + (frac >> 8)
        frac &= 0xFF
        for ch, c in enumerate(st):
            l = last[ch]
            if c.restarted:
                # sample 0 of what was uploaded is sample r of the unrotated wave
                _, rotations = book.lookup(c.wave)
                pce_idx[ch] = min(rotations, key=lambda r: distance(r, phase[ch] >> 16))
                pce_counter[ch] = float(c.period or 4096)
            pce_idx[ch] = (pce_idx[ch] + c.bumps) & 31

            g = c.gain
            if c.noise is not None:
                # We can step the LFSR once per sample at most. When the PCE clocks its noise
                # faster than that, most of the energy is above our Nyquist frequency (and above
                # anyone's hearing); ours would all end up below it. Same power in the band:
                noise_hz = PCE_CLOCK / (64 * ((c.noise ^ 0x1F) or 0.5))
                g *= min(1.0, rate / noise_hz) ** 0.5
            gain = min(gts.MAX_GAIN, int(round(g * amp * 256))) if c.audible else 0
            if gain == 0:
                # nothing else matters while it is silent; catch up when it comes back
                if l['gain'] != 0:
                    cmds.append(('gain', ch, 0)); l['gain'] = 0
                continue
            if c.noise is not None:
                ninc = max(1, min(65535, int(round(noise_hz / rate * 65536))))
                phase_known[ch] = False
            else:
                ninc = 0
                inc = to_inc(c.period)
                wav, _ = book.lookup(c.wave)
                # more than a sample apart (Furnace's anti-click prediction is that precise)?
                if not phase_known[ch] or distance(pce_idx[ch], phase[ch] >> 16) > 1:
                    resync[ch] = pce_idx[ch]
                    cmds.append(('phase', ch, resync[ch]))
                    phase_known[ch] = True
                if wav not in wave_index:
                    wave_index[wav] = len(song.waves); song.waves.append(list(wav))
                if l['wave'] != wave_index[wav]:
                    cmds.append(('wave', ch, wave_index[wav])); l['wave'] = wave_index[wav]
                if l['inc'] != inc:
                    cmds.append(('inc', ch, inc)); l['inc'] = inc
            if l['noise'] != ninc:
                cmds.append(('noise', ch, ninc)); l['noise'] = ninc
            if l['gain'] != gain:
                cmds.append(('gain', ch, gain)); l['gain'] = gain
        last_before = [dict(l) for l in last]
        for ch, c in enumerate(st):
            # ours keeps running at its last increment, also when silent
            if resync[ch] is None:
                phase[ch] = (phase[ch] + last[ch]['inc'] * n) & 0x1FFFFF
            else:
                phase[ch] = ((resync[ch] << 16) + last[ch]['inc'] * (n - 1)) & 0x1FFFFF
            if c.running:
                period = float(c.period or 4096)
                clocks = clocks_per_tick
                if clocks < pce_counter[ch]:
                    pce_counter[ch] -= clocks
                else:
                    clocks -= pce_counter[ch]
                    pce_idx[ch] = (pce_idx[ch] + 1 + int(clocks // period)) & 31
                    pce_counter[ch] = period - clocks % period
        phase_before = list(phase)
        if loop_tick is not None and tick_index == loop_tick + 1:
            # The second time round starts here, after the tick that replaces the loop tick
            # (below), so a tick has to start at this byte.
            flush()
            song.loop_offset = song.mark()
            loop_state = ([dict(l) for l in last_before], list(phase_before))
        if cmds or first or song.loop_offset == len(song.stream):
            flush()
            first = False
            for name, ch, v in cmds:
                getattr(song, name)(ch, v)
        pending_ticks += 1
    flush()
    if loop_state is None:
        song.end()
    else:
        # One more tick, standing in for the loop tick: whatever it takes to get from how the
        # song ends to how things were after the loop tick the first time. From there on the
        # stream just repeats. (Phases only to the wave sample; and the PCE wouldn't reset
        # them, but this keeps every round identical.)
        then, then_phase = loop_state
        for ch in range(6):
            now = last[ch]
            if then[ch]['wave'] is not None and then[ch]['wave'] != now['wave']:
                song.wave(ch, then[ch]['wave'])
            if then[ch]['inc'] != now['inc']:
                song.inc(ch, then[ch]['inc'])
            if then[ch]['noise'] != now['noise']:
                song.noise(ch, then[ch]['noise'])
            if then[ch]['gain'] != now['gain']:
                song.gain(ch, then[ch]['gain'])
            song.phase(ch, then_phase[ch] >> 16)
        song.tick()
    if not song.waves:
        song.waves.append([0] * 32)
    return song


def convert(path, rate_reg, known_waves=(), loop=True, headroom=124, verbose=True):
    """loop: go back to the song's loop point at the end (to the start if it has none)"""
    writes, total, tick_hz, loop_time = parse_vgm(path)
    states = chip_states(writes, total, tick_hz)
    loop_tick = int(round((loop_time or 0) * tick_hz / VGM_RATE)) if loop else None
    # pick the loudest gain that never overflows the 8 bit sum: measure at amp 1, scale, verify
    probe = build(states, rate_reg, tick_hz, 1.0, known_waves, loop_tick)
    peak = max(1, int(np.abs(gts.render(probe, wide=True, loops=1)).max()))
    amp = min(8.0, headroom / peak)
    while True:
        song = build(states, rate_reg, tick_hz, amp, known_waves, loop_tick)
        mix = gts.render(song, wide=True, loops=1)
        if mix.max() <= 127 and mix.min() >= -128:
            break
        amp *= 0.98
    if verbose:
        n_ticks = len(gts.parse_ticks(song.stream, song.loop_offset))
        print('%s: %d ticks at %g Hz (%.1f s), %d waves, %d stream bytes (%.1f per tick), '
              'amp %.2f -> peak %d..%d, %.0f Hz, %s'
              % (os.path.basename(path), n_ticks, tick_hz, n_ticks / tick_hz, len(song.waves), len(song.stream),
                 len(song.stream) / max(1, n_ticks), amp, mix.min(), mix.max(), gts.sample_rate(rate_reg),
                 'plays once' if song.loop_offset is None else 'loops to tick %d' % loop_tick))
    return song


def run_furnace(args, path, out):
    try:
        subprocess.run(['furnace', '-loglevel', 'error'] + args + [out, path],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except FileNotFoundError:
        sys.exit("'furnace' is not on the PATH. Add it, or export the song as .vgm from Furnace "
                 "(file > export > VGM) and pass that instead.")
    if not os.path.exists(out) or not os.path.getsize(out):
        sys.exit('furnace could not read %s' % path)


def fur_wavetables(path, workdir):
    """the 32x32 wavetables of a .fur, via furnace's text export"""
    txt = os.path.join(workdir, 'song.txt')
    run_furnace(['-txtout'], path, txt)
    waves = []
    for line in open(txt, errors='replace'):
        m = re.match(r'- \d+ \(32x32\): ([\d ]+)$', line.strip())
        if m:
            waves.append([int(v) & 31 for v in m.group(1).split()[:32]])
    return waves


def fur_to_vgm(path, workdir):
    out = os.path.join(workdir, os.path.splitext(os.path.basename(path))[0] + '.vgm')
    run_furnace(['-vgmout'], path, out)
    return out


def write_wav(path, dac, rate):
    with wave.open(path, 'wb') as w:
        w.setnchannels(1); w.setsampwidth(1); w.setframerate(int(round(rate)))
        w.writeframes(np.asarray(dac, dtype=np.uint8).tobytes())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('song', help='.fur (needs furnace on PATH) or .vgm')
    ap.add_argument('--rate', default='d0', help='ACP rate register, hex: d0 = 22233 Hz (default), ff = 13983 Hz')
    ap.add_argument('--rom', help='GameTank ROM (built with ./build.sh furnace) to put the song into, in place')
    ap.add_argument('--wav', help='write the exact DAC output to this wav (a looping song: two rounds)')
    ap.add_argument('--once', action='store_true', help="don't loop: stop after the last tick")
    a = ap.parse_args()

    with tempfile.TemporaryDirectory() as tmp:
        is_fur = a.song.lower().endswith('.fur')
        vgm = fur_to_vgm(a.song, tmp) if is_fur else a.song
        song = convert(vgm, int(a.rate, 16), fur_wavetables(a.song, tmp) if is_fur else (), loop=not a.once)
    if a.wav:
        write_wav(a.wav, gts.render(song, tail_ticks=30, loops=1), gts.sample_rate(song.rate_reg))
    if a.rom:
        gts.patch_rom(a.rom, a.rom, song)
        print('%s: song in ROM banks 0..%d' % (a.rom, len(gts.song_banks(song)) - 1))


if __name__ == '__main__':
    main()
