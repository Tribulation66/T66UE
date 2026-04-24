from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageFont, ImageOps


ROOT = Path(__file__).resolve().parents[1]
SOURCE_IMAGE = ROOT / "UI" / "screens" / "main_menu" / "reference" / "canonical_reference_1920x1080.png"
OUTPUT_ROOT = ROOT / "SourceAssets" / "UI" / "MainMenuReference"
TOPBAR_DIR = OUTPUT_ROOT / "TopBar"
CENTER_DIR = OUTPUT_ROOT / "Center"
LEFT_DIR = OUTPUT_ROOT / "LeftPanel"
RIGHT_DIR = OUTPUT_ROOT / "RightPanel"
TILES_DIR = OUTPUT_ROOT / "Tiles"
DEBUG_DIR = OUTPUT_ROOT / "debug"
MANIFEST_PATH = OUTPUT_ROOT / "asset_manifest.json"
PREVIEW_PATH = OUTPUT_ROOT / "asset_pack_preview.png"
OVERLAY_PATH = ROOT / "UI" / "screens" / "main_menu" / "review" / "reference_crop_overlay.png"
REFERENCE_LAYOUT_PATH = OUTPUT_ROOT / "reference_layout.json"
SCENE_BACKGROUND_SOURCE_PATH = OUTPUT_ROOT / "scene_background_generated_source.png"
SCENE_BACKGROUND_CANONICAL_PATH = OUTPUT_ROOT / "scene_background_1920x1080.png"
CONTENT_OWNERSHIP_PATH = OUTPUT_ROOT / "content_ownership.json"
TOPBAR_SHELL_CLEAN_PATH = TOPBAR_DIR / "topbar_shell_clean.png"
TOPBAR_BACKDROP_CLEAN_PATH = TOPBAR_DIR / "topbar_backdrop_clean.png"
CENTER_TITLE_WORDMARK_PATH = CENTER_DIR / "title_lockup_wordmark.png"
CENTER_STACK_FRAME_SLICE_PATH = OUTPUT_ROOT / "SheetSlices" / "Center" / "stack_frame.png"
GENERATED_LAYOUT_HEADER_PATH = ROOT / "Source" / "T66" / "UI" / "Style" / "T66MainMenuReferenceLayout.generated.h"

TOPBAR_COMPAT_ALIASES = {
    "TopBar/topbar_strip_full.png": ["topbar_shell.png", "TopBar/topbar_shell.png"],
    "TopBar/button_settings.png": ["TopBar/topbar_button_utility.png", "TopBar/topbar_button_square.png"],
    "TopBar/button_chat.png": ["TopBar/topbar_button_chat.png"],
    "TopBar/button_power.png": ["TopBar/topbar_button_power.png"],
    "TopBar/badge_profile.png": ["TopBar/topbar_button_home.png", "TopBar/topbar_button_home_active.png"],
    "TopBar/currency_slot_blank.png": ["TopBar/topbar_button_currency.png", "TopBar/topbar_button_coupon.png"],
    "TopBar/search_icon.png": ["icon_search.png"],
}

try:
    RESAMPLING = Image.Resampling
except AttributeError:
    class _Resampling:
        LANCZOS = Image.LANCZOS

    RESAMPLING = _Resampling()


Box = tuple[int, int, int, int]

CANONICAL_CANVAS_SIZE = (1920, 1080)
CANONICAL_CANVAS_WIDTH = 1920.0
CANONICAL_CANVAS_HEIGHT = 1080.0


TOPBAR_BOXES: dict[str, Box] = {
    "topbar_strip_full": (0, 0, 1920, 145),
    "button_settings": (14, 18, 124, 124),
    "button_chat": (145, 18, 255, 124),
    "tab_account": (319, 18, 574, 121),
    "badge_profile": (598, 14, 689, 137),
    "tab_power_up": (707, 18, 960, 121),
    "tab_achievements": (974, 18, 1253, 121),
    "tab_minigames": (1268, 18, 1525, 121),
    "currency_slot": (1572, 26, 1762, 112),
    "button_power": (1781, 18, 1902, 124),
}

TOPBAR_ICON_BOXES: dict[str, Box] = {
    "icon_powerup": (721, 34, 765, 83),
    "icon_achievements": (988, 36, 1033, 83),
    "icon_minigames": (1282, 34, 1330, 85),
}

CENTER_BOXES: dict[str, Box] = {
    "center_backdrop_full": (460, 138, 1463, 1080),
    "title_lockup": (459, 138, 1464, 326),
    "subtitle_lockup": (725, 252, 1249, 323),
    "hero_stage": (475, 298, 1450, 714),
    "cta_stack_full": (742, 698, 1300, 1009),
    "cta_button_new_game": (760, 720, 1282, 818),
    "cta_button_load_game": (760, 821, 1277, 915),
    "cta_button_daily_challenge": (762, 917, 1278, 1008),
}

LEFT_BOXES: dict[str, Box] = {
    "shell_full_reference": (21, 349, 508, 1075),
    "profile_card_reference": (40, 360, 464, 468),
    "search_field_reference": (46, 490, 450, 544),
    "search_icon": (405, 499, 439, 531),
    "friend_star_button": (291, 598, 353, 655),
    "friend_invite_button": (355, 598, 451, 655),
    "friend_offline_button": (355, 771, 452, 829),
    "friend_avatar_frame_source": (45, 596, 100, 652),
    "party_slot_source": (44, 916, 137, 1019),
    "close_button": (415, 862, 466, 910),
}

RIGHT_BOXES: dict[str, Box] = {
    "shell_full_reference": (1430, 368, 1888, 1077),
    "filter_world_button": (1497, 290, 1571, 362),
    "filter_friends_button": (1578, 290, 1652, 362),
    "filter_crown_button": (1659, 290, 1735, 362),
    "tab_weekly_active": (1445, 383, 1654, 448),
    "tab_all_time_inactive": (1652, 383, 1871, 448),
    "dropdown_shell_left": (1447, 456, 1655, 513),
    "dropdown_shell_right": (1658, 456, 1869, 513),
    "toggle_score_selected": (1445, 521, 1655, 582),
    "toggle_speedrun_unselected": (1658, 521, 1874, 582),
    "leaderboard_avatar_frame_source": (1533, 631, 1589, 687),
}

LAYOUT_METRICS: dict[str, float] = {
    "canvas_width": CANONICAL_CANVAS_WIDTH,
    "canvas_height": CANONICAL_CANVAS_HEIGHT,
    "topbar_reserved_height": 163.0,
    "topbar_surface_height": 145.0,
    "topbar_surface_offset_y": 0.0,
}

EXTRA_LAYOUT_BOXES: dict[str, dict[str, Box]] = {
    "MainMenu": {
        "full_canvas": (0, 0, 1920, 1080),
        "left_panel_assembly": (21, 349, 508, 1075),
        "right_panel_assembly": (1430, 290, 1888, 1077),
    }
}

CENTER_RUNTIME_PLATE_BOXES: dict[str, Box] = {
    "cta_button_new_game_plate": (760, 720, 1246, 818),
    "cta_button_load_game_plate": (760, 821, 1242, 915),
    "cta_button_daily_challenge_plate": (762, 917, 1244, 1008),
}

FILL_BOXES: dict[str, Box] = {
    "panel_fill_dark": (1481, 852, 1578, 948),
    "topbar_fill_warm": (260, 21, 310, 106),
}


def ensure_dirs() -> None:
    for directory in (OUTPUT_ROOT, TOPBAR_DIR, CENTER_DIR, LEFT_DIR, RIGHT_DIR, TILES_DIR, DEBUG_DIR):
        directory.mkdir(parents=True, exist_ok=True)


def crop(image: Image.Image, box: Box) -> Image.Image:
    return image.crop(box).convert("RGBA")


def rect_relative_to(box: Box, parent_box: Box) -> Box:
    return (
        box[0] - parent_box[0],
        box[1] - parent_box[1],
        box[2] - parent_box[0],
        box[3] - parent_box[1],
    )


def tile_fill(tile: Image.Image, size: tuple[int, int]) -> Image.Image:
    tiled = Image.new("RGBA", size, (0, 0, 0, 0))
    for y in range(0, size[1], tile.height):
        for x in range(0, size[0], tile.width):
            tiled.alpha_composite(tile, (x, y))
    return tiled


def fill_rectangles(base: Image.Image, rectangles: list[Box], tile: Image.Image) -> Image.Image:
    result = base.copy()
    for left, top, right, bottom in rectangles:
        fill = tile_fill(tile, (right - left, bottom - top))
        result.alpha_composite(fill, (left, top))
    return result


def transparent_center(base: Image.Image, inner_box: Box) -> Image.Image:
    result = base.copy()
    alpha = result.getchannel("A")
    ImageDraw.Draw(alpha).rectangle(inner_box, fill=0)
    result.putalpha(alpha)
    return result


def transparent_rectangles(base: Image.Image, rectangles: list[Box]) -> Image.Image:
    result = base.copy()
    alpha = result.getchannel("A")
    draw = ImageDraw.Draw(alpha)
    for rectangle in rectangles:
        draw.rectangle(rectangle, fill=0)
    result.putalpha(alpha)
    return result


def save(image: Image.Image, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)


def canonicalize_image(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    if rgba.size == CANONICAL_CANVAS_SIZE:
        return rgba
    return rgba.resize(CANONICAL_CANVAS_SIZE, RESAMPLING.LANCZOS)


def load_canonical_reference() -> Image.Image:
    with Image.open(SOURCE_IMAGE) as source:
        original_size = source.size
        reference = canonicalize_image(source)
    if reference.size != original_size:
        reference.save(SOURCE_IMAGE)
    return reference


def canonicalize_image_file(path: Path) -> bool:
    if not path.exists():
        return False
    with Image.open(path) as source:
        original_size = source.size
        image = canonicalize_image(source)
    if image.size != original_size:
        image.save(path)
    return True


def flood_clear_background(image: Image.Image, threshold: int = 18) -> Image.Image:
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    background = pixels[0, 0][:3]

    def is_background(x: int, y: int) -> bool:
        pixel = pixels[x, y]
        return max(abs(pixel[i] - background[i]) for i in range(3)) <= threshold

    visited = [[False for _ in range(width)] for _ in range(height)]
    queue: list[tuple[int, int]] = []

    for x in range(width):
        queue.append((x, 0))
        queue.append((x, height - 1))
    for y in range(height):
        queue.append((0, y))
        queue.append((width - 1, y))

    while queue:
        x, y = queue.pop()
        if x < 0 or y < 0 or x >= width or y >= height or visited[y][x]:
            continue
        visited[y][x] = True
        if not is_background(x, y):
            continue
        pixels[x, y] = (0, 0, 0, 0)
        queue.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))

    return rgba


def copy_alias(source: Path, relative_destination: str) -> Path:
    destination = OUTPUT_ROOT / relative_destination
    destination.parent.mkdir(parents=True, exist_ok=True)
    source_image = Image.open(source).convert("RGBA")
    source_image.save(destination)
    return destination


def load_font(size: int) -> ImageFont.ImageFont:
    candidates = [
        Path("C:/Windows/Fonts/arialbd.ttf"),
        Path("C:/Windows/Fonts/arial.ttf"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size=size)
    return ImageFont.load_default()


def add_manifest_entry(
    manifest: list[dict[str, object]],
    *,
    path: Path,
    group: str,
    kind: str,
    source_box: Box | None = None,
    notes: str,
    clean_boxes: list[Box] | None = None,
    recommended_margin_px: int | None = None,
) -> None:
    entry: dict[str, object] = {
        "path": path.relative_to(OUTPUT_ROOT).as_posix(),
        "group": group,
        "kind": kind,
        "notes": notes,
    }
    if source_box is not None:
        entry["source_box"] = list(source_box)
    if clean_boxes:
        entry["clean_boxes"] = [list(box) for box in clean_boxes]
    if recommended_margin_px is not None:
        entry["recommended_margin_px"] = recommended_margin_px
    manifest.append(entry)


def build_overlay(reference: Image.Image) -> None:
    overlay = reference.copy()
    draw = ImageDraw.Draw(overlay)
    font = load_font(16)

    groups = {
        "TopBar": (TOPBAR_BOXES, (236, 176, 57, 255)),
        "Center": (CENTER_BOXES, (185, 88, 255, 255)),
        "Left": (LEFT_BOXES, (102, 219, 167, 255)),
        "Right": (RIGHT_BOXES, (106, 176, 255, 255)),
        "Tiles": (FILL_BOXES, (255, 118, 118, 255)),
    }

    for label, (boxes, color) in groups.items():
        for name, box in boxes.items():
            draw.rectangle(box, outline=color, width=2)
            text = f"{label}:{name}"
            text_box = draw.textbbox((0, 0), text, font=font)
            text_width = text_box[2] - text_box[0]
            text_height = text_box[3] - text_box[1]
            text_bg = (box[0], max(0, box[1] - text_height - 6), box[0] + text_width + 8, max(0, box[1] - 2))
            draw.rectangle(text_bg, fill=(0, 0, 0, 180))
            draw.text((text_bg[0] + 4, text_bg[1] + 2), text, fill=color, font=font)

    save(overlay, OVERLAY_PATH)


def build_preview(preview_paths: list[tuple[str, Path]]) -> None:
    font = load_font(18)
    cols = 4
    thumb_size = (320, 180)
    card_size = (360, 240)
    rows = (len(preview_paths) + cols - 1) // cols
    sheet = Image.new("RGBA", (cols * card_size[0], rows * card_size[1]), (18, 18, 22, 255))

    for index, (label, path) in enumerate(preview_paths):
        image = Image.open(path).convert("RGBA")
        thumb = ImageOps.contain(image, thumb_size, RESAMPLING.LANCZOS)
        card = Image.new("RGBA", card_size, (28, 30, 34, 255))
        border = Image.new("RGBA", card_size, (0, 0, 0, 0))
        border_draw = ImageDraw.Draw(border)
        border_draw.rectangle((0, 0, card_size[0] - 1, card_size[1] - 1), outline=(122, 108, 82, 255), width=2)
        card.alpha_composite(border)
        card.alpha_composite(thumb, ((card.width - thumb.width) // 2, 20))
        label_box = draw_label_box(card.width, 40, label, font)
        card.alpha_composite(label_box, (0, card.height - 52))
        x = (index % cols) * card_size[0]
        y = (index // cols) * card_size[1]
        sheet.alpha_composite(card, (x, y))

    save(sheet, PREVIEW_PATH)


def draw_label_box(width: int, height: int, text: str, font: ImageFont.ImageFont) -> Image.Image:
    label = Image.new("RGBA", (width, height), (12, 12, 14, 230))
    draw = ImageDraw.Draw(label)
    text_box = draw.textbbox((0, 0), text, font=font)
    text_width = text_box[2] - text_box[0]
    text_height = text_box[3] - text_box[1]
    draw.text(((width - text_width) // 2, (height - text_height) // 2 - 1), text, fill=(235, 227, 208, 255), font=font)
    return label


def box_to_layout_dict(box: Box) -> dict[str, int]:
    return {
        "x": int(box[0]),
        "y": int(box[1]),
        "width": int(box[2] - box[0]),
        "height": int(box[3] - box[1]),
    }


def snake_to_pascal(value: str) -> str:
    return "".join(chunk.capitalize() for chunk in value.split("_"))


def write_reference_layout_artifacts() -> None:
    groups: dict[str, dict[str, Box]] = {
        "TopBar": TOPBAR_BOXES,
        "Center": CENTER_BOXES,
        "CenterRuntime": CENTER_RUNTIME_PLATE_BOXES,
        "Left": LEFT_BOXES,
        "Right": RIGHT_BOXES,
        **EXTRA_LAYOUT_BOXES,
    }

    layout_payload = {
        "canvas": {
            "width": int(LAYOUT_METRICS["canvas_width"]),
            "height": int(LAYOUT_METRICS["canvas_height"]),
        },
        "metrics": {key: value for key, value in LAYOUT_METRICS.items() if not key.startswith("canvas_")},
        "groups": {
            group_name: {name: box_to_layout_dict(box) for name, box in boxes.items()}
            for group_name, boxes in groups.items()
        },
    }
    REFERENCE_LAYOUT_PATH.write_text(json.dumps(layout_payload, indent=2), encoding="utf-8")

    lines = [
        "// Copyright Tribulation 66. All Rights Reserved.",
        "",
        "#pragma once",
        "",
        "namespace T66MainMenuReferenceLayout",
        "{",
        f"\tinline constexpr float CanvasWidth = {LAYOUT_METRICS['canvas_width']:.1f}f;",
        f"\tinline constexpr float CanvasHeight = {LAYOUT_METRICS['canvas_height']:.1f}f;",
        f"\tinline constexpr float TopBarReservedHeight = {LAYOUT_METRICS['topbar_reserved_height']:.1f}f;",
        f"\tinline constexpr float TopBarSurfaceHeight = {LAYOUT_METRICS['topbar_surface_height']:.1f}f;",
        f"\tinline constexpr float TopBarSurfaceOffsetY = {LAYOUT_METRICS['topbar_surface_offset_y']:.1f}f;",
        "",
    ]

    for group_name, boxes in groups.items():
        lines.append(f"\tnamespace {group_name}")
        lines.append("\t{")
        for name, box in boxes.items():
            rect = box_to_layout_dict(box)
            lines.append(
                f"\t\tinline constexpr FT66ReferenceRect {snake_to_pascal(name)}"
                f"{{{rect['x']:.1f}f, {rect['y']:.1f}f, {rect['width']:.1f}f, {rect['height']:.1f}f}};"
            )
        lines.append("\t}")
        lines.append("")

    lines.append("}")
    GENERATED_LAYOUT_HEADER_PATH.write_text("\n".join(lines), encoding="utf-8")


def fill_rectangles_from_sample(base: Image.Image, rectangles: list[Box], sample_rect: Box) -> Image.Image:
    tile = base.crop(sample_rect)
    return fill_rectangles(base, rectangles, tile)


def soften_tagline_for_runtime_master(base: Image.Image) -> Image.Image:
    """Remove baked tagline glyphs while preserving the painted title lockup."""
    result = base.copy()
    subtitle_box = CENTER_BOXES["subtitle_lockup"]
    expanded_box = (
        max(0, subtitle_box[0] - 21),
        max(0, subtitle_box[1] - 15),
        min(result.width, subtitle_box[2] + 32),
        min(result.height, subtitle_box[3] - 6),
    )
    region = result.crop(expanded_box)
    blurred_region = result.filter(ImageFilter.GaussianBlur(14)).crop(expanded_box)
    mask = Image.new("L", region.size, 0)
    region_pixels = region.load()
    mask_pixels = mask.load()

    for y in range(region.height):
        for x in range(region.width):
            red, green, blue, alpha = region_pixels[x, y]
            strongest = max(red, green, blue)
            weakest = min(red, green, blue)
            if alpha and strongest > 125 and (red > 115 or blue > 135) and (strongest - weakest) > 20:
                mask_pixels[x, y] = 255

    mask = mask.filter(ImageFilter.MaxFilter(7)).filter(ImageFilter.GaussianBlur(3))
    region.paste(blurred_region, (0, 0), mask)
    result.paste(region, expanded_box[:2])
    return result


def extract_title_wordmark(reference: Image.Image) -> Image.Image:
    """Extract the baked title art while removing the localizable tagline."""
    title = crop(reference, CENTER_BOXES["title_lockup"])
    mask = Image.new("L", title.size, 0)
    pixels = title.load()
    mask_pixels = mask.load()

    for y in range(title.height):
        for x in range(title.width):
            red, green, blue, alpha = pixels[x, y]
            if not alpha or y > 112:
                continue

            strongest = max(red, green, blue)
            weakest = min(red, green, blue)
            saturation = strongest - weakest
            is_gold = (
                red > 116
                and green > 82
                and blue > 42
                and strongest > 118
                and (red >= blue + 18 or green >= blue + 8)
            )
            is_purple_gem = (
                (x < 120 or x > 755)
                and blue > 105
                and red > 60
                and green < 135
                and saturation > 32
            )
            is_top_highlight = y < 86 and strongest > 168 and saturation > 25
            if is_gold or is_purple_gem or is_top_highlight:
                mask_pixels[x, y] = 255

    soft_mask = mask.filter(ImageFilter.MaxFilter(7)).filter(ImageFilter.GaussianBlur(1.0))
    soft_pixels = soft_mask.load()
    for y in range(title.height):
        for x in range(title.width):
            if 170 <= x <= 730 and y >= 92:
                soft_pixels[x, y] = 0
            elif y > 110:
                soft_pixels[x, y] = 0

    result = title.copy()
    result.putalpha(soft_mask)
    result_pixels = result.load()
    for y in range(result.height):
        for x in range(result.width):
            red, green, blue, alpha = result_pixels[x, y]
            if alpha < 12:
                result_pixels[x, y] = (red, green, blue, 0)
    return result


def soften_text_for_runtime_plate(base: Image.Image, search_rects: list[Box]) -> Image.Image:
    result = base.copy()
    blurred = base.filter(ImageFilter.GaussianBlur(8))
    mask = Image.new("L", base.size, 0)
    pixels = base.load()
    mask_pixels = mask.load()

    for left, top, right, bottom in search_rects:
        for y in range(max(0, top), min(base.height, bottom)):
            for x in range(max(0, left), min(base.width, right)):
                red, green, blue, alpha = pixels[x, y]
                if not alpha:
                    continue

                strongest = max(red, green, blue)
                weakest = min(red, green, blue)
                if strongest > 130 and weakest > 92 and (strongest - weakest) < 85:
                    mask_pixels[x, y] = 255

    mask = mask.filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.GaussianBlur(1.2))
    result.paste(blurred, (0, 0), mask)
    return result


def stretch_fill_rectangles_from_sample(base: Image.Image, rectangles: list[Box], sample_rect: Box) -> Image.Image:
    result = base.copy()
    sample = base.crop(sample_rect).filter(ImageFilter.GaussianBlur(radius=6))
    for left, top, right, bottom in rectangles:
        stretched = sample.resize((right - left, bottom - top), RESAMPLING.LANCZOS)
        result.alpha_composite(stretched, (left, top))
    return result


def brighten_region(image: Image.Image, region: Box, factor: float) -> Image.Image:
    result = image.copy()
    segment = result.crop(region)
    segment = ImageEnhance.Brightness(segment).enhance(factor)
    result.alpha_composite(segment, region[:2])
    return result


def add_alias_entries(
    manifest: list[dict[str, object]],
    *,
    source_box: Box,
    source_path: Path,
    aliases: list[str],
    group: str,
    notes: str,
) -> None:
    existing_paths = {entry.get("path") for entry in manifest}
    source_relative = source_path.relative_to(OUTPUT_ROOT).as_posix()
    for alias in aliases:
        alias_path = copy_alias(source_path, alias)
        alias_relative = alias_path.relative_to(OUTPUT_ROOT).as_posix()
        if alias_relative == source_relative or alias_relative in existing_paths:
            continue
        add_manifest_entry(
            manifest,
            path=alias_path,
            group=group,
            kind="alias_copy",
            source_box=source_box,
            notes=notes,
        )
        existing_paths.add(alias_relative)


def build_assets() -> None:
    from BuildCanonicalMainMenuReferencePack import build_pack

    build_pack()
    return

    ensure_dirs()
    reference = load_canonical_reference()
    master_background = reference.copy()

    manifest: list[dict[str, object]] = []
    preview_paths: list[tuple[str, Path]] = []

    scene_plate_specs = [
        (
            SCENE_BACKGROUND_SOURCE_PATH,
            "ui_free_scene_plate_source",
            "AI-generated UI-free dungeon/idol scene plate source. This is the production background family input, not the full reference screenshot.",
        ),
        (
            SCENE_BACKGROUND_CANONICAL_PATH,
            "ui_free_scene_plate_canonical",
            "Canonical 1920x1080 UI-free scene/background plate.",
        ),
    ]
    for scene_path, kind, notes in scene_plate_specs:
        if canonicalize_image_file(scene_path):
            add_manifest_entry(
                manifest,
                path=scene_path,
                group="Scene",
                kind=kind,
                notes=notes,
            )
            preview_paths.append((f"Scene/{scene_path.stem}", scene_path))

    for name, box in FILL_BOXES.items():
        path = TILES_DIR / f"{name}.png"
        save(crop(reference, box), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="Tiles",
            kind="fill_tile",
            source_box=box,
            notes="Reference fill sample for shell cleanup or textured repeats.",
        )

    panel_fill_tile = crop(reference, FILL_BOXES["panel_fill_dark"])
    topbar_fill_tile = crop(reference, FILL_BOXES["topbar_fill_warm"])
    center_fill_tile = crop(reference, FILL_BOXES["panel_fill_dark"])

    for name, box in TOPBAR_BOXES.items():
        path = TOPBAR_DIR / f"{name}.png"
        save(crop(reference, box), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="TopBar",
            kind="exact_crop",
            source_box=box,
            notes="Exact reference crop for the static top-bar rebuild.",
        )
        preview_paths.append((f"TopBar/{name}", path))

    for name, box in TOPBAR_ICON_BOXES.items():
        path = TOPBAR_DIR / f"{name}.png"
        save(flood_clear_background(crop(reference, box)), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="TopBar",
            kind="transparent_icon",
            source_box=box,
            notes="Transparent icon crop derived from the approved reference for live top-bar icon layering.",
        )
        preview_paths.append((f"TopBar/{name}", path))

    for relative_source, aliases in TOPBAR_COMPAT_ALIASES.items():
        source_path = OUTPUT_ROOT / relative_source
        if source_path.exists():
            add_alias_entries(
                manifest,
                source_box=TOPBAR_BOXES["topbar_strip_full"] if "topbar_strip_full" in relative_source else TOPBAR_BOXES["button_settings"],
                source_path=source_path,
                aliases=aliases,
                group="TopBar",
                notes="Compatibility alias for the top-bar widget loader.",
            )

    currency_slot_blank = fill_rectangles(
        crop(reference, TOPBAR_BOXES["currency_slot"]),
        [(88, 8, 150, 64)],
        topbar_fill_tile,
    )
    currency_slot_blank_path = TOPBAR_DIR / "currency_slot_blank.png"
    save(currency_slot_blank, currency_slot_blank_path)
    master_background.alpha_composite(currency_slot_blank, TOPBAR_BOXES["currency_slot"][:2])
    add_manifest_entry(
        manifest,
        path=currency_slot_blank_path,
        group="TopBar",
        kind="clean_component",
        source_box=TOPBAR_BOXES["currency_slot"],
        clean_boxes=[(1457, 31, 1519, 87)],
        notes="Coupon slot with the baked count removed so the live currency value can render cleanly.",
    )
    preview_paths.append(("TopBar/currency_slot_blank", currency_slot_blank_path))

    topbar_text_cleanup_specs = [
        {
            "source_name": "tab_account",
            "output_name": "topbar_button_account",
            "fill_rects": [(45, 18, 177, 64)],
            "sample_rect": (26, 66, 196, 72),
            "aliases": [
                "topbar_button_account.png",
                "TopBar/topbar_button_account.png",
            ],
        },
        {
            "source_name": "tab_power_up",
            "output_name": "topbar_button_nav_power_up",
            "fill_rects": [(68, 18, 196, 64)],
            "sample_rect": (58, 66, 204, 72),
            "aliases": [],
        },
        {
            "source_name": "tab_achievements",
            "output_name": "topbar_button_nav_achievements",
            "fill_rects": [(80, 18, 224, 64)],
            "sample_rect": (68, 66, 230, 72),
            "aliases": [],
        },
        {
            "source_name": "tab_minigames",
            "output_name": "topbar_button_nav_minigames",
            "fill_rects": [(66, 18, 204, 64)],
            "sample_rect": (56, 66, 208, 72),
            "aliases": [],
        },
    ]

    nav_clean_paths: list[Path] = []
    for spec in topbar_text_cleanup_specs:
        source = crop(reference, TOPBAR_BOXES[spec["source_name"]])
        cleaned = stretch_fill_rectangles_from_sample(source, spec["fill_rects"], spec["sample_rect"])
        clean_path = TOPBAR_DIR / f"{spec['output_name']}.png"
        hover_path = TOPBAR_DIR / f"{spec['output_name']}_hover.png"
        pressed_path = TOPBAR_DIR / f"{spec['output_name']}_pressed.png"
        save(cleaned, clean_path)
        save(brighten_region(cleaned, (0, 0, cleaned.width, cleaned.height), 1.10), hover_path)
        save(brighten_region(cleaned, (0, 0, cleaned.width, cleaned.height), 0.92), pressed_path)
        add_manifest_entry(
            manifest,
            path=clean_path,
            group="TopBar",
            kind="clean_component",
            source_box=TOPBAR_BOXES[spec["source_name"]],
            notes="Top-bar plate with baked label removed for live localizable text.",
        )
        add_manifest_entry(
            manifest,
            path=hover_path,
            group="TopBar",
            kind="state_variant",
            source_box=TOPBAR_BOXES[spec["source_name"]],
            notes="Hover-state brightness pass for the cleaned top-bar plate.",
        )
        add_manifest_entry(
            manifest,
            path=pressed_path,
            group="TopBar",
            kind="state_variant",
            source_box=TOPBAR_BOXES[spec["source_name"]],
            notes="Pressed-state brightness pass for the cleaned top-bar plate.",
        )
        preview_paths.append((f"TopBar/{spec['output_name']}", clean_path))
        nav_clean_paths.append(clean_path)
        if spec["aliases"]:
            add_alias_entries(
                manifest,
                source_box=TOPBAR_BOXES[spec["source_name"]],
                source_path=clean_path,
                aliases=spec["aliases"],
                group="TopBar",
                notes="Compatibility alias for cleaned top-bar account plate.",
            )

    if len(nav_clean_paths) >= 3:
        generic_nav_idle = nav_clean_paths[0].with_name("topbar_button_nav.png")
        generic_nav_hover = nav_clean_paths[0].with_name("topbar_button_nav_hover.png")
        generic_nav_pressed = nav_clean_paths[0].with_name("topbar_button_nav_pressed.png")
        Image.open(nav_clean_paths[1]).convert("RGBA").save(generic_nav_idle)
        Image.open(nav_clean_paths[1].with_name("topbar_button_nav_power_up_hover.png")).convert("RGBA").save(generic_nav_hover)
        Image.open(nav_clean_paths[1].with_name("topbar_button_nav_power_up_pressed.png")).convert("RGBA").save(generic_nav_pressed)
        add_manifest_entry(
            manifest,
            path=generic_nav_idle,
            group="TopBar",
            kind="alias_copy",
            source_box=TOPBAR_BOXES["tab_power_up"],
            notes="Canonical shared nav plate copied from the cleaned power-up nav plate for the top-bar widget loader.",
        )
        add_alias_entries(
            manifest,
            source_box=TOPBAR_BOXES["tab_power_up"],
            source_path=generic_nav_idle,
            aliases=[
                "topbar_button_nav.png",
                "TopBar/topbar_button_nav.png",
            ],
            group="TopBar",
            notes="Compatibility alias for cleaned shared nav plate.",
        )
        add_manifest_entry(
            manifest,
            path=generic_nav_hover,
            group="TopBar",
            kind="state_variant",
            source_box=TOPBAR_BOXES["tab_power_up"],
            notes="Compatibility hover-state pass for the cleaned shared nav plate.",
        )
        add_manifest_entry(
            manifest,
            path=generic_nav_pressed,
            group="TopBar",
            kind="state_variant",
            source_box=TOPBAR_BOXES["tab_power_up"],
            notes="Compatibility pressed-state pass for the cleaned shared nav plate.",
        )

    account_active_path = TOPBAR_DIR / "topbar_button_account_active.png"
    save(brighten_region(Image.open(TOPBAR_DIR / "topbar_button_account.png").convert("RGBA"), (0, 0, TOPBAR_BOXES["tab_account"][2] - TOPBAR_BOXES["tab_account"][0], TOPBAR_BOXES["tab_account"][3] - TOPBAR_BOXES["tab_account"][1]), 1.06), account_active_path)
    add_manifest_entry(
        manifest,
        path=account_active_path,
        group="TopBar",
        kind="state_variant",
        source_box=TOPBAR_BOXES["tab_account"],
        notes="Active-state brightness pass for the cleaned account plate.",
    )
    add_alias_entries(
        manifest,
        source_box=TOPBAR_BOXES["tab_account"],
        source_path=account_active_path,
        aliases=[
            "topbar_button_account_active.png",
            "TopBar/topbar_button_account_active.png",
        ],
        group="TopBar",
        notes="Compatibility alias for cleaned active account plate.",
    )

    for name, box in CENTER_BOXES.items():
        path = CENTER_DIR / f"{name}.png"
        save(crop(reference, box), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="Center",
            kind="exact_crop",
            source_box=box,
            notes="Exact center-scene reference crop for the mirror build.",
        )
        preview_paths.append((f"Center/{name}", path))

    title_wordmark = extract_title_wordmark(reference)
    save(title_wordmark, CENTER_TITLE_WORDMARK_PATH)
    add_manifest_entry(
        manifest,
        path=CENTER_TITLE_WORDMARK_PATH,
        group="Center",
        kind="baked_title_art",
        source_box=CENTER_BOXES["title_lockup"],
        notes="Transparent title wordmark and side crystal art. The tagline below it is deliberately removed and remains live localizable text.",
    )
    preview_paths.append(("Center/title_lockup_wordmark", CENTER_TITLE_WORDMARK_PATH))

    center_runtime_plate_specs = [
        {
            "layout_name": "cta_button_new_game_plate",
            "output_name": "cta_button_new_game_plate",
            "text_search_rects": [(42, 12, 374, 72)],
        },
        {
            "layout_name": "cta_button_load_game_plate",
            "output_name": "cta_button_load_game_plate",
            "text_search_rects": [(42, 8, 370, 68)],
        },
        {
            "layout_name": "cta_button_daily_challenge_plate",
            "output_name": "cta_button_daily_challenge_plate",
            "text_search_rects": [(36, 6, 382, 64)],
        },
    ]

    for spec in center_runtime_plate_specs:
        plate_box = CENTER_RUNTIME_PLATE_BOXES[spec["layout_name"]]
        plate_size = (plate_box[2] - plate_box[0], plate_box[3] - plate_box[1])
        normal_path = CENTER_DIR / f"{spec['output_name']}.png"
        hover_path = CENTER_DIR / f"{spec['output_name']}_hover.png"
        pressed_path = CENTER_DIR / f"{spec['output_name']}_pressed.png"
        disabled_path = CENTER_DIR / f"{spec['output_name']}_disabled.png"

        normal_plate = soften_text_for_runtime_plate(crop(reference, plate_box), spec["text_search_rects"])
        hover_plate = brighten_region(normal_plate, (0, 0, plate_size[0], plate_size[1]), 1.08)
        pressed_plate = brighten_region(normal_plate, (0, 0, plate_size[0], plate_size[1]), 0.88)
        disabled_plate = ImageEnhance.Brightness(ImageEnhance.Color(normal_plate).enhance(0.45)).enhance(0.66)

        save(normal_plate, normal_path)
        save(hover_plate, hover_path)
        save(pressed_plate, pressed_path)
        save(disabled_plate, disabled_path)

        add_manifest_entry(
            manifest,
            path=normal_path,
            group="Center",
            kind="runtime_plate",
            source_box=plate_box,
            notes="Exact reference CTA plate with baked label removed for live localizable text.",
        )
        add_manifest_entry(
            manifest,
            path=hover_path,
            group="Center",
            kind="state_variant",
            source_box=plate_box,
            notes="Hover-state CTA runtime plate promoted from the generated center family.",
        )
        add_manifest_entry(
            manifest,
            path=pressed_path,
            group="Center",
            kind="state_variant",
            source_box=plate_box,
            notes="Pressed-state CTA runtime plate promoted from the generated center family.",
        )
        add_manifest_entry(
            manifest,
            path=disabled_path,
            group="Center",
            kind="state_variant",
            source_box=plate_box,
            notes="Disabled-state CTA runtime plate promoted from the generated center family.",
        )
        preview_paths.append((f"Center/{spec['output_name']}", normal_path))

    center_runtime_wide_plate_specs = [
        {
            "layout_name": "cta_button_new_game",
            "output_name": "cta_button_new_game_wide_plate",
        },
        {
            "layout_name": "cta_button_load_game",
            "output_name": "cta_button_load_game_wide_plate",
        },
        {
            "layout_name": "cta_button_daily_challenge",
            "output_name": "cta_button_daily_challenge_wide_plate",
        },
    ]
    for spec in center_runtime_wide_plate_specs:
        source_box = CENTER_BOXES[spec["layout_name"]]
        for suffix, kind in (
            ("", "runtime_plate"),
            ("_hover", "state_variant"),
            ("_pressed", "state_variant"),
            ("_disabled", "state_variant"),
        ):
            plate_path = CENTER_DIR / f"{spec['output_name']}{suffix}.png"
            if not plate_path.exists():
                continue
            add_manifest_entry(
                manifest,
                path=plate_path,
                group="Center",
                kind=kind,
                source_box=source_box,
                notes="Clean wide CTA runtime plate promoted from the generated center family with no baked label text.",
            )
        normal_wide_path = CENTER_DIR / f"{spec['output_name']}.png"
        if normal_wide_path.exists():
            preview_paths.append((f"Center/{spec['output_name']}", normal_wide_path))

    left_shell_inner = (31, 317, 429, 921)
    left_shell_clean = fill_rectangles(
        crop(reference, LEFT_BOXES["shell_full_reference"]),
        [rect_relative_to(left_shell_inner, LEFT_BOXES["shell_full_reference"])],
        panel_fill_tile,
    )
    left_shell_clean_path = LEFT_DIR / "shell_clean.png"
    save(left_shell_clean, left_shell_clean_path)
    add_manifest_entry(
        manifest,
        path=left_shell_clean_path,
        group="LeftPanel",
        kind="clean_shell",
        source_box=LEFT_BOXES["shell_full_reference"],
        clean_boxes=[left_shell_inner],
        recommended_margin_px=22,
        notes="Outer left panel shell with the interior reset to dark fill for live Slate content.",
    )

    profile_card_avatar_box = (41, 320, 129, 401)
    profile_card_text_box = (142, 323, 397, 398)
    profile_card_shell = fill_rectangles(
        crop(reference, LEFT_BOXES["profile_card_reference"]),
        [
            rect_relative_to(profile_card_avatar_box, LEFT_BOXES["profile_card_reference"]),
            rect_relative_to(profile_card_text_box, LEFT_BOXES["profile_card_reference"]),
        ],
        panel_fill_tile,
    )
    profile_card_shell_path = LEFT_DIR / "profile_card_shell.png"
    save(profile_card_shell, profile_card_shell_path)
    add_manifest_entry(
        manifest,
        path=profile_card_shell_path,
        group="LeftPanel",
        kind="clean_component",
        source_box=LEFT_BOXES["profile_card_reference"],
        clean_boxes=[profile_card_avatar_box, profile_card_text_box],
        notes="Profile card frame cleaned for live avatar and account text.",
    )

    profile_avatar_reference_path = LEFT_DIR / "profile_avatar_reference.png"
    save(crop(reference, profile_card_avatar_box), profile_avatar_reference_path)
    add_manifest_entry(
        manifest,
        path=profile_avatar_reference_path,
        group="LeftPanel",
        kind="exact_crop",
        source_box=profile_card_avatar_box,
        notes="Exact profile portrait fallback crop from the reference card.",
    )

    search_field_shell = fill_rectangles(
        crop(reference, LEFT_BOXES["search_field_reference"]),
        [rect_relative_to((55, 437, 338, 465), LEFT_BOXES["search_field_reference"])],
        panel_fill_tile,
    )
    search_field_shell_path = LEFT_DIR / "search_field_shell.png"
    save(search_field_shell, search_field_shell_path)
    add_manifest_entry(
        manifest,
        path=search_field_shell_path,
        group="LeftPanel",
        kind="clean_component",
        source_box=LEFT_BOXES["search_field_reference"],
        clean_boxes=[(55, 437, 338, 465)],
        notes="Search field shell with placeholder text removed and icon preserved.",
    )

    friend_avatar_frame = transparent_center(
        crop(reference, LEFT_BOXES["friend_avatar_frame_source"]),
        rect_relative_to((43, 523, 84, 565), LEFT_BOXES["friend_avatar_frame_source"]),
    )
    friend_avatar_frame_path = LEFT_DIR / "friend_avatar_frame.png"
    save(friend_avatar_frame, friend_avatar_frame_path)
    add_manifest_entry(
        manifest,
        path=friend_avatar_frame_path,
        group="LeftPanel",
        kind="frame",
        source_box=LEFT_BOXES["friend_avatar_frame_source"],
        clean_boxes=[(43, 523, 84, 565)],
        notes="Friend avatar frame with transparent center for live Steam portraits.",
    )

    party_slot_frame = transparent_center(
        crop(reference, LEFT_BOXES["party_slot_source"]),
        rect_relative_to((44, 804, 113, 881), LEFT_BOXES["party_slot_source"]),
    )
    party_slot_frame_path = LEFT_DIR / "party_slot_frame.png"
    save(party_slot_frame, party_slot_frame_path)
    add_manifest_entry(
        manifest,
        path=party_slot_frame_path,
        group="LeftPanel",
        kind="frame",
        source_box=LEFT_BOXES["party_slot_source"],
        clean_boxes=[(44, 804, 113, 881)],
        notes="Party slot frame with transparent center for party member portraits.",
    )

    for name in (
        "shell_full_reference",
        "profile_card_reference",
        "search_field_reference",
        "search_icon",
        "friend_star_button",
        "friend_invite_button",
        "friend_offline_button",
        "close_button",
    ):
        path = LEFT_DIR / f"{name}.png"
        save(crop(reference, LEFT_BOXES[name]), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="LeftPanel",
            kind="exact_crop",
            source_box=LEFT_BOXES[name],
            notes="Exact reference crop for the left social panel rebuild.",
        )
        preview_paths.append((f"Left/{name}", path))

    preview_paths.append(("Left/shell_clean", left_shell_clean_path))
    preview_paths.append(("Left/profile_card_shell", profile_card_shell_path))
    preview_paths.append(("Left/search_field_shell", search_field_shell_path))
    preview_paths.append(("Left/friend_avatar_frame", friend_avatar_frame_path))
    preview_paths.append(("Left/party_slot_frame", party_slot_frame_path))

    right_shell_inner = (1258, 334, 1631, 892)
    right_shell_clean = fill_rectangles(
        crop(reference, RIGHT_BOXES["shell_full_reference"]),
        [rect_relative_to(right_shell_inner, RIGHT_BOXES["shell_full_reference"])],
        panel_fill_tile,
    )
    right_shell_clean_path = RIGHT_DIR / "shell_clean.png"
    save(right_shell_clean, right_shell_clean_path)
    add_manifest_entry(
        manifest,
        path=right_shell_clean_path,
        group="RightPanel",
        kind="clean_shell",
        source_box=RIGHT_BOXES["shell_full_reference"],
        clean_boxes=[right_shell_inner],
        recommended_margin_px=22,
        notes="Outer leaderboard shell with live-content area reset to dark fill.",
    )

    left_dropdown_shell = fill_rectangles(
        crop(reference, RIGHT_BOXES["dropdown_shell_left"]),
        [rect_relative_to((1274, 407, 1373, 436), RIGHT_BOXES["dropdown_shell_left"])],
        panel_fill_tile,
    )
    left_dropdown_shell_path = RIGHT_DIR / "dropdown_shell_left_clean.png"
    save(left_dropdown_shell, left_dropdown_shell_path)
    add_manifest_entry(
        manifest,
        path=left_dropdown_shell_path,
        group="RightPanel",
        kind="clean_component",
        source_box=RIGHT_BOXES["dropdown_shell_left"],
        clean_boxes=[(1274, 407, 1373, 436)],
        notes="Left leaderboard dropdown shell with label area removed and chevron preserved.",
    )

    right_dropdown_shell = fill_rectangles(
        crop(reference, RIGHT_BOXES["dropdown_shell_right"]),
        [rect_relative_to((1460, 407, 1548, 436), RIGHT_BOXES["dropdown_shell_right"])],
        panel_fill_tile,
    )
    right_dropdown_shell_path = RIGHT_DIR / "dropdown_shell_right_clean.png"
    save(right_dropdown_shell, right_dropdown_shell_path)
    add_manifest_entry(
        manifest,
        path=right_dropdown_shell_path,
        group="RightPanel",
        kind="clean_component",
        source_box=RIGHT_BOXES["dropdown_shell_right"],
        clean_boxes=[(1460, 407, 1548, 436)],
        notes="Right leaderboard dropdown shell with label area removed and chevron preserved.",
    )

    leaderboard_avatar_frame = transparent_center(
        crop(reference, RIGHT_BOXES["leaderboard_avatar_frame_source"]),
        rect_relative_to((1339, 554, 1380, 595), RIGHT_BOXES["leaderboard_avatar_frame_source"]),
    )
    leaderboard_avatar_frame_path = RIGHT_DIR / "leaderboard_avatar_frame.png"
    save(leaderboard_avatar_frame, leaderboard_avatar_frame_path)
    add_manifest_entry(
        manifest,
        path=leaderboard_avatar_frame_path,
        group="RightPanel",
        kind="frame",
        source_box=RIGHT_BOXES["leaderboard_avatar_frame_source"],
        clean_boxes=[(1339, 554, 1380, 595)],
        notes="Leaderboard avatar frame with transparent center for live avatars.",
    )

    fallback_avatar_boxes = {
        "leaderboard_avatar_fallback_01": (1346, 559, 1372, 585),
        "leaderboard_avatar_fallback_02": (1346, 618, 1372, 644),
        "leaderboard_avatar_fallback_03": (1346, 676, 1372, 702),
    }
    for name, box in fallback_avatar_boxes.items():
        path = RIGHT_DIR / f"{name}.png"
        save(crop(reference, box), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="RightPanel",
            kind="exact_crop",
            source_box=box,
            notes="Reference fallback leaderboard portrait used when live avatar data is unavailable.",
        )
        preview_paths.append((f"Right/{name}", path))

    for name in (
        "shell_full_reference",
        "filter_world_button",
        "filter_friends_button",
        "filter_crown_button",
        "tab_weekly_active",
        "tab_all_time_inactive",
        "dropdown_shell_left",
        "dropdown_shell_right",
        "toggle_score_selected",
        "toggle_speedrun_unselected",
    ):
        path = RIGHT_DIR / f"{name}.png"
        save(crop(reference, RIGHT_BOXES[name]), path)
        add_manifest_entry(
            manifest,
            path=path,
            group="RightPanel",
            kind="exact_crop",
            source_box=RIGHT_BOXES[name],
            notes="Exact reference crop for the leaderboard rebuild.",
        )
        preview_paths.append((f"Right/{name}", path))

    preview_paths.append(("Right/shell_clean", right_shell_clean_path))
    preview_paths.append(("Right/dropdown_left_clean", left_dropdown_shell_path))
    preview_paths.append(("Right/dropdown_right_clean", right_dropdown_shell_path))
    preview_paths.append(("Right/leaderboard_avatar_frame", leaderboard_avatar_frame_path))

    # The canonical full-screen comparison target lives in UI/screens/main_menu/reference.
    # Do not duplicate full-screen main-menu reference masters in SourceAssets.
    cta_stack_outer_frame_path = CENTER_DIR / "cta_stack_outer_frame.png"
    if cta_stack_outer_frame_path.exists():
        add_manifest_entry(
            manifest,
            path=cta_stack_outer_frame_path,
            group="Center",
            kind="runtime_frame",
            source_box=CENTER_BOXES["cta_stack_full"],
            notes="Runtime CTA stack frame generated from the native-aspect CTA family board.",
        )
    topbar_shell_clean = fill_rectangles(
        crop(reference, TOPBAR_BOXES["topbar_strip_full"]),
        [rect_relative_to(box, TOPBAR_BOXES["topbar_strip_full"]) for name, box in TOPBAR_BOXES.items() if name != "topbar_strip_full"],
        topbar_fill_tile,
    )
    save(topbar_shell_clean, TOPBAR_SHELL_CLEAN_PATH)
    add_manifest_entry(
        manifest,
        path=TOPBAR_SHELL_CLEAN_PATH,
        group="TopBar",
        kind="derived_shell",
        source_box=TOPBAR_BOXES["topbar_strip_full"],
        notes="Full-width top-bar strip with all baked buttons removed so the live top-bar controls are the only button layer.",
    )
    topbar_backdrop_clean = transparent_rectangles(
        crop(reference, TOPBAR_BOXES["topbar_strip_full"]),
        [rect_relative_to(box, TOPBAR_BOXES["topbar_strip_full"]) for name, box in TOPBAR_BOXES.items() if name != "topbar_strip_full"],
    )
    save(topbar_backdrop_clean, TOPBAR_BACKDROP_CLEAN_PATH)
    add_manifest_entry(
        manifest,
        path=TOPBAR_BACKDROP_CLEAN_PATH,
        group="TopBar",
        kind="foreground_backdrop",
        source_box=TOPBAR_BOXES["topbar_strip_full"],
        clean_boxes=[box for name, box in TOPBAR_BOXES.items() if name != "topbar_strip_full"],
        notes="Top-bar foreground backing with all button rectangles punched transparent so live button sprites own every control pixel.",
    )
    preview_paths.append(("TopBar/topbar_backdrop_clean", TOPBAR_BACKDROP_CLEAN_PATH))
    master_without_topbar_buttons = master_background.copy()
    master_without_topbar_buttons.alpha_composite(topbar_shell_clean, (0, 0))
    # Helper full-screen variants are intentionally not written. Regenerate screen
    # references through UI/SCREEN_WORKFLOW.md instead of preserving stale masters.

    center_stack_without_buttons = crop(reference, CENTER_BOXES["cta_stack_full"])
    center_button_rects = [
        rect_relative_to(CENTER_BOXES["cta_button_new_game"], CENTER_BOXES["cta_stack_full"]),
        rect_relative_to(CENTER_BOXES["cta_button_load_game"], CENTER_BOXES["cta_stack_full"]),
        rect_relative_to(CENTER_BOXES["cta_button_daily_challenge"], CENTER_BOXES["cta_stack_full"]),
    ]
    if CENTER_STACK_FRAME_SLICE_PATH.exists():
        stack_frame = Image.open(CENTER_STACK_FRAME_SLICE_PATH).convert("RGBA")
        sample_rect = (
            max(0, stack_frame.width // 3 - 16),
            max(0, stack_frame.height // 6 - 16),
            min(stack_frame.width, stack_frame.width // 3 + 16),
            min(stack_frame.height, stack_frame.height // 6 + 16),
        )
        center_fill_tile = stack_frame.crop(sample_rect)
    center_stack_without_buttons = fill_rectangles(center_stack_without_buttons, center_button_rects, center_fill_tile)
    master_without_buttons = master_without_topbar_buttons.copy()
    master_without_buttons.alpha_composite(center_stack_without_buttons, CENTER_BOXES["cta_stack_full"][:2])
    # Do not write no-buttons full-screen masters; they caused agents to use
    # helper composites as runtime or style authority.
    runtime_clean_master = soften_tagline_for_runtime_master(master_without_topbar_buttons)
    _ = runtime_clean_master
    if CONTENT_OWNERSHIP_PATH.exists():
        add_manifest_entry(
            manifest,
            path=CONTENT_OWNERSHIP_PATH,
            group="Metadata",
            kind="ownership_audit",
            source_box=(0, 0, 0, 0),
            notes="Code-first ownership audit covering live text, live data, and avatar wells that must remain runtime-owned.",
        )
    write_reference_layout_artifacts()
    build_overlay(reference)
    build_preview(preview_paths)

    manifest_payload = {
        "source_image": str(SOURCE_IMAGE),
        "canvas_size": [reference.width, reference.height],
        "output_root": str(OUTPUT_ROOT),
        "assets": manifest,
    }
    MANIFEST_PATH.write_text(json.dumps(manifest_payload, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    build_assets()
