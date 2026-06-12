"""
Split approved imagegen idol rarity sheets into source idol sprites.

The preferred input is one 2x2 sheet per rarity/group:
  Idols_black_gods_a.png -> Fire DOT, AOE, Summon, Bounce
  Idols_black_gods_b.png -> Ice DOT, AOE, Summon, Bounce
  Idols_black_gods_c.png -> Electricity DOT, AOE, Summon, Bounce
  Idols_black_gods_d.png -> Nature DOT, AOE, Summon, Bounce

This script only crops and resizes generated art. It does not draw, recolor,
pixelate, or synthesize replacement imagery.
"""

import argparse
import os

from PIL import Image


RARITIES = [
    ("Black", "black"),
    ("Red", "red"),
    ("Yellow", "yellow"),
    ("White", "white"),
]

GROUPS = [
    ("gods_a", ["Idol_Fire_DOT", "Idol_Fire_AOE", "Idol_Fire_Summon", "Idol_Fire_Bounce"]),
    ("gods_b", ["Idol_Ice_DOT", "Idol_Ice_AOE", "Idol_Ice_Summon", "Idol_Ice_Bounce"]),
    ("gods_c", ["Idol_Electricity_DOT", "Idol_Electricity_AOE", "Idol_Electricity_Summon", "Idol_Electricity_Bounce"]),
    ("gods_d", ["Idol_Nature_DOT", "Idol_Nature_AOE", "Idol_Nature_Summon", "Idol_Nature_Bounce"]),
]

GRID_POSITIONS = [(0, 0), (0, 1), (1, 0), (1, 1)]
OUTPUT_SIZE = 512


def crop_cell(sheet, row, col):
    width, height = sheet.size
    left = round(col * width / 2)
    right = round((col + 1) * width / 2)
    top = round(row * height / 2)
    bottom = round((row + 1) * height / 2)
    cell = sheet.crop((left, top, right, bottom)).convert("RGBA")
    return cell.resize((OUTPUT_SIZE, OUTPUT_SIZE), Image.Resampling.LANCZOS)


def validate_outputs(paths):
    blank_count = 0
    for path in paths:
        image = Image.open(path).convert("RGBA")
        alpha_bbox = image.getchannel("A").getbbox()
        if not alpha_bbox:
            blank_count += 1
    return blank_count


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sheet-dir", required=True, help="Directory containing Idols_<rarity>_<group>.png sheets.")
    parser.add_argument("--project-dir", default=os.getcwd())
    args = parser.parse_args()

    project_dir = os.path.abspath(args.project_dir)
    sheet_dir = os.path.abspath(args.sheet_dir)
    source_root = os.path.join(project_dir, "SourceAssets", "IdolSprites")
    os.makedirs(source_root, exist_ok=True)

    written = []
    missing = []

    for rarity_folder, rarity_suffix in RARITIES:
        out_dir = os.path.join(source_root, rarity_folder)
        os.makedirs(out_dir, exist_ok=True)

        for group_name, idol_ids in GROUPS:
            sheet_path = os.path.join(sheet_dir, f"Idols_{rarity_suffix}_{group_name}.png")
            if not os.path.exists(sheet_path):
                missing.append(sheet_path)
                continue

            sheet = Image.open(sheet_path).convert("RGBA")
            for idol_id, (row, col) in zip(idol_ids, GRID_POSITIONS):
                output_path = os.path.join(out_dir, f"{idol_id}_{rarity_suffix}.png")
                crop_cell(sheet, row, col).save(output_path)
                written.append(output_path)

    if missing:
        preview = "\n".join(missing[:12])
        suffix = "" if len(missing) <= 12 else f"\n...and {len(missing) - 12} more"
        raise RuntimeError(f"Missing idol rarity sheets:\n{preview}{suffix}")

    blank_count = validate_outputs(written)
    print(f"idol_rarity_sprite_count={len(written)}")
    print(f"blank_outputs={blank_count}")
    print(f"source_root={source_root}")


if __name__ == "__main__":
    main()
