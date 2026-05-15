"""
Build contact sheets from render_arthur_action_previews.py output.

Run after rendering preview frames:
  python Tools/make_preview_contact_sheets.py --preview-root Runs/Arthur_QuadRetro_UAL_Retarget_YYYYMMDD/PreviewFrames
"""

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def label_from_action(action_name):
    for prefix in ("AM_Hero_1_Chad_QuadRetroUALQA_", "AM_Hero_1_Chad_"):
        if action_name.startswith(prefix):
            return action_name.replace(prefix, "")
    return action_name


def load_font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def paste_labeled_image(sheet, image_path, x, y, label, font):
    image = Image.open(image_path).convert("RGB")
    sheet.paste(image, (x, y + 24))
    draw = ImageDraw.Draw(sheet)
    draw.text((x + 6, y + 4), label, fill=(10, 10, 10), font=font)
    return image.size


def build_action_sheet(preview_root, item, font, view_name=None):
    if view_name:
        images = [Path(path) for path in item["views"][view_name]]
    else:
        images = [Path(path) for path in item["images"]]
    with Image.open(images[0]) as first:
        width, height = first.size
    gap = 8
    label_height = 24
    sheet = Image.new("RGB", (len(images) * width + (len(images) - 1) * gap, height + label_height), (240, 240, 240))
    for index, image_path in enumerate(images):
        frame = item["frames"][index]
        paste_labeled_image(sheet, image_path, index * (width + gap), 0, f"f{frame}", font)
    suffix = f"_{view_name}" if view_name else ""
    out_path = preview_root / f"{item['action']}{suffix}_contact_sheet.png"
    sheet.save(out_path)
    return out_path


def build_all_sheet(preview_root, items, font, view_name=None):
    action_sheets = [(item, build_action_sheet(preview_root, item, font, view_name)) for item in items]
    opened = [(item, Image.open(path).convert("RGB")) for item, path in action_sheets]
    label_width = 96
    row_gap = 8
    total_width = label_width + max(image.width for _item, image in opened)
    total_height = sum(image.height for _item, image in opened) + row_gap * (len(opened) - 1)
    sheet = Image.new("RGB", (total_width, total_height), (248, 248, 248))
    draw = ImageDraw.Draw(sheet)
    y = 0
    for item, image in opened:
        draw.text((8, y + 8), label_from_action(item["action"]), fill=(10, 10, 10), font=font)
        sheet.paste(image, (label_width, y))
        y += image.height + row_gap
    suffix = f"_{view_name}" if view_name else ""
    out_path = preview_root / f"Arthur_All_Actions{suffix}_Contact_Sheet.png"
    sheet.save(out_path)
    return out_path


def build_all_views_sheet(preview_root, view_sheets, font):
    opened = [(view, Image.open(path).convert("RGB")) for view, path in view_sheets]
    label_height = 28
    gap = 10
    total_width = max(image.width for _view, image in opened)
    total_height = sum(image.height + label_height for _view, image in opened) + gap * (len(opened) - 1)
    sheet = Image.new("RGB", (total_width, total_height), (248, 248, 248))
    draw = ImageDraw.Draw(sheet)
    y = 0
    for view, image in opened:
        draw.text((8, y + 6), view.replace("_", " ").title(), fill=(10, 10, 10), font=font)
        sheet.paste(image, (0, y + label_height))
        y += image.height + label_height + gap
    out_path = preview_root / "Arthur_All_Actions_All_Views_Contact_Sheet.png"
    sheet.save(out_path)
    return out_path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--preview-root", required=True)
    args = parser.parse_args()

    preview_root = Path(args.preview_root).resolve()
    manifest = json.loads((preview_root / "preview_manifest.json").read_text(encoding="utf-8"))
    font = load_font(13)
    if isinstance(manifest, dict) and "items" in manifest:
        items = manifest["items"]
        view_sheets = []
        for view in manifest.get("views", []):
            view_sheets.append((view, build_all_sheet(preview_root, items, font, view)))
        out_path = build_all_views_sheet(preview_root, view_sheets, font) if view_sheets else None
        print(json.dumps({"view_sheets": [str(path) for _view, path in view_sheets], "all_views_sheet": str(out_path) if out_path else ""}, indent=2))
    else:
        out_path = build_all_sheet(preview_root, manifest, font)
        print(out_path)


if __name__ == "__main__":
    main()
