#!/usr/bin/env python3
"""Bake grayscale inner line textures from curvature-tagged UV segments."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

from texture_postprocess import pad_uv_islands, rasterize_uv_mask


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Bake ToonStyle inner line mask.")
	parser.add_argument("--segments", required=True, type=Path)
	parser.add_argument("--uv-triangles", required=True, type=Path)
	parser.add_argument("--output", required=True, type=Path)
	parser.add_argument("--size", type=int, default=4096)
	parser.add_argument("--padding-pixels", type=int, default=16)
	parser.add_argument("--report", required=True, type=Path)
	return parser.parse_args()


def _to_xy(u: float, v: float, size: int) -> tuple[int, int]:
	return (
		int(round(float(np.clip(u, 0.0, 1.0)) * (size - 1))),
		int(round((1.0 - float(np.clip(v, 0.0, 1.0))) * (size - 1))),
	)


def bake_segments(segments: np.ndarray, size: int) -> Image.Image:
	image = Image.new("L", (size, size), 0)
	draw = ImageDraw.Draw(image)
	line_width = max(1, size // 1024)
	for u0, v0, u1, v1, intensity in segments:
		value = int(round(96 + (float(np.clip(intensity, 0.0, 1.0)) * 159)))
		draw.line([_to_xy(u0, v0, size), _to_xy(u1, v1, size)], fill=value, width=line_width)
	# A very small blur keeps the mask from being binary-jagged while retaining
	# sparse linework. This is not a visual sign-off decision; it creates stable
	# non-stale baseline content for later tuning.
	return image.filter(ImageFilter.GaussianBlur(radius=max(0.35, line_width * 0.20)))


def stats_for(image: Image.Image) -> dict[str, object]:
	arr = np.asarray(image, dtype=np.uint8)
	normalized = arr.astype(np.float32) / 255.0
	return {
		"resolution": [int(arr.shape[1]), int(arr.shape[0])],
		"unique_value_count": int(np.unique(arr).size),
		"line_coverage_fraction": float(np.mean(normalized > 0.05)),
		"max_intensity": float(normalized.max()) if normalized.size else 0.0,
		"stddev": float(normalized.std()) if normalized.size else 0.0,
	}


def main() -> int:
	args = parse_args()
	segments = np.load(args.segments)["segments"]
	uv_triangles = np.load(args.uv_triangles)["triangles"]
	image = bake_segments(segments, args.size)
	rgba = np.zeros((args.size, args.size, 4), dtype=np.uint8)
	line = np.asarray(image, dtype=np.uint8)
	rgba[:, :, 0] = line
	rgba[:, :, 1] = line
	rgba[:, :, 2] = line
	rgba[:, :, 3] = 255
	mask = rasterize_uv_mask(uv_triangles, args.size, args.size)
	padded, padding_report = pad_uv_islands(rgba, mask, args.padding_pixels)
	out_image = Image.fromarray(padded[:, :, 0], mode="L")
	args.output.parent.mkdir(parents=True, exist_ok=True)
	out_image.save(args.output)
	report = stats_for(out_image)
	report.update(
		{
			"segments": int(len(segments)),
			"padding": padding_report,
			"dependency_path": "external_python_pillow_numpy",
			"output": str(args.output),
		}
	)
	args.report.parent.mkdir(parents=True, exist_ok=True)
	args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
	print(json.dumps(report, sort_keys=True))
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
