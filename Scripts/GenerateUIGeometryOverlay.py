#!/usr/bin/env python3
r"""Generate a labeled UI reference-geometry overlay from a geometry table.

Input tables follow the format used by UI/Geometry/*_reference_geometry.md:

Reference image: `C:\UE\T66\UI\Screen References\Hero Selection.png`
| Tag / Region | Ref BBox | Tolerance | Notes |
| HeroSelection.TopRow | `(0.010, 0.016, 0.655, 0.070)` | `±0.010` | ... |

The bounding boxes are normalized to 1920x1080 / same-as-reference aspect.
This script draws them over the native reference image and writes a PNG.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


REFERENCE_RE = re.compile(r"^Reference image:\s+`([^`]+)`\s*$", re.IGNORECASE)
BBOX_RE = re.compile(
    r"^\|\s*(?P<tag>[^|]+?)\s*\|\s*`\((?P<x>[-0-9.]+),\s*(?P<y>[-0-9.]+),\s*(?P<w>[-0-9.]+),\s*(?P<h>[-0-9.]+)\)`\s*\|"
)


def parse_geometry_table(path: Path) -> tuple[Path | None, list[tuple[str, tuple[float, float, float, float]]]]:
    reference_path: Path | None = None
    rows: list[tuple[str, tuple[float, float, float, float]]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if reference_path is None:
            match = REFERENCE_RE.match(line.strip())
            if match:
                reference_path = Path(match.group(1))
                continue

        match = BBOX_RE.match(line.strip())
        if not match:
            continue
        tag = match.group("tag").strip()
        if tag.lower().startswith("tag"):
            continue
        rect = (
            float(match.group("x")),
            float(match.group("y")),
            float(match.group("w")),
            float(match.group("h")),
        )
        rows.append((tag, rect))
    return reference_path, rows


def load_font(size: int) -> ImageFont.ImageFont:
    for candidate in (
        Path("C:/Windows/Fonts/consola.ttf"),
        Path("C:/Windows/Fonts/arial.ttf"),
    ):
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size)
    return ImageFont.load_default()


def main() -> int:
    parser = argparse.ArgumentParser(description="Draw UI geometry boxes over a reference image.")
    parser.add_argument("--geometry", required=True, type=Path, help="Path to *_reference_geometry.md")
    parser.add_argument("--reference", type=Path, help="Reference image path. Defaults to table metadata.")
    parser.add_argument("--output", required=True, type=Path, help="Output overlay PNG path.")
    parser.add_argument("--label-size", type=int, default=14)
    args = parser.parse_args()

    table_reference, rows = parse_geometry_table(args.geometry)
    reference = args.reference or table_reference
    if reference is None:
        raise SystemExit("No --reference provided and no Reference image metadata found in table.")
    if not reference.exists():
        raise SystemExit(f"Reference image not found: {reference}")
    if not rows:
        raise SystemExit(f"No geometry rows found in {args.geometry}")

    image = Image.open(reference).convert("RGBA")
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    font = load_font(args.label_size)
    width, height = image.size

    palette = [
        (255, 80, 95, 220),
        (160, 64, 208, 220),
        (31, 179, 88, 220),
        (255, 220, 70, 220),
        (60, 200, 240, 220),
    ]

    for index, (tag, (x, y, w, h)) in enumerate(rows):
        color = palette[index % len(palette)]
        left = round(x * width)
        top = round(y * height)
        right = round((x + w) * width)
        bottom = round((y + h) * height)
        draw.rectangle((left, top, right, bottom), outline=color, width=2)
        label = tag.split(".")[-1]
        text_bbox = draw.textbbox((left + 3, top + 3), label, font=font)
        pad = 2
        draw.rectangle(
            (text_bbox[0] - pad, text_bbox[1] - pad, text_bbox[2] + pad, text_bbox[3] + pad),
            fill=(0, 0, 0, 170),
        )
        draw.text((left + 3, top + 3), label, fill=color, font=font)

    result = Image.alpha_composite(image, overlay).convert("RGB")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    result.save(args.output)
    print(f"Wrote {args.output} with {len(rows)} boxes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
