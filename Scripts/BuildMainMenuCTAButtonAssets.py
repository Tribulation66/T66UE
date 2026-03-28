from pathlib import Path
import math

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "SourceAssets" / "UI" / "MainMenuGenerated"
OUT_PATH = OUT_DIR / "mainmenu_cta_fill_green.png"


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def smoothstep(a: float, b: float, x: float) -> float:
    if x <= a:
        return 0.0
    if x >= b:
        return 1.0
    t = (x - a) / (b - a)
    return t * t * (3.0 - 2.0 * t)


def generate_button_fill() -> Image.Image:
    width, height = 1024, 232
    image = Image.new("RGBA", (width, height))

    top = (52, 85, 74)
    middle = (96, 141, 120)
    bottom = (46, 79, 69)
    edge_tint = (38, 67, 59)

    for y in range(height):
        vy = y / float(height - 1)
        if vy < 0.52:
            vertical_t = smoothstep(0.0, 0.52, vy)
            base = [lerp(top[i], middle[i], vertical_t) for i in range(3)]
        else:
            vertical_t = smoothstep(0.52, 1.0, vy)
            base = [lerp(middle[i], bottom[i], vertical_t) for i in range(3)]

        top_gloss = max(0.0, 1.0 - (vy / 0.22))
        bottom_shade = max(0.0, (vy - 0.72) / 0.28)

        for x in range(width):
            vx = x / float(width - 1)
            edge_distance = abs(vx - 0.5) * 2.0
            vignette = smoothstep(0.18, 1.0, edge_distance)
            color = [lerp(base[i], edge_tint[i], vignette * 0.55) for i in range(3)]

            bloom_x = (vx - 0.5) / 0.58
            bloom_y = (vy - 0.50) / 0.85
            bloom = max(0.0, 1.0 - ((bloom_x * bloom_x) + (bloom_y * bloom_y)))
            color = [color[i] + bloom * (16.0 if i != 1 else 22.0) for i in range(3)]

            ripple = 0.5 + (0.5 * math.sin((vx * math.pi * 3.2) + (vy * math.pi * 0.9)))
            ripple2 = 0.5 + (0.5 * math.sin((vx * math.pi * 7.5) - (vy * math.pi * 1.7)))
            micro = (ripple * 0.8) + (ripple2 * 0.2) - 0.5
            color = [color[i] + micro * (2.2 if i != 1 else 3.0) for i in range(3)]

            color = [color[i] + (top_gloss * 8.0) for i in range(3)]
            color = [color[i] - (bottom_shade * 9.0) for i in range(3)]

            image.putpixel(
                (x, y),
                tuple(max(0, min(255, int(round(channel)))) for channel in color) + (255,))

    for y in range(2):
        boost = 22 - (y * 7)
        for x in range(width):
            red, green, blue, _ = image.getpixel((x, y))
            image.putpixel(
                (x, y),
                (min(255, red + boost), min(255, green + boost), min(255, blue + boost), 255))

    for x in range(width):
        for y in range(height):
            border_distance = min(x, width - 1 - x, y, height - 1 - y)
            red, green, blue, _ = image.getpixel((x, y))
            if border_distance == 0:
                image.putpixel((x, y), (max(0, red - 14), max(0, green - 10), max(0, blue - 8), 255))
            elif border_distance == 1:
                image.putpixel((x, y), (min(255, red + 5), min(255, green + 6), min(255, blue + 4), 255))

    return image


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    generate_button_fill().save(OUT_PATH)
    print(OUT_PATH)


if __name__ == "__main__":
    main()
