#!/usr/bin/env python3
"""
Build a flat-color 6-part mask from a full-body statue render on transparent background.

This is a heuristic splitter intended for front-facing UI statues with:
- transparent background
- readable head / torso / arms / legs
- arms slightly separated from the torso
- legs separated enough to identify the center gap

Default output colors:
  left_leg  = #FF00FF
  right_leg = #00FFFF
  torso     = #00FF00
  left_arm  = #0000FF
  right_arm = #FFFF00
  head      = #FF0000
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List, Tuple

from PIL import Image


PART_COLORS: Dict[str, Tuple[int, int, int, int]] = {
    "left_leg": (255, 0, 255, 255),
    "right_leg": (0, 255, 255, 255),
    "torso": (0, 255, 0, 255),
    "left_arm": (0, 0, 255, 255),
    "right_arm": (255, 255, 0, 255),
    "head": (255, 0, 0, 255),
}


def row_segments(alpha: Image.Image, y: int, x0: int, x1: int, threshold: int) -> List[Tuple[int, int]]:
    segments: List[Tuple[int, int]] = []
    in_run = False
    run_start = x0
    px = alpha.load()
    for x in range(x0, x1 + 1):
        solid = px[x, y] >= threshold
        if solid and not in_run:
            in_run = True
            run_start = x
        elif not solid and in_run:
            segments.append((run_start, x - 1))
            in_run = False
    if in_run:
        segments.append((run_start, x1))
    return segments


def find_bbox(alpha: Image.Image, threshold: int) -> Tuple[int, int, int, int]:
    bbox = alpha.point(lambda v: 255 if v >= threshold else 0).getbbox()
    if bbox is None:
        raise ValueError("image has no opaque silhouette above the alpha threshold")
    return bbox


def smooth(values: List[int], radius: int) -> List[float]:
    out: List[float] = []
    for i in range(len(values)):
        a = max(0, i - radius)
        b = min(len(values), i + radius + 1)
        window = values[a:b]
        out.append(sum(window) / max(1, len(window)))
    return out


def find_head_cutoff(alpha: Image.Image, bbox: Tuple[int, int, int, int], threshold: int) -> int:
    x0, y0, x1, y1 = bbox
    height = y1 - y0 + 1
    widths: List[int] = []
    for y in range(y0, y1 + 1):
        segments = row_segments(alpha, y, x0, x1, threshold)
        widths.append(sum((b - a + 1) for a, b in segments))

    smoothed = smooth(widths, 8)
    start = int(height * 0.08)
    end = int(height * 0.38)
    search = smoothed[start:end] if end > start else smoothed
    if not search:
        return y0 + int(height * 0.2)
    local_index = min(range(len(search)), key=lambda i: search[i])
    return y0 + start + local_index


def find_leg_cutoff(alpha: Image.Image, bbox: Tuple[int, int, int, int], threshold: int) -> int:
    x0, y0, x1, y1 = bbox
    center_x = (x0 + x1) // 2
    height = y1 - y0 + 1
    start_y = y0 + int(height * 0.52)
    end_y = y0 + int(height * 0.92)

    for y in range(start_y, min(y1, end_y) + 1):
        segments = row_segments(alpha, y, x0, x1, threshold)
        if len(segments) < 2:
            continue
        center_covered = any(a <= center_x <= b for a, b in segments)
        has_left = any(b < center_x for a, b in segments)
        has_right = any(a > center_x for a, b in segments)
        if not center_covered and has_left and has_right:
            return y

    return y0 + int(height * 0.72)


def build_mask(base_path: Path, out_path: Path, alpha_threshold: int, torso_ratio: float) -> None:
    image = Image.open(base_path).convert("RGBA")
    alpha = image.getchannel("A")
    bbox = find_bbox(alpha, alpha_threshold)
    x0, y0, x1, y1 = bbox
    width = x1 - x0 + 1
    center_x = (x0 + x1) // 2

    head_cutoff = find_head_cutoff(alpha, bbox, alpha_threshold)
    leg_cutoff = max(head_cutoff + 1, find_leg_cutoff(alpha, bbox, alpha_threshold))
    torso_half_width = max(24, int(width * torso_ratio * 0.5))
    torso_left = center_x - torso_half_width
    torso_right = center_x + torso_half_width

    out = Image.new("RGBA", image.size, (0, 0, 0, 0))
    out_px = out.load()
    alpha_px = alpha.load()

    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if alpha_px[x, y] < alpha_threshold:
                continue

            if y <= head_cutoff:
                out_px[x, y] = PART_COLORS["head"]
            elif y >= leg_cutoff:
                out_px[x, y] = PART_COLORS["left_leg"] if x < center_x else PART_COLORS["right_leg"]
            else:
                if x < torso_left:
                    out_px[x, y] = PART_COLORS["left_arm"]
                elif x > torso_right:
                    out_px[x, y] = PART_COLORS["right_arm"]
                else:
                    out_px[x, y] = PART_COLORS["torso"]

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out.save(out_path)
    print(f"Wrote {out_path}")
    print(
        f"bbox={bbox} head_cutoff={head_cutoff} leg_cutoff={leg_cutoff} torso_band=({torso_left},{torso_right})"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate a flat-color body-part mask from a transparent statue render.")
    parser.add_argument("--base", required=True, type=Path, help="Input base statue PNG.")
    parser.add_argument("--out", required=True, type=Path, help="Output mask PNG.")
    parser.add_argument("--alpha-threshold", type=int, default=40, help="Alpha threshold used for the silhouette. Default: 40")
    parser.add_argument(
        "--torso-ratio",
        type=float,
        default=0.24,
        help="Approximate torso width as a fraction of silhouette width. Default: 0.24",
    )
    args = parser.parse_args()

    build_mask(
        base_path=args.base,
        out_path=args.out,
        alpha_threshold=max(1, min(255, args.alpha_threshold)),
        torso_ratio=max(0.05, min(0.6, args.torso_ratio)),
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
