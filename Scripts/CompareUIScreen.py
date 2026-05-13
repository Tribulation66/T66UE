#!/usr/bin/env python3
"""
Compare an in-game UI screenshot against a reference image.

Outputs:
  - <name>.side_by_side.png
  - <name>.diff.png
  - <name>.compare.json
  - <name>.compare.md
"""

from __future__ import annotations

import argparse
import json
import math
import re
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw


def sanitize_name(value: str) -> str:
    value = re.sub(r"[^A-Za-z0-9_.-]+", "_", value.strip())
    return value.strip("._") or "ui_compare"


def load_rgb(path: Path) -> Image.Image:
    return Image.open(path).convert("RGB")


def padded(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    canvas = Image.new("RGB", size, (0, 0, 0))
    canvas.paste(image, (0, 0))
    return canvas


def save_side_by_side(reference: Image.Image, actual: Image.Image, output_path: Path) -> None:
    label_height = 32
    gutter = 24
    width = reference.width + actual.width + gutter
    height = max(reference.height, actual.height) + label_height
    canvas = Image.new("RGB", (width, height), (12, 12, 14))
    draw = ImageDraw.Draw(canvas)
    draw.text((8, 8), "REFERENCE", fill=(240, 240, 240))
    draw.text((reference.width + gutter + 8, 8), "ACTUAL", fill=(240, 240, 240))
    canvas.paste(reference, (0, label_height))
    canvas.paste(actual, (reference.width + gutter, label_height))
    canvas.save(output_path)


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare a UI screenshot with a reference image.")
    parser.add_argument("--reference", required=True, type=Path)
    parser.add_argument("--actual", required=True, type=Path)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--name")
    parser.add_argument("--threshold", type=int, default=24)
    args = parser.parse_args()

    reference_path = args.reference.resolve()
    actual_path = args.actual.resolve()
    if not reference_path.exists():
        raise FileNotFoundError(f"Reference image not found: {reference_path}")
    if not actual_path.exists():
        raise FileNotFoundError(f"Actual image not found: {actual_path}")

    output_dir = (args.output_dir or actual_path.parent).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    name = sanitize_name(args.name or actual_path.stem)

    reference = load_rgb(reference_path)
    actual = load_rgb(actual_path)
    comparison_size = (max(reference.width, actual.width), max(reference.height, actual.height))
    reference_cmp = padded(reference, comparison_size)
    actual_cmp = padded(actual, comparison_size)
    diff = ImageChops.difference(reference_cmp, actual_cmp)

    threshold = max(0, min(args.threshold, 255))
    diff_pixels = 0
    max_delta = 0
    sum_delta = 0
    min_x = comparison_size[0]
    min_y = comparison_size[1]
    max_x = -1
    max_y = -1

    diff_visual = Image.new("RGB", comparison_size, (0, 0, 0))
    diff_visual_pixels = diff_visual.load()
    diff_pixels_data = diff.load()
    for y in range(comparison_size[1]):
        for x in range(comparison_size[0]):
            r, g, b = diff_pixels_data[x, y]
            delta = max(r, g, b)
            max_delta = max(max_delta, delta)
            sum_delta += delta
            if delta > threshold:
                diff_pixels += 1
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)
                intensity = max(64, delta)
                diff_visual_pixels[x, y] = (intensity, 0, 0)

    total_pixels = comparison_size[0] * comparison_size[1]
    mismatch_ratio = diff_pixels / total_pixels if total_pixels else 0.0
    mean_delta = sum_delta / total_pixels if total_pixels else 0.0
    rms_delta = math.sqrt(sum_delta / total_pixels) if total_pixels else 0.0
    bbox = None if diff_pixels == 0 else [min_x, min_y, max_x, max_y]

    side_path = output_dir / f"{name}.side_by_side.png"
    diff_path = output_dir / f"{name}.diff.png"
    json_path = output_dir / f"{name}.compare.json"
    md_path = output_dir / f"{name}.compare.md"

    save_side_by_side(reference, actual, side_path)
    diff_visual.save(diff_path)

    result = {
        "reference": str(reference_path),
        "actual": str(actual_path),
        "reference_size": [reference.width, reference.height],
        "actual_size": [actual.width, actual.height],
        "comparison_size": list(comparison_size),
        "threshold": threshold,
        "diff_pixels": diff_pixels,
        "total_pixels": total_pixels,
        "mismatch_ratio": mismatch_ratio,
        "mean_delta": mean_delta,
        "rms_delta": rms_delta,
        "max_delta": max_delta,
        "diff_bounds": bbox,
        "side_by_side": str(side_path),
        "diff": str(diff_path),
        "json": str(json_path),
        "markdown": str(md_path),
    }

    json_path.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    md_path.write_text(
        "\n".join(
            [
                f"# UI Comparison: {name}",
                "",
                f"- Reference: `{reference_path}`",
                f"- Actual: `{actual_path}`",
                f"- Reference size: `{reference.width}x{reference.height}`",
                f"- Actual size: `{actual.width}x{actual.height}`",
                f"- Threshold: `{threshold}`",
                f"- Diff pixels: `{diff_pixels}` / `{total_pixels}`",
                f"- Mismatch ratio: `{mismatch_ratio:.6f}`",
                f"- Mean max-channel delta: `{mean_delta:.3f}`",
                f"- Max channel delta: `{max_delta}`",
                f"- Diff bounds: `{bbox}`",
                f"- Side-by-side: `{side_path}`",
                f"- Diff: `{diff_path}`",
                "",
            ]
        ),
        encoding="utf-8",
    )

    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
