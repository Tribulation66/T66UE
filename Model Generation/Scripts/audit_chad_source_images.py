#!/usr/bin/env python3
"""Audit Chad source images before they are allowed into TRELLIS."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from PIL import Image, ImageDraw


PART_RE = re.compile(r"_(BodyOnly|HeadOnly|WeaponOnly)_", re.IGNORECASE)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-dir", required=True)
    parser.add_argument("--output-json", required=True)
    parser.add_argument("--expected-bg", choices=("white", "green"), default="white")
    parser.add_argument("--allow-square-bodies", action="store_true")
    parser.add_argument("--internal-green-fail-pct", type=float)
    return parser.parse_args()


def median_channel(values: list[int]) -> int:
    values.sort()
    return values[len(values) // 2]


def color_distance(a: tuple[int, int, int], b: tuple[int, int, int]) -> float:
    return sum((int(a[i]) - int(b[i])) ** 2 for i in range(3)) ** 0.5


def classify_part(path: Path) -> str:
    match = PART_RE.search(path.name)
    return match.group(1).lower().replace("only", "") if match else "unknown"


def border_pixels(rgb: Image.Image) -> list[tuple[int, int, int]]:
    pixels = rgb.load()
    width, height = rgb.size
    coords: list[tuple[int, int]] = []
    for x in range(width):
        coords.append((x, 0))
        coords.append((x, height - 1))
    for y in range(height):
        coords.append((0, y))
        coords.append((width - 1, y))
    return [pixels[x, y] for x, y in coords]


def background_median(samples: list[tuple[int, int, int]]) -> tuple[int, int, int]:
    return tuple(median_channel([color[i] for color in samples]) for i in range(3))


def is_expected_background(color: tuple[int, int, int], expected: str) -> bool:
    r, g, b = color
    if expected == "white":
        return r >= 245 and g >= 245 and b >= 245
    return g >= 225 and r <= 45 and b <= 45


def flood_connected_background(rgb: Image.Image, bg: tuple[int, int, int]) -> Image.Image:
    # Source QA is a rejection gate, not a matte generator. Run the connected
    # background check on a downsampled copy so full batch audits stay quick.
    if max(rgb.size) > 256:
        rgb = rgb.resize((256, 256), Image.Resampling.NEAREST)
    width, height = rgb.size
    filled = rgb.copy()
    marker = (255, 0, 255)
    starts = [
        (0, 0),
        (width - 1, 0),
        (0, height - 1),
        (width - 1, height - 1),
        (width // 2, 0),
        (width // 2, height - 1),
        (0, height // 2),
        (width - 1, height // 2),
    ]
    pixels = rgb.load()
    for xy in starts:
        if color_distance(pixels[xy[0], xy[1]], bg) <= 42:
            ImageDraw.floodfill(filled, xy, marker, thresh=42)
    return filled


def image_data(image: Image.Image):
    if hasattr(image, "get_flattened_data"):
        return image.get_flattened_data()
    return image.getdata()


def audit_image(
    path: Path,
    expected_bg: str,
    allow_square_bodies: bool,
    internal_green_fail_pct: float,
) -> dict[str, object]:
    image = Image.open(path)
    rgba = image.convert("RGBA")
    rgb = rgba.convert("RGB")
    width, height = rgb.size
    part = classify_part(path)
    failures: list[str] = []
    warnings: list[str] = []

    border = border_pixels(rgb)
    bg = background_median(border)
    nonuniform_gt10 = sum(1 for color in border if color_distance(color, bg) > 10) / len(border) * 100.0
    nonuniform_gt25 = sum(1 for color in border if color_distance(color, bg) > 25) / len(border) * 100.0

    if rgba.getextrema()[3][0] < 255:
        failures.append("alpha pixels present; TRELLIS source must be opaque")
    if not is_expected_background(bg, expected_bg):
        failures.append(f"border median is not expected {expected_bg}: {bg}")
    if nonuniform_gt25 > 1.0:
        failures.append(f"border is not flat enough: {nonuniform_gt25:.3f}% pixels differ by >25")

    if part == "body":
        if height / max(width, 1) < 1.30 and not allow_square_bodies:
            failures.append("body source is not portrait enough; square body fallbacks need manual approval")
    elif part == "head":
        if width < 900 or height < 900:
            warnings.append("head source is below 900px on one axis")

    scan_rgb = rgb.resize((256, 256), Image.Resampling.NEAREST) if max(rgb.size) > 256 else rgb
    filled = flood_connected_background(scan_rgb, bg)
    internal_green = 0
    internal_nonwhite_bg = 0
    for color, filled_color in zip(image_data(scan_rgb), image_data(filled)):
        if filled_color == (255, 0, 255):
            continue
        r, g, b = color
        if g > 180 and r < 90 and b < 90:
            internal_green += 1
        if expected_bg == "white" and r > 235 and g > 235 and b > 235:
            internal_nonwhite_bg += 1

    total = scan_rgb.size[0] * scan_rgb.size[1]
    internal_green_pct = internal_green / total * 100.0
    internal_white_pct = internal_nonwhite_bg / total * 100.0
    if internal_green_pct > internal_green_fail_pct:
        failures.append(f"internal green contamination: {internal_green_pct:.3f}% of pixels")
    if expected_bg == "white" and internal_white_pct > 2.0:
        warnings.append(f"large disconnected white interior regions: {internal_white_pct:.3f}% of pixels")

    return {
        "file": str(path),
        "part": part,
        "size": [width, height],
        "border_median_rgb": list(bg),
        "border_nonuniform_pct_gt10": round(nonuniform_gt10, 3),
        "border_nonuniform_pct_gt25": round(nonuniform_gt25, 3),
        "internal_green_pct": round(internal_green_pct, 3),
        "internal_disconnected_white_pct": round(internal_white_pct, 3),
        "status": "fail" if failures else "pass",
        "failures": failures,
        "warnings": warnings,
    }


def main() -> None:
    args = parse_args()
    input_dir = Path(args.input_dir)
    files = sorted(input_dir.glob("*.png"))
    if not files:
        raise SystemExit(f"No PNG files found in {input_dir}")

    results = [
        audit_image(
            path,
            args.expected_bg,
            args.allow_square_bodies,
            args.internal_green_fail_pct
            if args.internal_green_fail_pct is not None
            else (0.02 if args.expected_bg == "white" else 0.20),
        )
        for path in files
    ]
    payload = {
        "input_dir": str(input_dir),
        "expected_bg": args.expected_bg,
        "total": len(results),
        "failed": sum(1 for item in results if item["status"] == "fail"),
        "results": results,
    }
    output = Path(args.output_json)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(payload, indent=2), encoding="ascii")
    print(f"audited={payload['total']} failed={payload['failed']} output={output}")


if __name__ == "__main__":
    main()
