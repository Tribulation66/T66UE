#!/usr/bin/env python3
"""Phase 1C image/rembg preflight checks for ToonStyle Pixal3D inputs."""

from __future__ import annotations

import argparse
import json
from collections import deque
from pathlib import Path
from typing import Any

import numpy as np
from PIL import Image


def _luminance(rgb: np.ndarray) -> np.ndarray:
    values = rgb.astype(np.float32) / 255.0
    return values[..., 0] * 0.2126 + values[..., 1] * 0.7152 + values[..., 2] * 0.0722


def _corner_patches(array: np.ndarray, size: int = 32) -> list[np.ndarray]:
    h, w = array.shape[:2]
    return [
        array[:size, :size],
        array[:size, w - size :],
        array[h - size :, :size],
        array[h - size :, w - size :],
    ]


def _coverage_center(mask: np.ndarray) -> tuple[float, tuple[float, float]]:
    h, w = mask.shape
    count = int(mask.sum())
    coverage = count / float(h * w)
    if count <= 0:
        return coverage, (float("nan"), float("nan"))
    ys, xs = np.nonzero(mask)
    return coverage, (float(xs.mean() / max(w - 1, 1)), float(ys.mean() / max(h - 1, 1)))


def _largest_component_fraction(mask: np.ndarray) -> float:
    h, w = mask.shape
    total = int(mask.sum())
    if total <= 0:
        return 0.0

    visited = np.zeros(mask.shape, dtype=np.bool_)
    largest = 0
    starts = np.argwhere(mask)
    for sy, sx in starts:
        y = int(sy)
        x = int(sx)
        if visited[y, x]:
            continue
        size = 0
        queue: deque[tuple[int, int]] = deque([(y, x)])
        visited[y, x] = True
        while queue:
            cy, cx = queue.popleft()
            size += 1
            for ny, nx in ((cy - 1, cx), (cy + 1, cx), (cy, cx - 1), (cy, cx + 1)):
                if ny < 0 or ny >= h or nx < 0 or nx >= w:
                    continue
                if visited[ny, nx] or not mask[ny, nx]:
                    continue
                visited[ny, nx] = True
                queue.append((ny, nx))
        largest = max(largest, size)

    return largest / float(total)


def check_source(path: Path, expected_size: int) -> dict[str, Any]:
    result: dict[str, Any] = {"path": str(path), "checks": {}, "passed": False}
    image = Image.open(path).convert("RGBA")
    arr = np.asarray(image)
    h, w = arr.shape[:2]
    rgb = arr[..., :3]
    lum = _luminance(rgb)
    nonwhite = lum < 0.9
    coverage, center = _coverage_center(nonwhite)
    corner_lum = [float(p.mean()) for p in _corner_patches(lum)]

    checks = {
        "readable_png": True,
        "expected_dimensions": [w, h],
        "expected_dimensions_pass": w == expected_size and h == expected_size,
        "corner_luminance_means": corner_lum,
        "corners_white_pass": all(v > 0.95 for v in corner_lum),
        "nonwhite_coverage": coverage,
        "nonwhite_coverage_pass": 0.15 <= coverage <= 0.85,
        "nonwhite_center_of_mass": {"x": center[0], "y": center[1]},
        "centered_pass": 0.2 <= center[0] <= 0.8 and 0.2 <= center[1] <= 0.8,
    }
    result["checks"] = checks
    result["passed"] = all(
        bool(checks[key])
        for key in (
            "readable_png",
            "expected_dimensions_pass",
            "corners_white_pass",
            "nonwhite_coverage_pass",
            "centered_pass",
        )
    )
    return result


def check_isolated(path: Path) -> dict[str, Any]:
    result: dict[str, Any] = {"path": str(path), "checks": {}, "passed": False}
    image = Image.open(path).convert("RGBA")
    arr = np.asarray(image)
    h, w = arr.shape[:2]
    alpha = arr[..., 3]
    alpha_positive = alpha > 0
    coverage, _center = _coverage_center(alpha_positive)
    corner_max = [int(p.max()) for p in _corner_patches(alpha)]
    component_fraction = _largest_component_fraction(alpha_positive)

    checks = {
        "readable_png": True,
        "has_alpha": image.mode == "RGBA",
        "corner_alpha_max": corner_max,
        "corners_transparent_pass": all(v == 0 for v in corner_max),
        "alpha_positive_coverage": coverage,
        "alpha_coverage_pass": 0.10 <= coverage <= 0.80,
        "largest_component_fraction": component_fraction,
        "largest_component_pass": component_fraction >= 0.90,
    }
    result["checks"] = checks
    result["passed"] = all(
        bool(checks[key])
        for key in (
            "readable_png",
            "has_alpha",
            "corners_transparent_pass",
            "alpha_coverage_pass",
            "largest_component_pass",
        )
    )
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="QA a white source and isolated alpha image.")
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--isolated", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--expected-size", type=int, default=1024)
    args = parser.parse_args()

    source = check_source(args.source, args.expected_size)
    isolated = check_isolated(args.isolated)
    result = {
        "source": source,
        "isolated": isolated,
        "passed": bool(source["passed"] and isolated["passed"]),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
