"""Generate source mask textures for the isolated Hero 1 axe AOE Niagara slash."""

import math
import os
import random

from PIL import Image, ImageFilter


OUT_DIR = r"C:\UE\T66\SourceAssets\VFX\Hero1Axe\AOE"
WIDTH = 512
HEIGHT = 128


def clamp01(value):
    return max(0.0, min(1.0, value))


def smoothstep(edge0, edge1, value):
    if edge0 == edge1:
        return 1.0 if value >= edge1 else 0.0
    t = clamp01((value - edge0) / (edge1 - edge0))
    return t * t * (3.0 - 2.0 * t)


def make_streak_mask(path):
    random.seed(6601)
    img = Image.new("L", (WIDTH, HEIGHT), 0)
    pixels = img.load()
    bands = [
        (0.52, 0.235, 0.92, 0.17),
    ]

    for y in range(HEIGHT):
        v = y / (HEIGHT - 1)
        for x in range(WIDTH):
            u = x / (WIDTH - 1)
            length_taper = max(0.0, math.sin(math.pi * u)) ** 0.58
            end_fade = smoothstep(0.015, 0.11, u) * (1.0 - smoothstep(0.90, 0.995, u))
            body_gate = smoothstep(0.05, 0.18, v) * (1.0 - smoothstep(0.88, 0.99, v))
            value = 0.0

            for center, width, weight, phase in bands:
                wiggle = 0.007 * math.sin((u * 1.35 + phase) * math.tau)
                wiggle += 0.003 * math.sin((u * 3.1 + phase * 0.7) * math.tau)
                band = math.exp(-(((v - center - wiggle) / width) ** 2.0))
                value += band * weight

            outer_edge = math.exp(-(((v - 0.80) / 0.080) ** 2.0)) * 0.04
            inner_edge = math.exp(-(((v - 0.20) / 0.090) ** 2.0)) * 0.03
            satin_fiber = 1.0
            value = (0.16 * body_gate + value + outer_edge + inner_edge) * satin_fiber
            value = clamp01(value * length_taper * end_fade)
            pixels[x, y] = int(value * 255)

    img = img.filter(ImageFilter.GaussianBlur(radius=0.85))
    img.save(path)


def make_dissolve_noise(path):
    random.seed(6602)
    base = Image.new("L", (WIDTH, HEIGHT), 0)
    pix = base.load()

    for y in range(HEIGHT):
        v = y / (HEIGHT - 1)
        for x in range(WIDTH):
            u = x / (WIDTH - 1)
            grain = random.random() * 0.14
            waves = 0.22 * math.sin((u * 5.0 + v * 1.2) * math.tau)
            waves += 0.12 * math.sin((u * 11.0 - v * 3.0) * math.tau)
            broad_flow = 0.18 * math.sin((u * 2.0 + v * 0.75) * math.tau)
            taper = max(0.0, math.sin(math.pi * u)) ** 0.52
            value = clamp01(0.52 + grain + waves + broad_flow)
            pix[x, y] = int(clamp01(value * (0.45 + 0.55 * taper)) * 255)

    base = base.filter(ImageFilter.GaussianBlur(radius=1.15))
    base.save(path)


def make_impact_mask(path):
    img = Image.new("L", (WIDTH, HEIGHT), 0)
    pix = img.load()

    for y in range(HEIGHT):
        v = y / (HEIGHT - 1)
        for x in range(WIDTH):
            u = x / (WIDTH - 1)
            core = math.exp(-(((u - 0.50) / 0.055) ** 2.0 + ((v - 0.78) / 0.18) ** 2.0))
            vertical = math.exp(-(((u - 0.50) / 0.030) ** 2.0)) * smoothstep(0.18, 0.68, v) * (1.0 - smoothstep(0.97, 1.0, v))
            crown = math.exp(-(((u - 0.50) / 0.22) ** 2.0 + ((v - 0.92) / 0.045) ** 2.0))
            value = clamp01(core * 1.0 + vertical * 0.42 + crown * 0.28)
            pix[x, y] = int(value * 255)

    img = img.filter(ImageFilter.GaussianBlur(radius=0.82))
    img.save(path)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    streak_path = os.path.join(OUT_DIR, "T_Hero1AxeAOE_StreakMask.png")
    dissolve_path = os.path.join(OUT_DIR, "T_Hero1AxeAOE_DissolveNoise.png")
    impact_path = os.path.join(OUT_DIR, "T_Hero1AxeAOE_ImpactMask.png")
    make_streak_mask(streak_path)
    make_dissolve_noise(dissolve_path)
    make_impact_mask(impact_path)
    print(streak_path)
    print(dissolve_path)
    print(impact_path)


if __name__ == "__main__":
    main()
