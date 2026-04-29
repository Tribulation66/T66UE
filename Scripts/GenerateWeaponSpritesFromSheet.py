"""
Build pixelated weapon sprites from a 5x4 imagegen source sheet.

The input sheet layout is:
  rows: Grey, Black, Red, Yellow, White
  cols: Pierce, Bounce, AOE, DOT

Outputs one PNG for every row in Content/Data/Weapons.csv under SourceAssets/WeaponSprites.
"""

import argparse
import csv
import os
import re
import shutil

from PIL import Image, ImageChops, ImageDraw


RARITY_ORDER = ["Grey", "Black", "Red", "Yellow", "White"]
BRANCH_ORDER = ["Pierce", "Bounce", "AOE", "DOT"]
RARITY_ACCENT = {
    "Grey": (126, 132, 142, 255),
    "Black": (34, 30, 45, 255),
    "Red": (222, 38, 38, 255),
    "Yellow": (245, 202, 56, 255),
    "White": (220, 238, 255, 255),
}
OUTPUT_SIZE = 512
PIXEL_SIZE = 128
PALETTE_COLORS = 56


def parse_color(value):
    match = re.search(
        r"R=([0-9.]+),G=([0-9.]+),B=([0-9.]+),A=([0-9.]+)",
        value or "",
        re.IGNORECASE,
    )
    if not match:
        return (142, 170, 220, 255)
    return tuple(max(0, min(255, round(float(match.group(i)) * 255))) for i in range(1, 5))


def remove_green_key(image):
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if g > 145 and r < 120 and b < 120 and g > r * 1.35 and g > b * 1.35:
                pixels[x, y] = (r, g, b, 0)
    return rgba


def crop_cell(sheet, row, col):
    width, height = sheet.size
    left = round(col * width / len(BRANCH_ORDER))
    right = round((col + 1) * width / len(BRANCH_ORDER))
    top = round(row * height / len(RARITY_ORDER))
    bottom = round((row + 1) * height / len(RARITY_ORDER))
    cell = remove_green_key(sheet.crop((left, top, right, bottom)))
    bbox = cell.getbbox()
    if not bbox:
        return Image.new("RGBA", (OUTPUT_SIZE, OUTPUT_SIZE), (0, 0, 0, 0))

    subject = cell.crop(bbox)
    max_edge = max(subject.size)
    scale = min(0.86, 440 / max_edge)
    new_size = (
        max(1, round(subject.size[0] * scale)),
        max(1, round(subject.size[1] * scale)),
    )
    subject = subject.resize(new_size, Image.Resampling.LANCZOS)

    canvas = Image.new("RGBA", (OUTPUT_SIZE, OUTPUT_SIZE), (0, 0, 0, 0))
    canvas.alpha_composite(subject, ((OUTPUT_SIZE - new_size[0]) // 2, (OUTPUT_SIZE - new_size[1]) // 2))
    return canvas


def tint_overlay(base, hero_color, rarity_color):
    image = base.copy()
    draw = ImageDraw.Draw(image, "RGBA")

    # Hero identity tab, small enough not to fight the weapon silhouette.
    tab = [
        (36, OUTPUT_SIZE - 114),
        (124, OUTPUT_SIZE - 68),
        (124, OUTPUT_SIZE - 36),
        (36, OUTPUT_SIZE - 36),
    ]
    draw.polygon(tab, fill=(*hero_color[:3], 215))
    draw.line(tab + [tab[0]], fill=(8, 8, 10, 210), width=6)

    # Rarity glint in the opposite corner. The UI border also shows rarity, but this
    # keeps the standalone sprite readable when used outside the altar card.
    draw.ellipse((390, 36, 472, 118), fill=(*rarity_color[:3], 90), outline=(*rarity_color[:3], 235), width=8)
    draw.ellipse((420, 66, 442, 88), fill=(255, 255, 255, 210))
    return image


def pixelate(image):
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    flat = Image.new("RGBA", rgba.size, (0, 0, 0, 0))
    flat.alpha_composite(rgba)
    small = flat.resize((PIXEL_SIZE, PIXEL_SIZE), Image.Resampling.BOX)
    small_alpha = alpha.resize((PIXEL_SIZE, PIXEL_SIZE), Image.Resampling.BOX)
    quantized = small.convert("RGB").quantize(colors=PALETTE_COLORS, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).convert("RGBA")
    quantized.putalpha(small_alpha.point(lambda p: 255 if p >= 32 else 0))
    return quantized.resize((OUTPUT_SIZE, OUTPUT_SIZE), Image.Resampling.NEAREST)


def validate_pixel_outputs(paths):
    max_visible_colors = 0
    block_failures = 0
    for path in paths:
        image = Image.open(path).convert("RGBA")
        pixels = image.get_flattened_data() if hasattr(image, "get_flattened_data") else image.getdata()
        visible_colors = {pixel for pixel in pixels if pixel[3] > 0}
        max_visible_colors = max(max_visible_colors, len(visible_colors))

        small = image.resize((PIXEL_SIZE, PIXEL_SIZE), Image.Resampling.NEAREST)
        upscaled = small.resize((OUTPUT_SIZE, OUTPUT_SIZE), Image.Resampling.NEAREST)
        if ImageChops.difference(image, upscaled).getbbox():
            block_failures += 1

    return max_visible_colors, block_failures


def read_heroes(project_dir):
    path = os.path.join(project_dir, "Content", "Data", "Heroes.csv")
    with open(path, newline="", encoding="utf-8-sig") as handle:
        return {row["HeroID"]: row for row in csv.DictReader(handle) if row.get("HeroID")}


def read_weapons(project_dir):
    path = os.path.join(project_dir, "Content", "Data", "Weapons.csv")
    with open(path, newline="", encoding="utf-8-sig") as handle:
        return [row for row in csv.DictReader(handle) if row.get("WeaponID")]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sheet", required=True, help="Path to the imagegen 5x4 source sheet.")
    parser.add_argument("--project-dir", default=os.getcwd())
    args = parser.parse_args()

    project_dir = os.path.abspath(args.project_dir)
    source_root = os.path.join(project_dir, "SourceAssets", "WeaponSprites")
    sheet_archive_dir = os.path.join(source_root, "_ImageGen")
    os.makedirs(source_root, exist_ok=True)
    os.makedirs(sheet_archive_dir, exist_ok=True)

    archived_sheet = os.path.join(sheet_archive_dir, "weapon_icon_master_sheet_imagegen.png")
    shutil.copy2(args.sheet, archived_sheet)

    sheet = Image.open(args.sheet).convert("RGBA")
    bases = {}
    for rarity_index, rarity in enumerate(RARITY_ORDER):
        for branch_index, branch in enumerate(BRANCH_ORDER):
            bases[(rarity, branch)] = crop_cell(sheet, rarity_index, branch_index)

    heroes = read_heroes(project_dir)
    weapons = read_weapons(project_dir)
    written = []

    for weapon in weapons:
        weapon_id = weapon["WeaponID"]
        rarity = weapon.get("Rarity") or "Grey"
        branch = weapon.get("Branch") or "Pierce"
        hero = heroes.get(weapon.get("HeroID"), {})
        hero_color = parse_color(hero.get("PlaceholderColor"))
        rarity_color = RARITY_ACCENT.get(rarity, RARITY_ACCENT["Grey"])
        base = bases.get((rarity, branch)) or bases[("Grey", "Pierce")]
        final = pixelate(tint_overlay(base, hero_color, rarity_color))
        output_path = os.path.join(source_root, f"{weapon_id}.png")
        final.save(output_path)
        written.append(output_path)

    max_visible_colors, block_failures = validate_pixel_outputs(written)

    print(f"archived_sheet={archived_sheet}")
    print(f"weapon_sprite_count={len(written)}")
    print(f"max_visible_colors={max_visible_colors}")
    print(f"block_failures={block_failures}")
    print(f"source_root={source_root}")


if __name__ == "__main__":
    main()
