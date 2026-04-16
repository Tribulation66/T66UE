from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

from T66MiniSplitSheets import remove_edge_connected_background


PROJECT_ROOT = Path(__file__).resolve().parents[2]
QUADRANT_ORDER = ["top_left", "top_right", "bottom_left", "bottom_right"]


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Normalize 2x2 walk-sheet candidates into aligned frame sequences.")
	parser.add_argument("--manifest", required=True, help="Path to the walk candidate manifest.")
	parser.add_argument("--only", nargs="*", default=[], help="Optional batch IDs to normalize.")
	return parser.parse_args()


def load_manifest(path: Path) -> dict:
	return json.loads(path.read_text(encoding="utf-8"))


def split_quadrants(image: Image.Image) -> list[Image.Image]:
	width, height = image.size
	half_width = width // 2
	half_height = height // 2
	boxes = {
		"top_left": (0, 0, half_width, half_height),
		"top_right": (half_width, 0, width, half_height),
		"bottom_left": (0, half_height, half_width, height),
		"bottom_right": (half_width, half_height, width, height),
	}
	return [remove_edge_connected_background(image.crop(boxes[quadrant])) for quadrant in QUADRANT_ORDER]


def normalize_frames(frames: list[Image.Image]) -> list[Image.Image]:
	if not frames:
		return []

	canvas_size = frames[0].size
	bboxes = [frame.getchannel("A").getbbox() for frame in frames]
	valid_boxes = [bbox for bbox in bboxes if bbox]
	if not valid_boxes:
		return [frame.convert("RGBA") for frame in frames]

	target_bottom = max(bbox[3] for bbox in valid_boxes)
	target_center_x = round(sum((bbox[0] + bbox[2]) / 2.0 for bbox in valid_boxes) / len(valid_boxes))
	target_height = round(sum((bbox[3] - bbox[1]) for bbox in valid_boxes) / len(valid_boxes))

	normalized: list[Image.Image] = []
	for frame, bbox in zip(frames, bboxes):
		canvas = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
		if not bbox:
			normalized.append(canvas)
			continue

		crop = frame.crop(bbox)
		crop_width, crop_height = crop.size
		if crop_height > 0 and target_height > 0 and crop_height != target_height:
			scale = target_height / crop_height
			resized_width = max(1, round(crop_width * scale))
			crop = crop.resize((resized_width, target_height), Image.Resampling.LANCZOS)
			crop_width, crop_height = crop.size

		dest_left = round(target_center_x - (crop_width / 2.0))
		dest_top = round(target_bottom - crop_height)
		canvas.alpha_composite(crop, (dest_left, dest_top))
		normalized.append(canvas)

	return normalized


def mirror_frames(frames: list[Image.Image]) -> list[Image.Image]:
	return [frame.transpose(Image.Transpose.FLIP_LEFT_RIGHT) for frame in frames]


def save_frames(output_dir: Path, prefix: str, suffixes: list[str], frames: list[Image.Image]) -> None:
	output_dir.mkdir(parents=True, exist_ok=True)
	for suffix, frame in zip(suffixes, frames):
		frame.save(output_dir / f"{prefix}_{suffix}.png")


def resolve_source_sheet(batch: dict) -> Path:
	output_root = (PROJECT_ROOT / batch["output_dir"] / batch["id"]).resolve()
	if "source_filename" in batch:
		return output_root / batch["source_filename"]
	return output_root / f"{batch['id']}-01.png"


def main() -> int:
	args = parse_args()
	manifest = load_manifest(Path(args.manifest).resolve())
	selected_ids = set(args.only)

	for batch in manifest.get("batches", []):
		batch_id = batch["id"]
		if selected_ids and batch_id not in selected_ids:
			continue
		if batch.get("kind") != "walk_candidate":
			continue

		source_sheet = resolve_source_sheet(batch)
		if not source_sheet.is_file():
			print(f"[T66MiniNormalizeWalkSheet] {batch_id}: missing source sheet {source_sheet}")
			continue

		output_dir = (PROJECT_ROOT / batch["normalized_output_dir"]).resolve()
		with Image.open(source_sheet) as source_image:
			right_frames = normalize_frames(split_quadrants(source_image.convert("RGBA")))
			left_frames = mirror_frames(right_frames)

		frame_suffixes = batch.get("frame_suffixes", ["Idle", "WalkA", "WalkB", "WalkC"])
		prefix = batch.get("frame_prefix", "Walk")
		save_frames(output_dir, prefix, [f"{suffix}_R" for suffix in frame_suffixes], right_frames)
		save_frames(output_dir, prefix, [f"{suffix}_L" for suffix in frame_suffixes], left_frames)
		print(f"[T66MiniNormalizeWalkSheet] {batch_id}: normalized -> {output_dir}")

	return 0


if __name__ == "__main__":
	raise SystemExit(main())
