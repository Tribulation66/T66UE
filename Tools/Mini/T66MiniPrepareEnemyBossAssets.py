from __future__ import annotations

from pathlib import Path
import shutil

from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[2]
ENEMY_VARIANT_ROOT = PROJECT_ROOT / "SourceAssets" / "Mini" / "Enemies" / "Variants" / "BatchRight"
ENEMY_SINGLE_ROOT = PROJECT_ROOT / "SourceAssets" / "Mini" / "Enemies" / "Singles"
BOSS_SINGLE_ROOT = PROJECT_ROOT / "SourceAssets" / "Mini" / "Bosses" / "Singles"

ENEMIES = ["Roost", "Cow", "Goat", "Pig"]


def ensure_dir(path: Path) -> None:
	path.mkdir(parents=True, exist_ok=True)


def copy_canonical_enemy_singles() -> None:
	ensure_dir(ENEMY_SINGLE_ROOT)

	for enemy_name in ENEMIES:
		source_path = ENEMY_VARIANT_ROOT / f"{enemy_name}.png"
		if not source_path.is_file():
			continue

		target_path = ENEMY_SINGLE_ROOT / f"{enemy_name}.png"
		shutil.copy2(source_path, target_path)


def build_boss_placeholders() -> None:
	ensure_dir(BOSS_SINGLE_ROOT)

	for enemy_name in ENEMIES:
		source_path = ENEMY_SINGLE_ROOT / f"{enemy_name}.png"
		if not source_path.is_file():
			continue

		with Image.open(source_path) as source_image:
			source_image = source_image.convert("RGBA")
			scale = 1.35
			scaled_size = (
				max(1, int(source_image.width * scale)),
				max(1, int(source_image.height * scale)),
			)
			scaled = source_image.resize(scaled_size, Image.Resampling.LANCZOS)
			canvas = Image.new("RGBA", scaled_size, (0, 0, 0, 0))
			canvas.paste(scaled, (0, 0), scaled)
			canvas.save(BOSS_SINGLE_ROOT / f"{enemy_name}_Boss.png")


def main() -> int:
	copy_canonical_enemy_singles()
	build_boss_placeholders()
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
