#!/usr/bin/env python3
"""Build the Mini Chadpocalypse low-res pixel-art runtime sprite set.

The input is a Codex/imagegen-produced chroma-key sprite atlas. The output is a
mode-owned loose runtime asset tree under SourceAssets/Mini that matches the
existing T66Mini visual-subsystem file names.
"""

from __future__ import annotations

import argparse
import csv
import json
import random
import re
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageEnhance, ImageOps


GREEN_KEY = (0, 255, 0)


@dataclass(frozen=True)
class CropSpec:
    name: str
    bbox: tuple[int, int, int, int]


CROPS: dict[str, CropSpec] = {
    "hero_idle": CropSpec("hero_idle", (70, 41, 190, 183)),
    "hero_walk_a": CropSpec("hero_walk_a", (274, 41, 391, 187)),
    "hero_walk_b": CropSpec("hero_walk_b", (459, 41, 570, 183)),
    "hero_walk_c": CropSpec("hero_walk_c", (653, 41, 769, 183)),
    "companion": CropSpec("companion", (861, 74, 968, 183)),
    "roost": CropSpec("roost", (72, 216, 217, 383)),
    "goat": CropSpec("goat", (632, 228, 771, 391)),
    "cow": CropSpec("cow", (328, 236, 540, 390)),
    "chest": CropSpec("chest", (1154, 241, 1320, 378)),
    "pig": CropSpec("pig", (862, 251, 1019, 387)),
    "fountain": CropSpec("fountain", (1147, 412, 1371, 622)),
    "roost_boss": CropSpec("roost_boss", (34, 413, 262, 617)),
    "cow_boss": CropSpec("cow_boss", (318, 416, 577, 615)),
    "goat_boss": CropSpec("goat_boss", (616, 416, 787, 618)),
    "pig_boss": CropSpec("pig_boss", (841, 429, 1055, 617)),
    "quick_revive": CropSpec("quick_revive", (423, 643, 578, 808)),
    "loot_crate": CropSpec("loot_crate", (100, 652, 289, 795)),
    "projectile_yellow": CropSpec("projectile_yellow", (714, 681, 854, 750)),
    "projectile_red": CropSpec("projectile_red", (966, 681, 1107, 749)),
    "projectile_black": CropSpec("projectile_black", (1211, 681, 1358, 757)),
    "tile_patch": CropSpec("tile_patch", (835, 813, 1355, 972)),
    "coin": CropSpec("coin", (122, 848, 216, 951)),
    "gem": CropSpec("gem", (361, 848, 446, 944)),
    "heart": CropSpec("heart", (553, 848, 660, 951)),
}


HERO_ACCENTS = [
    (210, 36, 30),
    (222, 108, 30),
    (216, 72, 48),
    (46, 150, 92),
    (78, 168, 206),
    (180, 122, 42),
    (196, 94, 176),
    (58, 76, 132),
    (176, 48, 94),
    (140, 140, 140),
    (78, 130, 212),
    (204, 178, 52),
]

COMPANION_ACCENTS = [
    (70, 146, 226),
    (222, 86, 48),
    (212, 212, 198),
    (220, 92, 42),
    (62, 184, 214),
    (236, 204, 92),
    (112, 174, 224),
    (232, 112, 34),
    (154, 98, 64),
    (138, 210, 238),
    (154, 86, 200),
    (92, 184, 112),
    (226, 86, 154),
    (190, 176, 230),
    (120, 220, 160),
    (84, 214, 220),
    (58, 70, 104),
    (150, 116, 86),
    (64, 198, 238),
    (88, 72, 112),
    (138, 210, 116),
    (238, 184, 160),
    (92, 104, 230),
    (198, 62, 62),
]


def sanitize(raw: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]", "", raw)


def read_csv_dicts(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def ensure_clean_dir(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def is_key_pixel(pixel: tuple[int, int, int, int]) -> bool:
    r, g, b, a = pixel
    return a == 0 or (g >= 170 and r <= 90 and b <= 90)


def keyed_crop(source: Image.Image, bbox: tuple[int, int, int, int]) -> Image.Image:
    crop = source.crop(bbox).convert("RGBA")
    pixels = crop.load()
    width, height = crop.size
    for y in range(height):
        for x in range(width):
            if is_key_pixel(pixels[x, y]):
                pixels[x, y] = (0, 0, 0, 0)
    alpha_box = crop.getbbox()
    return crop.crop(alpha_box) if alpha_box else crop


def fit_pixel_sprite(
    source: Image.Image,
    logical_size: int,
    output_size: int,
    occupancy: float = 0.78,
) -> Image.Image:
    trimmed = source.convert("RGBA")
    alpha_box = trimmed.getbbox()
    if alpha_box:
        trimmed = trimmed.crop(alpha_box)

    max_logical = max(1, int(logical_size * occupancy))
    scale = min(max_logical / max(1, trimmed.width), max_logical / max(1, trimmed.height))
    resized = trimmed.resize(
        (max(1, round(trimmed.width * scale)), max(1, round(trimmed.height * scale))),
        Image.Resampling.NEAREST,
    )

    canvas = Image.new("RGBA", (logical_size, logical_size), (0, 0, 0, 0))
    canvas.alpha_composite(resized, ((logical_size - resized.width) // 2, logical_size - resized.height - 2))
    return canvas.resize((output_size, output_size), Image.Resampling.NEAREST)


def tint_sprite(image: Image.Image, accent: tuple[int, int, int], strength: float = 0.78) -> Image.Image:
    out = image.convert("RGBA")
    pixels = out.load()
    target = accent
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                continue
            max_channel = max(r, g, b)
            min_channel = min(r, g, b)
            saturation = max_channel - min_channel
            if r > 120 and r > g * 1.15 and r > b * 1.15:
                pixels[x, y] = (
                    int(r * (1.0 - strength) + target[0] * strength),
                    int(g * (1.0 - strength) + target[1] * strength),
                    int(b * (1.0 - strength) + target[2] * strength),
                    a,
                )
            elif saturation > 52 and b > r and b > g:
                pixels[x, y] = (
                    int(r * 0.45 + target[0] * 0.35),
                    int(g * 0.45 + target[1] * 0.35),
                    int(b * 0.45 + target[2] * 0.35),
                    a,
                )
    return out


def darken_or_brighten(image: Image.Image, factor: float) -> Image.Image:
    alpha = image.getchannel("A")
    rgb = Image.new("RGBA", image.size, (0, 0, 0, 0))
    rgb.alpha_composite(image)
    enhanced = ImageEnhance.Brightness(rgb.convert("RGB")).enhance(factor).convert("RGBA")
    enhanced.putalpha(alpha)
    return enhanced


def purge_key_residue(image: Image.Image) -> Image.Image:
    out = image.convert("RGBA")
    if out.getchannel("A").getextrema()[0] > 0:
        return out

    pixels = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = pixels[x, y]
            if a > 0 and g > 130 and r < 55 and b < 55:
                pixels[x, y] = (0, 0, 0, 0)
    return out


def save_sprite(image: Image.Image, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    purge_key_residue(image).save(path)


def make_attack(sprite: Image.Image) -> Image.Image:
    out = sprite.copy()
    pixels = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                continue
            if r > 180 and g > 130:
                pixels[x, y] = (255, min(255, g + 35), min(255, b + 18), a)
    return ImageEnhance.Contrast(out).enhance(1.12)


def write_hero_set(name: str, frames: dict[str, Image.Image], out_root: Path, accent: tuple[int, int, int]) -> list[str]:
    visual_id = sanitize(name)
    written: list[str] = []
    anim_dir = out_root / "Heroes" / "AnimationSets" / visual_id
    single_dir = out_root / "Heroes" / "Singles"
    frame_map = {
        "Idle_R": tint_sprite(frames["hero_idle"], accent),
        "WalkA_R": tint_sprite(frames["hero_walk_a"], accent),
        "WalkB_R": tint_sprite(frames["hero_walk_b"], accent),
        "WalkC_R": tint_sprite(frames["hero_walk_c"], accent),
    }
    frame_map["Attack_R"] = make_attack(frame_map["WalkC_R"])
    for key, image in list(frame_map.items()):
        left_key = key.replace("_R", "_L")
        frame_map[left_key] = ImageOps.mirror(image)

    for key, image in frame_map.items():
        path = anim_dir / f"{visual_id}_{key}.png"
        save_sprite(image, path)
        written.append(str(path))

    projectile = tint_sprite(frames["projectile_yellow"], accent, 0.55)
    save_sprite(projectile, anim_dir / f"{visual_id}_SwordProjectile.png")
    save_sprite(projectile, anim_dir / f"{visual_id}_PrimaryProjectile.png")
    written.extend([str(anim_dir / f"{visual_id}_SwordProjectile.png"), str(anim_dir / f"{visual_id}_PrimaryProjectile.png")])
    save_sprite(frame_map["Idle_R"], single_dir / f"{visual_id}.png")
    written.append(str(single_dir / f"{visual_id}.png"))
    return written


def write_companion_set(name: str, base: Image.Image, out_root: Path, accent: tuple[int, int, int]) -> list[str]:
    visual_id = sanitize(name)
    written: list[str] = []
    anim_dir = out_root / "Companions" / "AnimationSets" / visual_id
    single_dir = out_root / "Companions" / "Singles"
    idle = tint_sprite(base, accent)
    walk_a = darken_or_brighten(idle, 0.92)
    walk_b = darken_or_brighten(idle, 1.08)
    walk_c = ImageOps.mirror(walk_a)
    attack = make_attack(idle)
    frame_map = {
        "Idle_R": idle,
        "WalkA_R": walk_a,
        "WalkB_R": walk_b,
        "WalkC_R": walk_c,
        "Attack_R": attack,
    }
    for key, image in list(frame_map.items()):
        frame_map[key.replace("_R", "_L")] = ImageOps.mirror(image)

    for key, image in frame_map.items():
        path = anim_dir / f"{visual_id}_{key}.png"
        save_sprite(image, path)
        written.append(str(path))
    save_sprite(idle, single_dir / f"{visual_id}.png")
    written.append(str(single_dir / f"{visual_id}.png"))
    return written


def make_background(tile_patch: Image.Image, out_root: Path) -> str:
    random.seed(66)
    tile = tile_patch.resize((128, 40), Image.Resampling.NEAREST)
    base = Image.new("RGBA", (512, 512), (35, 30, 55, 255))
    pixels = base.load()
    palette = [(35, 30, 55), (44, 38, 68), (52, 46, 78), (28, 24, 44)]
    for y in range(0, 512, 8):
        for x in range(0, 512, 8):
            color = random.choice(palette)
            for yy in range(y, min(y + 8, 512)):
                for xx in range(x, min(x + 8, 512)):
                    pixels[xx, yy] = (*color, 255)
            if random.random() < 0.10:
                detail = random.choice([(93, 102, 136), (64, 72, 108), (74, 120, 76)])
                for yy in range(y + 2, min(y + 6, 512)):
                    for xx in range(x + 2, min(x + 6, 512)):
                        pixels[xx, yy] = (*detail, 255)

    for y in range(0, 512, 128):
        for x in range(0, 512, 128):
            if random.random() < 0.45:
                base.alpha_composite(tile, (x, y + random.randrange(0, 64)))

    path = out_root / "Background.png"
    save_sprite(base, path)
    return str(path)


def write_manifest(out_root: Path, data: dict[str, object]) -> None:
    path = out_root / "ROTmgPixelSetManifest.json"
    path.write_text(json.dumps(data, indent=2), encoding="utf-8")


def write_readme(out_root: Path, source_atlas: Path) -> None:
    readme = f"""# Mini Pixel Runtime Assets

This folder is the live low-res pixel-art runtime set for Mini Chadpocalypse.

- Source atlas: `{source_atlas.as_posix()}`
- Built by: `Tools/ArtPipeline/Minigames/T66MiniBuildRotMGPixelSet.py`
- Scope: in-run game sprites, background, projectiles, pickups, and VFX.
- UI scope: none. Mini HUD fallback icons are copied through from `SourceAssets/Archive/Mini/HUD` unchanged.

Older painterly Mini source art is retained under `SourceAssets/Archive/Mini`.
"""
    (out_root / "README.md").write_text(readme, encoding="utf-8")


def copy_preserved_hud_assets(project_root: Path, out_root: Path) -> list[str]:
    """Keep Mini HUD fallback art unchanged when rebuilding gameplay sprites."""
    source_hud = project_root / "SourceAssets" / "Archive" / "Mini" / "HUD"
    if not source_hud.exists():
        return []

    destination_hud = out_root / "HUD"
    shutil.copytree(source_hud, destination_hud, dirs_exist_ok=True)
    return [str(path) for path in destination_hud.rglob("*") if path.is_file()]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-atlas", required=True, type=Path)
    parser.add_argument("--project-root", default=Path.cwd(), type=Path)
    parser.add_argument("--output-root", default=Path("SourceAssets/Mini"), type=Path)
    args = parser.parse_args()

    project_root = args.project_root.resolve()
    source_atlas = args.source_atlas.resolve()
    out_root = (project_root / args.output_root).resolve()
    data_root = project_root / "Content" / "Mini" / "Data"

    ensure_clean_dir(out_root)
    generated_dir = project_root / "SourceAssets" / "Archive" / "Mini" / "Generated" / "ImageGen"
    generated_dir.mkdir(parents=True, exist_ok=True)
    copied_atlas = generated_dir / "rotmg_stage1_source_atlas.png"
    shutil.copy2(source_atlas, copied_atlas)

    source = Image.open(source_atlas).convert("RGBA")
    frames: dict[str, Image.Image] = {}
    for key, spec in CROPS.items():
        logical = 32
        output = 128
        occupancy = 0.80
        if "boss" in key:
            logical, output, occupancy = 40, 160, 0.86
        elif key.startswith("projectile"):
            logical, output, occupancy = 20, 80, 0.82
        elif key in {"coin", "gem", "heart"}:
            logical, output, occupancy = 20, 80, 0.78
        elif key == "tile_patch":
            logical, output, occupancy = 64, 128, 0.95
        frames[key] = fit_pixel_sprite(keyed_crop(source, spec.bbox), logical, output, occupancy)

    manifest: dict[str, object] = {
        "source_atlas": str(copied_atlas.relative_to(project_root)),
        "generated_by": "Tools/ArtPipeline/Minigames/T66MiniBuildRotMGPixelSet.py",
        "style": "low-res hard-pixel Mini runtime set",
        "ui_scope": "none",
        "files": [],
    }
    files: list[str] = []
    preserved_files = copy_preserved_hud_assets(project_root, out_root)

    heroes = read_csv_dicts(data_root / "T66Mini_Heroes.csv")
    for index, hero in enumerate(heroes):
        display = hero.get("DisplayName", "") or hero.get("HeroID", "")
        files.extend(write_hero_set(display, frames, out_root, HERO_ACCENTS[index % len(HERO_ACCENTS)]))

    companions = read_csv_dicts(data_root / "T66Mini_Companions.csv")
    for index, companion in enumerate(companions):
        visual = companion.get("VisualID", "") or companion.get("DisplayName", "") or companion.get("CompanionID", "")
        files.extend(write_companion_set(visual, frames["companion"], out_root, COMPANION_ACCENTS[index % len(COMPANION_ACCENTS)]))

    enemy_map = {
        "Roost": frames["roost"],
        "Cow": frames["cow"],
        "Goat": frames["goat"],
        "Pig": frames["pig"],
    }
    for visual_id, image in enemy_map.items():
        path = out_root / "Enemies" / "Singles" / f"{visual_id}.png"
        save_sprite(image, path)
        files.append(str(path))

    boss_map = {
        "Roost_Boss": frames["roost_boss"],
        "Cow_Boss": frames["cow_boss"],
        "Goat_Boss": frames["goat_boss"],
        "Pig_Boss": frames["pig_boss"],
    }
    for visual_id, image in boss_map.items():
        path = out_root / "Bosses" / "Singles" / f"{visual_id}.png"
        save_sprite(image, path)
        files.append(str(path))

    interactables = {
        "TreasureChest": frames["chest"],
        "Fountain": frames["fountain"],
        "LootCrate": frames["loot_crate"],
        "QuickReviveMachine": frames["quick_revive"],
        "LootBag_Yellow": frames["coin"],
        "LootBag_Red": frames["heart"],
        "LootBag_Black": frames["gem"],
    }
    for visual_id, image in interactables.items():
        path = out_root / "Interactables" / "Singles" / f"{visual_id}.png"
        save_sprite(image, path)
        files.append(str(path))

    effects = {
        "EnemyProjectile_Ranged": frames["projectile_red"],
        "EnemyProjectile_Boss": frames["projectile_black"],
        "EnemyProjectile_Impact": frames["projectile_yellow"],
        "Trap_Core_Arcane": frames["projectile_black"],
        "Trap_Core_Fire": frames["projectile_red"],
        "Trap_Core_Spike": frames["projectile_yellow"],
        "Trap_Pulse_Impact": frames["projectile_yellow"],
        "Trap_Telegraph_Ring": frames["gem"],
    }
    for name, image in effects.items():
        path = out_root / "Effects" / f"{name}.png"
        save_sprite(image, path)
        files.append(str(path))

    idols = read_csv_dicts(data_root / "T66Mini_Idols.csv")
    for idol in idols:
        idol_id = sanitize(idol.get("IdolID", ""))
        if not idol_id:
            continue
        path = out_root / "Idols" / "Effects" / "Singles" / f"{idol_id}.png"
        source_image = frames["projectile_yellow"] if len(files) % 2 == 0 else frames["gem"]
        save_sprite(source_image, path)
        files.append(str(path))

    files.append(make_background(frames["tile_patch"], out_root))
    write_readme(out_root, copied_atlas.relative_to(project_root))

    manifest["files"] = [str(Path(file).resolve().relative_to(project_root)) for file in files]
    manifest["preserved_ui_files"] = [str(Path(file).resolve().relative_to(project_root)) for file in preserved_files]
    write_manifest(out_root, manifest)
    print(json.dumps({"output_root": str(out_root), "file_count": len(files)}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
