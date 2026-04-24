from __future__ import annotations

import json
import math
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageFont, ImageOps


ROOT = Path(__file__).resolve().parents[1]
SOURCE_REFERENCE = ROOT / "UI" / "screens" / "main_menu" / "reference" / "canonical_reference_1920x1080.png"
UI_MAIN_MENU_ROOT = ROOT / "UI" / "screens" / "main_menu"
UI_MAIN_MENU_REVIEW_DIR = UI_MAIN_MENU_ROOT / "review"
OUTPUT_ROOT = ROOT / "SourceAssets" / "UI" / "MainMenuReference"
TOPBAR_DIR = OUTPUT_ROOT / "TopBar"
CENTER_DIR = OUTPUT_ROOT / "Center"
LEFT_DIR = OUTPUT_ROOT / "LeftPanel"
RIGHT_DIR = OUTPUT_ROOT / "RightPanel"
TILES_DIR = OUTPUT_ROOT / "Tiles"
SPRITESHEET_DIR = OUTPUT_ROOT / "SpriteSheets"
SHEET_SLICES_DIR = OUTPUT_ROOT / "SheetSlices"
DEBUG_DIR = OUTPUT_ROOT / "debug"
LAYOUT_HEADER = ROOT / "Source" / "T66" / "UI" / "Style" / "T66MainMenuReferenceLayout.generated.h"
GENERATED_SOURCE_DIR = ROOT / "SourceAssets" / "UI" / "GeneratedSources" / "MainMenu"
SCENE_IMAGEGEN_SOURCE = GENERATED_SOURCE_DIR / "mainmenu_scene_background_imagegen_v2.png"
TOPBAR_STRIP_IMAGEGEN = GENERATED_SOURCE_DIR / "mainmenu_topbar_strip_imagegen_v1.png"
TOPBAR_IMAGEGEN_BOARD = GENERATED_SOURCE_DIR / "mainmenu_topbar_family_imagegen_v4.png"
CTA_BUTTONS_IMAGEGEN_ATLAS = GENERATED_SOURCE_DIR / "mainmenu_cta_buttons_imagegen_v5.png"
CTA_STACK_FRAME_IMAGEGEN = GENERATED_SOURCE_DIR / "mainmenu_cta_stack_frame_imagegen_v3.png"
PANELS_IMAGEGEN_BOARD = GENERATED_SOURCE_DIR / "mainmenu_panels_family_imagegen_v1.png"
LEADERBOARD_AVATAR_FRAME_IMAGEGEN_ALPHA = GENERATED_SOURCE_DIR / "leaderboard_avatar_socket_frame_imagegen_20260424_alpha.png"

CANVAS = (1920, 1080)
Box = tuple[int, int, int, int]

TOPBAR_IMAGEGEN_BOXES: dict[str, Box] = {
    "backdrop": (20, 514, 1409, 625),
    "settings": (31, 291, 220, 499),
    "chat": (240, 291, 426, 499),
    "account": (450, 293, 862, 501),
    "profile": (890, 278, 1069, 508),
    "powerup": (1096, 293, 1495, 501),
    "achievements": (1522, 293, 1954, 501),
    "minigames": (30, 552, 516, 746),
    "currency": (557, 555, 1011, 739),
    "quit": (1054, 552, 1239, 746),
    "icon_powerup": (1137, 349, 1189, 429),
    "icon_achievements": (1563, 351, 1640, 428),
    "icon_minigames": (76, 606, 156, 689),
}

CTA_IMAGEGEN_BOXES: dict[str, Box] = {
    "green_normal": (45, 117, 550, 240),
    "green_hover": (579, 117, 1081, 240),
    "green_pressed": (1112, 117, 1600, 240),
    "green_disabled": (1631, 117, 2126, 240),
    "blue_normal": (45, 299, 550, 422),
    "blue_hover": (579, 299, 1081, 422),
    "blue_pressed": (1112, 299, 1600, 422),
    "blue_disabled": (1631, 299, 2126, 422),
    "purple_normal": (45, 484, 550, 607),
    "purple_hover": (579, 484, 1081, 607),
    "purple_pressed": (1112, 484, 1600, 607),
    "purple_disabled": (1631, 484, 2126, 607),
}

CTA_STACK_FRAME_IMAGEGEN_BOX: Box = (45, 66, 1455, 960)

PANEL_IMAGEGEN_BOXES: dict[str, Box] = {
    "left_shell": (29, 36, 433, 866),
    "profile_card": (462, 51, 1263, 346),
    "right_shell": (1290, 37, 1686, 515),
    "search_field": (463, 378, 938, 507),
    "friend_avatar_frame": (957, 371, 1097, 508),
    "party_slot_frame": (1115, 371, 1256, 508),
    "close_button": (462, 552, 565, 658),
    "friend_star_button": (577, 552, 680, 658),
    "friend_invite_button": (691, 552, 793, 658),
    "friend_offline_button": (803, 552, 905, 658),
    "filter_world_button": (457, 702, 585, 833),
    "filter_friends_button": (596, 702, 723, 833),
    "filter_crown_button": (734, 702, 862, 833),
    "tab_weekly_active": (928, 541, 1282, 628),
    "tab_all_time_inactive": (1296, 541, 1666, 630),
    "dropdown_shell_left": (925, 653, 1284, 730),
    "dropdown_shell_right": (1296, 653, 1666, 730),
    "toggle_score_selected": (1293, 764, 1576, 858),
    "toggle_speedrun_unselected": (924, 764, 1209, 858),
}

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
    "cta_stack_full": (742, 698, 1187, 1009),
    "cta_button_new_game": (770, 719, 1158, 819),
    "cta_button_load_game": (770, 819, 1158, 916),
    "cta_button_daily_challenge": (770, 916, 1158, 1008),
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
    "leaderboard_avatar_live_rect": (1541, 639, 1581, 679),
}

EXTRA_LAYOUT_BOXES: dict[str, dict[str, Box]] = {
    "MainMenu": {
        "full_canvas": (0, 0, 1920, 1080),
        "left_panel_assembly": (21, 349, 508, 1075),
        "right_panel_assembly": (1430, 290, 1888, 1077),
    }
}

LAYOUT_METRICS: dict[str, float] = {
    "canvas_width": 1920.0,
    "canvas_height": 1080.0,
    "topbar_reserved_height": 163.0,
    "topbar_surface_height": 145.0,
    "topbar_surface_offset_y": 0.0,
}

CREAM = (238, 229, 207, 255)
MUTED = (160, 151, 132, 255)
GOLD = (207, 170, 99, 255)
GOLD_DARK = (74, 54, 31, 255)
GREEN = (85, 129, 72, 255)
BLUE = (63, 86, 126, 255)
PURPLE = (101, 69, 128, 255)
PANEL = (13, 15, 18, 244)
PANEL_LIGHT = (33, 34, 37, 230)


try:
    RESAMPLING = Image.Resampling
except AttributeError:
    class _Resampling:
        LANCZOS = Image.LANCZOS
        BICUBIC = Image.BICUBIC

    RESAMPLING = _Resampling()


def size_of(box: Box) -> tuple[int, int]:
    return (box[2] - box[0], box[3] - box[1])


def rel_box(box: Box, parent: Box) -> Box:
    return (box[0] - parent[0], box[1] - parent[1], box[2] - parent[0], box[3] - parent[1])


def clamp_box(box: Box, size: tuple[int, int]) -> Box:
    width, height = size
    return (
        max(0, min(width, box[0])),
        max(0, min(height, box[1])),
        max(0, min(width, box[2])),
        max(0, min(height, box[3])),
    )


def rect_payload(box: Box) -> dict[str, int]:
    return {
        "x": box[0],
        "y": box[1],
        "width": box[2] - box[0],
        "height": box[3] - box[1],
    }


def snake_to_pascal(value: str) -> str:
    return "".join(part.capitalize() for part in value.split("_"))


def font(size: int, *, bold: bool = False) -> ImageFont.ImageFont:
    candidates = [
        ROOT / "RuntimeDependencies" / "T66" / "Fonts" / "Reaver-Bold.woff",
        ROOT / "RuntimeDependencies" / "T66" / "Fonts" / "radiance.ttf",
        Path("C:/Windows/Fonts/arialbd.ttf") if bold else Path("C:/Windows/Fonts/arial.ttf"),
        Path("C:/Windows/Fonts/arial.ttf"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size=size)
    return ImageFont.load_default()


def reset_output_tree() -> None:
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    for directory in (TOPBAR_DIR, CENTER_DIR, LEFT_DIR, RIGHT_DIR, TILES_DIR, SPRITESHEET_DIR, SHEET_SLICES_DIR, DEBUG_DIR):
        if directory.exists():
            shutil.rmtree(directory)
        directory.mkdir(parents=True, exist_ok=True)

    generated_root_files = [
        "asset_manifest.json",
        "asset_pack_preview.png",
        "center_stage.png",
        "left_panel_shell.png",
        "left_upper_strip.png",
        "reference_layout.json",
        "right_panel_full_shell.png",
        "right_panel_shell.png",
        "right_upper_strip.png",
        "scene_background_1920x1080.png",
        "scene_background_generated_source.png",
        "topbar_backdrop.png",
        "topbar_shell.png",
        "topbar_button_account_active.png",
        "topbar_button_account.png",
        "topbar_button_nav.png",
        "icon_powerup.png",
        "icon_achievements.png",
        "icon_minigames.png",
    ]
    for name in generated_root_files:
        path = OUTPUT_ROOT / name
        if path.exists():
            path.unlink()


def make_linear_gradient(size: tuple[int, int], top: tuple[int, int, int], bottom: tuple[int, int, int]) -> Image.Image:
    width, height = size
    image = Image.new("RGBA", size, (0, 0, 0, 255))
    pixels = image.load()
    for y in range(height):
        t = y / max(1, height - 1)
        color = tuple(int(top[i] * (1 - t) + bottom[i] * t) for i in range(3))
        for x in range(width):
            pixels[x, y] = (*color, 255)
    return image


def make_scene_background() -> Image.Image:
    image = make_linear_gradient(CANVAS, (13, 14, 18), (5, 6, 8))
    draw = ImageDraw.Draw(image, "RGBA")
    width, height = CANVAS

    for x in range(0, width, 96):
        shade = 12 + (x // 96) % 3 * 3
        draw.rectangle((x, 0, x + 48, height), fill=(shade, shade + 2, shade + 4, 42))

    moon = Image.new("RGBA", CANVAS, (0, 0, 0, 0))
    moon_draw = ImageDraw.Draw(moon, "RGBA")
    moon_draw.ellipse((780, 65, 1140, 425), fill=(205, 177, 113, 54))
    moon_draw.ellipse((850, 130, 1070, 350), fill=(191, 156, 89, 48))
    moon = moon.filter(ImageFilter.GaussianBlur(44))
    image.alpha_composite(moon)

    for x in (250, 395, 1520, 1665):
        draw.rectangle((x, 250, x + 52, height), fill=(22, 23, 25, 190))
        draw.rectangle((x - 18, 238, x + 70, 270), fill=(48, 43, 35, 190))
        draw.rectangle((x - 24, 805, x + 76, height), fill=(15, 16, 18, 210))
        draw.line((x + 6, 280, x + 6, 800), fill=(77, 67, 47, 70), width=2)
        draw.line((x + 43, 280, x + 43, 800), fill=(0, 0, 0, 90), width=3)

    # Center idol silhouette and plinth.
    draw.polygon([(930, 225), (1010, 312), (1055, 584), (960, 708), (850, 584), (890, 312)], fill=(20, 22, 22, 230))
    draw.ellipse((885, 185, 1015, 320), fill=(38, 41, 35, 230), outline=(121, 101, 57, 130), width=4)
    draw.polygon([(830, 620), (1090, 620), (1180, 780), (735, 780)], fill=(23, 22, 20, 245))
    draw.rectangle((690, 780, 1230, 875), fill=(17, 17, 18, 248))
    draw.line((730, 780, 1190, 780), fill=(159, 119, 55, 100), width=3)

    fire = Image.new("RGBA", CANVAS, (0, 0, 0, 0))
    fire_draw = ImageDraw.Draw(fire, "RGBA")
    for cx, cy in ((550, 780), (1375, 780), (710, 895), (1210, 895)):
        fire_draw.ellipse((cx - 95, cy - 145, cx + 95, cy + 95), fill=(201, 105, 38, 35))
        fire_draw.ellipse((cx - 52, cy - 88, cx + 52, cy + 55), fill=(226, 166, 71, 60))
    fire = fire.filter(ImageFilter.GaussianBlur(28))
    image.alpha_composite(fire)

    vignette = Image.new("RGBA", CANVAS, (0, 0, 0, 0))
    vdraw = ImageDraw.Draw(vignette, "RGBA")
    vdraw.rectangle((0, 0, width, height), fill=(0, 0, 0, 125))
    vdraw.rectangle((160, 80, width - 160, height - 80), fill=(0, 0, 0, 0))
    vignette = vignette.filter(ImageFilter.GaussianBlur(90))
    image.alpha_composite(vignette)
    return image


def add_noise(image: Image.Image, opacity: int = 18) -> Image.Image:
    noise = Image.effect_noise(image.size, 18).convert("L")
    alpha = Image.new("L", image.size, opacity)
    texture = Image.merge("RGBA", (noise, noise, noise, alpha))
    result = image.copy()
    result.alpha_composite(texture)
    return result


def bevel_panel(size: tuple[int, int], *, fill=PANEL, border=GOLD_DARK, inner=GOLD, alpha_center: bool = False) -> Image.Image:
    width, height = size
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(image, "RGBA")
    draw.rectangle((0, 0, width - 1, height - 1), fill=border)
    draw.rectangle((2, 2, width - 3, height - 3), fill=inner)
    draw.rectangle((5, 5, width - 6, height - 6), fill=fill)
    draw.rectangle((8, 8, width - 9, min(height - 9, 30)), fill=(255, 239, 196, 28))
    draw.line((7, height - 8, width - 8, height - 8), fill=(0, 0, 0, 115), width=3)
    draw.line((7, 7, width - 8, 7), fill=(255, 245, 214, 48), width=2)
    if alpha_center:
        draw.rectangle((12, 12, width - 13, height - 13), fill=(0, 0, 0, 0))
    return add_noise(image, 8)


def make_button(size: tuple[int, int], base: tuple[int, int, int, int], state: str = "normal") -> Image.Image:
    factor = {"normal": 1.0, "hover": 1.16, "pressed": 0.78, "disabled": 0.55}.get(state, 1.0)
    color = tuple(max(0, min(255, int(base[i] * factor))) for i in range(3)) + (base[3],)
    if state == "disabled":
        color = tuple(int((color[i] + 70) * 0.45) for i in range(3)) + (210,)
    image = bevel_panel(size, fill=color, border=(55, 39, 25, 255), inner=(218, 180, 104, 255))
    draw = ImageDraw.Draw(image, "RGBA")
    w, h = size
    draw.rectangle((10, 10, w - 11, h // 2), fill=(255, 248, 214, 26 if state != "pressed" else 8))
    if state == "hover":
        draw.rectangle((4, 4, w - 5, h - 5), outline=(255, 237, 166, 130), width=2)
    if state == "pressed":
        draw.rectangle((7, 7, w - 8, h - 8), fill=(0, 0, 0, 38))
    return image


def make_icon(kind: str, size: tuple[int, int], color=CREAM) -> Image.Image:
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(image, "RGBA")
    w, h = size
    if kind == "power":
        draw.arc((w * 0.22, h * 0.22, w * 0.78, h * 0.82), 40, 320, fill=color, width=max(3, w // 10))
        draw.line((w // 2, h * 0.12, w // 2, h * 0.48), fill=color, width=max(3, w // 10))
    elif kind == "chat":
        draw.rounded_rectangle((w * 0.16, h * 0.22, w * 0.82, h * 0.68), radius=max(3, w // 8), outline=color, width=max(2, w // 12))
        draw.polygon([(w * 0.35, h * 0.68), (w * 0.28, h * 0.88), (w * 0.55, h * 0.68)], fill=color)
    elif kind == "settings":
        cx, cy = w // 2, h // 2
        r_outer = min(w, h) * 0.34
        for i in range(8):
            a = math.tau * i / 8
            x = cx + math.cos(a) * r_outer
            y = cy + math.sin(a) * r_outer
            draw.rectangle((x - 2, y - 5, x + 2, y + 5), fill=color)
        draw.ellipse((cx - r_outer * 0.55, cy - r_outer * 0.55, cx + r_outer * 0.55, cy + r_outer * 0.55), outline=color, width=max(2, w // 14))
        draw.ellipse((cx - r_outer * 0.17, cy - r_outer * 0.17, cx + r_outer * 0.17, cy + r_outer * 0.17), fill=color)
    elif kind == "crown":
        draw.polygon([(w * 0.12, h * 0.72), (w * 0.18, h * 0.30), (w * 0.38, h * 0.56), (w * 0.50, h * 0.22), (w * 0.64, h * 0.56), (w * 0.84, h * 0.30), (w * 0.90, h * 0.72)], fill=color)
        draw.rectangle((w * 0.18, h * 0.70, w * 0.84, h * 0.82), fill=color)
    elif kind == "friends":
        draw.ellipse((w * 0.20, h * 0.18, w * 0.48, h * 0.48), fill=color)
        draw.ellipse((w * 0.52, h * 0.24, w * 0.76, h * 0.50), fill=color)
        draw.ellipse((w * 0.10, h * 0.50, w * 0.58, h * 0.92), fill=color)
        draw.ellipse((w * 0.44, h * 0.54, w * 0.90, h * 0.92), fill=color)
    elif kind == "world":
        draw.ellipse((w * 0.16, h * 0.16, w * 0.84, h * 0.84), outline=color, width=max(2, w // 12))
        draw.arc((w * 0.30, h * 0.16, w * 0.70, h * 0.84), 90, 270, fill=color, width=max(2, w // 18))
        draw.arc((w * 0.30, h * 0.16, w * 0.70, h * 0.84), 270, 90, fill=color, width=max(2, w // 18))
        draw.line((w * 0.16, h * 0.50, w * 0.84, h * 0.50), fill=color, width=max(2, w // 18))
    elif kind == "star":
        points = []
        cx, cy = w / 2, h / 2
        for i in range(10):
            r = min(w, h) * (0.42 if i % 2 == 0 else 0.18)
            a = -math.pi / 2 + math.tau * i / 10
            points.append((cx + math.cos(a) * r, cy + math.sin(a) * r))
        draw.polygon(points, fill=color)
    else:
        draw.ellipse((w * 0.18, h * 0.18, w * 0.82, h * 0.82), outline=color, width=max(2, w // 12))
    return image


def transparent_frame(size: tuple[int, int], inset: int) -> Image.Image:
    image = bevel_panel(size, fill=(22, 24, 26, 230), border=(48, 48, 50, 255), inner=(158, 149, 125, 255))
    alpha = image.getchannel("A")
    ImageDraw.Draw(alpha).rectangle((inset, inset, size[0] - inset - 1, size[1] - inset - 1), fill=0)
    image.putalpha(alpha)
    return image


def title_wordmark() -> Image.Image:
    size = size_of(CENTER_BOXES["title_lockup"])
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(image, "RGBA")
    title_font = font(82, bold=True)
    text = "CHADPOCALYPSE"
    bbox = draw.textbbox((0, 0), text, font=title_font, stroke_width=3)
    x = (size[0] - (bbox[2] - bbox[0])) // 2
    y = 12
    draw.text((x + 5, y + 7), text, font=title_font, fill=(0, 0, 0, 150), stroke_width=5, stroke_fill=(0, 0, 0, 170))
    draw.text((x, y), text, font=title_font, fill=(238, 218, 168, 255), stroke_width=3, stroke_fill=(74, 36, 26, 255))
    draw.line((220, 123, size[0] - 220, 123), fill=(142, 104, 52, 180), width=3)
    return image


def save_asset(
    image: Image.Image,
    path: Path,
    manifest: list[dict[str, object]],
    *,
    group: str,
    kind: str,
    notes: str,
    layout_key: str | None = None,
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)
    manifest.append(
        {
            "path": path.relative_to(OUTPUT_ROOT).as_posix(),
            "group": group,
            "kind": kind,
            "source_box": [0, 0, image.width, image.height],
            "layout_key": layout_key,
            "notes": notes,
        }
    )


def copy_asset(source: Path, dest: Path, manifest: list[dict[str, object]], *, group: str, kind: str, notes: str) -> None:
    image = Image.open(source).convert("RGBA")
    save_asset(image, dest, manifest, group=group, kind=kind, notes=notes)


def paste_at(canvas: Image.Image, image: Image.Image, box: Box) -> None:
    canvas.alpha_composite(image, box[:2])


def build_layout_files() -> None:
    groups = {
        "TopBar": TOPBAR_BOXES,
        "Center": CENTER_BOXES,
        "CenterRuntime": {
            "cta_button_new_game_plate": CENTER_BOXES["cta_button_new_game"],
            "cta_button_load_game_plate": CENTER_BOXES["cta_button_load_game"],
            "cta_button_daily_challenge_plate": CENTER_BOXES["cta_button_daily_challenge"],
        },
        "Left": LEFT_BOXES,
        "Right": RIGHT_BOXES,
        **EXTRA_LAYOUT_BOXES,
    }
    layout = {
        "canvas": {"width": CANVAS[0], "height": CANVAS[1]},
        "metrics": {k: v for k, v in LAYOUT_METRICS.items() if not k.startswith("canvas_")},
        "groups": {
            group_name: {name: rect_payload(box) for name, box in boxes.items()}
            for group_name, boxes in groups.items()
        },
    }
    (OUTPUT_ROOT / "reference_layout.json").write_text(json.dumps(layout, indent=2) + "\n", encoding="utf-8")

    lines = [
        "// Copyright Tribulation 66. All Rights Reserved.",
        "",
        "#pragma once",
        "",
        "namespace T66MainMenuReferenceLayout",
        "{",
        "\tinline constexpr float CanvasWidth = 1920.0f;",
        "\tinline constexpr float CanvasHeight = 1080.0f;",
        f"\tinline constexpr float TopBarReservedHeight = {LAYOUT_METRICS['topbar_reserved_height']:.1f}f;",
        f"\tinline constexpr float TopBarSurfaceHeight = {LAYOUT_METRICS['topbar_surface_height']:.1f}f;",
        f"\tinline constexpr float TopBarSurfaceOffsetY = {LAYOUT_METRICS['topbar_surface_offset_y']:.1f}f;",
        "",
    ]
    for group_name, boxes in groups.items():
        lines.append(f"\tnamespace {group_name}")
        lines.append("\t{")
        for name, box in boxes.items():
            rect = rect_payload(box)
            lines.append(
                f"\t\tinline constexpr FT66ReferenceRect {snake_to_pascal(name)}"
                f"{{{rect['x']:.1f}f, {rect['y']:.1f}f, {rect['width']:.1f}f, {rect['height']:.1f}f}};"
            )
        lines.append("\t}")
        lines.append("")
    lines.append("}")
    LAYOUT_HEADER.write_text("\n".join(lines), encoding="utf-8")


def build_pack() -> None:
    reset_output_tree()
    manifest: list[dict[str, object]] = []
    preview: list[tuple[str, Path]] = []

    background_source = Image.open(SCENE_IMAGEGEN_SOURCE).convert("RGBA")
    background = ImageOps.fit(background_source, CANVAS, method=RESAMPLING.LANCZOS, centering=(0.5, 0.5))
    save_asset(background, OUTPUT_ROOT / "scene_background_1920x1080.png", manifest, group="Scene", kind="ui_free_scene_plate", notes="Fresh imagegen UI-free 1920x1080 scene/background plate.")
    save_asset(background, OUTPUT_ROOT / "scene_background_generated_source.png", manifest, group="Scene", kind="ui_free_scene_plate_source", notes="Fresh imagegen scene plate source, fit to the runtime canvas without pixel cleanup.")

    topbar_strip_source = Image.open(TOPBAR_STRIP_IMAGEGEN).convert("RGBA")
    topbar_component_source = Image.open(TOPBAR_IMAGEGEN_BOARD).convert("RGBA")
    topbar_shell = crop_resize(topbar_strip_source, TOPBAR_IMAGEGEN_BOXES["backdrop"], size_of(TOPBAR_BOXES["topbar_strip_full"]))
    save_asset(topbar_shell, TOPBAR_DIR / "topbar_shell_clean.png", manifest, group="TopBar", kind="imagegen_slice", layout_key="TopBar.topbar_strip_full", notes="Fresh imagegen top-bar backing strip; buttons own their chrome.")
    save_asset(topbar_shell, TOPBAR_DIR / "topbar_backdrop_clean.png", manifest, group="TopBar", kind="imagegen_slice", layout_key="TopBar.topbar_strip_full", notes="Fresh imagegen top-bar backing strip; buttons own their chrome.")
    save_asset(topbar_shell, TOPBAR_DIR / "topbar_strip_full.png", manifest, group="TopBar", kind="imagegen_slice", layout_key="TopBar.topbar_strip_full", notes="Fresh imagegen top-bar strip.")
    copy_asset(TOPBAR_DIR / "topbar_shell_clean.png", OUTPUT_ROOT / "topbar_shell.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(TOPBAR_DIR / "topbar_backdrop_clean.png", OUTPUT_ROOT / "topbar_backdrop.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    utility_defs = {
        "button_settings": ("settings", TOPBAR_BOXES["button_settings"]),
        "button_chat": ("chat", TOPBAR_BOXES["button_chat"]),
        "button_power": ("quit", TOPBAR_BOXES["button_power"]),
        "badge_profile": ("profile", TOPBAR_BOXES["badge_profile"]),
        "currency_slot_blank": ("currency", TOPBAR_BOXES["currency_slot"]),
    }
    for name, (source_key, box) in utility_defs.items():
        layout_name = "currency_slot" if name == "currency_slot_blank" else name
        plate = crop_resize(topbar_component_source, TOPBAR_IMAGEGEN_BOXES[source_key], size_of(box))
        save_asset(plate, TOPBAR_DIR / f"{name}.png", manifest, group="TopBar", kind="imagegen_slice", layout_key=f"TopBar.{layout_name}", notes="Fresh imagegen top-bar plate with no baked runtime-owned text.")

    nav_defs = {
        "topbar_button_account": (TOPBAR_BOXES["tab_account"], "account"),
        "topbar_button_home": (TOPBAR_BOXES["badge_profile"], "profile"),
        "topbar_button_nav_power_up": (TOPBAR_BOXES["tab_power_up"], "powerup"),
        "topbar_button_nav_achievements": (TOPBAR_BOXES["tab_achievements"], "achievements"),
        "topbar_button_nav_minigames": (TOPBAR_BOXES["tab_minigames"], "minigames"),
    }
    for name, (box, source_key) in nav_defs.items():
        plate = crop_resize(topbar_component_source, TOPBAR_IMAGEGEN_BOXES[source_key], size_of(box))
        for suffix in ("", "_hover", "_pressed"):
            save_asset(plate, TOPBAR_DIR / f"{name}{suffix}.png", manifest, group="TopBar", kind="imagegen_slice", layout_key=f"TopBar.{name}", notes="Fresh imagegen top-bar plate. Hover/pressed reuse the generated plate until a generated state board exists.")
        if name in {"topbar_button_account", "topbar_button_home"}:
            save_asset(plate, TOPBAR_DIR / f"{name}_active.png", manifest, group="TopBar", kind="imagegen_slice", layout_key=f"TopBar.{name}", notes="Fresh imagegen top-bar active plate reuse.")
    for suffix in ("", "_hover", "_pressed"):
        copy_asset(TOPBAR_DIR / f"topbar_button_nav_power_up{suffix}.png", TOPBAR_DIR / f"topbar_button_nav{suffix}.png", manifest, group="TopBar", kind="alias_copy", notes="Canonical shared nav state alias.")
    copy_asset(TOPBAR_DIR / "topbar_button_account.png", OUTPUT_ROOT / "topbar_button_account.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(TOPBAR_DIR / "topbar_button_account_active.png", OUTPUT_ROOT / "topbar_button_account_active.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(TOPBAR_DIR / "topbar_button_nav_power_up.png", OUTPUT_ROOT / "topbar_button_nav.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    currency = Image.open(TOPBAR_DIR / "currency_slot_blank.png").convert("RGBA")

    nav_generic_size = size_of(TOPBAR_BOXES["tab_power_up"])
    nav_source = Image.open(TOPBAR_DIR / "topbar_button_nav_power_up.png").convert("RGBA")
    for suffix in ("", "_hover", "_pressed", "_disabled"):
        name = "nav" + suffix
        save_asset(nav_source.resize(nav_generic_size, RESAMPLING.LANCZOS), SHEET_SLICES_DIR / "TopBar" / f"{name}.png", manifest, group="TopBar", kind="imagegen_slice", notes="Generated generic nav state plate alias.")
    copy_asset(SHEET_SLICES_DIR / "TopBar" / "nav.png", SHEET_SLICES_DIR / "TopBar" / "nav_normal.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias for the canonical generic nav normal state.")

    for icon_name, box in TOPBAR_ICON_BOXES.items():
        icon = crop_resize(topbar_component_source, TOPBAR_IMAGEGEN_BOXES[icon_name], size_of(box))
        save_asset(icon, TOPBAR_DIR / f"{icon_name}.png", manifest, group="TopBar", kind="imagegen_slice", layout_key=f"TopBar.{icon_name}", notes="Generated icon crop. Runtime nav icon overlay is disabled while nav plates bake icons.")
        copy_asset(TOPBAR_DIR / f"{icon_name}.png", OUTPUT_ROOT / f"{icon_name}.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    title = title_wordmark()
    save_asset(title, CENTER_DIR / "title_lockup_wordmark.png", manifest, group="Center", kind="baked_title_art", layout_key="Center.title_lockup", notes="Baked title art only. Tagline remains runtime/localizable text.")

    cta_stack_source = Image.open(CTA_STACK_FRAME_IMAGEGEN).convert("RGBA")
    cta_buttons_source = Image.open(CTA_BUTTONS_IMAGEGEN_ATLAS).convert("RGBA")
    stack = crop_resize(cta_stack_source, CTA_STACK_FRAME_IMAGEGEN_BOX, size_of(CENTER_BOXES["cta_stack_full"]))
    save_asset(stack, CENTER_DIR / "cta_stack_outer_frame.png", manifest, group="Center", kind="imagegen_slice", layout_key="Center.cta_stack_full", notes="Fresh imagegen CTA stack frame. Runtime buttons and labels render above it.")

    cta_defs = {
        "cta_button_new_game_wide_plate": (CENTER_BOXES["cta_button_new_game"], "green"),
        "cta_button_load_game_wide_plate": (CENTER_BOXES["cta_button_load_game"], "blue"),
        "cta_button_daily_challenge_wide_plate": (CENTER_BOXES["cta_button_daily_challenge"], "purple"),
    }
    plate_aliases = {
        "cta_button_new_game_wide_plate": "cta_button_new_game_plate",
        "cta_button_load_game_wide_plate": "cta_button_load_game_plate",
        "cta_button_daily_challenge_wide_plate": "cta_button_daily_challenge_plate",
    }
    for name, (box, color_key) in cta_defs.items():
        layout_key = "Center." + name.replace("_wide_plate", "")
        for suffix, state in (("", "normal"), ("_hover", "hover"), ("_pressed", "pressed"), ("_disabled", "disabled")):
            image = crop_resize(cta_buttons_source, CTA_IMAGEGEN_BOXES[f"{color_key}_{state}"], size_of(box))
            save_asset(image, CENTER_DIR / f"{name}{suffix}.png", manifest, group="Center", kind="imagegen_slice", layout_key=layout_key, notes="Fresh imagegen CTA button state plate with no baked text.")
            alias = plate_aliases[name] + suffix
            save_asset(image, CENTER_DIR / f"{alias}.png", manifest, group="Center", kind="imagegen_slice", layout_key=layout_key, notes="Fresh imagegen CTA plate alias with no baked text.")

    panel_source = Image.open(PANELS_IMAGEGEN_BOARD).convert("RGBA")
    left_shell = crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["left_shell"], size_of(LEFT_BOXES["shell_full_reference"]))
    save_asset(left_shell, LEFT_DIR / "shell_clean.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.shell_full_reference", notes="Fresh imagegen left panel shell with live content interior.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["profile_card"], size_of(LEFT_BOXES["profile_card_reference"])), LEFT_DIR / "profile_card_shell.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.profile_card_reference", notes="Fresh imagegen profile card shell for live avatar and text.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["search_field"], size_of(LEFT_BOXES["search_field_reference"])), LEFT_DIR / "search_field_shell.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.search_field_reference", notes="Fresh imagegen search shell for live text.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["friend_star_button"], size_of(LEFT_BOXES["friend_star_button"])), LEFT_DIR / "friend_star_button.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.friend_star_button", notes="Fresh imagegen friend favorite button.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["friend_invite_button"], size_of(LEFT_BOXES["friend_invite_button"])), LEFT_DIR / "friend_invite_button.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.friend_invite_button", notes="Fresh imagegen friend invite button.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["friend_offline_button"], size_of(LEFT_BOXES["friend_offline_button"])), LEFT_DIR / "friend_offline_button.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.friend_offline_button", notes="Fresh imagegen friend disabled/offline button.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["friend_avatar_frame"], size_of(LEFT_BOXES["friend_avatar_frame_source"])), LEFT_DIR / "friend_avatar_frame.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.friend_avatar_frame_source", notes="Fresh imagegen friend avatar frame. Runtime portrait renders over the center.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["party_slot_frame"], size_of(LEFT_BOXES["party_slot_source"])), LEFT_DIR / "party_slot_frame.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.party_slot_source", notes="Fresh imagegen party portrait frame. Runtime portrait renders over the center.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["close_button"], size_of(LEFT_BOXES["close_button"])), LEFT_DIR / "close_button.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.close_button", notes="Fresh imagegen close/leave button.")
    save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["filter_world_button"], size_of(LEFT_BOXES["search_icon"])), LEFT_DIR / "search_icon.png", manifest, group="LeftPanel", kind="imagegen_slice", layout_key="Left.search_icon", notes="Fresh imagegen search/filter icon crop.")
    profile_fallback = make_linear_gradient((101, 93), (68, 83, 76), (21, 24, 27))
    ImageDraw.Draw(profile_fallback, "RGBA").ellipse((24, 12, 77, 65), fill=(122, 119, 99, 255))
    ImageDraw.Draw(profile_fallback, "RGBA").rectangle((18, 62, 83, 92), fill=(54, 58, 52, 255))
    save_asset(profile_fallback, LEFT_DIR / "profile_avatar_reference.png", manifest, group="LeftPanel", kind="fallback_avatar", notes="Fallback portrait only; Steam avatar remains live.")

    right_shell = crop_resize(panel_source, PANEL_IMAGEGEN_BOXES["right_shell"], size_of(RIGHT_BOXES["shell_full_reference"]))
    save_asset(right_shell, RIGHT_DIR / "shell_clean.png", manifest, group="RightPanel", kind="imagegen_slice", layout_key="Right.shell_full_reference", notes="Fresh imagegen right leaderboard shell.")
    for name in ("filter_world_button", "filter_friends_button", "filter_crown_button", "tab_weekly_active", "tab_all_time_inactive", "dropdown_shell_left", "dropdown_shell_right", "toggle_score_selected", "toggle_speedrun_unselected"):
        save_asset(crop_resize(panel_source, PANEL_IMAGEGEN_BOXES[name], size_of(RIGHT_BOXES[name])), RIGHT_DIR / f"{name}.png", manifest, group="RightPanel", kind="imagegen_slice", layout_key=f"Right.{name}", notes="Fresh imagegen right panel control shell.")
    copy_asset(RIGHT_DIR / "dropdown_shell_left.png", RIGHT_DIR / "dropdown_shell_left_clean.png", manifest, group="RightPanel", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(RIGHT_DIR / "dropdown_shell_right.png", RIGHT_DIR / "dropdown_shell_right_clean.png", manifest, group="RightPanel", kind="alias_copy", notes="Runtime compatibility alias.")
    leaderboard_avatar_frame = crop_visible_square_resize(
        Image.open(LEADERBOARD_AVATAR_FRAME_IMAGEGEN_ALPHA),
        size_of(RIGHT_BOXES["leaderboard_avatar_frame_source"]))
    save_asset(leaderboard_avatar_frame, RIGHT_DIR / "leaderboard_avatar_frame.png", manifest, group="RightPanel", kind="socket_frame", layout_key="Right.leaderboard_avatar_frame_source", notes="Fresh Codex-native generated leaderboard avatar socket frame with transparent center aperture. Runtime Steam/API portrait renders underneath.")

    for i in range(1, 4):
        fallback = make_linear_gradient((30, 30), (72 + i * 8, 76, 69), (26, 28, 31))
        ImageDraw.Draw(fallback, "RGBA").ellipse((7, 5, 23, 21), fill=(142, 126, 98, 255))
        save_asset(fallback, RIGHT_DIR / f"leaderboard_avatar_fallback_{i:02d}.png", manifest, group="RightPanel", kind="fallback_avatar", notes="Fallback leaderboard avatar only.")

    master = background.copy()
    paste_at(master, topbar_shell, TOPBAR_BOXES["topbar_strip_full"])
    for name in ("button_settings", "button_chat", "tab_account", "badge_profile", "tab_power_up", "tab_achievements", "tab_minigames", "currency_slot", "button_power"):
        if name.startswith("tab_"):
            asset_name = {
                "tab_account": "topbar_button_account",
                "tab_power_up": "topbar_button_nav_power_up",
                "tab_achievements": "topbar_button_nav_achievements",
                "tab_minigames": "topbar_button_nav_minigames",
            }[name]
            image = Image.open(TOPBAR_DIR / f"{asset_name}.png").convert("RGBA")
        elif name == "badge_profile":
            image = Image.open(TOPBAR_DIR / "badge_profile.png").convert("RGBA")
        elif name == "currency_slot":
            image = currency
        else:
            image = Image.open(TOPBAR_DIR / f"{name}.png").convert("RGBA")
        paste_at(master, image, TOPBAR_BOXES[name])
    paste_at(master, title, CENTER_BOXES["title_lockup"])
    paste_at(master, stack, CENTER_BOXES["cta_stack_full"])
    for plate, box in (
        ("cta_button_new_game_wide_plate", CENTER_BOXES["cta_button_new_game"]),
        ("cta_button_load_game_wide_plate", CENTER_BOXES["cta_button_load_game"]),
        ("cta_button_daily_challenge_wide_plate", CENTER_BOXES["cta_button_daily_challenge"]),
    ):
        paste_at(master, Image.open(CENTER_DIR / f"{plate}.png").convert("RGBA"), box)
    paste_at(master, left_shell, LEFT_BOXES["shell_full_reference"])
    paste_at(master, right_shell, RIGHT_BOXES["shell_full_reference"])
    master.save(SOURCE_REFERENCE)

    overlay = master.copy()
    overlay_draw = ImageDraw.Draw(overlay, "RGBA")
    for boxes in (TOPBAR_BOXES, CENTER_BOXES, LEFT_BOXES, RIGHT_BOXES, EXTRA_LAYOUT_BOXES["MainMenu"]):
        for box in boxes.values():
            overlay_draw.rectangle(box, outline=(255, 204, 74, 190), width=2)
    overlay.save(DEBUG_DIR / "reference_crop_overlay.png")
    manifest.append(
        {
            "path": "debug/reference_crop_overlay.png",
            "group": "Debug",
            "kind": "layout_overlay",
            "source_box": [0, 0, overlay.width, overlay.height],
            "notes": "Canonical 1920 layout overlay for debugging only.",
        }
    )

    build_layout_files()

    contact_items = [
        ("Top bar", TOPBAR_DIR / "topbar_shell_clean.png"),
        ("CTA frame", CENTER_DIR / "cta_stack_outer_frame.png"),
        ("New game", CENTER_DIR / "cta_button_new_game_wide_plate.png"),
        ("Load game", CENTER_DIR / "cta_button_load_game_wide_plate.png"),
        ("Daily", CENTER_DIR / "cta_button_daily_challenge_wide_plate.png"),
        ("Left shell", LEFT_DIR / "shell_clean.png"),
        ("Right shell", RIGHT_DIR / "shell_clean.png"),
        ("Title", CENTER_DIR / "title_lockup_wordmark.png"),
    ]
    contact = Image.new("RGBA", (1600, 1200), (18, 18, 21, 255))
    draw = ImageDraw.Draw(contact)
    label_font = font(22, bold=True)
    x, y = 30, 30
    for label, path in contact_items:
        item = Image.open(path).convert("RGBA")
        thumb = ImageOps.contain(item, (720, 210), RESAMPLING.LANCZOS)
        contact.alpha_composite(thumb, (x, y + 34))
        draw.text((x, y), label, font=label_font, fill=CREAM)
        x += 780
        if x > 1000:
            x = 30
            y += 285
    save_asset(contact, SPRITESHEET_DIR / "mainmenu_canonical_components_1920.png", manifest, group="SpriteSheet", kind="contact_sheet", notes="Canonical 1920 component family sheet for review.")
    contact.save(OUTPUT_ROOT / "asset_pack_preview.png")

    slice_manifest = [
        {
            "source_sheet": "mainmenu_canonical_components_1920.png",
            "output": entry["path"],
            "size": [entry["source_box"][2], entry["source_box"][3]],
            "kind": entry["kind"],
        }
        for entry in manifest
        if entry["path"].startswith(("TopBar/", "Center/", "LeftPanel/", "RightPanel/", "SheetSlices/"))
    ]
    (SHEET_SLICES_DIR / "slice_manifest.json").write_text(json.dumps(slice_manifest, indent=2) + "\n", encoding="utf-8")

    manifest_payload = {
        "source_image": str(SOURCE_REFERENCE),
        "canvas_size": [CANVAS[0], CANVAS[1]],
        "output_root": str(OUTPUT_ROOT),
        "assets": manifest,
    }
    (OUTPUT_ROOT / "asset_manifest.json").write_text(json.dumps(manifest_payload, indent=2) + "\n", encoding="utf-8")

    ownership_path = OUTPUT_ROOT / "content_ownership.json"
    if not ownership_path.exists():
        ownership_path.write_text(
            json.dumps(
                {
                    "screen": "MainMenu",
                    "status": "canonical_1920_required",
                    "notes": [
                        "The title wordmark is baked art.",
                        "The tagline, labels, friend/profile data, leaderboard rows, values, and avatars are runtime-owned.",
                    ],
                    "regions": [],
                },
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )


def load_style_reference() -> Image.Image:
    with Image.open(SOURCE_REFERENCE) as image:
        reference = image.convert("RGBA").resize(CANVAS, RESAMPLING.LANCZOS)
    reference.save(SOURCE_REFERENCE)
    return reference


def crop_ref(reference: Image.Image, box: Box) -> Image.Image:
    return reference.crop(box).convert("RGBA")


def crop_resize(source: Image.Image, box: Box, size: tuple[int, int]) -> Image.Image:
    return source.crop(box).convert("RGBA").resize(size, RESAMPLING.LANCZOS)


def crop_visible_square_resize(source: Image.Image, size: tuple[int, int]) -> Image.Image:
    source = source.convert("RGBA")
    bbox = source.getchannel("A").getbbox()
    if bbox is None:
        raise ValueError("generated alpha source has no visible pixels")

    left, top, right, bottom = bbox
    side = max(right - left, bottom - top)
    center_x = (left + right) / 2
    center_y = (top + bottom) / 2
    crop_box = (
        int(round(center_x - side / 2)),
        int(round(center_y - side / 2)),
        int(round(center_x + side / 2)),
        int(round(center_y + side / 2)),
    )

    cropped = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    src_box = (
        max(0, crop_box[0]),
        max(0, crop_box[1]),
        min(source.width, crop_box[2]),
        min(source.height, crop_box[3]),
    )
    dest = (src_box[0] - crop_box[0], src_box[1] - crop_box[1])
    cropped.alpha_composite(source.crop(src_box), dest)
    return cropped.resize(size, RESAMPLING.LANCZOS)


def fill_rects_with_sample(image: Image.Image, rects: list[Box], sample_box: Box | None = None, blur: int = 8) -> Image.Image:
    result = image.copy()
    draw = ImageDraw.Draw(result, "RGBA")
    for rect in rects:
        if sample_box is not None:
            sample = result.crop(sample_box).filter(ImageFilter.GaussianBlur(blur))
            fill = sample.resize((rect[2] - rect[0], rect[3] - rect[1]), RESAMPLING.LANCZOS)
            result.alpha_composite(fill, rect[:2])
        else:
            draw.rectangle(rect, fill=(12, 14, 17, 235))
    return result


def clear_rect_alpha(image: Image.Image, rect: Box) -> Image.Image:
    result = image.copy()
    alpha = result.getchannel("A")
    ImageDraw.Draw(alpha).rectangle(rect, fill=0)
    result.putalpha(alpha)
    return result


def clear_rects_alpha(image: Image.Image, rects: list[Box], inflate: int = 0) -> Image.Image:
    result = image.convert("RGBA")
    alpha = result.getchannel("A")
    draw = ImageDraw.Draw(alpha)
    pixels = result.load()
    for rect in rects:
        expanded = (
            rect[0] - inflate,
            rect[1] - inflate,
            rect[2] + inflate,
            rect[3] + inflate,
        )
        cleared = clamp_box(expanded, result.size)
        draw.rectangle(cleared, fill=0)
        for y in range(cleared[1], cleared[3]):
            for x in range(cleared[0], cleared[2]):
                pixels[x, y] = (0, 0, 0, 0)
    result.putalpha(alpha)
    return result


def remove_label_band(image: Image.Image, rect: Box) -> Image.Image:
    sample = (
        max(0, rect[0]),
        max(0, rect[1] - 10),
        min(image.width, rect[2]),
        max(rect[1], rect[1] - 2),
    )
    if sample[3] <= sample[1]:
        sample = None
    return fill_rects_with_sample(image, [rect], sample, blur=12)


def remove_bright_label_text(image: Image.Image, rect: Box) -> Image.Image:
    result = image.convert("RGBA")
    x0, y0, x1, y1 = clamp_box(rect, result.size)
    if x1 <= x0 or y1 <= y0:
        return result

    mask = Image.new("L", result.size, 0)
    source_pixels = result.load()
    mask_pixels = mask.load()
    for y in range(y0, y1):
        for x in range(x0, x1):
            red, green, blue, alpha = source_pixels[x, y]
            if alpha == 0:
                continue
            brightness = max(red, green, blue)
            saturation = max(red, green, blue) - min(red, green, blue)
            warm_label = red > 145 and green > 122 and blue > 82
            pale_label = brightness > 150 and saturation < 92
            if warm_label or pale_label:
                mask_pixels[x, y] = 255

    if mask.getbbox() is None:
        return result

    dilated_mask = mask.filter(ImageFilter.MaxFilter(9)).filter(ImageFilter.GaussianBlur(1.2))
    fill = result.copy()
    fill_pixels = fill.load()
    mask_pixels = mask.load()
    for y in range(y0, y1):
        row_samples: list[tuple[int, int, int, int]] = []
        for x in range(x0, x1):
            if mask_pixels[x, y] != 0:
                continue
            red, green, blue, alpha = source_pixels[x, y]
            if alpha == 0:
                continue
            brightness = max(red, green, blue)
            if brightness < 140:
                row_samples.append((red, green, blue, alpha))

        if not row_samples:
            continue

        median = tuple(
            sorted(pixel[channel] for pixel in row_samples)[len(row_samples) // 2]
            for channel in range(4)
        )
        for x in range(x0, x1):
            if mask_pixels[x, y] != 0:
                fill_pixels[x, y] = median

    fill = fill.filter(ImageFilter.GaussianBlur(1.4))
    return Image.composite(fill, result, dilated_mask)


def apply_beveled_alpha(image: Image.Image, corner: int = 16) -> Image.Image:
    result = image.convert("RGBA")
    width, height = result.size
    mask = Image.new("L", result.size, 0)
    inset = max(1, min(corner, width // 5, height // 3))
    polygon = [
        (inset, 0),
        (width - inset, 0),
        (width, inset),
        (width, height - inset),
        (width - inset, height),
        (inset, height),
        (0, height - inset),
        (0, inset),
    ]
    ImageDraw.Draw(mask).polygon(polygon, fill=255)
    result.putalpha(mask)
    return result


def title_from_reference(reference: Image.Image) -> Image.Image:
    crop = crop_ref(reference, CENTER_BOXES["title_lockup"])
    rgba = crop.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    for y in range(height):
        for x in range(width):
            red, green, blue, alpha = pixels[x, y]
            brightness = max(red, green, blue)
            saturation = max(red, green, blue) - min(red, green, blue)
            keep_title = brightness > 94 and saturation > 10 and y < 126
            if not keep_title:
                pixels[x, y] = (red, green, blue, 0)
    return rgba.filter(ImageFilter.UnsharpMask(radius=1.0, percent=115, threshold=3))


def alpha_bright_icon(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    border_pixels = []
    width, height = rgba.size
    for x in range(width):
        border_pixels.append(rgba.getpixel((x, 0))[:3])
        border_pixels.append(rgba.getpixel((x, height - 1))[:3])
    for y in range(height):
        border_pixels.append(rgba.getpixel((0, y))[:3])
        border_pixels.append(rgba.getpixel((width - 1, y))[:3])
    background = tuple(sorted(channel)[len(channel) // 2] for channel in zip(*border_pixels))
    pixels = rgba.load()
    for y in range(height):
        for x in range(width):
            red, green, blue, alpha = pixels[x, y]
            brightness = max(red, green, blue)
            distance = abs(red - background[0]) + abs(green - background[1]) + abs(blue - background[2])
            saturation = max(red, green, blue) - min(red, green, blue)
            if brightness < 82 or distance < 34 or saturation < 8:
                pixels[x, y] = (red, green, blue, 0)
            else:
                pixels[x, y] = (red, green, blue, alpha)
    return rgba.filter(ImageFilter.UnsharpMask(radius=0.8, percent=130, threshold=2))


def make_background_from_reference(reference: Image.Image) -> Image.Image:
    result = reference.copy()
    blurred = reference.filter(ImageFilter.GaussianBlur(28))
    mask_rects = [
        TOPBAR_BOXES["topbar_strip_full"],
        EXTRA_LAYOUT_BOXES["MainMenu"]["left_panel_assembly"],
        EXTRA_LAYOUT_BOXES["MainMenu"]["right_panel_assembly"],
        CENTER_BOXES["title_lockup"],
        CENTER_BOXES["subtitle_lockup"],
        CENTER_BOXES["cta_stack_full"],
    ]
    for rect in mask_rects:
        patch = blurred.crop(rect)
        patch = ImageEnhance.Brightness(patch).enhance(0.72)
        result.alpha_composite(patch, rect[:2])
    return result


def build_pack_legacy_reference_cleanup_disabled() -> None:
    reset_output_tree()
    manifest: list[dict[str, object]] = []
    reference = load_style_reference()
    topbar_generated = Image.open(TOPBAR_IMAGEGEN_BOARD).convert("RGBA") if TOPBAR_IMAGEGEN_BOARD.exists() else None
    panel_sample = (40, 620, 165, 715)
    topbar_sample = (258, 28, 316, 118)

    background = make_background_from_reference(reference)
    save_asset(background, OUTPUT_ROOT / "scene_background_1920x1080.png", manifest, group="Scene", kind="ui_free_scene_plate", notes="Canonical 1920 UI-free scene/background plate derived from the approved style reference.")
    save_asset(background, OUTPUT_ROOT / "scene_background_generated_source.png", manifest, group="Scene", kind="ui_free_scene_plate_source", notes="Canonical style-derived scene plate source.")

    topbar_full = crop_ref(reference, TOPBAR_BOXES["topbar_strip_full"])
    if topbar_generated is not None:
        topbar_clean = crop_resize(topbar_generated, TOPBAR_IMAGEGEN_BOXES["backdrop"], size_of(TOPBAR_BOXES["topbar_strip_full"]))
    else:
        topbar_button_rects = [rel_box(box, TOPBAR_BOXES["topbar_strip_full"]) for key, box in TOPBAR_BOXES.items() if key != "topbar_strip_full"]
        topbar_backdrop_clean_rects = [
            clamp_box((rect[0] - 8, rect[1] - 8, rect[2] + 8, rect[3] + 8), size_of(TOPBAR_BOXES["topbar_strip_full"]))
            for rect in topbar_button_rects
        ]
        topbar_clean = fill_rects_with_sample(topbar_full, topbar_backdrop_clean_rects, rel_box(topbar_sample, TOPBAR_BOXES["topbar_strip_full"]), blur=18)
    save_asset(topbar_full, TOPBAR_DIR / "topbar_strip_full.png", manifest, group="TopBar", kind="reference_crop", layout_key="TopBar.topbar_strip_full", notes="Approved reference top-bar crop.")
    save_asset(topbar_clean, TOPBAR_DIR / "topbar_shell_clean.png", manifest, group="TopBar", kind="generated_shell", layout_key="TopBar.topbar_strip_full", notes="Fresh imagegen top-bar backing; individual button plates own control borders and wedges.")
    save_asset(topbar_clean, TOPBAR_DIR / "topbar_backdrop_clean.png", manifest, group="TopBar", kind="generated_shell", layout_key="TopBar.topbar_strip_full", notes="Fresh imagegen top-bar backing; individual button plates own control borders and wedges.")
    copy_asset(TOPBAR_DIR / "topbar_shell_clean.png", OUTPUT_ROOT / "topbar_shell.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(TOPBAR_DIR / "topbar_backdrop_clean.png", OUTPUT_ROOT / "topbar_backdrop.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    for name in ("button_settings", "button_chat", "button_power", "badge_profile", "currency_slot"):
        output_name = "currency_slot_blank" if name == "currency_slot" else name
        generated_topbar_key = {
            "button_settings": "settings",
            "button_chat": "chat",
            "button_power": "quit",
            "badge_profile": "profile",
            "currency_slot": "currency",
        }[name]
        if topbar_generated is not None:
            image = crop_resize(topbar_generated, TOPBAR_IMAGEGEN_BOXES[generated_topbar_key], size_of(TOPBAR_BOXES[name]))
            notes = "Fresh imagegen top-bar button plate. Runtime owns labels and icons."
        else:
            image = crop_ref(reference, TOPBAR_BOXES[name])
            if name == "currency_slot":
                image = remove_bright_label_text(image, (74, 12, image.width - 14, image.height - 12))
            notes = "Top-bar component cropped from approved reference style."
        save_asset(image, TOPBAR_DIR / f"{output_name}.png", manifest, group="TopBar", kind="button_plate", layout_key=f"TopBar.{name}", notes=notes)

    nav_defs = {
        "topbar_button_account": (TOPBAR_BOXES["tab_account"], "account"),
        "topbar_button_home": (TOPBAR_BOXES["badge_profile"], "profile"),
        "topbar_button_nav_power_up": (TOPBAR_BOXES["tab_power_up"], "powerup"),
        "topbar_button_nav_achievements": (TOPBAR_BOXES["tab_achievements"], "achievements"),
        "topbar_button_nav_minigames": (TOPBAR_BOXES["tab_minigames"], "minigames"),
    }
    for asset_name, (box, generated_key) in nav_defs.items():
        if topbar_generated is not None:
            clean = crop_resize(topbar_generated, TOPBAR_IMAGEGEN_BOXES[generated_key], size_of(box))
        else:
            plate = crop_ref(reference, box)
            label_rect = (22, 16, plate.width - 16, plate.height - 16)
            clean = remove_bright_label_text(plate, label_rect)
        for suffix, factor in (("", 1.0), ("_hover", 1.12), ("_pressed", 0.82)):
            variant = ImageEnhance.Brightness(clean).enhance(factor)
            save_asset(variant, TOPBAR_DIR / f"{asset_name}{suffix}.png", manifest, group="TopBar", kind="button_state", layout_key=f"TopBar.{asset_name}", notes="Top-bar state plate cropped and cleaned from approved reference style.")
        if asset_name in {"topbar_button_account", "topbar_button_home"}:
            save_asset(ImageEnhance.Brightness(clean).enhance(1.16), TOPBAR_DIR / f"{asset_name}_active.png", manifest, group="TopBar", kind="button_state", layout_key=f"TopBar.{asset_name}", notes="Top-bar active plate cropped and cleaned from approved reference style.")
    for suffix in ("", "_hover", "_pressed"):
        copy_asset(TOPBAR_DIR / f"topbar_button_nav_power_up{suffix}.png", TOPBAR_DIR / f"topbar_button_nav{suffix}.png", manifest, group="TopBar", kind="alias_copy", notes="Canonical shared nav state alias.")
    copy_asset(TOPBAR_DIR / "topbar_button_account.png", OUTPUT_ROOT / "topbar_button_account.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(TOPBAR_DIR / "topbar_button_account_active.png", OUTPUT_ROOT / "topbar_button_account_active.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")
    copy_asset(TOPBAR_DIR / "topbar_button_nav_power_up.png", OUTPUT_ROOT / "topbar_button_nav.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    for suffix, factor in (("", 1.0), ("_hover", 1.12), ("_pressed", 0.82), ("_disabled", 0.5)):
        nav = ImageEnhance.Brightness(Image.open(TOPBAR_DIR / "topbar_button_nav_power_up.png").convert("RGBA")).enhance(factor)
        save_asset(nav, SHEET_SLICES_DIR / "TopBar" / f"nav{suffix}.png", manifest, group="TopBar", kind="button_state", notes="Canonical generic nav state plate.")
    copy_asset(SHEET_SLICES_DIR / "TopBar" / "nav.png", SHEET_SLICES_DIR / "TopBar" / "nav_normal.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    icon_fallbacks = {
        "icon_powerup": "crown",
        "icon_achievements": "star",
        "icon_minigames": "world",
    }
    for name, box in TOPBAR_ICON_BOXES.items():
        icon = alpha_bright_icon(crop_ref(reference, box))
        alpha = icon.getchannel("A")
        alpha_values = list(alpha.getdata())
        transparent_count = sum(1 for value in alpha_values if value == 0)
        if icon.getbbox() is None or transparent_count < len(alpha_values) * 0.12:
            icon = make_icon(icon_fallbacks[name], size_of(box), color=(235, 215, 159, 255))
        save_asset(icon, TOPBAR_DIR / f"{name}.png", manifest, group="TopBar", kind="icon", layout_key=f"TopBar.{name}", notes="Top-bar icon crop from approved reference.")
        copy_asset(TOPBAR_DIR / f"{name}.png", OUTPUT_ROOT / f"{name}.png", manifest, group="TopBar", kind="alias_copy", notes="Runtime compatibility alias.")

    title = title_from_reference(reference)
    save_asset(title, CENTER_DIR / "title_lockup_wordmark.png", manifest, group="Center", kind="baked_title_art", layout_key="Center.title_lockup", notes="Baked title art from approved style reference. Tagline remains live text.")

    cta_stack = crop_ref(reference, CENTER_BOXES["cta_stack_full"])
    button_rects = [
        rel_box(CENTER_BOXES["cta_button_new_game"], CENTER_BOXES["cta_stack_full"]),
        rel_box(CENTER_BOXES["cta_button_load_game"], CENTER_BOXES["cta_stack_full"]),
        rel_box(CENTER_BOXES["cta_button_daily_challenge"], CENTER_BOXES["cta_stack_full"]),
    ]
    cta_stack_clean = fill_rects_with_sample(cta_stack, button_rects, (16, 118, 96, 188), blur=16)
    save_asset(cta_stack_clean, CENTER_DIR / "cta_stack_outer_frame.png", manifest, group="Center", kind="runtime_frame", layout_key="Center.cta_stack_full", notes="CTA stack frame with neutral button wells; CTA button plates own the visible controls.")

    cta_defs = {
        "cta_button_new_game": CENTER_BOXES["cta_button_new_game"],
        "cta_button_load_game": CENTER_BOXES["cta_button_load_game"],
        "cta_button_daily_challenge": CENTER_BOXES["cta_button_daily_challenge"],
    }
    for base, box in cta_defs.items():
        plate = crop_ref(reference, box)
        label_rect = (38, 20, plate.width - 38, plate.height - 18)
        clean = apply_beveled_alpha(remove_label_band(plate, label_rect), corner=17)
        for suffix, factor in (("", 1.0), ("_hover", 1.12), ("_pressed", 0.82), ("_disabled", 0.52)):
            variant = ImageEnhance.Brightness(clean).enhance(factor)
            save_asset(variant, CENTER_DIR / f"{base}_wide_plate{suffix}.png", manifest, group="Center", kind="button_state", layout_key=f"Center.{base}", notes="CTA state plate cropped and text-cleaned from approved style reference.")
            save_asset(variant, CENTER_DIR / f"{base}_plate{suffix}.png", manifest, group="Center", kind="button_state", layout_key=f"Center.{base}", notes="CTA state plate alias cropped and text-cleaned from approved style reference.")

    left_shell = crop_ref(reference, LEFT_BOXES["shell_full_reference"])
    left_shell = fill_rects_with_sample(left_shell, [(26, 26, left_shell.width - 26, left_shell.height - 26)], rel_box(panel_sample, LEFT_BOXES["shell_full_reference"]), blur=12)
    save_asset(left_shell, LEFT_DIR / "shell_clean.png", manifest, group="LeftPanel", kind="clean_shell", layout_key="Left.shell_full_reference", notes="Left panel shell cropped from approved reference with live interior cleaned.")
    profile_card = crop_ref(reference, LEFT_BOXES["profile_card_reference"])
    profile_card = fill_rects_with_sample(profile_card, [(22, 15, profile_card.width - 16, profile_card.height - 14)], (20, 20, min(profile_card.width - 20, 110), min(profile_card.height - 10, 75)), blur=12)
    save_asset(profile_card, LEFT_DIR / "profile_card_shell.png", manifest, group="LeftPanel", kind="clean_component", layout_key="Left.profile_card_reference", notes="Profile card shell with live avatar/text wells cleaned.")
    search = crop_ref(reference, LEFT_BOXES["search_field_reference"])
    search = remove_label_band(search, (28, 12, search.width - 48, search.height - 10))
    save_asset(search, LEFT_DIR / "search_field_shell.png", manifest, group="LeftPanel", kind="clean_component", layout_key="Left.search_field_reference", notes="Search shell with live text well cleaned.")
    for name in ("search_icon", "friend_star_button", "friend_invite_button", "friend_offline_button", "close_button"):
        save_asset(crop_ref(reference, LEFT_BOXES[name]), LEFT_DIR / f"{name}.png", manifest, group="LeftPanel", kind="component_crop", layout_key=f"Left.{name}", notes="Left panel component crop from approved reference style.")
    friend_frame = clear_rect_alpha(crop_ref(reference, LEFT_BOXES["friend_avatar_frame_source"]), (8, 8, size_of(LEFT_BOXES["friend_avatar_frame_source"])[0] - 8, size_of(LEFT_BOXES["friend_avatar_frame_source"])[1] - 8))
    save_asset(friend_frame, LEFT_DIR / "friend_avatar_frame.png", manifest, group="LeftPanel", kind="socket_frame", layout_key="Left.friend_avatar_frame_source", notes="Transparent friend avatar frame cropped from approved style reference.")
    party_frame = clear_rect_alpha(crop_ref(reference, LEFT_BOXES["party_slot_source"]), (10, 10, size_of(LEFT_BOXES["party_slot_source"])[0] - 10, size_of(LEFT_BOXES["party_slot_source"])[1] - 10))
    save_asset(party_frame, LEFT_DIR / "party_slot_frame.png", manifest, group="LeftPanel", kind="socket_frame", layout_key="Left.party_slot_source", notes="Transparent party avatar frame cropped from approved style reference.")
    save_asset(crop_ref(reference, (47, 367, 148, 460)), LEFT_DIR / "profile_avatar_reference.png", manifest, group="LeftPanel", kind="fallback_avatar", notes="Fallback profile portrait crop only; Steam avatar remains live.")

    right_shell = crop_ref(reference, RIGHT_BOXES["shell_full_reference"])
    right_shell = fill_rects_with_sample(right_shell, [(18, 26, right_shell.width - 18, right_shell.height - 26)], (28, 70, min(right_shell.width - 20, 130), min(right_shell.height - 20, 170)), blur=12)
    save_asset(right_shell, RIGHT_DIR / "shell_clean.png", manifest, group="RightPanel", kind="clean_shell", layout_key="Right.shell_full_reference", notes="Right panel shell cropped from approved reference with leaderboard interior cleaned.")
    for name in ("filter_world_button", "filter_friends_button", "filter_crown_button", "tab_weekly_active", "tab_all_time_inactive", "dropdown_shell_left", "dropdown_shell_right", "toggle_score_selected", "toggle_speedrun_unselected"):
        image = crop_ref(reference, RIGHT_BOXES[name])
        if name.startswith(("tab_", "dropdown_", "toggle_")):
            image = remove_label_band(image, (16, 12, image.width - 16, image.height - 10))
        save_asset(image, RIGHT_DIR / f"{name}.png", manifest, group="RightPanel", kind="control_shell", layout_key=f"Right.{name}", notes="Right panel control cropped and cleaned from approved reference style.")
    leaderboard_frame = clear_rect_alpha(crop_ref(reference, RIGHT_BOXES["leaderboard_avatar_frame_source"]), (8, 8, size_of(RIGHT_BOXES["leaderboard_avatar_frame_source"])[0] - 8, size_of(RIGHT_BOXES["leaderboard_avatar_frame_source"])[1] - 8))
    save_asset(leaderboard_frame, RIGHT_DIR / "leaderboard_avatar_frame.png", manifest, group="RightPanel", kind="socket_frame", layout_key="Right.leaderboard_avatar_frame_source", notes="Transparent leaderboard avatar frame cropped from approved reference.")
    for idx, box in enumerate(((1545, 643, 1575, 673), (1545, 707, 1575, 737), (1545, 771, 1575, 801)), start=1):
        save_asset(crop_ref(reference, box), RIGHT_DIR / f"leaderboard_avatar_fallback_{idx:02d}.png", manifest, group="RightPanel", kind="fallback_avatar", notes="Fallback leaderboard avatar crop only.")

    reference.save(SOURCE_REFERENCE)

    overlay = reference.copy()
    overlay_draw = ImageDraw.Draw(overlay, "RGBA")
    for boxes in (TOPBAR_BOXES, CENTER_BOXES, LEFT_BOXES, RIGHT_BOXES, EXTRA_LAYOUT_BOXES["MainMenu"]):
        for box in boxes.values():
            overlay_draw.rectangle(box, outline=(255, 204, 74, 190), width=2)
    UI_MAIN_MENU_REVIEW_DIR.mkdir(parents=True, exist_ok=True)
    overlay.save(UI_MAIN_MENU_REVIEW_DIR / "reference_crop_overlay.png")

    build_layout_files()

    contact = Image.new("RGBA", (1600, 1200), (18, 18, 21, 255))
    draw = ImageDraw.Draw(contact)
    label_font = font(22, bold=True)
    contact_items = [
        ("Reference", SOURCE_REFERENCE),
        ("Background", OUTPUT_ROOT / "scene_background_1920x1080.png"),
        ("Title", CENTER_DIR / "title_lockup_wordmark.png"),
        ("CTA frame", CENTER_DIR / "cta_stack_outer_frame.png"),
        ("New game", CENTER_DIR / "cta_button_new_game_wide_plate.png"),
        ("Left shell", LEFT_DIR / "shell_clean.png"),
        ("Right shell", RIGHT_DIR / "shell_clean.png"),
        ("Topbar", TOPBAR_DIR / "topbar_shell_clean.png"),
    ]
    x, y = 30, 30
    for label, path in contact_items:
        thumb = ImageOps.contain(Image.open(path).convert("RGBA"), (720, 220), RESAMPLING.LANCZOS)
        contact.alpha_composite(thumb, (x, y + 34))
        draw.text((x, y), label, font=label_font, fill=CREAM)
        x += 780
        if x > 1000:
            x = 30
            y += 285
    save_asset(contact, SPRITESHEET_DIR / "mainmenu_canonical_components_1920.png", manifest, group="SpriteSheet", kind="contact_sheet", notes="Canonical 1920 style-derived component family sheet.")
    contact.save(OUTPUT_ROOT / "asset_pack_preview.png")

    slice_manifest = [
        {
            "source_sheet": "mainmenu_canonical_components_1920.png",
            "output": entry["path"],
            "size": [entry["source_box"][2], entry["source_box"][3]],
            "kind": entry["kind"],
        }
        for entry in manifest
        if entry["path"].startswith(("TopBar/", "Center/", "LeftPanel/", "RightPanel/", "SheetSlices/"))
    ]
    (SHEET_SLICES_DIR / "slice_manifest.json").write_text(json.dumps(slice_manifest, indent=2) + "\n", encoding="utf-8")

    manifest_payload = {
        "source_image": str(SOURCE_REFERENCE),
        "canvas_size": [CANVAS[0], CANVAS[1]],
        "output_root": str(OUTPUT_ROOT),
        "assets": manifest,
    }
    (OUTPUT_ROOT / "asset_manifest.json").write_text(json.dumps(manifest_payload, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    build_pack()
