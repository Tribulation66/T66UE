from __future__ import annotations

import argparse
import json
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
REFERENCE_ROOT = ROOT / "SourceAssets" / "UI" / "MainMenuReference"
CANONICAL_REFERENCE_PATH = ROOT / "UI" / "screens" / "main_menu" / "reference" / "canonical_reference_1920x1080.png"
ASSET_MANIFEST_PATH = REFERENCE_ROOT / "asset_manifest.json"
REFERENCE_LAYOUT_PATH = REFERENCE_ROOT / "reference_layout.json"
CONTENT_OWNERSHIP_PATH = REFERENCE_ROOT / "content_ownership.json"
MAIN_MENU_SOURCE_PATH = ROOT / "Source" / "T66" / "UI" / "Screens" / "T66MainMenuScreen.cpp"
TOP_BAR_SOURCE_PATH = ROOT / "Source" / "T66" / "UI" / "T66FrontendTopBarWidget.cpp"

EXPECTED_CANVAS = (1920, 1080)
STATE_SUFFIXES = ("hover", "pressed", "disabled", "active")
KNOWN_VALIDATION_MODES = {
    "manual-interior",
    "strict-no-foreground-ui",
    "strict-component-layer",
    "strict-layout-manual-text",
}

REQUIRED_FILES = (
    "asset_manifest.json",
    "reference_layout.json",
    "content_ownership.json",
    "asset_pack_preview.png",
    "scene_background_generated_source.png",
    "scene_background_1920x1080.png",
    "scene_background_purple_imagegen_1920x1080.png",
    "TopBar/topbar_strip_full.png",
    "TopBar/topbar_shell_clean.png",
    "TopBar/topbar_backdrop_clean.png",
    "TopBar/topbar_button_account.png",
    "TopBar/topbar_button_account_hover.png",
    "TopBar/topbar_button_account_pressed.png",
    "TopBar/topbar_button_account_active.png",
    "TopBar/topbar_button_nav.png",
    "TopBar/topbar_button_nav_hover.png",
    "TopBar/topbar_button_nav_pressed.png",
    "TopBar/currency_slot_blank.png",
    "Center/cta_stack_outer_frame.png",
    "Center/title_lockup_wordmark.png",
    "Center/cta_button_new_game_plate.png",
    "Center/cta_button_new_game_plate_hover.png",
    "Center/cta_button_new_game_plate_pressed.png",
    "Center/cta_button_new_game_plate_disabled.png",
    "Center/cta_button_new_game_wide_plate.png",
    "Center/cta_button_new_game_wide_plate_hover.png",
    "Center/cta_button_new_game_wide_plate_pressed.png",
    "Center/cta_button_new_game_wide_plate_disabled.png",
    "Center/cta_button_load_game_plate.png",
    "Center/cta_button_load_game_plate_hover.png",
    "Center/cta_button_load_game_plate_pressed.png",
    "Center/cta_button_load_game_plate_disabled.png",
    "Center/cta_button_load_game_wide_plate.png",
    "Center/cta_button_load_game_wide_plate_hover.png",
    "Center/cta_button_load_game_wide_plate_pressed.png",
    "Center/cta_button_load_game_wide_plate_disabled.png",
    "Center/cta_button_daily_challenge_plate.png",
    "Center/cta_button_daily_challenge_plate_hover.png",
    "Center/cta_button_daily_challenge_plate_pressed.png",
    "Center/cta_button_daily_challenge_plate_disabled.png",
    "Center/cta_button_daily_challenge_wide_plate.png",
    "Center/cta_button_daily_challenge_wide_plate_hover.png",
    "Center/cta_button_daily_challenge_wide_plate_pressed.png",
    "Center/cta_button_daily_challenge_wide_plate_disabled.png",
    "LeftPanel/shell_clean.png",
    "LeftPanel/profile_card_shell.png",
    "LeftPanel/friend_avatar_frame.png",
    "LeftPanel/party_slot_frame.png",
    "RightPanel/shell_clean.png",
    "RightPanel/leaderboard_avatar_frame.png",
)

REQUIRED_LAYOUT_KEYS = (
    "TopBar.topbar_strip_full",
    "TopBar.button_settings",
    "TopBar.button_chat",
    "TopBar.tab_account",
    "TopBar.badge_profile",
    "TopBar.tab_power_up",
    "TopBar.tab_achievements",
    "TopBar.tab_minigames",
    "TopBar.currency_slot",
    "TopBar.button_power",
    "Center.center_backdrop_full",
    "Center.title_lockup",
    "Center.subtitle_lockup",
    "Center.cta_stack_full",
    "Center.cta_button_new_game",
    "Center.cta_button_load_game",
    "Center.cta_button_daily_challenge",
    "CenterRuntime.cta_button_new_game_plate",
    "CenterRuntime.cta_button_load_game_plate",
    "CenterRuntime.cta_button_daily_challenge_plate",
    "Left.shell_full_reference",
    "Left.profile_card_reference",
    "Left.friend_avatar_frame_source",
    "Left.party_slot_source",
    "Right.shell_full_reference",
    "Right.leaderboard_avatar_frame_source",
    "MainMenu.full_canvas",
    "MainMenu.left_panel_assembly",
    "MainMenu.right_panel_assembly",
)

EXPECTED_ALPHA_FILES = (
    "Center/title_lockup_wordmark.png",
)

REQUIRED_OPEN_APERTURE_FILES = (
    "RightPanel/leaderboard_avatar_frame.png",
)


@dataclass
class ValidationResult:
    failures: list[str]
    warnings: list[str]

    def fail(self, message: str) -> None:
        self.failures.append(message)

    def warn(self, message: str) -> None:
        self.warnings.append(message)


def load_json(path: Path, result: ValidationResult) -> Any | None:
    if not path.exists():
        result.fail(f"Missing metadata file: {path}")
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        result.fail(f"Invalid JSON in {path}: {exc}")
        return None


def normalized_path(value: str) -> str:
    return value.replace("\\", "/")


def image_size(path: Path, result: ValidationResult) -> tuple[int, int] | None:
    try:
        with Image.open(path) as image:
            return image.size
    except OSError as exc:
        result.fail(f"Unreadable image: {path} ({exc})")
        return None


def alpha_extrema(path: Path, result: ValidationResult) -> tuple[int, int] | None:
    try:
        with Image.open(path) as image:
            rgba = image.convert("RGBA")
            return rgba.getchannel("A").getextrema()
    except OSError as exc:
        result.fail(f"Unreadable image: {path} ({exc})")
        return None


def center_alpha(path: Path, result: ValidationResult) -> int | None:
    try:
        with Image.open(path) as image:
            rgba = image.convert("RGBA")
            return rgba.getpixel((rgba.width // 2, rgba.height // 2))[3]
    except OSError as exc:
        result.fail(f"Unreadable image: {path} ({exc})")
        return None


def layout_rect(layout: dict[str, Any], key: str) -> dict[str, Any] | None:
    group_name, rect_name = key.split(".", 1)
    groups = layout.get("groups", {})
    group = groups.get(group_name, {})
    rect = group.get(rect_name)
    return rect if isinstance(rect, dict) else None


def rect_size_from_source_box(entry: dict[str, Any]) -> tuple[int, int] | None:
    box = entry.get("source_box")
    if not isinstance(box, list) or len(box) != 4:
        return None
    try:
        left, top, right, bottom = (int(value) for value in box)
    except (TypeError, ValueError):
        return None
    return (right - left, bottom - top)


def state_family(path: str) -> tuple[str, str] | None:
    directory, _, filename = path.rpartition("/")
    if not filename.endswith(".png"):
        return None
    stem = filename[:-4]
    for suffix in STATE_SUFFIXES:
        token = f"_{suffix}"
        if stem.endswith(token):
            base = stem[: -len(token)]
            return (f"{directory}/{base}".lstrip("/"), suffix)
    return None


def check_required_files(result: ValidationResult) -> None:
    for relative in REQUIRED_FILES:
        path = REFERENCE_ROOT / relative
        if not path.exists():
            result.fail(f"Missing required reference-pack file: {relative}")


def check_manifest(manifest: dict[str, Any], result: ValidationResult) -> dict[str, dict[str, Any]]:
    assets = manifest.get("assets")
    if not isinstance(assets, list):
        result.fail("asset_manifest.json must contain an assets array")
        return {}

    by_path: dict[str, dict[str, Any]] = {}
    for index, entry in enumerate(assets):
        if not isinstance(entry, dict):
            result.fail(f"asset_manifest.json assets[{index}] is not an object")
            continue
        path_value = entry.get("path")
        if not isinstance(path_value, str) or not path_value:
            result.fail(f"asset_manifest.json assets[{index}] is missing path")
            continue
        relative = normalized_path(path_value)
        if relative in by_path:
            result.warn(f"Duplicate asset manifest path: {relative}")
        by_path[relative] = entry

        asset_path = REFERENCE_ROOT / relative
        if not asset_path.exists():
            result.fail(f"Manifest asset is missing on disk: {relative}")
            continue

        if asset_path.suffix.lower() != ".png":
            continue

        size = image_size(asset_path, result)
        if size is None:
            continue

        expected_size = rect_size_from_source_box(entry)
        kind = entry.get("kind")
        if expected_size and kind not in {"alias_copy", "derived_master"}:
            if expected_size[0] <= 0 or expected_size[1] <= 0:
                result.fail(f"Manifest asset has non-positive source_box: {relative}")
            elif size != expected_size:
                result.fail(
                    f"Manifest asset size does not match source_box: {relative} "
                    f"has {size[0]}x{size[1]}, expected {expected_size[0]}x{expected_size[1]}"
                )

        extrema = alpha_extrema(asset_path, result)
        if extrema is not None and extrema[1] == 0:
            result.fail(f"Image is fully transparent: {relative}")

    return by_path


def check_layout(layout: dict[str, Any], result: ValidationResult) -> None:
    canvas = layout.get("canvas", {})
    width = canvas.get("width")
    height = canvas.get("height")
    if (width, height) != EXPECTED_CANVAS:
        result.fail(
            f"reference_layout.json canvas is {width}x{height}; expected "
            f"{EXPECTED_CANVAS[0]}x{EXPECTED_CANVAS[1]}"
        )
    if "source_canvas" in layout:
        result.fail("reference_layout.json must not contain source_canvas; the pack is canonical 1920x1080 only")

    for key in REQUIRED_LAYOUT_KEYS:
        if layout_rect(layout, key) is None:
            result.fail(f"Missing required layout key: {key}")

    groups = layout.get("groups", {})
    if not isinstance(groups, dict):
        result.fail("reference_layout.json groups must be an object")
        return

    for group_name, group in groups.items():
        if not isinstance(group, dict):
            result.fail(f"Layout group is not an object: {group_name}")
            continue
        for rect_name, rect in group.items():
            if not isinstance(rect, dict):
                result.fail(f"Layout rect is not an object: {group_name}.{rect_name}")
                continue
            required_fields = ("x", "y", "width", "height")
            if any(field not in rect for field in required_fields):
                result.fail(f"Layout rect missing x/y/width/height: {group_name}.{rect_name}")
                continue
            try:
                x = int(rect["x"])
                y = int(rect["y"])
                rect_width = int(rect["width"])
                rect_height = int(rect["height"])
            except (TypeError, ValueError):
                result.fail(f"Layout rect contains non-integer values: {group_name}.{rect_name}")
                continue
            if rect_width <= 0 or rect_height <= 0:
                result.fail(f"Layout rect has non-positive size: {group_name}.{rect_name}")
            if x < 0 or y < 0 or x + rect_width > int(width) or y + rect_height > int(height):
                result.fail(f"Layout rect is outside canvas: {group_name}.{rect_name}")


def check_source_and_master_sizes(manifest: dict[str, Any], result: ValidationResult) -> None:
    if "source_size" in manifest or "runtime_canvas_size" in manifest:
        result.fail("asset_manifest.json must use canvas_size only; source/runtime canvas split is no longer allowed")
    canvas_size = manifest.get("canvas_size")
    if canvas_size != list(EXPECTED_CANVAS):
        result.fail(f"asset_manifest.json canvas_size is {canvas_size}; expected {list(EXPECTED_CANVAS)}")

    if not CANONICAL_REFERENCE_PATH.exists():
        result.fail(f"Missing canonical main-menu style anchor: {CANONICAL_REFERENCE_PATH}")
    else:
        with Image.open(CANONICAL_REFERENCE_PATH) as image:
            if image.size != EXPECTED_CANVAS:
                result.fail(f"{CANONICAL_REFERENCE_PATH} has size {image.size}; expected {EXPECTED_CANVAS}")

    for relative in (
        "scene_background_generated_source.png",
        "scene_background_1920x1080.png",
        "scene_background_purple_imagegen_1920x1080.png",
    ):
        path = REFERENCE_ROOT / relative
        if not path.exists():
            continue
        size = image_size(path, result)
        if size != EXPECTED_CANVAS:
            result.fail(f"Master reference has unexpected size: {relative} is {size}, expected {EXPECTED_CANVAS}")


def check_alpha_expectations(result: ValidationResult) -> None:
    for relative in EXPECTED_ALPHA_FILES:
        path = REFERENCE_ROOT / relative
        if not path.exists():
            result.fail(f"Missing expected transparent asset: {relative}")
            continue
        extrema = alpha_extrema(path, result)
        if extrema is None:
            continue
        if extrema[0] >= 255:
            result.fail(f"Expected transparency but image is fully opaque: {relative}")
        if extrema[1] == 0:
            result.fail(f"Expected visible pixels but image is fully transparent: {relative}")

    for relative in REQUIRED_OPEN_APERTURE_FILES:
        path = REFERENCE_ROOT / relative
        if not path.exists():
            result.fail(f"Missing expected open-aperture socket frame: {relative}")
            continue
        extrema = alpha_extrema(path, result)
        if extrema is None:
            continue
        if extrema[0] >= 255:
            result.fail(f"Expected an alpha aperture but image is fully opaque: {relative}")
        alpha = center_alpha(path, result)
        if alpha is not None and alpha > 32:
            result.fail(f"Expected transparent center aperture in socket frame: {relative} center alpha is {alpha}")


def check_state_sizes(result: ValidationResult) -> None:
    png_paths = sorted(path for path in REFERENCE_ROOT.rglob("*.png") if "SheetSlices" not in path.parts)
    sizes_by_relative: dict[str, tuple[int, int]] = {}
    for path in png_paths:
        relative = normalized_path(path.relative_to(REFERENCE_ROOT).as_posix())
        size = image_size(path, result)
        if size is not None:
            sizes_by_relative[relative] = size

    families: dict[str, dict[str, tuple[str, tuple[int, int]]]] = {}
    for relative, size in sizes_by_relative.items():
        parsed = state_family(relative)
        if parsed is None:
            continue
        base, state = parsed
        families.setdefault(base, {})[state] = (relative, size)
        base_path = f"{base}.png"
        if base_path in sizes_by_relative:
            families[base]["normal"] = (base_path, sizes_by_relative[base_path])

    for base, states in sorted(families.items()):
        if len(states) < 2 or "normal" not in states:
            continue
        normal_path, normal_size = states["normal"]
        for state, (relative, size) in sorted(states.items()):
            if state == "normal":
                continue
            if size != normal_size:
                result.fail(
                    f"State size mismatch for {base}: {relative} is {size[0]}x{size[1]}, "
                    f"but {normal_path} is {normal_size[0]}x{normal_size[1]}"
                )


def check_ownership(ownership: dict[str, Any], layout: dict[str, Any], result: ValidationResult) -> None:
    if ownership.get("screen") != "MainMenu":
        result.fail(f"content_ownership.json screen is {ownership.get('screen')!r}; expected 'MainMenu'")

    regions = ownership.get("regions")
    if not isinstance(regions, list) or not regions:
        result.fail("content_ownership.json must contain at least one region")
        return

    seen_ids: set[str] = set()
    for index, region in enumerate(regions):
        if not isinstance(region, dict):
            result.fail(f"content_ownership.json regions[{index}] is not an object")
            continue
        region_id = region.get("id")
        if not isinstance(region_id, str) or not region_id:
            result.fail(f"content_ownership.json regions[{index}] is missing id")
        elif region_id in seen_ids:
            result.fail(f"Duplicate ownership region id: {region_id}")
        else:
            seen_ids.add(region_id)

        key = region.get("reference_layout_key")
        if not isinstance(key, str) or "." not in key:
            result.fail(f"Ownership region {region_id or index} has invalid reference_layout_key")
        elif layout_rect(layout, key) is None:
            result.fail(f"Ownership region {region_id or index} references missing layout key: {key}")

        for field in ("content_type", "render_contract", "validation_mode", "notes"):
            if not region.get(field):
                result.fail(f"Ownership region {region_id or index} is missing {field}")

        validation_mode = region.get("validation_mode")
        if validation_mode not in KNOWN_VALIDATION_MODES:
            result.warn(
                f"Ownership region {region_id or index} uses validation_mode "
                f"{validation_mode!r}; confirm automated review handles it."
            )


def check_manifest_coverage(by_path: dict[str, dict[str, Any]], result: ValidationResult) -> None:
    for relative in REQUIRED_FILES:
        if not relative.endswith(".png"):
            continue
        if relative not in by_path and not relative.startswith("debug/") and relative != "asset_pack_preview.png":
            result.warn(f"Required image is not listed in asset_manifest.json: {relative}")


def check_runtime_layer_contract(result: ValidationResult) -> None:
    if not MAIN_MENU_SOURCE_PATH.exists():
        result.warn(f"Cannot inspect runtime layer contract; missing {MAIN_MENU_SOURCE_PATH}")
        return

    source_text = MAIN_MENU_SOURCE_PATH.read_text(encoding="utf-8", errors="replace")
    if "SourceAssets/UI/MainMenuReference/scene_background_purple_imagegen_1920x1080.png" not in source_text:
        result.fail("Main menu runtime does not load the canonical 1920x1080 UI-free scene background plate.")
    if "SourceAssets/UI/MainMenuReference/Center/title_lockup_wordmark.png" not in source_text:
        result.fail("Main menu runtime does not load the separate baked title wordmark layer.")
    if "SourceAssets/UI/MainMenuReference/reference_main_menu_master_runtime_clean.png" in source_text:
        result.fail("Main menu runtime still loads the old full-screen runtime-clean reference master.")

    if not TOP_BAR_SOURCE_PATH.exists():
        result.warn(f"Cannot inspect top-bar layer contract; missing {TOP_BAR_SOURCE_PATH}")
        return
    top_bar_source = TOP_BAR_SOURCE_PATH.read_text(encoding="utf-8", errors="replace")
    if "SourceAssets/UI/MainMenuReference/TopBar/topbar_backdrop_clean.png" not in top_bar_source:
        result.fail("Top bar runtime does not load the punched clean foreground backdrop.")


def run_validation() -> ValidationResult:
    result = ValidationResult(failures=[], warnings=[])
    manifest = load_json(ASSET_MANIFEST_PATH, result)
    layout = load_json(REFERENCE_LAYOUT_PATH, result)
    ownership = load_json(CONTENT_OWNERSHIP_PATH, result)

    check_required_files(result)

    manifest_paths: dict[str, dict[str, Any]] = {}
    if isinstance(manifest, dict):
        manifest_paths = check_manifest(manifest, result)
        check_source_and_master_sizes(manifest, result)

    if isinstance(layout, dict):
        check_layout(layout, result)

    check_alpha_expectations(result)
    check_state_sizes(result)

    if isinstance(ownership, dict) and isinstance(layout, dict):
        check_ownership(ownership, layout, result)

    check_manifest_coverage(manifest_paths, result)
    check_runtime_layer_contract(result)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate the generated Main Menu reference pack.")
    parser.add_argument(
        "--warnings-as-errors",
        action="store_true",
        help="Return a failing exit code when warnings are present.",
    )
    args = parser.parse_args()

    result = run_validation()

    for failure in result.failures:
        print(f"FAIL: {failure}")
    for warning in result.warnings:
        print(f"WARN: {warning}")

    print(
        f"MainMenu reference pack validation: "
        f"{len(result.failures)} failure(s), {len(result.warnings)} warning(s)"
    )

    if result.failures or (args.warnings_as_errors and result.warnings):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
