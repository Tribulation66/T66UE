"""
Generate placeholder OST loops and stingers for T66 (comedic synthwave / chiptune hybrid).

Deterministic (seeded per track name): re-running produces identical files.
Outputs .ogg files directly into Content/Audio/OSTS/<Folder>/ following the
UT66MusicSubsystem folder contract:

  Stages/Stage_NN/        per-stage gameplay theme (Easy = Stage_01..Stage_04)
  Bosses/<BossID>/        per-boss fight theme
  Areas/<NPCID>/          NPC bubble override theme (VendorNPC, CasinoNPC, Saint, Ouroboros, LoanShark)
  Heroes/<HeroKey>/       hero gameplay theme (HeroKey = Heroes.csv MapTheme; Hero_1 -> Hero_4)
  Stingers/<StingerID>/   one-shot cues (Victory, Defeat)

Replacing any placeholder with a professional track = overwrite the .ogg at the
same path and re-run Scripts/ImportGeneratedOSTs.py (or import in-editor).

Requires: numpy, scipy, ffmpeg on PATH. Run with plain Python (no Unreal needed):
  python Scripts/ComposeT66PlaceholderOSTs.py
"""

import hashlib
import subprocess
import sys
import wave
from pathlib import Path

import numpy as np
from scipy.signal import butter, lfilter

SR = 48000
PROJECT_ROOT = Path(__file__).resolve().parents[1]
OSTS_ROOT = PROJECT_ROOT / "Content" / "Audio" / "OSTS"
TEMP_WAV_DIR = PROJECT_ROOT / "Saved" / "AudioGen"


# ---------------------------------------------------------------- synth core

def midi_to_freq(m):
    return 440.0 * 2.0 ** ((m - 69) / 12.0)


def lowpass(x, cutoff_hz, order=2):
    cutoff = min(max(cutoff_hz, 40.0), SR * 0.45)
    b, a = butter(order, cutoff / (SR * 0.5), btype="low")
    return lfilter(b, a, x).astype(np.float32)


def highpass(x, cutoff_hz, order=2):
    cutoff = min(max(cutoff_hz, 20.0), SR * 0.45)
    b, a = butter(order, cutoff / (SR * 0.5), btype="high")
    return lfilter(b, a, x).astype(np.float32)


def osc_sine(freq, n, phase=0.0):
    t = np.arange(n, dtype=np.float32) / SR
    return np.sin(2 * np.pi * freq * t + phase).astype(np.float32)


def osc_saw(freq, n, detune=0.0):
    t = np.arange(n, dtype=np.float32) / SR
    f = freq * (1.0 + detune)
    return (2.0 * ((f * t) % 1.0) - 1.0).astype(np.float32)


def osc_pulse(freq, n, duty=0.5):
    t = np.arange(n, dtype=np.float32) / SR
    return np.where((freq * t) % 1.0 < duty, 1.0, -1.0).astype(np.float32)


def osc_triangle(freq, n):
    t = np.arange(n, dtype=np.float32) / SR
    return (2.0 * np.abs(2.0 * ((freq * t) % 1.0) - 1.0) - 1.0).astype(np.float32)


def env_decay(n, decay_s):
    t = np.arange(n, dtype=np.float32) / SR
    return np.exp(-t / max(decay_s, 1e-3)).astype(np.float32)


def env_adsr(n, a=0.01, d=0.05, s=0.7, r=0.05):
    na, nd, nr = (max(1, int(x * SR)) for x in (a, d, r))
    ns = max(0, n - na - nd - nr)
    parts = [
        np.linspace(0, 1, na, dtype=np.float32),
        np.linspace(1, s, nd, dtype=np.float32),
        np.full(ns, s, dtype=np.float32),
        np.linspace(s, 0, nr, dtype=np.float32),
    ]
    env = np.concatenate(parts)
    return env[:n] if len(env) >= n else np.pad(env, (0, n - len(env)))


def echo(x, time_s, feedback=0.35, mix=0.3, taps=4):
    out = x.copy()
    d = int(time_s * SR)
    gain = mix
    for i in range(1, taps + 1):
        shift = d * i
        if shift >= len(x):
            break
        out[shift:] += x[:-shift] * gain
        gain *= feedback
    return out


def add_at(buf, start, sig, gain=1.0):
    end = min(len(buf), start + len(sig))
    if end > start >= 0:
        buf[start:end] += sig[: end - start] * gain


# ---------------------------------------------------------------- drum kit

def drum_kick(punch=1.0):
    n = int(0.28 * SR)
    t = np.arange(n, dtype=np.float32) / SR
    freq = 38.0 + 110.0 * np.exp(-t * 28.0)
    phase = np.cumsum(2 * np.pi * freq / SR).astype(np.float32)
    body = np.sin(phase) * env_decay(n, 0.16)
    click = (np.random.default_rng(7).standard_normal(int(0.008 * SR)).astype(np.float32)) * 0.4
    out = body * punch
    out[: len(click)] += click * punch
    return out


def drum_snare(rng, bright=1.0):
    n = int(0.2 * SR)
    nz = rng.standard_normal(n).astype(np.float32)
    nz = highpass(nz, 1500.0) * env_decay(n, 0.07) * 0.8
    tone = osc_sine(190.0, n) * env_decay(n, 0.05) * 0.5
    return (nz * bright + tone).astype(np.float32)


def drum_hat(rng, open_=False):
    n = int((0.14 if open_ else 0.045) * SR)
    nz = rng.standard_normal(n).astype(np.float32)
    nz = highpass(nz, 6500.0) * env_decay(n, 0.09 if open_ else 0.018)
    return nz * 0.5


# ---------------------------------------------------------------- chords

QUALITIES = {
    "M": (0, 4, 7),
    "m": (0, 3, 7),
    "m7": (0, 3, 7, 10),
    "7": (0, 4, 7, 10),
    "M7": (0, 4, 7, 11),
    "sus": (0, 5, 7),
    "dim": (0, 3, 6),
    "5": (0, 7, 12),
}


def chord_midis(key_root, offset, quality, octave=0):
    return [key_root + offset + iv + 12 * octave for iv in QUALITIES[quality]]


# ---------------------------------------------------------------- track renderer

def render_loop(spec):
    name = spec["name"]
    seed = int(hashlib.sha256(name.encode()).hexdigest(), 16) % (2**32)
    rng = np.random.default_rng(seed)

    bpm = spec["bpm"]
    bars = spec["bars"]
    prog = spec["progression"]
    key_root = spec["key_root"]
    energy = spec.get("energy", 0.6)
    swing = spec.get("swing", 0.0)

    spb = int(SR * 60.0 / bpm)          # samples per beat
    bar_len = spb * 4
    loop_len = bar_len * bars
    total = loop_len * 2                # render two loops, keep the 2nd (seamless tails)

    drums = np.zeros(total, dtype=np.float32)
    bass = np.zeros(total, dtype=np.float32)
    pad = np.zeros(total, dtype=np.float32)
    arp = np.zeros(total, dtype=np.float32)
    lead = np.zeros(total, dtype=np.float32)

    kick_sig = drum_kick(punch=0.9 + 0.4 * energy)
    snare_sig = drum_snare(rng, bright=0.8 + 0.5 * energy)
    hat_sig = drum_hat(rng)
    ohat_sig = drum_hat(rng, open_=True)

    kick_pat = spec.get("kick", "x...x...x...x...")
    snare_pat = spec.get("snare", "....x.......x...")
    hat_pat = spec.get("hat", "x.x.x.x.x.x.x.x.")
    drums_on = spec.get("drums", True)

    def step_offset(step):
        base = step * spb // 4
        if swing > 0 and step % 2 == 1:
            base += int(spb / 4 * swing * 0.5)
        return base

    # Melody: one loop's worth of events, repeated identically both loops.
    melody_events = []
    if spec.get("lead", True):
        scale = spec.get("scale", (0, 2, 3, 5, 7, 8, 10))  # natural minor default
        degree = rng.integers(0, len(scale))
        lead_bars = spec.get("lead_bars", range(bars // 2, bars))
        for bar in lead_bars:
            beat = 0.0
            while beat < 4.0:
                dur = float(rng.choice((0.5, 0.5, 1.0, 1.0, 2.0)))
                if rng.random() > 0.22:  # rests keep it from noodling constantly
                    degree = int(np.clip(degree + rng.integers(-2, 3), 0, len(scale) * 2 - 1))
                    midi = key_root + 12 + scale[degree % len(scale)] + 12 * (degree // len(scale))
                    melody_events.append((bar, beat, dur, midi))
                beat += dur

    for loop_i in range(2):
        loop_base = loop_i * loop_len
        for bar in range(bars):
            bar_base = loop_base + bar * bar_len
            offset, quality = prog[bar % len(prog)]
            chord = chord_midis(key_root, offset, quality)

            if drums_on:
                for step in range(16):
                    pos = bar_base + step_offset(step)
                    if kick_pat[step % len(kick_pat)] == "x":
                        add_at(drums, pos, kick_sig, 0.95)
                    if snare_pat[step % len(snare_pat)] == "x":
                        add_at(drums, pos, snare_sig, 0.8)
                    hp = hat_pat[step % len(hat_pat)]
                    if hp == "x":
                        add_at(drums, pos, hat_sig, 0.45 + 0.25 * energy)
                    elif hp == "o":
                        add_at(drums, pos, ohat_sig, 0.4)

            # Bass: eighths on the chord root, octave pop by energy.
            root_midi = key_root + offset - 12
            bass_pat = spec.get("bass_pat", "x.x.x.x.x.x.x.x.")
            for step in range(16):
                if bass_pat[step % len(bass_pat)] != "x":
                    continue
                pos = bar_base + step_offset(step)
                m = root_midi + (12 if (energy > 0.65 and step % 8 == 6) else 0)
                n = int(spb * 0.45)
                sig = osc_saw(midi_to_freq(m), n) * env_decay(n, 0.12) * 0.9
                if spec.get("bass_drive", 0) > 0:
                    sig = np.tanh(sig * (1.0 + spec["bass_drive"] * 3.0)).astype(np.float32)
                add_at(bass, pos, sig)

            # Pad: detuned saw chord held for the bar.
            if spec.get("pad", True):
                n = bar_len
                sig = np.zeros(n, dtype=np.float32)
                for m in chord:
                    for det in (-0.004, 0.0, 0.004):
                        sig += osc_saw(midi_to_freq(m), n, detune=det)
                sig *= env_adsr(n, a=0.25, d=0.2, s=0.8, r=0.4) / (len(chord) * 3)
                add_at(pad, bar_base, sig, 0.65)

            # Arp: 16th-note chord-tone cycling.
            if spec.get("arp", True):
                tones = chord + [chord[0] + 12]
                for step in range(16):
                    if rng.random() > (0.55 + 0.45 * energy):
                        continue
                    pos = bar_base + step_offset(step)
                    m = tones[step % len(tones)] + 12
                    n = int(spb * 0.22)
                    sig = osc_pulse(midi_to_freq(m), n, duty=0.3) * env_decay(n, 0.05)
                    add_at(arp, pos, sig, 0.4)

        for bar, beat, dur, midi in melody_events:
            pos = loop_base + bar * bar_len + int(beat * spb)
            n = int(dur * spb * 0.92)
            vib = 1.0 + 0.004 * osc_sine(5.5, n)
            sig = osc_triangle(midi_to_freq(midi), n) * vib * env_adsr(n, a=0.02, d=0.08, s=0.6, r=0.1)
            add_at(lead, pos, sig, 0.55)

    bass = lowpass(bass, 400.0 + 1800.0 * energy)
    pad = lowpass(pad, 1200.0 + 2200.0 * energy)
    arp = echo(lowpass(arp, 5000.0), time_s=60.0 / bpm * 0.75, feedback=0.4, mix=0.35)
    lead = echo(lead, time_s=60.0 / bpm * 0.5, feedback=0.3, mix=0.3)

    mono_mix = (
        drums * spec.get("drums_gain", 1.0)
        + bass * spec.get("bass_gain", 0.9)
        + pad * spec.get("pad_gain", 0.8)
        + arp * spec.get("arp_gain", 0.8)
        + lead * spec.get("lead_gain", 0.9)
    )

    # Stereo: pad/arp wide (Haas), drums/bass center.
    haas = int(0.012 * SR)
    side = pad * 0.5 + arp * 0.6
    left = mono_mix + np.roll(side, 0) * 0.3
    right = mono_mix + np.roll(side, haas) * 0.3
    stereo = np.stack([left, right], axis=1)

    out = stereo[loop_len : loop_len * 2]
    out = np.tanh(out * 1.1)
    peak = np.max(np.abs(out)) or 1.0
    return (out / peak * 0.86).astype(np.float32)


# ---------------------------------------------------------------- stingers

def render_stinger_victory():
    rng = np.random.default_rng(11)
    n = int(5.0 * SR)
    out = np.zeros(n, dtype=np.float32)
    key = 57  # A
    arp_notes = [0, 4, 7, 12, 16, 19, 24]
    step = int(0.09 * SR)
    for i, iv in enumerate(arp_notes):
        sig_n = int(0.5 * SR)
        sig = osc_pulse(midi_to_freq(key + iv), sig_n, 0.4) * env_decay(sig_n, 0.18)
        add_at(out, i * step, sig, 0.5)
    chord_start = len(arp_notes) * step
    chord_n = n - chord_start
    for iv in (0, 4, 7, 12, 19):
        for det in (-0.005, 0.0, 0.005):
            sig = osc_saw(midi_to_freq(key + 12 + iv), chord_n, detune=det)
            add_at(out, chord_start, sig * env_adsr(chord_n, a=0.01, d=0.4, s=0.5, r=2.2), 0.09)
    shimmer = highpass(rng.standard_normal(chord_n).astype(np.float32), 8000.0)
    add_at(out, chord_start, shimmer * env_decay(chord_n, 0.8), 0.05)
    out = echo(out, 0.22, feedback=0.35, mix=0.3)
    return _finalize_oneshot(out)


def render_stinger_defeat():
    n = int(4.2 * SR)
    out = np.zeros(n, dtype=np.float32)
    key = 50  # D
    sad_walk = [12, 11, 8, 5, 0]  # chromatic-ish slump down to the root
    step = int(0.42 * SR)
    for i, iv in enumerate(sad_walk):
        sig_n = int(0.6 * SR) if i < len(sad_walk) - 1 else int(2.2 * SR)
        bend = np.linspace(1.0, 0.985, sig_n, dtype=np.float32)  # droop
        t = np.arange(sig_n, dtype=np.float32) / SR
        phase = np.cumsum(2 * np.pi * midi_to_freq(key + iv) * bend / SR)
        sig = np.sin(phase).astype(np.float32) * env_adsr(sig_n, a=0.02, d=0.1, s=0.7, r=0.8)
        add_at(out, i * step, sig, 0.5)
    thud_n = int(1.2 * SR)
    thud = osc_sine(48.0, thud_n) * env_decay(thud_n, 0.5)
    add_at(out, len(sad_walk) * step - int(0.2 * SR), thud, 0.7)
    out = echo(out, 0.3, feedback=0.3, mix=0.25)
    return _finalize_oneshot(out)


def _finalize_oneshot(mono):
    fade = int(0.4 * SR)
    mono[-fade:] *= np.linspace(1, 0, fade, dtype=np.float32)
    stereo = np.stack([mono, np.roll(mono, int(0.008 * SR))], axis=1)
    stereo = np.tanh(stereo * 1.1)
    peak = np.max(np.abs(stereo)) or 1.0
    return (stereo / peak * 0.88).astype(np.float32)


# ---------------------------------------------------------------- output

def write_ogg(rel_path, audio):
    ogg_path = OSTS_ROOT / rel_path
    ogg_path.parent.mkdir(parents=True, exist_ok=True)
    TEMP_WAV_DIR.mkdir(parents=True, exist_ok=True)
    wav_path = TEMP_WAV_DIR / (ogg_path.stem + ".wav")

    pcm = (np.clip(audio, -1.0, 1.0) * 32767).astype(np.int16)
    with wave.open(str(wav_path), "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(pcm.tobytes())

    result = subprocess.run(
        ["ffmpeg", "-y", "-loglevel", "error", "-i", str(wav_path), "-c:a", "libvorbis", "-qscale:a", "5", str(ogg_path)],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"ffmpeg failed for {ogg_path}: {result.stderr}")

    rms = float(np.sqrt(np.mean(audio**2)))
    secs = len(audio) / SR
    print(f"  {rel_path}  {secs:5.1f}s  rms={rms:.3f}  ({ogg_path.stat().st_size // 1024} KB)")


# ---------------------------------------------------------------- catalog

MINOR = (0, 2, 3, 5, 7, 8, 10)
DORIAN = (0, 2, 3, 5, 7, 9, 10)
MAJOR = (0, 2, 4, 5, 7, 9, 11)
PHRYGIAN = (0, 1, 3, 5, 7, 8, 10)

TRACKS = [
    # --- Easy stage themes (Dungeon biome, escalating) ---
    dict(name="Stage_01", path="Stages/Stage_01/MUS_Stage_01.ogg", bpm=104, bars=16, key_root=45, scale=MINOR,
         progression=[(0, "m"), (0, "m"), (8, "M"), (10, "M"), (3, "M"), (3, "M"), (10, "M"), (7, "m")],
         energy=0.5, kick="x...x...x...x...", snare="....x.......x...", hat="x.x.x.x.x.x.x.x."),
    dict(name="Stage_02", path="Stages/Stage_02/MUS_Stage_02.ogg", bpm=112, bars=16, key_root=43, scale=MINOR,
         progression=[(0, "m"), (5, "m"), (8, "M"), (10, "M")],
         energy=0.6, kick="x...x..x..x.x...", snare="....x.......x...", hat="x.xxx.x.x.xxx.x."),
    dict(name="Stage_03", path="Stages/Stage_03/MUS_Stage_03.ogg", bpm=120, bars=16, key_root=41, scale=DORIAN,
         progression=[(0, "m"), (3, "M"), (10, "M"), (5, "m")],
         energy=0.7, bass_drive=0.3, kick="x..x..x...x..x..", snare="....x.......x..x", hat="xxxxxxxxxxxxxxxx"),
    dict(name="Stage_04", path="Stages/Stage_04/MUS_Stage_04.ogg", bpm=128, bars=16, key_root=40, scale=PHRYGIAN,
         progression=[(0, "m"), (1, "M"), (0, "m"), (10, "M")],
         energy=0.85, bass_drive=0.5, kick="x.x.x.x.x.x.x.x.", snare="....x.......x...", hat="x.xxx.xxx.xxx.xx"),

    # --- Easy boss themes ---
    dict(name="Boss_SewerSlimeKing", path="Bosses/Dungeon_SewerSlimeKing/MUS_Boss_SewerSlimeKing.ogg",
         bpm=96, bars=8, key_root=38, scale=MINOR,
         progression=[(0, "m"), (0, "m"), (6, "dim"), (5, "m")],
         energy=0.75, bass_drive=0.8, swing=0.4, bass_pat="x..x..x.x..x..x.",
         kick="x..x....x..x....", snare="....x.......x...", hat="..x...x...x...x.", arp_gain=0.5),
    dict(name="Boss_WebMatriarch", path="Bosses/Dungeon_WebMatriarch/MUS_Boss_WebMatriarch.ogg",
         bpm=132, bars=8, key_root=42, scale=PHRYGIAN,
         progression=[(0, "m"), (1, "M"), (3, "dim"), (1, "M")],
         energy=0.8, bass_drive=0.4, hat="xxxxxxxxxxxxxxxx", kick="x...x...x...x...",
         snare="....x..x....x...", arp_gain=1.0, pad_gain=0.6),
    dict(name="Boss_BoneJailer", path="Bosses/Dungeon_BoneJailer/MUS_Boss_BoneJailer.ogg",
         bpm=120, bars=8, key_root=40, scale=MINOR,
         progression=[(0, "5"), (3, "5"), (0, "5"), (5, "5")],
         energy=0.8, bass_drive=0.6, kick="x..x.x..x..x.x..", snare="....x.......x...",
         hat="x.x.x.x.x.x.xxx.", pad=False, arp_gain=0.9, lead_bars=range(4, 8)),
    dict(name="Boss_BaelFallenChad", path="Bosses/Dungeon_BaelFallenChad/MUS_Boss_BaelFallenChad.ogg",
         bpm=140, bars=16, key_root=38, scale=PHRYGIAN,
         progression=[(0, "m"), (0, "m"), (1, "M"), (1, "M"), (8, "M"), (8, "M"), (1, "M"), (0, "m")],
         energy=0.95, bass_drive=0.9, kick="x.x.x.x.x.x.x.x.", snare="....x.......x...",
         hat="xxxxxxxxxxxxxxxx", drums_gain=1.1, lead_gain=1.0),
    dict(name="Boss_Vendor", path="Bosses/VendorBoss/MUS_Boss_Vendor.ogg",
         bpm=150, bars=8, key_root=45, scale=MINOR,
         progression=[(0, "m"), (10, "M"), (8, "M"), (10, "M")],
         energy=0.9, bass_drive=0.7, kick="x..xx..xx..xx..x", snare="....x.......x...",
         hat="x.x.x.x.x.x.x.x.", arp_gain=1.0),

    # --- NPC area themes ---
    dict(name="Area_Vendor", path="Areas/VendorNPC/MUS_Area_Vendor.ogg",
         bpm=102, bars=8, key_root=48, scale=MAJOR,
         progression=[(0, "M"), (5, "M"), (7, "7"), (0, "M")],
         energy=0.45, kick="x...x...x...x...", snare="....x.......x...", hat="..x...x...x...x.",
         bass_pat="x...x...x...x...", arp_gain=0.7, lead_bars=range(2, 8)),
    dict(name="Area_Casino", path="Areas/CasinoNPC/MUS_Area_Casino.ogg",
         bpm=116, bars=8, key_root=46, scale=DORIAN, swing=0.6,
         progression=[(0, "m7"), (5, "7"), (10, "M7"), (3, "7")],
         energy=0.55, kick="x.....x.x.......", snare="....x.......x...", hat="x.xox.x.x.xox.x.",
         bass_pat="x..x..x...x..x..", pad_gain=0.7, arp_gain=0.6),
    dict(name="Area_Saint", path="Areas/Saint/MUS_Area_Saint.ogg",
         bpm=72, bars=8, key_root=50, scale=MAJOR, drums=False,
         progression=[(0, "M7"), (5, "M7"), (9, "m7"), (7, "sus")],
         energy=0.3, bass_pat="x...............", pad_gain=1.0, arp_gain=0.35, lead_gain=0.5,
         lead_bars=range(4, 8)),
    dict(name="Area_Ouroboros", path="Areas/Ouroboros/MUS_Area_Ouroboros.ogg",
         bpm=66, bars=8, key_root=36, scale=PHRYGIAN, drums=False,
         progression=[(0, "m"), (0, "m"), (1, "dim"), (0, "m")],
         energy=0.25, bass_pat="x.......x.......", pad_gain=1.0, arp=False, lead_gain=0.4,
         lead_bars=range(2, 8)),
    dict(name="Area_LoanShark", path="Areas/LoanShark/MUS_Area_LoanShark.ogg",
         bpm=92, bars=8, key_root=40, scale=PHRYGIAN,
         progression=[(0, "m"), (1, "M"), (0, "m"), (1, "M")],
         energy=0.6, bass_drive=0.6, bass_pat="x.x.............", kick="x.......x.......",
         snare="............x...", hat="..x...x...x...x.", pad_gain=0.5, arp=False, lead_gain=0.45),

    # --- Hero theme (Hero_1 Founding Chad; folder key = Heroes.csv MapTheme = Hero_4) ---
    dict(name="Hero_1_FoundingChad", path="Heroes/Hero_4/MUS_Hero_1_FoundingChad.ogg",
         bpm=118, bars=16, key_root=45, scale=DORIAN,
         progression=[(0, "m"), (10, "M"), (5, "m"), (8, "M7")],
         energy=0.7, kick="x...x...x...x..x", snare="....x.......x...", hat="x.x.x.xxx.x.x.x.",
         bass_drive=0.3, lead_gain=1.0),
]


def main():
    if not OSTS_ROOT.exists():
        print(f"ERROR: {OSTS_ROOT} not found; run from the T66 repo.", file=sys.stderr)
        return 1

    print(f"Rendering {len(TRACKS)} loops + 2 stingers to {OSTS_ROOT}")
    for spec in TRACKS:
        audio = render_loop(spec)
        write_ogg(spec["path"], audio)

    write_ogg("Stingers/Victory/MUS_Stinger_Victory.ogg", render_stinger_victory())
    write_ogg("Stingers/Defeat/MUS_Stinger_Defeat.ogg", render_stinger_defeat())
    print("Done. Import into Unreal with Scripts/ImportGeneratedOSTs.py (editor commandlet).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
