import math
from pathlib import Path

from PIL import Image


PROJECT_DIR = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_DIR / "SourceAssets" / "UI" / "MainMenuBlueWoodFill"
IMAGE_SIZE = (512, 128)


STATE_SPECS = {
    "N": {
        "rotation_deg": -6.0,
        "band_scale": 8.9,
        "band_smoothness": 0.12,
        "color1": (72, 112, 188),
        "color2": (16, 46, 104),
        "color3": (2, 7, 20),
        "spot_color": (118, 163, 236),
        "brightness": 0.92,
        "spot_amount": 0.18,
        "center_shadow": 0.08,
        "rim_strength": 0.28,
        "grain_strength": 7.0,
        "seed": 149.0,
        "offset": 1.4,
    },
    "H": {
        "rotation_deg": -4.5,
        "band_scale": 9.3,
        "band_smoothness": 0.13,
        "color1": (232, 203, 114),
        "color2": (116, 89, 22),
        "color3": (5, 12, 30),
        "spot_color": (255, 234, 170),
        "brightness": 0.95,
        "spot_amount": 0.24,
        "center_shadow": 0.13,
        "rim_strength": 0.24,
        "grain_strength": 7.5,
        "seed": 263.0,
        "offset": 2.1,
    },
    "P": {
        "rotation_deg": -7.5,
        "band_scale": 9.7,
        "band_smoothness": 0.11,
        "color1": (198, 166, 84),
        "color2": (82, 62, 17),
        "color3": (3, 8, 19),
        "spot_color": (229, 197, 118),
        "brightness": 0.84,
        "spot_amount": 0.18,
        "center_shadow": 0.14,
        "rim_strength": 0.30,
        "grain_strength": 6.4,
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

            rotated_u, rotated_v = rotate_uv(u, v, spec["rotation_deg"])
            sample_u = quantize(
                rotated_u + spec["offset"] * 0.032 + hash_noise(spec["seed"], 1.0, 17.0) * 0.08,
                128.0,
            )
            sample_v = quantize(rotated_v + spec["offset"] * 0.012, 36.0)

            warp_a = value_noise(sample_u * 5.0 + 3.0, sample_v * 4.0 + 1.0, spec["seed"]) * 2.0 - 1.0
            warp_b = value_noise(sample_u * 11.0 + 7.0, sample_v * 8.0 + 5.0, spec["seed"] + 19.0) * 2.0 - 1.0
            warp = warp_a * 0.38 + warp_b * 0.16

            band_noise = value_noise(sample_u * 2.5 + 2.0, sample_v * 1.5 + 9.0, spec["seed"] + 41.0)
            bands = math.sin((sample_u * spec["band_scale"] + warp + band_noise * 0.65) * math.tau)
            bands = bands * 0.5 + 0.5

            stepped_1 = smoothstep(0.30 - spec["band_smoothness"], 0.30 + spec["band_smoothness"], bands)
            stepped_2 = smoothstep(0.62 - spec["band_smoothness"], 0.62 + spec["band_smoothness"], bands)

            color = spec["color3"]
            color = lerp_color(color, spec["color2"], stepped_1)
            color = lerp_color(color, spec["color1"], stepped_2)

            spot_noise_a = value_noise(sample_u * 17.0 + 3.0, sample_v * 9.0 + 11.0, spec["seed"] + 59.0)
            spot_noise_b = value_noise(sample_u * 29.0 + 7.0, sample_v * 14.0 + 5.0, spec["seed"] + 83.0)
            spot_mask = smoothstep(0.60, 0.74, spot_noise_a * 0.6 + spot_noise_b * 0.4)

            if stepped_2 > 0.5:
                spot_color = spec["color2"]
            elif stepped_1 > 0.5:
                spot_color = lerp_color(spec["color1"], spec["color3"], 0.35)
            else:
                spot_color = spec["spot_color"]

            color = lerp_color(color, spot_color, spot_mask * spec["spot_amount"])

            edge = min(u, 1.0 - u, v, 1.0 - v)
            rim = 1.0 - smoothstep(0.0, 0.18, edge)
            rim_multiplier = 1.0 - rim * spec["rim_strength"]

            center_band = 1.0 - abs(v - 0.5) * 2.0
            center_band = smoothstep(0.0, 1.0, center_band)
            text_lane = 1.0 - center_band * spec["center_shadow"]

            noise_dither = (
                value_noise(sample_u * 61.0 + 4.0, sample_v * 31.0 + 13.0, spec["seed"] + 137.0) - 0.5
            ) * spec["grain_strength"]

            final = []
            for channel in color:
                shaded = channel * spec["brightness"] * rim_multiplier * text_lane + noise_dither
                final.append(int(max(0.0, min(255.0, shaded))))

            pixels[x, y] = (final[0], final[1], final[2], 255)

    return image


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    for state, spec in STATE_SPECS.items():
        image = generate_fill_image(IMAGE_SIZE[0], IMAGE_SIZE[1], spec)
        output_path = OUTPUT_DIR / f"T_UI_MainMenuBlueWoodFill_{state}.png"
        image.save(output_path)

    print(f"[BUTTONFILL] Generated {len(STATE_SPECS)} blue wood fill textures in {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
