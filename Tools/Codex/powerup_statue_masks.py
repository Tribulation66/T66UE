#!/usr/bin/env python3
"""
Builds power-up statue reveal assets from:
1. A fully rendered statue image on transparent background.
2. A flat-color body-part ID mask aligned to the same silhouette.

Outputs:
- one black locked silhouette
- one transparent colored layer per body part
- one transparent black cover layer per body part
- one grayscale mask per body part
- a small JSON manifest describing the outputs and unlock order

Default mask palette:
  left_leg  = #FF00FF
  right_leg = #00FFFF
  torso     = #00FF00
  left_arm  = #0000FF
  right_arm = #FFFF00
  head      = #FF0000

Example:
  python Tools/Codex/powerup_statue_masks.py ^
    --base output/imagegen/damage_statue.png ^
    --mask output/imagegen/damage_statue_mask.png ^
    --out-dir output/powerup_masks/damage ^
    --prefix damage_statue
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Sequence, Tuple

from PIL import Image


DEFAULT_ORDER: Tuple[str, ...] = (
    "left_leg",
    "right_leg",
    "torso",
    "left_arm",
    "right_arm",
    "head",
)

DEFAULT_COLORS: Dict[str, Tuple[int, int, int]] = {
    "left_leg": (255, 0, 255),
    "right_leg": (0, 255, 255),
    "torso": (0, 255, 0),
    "left_arm": (0, 0, 255),
    "right_arm": (255, 255, 0),
    "head": (255, 0, 0),
}


@dataclass(frozen=True)
class PartSpec:
    name: str
    color: Tuple[int, int, int]


def parse_hex_color(value: str) -> Tuple[int, int, int]:
    text = value.strip().lstrip("#")
    if len(text) != 6:
        raise argparse.ArgumentTypeError(f"expected RRGGBB color, got: {value!r}")
    try:
        return tuple(int(text[i : i + 2], 16) for i in (0, 2, 4))  # type: ignore[return-value]
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"invalid color: {value!r}") from exc


def parse_part_override(value: str) -> Tuple[str, Tuple[int, int, int]]:
    if "=" not in value:
        raise argparse.ArgumentTypeError("expected NAME=#RRGGBB")
    name, color_text = value.split("=", 1)
    name = name.strip().lower()
    if not name:
        raise argparse.ArgumentTypeError("part name cannot be empty")
    return name, parse_hex_color(color_text)


def clamp_u8(value: int) -> int:
    return max(0, min(255, value))


def rgb_within_tolerance(
    sample: Tuple[int, int, int], target: Tuple[int, int, int], tolerance: int
) -> bool:
    return all(abs(sample[i] - target[i]) <= tolerance for i in range(3))


def resolve_parts(overrides: Sequence[Tuple[str, Tuple[int, int, int]]]) -> List[PartSpec]:
    color_map = dict(DEFAULT_COLORS)
    for name, color in overrides:
        color_map[name] = color

    missing = [name for name in DEFAULT_ORDER if name not in color_map]
    if missing:
        raise ValueError(f"missing color definitions for: {', '.join(missing)}")

    return [PartSpec(name=name, color=color_map[name]) for name in DEFAULT_ORDER]


def ensure_rgba(image: Image.Image) -> Image.Image:
    return image if image.mode == "RGBA" else image.convert("RGBA")


def make_black_rgba(alpha_img: Image.Image) -> Image.Image:
    black = Image.new("RGBA", alpha_img.size, (0, 0, 0, 0))
    black.putalpha(alpha_img)
    return black


def write_image(path: Path, image: Image.Image) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)


def coverage_percent(numerator: int, denominator: int) -> float:
    if denominator <= 0:
        return 100.0
    return round((float(numerator) / float(denominator)) * 100.0, 2)


def build_assets(
    base_path: Path,
    mask_path: Path,
    out_dir: Path,
    prefix: str,
    tolerance: int,
    parts: Sequence[PartSpec],
    silhouette_source: str,
    emit_previews: bool,
) -> Dict[str, object]:
    base = ensure_rgba(Image.open(base_path))
    mask = ensure_rgba(Image.open(mask_path))

    if base.size != mask.size:
        raise ValueError(
            f"image sizes do not match: base={base.size}, mask={mask.size}"
        )

    width, height = base.size
    base_px = base.load()
    mask_px = mask.load()

    part_masks: Dict[str, Image.Image] = {
        part.name: Image.new("L", base.size, 0) for part in parts
    }
    part_mask_px = {name: img.load() for name, img in part_masks.items()}

    silhouette = Image.new("L", base.size, 0)
    silhouette_px = silhouette.load()

    matched_mask_pixels = 0
    base_alpha_pixels = 0
    matched_base_pixels = 0

    for y in range(height):
        for x in range(width):
            _, _, _, base_a = base_px[x, y]
            mask_r, mask_g, mask_b, mask_a = mask_px[x, y]
            if base_a > 0:
                base_alpha_pixels += 1

            matched_part: PartSpec | None = None
            if mask_a > 0:
                sample_rgb = (mask_r, mask_g, mask_b)
                for part in parts:
                    if rgb_within_tolerance(sample_rgb, part.color, tolerance):
                        matched_part = part
                        break

            matched_alpha = 0
            if matched_part is not None:
                matched_mask_pixels += 1
                matched_alpha = min(base_a, mask_a) if base_a > 0 else mask_a
                part_mask_px[matched_part.name][x, y] = matched_alpha
                if base_a > 0:
                    matched_base_pixels += 1

            if silhouette_source == "base":
                silhouette_alpha = base_a
            elif silhouette_source == "mask":
                silhouette_alpha = matched_alpha
            else:
                silhouette_alpha = max(base_a, matched_alpha)

            silhouette_px[x, y] = clamp_u8(silhouette_alpha)

    locked_path = out_dir / f"{prefix}_locked.png"
    write_image(locked_path, make_black_rgba(silhouette))

    manifest_parts: List[Dict[str, object]] = []
    for unlock_index, part in enumerate(parts):
        mask_img = part_masks[part.name]
        layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
        cover = make_black_rgba(mask_img)

        for y in range(height):
            for x in range(width):
                alpha = mask_img.getpixel((x, y))
                if alpha <= 0:
                    continue
                r, g, b, base_a = base_px[x, y]
                layer.putpixel((x, y), (r, g, b, min(alpha, base_a if base_a > 0 else alpha)))

        layer_path = out_dir / f"{prefix}_{part.name}.png"
        cover_path = out_dir / f"{prefix}_{part.name}_cover.png"
        alpha_path = out_dir / f"{prefix}_{part.name}_mask.png"

        write_image(layer_path, layer)
        write_image(cover_path, cover)
        write_image(alpha_path, mask_img)

        manifest_parts.append(
            {
                "name": part.name,
                "unlock_index": unlock_index,
                "mask_color": "#{:02X}{:02X}{:02X}".format(*part.color),
                "layer_file": layer_path.name,
                "cover_file": cover_path.name,
                "mask_file": alpha_path.name,
            }
        )

    if emit_previews:
        for revealed_count in range(len(parts) + 1):
            preview = make_black_rgba(silhouette)
            preview_px = preview.load()
            for part in parts[:revealed_count]:
                mask_img = part_masks[part.name]
                for y in range(height):
                    for x in range(width):
                        alpha = mask_img.getpixel((x, y))
                        if alpha <= 0:
                            continue
                        r, g, b, base_a = base_px[x, y]
                        preview_px[x, y] = (r, g, b, min(alpha, base_a if base_a > 0 else alpha))

            write_image(out_dir / f"{prefix}_preview_{revealed_count}.png", preview)

    manifest = {
        "base_file": str(base_path),
        "mask_file": str(mask_path),
        "size": {"width": width, "height": height},
        "unlock_order": list(DEFAULT_ORDER),
        "parts": manifest_parts,
        "locked_file": locked_path.name,
        "coverage": {
            "base_alpha_pixels": base_alpha_pixels,
            "matched_base_pixels": matched_base_pixels,
            "matched_mask_pixels": matched_mask_pixels,
            "base_coverage_percent": coverage_percent(matched_base_pixels, base_alpha_pixels),
        },
    }

    manifest_path = out_dir / f"{prefix}_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    return manifest


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate layered statue reveal assets from a base render and a flat-color mask.",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument("--base", required=True, type=Path, help="Path to the final statue PNG.")
    parser.add_argument("--mask", required=True, type=Path, help="Path to the flat-color part mask PNG.")
    parser.add_argument("--out-dir", required=True, type=Path, help="Directory for generated outputs.")
    parser.add_argument(
        "--prefix",
        default="statue",
        help="Filename prefix for generated assets.",
    )
    parser.add_argument(
        "--tolerance",
        type=int,
        default=16,
        help="Per-channel RGB tolerance for matching mask colors. Default: 16",
    )
    parser.add_argument(
        "--silhouette-source",
        choices=("base", "mask", "union"),
        default="union",
        help="Alpha source for the locked silhouette. Default: union",
    )
    parser.add_argument(
        "--part",
        action="append",
        default=[],
        type=parse_part_override,
        help=(
            "Override a default part color. Repeat as needed.\n"
            "Example: --part head=#FF0000 --part torso=#00FF00"
        ),
    )
    parser.add_argument(
        "--emit-previews",
        action="store_true",
        help="Also write preview_0..preview_6 composite images.",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    parts = resolve_parts(args.part)
    manifest = build_assets(
        base_path=args.base,
        mask_path=args.mask,
        out_dir=args.out_dir,
        prefix=args.prefix,
        tolerance=max(0, min(255, args.tolerance)),
        parts=parts,
        silhouette_source=args.silhouette_source,
        emit_previews=args.emit_previews,
    )

    coverage = manifest["coverage"]
    print(f"Wrote assets to: {args.out_dir}")
    print(f"Locked silhouette: {manifest['locked_file']}")
    print(f"Base coverage from mask: {coverage['base_coverage_percent']}%")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
