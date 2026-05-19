#!/usr/bin/env python3
"""Flatten Pixal3D diffuse textures into a small matte color set.

The module is intentionally usable both inside Blender and from normal Python.
Inside Blender it avoids PIL so the pipeline can run in Blender's bundled
Python, which currently has numpy but not Pillow.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import numpy as np


def _compress_highlights(rgb: np.ndarray, cap: float) -> np.ndarray:
	if cap <= 0.0 or cap >= 1.0:
		return rgb
	luma = (rgb[:, 0] * 0.2126) + (rgb[:, 1] * 0.7152) + (rgb[:, 2] * 0.0722)
	mask = luma > cap
	if not np.any(mask):
		return rgb
	result = rgb.copy()
	scale = np.ones_like(luma)
	# Smoothly pull hot pixels toward the cap rather than hard-clamping channels.
	scale[mask] = (cap + ((luma[mask] - cap) * 0.25)) / np.maximum(luma[mask], 1e-6)
	result[:, :3] = np.clip(result[:, :3] * scale[:, None], 0.0, 1.0)
	return result


def _initial_centers(pixels: np.ndarray, k: int) -> np.ndarray:
	# Deterministic quantile initialization is stable enough for reports and avoids
	# a dependency on scikit-learn inside Blender.
	luma = (pixels[:, 0] * 0.2126) + (pixels[:, 1] * 0.7152) + (pixels[:, 2] * 0.0722)
	order = np.argsort(luma)
	if len(order) == 0:
		raise RuntimeError("Cannot cluster empty texture")
	indices = np.linspace(0, len(order) - 1, k).round().astype(np.int64)
	return pixels[order[indices], :3].astype(np.float32, copy=True)


def _mini_batch_kmeans(pixels: np.ndarray, k: int, max_iter: int = 24, sample_limit: int = 240000) -> tuple[np.ndarray, int, float]:
	if k <= 0:
		raise ValueError("k must be positive")
	if pixels.shape[0] > sample_limit:
		step = max(1, pixels.shape[0] // sample_limit)
		sample = pixels[::step][:sample_limit, :3]
	else:
		sample = pixels[:, :3]
	centers = _initial_centers(sample, k)
	last_shift = float("inf")
	iteration = 0
	for iteration in range(1, max_iter + 1):
		dist = ((sample[:, None, :3] - centers[None, :, :]) ** 2).sum(axis=2)
		labels = np.argmin(dist, axis=1)
		new_centers = centers.copy()
		for idx in range(k):
			members = sample[labels == idx, :3]
			if len(members) > 0:
				new_centers[idx] = members.mean(axis=0)
		last_shift = float(np.linalg.norm(new_centers - centers))
		centers = new_centers
		if last_shift < 0.001:
			break
	return centers, iteration, last_shift


def _assign_centers(pixels: np.ndarray, centers: np.ndarray, chunk_size: int = 500000) -> np.ndarray:
	output = pixels.copy()
	for start in range(0, pixels.shape[0], chunk_size):
		end = min(start + chunk_size, pixels.shape[0])
		chunk = pixels[start:end, :3]
		dist = ((chunk[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
		labels = np.argmin(dist, axis=1)
		output[start:end, :3] = centers[labels]
	return output


def flatten_pixels(rgba: np.ndarray, k: int = 6, highlight_cap: float = 0.85) -> tuple[np.ndarray, dict[str, object]]:
	if rgba.ndim != 3 or rgba.shape[2] < 3:
		raise RuntimeError(f"Expected HxWxRGB(A) image, got shape {rgba.shape}")
	height, width, channels = rgba.shape
	flat = np.clip(rgba.reshape((-1, channels)).astype(np.float32), 0.0, 1.0)
	alpha = flat[:, 3:4] if channels >= 4 else None
	rgb = flat[:, :3]
	active = np.ones((flat.shape[0],), dtype=bool)
	if alpha is not None:
		active = alpha[:, 0] > 0.01
	active_rgb = rgb[active]
	if active_rgb.shape[0] == 0:
		raise RuntimeError("Texture has no active pixels to flatten")
	compressed = _compress_highlights(active_rgb, highlight_cap)
	centers, iterations, shift = _mini_batch_kmeans(compressed, min(k, max(1, len(compressed))))
	flattened_active = _assign_centers(compressed, centers)
	out_flat = flat.copy()
	out_flat[active, :3] = flattened_active[:, :3]
	out = out_flat.reshape((height, width, channels))
	unique_before = int(np.unique(np.round(active_rgb * 255).astype(np.uint8), axis=0).shape[0])
	unique_after = int(np.unique(np.round(flattened_active[:, :3] * 255).astype(np.uint8), axis=0).shape[0])
	report = {
		"width": int(width),
		"height": int(height),
		"k": int(k),
		"highlight_cap": float(highlight_cap),
		"active_pixels": int(active_rgb.shape[0]),
		"unique_colors_before": unique_before,
		"unique_colors_after": unique_after,
		"iterations": int(iterations),
		"final_center_shift": shift,
		"centers_rgb_255": np.round(np.clip(centers, 0.0, 1.0) * 255).astype(int).tolist(),
	}
	return out, report


def flatten_with_pillow(input_path: Path, output_path: Path, k: int, highlight_cap: float) -> dict[str, object]:
	from PIL import Image

	img = Image.open(input_path).convert("RGBA")
	arr = np.asarray(img).astype(np.float32) / 255.0
	out, report = flatten_pixels(arr, k=k, highlight_cap=highlight_cap)
	output_path.parent.mkdir(parents=True, exist_ok=True)
	Image.fromarray(np.round(np.clip(out, 0.0, 1.0) * 255).astype(np.uint8), mode="RGBA").save(output_path)
	report.update({"input": str(input_path), "output": str(output_path), "mode": "pillow"})
	return report


def flatten_blender_image(image, output_path: Path, k: int, highlight_cap: float) -> dict[str, object]:
	import bpy

	width, height = int(image.size[0]), int(image.size[1])
	pixels = np.array(image.pixels[:], dtype=np.float32).reshape((height, width, 4))
	out, report = flatten_pixels(pixels, k=k, highlight_cap=highlight_cap)
	flat = out.reshape((-1,)).astype(np.float32)
	image.pixels.foreach_set(flat)
	image.update()
	output_path.parent.mkdir(parents=True, exist_ok=True)
	old_path = image.filepath_raw
	old_format = image.file_format
	try:
		image.filepath_raw = str(output_path)
		image.file_format = "PNG"
		image.save()
	finally:
		image.filepath_raw = old_path
		image.file_format = old_format
	report.update({"image_name": image.name, "output": str(output_path), "mode": "blender"})
	return report


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Flatten a diffuse texture with deterministic k-means.")
	parser.add_argument("input", type=Path)
	parser.add_argument("output", type=Path)
	parser.add_argument("--k", type=int, default=6)
	parser.add_argument("--highlight-cap", type=float, default=0.85)
	parser.add_argument("--report", type=Path)
	return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
	args = parse_args(argv)
	report = flatten_with_pillow(args.input, args.output, args.k, args.highlight_cap)
	if args.report:
		args.report.parent.mkdir(parents=True, exist_ok=True)
		args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
	print(json.dumps(report, sort_keys=True))
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
