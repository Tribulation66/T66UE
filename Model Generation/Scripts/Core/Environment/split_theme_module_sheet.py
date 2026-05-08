import argparse
import json
import os
from pathlib import Path

from PIL import Image, ImageColor


def parse_args():
    parser = argparse.ArgumentParser(
        description="Split coherent 2x2 theme source sheets into individual TRELLIS module PNG inputs."
    )
    parser.add_argument("--run-root", required=True, help="Run folder containing batch_manifest.json.")
    parser.add_argument("--manifest", required=True, help="Path to the coherent theme batch manifest.")
    parser.add_argument(
        "--out-dir",
        default="Inputs/approved_seed_images",
        help="Output directory relative to run-root unless absolute.",
    )
    parser.add_argument(
        "--missing-ok",
        action="store_true",
        help="Skip sheets that do not exist yet. Useful before all imagegen sheets are ready.",
    )
    return parser.parse_args()


def resolve_path(run_root, value):
    path = Path(value)
    if path.is_absolute():
        return path
    return run_root / path


def fit_to_target(image, target_width, target_height, fill):
    if image.size == (target_width, target_height):
        return image

    source_width, source_height = image.size
    scale = min(target_width / source_width, target_height / source_height)
    resized_width = max(1, round(source_width * scale))
    resized_height = max(1, round(source_height * scale))
    resized = image.resize((resized_width, resized_height), Image.Resampling.LANCZOS)

    output = Image.new("RGB", (target_width, target_height), fill)
    offset = ((target_width - resized_width) // 2, (target_height - resized_height) // 2)
    output.paste(resized, offset)
    return output


def crop_cell(sheet, rows, cols, row, col):
    width, height = sheet.size
    left = round((width * col) / cols)
    right = round((width * (col + 1)) / cols)
    top = round((height * row) / rows)
    bottom = round((height * (row + 1)) / rows)
    return sheet.crop((left, top, right, bottom))


def main():
    args = parse_args()
    run_root = Path(args.run_root)
    manifest_path = Path(args.manifest)
    if not manifest_path.is_absolute():
        manifest_path = run_root / manifest_path

    with manifest_path.open("r", encoding="utf-8") as handle:
        manifest = json.load(handle)

    out_dir = resolve_path(run_root, args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    fill = ImageColor.getrgb(manifest.get("source_sheet_background", "#00ff00"))
    written = []
    skipped = []

    for sheet_spec in manifest["sheets"]:
        source_path = resolve_path(run_root, sheet_spec["source_sheet"])
        if not source_path.exists():
            if args.missing_ok:
                skipped.append(str(source_path))
                continue
            raise FileNotFoundError(source_path)

        with Image.open(source_path) as source:
            sheet = source.convert("RGB")

        layout = sheet_spec["layout"]
        rows = int(layout["rows"])
        cols = int(layout["cols"])
        target = sheet_spec["target_size"]
        target_width = int(target["width"])
        target_height = int(target["height"])

        for module in sheet_spec["modules"]:
            row, col = module["cell"]
            module_id = module["module_id"]
            crop = crop_cell(sheet, rows, cols, int(row), int(col))
            crop = fit_to_target(crop, target_width, target_height, fill)

            output_path = out_dir / f"{module_id}.png"
            crop.save(output_path)
            written.append(str(output_path))

    summary = {
        "run_root": str(run_root),
        "manifest": str(manifest_path),
        "written_count": len(written),
        "written": written,
        "skipped_missing_count": len(skipped),
        "skipped_missing": skipped,
    }
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
