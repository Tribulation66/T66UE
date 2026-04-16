from __future__ import annotations

import argparse
from collections import deque
import json
import re
from pathlib import Path

from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = PROJECT_ROOT / "Docs" / "Mini" / "T66Mini_SpriteBatchManifest.json"
QUADRANT_ORDER = ["top_left", "top_right", "bottom_left", "bottom_right"]


def sanitize_name(name: str) -> str:
	return re.sub(r"[^A-Za-z0-9_\\-]+", "_", name).strip("_")


def color_distance(left: tuple[int, int, int], right: tuple[int, int, int]) -> int:
	return abs(left[0] - right[0]) + abs(left[1] - right[1]) + abs(left[2] - right[2])


def remove_edge_connected_background(image: Image.Image) -> Image.Image:
	rgba = image.convert("RGBA")
	width, height = rgba.size
	if width <= 2 or height <= 2:
		return rgba

	pixels = rgba.load()
	border_has_alpha = any(
		pixels[x, y][3] < 255
		for x in range(width)
		for y in (0, height - 1)
	) or any(
		pixels[x, y][3] < 255
		for y in range(height)
		for x in (0, width - 1)
	)
	if border_has_alpha:
		return rgba

	queue: deque[tuple[int, int]] = deque()
	visited: set[tuple[int, int]] = set()

	for x in range(width):
		queue.append((x, 0))
		queue.append((x, height - 1))
	for y in range(height):
		queue.append((0, y))
		queue.append((width - 1, y))

	while queue:
		x, y = queue.popleft()
		if (x, y) in visited:
			continue

		visited.add((x, y))
		current = pixels[x, y]
		if current[3] == 0:
			continue

		for offset_x, offset_y in ((1, 0), (-1, 0), (0, 1), (0, -1)):
			neighbor_x = x + offset_x
			neighbor_y = y + offset_y
			if neighbor_x < 0 or neighbor_x >= width or neighbor_y < 0 or neighbor_y >= height:
				continue
			if (neighbor_x, neighbor_y) in visited:
				continue

			neighbor = pixels[neighbor_x, neighbor_y]
			if neighbor[3] == 0:
				continue

			if color_distance(current[:3], neighbor[:3]) <= 22:
				queue.append((neighbor_x, neighbor_y))

	for x, y in visited:
		r, g, b, a = pixels[x, y]
		pixels[x, y] = (r, g, b, 0)

	return rgba


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Split 2x2 mini sprite sheets into individual subject PNGs.")
	parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="Path to the mini sprite batch manifest.")
	parser.add_argument("--only", nargs="*", default=[], help="Optional batch IDs to split.")
	return parser.parse_args()


def split_sheet(sheet_path: Path, output_root: Path, subjects: list[str]) -> None:
	with Image.open(sheet_path) as image:
		width, height = image.size
		half_width = width // 2
		half_height = height // 2
		boxes = {
			"top_left": (0, 0, half_width, half_height),
			"top_right": (half_width, 0, width, half_height),
			"bottom_left": (0, half_height, half_width, height),
			"bottom_right": (half_width, half_height, width, height),
		}

		for index, quadrant in enumerate(QUADRANT_ORDER):
			cropped = remove_edge_connected_background(image.crop(boxes[quadrant]))
			target_path = output_root / f"{sanitize_name(subjects[index])}.png"
			cropped.save(target_path)


def main() -> int:
	args = parse_args()
	manifest = json.loads(Path(args.manifest).resolve().read_text(encoding="utf-8"))
	selected_ids = set(args.only)

	for batch in manifest.get("batches", []):
		batch_id = batch["id"]
		subjects = batch.get("subjects", [])
		split_output_dir = batch.get("split_output_dir")
		if not subjects or len(subjects) != 4 or not split_output_dir:
			continue
		if selected_ids and batch_id not in selected_ids:
			continue

		output_root = (PROJECT_ROOT / split_output_dir).resolve()
		output_root.mkdir(parents=True, exist_ok=True)
		batch_root = PROJECT_ROOT / batch["output_dir"] / batch_id

		sheet_variants = batch.get("sheet_variants", [])
		if sheet_variants:
			for variant in sheet_variants:
				sheet_path = batch_root / variant["filename"]
				if not sheet_path.is_file():
					continue
				variant_output = (PROJECT_ROOT / variant.get("split_output_dir", split_output_dir)).resolve()
				variant_output.mkdir(parents=True, exist_ok=True)
				split_sheet(sheet_path, variant_output, variant.get("subjects", subjects))
			continue

		sheet_path = batch_root / f"{batch_id}-01.png"
		if not sheet_path.is_file():
			continue

		split_sheet(sheet_path, output_root, subjects)

	return 0


if __name__ == "__main__":
	raise SystemExit(main())
