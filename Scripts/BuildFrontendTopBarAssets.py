from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageChops, ImageColor, ImageDraw, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "SourceAssets" / "UI" / "Dota" / "Generated"
OUTPUT_DIR = SOURCE_DIR

NEUTRAL_PLATE = SOURCE_DIR / "dota_neutral_button_plate.png"
DANGER_PLATE = SOURCE_DIR / "dota_danger_button_plate.png"

TOPBAR_OUT = OUTPUT_DIR / "frontend_topbar_plate.png"
TOPBAR_LEFT_OUT = OUTPUT_DIR / "frontend_topbar_plate_left.png"
TOPBAR_FILL_OUT = OUTPUT_DIR / "frontend_topbar_plate_fill.png"
TOPBAR_RIGHT_OUT = OUTPUT_DIR / "frontend_topbar_plate_right.png"
SETTINGS_SLOT_OUT = OUTPUT_DIR / "frontend_topbar_settings_slot.png"
UTILITY_SLOT_OUT = OUTPUT_DIR / "frontend_topbar_utility_slot.png"
QUIT_SLOT_OUT = OUTPUT_DIR / "frontend_topbar_quit_slot.png"
NAV_TAB_OUT = OUTPUT_DIR / "frontend_topbar_nav_tab.png"
ACTIVE_TAB_OUT = OUTPUT_DIR / "frontend_topbar_active_tab.png"
SEPARATOR_OUT = OUTPUT_DIR / "frontend_topbar_separator.png"
SETTINGS_ICON_OUT = OUTPUT_DIR / "frontend_topbar_settings_icon.png"
LANGUAGE_ICON_OUT = OUTPUT_DIR / "frontend_topbar_language_icon.png"
ACHIEVEMENT_COINS_ICON_OUT = OUTPUT_DIR / "frontend_topbar_achievement_coins_icon.png"
POWER_COUPONS_ICON_OUT = OUTPUT_DIR / "frontend_topbar_power_coupons_icon.png"
QUIT_ICON_OUT = OUTPUT_DIR / "frontend_topbar_quit_icon.png"
QUIT_GLOW_OUT = OUTPUT_DIR / "frontend_topbar_quit_glow.png"
ICON_PREVIEW_OUT = OUTPUT_DIR / "topbar_icon_sheet_preview.png"
AI_ACHIEVEMENT_BASE = OUTPUT_DIR / "topbar_ai_achievement_base.png"
AI_COUPON_BASE = OUTPUT_DIR / "topbar_ai_coupon_base.png"
AI_LANGUAGE_BASE = OUTPUT_DIR / "topbar_ai_language_base.png"

UI_FONT_BOLD = ROOT / "Content" / "Slate" / "Fonts" / "New Fonts" / "Cinzel" / "static" / "Cinzel-Bold.ttf"
UI_FONT_REGULAR = ROOT / "Content" / "Slate" / "Fonts" / "New Fonts" / "Cinzel" / "static" / "Cinzel-Regular.ttf"


def crop_to_alpha(image: Image.Image) -> Image.Image:
    alpha = image.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        raise RuntimeError("Image has no visible pixels to crop.")
    return image.crop(bbox)


def tint_image(image: Image.Image, color: tuple[int, int, int], alpha_scale: float = 1.0) -> Image.Image:
    image = image.convert("RGBA")
    alpha = image.getchannel("A")
    if alpha_scale != 1.0:
        alpha = alpha.point(lambda value: max(0, min(255, int(value * alpha_scale))))
    tinted = Image.new("RGBA", image.size, color + (0,))
    tinted.putalpha(alpha)
    return tinted


def alpha_composite_center(dst: Image.Image, src: Image.Image, y: int, x: int | None = None) -> None:
    if x is None:
        x = (dst.width - src.width) // 2
    dst.alpha_composite(src, (x, y))


def make_shadow(alpha_source: Image.Image, color: tuple[int, int, int, int], blur: float) -> Image.Image:
    shadow = Image.new("RGBA", alpha_source.size, color[:3] + (0,))
    shadow.putalpha(alpha_source.getchannel("A"))
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=blur))
    if color[3] < 255:
        shadow_alpha = shadow.getchannel("A").point(lambda value: int(value * (color[3] / 255.0)))
        shadow.putalpha(shadow_alpha)
    return shadow


def fit_crop(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    image = image.convert("RGBA")
    scale = max(size[0] / image.width, size[1] / image.height)
    resized = image.resize(
        (max(1, int(round(image.width * scale))), max(1, int(round(image.height * scale)))),
        Image.Resampling.LANCZOS,
    )
    left = max(0, (resized.width - size[0]) // 2)
    top = max(0, (resized.height - size[1]) // 2)
    return resized.crop((left, top, left + size[0], top + size[1]))


def fit_visible_to_canvas(image: Image.Image, canvas_size: int = 256, padding: int = 18) -> Image.Image:
    image = crop_to_alpha(image.convert("RGBA"))
    inner_size = max(1, canvas_size - (padding * 2))
    scale = min(inner_size / image.width, inner_size / image.height)
    resized = image.resize(
        (max(1, int(round(image.width * scale))), max(1, int(round(image.height * scale)))),
        Image.Resampling.LANCZOS,
    )
    canvas = Image.new("RGBA", (canvas_size, canvas_size), (0, 0, 0, 0))
    offset = ((canvas_size - resized.width) // 2, (canvas_size - resized.height) // 2)
    canvas.alpha_composite(resized, offset)
    return canvas


def fit_crop_center_band(image: Image.Image, size: tuple[int, int], horizontal_trim_ratio: float = 0.24) -> Image.Image:
    image = crop_to_alpha(image.convert("RGBA"))
    trim = int(image.width * horizontal_trim_ratio)
    trim = max(0, min(trim, (image.width // 2) - 1))
    if trim > 0:
        image = image.crop((trim, 0, image.width - trim, image.height))
    return fit_crop(image, size)


def apply_polygon_mask(image: Image.Image, points: list[tuple[int, int]]) -> Image.Image:
    image = image.convert("RGBA")
    mask = Image.new("L", image.size, 0)
    ImageDraw.Draw(mask).polygon(points, fill=255)
    image.putalpha(ImageChops.multiply(image.getchannel("A"), mask))
    return image


def add_horizontal_fade_line(image: Image.Image, box: tuple[int, int, int, int], color: tuple[int, int, int], alpha_top: int, alpha_bottom: int) -> None:
    overlay = Image.new("RGBA", (box[2] - box[0], box[3] - box[1]), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    height = max(1, overlay.height - 1)
    for y in range(overlay.height):
        alpha = int(alpha_top + ((alpha_bottom - alpha_top) * (y / height)))
        draw.line((0, y, overlay.width, y), fill=color + (alpha,), width=1)
    image.alpha_composite(overlay, (box[0], box[1]))


def warm_overlay(size: tuple[int, int], color: tuple[int, int, int], alpha: int) -> Image.Image:
    overlay = Image.new("RGBA", size, color + (alpha,))
    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=12))
    return overlay


def load_font_from_candidates(candidates: list[Path], size: int) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    for candidate in candidates:
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size=size)

    return ImageFont.load_default()


def load_ui_font(size: int, bold: bool = True) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    return load_font_from_candidates(
        [
            UI_FONT_BOLD if bold else UI_FONT_REGULAR,
            Path("C:/Windows/Fonts/georgiab.ttf") if bold else Path("C:/Windows/Fonts/georgia.ttf"),
            Path("C:/Windows/Fonts/arialbd.ttf") if bold else Path("C:/Windows/Fonts/arial.ttf"),
        ],
        size,
    )


def load_cjk_font(size: int, bold: bool = True) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    return load_font_from_candidates(
        [
            Path("C:/Windows/Fonts/msyhbd.ttc") if bold else Path("C:/Windows/Fonts/msyh.ttc"),
            Path("C:/Windows/Fonts/YuGothB.ttc") if bold else Path("C:/Windows/Fonts/YuGothR.ttc"),
            Path("C:/Windows/Fonts/simsunb.ttf") if bold else Path("C:/Windows/Fonts/simsun.ttc"),
            Path("C:/Windows/Fonts/seguisym.ttf"),
        ],
        size,
    )


def load_icon_base(path: Path) -> Image.Image | None:
    if not path.exists():
        return None

    return crop_to_alpha(Image.open(path).convert("RGBA"))


def draw_centered_text(
    image: Image.Image,
    box: tuple[int, int, int, int],
    text: str,
    font: ImageFont.FreeTypeFont | ImageFont.ImageFont,
    fill: tuple[int, int, int, int],
    *,
    shadow_fill: tuple[int, int, int, int] | None = None,
    shadow_offset: tuple[int, int] = (0, 0),
    stroke_fill: tuple[int, int, int, int] | None = None,
    stroke_width: int = 0,
) -> None:
    draw = ImageDraw.Draw(image)
    bbox = draw.textbbox((0, 0), text, font=font, stroke_width=stroke_width)
    x = box[0] + ((box[2] - box[0] - (bbox[2] - bbox[0])) / 2.0) - bbox[0]
    y = box[1] + ((box[3] - box[1] - (bbox[3] - bbox[1])) / 2.0) - bbox[1]

    if shadow_fill is not None:
        draw.text(
            (x + shadow_offset[0], y + shadow_offset[1]),
            text,
            font=font,
            fill=shadow_fill,
            stroke_width=stroke_width,
            stroke_fill=stroke_fill,
        )

    draw.text(
        (x, y),
        text,
        font=font,
        fill=fill,
        stroke_width=stroke_width,
        stroke_fill=stroke_fill,
    )


def draw_centered_ellipse_glow(
    image: Image.Image,
    box: tuple[int, int, int, int],
    color: tuple[int, int, int],
    alpha: int,
) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    draw.ellipse(box, fill=color + (alpha,))
    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=10.0))
    image.alpha_composite(overlay)


def draw_centered_text_pair(
    image: Image.Image,
    box: tuple[int, int, int, int],
    left_text: str,
    right_text: str,
    font: ImageFont.FreeTypeFont | ImageFont.ImageFont,
    fill: tuple[int, int, int, int],
    *,
    shadow_fill: tuple[int, int, int, int] | None = None,
    shadow_offset: tuple[int, int] = (0, 0),
    stroke_fill: tuple[int, int, int, int] | None = None,
    stroke_width: int = 0,
    tracking: int = 0,
) -> None:
    draw = ImageDraw.Draw(image)
    left_bbox = draw.textbbox((0, 0), left_text, font=font, stroke_width=stroke_width)
    right_bbox = draw.textbbox((0, 0), right_text, font=font, stroke_width=stroke_width)

    left_width = left_bbox[2] - left_bbox[0]
    right_width = right_bbox[2] - right_bbox[0]
    total_width = left_width + tracking + right_width
    start_x = box[0] + ((box[2] - box[0] - total_width) / 2.0)

    left_box = (int(start_x), box[1], int(start_x + left_width), box[3])
    right_box = (int(start_x + left_width + tracking), box[1], int(start_x + total_width), box[3])

    draw_centered_text(
        image,
        left_box,
        left_text,
        font,
        fill,
        shadow_fill=shadow_fill,
        shadow_offset=shadow_offset,
        stroke_fill=stroke_fill,
        stroke_width=stroke_width,
    )
    draw_centered_text(
        image,
        right_box,
        right_text,
        font,
        fill,
        shadow_fill=shadow_fill,
        shadow_offset=shadow_offset,
        stroke_fill=stroke_fill,
        stroke_width=stroke_width,
    )


def add_diagonal_panel_overlay(
    image: Image.Image,
    box: tuple[int, int, int, int],
    color: tuple[int, int, int],
    alpha: int,
    *,
    inset: int = 18,
) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    left, top, right, bottom = box
    draw.polygon(
        [
            (left + inset, top),
            (right, top),
            (right - inset, bottom),
            (left, bottom),
        ],
        fill=color + (alpha,),
    )
    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=0.6))
    image.alpha_composite(overlay)


def build_recessed_slot_texture(size: tuple[int, int], *, warm: bool = False) -> Image.Image:
    w, h = size
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    overlay = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    outer_fill = (42, 50, 58, 255) if not warm else (69, 41, 28, 255)
    cavity_fill = (24, 29, 35, 244) if not warm else (37, 20, 14, 244)
    cavity_line = (78, 89, 99, 176) if not warm else (139, 86, 52, 186)
    top_highlight = (228, 233, 238, 54) if not warm else (244, 196, 138, 70)
    base_shadow = (8, 11, 14, 188) if not warm else (33, 13, 8, 204)
    rim_glow = (150, 160, 170) if not warm else (230, 143, 76)

    outer = [
        (14, 0),
        (w - 14, 0),
        (w, 14),
        (w, h - 14),
        (w - 14, h),
        (14, h),
        (0, h - 14),
        (0, 14),
    ]
    cavity = [
        (18, 10),
        (w - 18, 10),
        (w - 10, 18),
        (w - 10, h - 18),
        (w - 18, h - 10),
        (18, h - 10),
        (10, h - 18),
        (10, 18),
    ]

    draw.polygon(outer, fill=outer_fill, outline=base_shadow)
    draw.polygon(cavity, fill=cavity_fill, outline=cavity_line)
    draw.line((18, 8, w - 18, 8), fill=top_highlight, width=2)
    draw.line((16, h - 8, w - 16, h - 8), fill=base_shadow, width=1)

    if warm:
        overlay.alpha_composite(warm_overlay((w - 12, h - 12), rim_glow, 22), (6, 5))
    else:
        overlay.alpha_composite(warm_overlay((w - 14, h - 14), rim_glow, 10), (7, 7))

    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=0.4))
    canvas.alpha_composite(make_shadow(overlay, (0, 0, 0, 170), 6.0), (0, 5))
    canvas.alpha_composite(overlay)
    return canvas


def build_opaque_bar_base(width: int, height: int) -> Image.Image:
    base = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(base)
    top = (29, 39, 47)
    mid = (38, 50, 60)
    bottom = (22, 29, 35)
    for y in range(height):
        if y < height * 0.55:
            t = y / max(1, int(height * 0.55) - 1)
            color = tuple(int(top[i] + (mid[i] - top[i]) * t) for i in range(3))
        else:
            t = (y - height * 0.55) / max(1, height - height * 0.55 - 1)
            color = tuple(int(mid[i] + (bottom[i] - mid[i]) * t) for i in range(3))
        draw.line((0, y, width, y), fill=color + (255,), width=1)

    add_diagonal_panel_overlay(base, (58, 0, 254, height), (16, 21, 26), 58, inset=34)
    add_diagonal_panel_overlay(base, (294, 0, 624, height), (10, 14, 18), 34, inset=40)
    add_diagonal_panel_overlay(base, (662, 0, 978, height), (16, 21, 26), 26, inset=36)
    add_diagonal_panel_overlay(base, (1012, 0, 1326, height), (10, 14, 18), 30, inset=34)
    add_diagonal_panel_overlay(base, (1368, 0, 1668, height), (16, 21, 26), 48, inset=36)

    vignette = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    vdraw = ImageDraw.Draw(vignette)
    edge_alpha = 26
    center_alpha = 0
    for x in range(width):
        distance = abs((x / max(1, width - 1)) - 0.5) * 2.0
        alpha = int(center_alpha + (edge_alpha - center_alpha) * (distance ** 1.6))
        vdraw.line((x, 0, x, height), fill=(0, 0, 0, alpha), width=1)
    base.alpha_composite(vignette)
    draw.line((0, 0, width, 0), fill=(71, 101, 124, 186), width=2)
    draw.line((0, height - 2, width, height - 2), fill=(113, 65, 40, 164), width=2)
    return base


def add_side_rib_cluster(image: Image.Image, x: int, y: int, h: int, warm: bool = False) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    steel = (191, 198, 202) if not warm else (213, 168, 111)
    bronze = (157, 111, 63) if not warm else (203, 129, 67)
    shadow = (15, 18, 22)

    for offset, width, color, alpha in (
        (0, 4, steel, 124),
        (8, 3, shadow, 178),
        (15, 2, bronze, 150),
    ):
        draw.rectangle((x + offset, y, x + offset + width, y + h), fill=color + (alpha,))

    glow = warm_overlay((34, h + 10), (216, 138, 72), 18 if not warm else 34)
    overlay.alpha_composite(glow, (x - 6, max(0, y - 3)))
    image.alpha_composite(overlay)


def build_fill_strip_texture(width: int, height: int) -> Image.Image:
    metallic_overlay = fit_crop_center_band(Image.open(NEUTRAL_PLATE).convert("RGBA"), (width, height), 0.24)
    metallic_alpha = metallic_overlay.getchannel("A").point(lambda value: int(value * 0.62))
    metallic_overlay.putalpha(metallic_alpha)
    shadow = make_shadow(metallic_overlay, (0, 0, 0, 110), 8.0)
    canvas = Image.new("RGBA", (width, height + 20), (0, 0, 0, 0))
    canvas.alpha_composite(build_opaque_bar_base(width, height), (0, 0))
    canvas.alpha_composite(shadow, (0, 10))
    canvas.alpha_composite(metallic_overlay, (0, 0))
    canvas.alpha_composite(warm_overlay((width - 96, 18), (132, 150, 168), 8), (48, height - 9))
    return canvas


def add_recess_housing(image: Image.Image, box: tuple[int, int, int, int], warm: bool = False) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    outer_fill = (26, 31, 37, 228) if not warm else (39, 25, 20, 235)
    outer_outline = (86, 96, 106, 210) if not warm else (132, 84, 52, 215)
    inner_outline = (202, 208, 210, 78) if not warm else (243, 180, 111, 110)
    glow_color = (214, 139, 72) if warm else (167, 176, 182)
    draw.rounded_rectangle(box, radius=11, fill=outer_fill, outline=outer_outline, width=2)
    draw.rounded_rectangle((box[0] + 5, box[1] + 5, box[2] - 5, box[3] - 5), radius=8, outline=inner_outline, width=1)
    glow = warm_overlay((box[2] - box[0] + 16, box[3] - box[1] + 10), glow_color, 16 if not warm else 34)
    overlay.alpha_composite(glow, (box[0] - 8, box[1] - 2))
    image.alpha_composite(overlay)


def add_outer_stub(image: Image.Image, box: tuple[int, int, int, int], warm: bool = False) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    fill = (45, 53, 61, 228) if not warm else (72, 39, 28, 235)
    line = (195, 198, 196, 78) if not warm else (235, 163, 92, 102)
    edge = (17, 20, 24, 210)
    draw.rounded_rectangle(box, radius=4, fill=fill, outline=edge, width=1)
    draw.line((box[0] + 4, box[1] + 3, box[2] - 4, box[1] + 3), fill=line, width=1)
    image.alpha_composite(overlay)


def add_embedded_end_well(image: Image.Image, box: tuple[int, int, int, int], warm: bool = False) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    fill = (20, 25, 30, 210) if not warm else (37, 23, 19, 218)
    outer = (92, 102, 112, 168) if not warm else (164, 102, 62, 178)
    inner = (214, 220, 222, 60) if not warm else (242, 188, 122, 84)
    line = (175, 126, 73, 88) if not warm else (225, 141, 73, 116)
    draw.rounded_rectangle(box, radius=9, fill=fill, outline=outer, width=2)
    draw.rounded_rectangle((box[0] + 4, box[1] + 4, box[2] - 4, box[3] - 4), radius=7, outline=inner, width=1)
    draw.line((box[0] + 8, box[3] - 8, box[2] - 8, box[3] - 8), fill=line, width=1)
    if warm:
        overlay.alpha_composite(warm_overlay((box[2] - box[0] + 18, box[3] - box[1] + 10), (232, 146, 77), 20), (box[0] - 9, box[1] - 3))
    image.alpha_composite(overlay)


def add_end_anchor_shadow(image: Image.Image, box: tuple[int, int, int, int], warm: bool = False) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    fill = (0, 0, 0, 58) if not warm else (52, 18, 8, 66)
    draw.rounded_rectangle(box, radius=12, fill=fill)
    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=9))
    image.alpha_composite(overlay)


def build_settings_slot_texture(size: tuple[int, int]) -> Image.Image:
    return build_recessed_slot_texture(size, warm=False)


def build_utility_slot_texture(size: tuple[int, int]) -> Image.Image:
    return build_recessed_slot_texture(size, warm=False)


def build_quit_slot_texture(size: tuple[int, int]) -> Image.Image:
    return build_recessed_slot_texture(size, warm=True)


def build_nav_tab_texture(size: tuple[int, int], *, active: bool = False) -> Image.Image:
    w, h = size
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    overlay = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    outer = [(18, 4), (w - 18, 4), (w - 4, h - 12), (4, h - 12)]
    middle = [(24, 9), (w - 24, 9), (w - 10, h - 18), (10, h - 18)]
    inner = [(30, 13), (w - 30, 13), (w - 16, h - 23), (16, h - 23)]

    if active:
        outer_fill = (53, 58, 66, 252)
        outer_line = (122, 132, 144, 196)
        middle_fill = (18, 22, 28, 252)
        middle_line = (109, 118, 127, 210)
        inner_fill = (39, 44, 50, 250)
        top_highlight = (236, 241, 246, 64)
        bottom_accent = (132, 150, 168, 132)
        glow_alpha = 14
    else:
        outer_fill = (45, 53, 61, 238)
        outer_line = (88, 100, 113, 160)
        middle_fill = (15, 20, 25, 242)
        middle_line = (66, 76, 86, 170)
        inner_fill = (27, 33, 40, 245)
        top_highlight = (236, 241, 246, 34)
        bottom_accent = (108, 124, 140, 58)
        glow_alpha = 6

    draw.polygon(outer, fill=outer_fill, outline=(12, 15, 18, 214))
    draw.polygon(middle, fill=middle_fill, outline=outer_line)
    draw.polygon(inner, fill=inner_fill, outline=middle_line)
    draw.line((30, 12, w - 30, 12), fill=top_highlight, width=2)
    draw.line((26, h - 16, w - 26, h - 16), fill=bottom_accent, width=2)
    draw.line((18, 18, 26, h - 22), fill=(0, 0, 0, 86), width=1)
    draw.line((w - 26, 18, w - 18, h - 22), fill=(0, 0, 0, 86), width=1)
    overlay.alpha_composite(warm_overlay((w - 48, h - 28), (124, 146, 170), glow_alpha), (24, 12))

    canvas.alpha_composite(make_shadow(overlay, (0, 0, 0, 150), 6.0), (0, 4))
    canvas.alpha_composite(overlay)
    return canvas


def build_inactive_tab_texture(size: tuple[int, int]) -> Image.Image:
    return build_nav_tab_texture(size, active=False)


def build_active_tab_texture(size: tuple[int, int]) -> Image.Image:
    return build_nav_tab_texture(size, active=True)


def build_separator_texture(size: tuple[int, int]) -> Image.Image:
    w, h = size
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    draw.line((4, h - 3, w - 8, 7), fill=(33, 38, 45, 255), width=2)
    draw.line((10, h - 4, w - 3, 9), fill=(125, 105, 73, 160), width=1)
    draw.line((8, h - 5, w - 5, 6), fill=(194, 173, 132, 44), width=1)
    return canvas.filter(ImageFilter.GaussianBlur(radius=0.35))


def build_left_cap_texture(width: int, height: int) -> Image.Image:
    canvas = build_fill_strip_texture(width, height)
    add_embedded_end_well(canvas, (16, 10, 104, 76), warm=False)
    add_outer_stub(canvas, (12, 37, 54, 53), warm=False)
    add_side_rib_cluster(canvas, width - 56, 6, height - 10, warm=False)
    add_horizontal_fade_line(canvas, (10, height - 5, width - 34, height - 2), (160, 113, 64), 44, 8)
    return canvas


def build_right_cap_texture(width: int, height: int) -> Image.Image:
    canvas = build_fill_strip_texture(width, height)
    add_embedded_end_well(canvas, (width - 104, 10, width - 16, 76), warm=True)
    add_outer_stub(canvas, (width - 54, 37, width - 14, 53), warm=True)
    add_side_rib_cluster(canvas, 34, 6, height - 10, warm=True)
    canvas.alpha_composite(warm_overlay((104, 60), (234, 148, 77), 20), (width - 128, 10))
    add_horizontal_fade_line(canvas, (34, height - 5, width - 8, height - 2), (196, 121, 63), 58, 14)
    return canvas


def build_topbar_plate_texture(width: int, height: int) -> Image.Image:
    canvas = build_fill_strip_texture(width, height)
    add_horizontal_fade_line(canvas, (52, 7, width - 52, 10), (220, 228, 234), 36, 0)
    return canvas


def make_topbar_plate() -> None:
    plate = build_topbar_plate_texture(1680, 88)
    plate.save(TOPBAR_OUT)

    fill_slice_x = (plate.width // 2) - 32
    plate.crop((fill_slice_x, 0, fill_slice_x + 64, plate.height)).save(TOPBAR_FILL_OUT)
    plate.crop((0, 0, 300, plate.height)).save(TOPBAR_LEFT_OUT)
    plate.crop((plate.width - 300, 0, plate.width, plate.height)).save(TOPBAR_RIGHT_OUT)

    build_settings_slot_texture((78, 78)).save(SETTINGS_SLOT_OUT)
    build_utility_slot_texture((78, 78)).save(UTILITY_SLOT_OUT)
    build_quit_slot_texture((78, 78)).save(QUIT_SLOT_OUT)
    build_inactive_tab_texture((210, 76)).save(NAV_TAB_OUT)
    build_active_tab_texture((226, 82)).save(ACTIVE_TAB_OUT)
    build_separator_texture((26, 68)).save(SEPARATOR_OUT)


def draw_rounded_stroke(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], fill: tuple[int, int, int, int], width: int) -> None:
    draw.rounded_rectangle(box, radius=(box[2] - box[0]) // 2, outline=fill, width=width)


def build_star_points(center_x: int, center_y: int, outer_radius: int, inner_radius: int, tips: int = 8) -> list[tuple[float, float]]:
    from math import cos, pi, sin

    points: list[tuple[float, float]] = []
    for index in range(tips * 2):
        angle = (-pi / 2.0) + ((pi / tips) * index)
        radius = outer_radius if index % 2 == 0 else inner_radius
        points.append((center_x + (cos(angle) * radius), center_y + (sin(angle) * radius)))
    return points


def make_power_symbol(size: int = 256) -> Image.Image:
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sdraw = ImageDraw.Draw(shadow)
    sdraw.ellipse((30, 30, size - 30, size - 30), fill=(0, 0, 0, 110))
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=12.0))
    canvas.alpha_composite(shadow, (0, 10))

    ring = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(ring)

    outer_dark = (24, 16, 13, 255)
    outer_mid = (78, 51, 35, 255)
    warm_dark = (231, 124, 47, 255)
    warm_light = (255, 226, 168, 255)
    ivory = (254, 247, 232, 255)

    draw.ellipse((38, 38, size - 38, size - 38), fill=outer_dark)
    draw.ellipse((52, 52, size - 52, size - 52), outline=outer_mid, width=10)
    draw.arc((56, 56, size - 56, size - 56), start=42, end=318, fill=warm_dark, width=24)
    draw.arc((62, 62, size - 62, size - 62), start=42, end=318, fill=warm_light, width=10)
    draw.arc((60, 60, size - 60, size - 60), start=42, end=318, fill=ivory, width=5)

    draw.rounded_rectangle((size // 2 - 18, 24, size // 2 + 18, 132), radius=16, fill=outer_dark)
    draw.rounded_rectangle((size // 2 - 10, 28, size // 2 + 10, 124), radius=10, fill=warm_light)
    draw.rounded_rectangle((size // 2 - 6, 34, size // 2 + 6, 116), radius=6, fill=ivory)

    ring = ring.filter(ImageFilter.GaussianBlur(radius=0.35))
    canvas.alpha_composite(ring)

    sheen = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sdraw = ImageDraw.Draw(sheen)
    sdraw.arc((60, 60, size - 60, size - 60), start=210, end=300, fill=(255, 244, 220, 84), width=10)
    sdraw.rounded_rectangle((size // 2 - 10, 34, size // 2 - 2, 86), radius=5, fill=(255, 248, 234, 68))
    sheen = sheen.filter(ImageFilter.GaussianBlur(radius=1.8))
    canvas.alpha_composite(sheen)

    return canvas


def make_settings_gear(size: int = 256) -> Image.Image:
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sdraw = ImageDraw.Draw(shadow)
    sdraw.ellipse((42, 42, size - 42, size - 42), fill=(0, 0, 0, 82))
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=10.0))
    canvas.alpha_composite(shadow, (0, 8))

    ring = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(ring)
    center = size // 2
    tooth_outer = 116
    tooth_inner = 80

    steel_dark = (43, 50, 58, 255)
    steel_mid = (93, 105, 118, 255)
    steel_light = (216, 223, 230, 255)
    steel_soft = (167, 177, 187, 255)
    core_dark = (26, 31, 36, 255)

    for angle in range(0, 360, 45):
        tooth = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        tdraw = ImageDraw.Draw(tooth)
        tdraw.rounded_rectangle((center - 20, center - tooth_outer, center + 20, center - tooth_inner), radius=10, fill=steel_mid)
        tooth = tooth.rotate(angle, resample=Image.Resampling.BICUBIC, center=(center, center))
        ring.alpha_composite(tooth)

    draw.ellipse((48, 48, size - 48, size - 48), fill=steel_dark)
    draw.ellipse((66, 66, size - 66, size - 66), outline=steel_light, width=18)
    draw.ellipse((84, 84, size - 84, size - 84), outline=steel_soft, width=7)
    draw.ellipse((98, 98, size - 98, size - 98), fill=core_dark)
    draw.ellipse((112, 112, size - 112, size - 112), fill=steel_light)
    draw.ellipse((122, 122, size - 122, size - 122), fill=steel_dark)

    sheen = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sdraw = ImageDraw.Draw(sheen)
    sdraw.arc((60, 60, size - 60, size - 60), start=210, end=300, fill=(248, 252, 255, 82), width=10)
    sheen = sheen.filter(ImageFilter.GaussianBlur(radius=2.0))
    ring.alpha_composite(sheen)
    canvas.alpha_composite(ring)
    return canvas


def make_language_icon(size: int = 256) -> Image.Image:
    base = load_icon_base(AI_LANGUAGE_BASE)
    if base is not None:
        canvas = fit_visible_to_canvas(base, size, 10)
        shadow = make_shadow(canvas, (0, 0, 0, 110), 8.0)
        framed = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        framed.alpha_composite(shadow, (0, 6))
        framed.alpha_composite(canvas)
        return framed

    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sdraw = ImageDraw.Draw(shadow)
    sdraw.rounded_rectangle((34, 78, 122, 186), radius=18, fill=(0, 0, 0, 84))
    sdraw.rounded_rectangle((132, 72, 222, 180), radius=18, fill=(0, 0, 0, 76))
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=9.0))
    canvas.alpha_composite(shadow, (0, 8))

    icon = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(icon)

    left = [(42, 86), (118, 86), (128, 96), (128, 168), (118, 178), (42, 178), (32, 168), (32, 96)]
    right = [(140, 78), (216, 78), (226, 88), (226, 160), (216, 170), (140, 170), (130, 160), (130, 88)]
    arrow_top = [(94, 108), (150, 108), (150, 96), (184, 128), (150, 160), (150, 148), (94, 148)]
    arrow_bottom = [(154, 114), (98, 114), (98, 102), (64, 134), (98, 166), (98, 154), (154, 154)]

    draw.polygon(left, fill=(46, 60, 76, 255), outline=(114, 129, 143, 220))
    draw.polygon(right, fill=(236, 229, 210, 255), outline=(209, 192, 154, 210))
    draw.line((48, 94, 112, 94), fill=(237, 243, 248, 60), width=2)
    draw.line((146, 86, 210, 86), fill=(255, 250, 235, 72), width=2)
    draw.polygon(arrow_top, fill=(73, 95, 120, 238), outline=(29, 38, 48, 170))
    draw.polygon(arrow_bottom, fill=(225, 209, 168, 236), outline=(126, 99, 57, 150))

    latin_font = load_ui_font(int(size * 0.30), bold=True)
    cjk_font = load_cjk_font(int(size * 0.28), bold=True)
    draw_centered_text(
        icon,
        (42, 96, 110, 168),
        "A",
        latin_font,
        (246, 244, 236, 255),
        shadow_fill=(12, 16, 22, 180),
        shadow_offset=(0, 2),
        stroke_fill=(36, 51, 66, 140),
        stroke_width=max(1, size // 96),
    )
    draw_centered_text(
        icon,
        (150, 88, 214, 160),
        "文",
        cjk_font,
        (70, 67, 58, 255),
        shadow_fill=(255, 247, 225, 80),
        shadow_offset=(0, -1),
        stroke_fill=(176, 150, 111, 110),
        stroke_width=max(1, size // 112),
    )

    icon = icon.filter(ImageFilter.GaussianBlur(radius=0.2))
    canvas.alpha_composite(icon)
    return canvas


def make_achievement_coin_icon(size: int = 256) -> Image.Image:
    base = load_icon_base(AI_ACHIEVEMENT_BASE)
    if base is None:
        base = Image.new("RGBA", (size, size), (0, 0, 0, 0))

    canvas = fit_visible_to_canvas(base, size, 8)
    draw_centered_ellipse_glow(
        canvas,
        (int(size * 0.28), int(size * 0.30), int(size * 0.72), int(size * 0.74)),
        (36, 21, 12),
        66,
    )
    font = load_ui_font(int(size * 0.24), bold=True)
    draw_centered_text_pair(
        canvas,
        (int(size * 0.24), int(size * 0.28), int(size * 0.76), int(size * 0.74)),
        "A",
        "C",
        font,
        (247, 238, 222, 255),
        shadow_fill=(40, 22, 11, 215),
        shadow_offset=(0, max(2, size // 56)),
        stroke_fill=(110, 63, 28, 156),
        stroke_width=max(1, size // 96),
        tracking=max(6, size // 40),
    )
    return canvas


def make_power_coupon_icon(size: int = 256) -> Image.Image:
    base = load_icon_base(AI_COUPON_BASE)
    if base is None:
        base = Image.new("RGBA", (size, size), (0, 0, 0, 0))

    canvas = fit_visible_to_canvas(base, size, 8)
    draw_centered_ellipse_glow(
        canvas,
        (int(size * 0.23), int(size * 0.34), int(size * 0.77), int(size * 0.66)),
        (64, 29, 10),
        48,
    )
    font = load_ui_font(int(size * 0.22), bold=True)
    draw_centered_text_pair(
        canvas,
        (int(size * 0.24), int(size * 0.32), int(size * 0.76), int(size * 0.67)),
        "P",
        "C",
        font,
        (247, 236, 214, 255),
        shadow_fill=(58, 27, 11, 205),
        shadow_offset=(0, max(2, size // 56)),
        stroke_fill=(129, 69, 31, 152),
        stroke_width=max(1, size // 96),
        tracking=max(8, size // 30),
    )
    return canvas


def make_quit_glow(size: int = 256) -> Image.Image:
    base = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(base)

    ember = ImageColor.getrgb("#ff8e3d")
    hot = ImageColor.getrgb("#ffd38e")
    deep = ImageColor.getrgb("#c73a1c")

    draw.ellipse((42, 42, size - 42, size - 42), fill=deep + (34,))
    draw.arc((48, 48, size - 48, size - 48), start=34, end=326, fill=ember + (248,), width=28)
    draw.arc((56, 56, size - 56, size - 56), start=34, end=326, fill=hot + (252,), width=14)
    draw.rounded_rectangle((size // 2 - 14, 28, size // 2 + 14, 126), radius=13, fill=hot + (228,))
    draw.rounded_rectangle((size // 2 - 8, 36, size // 2 + 8, 118), radius=8, fill=ember + (248,))

    blur_large = base.filter(ImageFilter.GaussianBlur(radius=16))
    blur_small = base.filter(ImageFilter.GaussianBlur(radius=7))

    glow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    glow.alpha_composite(tint_image(blur_large, ember, 0.78))
    glow.alpha_composite(tint_image(blur_small, hot, 0.84))

    hot_ring = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    hdraw = ImageDraw.Draw(hot_ring)
    hdraw.arc((64, 64, size - 64, size - 64), start=34, end=326, fill=hot + (208,), width=12)
    hdraw.rounded_rectangle((size // 2 - 9, 40, size // 2 + 9, 112), radius=8, fill=hot + (176,))
    hot_ring = hot_ring.filter(ImageFilter.GaussianBlur(radius=4.0))
    glow.alpha_composite(hot_ring)

    return glow


def save_icon_preview_sheet(icons: list[tuple[str, Image.Image]]) -> None:
    canvas = Image.new("RGBA", (1200, 300), (18, 22, 29, 255))
    draw = ImageDraw.Draw(canvas)
    label_font = load_ui_font(18, bold=False)

    for index, (label, icon) in enumerate(icons):
        x = 40 + (index * 232)
        y = 32
        fitted = fit_visible_to_canvas(icon, 180, 16)
        canvas.alpha_composite(fitted, (x, y))
        draw_centered_text(canvas, (x, 224, x + 180, 266), label, label_font, (235, 238, 242, 255))

    canvas.save(ICON_PREVIEW_OUT)


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    make_topbar_plate()
    settings_icon = make_settings_gear(256)
    language_icon = make_language_icon(256)
    achievement_icon = make_achievement_coin_icon(256)
    coupon_icon = make_power_coupon_icon(256)
    quit_icon = make_power_symbol(256)
    quit_glow = make_quit_glow(256)

    fit_visible_to_canvas(settings_icon, 256, 14).save(SETTINGS_ICON_OUT)
    fit_visible_to_canvas(language_icon, 256, 8).save(LANGUAGE_ICON_OUT)
    fit_visible_to_canvas(achievement_icon, 256, 8).save(ACHIEVEMENT_COINS_ICON_OUT)
    fit_visible_to_canvas(coupon_icon, 256, 8).save(POWER_COUPONS_ICON_OUT)
    fit_visible_to_canvas(quit_icon, 256, 14).save(QUIT_ICON_OUT)
    fit_visible_to_canvas(quit_glow, 256, 10).save(QUIT_GLOW_OUT)
    save_icon_preview_sheet(
        [
            ("settings", settings_icon),
            ("language", language_icon),
            ("achievement_coins", achievement_icon),
            ("power_coupons", coupon_icon),
            ("quit", quit_icon),
        ]
    )
    print(f"Wrote {TOPBAR_OUT}")
    print(f"Wrote {TOPBAR_LEFT_OUT}")
    print(f"Wrote {TOPBAR_FILL_OUT}")
    print(f"Wrote {TOPBAR_RIGHT_OUT}")
    print(f"Wrote {SETTINGS_SLOT_OUT}")
    print(f"Wrote {UTILITY_SLOT_OUT}")
    print(f"Wrote {QUIT_SLOT_OUT}")
    print(f"Wrote {NAV_TAB_OUT}")
    print(f"Wrote {ACTIVE_TAB_OUT}")
    print(f"Wrote {SEPARATOR_OUT}")
    print(f"Wrote {SETTINGS_ICON_OUT}")
    print(f"Wrote {LANGUAGE_ICON_OUT}")
    print(f"Wrote {ACHIEVEMENT_COINS_ICON_OUT}")
    print(f"Wrote {POWER_COUPONS_ICON_OUT}")
    print(f"Wrote {QUIT_ICON_OUT}")
    print(f"Wrote {QUIT_GLOW_OUT}")
    print(f"Wrote {ICON_PREVIEW_OUT}")


if __name__ == "__main__":
    main()
