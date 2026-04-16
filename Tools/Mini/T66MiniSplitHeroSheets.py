from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = PROJECT_ROOT / "Docs" / "Mini" / "T66Mini_SpriteBatchManifest.json"
OUTPUT_ROOT = PROJECT_ROOT / "SourceAssets" / "Mini" / "Heroes" / "Singles"
QUADRANT_ORDER = ["top_left", "top_right", "bottom_left", "bottom_right"]


HERO_NAME_MAP = {
	"heroes_batch_01": ["Arthur", "Merlin", "LuBu", "Mike"],
	"heroes_batch_02": ["George", "Zeus", "Robo", "Billy"],
	"heroes_batch_03": ["Rabbit", "Dog", "Shroud", "xQc"],
	"heroes_batch_04": ["Moist", "North", "Asmon", "Forsen"],
}


def main() -> int:
	manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
	OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)

	for batch in manifest.get("batches", []):
		batch_id = batch["id"]
		names = HERO_NAME_MAP.get(batch_id)
		if not names:
			continue

		batch_dir = PROJECT_ROOT / batch["output_dir"] / batch_id
		sheet_path = batch_dir / f"{batch_id}-01.png"
		if not sheet_path.is_file():
			continue

		with Image.open(sheet_path) as image:
			width, height = image.size
			half_w = width // 2
			half_h = height // 2
			boxes = {
				"top_left": (0, 0, half_w, half_h),
				"top_right": (half_w, 0, width, half_h),
				"bottom_left": (0, half_h, half_w, height),
				"bottom_right": (half_w, half_h, width, height),
			}

			for index, quadrant in enumerate(QUADRANT_ORDER):
				cropped = image.crop(boxes[quadrant])
				target_path = OUTPUT_ROOT / f"{names[index]}.png"
				cropped.save(target_path)

	return 0


if __name__ == "__main__":
	raise SystemExit(main())
