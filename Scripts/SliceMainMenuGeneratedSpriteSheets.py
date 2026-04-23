from __future__ import annotations

import argparse
import json
from collections import deque
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = ROOT / "SourceAssets" / "UI" / "MainMenuReference"
SPRITESHEET_ROOT = SOURCE_ROOT / "SpriteSheets"
OUTPUT_ROOT = SOURCE_ROOT / "SheetSlices"
MANIFEST_PATH = OUTPUT_ROOT / "slice_manifest.json"
CTA_BOARD_V3_PATH = SPRITESHEET_ROOT / "mainmenu_cta_board_gen_v3.png"


def _topbar(path: str) -> Path:
    return OUTPUT_ROOT / "TopBar" / path


def _center(path: str) -> Path:
    return OUTPUT_ROOT / "Center" / path


def _center_reference(path: str) -> Path:
    return SOURCE_ROOT / "Center" / path


def _board_box(column: tuple[int, int], row: tuple[int, int]) -> tuple[int, int, int, int]:
    return (column[0], row[0], column[1], row[1])


SLICE_DEFS: dict[str, list[dict[str, object]]] = {
    "mainmenu_topbar_sheet_gen_v1.png": [
        {"name": "settings_normal", "box": (33, 118, 141, 230), "out": _topbar("settings_normal.png")},
        {"name": "settings_hover", "box": (170, 117, 283, 230), "out": _topbar("settings_hover.png")},
        {"name": "settings_pressed", "box": (311, 118, 421, 230), "out": _topbar("settings_pressed.png")},
        {"name": "settings_disabled", "box": (446, 118, 555, 230), "out": _topbar("settings_disabled.png")},
        {"name": "chat_normal", "box": (33, 247, 141, 358), "out": _topbar("chat_normal.png")},
        {"name": "chat_hover", "box": (169, 246, 282, 360), "out": _topbar("chat_hover.png")},
        {"name": "chat_pressed", "box": (312, 248, 420, 360), "out": _topbar("chat_pressed.png")},
        {"name": "chat_disabled", "box": (446, 247, 555, 359), "out": _topbar("chat_disabled.png")},
        {"name": "nav_normal", "box": (602, 122, 852, 237), "out": _topbar("nav_normal.png")},
        {"name": "nav_hover", "box": (875, 121, 1211, 238), "out": _topbar("nav_hover.png")},
        {"name": "nav_pressed", "box": (1233, 124, 1514, 239), "out": _topbar("nav_pressed.png")},
        {"name": "nav_disabled", "box": (1539, 125, 1794, 241), "out": _topbar("nav_disabled.png")},
        {"name": "portrait_normal", "box": (35, 471, 152, 615), "out": _topbar("portrait_normal.png")},
        {"name": "portrait_hover", "box": (220, 471, 339, 615), "out": _topbar("portrait_hover.png")},
        {"name": "portrait_pressed", "box": (410, 471, 529, 615), "out": _topbar("portrait_pressed.png")},
        {"name": "portrait_disabled", "box": (598, 471, 715, 615), "out": _topbar("portrait_disabled.png")},
        {"name": "coupon_normal", "box": (895, 476, 1064, 586), "out": _topbar("coupon_normal.png")},
        {"name": "coupon_hover", "box": (1109, 476, 1284, 586), "out": _topbar("coupon_hover.png")},
        {"name": "coupon_pressed", "box": (1338, 477, 1524, 587), "out": _topbar("coupon_pressed.png")},
        {"name": "coupon_disabled", "box": (1575, 478, 1780, 588), "out": _topbar("coupon_disabled.png")},
        {"name": "power_normal", "box": (33, 719, 132, 819), "out": _topbar("power_normal.png")},
        {"name": "power_hover", "box": (168, 719, 268, 819), "out": _topbar("power_hover.png")},
        {"name": "power_pressed", "box": (298, 719, 397, 818), "out": _topbar("power_pressed.png")},
        {"name": "power_disabled", "box": (425, 719, 523, 819), "out": _topbar("power_disabled.png")},
        {"name": "leaf_cluster_small_01", "box": (601, 722, 744, 791), "out": _topbar("leaf_cluster_small_01.png")},
        {"name": "leaf_cluster_small_02", "box": (758, 723, 865, 794), "out": _topbar("leaf_cluster_small_02.png")},
        {"name": "leaf_cluster_small_03", "box": (879, 724, 931, 793), "out": _topbar("leaf_cluster_small_03.png")},
        {"name": "separator_short", "box": (1214, 745, 1328, 782), "out": _topbar("separator_short.png")},
        {"name": "separator_medium", "box": (1350, 745, 1494, 782), "out": _topbar("separator_medium.png")},
        {"name": "separator_diamond", "box": (1517, 744, 1791, 783), "out": _topbar("separator_diamond.png")},
    ],
    "mainmenu_cta_sheet_gen_v1.png": [
        {"name": "stack_frame", "box": (33, 154, 421, 479), "out": _center("stack_frame.png")},
        {"name": "green_normal", "box": (472, 145, 762, 289), "out": _center("green_normal.png")},
        {"name": "green_hover", "box": (783, 145, 1072, 289), "out": _center("green_hover.png")},
        {"name": "green_pressed", "box": (1093, 148, 1380, 293), "out": _center("green_pressed.png")},
        {"name": "green_disabled", "box": (1404, 146, 1688, 289), "out": _center("green_disabled.png")},
        {"name": "blue_normal", "box": (471, 414, 760, 559), "out": _center("blue_normal.png")},
        {"name": "blue_hover", "box": (782, 414, 1072, 559), "out": _center("blue_hover.png")},
        {"name": "blue_pressed", "box": (1093, 417, 1380, 562), "out": _center("blue_pressed.png")},
        {"name": "blue_disabled", "box": (1405, 417, 1688, 560), "out": _center("blue_disabled.png")},
        {"name": "purple_normal", "box": (471, 684, 760, 829), "out": _center("purple_normal.png")},
        {"name": "purple_hover", "box": (782, 684, 1072, 829), "out": _center("purple_hover.png")},
        {"name": "purple_pressed", "box": (1093, 687, 1380, 832), "out": _center("purple_pressed.png")},
        {"name": "purple_disabled", "box": (1404, 686, 1688, 829), "out": _center("purple_disabled.png")},
    ],
}

CTA_BOARD_ROWS: dict[str, tuple[int, int]] = {
    "green": (82, 229),
    "blue": (327, 471),
    "purple": (556, 699),
}

CTA_BOARD_COLS: dict[str, tuple[int, int]] = {
    "normal": (556, 873),
    "hover": (870, 1181),
    "pressed": (1188, 1476),
    "disabled": (1484, 1745),
}

CTA_BOARD_RUNTIME_DEFS: list[dict[str, object]] = [
    {
        "name": "cta_button_new_game_plate",
        "box": _board_box(CTA_BOARD_COLS["normal"], CTA_BOARD_ROWS["green"]),
        "size": (423, 86),
        "out": _center_reference("cta_button_new_game_plate.png"),
    },
    {
        "name": "cta_button_new_game_plate_hover",
        "box": _board_box(CTA_BOARD_COLS["hover"], CTA_BOARD_ROWS["green"]),
        "size": (423, 86),
        "out": _center_reference("cta_button_new_game_plate_hover.png"),
    },
    {
        "name": "cta_button_new_game_plate_pressed",
        "box": _board_box(CTA_BOARD_COLS["pressed"], CTA_BOARD_ROWS["green"]),
        "size": (423, 86),
        "out": _center_reference("cta_button_new_game_plate_pressed.png"),
    },
    {
        "name": "cta_button_new_game_plate_disabled",
        "box": _board_box(CTA_BOARD_COLS["disabled"], CTA_BOARD_ROWS["green"]),
        "size": (423, 86),
        "out": _center_reference("cta_button_new_game_plate_disabled.png"),
    },
    {
        "name": "cta_button_load_game_plate",
        "box": _board_box(CTA_BOARD_COLS["normal"], CTA_BOARD_ROWS["blue"]),
        "size": (420, 82),
        "out": _center_reference("cta_button_load_game_plate.png"),
    },
    {
        "name": "cta_button_load_game_plate_hover",
        "box": _board_box(CTA_BOARD_COLS["hover"], CTA_BOARD_ROWS["blue"]),
        "size": (420, 82),
        "out": _center_reference("cta_button_load_game_plate_hover.png"),
    },
    {
        "name": "cta_button_load_game_plate_pressed",
        "box": _board_box(CTA_BOARD_COLS["pressed"], CTA_BOARD_ROWS["blue"]),
        "size": (420, 82),
        "out": _center_reference("cta_button_load_game_plate_pressed.png"),
    },
    {
        "name": "cta_button_load_game_plate_disabled",
        "box": _board_box(CTA_BOARD_COLS["disabled"], CTA_BOARD_ROWS["blue"]),
        "size": (420, 82),
        "out": _center_reference("cta_button_load_game_plate_disabled.png"),
    },
    {
        "name": "cta_button_daily_challenge_plate",
        "box": _board_box(CTA_BOARD_COLS["normal"], CTA_BOARD_ROWS["purple"]),
        "size": (419, 79),
        "out": _center_reference("cta_button_daily_challenge_plate.png"),
    },
    {
        "name": "cta_button_daily_challenge_plate_hover",
        "box": _board_box(CTA_BOARD_COLS["hover"], CTA_BOARD_ROWS["purple"]),
        "size": (419, 79),
        "out": _center_reference("cta_button_daily_challenge_plate_hover.png"),
    },
    {
        "name": "cta_button_daily_challenge_plate_pressed",
        "box": _board_box(CTA_BOARD_COLS["pressed"], CTA_BOARD_ROWS["purple"]),
        "size": (419, 79),
        "out": _center_reference("cta_button_daily_challenge_plate_pressed.png"),
    },
    {
        "name": "cta_button_daily_challenge_plate_disabled",
        "box": _board_box(CTA_BOARD_COLS["disabled"], CTA_BOARD_ROWS["purple"]),
        "size": (419, 79),
        "out": _center_reference("cta_button_daily_challenge_plate_disabled.png"),
    },
    {
        "name": "cta_stack_outer_frame",
        "box": (20, 60, 410, 505),
        "size": (486, 271),
        "out": _center_reference("cta_stack_outer_frame.png"),
        "threshold": 16,
    },
]


def flood_clear_background(image: Image.Image, threshold: int = 16) -> Image.Image:
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    background = pixels[0, 0][:3]

    def is_background(x: int, y: int) -> bool:
        pixel = pixels[x, y]
        return max(abs(pixel[i] - background[i]) for i in range(3)) <= threshold

    visited = [[False for _ in range(width)] for _ in range(height)]
    queue: deque[tuple[int, int]] = deque()

    for x in range(width):
        queue.append((x, 0))
        queue.append((x, height - 1))
    for y in range(height):
        queue.append((0, y))
        queue.append((width - 1, y))

    while queue:
        x, y = queue.popleft()
        if x < 0 or y < 0 or x >= width or y >= height or visited[y][x]:
            continue
        visited[y][x] = True
        if not is_background(x, y):
            continue
        pixels[x, y] = (0, 0, 0, 0)
        queue.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))

    return rgba


def save_slice(sheet_path: Path, definition: dict[str, object]) -> dict[str, object]:
    image = Image.open(sheet_path)
    box = tuple(int(v) for v in definition["box"])  # type: ignore[index]
    crop = image.crop(box)
    crop = flood_clear_background(crop)
    output_path = Path(definition["out"])  # type: ignore[arg-type]
    output_path.parent.mkdir(parents=True, exist_ok=True)
    crop.save(output_path)
    return {
        "source_sheet": sheet_path.name,
        "name": definition["name"],
        "box": list(box),
        "output": output_path.relative_to(SOURCE_ROOT).as_posix(),
        "size": [crop.width, crop.height],
    }


def extract_cta_board_v3() -> list[dict[str, object]]:
    if not CTA_BOARD_V3_PATH.exists():
        raise FileNotFoundError(f"Missing CTA board: {CTA_BOARD_V3_PATH}")

    image = Image.open(CTA_BOARD_V3_PATH)
    manifest: list[dict[str, object]] = []

    for definition in CTA_BOARD_RUNTIME_DEFS:
        box = tuple(int(v) for v in definition["box"])  # type: ignore[index]
        crop = image.crop(box)
        threshold = int(definition.get("threshold", 18))
        crop = flood_clear_background(crop, threshold=threshold)
        bounds = crop.getbbox()
        if bounds is not None:
            crop = crop.crop(bounds)
        size = tuple(int(v) for v in definition["size"])  # type: ignore[index]
        crop = crop.resize(size, Image.Resampling.LANCZOS)
        output_path = Path(definition["out"])  # type: ignore[arg-type]
        output_path.parent.mkdir(parents=True, exist_ok=True)
        crop.save(output_path)
        manifest.append(
            {
                "source_sheet": CTA_BOARD_V3_PATH.name,
                "name": definition["name"],
                "box": list(box),
                "output": output_path.relative_to(SOURCE_ROOT).as_posix(),
                "size": [crop.width, crop.height],
            }
        )

    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description="Slice generated main menu sprite sheets.")
    parser.add_argument(
        "--cta-board-v3-only",
        action="store_true",
        help="Extract the v3 CTA board into the Center reference pack without touching SheetSlices outputs.",
    )
    args = parser.parse_args()

    if args.cta_board_v3_only:
        manifest = extract_cta_board_v3()
        print(f"Wrote {len(manifest)} CTA board assets to {SOURCE_ROOT / 'Center'}")
        return

    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    manifest: list[dict[str, object]] = []

    for sheet_name, definitions in SLICE_DEFS.items():
        sheet_path = SPRITESHEET_ROOT / sheet_name
        if not sheet_path.exists():
            raise FileNotFoundError(f"Missing spritesheet: {sheet_path}")
        for definition in definitions:
            manifest.append(save_slice(sheet_path, definition))

    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Wrote {len(manifest)} slices to {OUTPUT_ROOT}")


if __name__ == "__main__":
    main()
