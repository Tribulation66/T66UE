import math
from pathlib import Path

from PIL import Image


PROJECT_DIR = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_DIR / "SourceAssets" / "UI" / "MainMenuArcaneFill"
IMAGE_SIZE = (512, 128)


STATE_SPECS = {
    "N": {
        "rotation_deg": -7.5,
        "band_scale": 8.2,
        "color1": (96, 49, 118),
        "color2": (24, 78, 58),
        "color3": (8, 9, 18),
        "spot_color": (72, 115, 88),
        "brightness": 0.95,
        "spot_amount": 0.20,
        "seed": 117.0,
        "offset": 1.3,
    },
    "H": {
        "rotation_deg": -5.0,
        "band_scale": 8.8,
        "color1": (118, 66, 140),
        "color2": (34, 102, 74),
        "color3": (10, 12, 24),
        "spot_color": (92, 136, 110),
        "brightness": 1.05,
        "spot_amount": 0.24,
        "seed": 241.0,
        "offset": 2.1,
    },
    "P": {
        "rotation_deg": -9.0,
        "band_scale": 9.1,
        "color1": (80, 38, 98),
        "color2": (18, 62, 46),
        "color3": (6, 7, 16),
        "spot_color": (58, 92, 74),
        "brightness": 0.84,
        "spot_amount": 0.16,
        "seed": 389.0,
        "offset": 3.0,
    },
}


def clamp01(value):
    return max(0.0, min(1.0, value))


def smoothstep(edge0, edge1, x):
    if edge0 == edge1:
        return 0.0
    t = clamp01((x - edge0) / (edge1 - edge0))
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


def rotate_uv(u, v, angle_deg):
    angle = math.radians(angle_deg)
    centered_x = u - 0.5
    centered_y = v - 0.5
    cos_a = math.cos(angle)
    sin_a = math.sin(angle)
    return (
        centered_x * cos_a - centered_y * sin_a + 0.5,
        centered_x * sin_a + centered_y * cos_a + 0.5,
    )


def quantize(value, steps):
    return math.floor(value * steps) / max(1.0, steps)


def generate_fill_image(width, height, spec):
    image = Image.new("RGBA", (width, height))
    pixels = image.load()

    for y in range(height):
        for x in range(width):
            u = x / max(1, width - 1)
            v = y / max(1, height - 1)

            ru, rv = rotate_uv(u, v, spec["rotation_deg"])
            seeded_u = ru + spec["offset"] * 0.035 + hash_noise(spec["seed"], 1.0, 17.0) * 0.1
            seeded_v = rv + spec["offset"] * 0.01

            major = quantize(seeded_u, 96.0)
            minor = quantize(seeded_v, 24.0)

            warp_a = value_noise(major * 6.0 + 1.0, minor * 5.0 + 3.0, spec["seed"]) * 2.0 - 1.0
            warp_b = value_noise(major * 13.0 + 5.0, minor * 9.0 + 7.0, spec["seed"] + 19.0) * 2.0 - 1.0
            warp = warp_a * 0.34 + warp_b * 0.13

            bands = math.sin((major * spec["band_scale"] + warp) * math.tau)
            bands = bands * 0.5 + 0.5

            stepped_1 = smoothstep(0.28 - 0.14, 0.28 + 0.14, bands)
            stepped_2 = smoothstep(0.61 - 0.14, 0.61 + 0.14, bands)

            color = spec["color3"]
            color = lerp_color(color, spec["color2"], stepped_1)
            color = lerp_color(color, spec["color1"], stepped_2)

            spot_noise = value_noise(major * 22.0 + 2.0, minor * 8.0 + 9.0, spec["seed"] + 41.0)
            spot_noise_2 = value_noise(major * 37.0 + 8.0, minor * 15.0 + 4.0, spec["seed"] + 87.0)
            spots = smoothstep(0.68, 0.78, spot_noise * 0.7 + spot_noise_2 * 0.3)
            color = lerp_color(color, spec["spot_color"], spots * spec["spot_amount"])

            edge = min(u, 1.0 - u, v, 1.0 - v)
            rim = 1.0 - smoothstep(0.0, 0.12, edge)
            rim_strength = 0.34 + spots * 0.06
            rim_multiplier = 1.0 - rim * rim_strength

            center_band = 1.0 - abs(v - 0.5) * 2.0
            center_band = smoothstep(0.0, 1.0, center_band)
            text_readability = 0.90 + center_band * 0.08

            noise_dither = (value_noise(major * 64.0 + 4.0, minor * 32.0 + 11.0, spec["seed"] + 123.0) - 0.5) * 10.0

            final = []
            for channel in color:
                shaded = channel * spec["brightness"] * rim_multiplier * text_readability + noise_dither
                final.append(int(max(0.0, min(255.0, shaded))))

            pixels[x, y] = (final[0], final[1], final[2], 255)

    return image


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    for state, spec in STATE_SPECS.items():
        image = generate_fill_image(IMAGE_SIZE[0], IMAGE_SIZE[1], spec)
        output_path = OUTPUT_DIR / f"T_UI_MainMenuArcaneFill_{state}.png"
        image.save(output_path)

    print(f"[BUTTONFILL] Generated {len(STATE_SPECS)} arcane fill textures in {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
