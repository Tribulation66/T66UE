from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


PROJECT_ROOT = Path(__file__).resolve().parents[2]


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Build a comparison sheet from normalized walk candidates.")
	parser.add_argument("--manifest", required=True, help="Path to the walk candidate manifest.")
	parser.add_argument("--group", required=True, help="Comparison group ID.")
	return parser.parse_args()


def load_manifest(path: Path) -> dict:
	return json.loads(path.read_text(encoding="utf-8"))


def load_row_images(batch: dict) -> list[Image.Image]:
	output_dir = (PROJECT_ROOT / batch["normalized_output_dir"]).resolve()
	prefix = batch.get("frame_prefix", "Walk")
	frame_suffixes = batch.get("frame_suffixes", ["Idle", "WalkA", "WalkB", "WalkC"])
	images: list[Image.Image] = []
	for suffix in frame_suffixes:
		path = output_dir / f"{prefix}_{suffix}_R.png"
		if not path.is_file():
			raise FileNotFoundError(path)
		images.append(Image.open(path).convert("RGBA"))
	return images


def build_comparison_sheet(batches: list[dict], output_path: Path) -> None:
	if not batches:
		return

	font = ImageFont.load_default()
	row_images = [load_row_images(batch) for batch in batches]
	cell_width, cell_height = row_images[0][0].size
	row_label_width = 180
	row_gap = 18
	header_height = 34
	padding = 20
	sheet_width = padding * 2 + row_label_width + (cell_width * 4)
	sheet_height = padding * 2 + (len(batches) * (header_height + cell_height + row_gap)) - row_gap

	sheet = Image.new("RGBA", (sheet_width, sheet_height), (9, 14, 24, 255))
	draw = ImageDraw.Draw(sheet)

	for row_index, (batch, images) in enumerate(zip(batches, row_images)):
		row_top = padding + (row_index * (header_height + cell_height + row_gap))
		label = batch.get("comparison_label", batch["id"])
		draw.text((padding, row_top + 8), label, fill=(235, 238, 245, 255), font=font)
		for frame_index, frame in enumerate(images):
			x = padding + row_label_width + (frame_index * cell_width)
			y = row_top + header_height
			sheet.alpha_composite(frame, (x, y))
		draw.line((padding, row_top + header_height + cell_height + 6, sheet_width - padding, row_top + header_height + cell_height + 6), fill=(45, 57, 76, 255), width=1)

	output_path.parent.mkdir(parents=True, exist_ok=True)
	sheet.save(output_path)


def main() -> int:
	args = parse_args()
	manifest = load_manifest(Path(args.manifest).resolve())
	batches = [
		batch
		for batch in manifest.get("batches", [])
		if batch.get("kind") == "walk_candidate" and batch.get("comparison_group") == args.group
	]
	if not batches:
		print(f"[T66MiniBuildWalkComparison] no batches found for group {args.group}")
		return 0

	output_path = (PROJECT_ROOT / batches[0]["comparison_output"]).resolve()
	build_comparison_sheet(batches, output_path)
	print(f"[T66MiniBuildWalkComparison] wrote {output_path}")
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
