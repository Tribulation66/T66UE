"""
Create normalized floor/wall base-color textures for CoherentThemeKit01.

The source Trellis textures can drift heavily in hue between modules. This pass
keeps luminance/detail from each texture, then remaps it onto one stable color
per theme/surface before Unreal import.
"""

from __future__ import annotations

import json
from pathlib import Path

import numpy as np
from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = PROJECT_ROOT / "SourceAssets" / "Import" / "WorldKit" / "CoherentThemeKit01" / "Textures"
OUTPUT_DIR = PROJECT_ROOT / "SourceAssets" / "Import" / "WorldKit" / "CoherentThemeKit01" / "Textures_Recolored"
REPORT_PATH = PROJECT_ROOT / "Saved" / "CoherentThemeKit01TextureRecolorReport.json"

SURFACE_COLORS = {
    ("Dungeon", "Floor"): (0.48, 0.50, 0.46),
    ("Dungeon", "Wall"): (0.38, 0.39, 0.36),
    ("Forest", "Floor"): (0.34, 0.42, 0.29),
    ("Forest", "Wall"): (0.36, 0.32, 0.25),
    ("Ocean", "Floor"): (0.48, 0.58, 0.54),
    ("Ocean", "Wall"): (0.36, 0.49, 0.49),
    ("Martian", "Floor"): (0.56, 0.37, 0.30),
    ("Martian", "Wall"): (0.47, 0.31, 0.27),
    ("Hell", "Floor"): (0.36, 0.30, 0.27),
    ("Hell", "Wall"): (0.30, 0.26, 0.24),
}


def classify_texture(path: Path) -> tuple[str, str] | None:
    name = path.name
    for theme in ("Dungeon", "Forest", "Ocean", "Martian", "Hell"):
        if not name.startswith(theme):
            continue
        if "Floor" in name:
            return theme, "Floor"
        if "Wall" in name:
            return theme, "Wall"
    return None


def recolor_image(source_path: Path, output_path: Path, target_rgb: tuple[float, float, float]) -> dict:
    image = Image.open(source_path).convert("RGBA")
    rgba = np.asarray(image).astype(np.float32) / 255.0
    rgb = rgba[..., :3]
    alpha = rgba[..., 3:4]

    luminance = (rgb[..., 0] * 0.2126) + (rgb[..., 1] * 0.7152) + (rgb[..., 2] * 0.0722)
    visible = alpha[..., 0] > 0.01
    if np.any(visible):
        low = float(np.percentile(luminance[visible], 4))
        high = float(np.percentile(luminance[visible], 96))
    else:
        low = float(np.percentile(luminance, 4))
        high = float(np.percentile(luminance, 96))

    if high <= low + 1e-4:
        detail = np.full_like(luminance, 0.5)
    else:
        detail = np.clip((luminance - low) / (high - low), 0.0, 1.0)

    # Keep cracks/readability, but clamp away from pure black so undersides do
    # not become black/green when viewed from below.
    shade = 0.58 + (detail * 0.62)
    target = np.asarray(target_rgb, dtype=np.float32).reshape(1, 1, 3)
    recolored_rgb = np.clip(target * shade[..., None], 0.0, 1.0)

    out = np.concatenate([recolored_rgb, alpha], axis=2)
    out_u8 = np.clip(out * 255.0 + 0.5, 0, 255).astype(np.uint8)
    Image.fromarray(out_u8, "RGBA").save(output_path)

    return {
        "source": str(source_path.relative_to(PROJECT_ROOT)).replace("\\", "/"),
        "output": str(output_path.relative_to(PROJECT_ROOT)).replace("\\", "/"),
        "low_luminance": low,
        "high_luminance": high,
        "target_rgb": target_rgb,
    }


def main() -> None:
    if not SOURCE_DIR.is_dir():
        raise FileNotFoundError(f"Missing texture source directory: {SOURCE_DIR}")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)

    report = []
    skipped = []
    for source_path in sorted(SOURCE_DIR.glob("*_BaseColor_00.png")):
        classification = classify_texture(source_path)
        if not classification:
            skipped.append(str(source_path.relative_to(PROJECT_ROOT)).replace("\\", "/"))
            continue

        target_rgb = SURFACE_COLORS[classification]
        output_path = OUTPUT_DIR / source_path.name
        entry = recolor_image(source_path, output_path, target_rgb)
        entry["theme"] = classification[0]
        entry["surface"] = classification[1]
        report.append(entry)

    REPORT_PATH.write_text(
        json.dumps({"recolored": report, "skipped": skipped}, indent=2),
        encoding="utf-8",
    )
    print(f"Recolored {len(report)} textures into {OUTPUT_DIR}")
    print(f"Report: {REPORT_PATH}")


if __name__ == "__main__":
    main()
