#!/usr/bin/env python3
"""Create Mike A04 Trellis head source from built-in ImageGen output.

This keeps the useful A04 face/hair read, flattens the background to white,
and narrows the generated neck column into a centered plug for assembly tests.
"""

from __future__ import annotations

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Runs" / "Heroes" / "Chad_Pass02_ProcessBuild" / "Inputs" / "approved_seed_images" / "Hero_3_Mike_Chad_Standard_HeadOnly_White_Attempt04_Source.png"
TARGET = ROOT / "Runs" / "Heroes" / "Chad_Pass02_ProcessBuild" / "Inputs" / "preprocessed" / "Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckFullHair_A04.png"


def keep_half_width(y: int, height: int) -> float:
    # Below the jaw, taper from a visible neck into a narrower insertion plug.
    start = height * 0.585
    end = height * 0.930
    t = max(0.0, min(1.0, (y - start) / max(1.0, end - start)))
    return (0.145 - 0.035 * t) * height


def main() -> None:
    image = Image.open(SOURCE).convert("RGBA")
    width, height = image.size
    pixels = image.load()
    cx = width * 0.5
    trim_start = int(height * 0.585)

    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a < 10 or (r > 238 and g > 238 and b > 238):
                pixels[x, y] = (255, 255, 255, 255)
                continue
            if y >= trim_start and abs(x - cx) > keep_half_width(y, height):
                pixels[x, y] = (255, 255, 255, 255)

    TARGET.parent.mkdir(parents=True, exist_ok=True)
    image.save(TARGET)
    print(TARGET)


if __name__ == "__main__":
    main()
