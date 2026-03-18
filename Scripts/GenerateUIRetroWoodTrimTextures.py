import math
import os
from pathlib import Path

from PIL import Image


PROJECT_DIR = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_DIR / "SourceAssets" / "UI" / "RetroWoodTrimV2"

HORIZONTAL_SIZE = (256, 24)
VERTICAL_SIZE = (24, 256)

PALETTES = {
    "N": {
        "light": (245, 214, 102),
        "mid": (120, 195, 246),
        "dark": (44, 77, 135),
        "accent": (255, 241, 170),
        "rim": 0.52,
        "brightness": 1.0,
    },
    "H": {
        "light": (255, 230, 132),
        "mid": (149, 213, 255),
        "dark": (56, 99, 166),
        "accent": (255, 248, 194),
        "rim": 0.40,
        "brightness": 1.10,
    },
    "P": {
        "light": (216, 182, 78),
        "mid": (85, 156, 214),
        "dark": (28, 55, 104),
        "accent": (240, 214, 138),
        "rim": 0.64,
        "brightness": 0.88,
    },
}


def smoothstep(edge0, edge1, x):
    if edge0 == edge1:
        return 0.0
    t = max(0.0, min(1.0, (x - edge0) / (edge1 - edge0)))
    return t * t * (3.0 - 2.0 * t)


def lerp(a, b, t):
    return a + (b - a) * t


def lerp_color(a, b, t):
    return tuple(lerp(a[i], b[i], t) for i in range(3))


def hash_noise(ix, iy, seed):
    value = math.sin(ix * 127.1 + iy * 311.7 + seed * 91.7) * 43758.5453123
    return value - math.floor(value)


def value_noise(x, y, seed):
    x0 = math.floor(x)
    y0 = math.floor(y)
    x1 = x0 + 1
    y1 = y0 + 1
    tx = x - x0
    ty = y - y0
    sx = tx * tx * (3.0 - 2.0 * tx)
    sy = ty * ty * (3.0 - 2.0 * ty)

    n00 = hash_noise(x0, y0, seed)
    n10 = hash_noise(x1, y0, seed)
    n01 = hash_noise(x0, y1, seed)
    n11 = hash_noise(x1, y1, seed)

    ix0 = lerp(n00, n10, sx)
    ix1 = lerp(n01, n11, sx)
    return lerp(ix0, ix1, sy)


def generate_trim_image(width, height, orientation, palette, seed):
    image = Image.new("RGBA", (width, height))
    pixels = image.load()

    for y in range(height):
        for x in range(width):
            u = x / max(1, width - 1)
            v = y / max(1, height - 1)

            major = u if orientation == "H" else v
            minor = v if orientation == "H" else u

            # Quantize the sampling a bit to keep a chunky retro look.
            major_q = math.floor(major * 96.0) / 96.0
            minor_q = math.floor(minor * 18.0) / 18.0

            warp_a = value_noise(major_q * 7.0 + 3.0, minor_q * 3.5 + 1.0, seed) * 2.0 - 1.0
            warp_b = value_noise(major_q * 13.0 + 8.0, minor_q * 6.0 + 4.0, seed + 17.0) * 2.0 - 1.0
            warp = warp_a * 0.26 + warp_b * 0.10

            bands = math.sin((major_q * 10.5 + warp) * math.tau)
            bands = bands * 0.5 + 0.5

            step_a = smoothstep(0.22, 0.48, bands)
            step_b = smoothstep(0.56, 0.80, bands)

            color = palette["dark"]
            color = lerp_color(color, palette["mid"], step_a)
            color = lerp_color(color, palette["light"], step_b)

            spot_1 = value_noise(major_q * 18.0 + 2.0, minor_q * 9.0 + 7.0, seed + 31.0)
            spot_2 = value_noise(major_q * 31.0 + 5.0, minor_q * 15.0 + 9.0, seed + 59.0)
            streak = value_noise(major_q * 42.0 + 11.0, minor_q * 4.0 + 13.0, seed + 83.0)
            spots = smoothstep(0.60, 0.77, spot_1 * 0.45 + spot_2 * 0.35 + streak * 0.20)
            color = lerp_color(color, palette["accent"], spots * 0.34)

            edge = min(u, 1.0 - u, v, 1.0 - v)
            rim = 1.0 - smoothstep(0.0, 0.18, edge)
            rim_multiplier = 1.0 - rim * palette["rim"]

            color = tuple(max(0.0, min(255.0, channel * palette["brightness"] * rim_multiplier)) for channel in color)
            pixels[x, y] = (int(color[0]), int(color[1]), int(color[2]), 255)

    return image


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    specs = [
        ("T_UI_RetroWoodTrimH_V2_N.png", HORIZONTAL_SIZE, "H", PALETTES["N"], 101.0),
        ("T_UI_RetroWoodTrimH_V2_H.png", HORIZONTAL_SIZE, "H", PALETTES["H"], 203.0),
        ("T_UI_RetroWoodTrimH_V2_P.png", HORIZONTAL_SIZE, "H", PALETTES["P"], 307.0),
        ("T_UI_RetroWoodTrimV_V2_N.png", VERTICAL_SIZE, "V", PALETTES["N"], 401.0),
        ("T_UI_RetroWoodTrimV_V2_H.png", VERTICAL_SIZE, "V", PALETTES["H"], 503.0),
        ("T_UI_RetroWoodTrimV_V2_P.png", VERTICAL_SIZE, "V", PALETTES["P"], 607.0),
    ]

    for filename, size, orientation, palette, seed in specs:
        image = generate_trim_image(size[0], size[1], orientation, palette, seed)
        image.save(OUTPUT_DIR / filename)

    print(f"[BORDER] Generated {len(specs)} retro wood trim textures in {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
