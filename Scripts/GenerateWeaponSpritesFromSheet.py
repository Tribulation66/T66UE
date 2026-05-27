"""
Split an approved imagegen weapon sheet into source weapon sprites.

The source sheet layout is:
  rows: Black, Red, Yellow, White
  cols: Pierce, Bounce, AOE, DOT

This script intentionally does not draw, recolor, pixelate, or repair the art.
It only removes a flat chroma background if present, crops each generated cell,
and writes the exact centered source icons expected by ImportWeaponSpritesAndSetup.

Preferred batch mode consumes one 2x2 icon sheet per hero/rarity:
  Hero_1_black_weapons.png -> Pierce, Bounce, AOE, DOT
"""

import argparse
import csv
import os
import shutil

from PIL import Image, ImageChops


RARITY_ORDER = ["Black", "Red", "Yellow", "White"]
BRANCH_ORDER = ["Pierce", "Bounce", "AOE", "DOT"]
BRANCH_GRID_POSITIONS = {
    "Pierce": (0, 0),
    "Bounce": (0, 1),
    "AOE": (1, 0),
    "DOT": (1, 1),
}
OUTPUT_SIZE = 512
TARGET_MAX_EDGE = 460
ALPHA_THRESHOLD = 12


def remove_chroma_edges(image):
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    edge_samples = []

    for x in range(width):
        edge_samples.append(pixels[x, 0])
        edge_samples.append(pixels[x, height - 1])
    for y in range(height):
        edge_samples.append(pixels[0, y])
        edge_samples.append(pixels[width - 1, y])

    if not edge_samples:
        return rgba

    channels = []
    for index in range(3):
        values = sorted(pixel[index] for pixel in edge_samples if pixel[3] > 0)
        if not values:
            channels.append(0)
        else:
            channels.append(values[len(values) // 2])

    key = tuple(channels)
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a <= 0:
                continue
            distance = abs(r - key[0]) + abs(g - key[1]) + abs(b - key[2])
            if distance <= 48:
                pixels[x, y] = (r, g, b, 0)

    return rgba


def trim_transparent_edges(image):
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A").point(lambda value: 255 if value > ALPHA_THRESHOLD else 0)
    bbox = alpha.getbbox()
    if not bbox:
        return Image.new("RGBA", (OUTPUT_SIZE, OUTPUT_SIZE), (0, 0, 0, 0))

    subject = rgba.crop(bbox)
    max_edge = max(subject.size)
    scale = min(1.0, TARGET_MAX_EDGE / max_edge)
    new_size = (
        max(1, round(subject.size[0] * scale)),
        max(1, round(subject.size[1] * scale)),
    )
    subject = subject.resize(new_size, Image.Resampling.LANCZOS)

    canvas = Image.new("RGBA", (OUTPUT_SIZE, OUTPUT_SIZE), (0, 0, 0, 0))
    canvas.alpha_composite(subject, ((OUTPUT_SIZE - new_size[0]) // 2, (OUTPUT_SIZE - new_size[1]) // 2))
    return canvas


def crop_cell(sheet, row, col, rows, cols):
    width, height = sheet.size
    left = round(col * width / cols)
    right = round((col + 1) * width / cols)
    top = round(row * height / rows)
    bottom = round((row + 1) * height / rows)
    return trim_transparent_edges(remove_chroma_edges(sheet.crop((left, top, right, bottom))))


def read_weapons(project_dir):
    path = os.path.join(project_dir, "Content", "Data", "Weapons.csv")
    with open(path, newline="", encoding="utf-8-sig") as handle:
        return [row for row in csv.DictReader(handle) if row.get("WeaponID")]


def split_master_sheet(sheet_path, weapons):
    sheet = Image.open(sheet_path).convert("RGBA")
    bases = {}
    for rarity_index, rarity in enumerate(RARITY_ORDER):
        for branch_index, branch in enumerate(BRANCH_ORDER):
            bases[(None, rarity, branch)] = crop_cell(
                sheet,
                rarity_index,
                branch_index,
                len(RARITY_ORDER),
                len(BRANCH_ORDER),
            )

    outputs = {}
    for weapon in weapons:
        rarity = weapon.get("Rarity") or "Black"
        branch = weapon.get("Branch") or "Pierce"
        base = bases.get((None, rarity, branch)) or bases[(None, "Black", "Pierce")]
        outputs[weapon["WeaponID"]] = base
    return outputs


def split_rarity_sheets(sheet_dir, weapons):
    outputs = {}
    indexed_rows = {
        (row.get("HeroID", "").strip(), row.get("Rarity", "").strip(), row.get("Branch", "").strip()): row
        for row in weapons
        if row.get("HeroID") and row.get("Rarity") and row.get("Branch")
    }

    for hero_id in sorted({row.get("HeroID", "").strip() for row in weapons if row.get("HeroID")}):
        for rarity in RARITY_ORDER:
            sheet_path = os.path.join(sheet_dir, f"{hero_id}_{rarity.lower()}_weapons.png")
            if not os.path.exists(sheet_path):
                continue

            sheet = Image.open(sheet_path).convert("RGBA")
            for branch, (row_index, col_index) in BRANCH_GRID_POSITIONS.items():
                weapon = indexed_rows.get((hero_id, rarity, branch))
                if not weapon:
                    continue
                outputs[weapon["WeaponID"]] = crop_cell(sheet, row_index, col_index, 2, 2)

    return outputs


def validate_outputs(paths):
    blank_count = 0
    changed_by_pixel_grid = 0

    for path in paths:
        image = Image.open(path).convert("RGBA")
        if not image.getchannel("A").getbbox():
            blank_count += 1

        nearest = image.resize((128, 128), Image.Resampling.NEAREST).resize((OUTPUT_SIZE, OUTPUT_SIZE), Image.Resampling.NEAREST)
        if ImageChops.difference(image, nearest).getbbox():
            changed_by_pixel_grid += 1

    return blank_count, changed_by_pixel_grid


def main():
    parser = argparse.ArgumentParser()
    input_group = parser.add_mutually_exclusive_group(required=True)
    input_group.add_argument("--sheet", help="Path to the approved 4x4 imagegen source sheet.")
    input_group.add_argument("--sheet-dir", help="Directory of approved hero/rarity 2x2 four-icon sheets.")
    parser.add_argument("--project-dir", default=os.getcwd())
    parser.add_argument("--archive-name", default="weapon_icon_master_sheet_imagegen.png")
    args = parser.parse_args()

    project_dir = os.path.abspath(args.project_dir)
    source_root = os.path.join(project_dir, "SourceAssets", "WeaponSprites")
    sheet_archive_dir = os.path.join(source_root, "_ImageGen")
    os.makedirs(source_root, exist_ok=True)
    os.makedirs(sheet_archive_dir, exist_ok=True)

    weapons = read_weapons(project_dir)
    outputs = {}

    if args.sheet:
        archived_sheet = os.path.join(sheet_archive_dir, args.archive_name)
        shutil.copy2(args.sheet, archived_sheet)
        outputs = split_master_sheet(args.sheet, weapons)
    else:
        archived_sheet = os.path.join(sheet_archive_dir, "row_sheets")
        if os.path.exists(archived_sheet):
            shutil.rmtree(archived_sheet)
        shutil.copytree(args.sheet_dir, archived_sheet)
        outputs = split_rarity_sheets(args.sheet_dir, weapons)

    written = []
    for weapon_id, base in outputs.items():
        output_path = os.path.join(source_root, f"{weapon_id}.png")
        base.save(output_path)
        written.append(output_path)

    blank_count, changed_by_pixel_grid = validate_outputs(written)

    print(f"archived_sheet={archived_sheet}")
    print(f"weapon_sprite_count={len(written)}")
    print(f"blank_outputs={blank_count}")
    print(f"non_pixelated_outputs={changed_by_pixel_grid}")
    print(f"source_root={source_root}")


if __name__ == "__main__":
    main()
