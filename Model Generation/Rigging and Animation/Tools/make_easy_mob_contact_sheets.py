"""
Build contact sheets from create_easy_mob_vat_sources.py preview output.

Run:
  python "Model Generation/Rigging and Animation/Tools/make_easy_mob_contact_sheets.py" --preview-root "Model Generation/Rigging and Animation/Runs/Easy_Mob_VAT_20260514/PreviewFrames"
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def load_font(size: int):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def paste_label(draw: ImageDraw.ImageDraw, xy, text: str, font) -> None:
    draw.rectangle([xy[0] - 2, xy[1] - 2, xy[0] + len(text) * 7 + 8, xy[1] + 18], fill=(248, 248, 248))
    draw.text(xy, text, fill=(10, 10, 10), font=font)


def build_item_sheet(preview_root: Path, item: dict, view_name: str, font) -> Path:
    images = [Path(path) for path in item["views"][view_name]]
    opened = [Image.open(path).convert("RGB") for path in images]
    width, height = opened[0].size
    label_h = 26
    gap = 8
    sheet = Image.new("RGB", (len(opened) * width + (len(opened) - 1) * gap, height + label_h), (238, 238, 238))
    draw = ImageDraw.Draw(sheet)
    for index, image in enumerate(opened):
        x = index * (width + gap)
        sheet.paste(image, (x, label_h))
        paste_label(draw, (x + 6, 5), f"f{item['frames'][index]}", font)
    out_dir = preview_root / item["enemy_id"]
    out_path = out_dir / f"{item['enemy_id']}_{item['clip']}_{view_name}_Contact_Sheet.png"
    sheet.save(out_path)
    return out_path


def build_enemy_view_sheet(preview_root: Path, items: list[dict], enemy_id: str, view_name: str, font) -> Path:
    enemy_items = [item for item in items if item["enemy_id"] == enemy_id]
    rows = [(item, Image.open(build_item_sheet(preview_root, item, view_name, font)).convert("RGB")) for item in enemy_items]
    label_w = 106
    gap = 8
    width = label_w + max(image.width for _item, image in rows)
    height = sum(image.height for _item, image in rows) + gap * (len(rows) - 1)
    sheet = Image.new("RGB", (width, height), (248, 248, 248))
    draw = ImageDraw.Draw(sheet)
    y = 0
    for item, image in rows:
        paste_label(draw, (8, y + 10), item["clip"], font)
        sheet.paste(image, (label_w, y))
        y += image.height + gap
    out_path = preview_root / enemy_id / f"{enemy_id}_{view_name}_AllClips_Contact_Sheet.png"
    sheet.save(out_path)
    return out_path


def build_enemy_all_views_sheet(preview_root: Path, view_sheets: list[tuple[str, Path]], enemy_id: str, font) -> Path:
    opened = [(view, Image.open(path).convert("RGB")) for view, path in view_sheets]
    label_h = 28
    gap = 10
    width = max(image.width for _view, image in opened)
    height = sum(image.height + label_h for _view, image in opened) + gap * (len(opened) - 1)
    sheet = Image.new("RGB", (width, height), (248, 248, 248))
    draw = ImageDraw.Draw(sheet)
    y = 0
    for view, image in opened:
        paste_label(draw, (8, y + 6), view.replace("_", " ").title(), font)
        sheet.paste(image, (0, y + label_h))
        y += image.height + label_h + gap
    out_path = preview_root / enemy_id / f"{enemy_id}_AllClips_AllViews_Contact_Sheet.png"
    sheet.save(out_path)
    return out_path


def build_index_sheet(preview_root: Path, all_view_sheets: list[Path], font) -> Path:
    thumbs = []
    for path in all_view_sheets:
        image = Image.open(path).convert("RGB")
        image.thumbnail((360, 760))
        thumbs.append((path.parent.name, image.copy()))
    cols = 2
    gap = 14
    label_h = 26
    cell_w = max(image.width for _name, image in thumbs)
    cell_h = max(image.height for _name, image in thumbs) + label_h
    rows = (len(thumbs) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * cell_w + (cols - 1) * gap, rows * cell_h + (rows - 1) * gap), (248, 248, 248))
    draw = ImageDraw.Draw(sheet)
    for index, (enemy_id, image) in enumerate(thumbs):
        col = index % cols
        row = index // cols
        x = col * (cell_w + gap)
        y = row * (cell_h + gap)
        paste_label(draw, (x + 6, y + 5), enemy_id, font)
        sheet.paste(image, (x, y + label_h))
    out_path = preview_root / "Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png"
    sheet.save(out_path)
    return out_path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--preview-root", required=True)
    args = parser.parse_args()
    preview_root = Path(args.preview_root).resolve()
    manifest = json.loads((preview_root / "preview_manifest.json").read_text(encoding="utf-8"))
    font = load_font(13)
    enemy_ids = []
    for item in manifest["items"]:
        if item["enemy_id"] not in enemy_ids:
            enemy_ids.append(item["enemy_id"])
    all_view_sheets = []
    for enemy_id in enemy_ids:
        view_sheets = []
        for view in manifest.get("views", []):
            view_sheets.append((view, build_enemy_view_sheet(preview_root, manifest["items"], enemy_id, view, font)))
        all_view_sheets.append(build_enemy_all_views_sheet(preview_root, view_sheets, enemy_id, font))
    index_sheet = build_index_sheet(preview_root, all_view_sheets, font)
    payload = {
        "enemy_sheets": [str(path) for path in all_view_sheets],
        "index_sheet": str(index_sheet),
    }
    (preview_root / "contact_sheet_manifest.json").write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(json.dumps(payload, indent=2))


if __name__ == "__main__":
    main()
