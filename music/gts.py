"""GTS: the GameTank tracker stream format, and a bit-exact model of the ACP firmware that plays it.

This file is the spec. The firmware (../src/audio/acp_fw.c) has to produce exactly the DAC bytes
that `render()` returns; the tests in the gametank-tracker repo check that sample for sample.

The synth
---------
Six channels, modelled on the PC Engine PSG. Per channel and per output sample:

    phase = (phase + inc) & 0x1FFFFF       # 5.16 fixed point, in wave samples
    out   = table[phase >> 16]             # 32 signed bytes, volume already baked in

`table` is rebuilt from a 32-sample 5-bit wave and a 16-bit gain whenever either changes (see
`render_table`). Channels 4 and 5 can switch to noise: a 16-bit LFSR that is stepped whenever a
16-bit accumulator overflows, giving +-table amplitude. The DAC gets (sum of the six outs) + 128,
wrapping, so the converter has to pick gains that keep the sum inside -128..127.

Time
----
Music time is counted in ticks by the ACP itself: `tick_n` + `tick_f`/256 samples per tick. All
changes of one tick take effect atomically on the first sample of that tick.

The stream
----------
A byte stream that the main CPU copies blindly from ROM into a ring buffer:

    00+ch  i0 i1 i2   SET_INC    phase increment, 24 bit little endian
    08+ch  g0 g1      SET_GAIN   256 = one DAC count per wave step (wave 0..31 -> -15.5..+15.5)
    10+ch  w          SET_WAVE   index into the wave table
    18+ch  n0 n1      SET_NOISE  ch 4/5 only. LFSR steps per sample * 65536; 0 = back to wavetable
    20+ch  p          SET_PHASE  restart the wave at sample p (0..31). Unlike everything else this
                                 takes effect on the *second* sample of the tick: the firmware
                                 can't afford it in the same IRQ as the parameter switch.
    FD                end of song: silence, stop. A looping song doesn't have it: the main CPU
                      carries on copying from the loop point when it reaches the end of the
                      stream. The firmware never notices; it is the converter's job to make
                      the state at the end fit what follows the loop point.
    FE n              end of tick, followed by n more ticks in which nothing changes
    FF                end of tick
"""
import struct
from dataclasses import dataclass, field

import numpy as np

NCH = 6
CMD_INC, CMD_GAIN, CMD_WAVE, CMD_NOISE, CMD_PHASE = 0x00, 0x08, 0x10, 0x18, 0x20
CMD_END, CMD_WAIT, CMD_TICK = 0xFD, 0xFE, 0xFF

MAIN_CLOCK = 315e6 / 88          # Hz; the ACP runs at 4x this

LFSR_SEED = 0x0001
LFSR_TAPS = 0xB400               # 16 bit Galois LFSR, period 65535


def timer_period(rate_reg):
    """main-CPU cycles per sample for a value of the $2006 rate register"""
    v = rate_reg & 0x7F
    return (((v << 1) & 0xFE) | (v & 1)) + 1


def sample_rate(rate_reg):
    return MAIN_CLOCK / timer_period(rate_reg)


def tick_params(rate_reg, tick_hz=60.0):
    """-> (tick_n, tick_f): samples per tick as integer + 1/256ths"""
    spt = sample_rate(rate_reg) / tick_hz
    n = int(spt)
    f = int(round((spt - n) * 256))
    if f == 256:
        n, f = n + 1, 0
    # the firmware's 16 bit down counter can't represent a low byte of 0 (+ the carry of 1)
    assert 2 <= n < 65536 and (n & 0xFF) != 0, n
    return n, f


@dataclass
class Song:
    rate_reg: int = 0xD0
    tick_hz: float = 60.0
    waves: list = field(default_factory=list)     # each: 32 ints 0..31
    stream: bytearray = field(default_factory=bytearray)
    loop_offset: int = None      # stream offset (of the start of a tick) to continue from at the end

    # --- building ---
    def inc(self, ch, inc):
        assert 0 <= inc < (1 << 24)
        self.stream += bytes([CMD_INC + ch, inc & 255, (inc >> 8) & 255, inc >> 16])

    def gain(self, ch, g):
        assert 0 <= g <= MAX_GAIN
        self.stream += bytes([CMD_GAIN + ch, g & 255, g >> 8])

    def wave(self, ch, w):
        assert 0 <= w < len(self.waves)
        self.stream += bytes([CMD_WAVE + ch, w])

    def noise(self, ch, n):
        assert ch in (4, 5) and 0 <= n < 65536
        self.stream += bytes([CMD_NOISE + ch, n & 255, n >> 8])

    def phase(self, ch, p):
        assert 0 <= p < 32
        self.stream += bytes([CMD_PHASE + ch, p])

    def tick(self, extra=0):
        """end the current tick; `extra` more ticks pass without changes"""
        while extra > 255:
            self.stream += bytes([CMD_WAIT, 255])
            extra -= 256      # FE 255 = this tick + 255 more; the next FE/FF ends an empty tick
        self.stream += bytes([CMD_WAIT, extra]) if extra else bytes([CMD_TICK])

    def end(self):
        self.stream += bytes([CMD_END])

    def loop_to(self, offset):
        """end the song by looping back to `offset`, a value that mark() returned earlier"""
        assert 0 <= offset < len(self.stream)
        self.loop_offset = offset

    def mark(self):
        """stream offset of the tick that starts here (call right after tick())"""
        return len(self.stream)

    def hz_to_inc(self, hz):
        return int(round(hz * 32 * 65536 / sample_rate(self.rate_reg)))


# row[15] (the largest table entry) has to fit a signed byte: (15.5 * g + 128) >> 8 <= 127
MAX_GAIN = 2080


def render_table(wave, gain, noise=False):
    """The 32-entry signed table for a wave at a gain. Mirrors the firmware's arithmetic:
    row[m] = round_half_up((m + 0.5) * gain / 256), accumulated in 16 bits."""
    row = []
    acc = gain >> 1
    for _ in range(16):
        row.append(((acc + 0x80) >> 8) & 0xFF)
        acc = (acc + gain) & 0xFFFF
    if noise:
        return [-row[15], row[15]] + [0] * 30
    return [row[v - 16] if v >= 16 else -row[15 - v] for v in wave]


_lfsr_bits = None


def lfsr_bits():
    """output bit (bit 0 of the state) after n steps, n = 0..65534"""
    global _lfsr_bits
    if _lfsr_bits is None:
        out = np.empty(65535, dtype=np.int64)
        s = LFSR_SEED
        for i in range(65535):
            out[i] = s & 1
            c = s & 1
            s >>= 1
            if c:
                s ^= LFSR_TAPS
        assert s == LFSR_SEED
        _lfsr_bits = out
    return _lfsr_bits


def parse_ticks(stream, loop_offset=None, loops=0):
    """-> list of ticks, each a list of (cmd_type, ch, value).
    Without loop_offset the stream has to end with CMD_END. With it, it must not, and the result
    is the whole stream followed by `loops` times the part from loop_offset on."""
    def parse(p):
        ticks, cur = [], []
        while p < len(stream):
            c = stream[p]
            if c == CMD_END:
                assert not cur, 'commands after the last end-of-tick'
                return ticks, True
            if c == CMD_TICK:
                ticks.append(cur); cur = []; p += 1
            elif c == CMD_WAIT:
                ticks.append(cur); cur = []
                ticks.extend([] for _ in range(stream[p + 1])); p += 2
            else:
                typ, ch = c & 0xF8, c & 7
                assert ch < NCH, hex(c)
                if typ == CMD_INC:
                    cur.append((typ, ch, stream[p + 1] | stream[p + 2] << 8 | stream[p + 3] << 16)); p += 4
                elif typ in (CMD_GAIN, CMD_NOISE):
                    cur.append((typ, ch, stream[p + 1] | stream[p + 2] << 8)); p += 3
                elif typ in (CMD_WAVE, CMD_PHASE):
                    cur.append((typ, ch, stream[p + 1])); p += 2
                else:
                    raise ValueError('bad command %02x at %d' % (c, p))
        assert not cur, 'stream ends in the middle of a tick'
        return ticks, False

    ticks, ended = parse(0)
    if loop_offset is None:
        assert ended, 'stream has neither CMD_END nor a loop'
        return ticks
    assert not ended, 'a looping stream must not contain CMD_END'
    return ticks + parse(loop_offset)[0] * loops


def render(song, channels=range(NCH), wide=False, tail_ticks=0, loops=0):
    """-> DAC bytes (uint8 array), starting with the first sample of tick 0.
    wide=True returns the unwrapped signed sum instead (to check headroom).
    After the last tick of a song that doesn't loop the firmware is silent; tail_ticks adds that
    many ticks of it. loops: how many times to go round a looping song after the first pass."""
    ticks = parse_ticks(song.stream, song.loop_offset, loops)
    if song.loop_offset is not None:
        tail_ticks = 0
    tick_n, tick_f = tick_params(song.rate_reg, song.tick_hz)
    bits = lfsr_bits()

    inc = [0] * NCH; gain = [0] * NCH; wav = [0] * NCH; ninc = [0] * NCH
    phase = [0] * NCH; nacc = [0] * NCH; nsteps = [0] * NCH
    # what the IRQ handler is currently playing; starts as silence
    table = [np.zeros(32, dtype=np.int64) for _ in range(NCH)]
    nmode = [False] * NCH

    out = []
    frac = 0
    for cmds in ticks + [None] * tail_ticks:
        setphase = {}
        if cmds is None:
            table = [np.zeros(32, dtype=np.int64) for _ in range(NCH)]
        else:
            dirty = set()
            for typ, ch, v in cmds:
                if typ == CMD_INC:
                    inc[ch] = v
                elif typ == CMD_GAIN:
                    gain[ch] = v; dirty.add(ch)
                elif typ == CMD_WAVE:
                    wav[ch] = v; dirty.add(ch)
                elif typ == CMD_NOISE:
                    ninc[ch] = v; dirty.add(ch)
                elif typ == CMD_PHASE:
                    setphase[ch] = v
            for ch in dirty:
                nmode[ch] = ninc[ch] != 0
                table[ch] = np.array(render_table(song.waves[wav[ch]], gain[ch], nmode[ch]), dtype=np.int64)
        frac += tick_f
        n = tick_n + (frac >> 8)
        frac &= 0xFF

        k = np.arange(1, n + 1, dtype=np.int64)
        mix = np.zeros(n, dtype=np.int64)
        for ch in range(NCH):
            if nmode[ch]:
                acc = nacc[ch] + ninc[ch] * k
                steps = nsteps[ch] + (acc >> 16)
                nacc[ch] = int(acc[-1] & 0xFFFF); nsteps[ch] = int(steps[-1] % 65535)
                idx = bits[steps % 65535]
                # the firmware keeps the table index in the top phase byte, noise or not
                phase[ch] = (phase[ch] & 0xFFFF) | (int(idx[-1]) << 16)
                if ch in setphase:
                    # lands on the second sample, the noise generator overwrites the index again
                    phase[ch] = int(idx[-1]) << 16
            else:
                ph = (phase[ch] + inc[ch] * k) & 0x1FFFFF
                if ch in setphase:
                    # first sample as usual, then restart from the new phase
                    ph[1:] = ((setphase[ch] << 16) + inc[ch] * k[:-1]) & 0x1FFFFF
                phase[ch] = int(ph[-1])
                idx = ph >> 16
            if ch in channels:
                mix += table[ch][idx]
        out.append(mix)
    mix = np.concatenate(out) if out else np.zeros(0, dtype=np.int64)
    if wide:
        return mix
    return ((mix + 128) & 0xFF).astype(np.uint8)


# --- ROM image -------------------------------------------------------------------------------
# Bank 0 at $8000: header, then the waves. Stream in banks 1.., each bank: u16 length, data;
# length 0 (or an erased bank: $FFFF) ends it. The main CPU only ever looks at these lengths,
# never inside the stream. Header:
#   0 'GTS1'   4 rate register   5 number of waves   6 tick_n (u16)   8 tick_f
#   9 loop bank, 0 = play once   10 loop address (u16, $8002..): where to go on from at the end

MAGIC = b'GTS1'
BANK_SIZE = 0x4000
HEADER_SIZE = 16
MAX_WAVES = 57


def song_banks(song):
    """-> list of 16K bank images, for GameTank ROM banks 0.."""
    assert len(song.waves) <= MAX_WAVES, 'too many waves: %d' % len(song.waves)
    parse_ticks(song.stream, song.loop_offset)      # validates
    tick_n, tick_f = tick_params(song.rate_reg, song.tick_hz)
    per = BANK_SIZE - 2
    loop_bank, loop_addr = 0, 0
    if song.loop_offset is not None:
        loop_bank, loop_addr = 1 + song.loop_offset // per, 0x8002 + song.loop_offset % per
    hdr = MAGIC + struct.pack('<BBHBBH', song.rate_reg | 0x80, len(song.waves), tick_n, tick_f, loop_bank, loop_addr)
    hdr = hdr.ljust(HEADER_SIZE, b'\0')
    b0 = hdr + bytes(v for w in song.waves for v in w)
    banks = [b0.ljust(BANK_SIZE, b'\xff')]
    for i in range(0, len(song.stream), per):
        chunk = bytes(song.stream[i:i + per])
        banks.append((struct.pack('<H', len(chunk)) + chunk).ljust(BANK_SIZE, b'\xff'))
    banks.append(struct.pack('<H', 0).ljust(BANK_SIZE, b'\xff'))
    return banks


def patch_rom(rom_in, rom_out, song):
    rom = bytearray(open(rom_in, 'rb').read())
    assert len(rom) == 128 * BANK_SIZE
    banks = song_banks(song)
    assert len(banks) <= 120
    for i, b in enumerate(banks):
        rom[i * BANK_SIZE:(i + 1) * BANK_SIZE] = b
    open(rom_out, 'wb').write(rom)
