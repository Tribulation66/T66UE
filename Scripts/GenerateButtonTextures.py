"""
Generate button texture PNGs for T66 UI (Dark/Light x Normal/Hovered/Pressed).
Each texture has: rounded plate, gradient border, outline, gloss highlight.

Requires: pip install Pillow
Run:      python Scripts/GenerateButtonTextures.py

Output:   SourceAssets/Images/Buttons/ButtonDark_N.png  (etc.)
Import:   Run Scripts/ImportButtonTextures.py in the Unreal Editor to create UTexture2D assets.
"""

import os
from PIL import Image, ImageDraw

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
OUT_DIR = os.path.join(PROJECT_DIR, "SourceAssets", "Images", "Buttons")

# --- Texture dimensions ---
WIDTH = 512
HEIGHT = 128

# --- Shape settings (pixels) ---
CORNER_RADIUS = 12
OUTLINE_WIDTH = 2
BORDER_WIDTH = 8

# --- Gloss band (fraction of texture height) ---
GLOSS_TOP = 0.08
GLOSS_BOTTOM = 0.38


def _lerp(a, b, t):
    return int(a + (b - a) * t)


def _lerp_rgb(c1, c2, t):
    return (_lerp(c1[0], c2[0], t), _lerp(c1[1], c2[1], t), _lerp(c1[2], c2[2], t))


def generate_button(filepath, plate_rgb, border_top_rgb, border_bottom_rgb,
                    outline_rgb, gloss_strength):
    """Render one button texture: outline -> gradient border -> plate -> gloss."""
    img = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))

    # ---- 1. Outline (outermost rounded rect) ----
    outline_layer = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    ImageDraw.Draw(outline_layer).rounded_rectangle(
        [0, 0, WIDTH - 1, HEIGHT - 1],
        radius=CORNER_RADIUS,
        fill=(*outline_rgb, 255),
    )
    img = Image.alpha_composite(img, outline_layer)

    # ---- 2. Gradient border (inset by OUTLINE_WIDTH) ----
    i1 = OUTLINE_WIDTH
    r1 = max(1, CORNER_RADIUS - i1)
    # Build a mask for the border zone (rounded rect)
    border_mask = Image.new("L", (WIDTH, HEIGHT), 0)
    ImageDraw.Draw(border_mask).rounded_rectangle(
        [i1, i1, WIDTH - 1 - i1, HEIGHT - 1 - i1], radius=r1, fill=255
    )
    # Gradient fill (top color -> bottom color, row by row)
    border_fill = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    for y in range(i1, HEIGHT - i1):
        t = (y - i1) / max(1, HEIGHT - 1 - 2 * i1)
        c = _lerp_rgb(border_top_rgb, border_bottom_rgb, t)
        ImageDraw.Draw(border_fill).line([(0, y), (WIDTH - 1, y)], fill=(*c, 255))
    border_fill.putalpha(border_mask)
    img = Image.alpha_composite(img, border_fill)

    # ---- 3. Plate fill (inset by OUTLINE_WIDTH + BORDER_WIDTH) ----
    i2 = OUTLINE_WIDTH + BORDER_WIDTH
    r2 = max(1, CORNER_RADIUS - i2)
    plate_layer = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    ImageDraw.Draw(plate_layer).rounded_rectangle(
        [i2, i2, WIDTH - 1 - i2, HEIGHT - 1 - i2], radius=r2, fill=(*plate_rgb, 255)
    )
    img = Image.alpha_composite(img, plate_layer)

    # ---- 4. Gloss highlight (fading white band on upper plate area) ----
    if gloss_strength > 0:
        gy0 = int(HEIGHT * GLOSS_TOP)
        gy1 = int(HEIGHT * GLOSS_BOTTOM)
        peak_alpha = int(255 * min(gloss_strength, 1.0))
        gloss_layer = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
        draw_g = ImageDraw.Draw(gloss_layer)
        for y in range(max(i2, gy0), min(HEIGHT - i2, gy1)):
            progress = (y - gy0) / max(1, gy1 - gy0)
            alpha = int(peak_alpha * (1.0 - progress * progress))  # quadratic fade
            draw_g.line([(i2 + 2, y), (WIDTH - 1 - i2 - 2, y)], fill=(255, 255, 255, alpha))
        img = Image.alpha_composite(img, gloss_layer)

    # ---- 5. Subtle bevel: thin bright line at plate top, thin dark line at plate bottom ----
    bevel = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    draw_b = ImageDraw.Draw(bevel)
    # Top bevel (bright)
    for dy in range(2):
        draw_b.line(
            [(i2 + r2, i2 + dy), (WIDTH - 1 - i2 - r2, i2 + dy)],
            fill=(255, 255, 255, 40),
        )
    # Bottom bevel (dark)
    for dy in range(2):
        draw_b.line(
            [(i2 + r2, HEIGHT - 1 - i2 - dy), (WIDTH - 1 - i2 - r2, HEIGHT - 1 - i2 - dy)],
            fill=(0, 0, 0, 50),
        )
    img = Image.alpha_composite(img, bevel)

    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    img.save(filepath, "PNG")
    print(f"  Created: {os.path.relpath(filepath, PROJECT_DIR)}")


def main():
    print("=== Generate T66 Button Textures ===")
    os.makedirs(OUT_DIR, exist_ok=True)

    # Colors derived from SourceAssets/Data/ButtonProcedural_MaterialInstances.json
    # (linear -> approximate sRGB for visual match)
    configs = [
        # (filename,            plate,          border_top,      border_bottom,   outline,         gloss)
        ("ButtonDark_N.png",   (18, 18, 20),   (255, 255, 255), (210, 210, 210), (255, 255, 255), 0.35),
        ("ButtonDark_H.png",   (28, 28, 30),   (255, 255, 255), (220, 220, 220), (255, 255, 255), 0.48),
        ("ButtonDark_P.png",   (10, 10, 12),   (240, 240, 240), (195, 195, 195), (255, 255, 255), 0.18),
        ("ButtonLight_N.png",  (240, 240, 240), (55, 55, 55),    (20, 20, 20),    (10, 10, 10),    0.30),
        ("ButtonLight_H.png",  (248, 248, 248), (55, 55, 55),    (20, 20, 20),    (10, 10, 10),    0.42),
        ("ButtonLight_P.png",  (228, 228, 228), (70, 70, 70),    (20, 20, 20),    (10, 10, 10),    0.15),
    ]

    for name, plate, btop, bbot, outline, gloss in configs:
        generate_button(os.path.join(OUT_DIR, name), plate, btop, bbot, outline, gloss)

    print(f"\n  Output directory: {os.path.relpath(OUT_DIR, PROJECT_DIR)}")
    print("  Next: run Scripts/ImportButtonTextures.py in the Unreal Editor to import as UTexture2D assets.")
    print("=== Done ===")


if __name__ == "__main__":
    main()
