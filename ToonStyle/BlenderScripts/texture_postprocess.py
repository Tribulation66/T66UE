#!/usr/bin/env python3
"""Post-process ToonStyle flattened textures.

This runs outside Blender's bundled Python when Pillow/scipy are needed. The
Blender pipeline writes a compact UV triangle file, then calls this helper to:

- merge tiny isolated color regions after K-means flattening,
- dilate UV-island colors into nearby empty texture space,
- optionally derive the Tint texture used by M_Toon_Character.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

try:
	from scipy import ndimage as ndi
except Exception:  # pragma: no cover - dependency depends on host Python.
	ndi = None


def load_rgba(path: Path) -> np.ndarray:
	return np.asarray(Image.open(path).convert("RGBA"), dtype=np.uint8)


def save_rgba(path: Path, rgba: np.ndarray) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	Image.fromarray(np.clip(rgba, 0, 255).astype(np.uint8), mode="RGBA").save(path)


def snap_to_palette(rgba: np.ndarray, centers_rgb_255: list[list[int]] | None) -> tuple[np.ndarray, dict[str, object]]:
	if not centers_rgb_255:
		return rgba, {"applied": False, "reason": "no_palette_centers"}
	centers = np.asarray(centers_rgb_255, dtype=np.int16)
	if centers.ndim != 2 or centers.shape[1] != 3 or centers.shape[0] == 0:
		return rgba, {"applied": False, "reason": "invalid_palette_centers"}
	out = rgba.copy()
	active = np.ones(out.shape[:2], dtype=bool)
	pixels = out[:, :, :3].reshape((-1, 3)).astype(np.int16)
	active_flat = active.reshape((-1,))
	for start in range(0, pixels.shape[0], 500000):
		end = min(start + 500000, pixels.shape[0])
		mask = active_flat[start:end]
		if not mask.any():
			continue
		block = pixels[start:end]
		chunk = block[mask]
		dist = ((chunk[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
		block[mask] = centers[np.argmin(dist, axis=1)]
	out[:, :, :3] = pixels.reshape(out.shape[0], out.shape[1], 3).astype(np.uint8)
	return out, {
		"applied": True,
		"palette_size": int(centers.shape[0]),
		"centers_rgb_255": centers.astype(int).tolist(),
	}


def rasterize_uv_mask(uv_triangles: np.ndarray, width: int, height: int) -> np.ndarray:
	mask_img = Image.new("L", (width, height), 0)
	draw = ImageDraw.Draw(mask_img)
	for triangle in uv_triangles:
		points = []
		for u, v in triangle:
			x = int(round(float(np.clip(u, 0.0, 1.0)) * (width - 1)))
			y = int(round((1.0 - float(np.clip(v, 0.0, 1.0))) * (height - 1)))
			points.append((x, y))
		if len(points) == 3:
			draw.polygon(points, fill=255)
	return np.asarray(mask_img, dtype=np.uint8) > 0


def _label_components(mask: np.ndarray) -> tuple[np.ndarray, int]:
	if ndi is not None:
		structure = np.ones((3, 3), dtype=np.uint8)
		return ndi.label(mask, structure=structure)

	labels = np.zeros(mask.shape, dtype=np.int32)
	next_label = 0
	height, width = mask.shape
	for y in range(height):
		for x in range(width):
			if not mask[y, x] or labels[y, x] != 0:
				continue
			next_label += 1
			stack = [(y, x)]
			labels[y, x] = next_label
			while stack:
				cy, cx = stack.pop()
				for ny in range(max(0, cy - 1), min(height, cy + 2)):
					for nx in range(max(0, cx - 1), min(width, cx + 2)):
						if mask[ny, nx] and labels[ny, nx] == 0:
							labels[ny, nx] = next_label
							stack.append((ny, nx))
	return labels, next_label


def cleanup_speckles(rgba: np.ndarray, min_pixels: int) -> tuple[np.ndarray, dict[str, object]]:
	if ndi is None:
		return rgba, {"applied": False, "reason": "scipy_unavailable_for_fast_median_filter"}

	out = rgba.copy()
	active = out[:, :, 3] > 2
	if not active.any():
		return out, {"applied": False, "reason": "no_active_pixels"}
	rgb = out[:, :, :3].astype(np.int32)
	median = np.stack(
		[ndi.median_filter(rgb[:, :, channel], size=3, mode="nearest") for channel in range(3)],
		axis=2,
	).astype(np.int32)
	diff = np.sqrt(((rgb - median) ** 2).sum(axis=2))
	# The threshold is intentionally conservative: it catches isolated hot/cold
	# pixels without flattening intentional adjacent color regions.
	candidate = active & (diff >= 42.0)
	out[candidate, :3] = np.clip(median[candidate], 0, 255).astype(np.uint8)
	return out, {
		"applied": True,
		"method": "3x3_local_median_high_contrast_pixel_replacement",
		"changed_pixels": int(candidate.sum()),
		"diff_threshold": 42.0,
		"prompt_region_threshold_reference": int(min_pixels),
	}


def pad_uv_islands(rgba: np.ndarray, island_mask: np.ndarray, padding_pixels: int) -> tuple[np.ndarray, dict[str, object]]:
	if padding_pixels <= 0:
		return rgba, {"applied": False, "reason": "padding_le_0"}
	if not island_mask.any():
		return rgba, {"applied": False, "reason": "empty_uv_mask"}

	out = rgba.copy()
	known = island_mask.copy()
	fill_mask = np.zeros(island_mask.shape, dtype=bool)
	directions = [(-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (-1, 1), (1, -1), (1, 1)]
	for _ in range(padding_pixels):
		if ndi is not None:
			frontier = ndi.binary_dilation(known, structure=np.ones((3, 3), dtype=bool)) & ~known
		else:
			frontier = np.zeros_like(known)
			frontier[:-1, :] |= known[1:, :]
			frontier[1:, :] |= known[:-1, :]
			frontier[:, :-1] |= known[:, 1:]
			frontier[:, 1:] |= known[:, :-1]
			frontier &= ~known
		if not frontier.any():
			break
		remaining = frontier.copy()
		for dy, dx in directions:
			source_known = np.zeros_like(known)
			target = np.zeros_like(known)
			if dy < 0:
				src_y = slice(0, dy)
				dst_y = slice(-dy, None)
			elif dy > 0:
				src_y = slice(dy, None)
				dst_y = slice(0, -dy)
			else:
				src_y = slice(None)
				dst_y = slice(None)
			if dx < 0:
				src_x = slice(0, dx)
				dst_x = slice(-dx, None)
			elif dx > 0:
				src_x = slice(dx, None)
				dst_x = slice(0, -dx)
			else:
				src_x = slice(None)
				dst_x = slice(None)
			source_known[dst_y, dst_x] = known[src_y, src_x]
			target = remaining & source_known
			if not target.any():
				continue
			shifted = np.zeros_like(out)
			shifted[dst_y, dst_x] = out[src_y, src_x]
			out[target] = shifted[target]
			remaining[target] = False
		fill_mask |= frontier
		known |= frontier
	return out, {
		"applied": True,
		"padding_pixels": int(padding_pixels),
		"filled_pixels": int(fill_mask.sum()),
		"method": "iterative_frontier_expansion",
	}


def derive_tint(rgba: np.ndarray, value_scale: float, saturation_scale: float) -> np.ndarray:
	arr = rgba.astype(np.float32) / 255.0
	r = arr[:, :, 0]
	g = arr[:, :, 1]
	b = arr[:, :, 2]
	maxc = np.maximum(np.maximum(r, g), b)
	minc = np.minimum(np.minimum(r, g), b)
	v = np.clip(maxc * value_scale, 0.0, 1.0)
	delta = maxc - minc
	s = np.zeros_like(maxc)
	nonzero = maxc > 1e-6
	s[nonzero] = delta[nonzero] / maxc[nonzero]
	s = np.clip(s * saturation_scale, 0.0, 1.0)

	h = np.zeros_like(maxc)
	mask = delta > 1e-6
	rmax = mask & (maxc == r)
	gmax = mask & (maxc == g)
	bmax = mask & (maxc == b)
	h[rmax] = ((g[rmax] - b[rmax]) / delta[rmax]) % 6.0
	h[gmax] = ((b[gmax] - r[gmax]) / delta[gmax]) + 2.0
	h[bmax] = ((r[bmax] - g[bmax]) / delta[bmax]) + 4.0
	h = h / 6.0

	c = v * s
	x = c * (1.0 - np.abs(((h * 6.0) % 2.0) - 1.0))
	m = v - c
	h6 = h * 6.0
	rp = np.zeros_like(h)
	gp = np.zeros_like(h)
	bp = np.zeros_like(h)
	for low, values in [
		(0, (c, x, 0)),
		(1, (x, c, 0)),
		(2, (0, c, x)),
		(3, (0, x, c)),
		(4, (x, 0, c)),
		(5, (c, 0, x)),
	]:
		region = (h6 >= low) & (h6 < low + 1)
		vr, vg, vb = values
		rp[region] = vr[region] if hasattr(vr, "__getitem__") else vr
		gp[region] = vg[region] if hasattr(vg, "__getitem__") else vg
		bp[region] = vb[region] if hasattr(vb, "__getitem__") else vb
	tint = arr.copy()
	tint[:, :, 0] = rp + m
	tint[:, :, 1] = gp + m
	tint[:, :, 2] = bp + m
	return np.round(np.clip(tint, 0.0, 1.0) * 255.0).astype(np.uint8)


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Clean, pad, and derive Tint for flattened ToonStyle textures.")
	parser.add_argument("--input", required=True, type=Path)
	parser.add_argument("--output", required=True, type=Path)
	parser.add_argument("--uv-triangles", required=True, type=Path)
	parser.add_argument("--padding-pixels", type=int, default=16)
	parser.add_argument("--speckle-threshold", type=int, default=8)
	parser.add_argument("--palette-centers-json")
	parser.add_argument("--tint-output", type=Path)
	parser.add_argument("--tint-value-scale", type=float, default=0.6)
	parser.add_argument("--tint-saturation-scale", type=float, default=1.1)
	parser.add_argument("--report", type=Path)
	return parser.parse_args()


def main() -> int:
	args = parse_args()
	rgba = load_rgba(args.input)
	height, width = rgba.shape[:2]
	centers = json.loads(args.palette_centers_json) if args.palette_centers_json else None
	rgba, palette_report = snap_to_palette(rgba, centers)
	uv_data = np.load(args.uv_triangles)
	uv_triangles = uv_data["triangles"]
	mask = rasterize_uv_mask(uv_triangles, width, height)

	unique_before = int(np.unique(rgba[:, :, :3].reshape((-1, 3)), axis=0).shape[0])
	cleaned, speckle_report = cleanup_speckles(rgba, args.speckle_threshold)
	padded, padding_report = pad_uv_islands(cleaned, mask, args.padding_pixels)
	padded, final_palette_report = snap_to_palette(padded, centers)
	save_rgba(args.output, padded)

	tint_report = {"generated": False}
	if args.tint_output:
		tint = derive_tint(padded, args.tint_value_scale, args.tint_saturation_scale)
		save_rgba(args.tint_output, tint)
		tint_report = {
			"generated": True,
			"output": str(args.tint_output),
			"value_scale": float(args.tint_value_scale),
			"saturation_scale": float(args.tint_saturation_scale),
		}

	unique_after = int(np.unique(padded[:, :, :3].reshape((-1, 3)), axis=0).shape[0])
	report = {
		"input": str(args.input),
		"output": str(args.output),
		"width": int(width),
		"height": int(height),
		"dependency_path": "external_python_pillow_scipy" if ndi is not None else "external_python_pillow_numpy_only",
		"uv_triangles": int(len(uv_triangles)),
		"uv_mask_pixels": int(mask.sum()),
		"unique_rgb_before": unique_before,
		"unique_rgb_after": unique_after,
		"palette_snap": palette_report,
		"final_palette_snap": final_palette_report,
		"speckle_cleanup": speckle_report,
		"uv_padding": padding_report,
		"tint": tint_report,
	}
	if args.report:
		args.report.parent.mkdir(parents=True, exist_ok=True)
		args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
	print(json.dumps(report, sort_keys=True))
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
