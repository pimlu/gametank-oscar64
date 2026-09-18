#!/usr/bin/env python3
"""Fidelity test: does the GTS version of a song sound like Furnace's own render of it?

Compares, for every channel on its own and for the full mix, the exact model of the GameTank
output (music/gts.py; test_firmware.py and check_song.py prove the firmware equals it) with
Furnace's render (-outmode perchan), mixed down to mono and resampled to the GameTank's rate.

The two can't match sample for sample (mono, 8 bit, no band-limiting, free-running phase, our
own noise generator), so the measures are phase-blind:

  level    RMS level difference in dB, after undoing the converter's overall gain
  envelope correlation of the RMS envelopes (one value per tick): notes, volume macros, timing
  spectrum correlation of the log-magnitude spectrograms below 5 kHz: pitch, waveforms

Needs the furnace command line and scipy; no emulator.

usage: compare_furnace.py song.fur [--rate d0]        exit status 1 if a bound is exceeded
"""
import argparse
import glob
import os
import subprocess
import sys
import tempfile

import shutil

import numpy as np

SKIPPED = 77
try:
    from scipy import signal
    from scipy.io import wavfile
except ImportError:
    print('skipped: needs scipy (pip install scipy)')
    sys.exit(SKIPPED)
if not shutil.which('furnace'):
    print("skipped: needs the furnace command line on the PATH, to render the reference")
    sys.exit(SKIPPED)

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import gts  # noqa: E402
import vgm2gts  # noqa: E402

# Regression bounds: (max |level| dB, min envelope, min spectrum), somewhat below what the six PCE
# demo songs of furnace reach. Known differences that they leave room for:
#  - The PCE pops when a loud note is cut: its waves are unsigned, so the output level jumps.
#    Ours are centered and don't. Costs a channel with many hard note-offs ~0.02 of envelope.
#  - Noise is a different random sequence, so spectrograms of the noise-capable channels (5, 6)
#    only agree in shape.
#  - The mix also depends on how the channels' phases relate (a kick adds to or cancels the bass
#    note under it). The converter follows the PCE's wave positions to within about a wave
#    sample, which is good for bass notes and meaningless for a 2 kHz hi-hat.
TONE_BOUNDS = (0.5, 0.97, 0.90)
NOISE_BOUNDS = (0.5, 0.97, 0.70)
MIX_BOUNDS = (0.5, 0.92, 0.80)


def measures(ref, got, rate, tick_hz):
    win = int(rate / tick_hz)
    k = min(len(ref), len(got)) // win

    def env(x):
        return np.sqrt((x[:k * win].reshape(k, win) ** 2).mean(axis=1))

    def spec(x):
        f, _, z = signal.stft(x, fs=rate, nperseg=2048, noverlap=1024)
        z = np.abs(z[f < 5000])
        return np.log10(z + 1e-2 * z.max())      # 40 dB of range: above our quantization noise

    level = 20 * np.log10(got.std() / ref.std())
    return level, np.corrcoef(env(ref), env(got))[0, 1], np.corrcoef(spec(ref).ravel(), spec(got).ravel())[0, 1]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('song')
    ap.add_argument('--rate', default='d0')
    a = ap.parse_args()

    with tempfile.TemporaryDirectory() as tmp:
        vgm = vgm2gts.fur_to_vgm(a.song, tmp)
        # one pass, like furnace -loops 0
        song = vgm2gts.convert(vgm, int(a.rate, 16), vgm2gts.fur_wavetables(a.song, tmp), loop=False)
        subprocess.run(['furnace', '-loglevel', 'error', '-loops', '0', '-outmode', 'perchan',
                        '-output', os.path.join(tmp, 'ref.wav'), a.song], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        refs = []
        for path in sorted(glob.glob(os.path.join(tmp, 'ref_c*.wav'))):
            fs, x = wavfile.read(path)
            refs.append(x.astype(np.float64).mean(axis=1))
    rate = gts.sample_rate(song.rate_reg)
    # resample furnace's 44100 Hz to our rate (band-limited)
    up, down = int(round(rate)), fs
    refs = [signal.resample_poly(x, up, down) for x in refs]

    # The PCE's waves are unsigned, so its output has a DC level that moves with the volume;
    # ours are centered. Inaudible, but it would dominate levels and RMS envelopes.
    hp = signal.butter(2, 30, 'highpass', fs=rate, output='sos')
    n = min(len(refs[0]), len(gts.render(song, wide=True)))

    def prepare(x):
        return signal.sosfilt(hp, np.asarray(x[:n], dtype=np.float64))

    failed = False
    rows = [('ch %d' % (c + 1), prepare(refs[c]), prepare(gts.render(song, channels=[c], wide=True))) for c in range(6)]
    rows.append(('mix', sum(r[1] for r in rows), sum(r[2] for r in rows)))
    # one overall gain relates the two scales (furnace's output level vs the converter's amp)
    scale = rows[-1][2].std() / rows[-1][1].std()
    print('%-5s %9s %9s %9s' % ('', 'level dB', 'envelope', 'spectrum'))
    for name, ref, got in rows:
        if ref.std() < 1e-9:
            print('%-5s (silent)' % name)
            continue
        level, envelope, spectrum = measures(ref * scale, got, rate, song.tick_hz)
        max_level, min_envelope, min_spectrum = MIX_BOUNDS if name == 'mix' else NOISE_BOUNDS if name in ('ch 5', 'ch 6') else TONE_BOUNDS
        ok = abs(level) <= max_level and envelope >= min_envelope and spectrum >= min_spectrum
        failed |= not ok
        print('%-5s %+9.2f %9.4f %9.4f  %s' % (name, level, envelope, spectrum, 'ok' if ok else 'FAIL'))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
