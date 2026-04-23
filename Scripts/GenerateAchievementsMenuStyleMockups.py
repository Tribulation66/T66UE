from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "output" / "imagegen"
OUT_DIR.mkdir(parents=True, exist_ok=True)

CANVAS_SIZE = (1672, 941)
TOPBAR_HEIGHT = 126


def load_rgba(path: Path) -> Image.Image:
    return Image.open(path).convert("RGBA")


ASSET_ROOT = ROOT / "SourceAssets" / "UI" / "MainMenuReference"
TOPBAR = load_rgba(ASSET_ROOT / "TopBar" / "topbar_strip_full.png")
REFERENCE_MENU = load_rgba(ROOT / "SourceAssets" / "Reference Main Menu.png")
PANEL_TILE = load_rgba(ASSET_ROOT / "Tiles" / "panel_fill_dark.png")
WARM_TILE = load_rgba(ASSET_ROOT / "Tiles" / "topbar_fill_warm.png")
ACTIVE_SUBTAB = load_rgba(ASSET_ROOT / "RightPanel" / "tab_weekly_active.png")
INACTIVE_SUBTAB = load_rgba(ASSET_ROOT / "RightPanel" / "tab_all_time_inactive.png")
BACK_BUTTON_SHELL = load_rgba(ASSET_ROOT / "LeftPanel" / "search_field_shell.png")
STAR_BADGE = load_rgba(ASSET_ROOT / "LeftPanel" / "friend_star_button.png")
GREEN_FILL = load_rgba(ROOT / "RuntimeDependencies" / "T66" / "UI" / "MiniMainMenu" / "mini_mainmenu_cta_fill_green.png")

FONT_REAVER = str(ROOT / "RuntimeDependencies" / "T66" / "Fonts" / "Reaver-Bold.woff")
FONT_RADIANCE = str(ROOT / "RuntimeDependencies" / "T66" / "Fonts" / "radiance.ttf")


CREAM = (241, 233, 220, 255)
MUTED = (182, 170, 149, 255)
GOLD = (218, 189, 126, 255)
GOLD_DARK = (91, 69, 41, 255)
COOL_LINE = (132, 136, 145, 255)
PANEL_BASE = (14, 16, 19, 255)
CARD_BASE = (8, 9, 11, 255)
TRACK_FILL = (7, 8, 10, 255)
SCRIM = (5, 6, 8, 220)


def font(path: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(path, size)


def text_size(ft: ImageFont.FreeTypeFont, text: str) -> tuple[int, int]:
    bbox = ft.getbbox(text)
    return bbox[2] - bbox[0], bbox[3] - bbox[1]


def tile_image(tile: Image.Image, size: tuple[int, int]) -> Image.Image:
    out = Image.new("RGBA", size, (0, 0, 0, 0))
    for y in range(0, size[1], tile.height):
        for x in range(0, size[0], tile.width):
            out.alpha_composite(tile, (x, y))
    return out


def apply_alpha(image: Image.Image, alpha: int) -> Image.Image:
    result = image.copy()
    result.putalpha(alpha)
    return result


def shadowed_text(
    draw: ImageDraw.ImageDraw,
    xy: tuple[float, float],
    text: str,
    ft: ImageFont.FreeTypeFont,
    fill: tuple[int, int, int, int],
    *,
    anchor: str = "lt",
    shadow: tuple[int, int] = (0, 2),
    shadow_fill: tuple[int, int, int, int] = (12, 9, 7, 180),
    stroke_width: int = 0,
    stroke_fill: tuple[int, int, int, int] | None = None,
) -> None:
    draw.text(
        (xy[0] + shadow[0], xy[1] + shadow[1]),
        text,
        font=ft,
        fill=shadow_fill,
        anchor=anchor,
        stroke_width=stroke_width,
        stroke_fill=stroke_fill,
    )
    draw.text(
        xy,
        text,
        font=ft,
        fill=fill,
        anchor=anchor,
        stroke_width=stroke_width,
        stroke_fill=stroke_fill,
    )


def create_background() -> Image.Image:
    canvas = Image.new("RGBA", CANVAS_SIZE, (7, 8, 11, 255))

    lower = REFERENCE_MENU.crop((0, TOPBAR_HEIGHT, CANVAS_SIZE[0], CANVAS_SIZE[1])).convert("RGBA")
    lower = lower.resize((CANVAS_SIZE[0], CANVAS_SIZE[1] - TOPBAR_HEIGHT), Image.Resampling.LANCZOS)
    lower = lower.filter(ImageFilter.GaussianBlur(16))
    lower = ImageEnhance.Brightness(lower).enhance(0.33)
    lower = ImageEnhance.Color(lower).enhance(0.55)
    canvas.alpha_composite(lower, (0, TOPBAR_HEIGHT))

    texture = tile_image(PANEL_TILE, CANVAS_SIZE)
    texture = ImageEnhance.Brightness(texture).enhance(0.42)
    canvas = Image.blend(canvas, texture, 0.12)

    glow = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_draw.ellipse((480, 150, 1200, 760), fill=(188, 150, 88, 38))
    glow_draw.ellipse((590, 180, 1090, 620), fill=(135, 164, 118, 28))
    glow = glow.filter(ImageFilter.GaussianBlur(70))
    canvas.alpha_composite(glow)

    vignette = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    vignette_draw = ImageDraw.Draw(vignette)
    vignette_draw.rectangle((0, 0, CANVAS_SIZE[0], CANVAS_SIZE[1]), fill=(0, 0, 0, 110))
    vignette_draw.rectangle((90, 70, CANVAS_SIZE[0] - 90, CANVAS_SIZE[1] - 70), fill=(0, 0, 0, 0))
    vignette = vignette.filter(ImageFilter.GaussianBlur(60))
    canvas.alpha_composite(vignette)

    under_topbar = Image.new("RGBA", (CANVAS_SIZE[0], CANVAS_SIZE[1] - TOPBAR_HEIGHT), SCRIM)
    canvas.alpha_composite(under_topbar, (0, TOPBAR_HEIGHT))
    canvas.alpha_composite(TOPBAR, (0, 0))
    return canvas


def panel_surface(size: tuple[int, int], base: tuple[int, int, int, int], tile: Image.Image, tile_alpha: int) -> Image.Image:
    plate = Image.new("RGBA", size, base)
    tiled = tile_image(tile, size)
    tiled.putalpha(tile_alpha)
    plate.alpha_composite(tiled)

    gloss = Image.new("RGBA", size, (0, 0, 0, 0))
    gloss_draw = ImageDraw.Draw(gloss)
    for y in range(size[1]):
        alpha = int(max(0, 34 - (y * 34 / max(1, size[1] - 1))))
        gloss_draw.line((0, y, size[0], y), fill=(255, 244, 226, alpha))
    gloss = gloss.filter(ImageFilter.GaussianBlur(1))
    plate.alpha_composite(gloss)
    return plate


def make_plate(
    size: tuple[int, int],
    *,
    fill_mode: str = "dark",
    border_outer: tuple[int, int, int, int] = GOLD_DARK,
    border_inner: tuple[int, int, int, int] = GOLD,
) -> Image.Image:
    plate = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(plate)
    w, h = size

    draw.rectangle((0, 0, w - 1, h - 1), fill=border_outer)
    draw.rectangle((1, 1, w - 2, h - 2), fill=border_inner)
    draw.rectangle((3, 3, w - 4, h - 4), fill=(26, 27, 31, 255))

    if fill_mode == "green":
        fill_crop = GREEN_FILL.crop((120, 32, 904, 200)).resize((w - 6, h - 6), Image.Resampling.LANCZOS)
        inner = fill_crop
    elif fill_mode == "warm":
        inner = panel_surface((w - 6, h - 6), (114, 94, 70, 255), WARM_TILE, 95)
    else:
        inner = panel_surface((w - 6, h - 6), PANEL_BASE, PANEL_TILE, 68)

    plate.alpha_composite(inner, (3, 3))

    highlight = ImageDraw.Draw(plate)
    highlight.rectangle((4, 4, w - 5, 7), fill=(247, 233, 204, 48))
    highlight.rectangle((4, h - 8, w - 5, h - 5), fill=(0, 0, 0, 96))
    highlight.line((4, h - 4, w - 5, h - 4), fill=(86, 92, 100, 180), width=1)
    return plate


def paste_with_shadow(base: Image.Image, overlay: Image.Image, xy: tuple[int, int], blur: int = 12, alpha: int = 120) -> None:
    shadow = Image.new("RGBA", base.size, (0, 0, 0, 0))
    shadow_box = Image.new("RGBA", overlay.size, (0, 0, 0, alpha))
    shadow.alpha_composite(shadow_box, (xy[0] + 4, xy[1] + 7))
    shadow = shadow.filter(ImageFilter.GaussianBlur(blur))
    base.alpha_composite(shadow)
    base.alpha_composite(overlay, xy)


def make_subtab(label: str, *, active: bool) -> Image.Image:
    source = ACTIVE_SUBTAB if active else INACTIVE_SUBTAB
    size = (284, 82)
    tab = source.resize(size, Image.Resampling.LANCZOS)
    draw = ImageDraw.Draw(tab)
    label_font = font(FONT_REAVER, 29)
    fill = (74, 79, 58, 255) if active else CREAM
    stroke = (255, 255, 255, 34) if active else GOLD_DARK
    shadowed_text(
        draw,
        (size[0] // 2, size[1] // 2 - 2),
        label,
        label_font,
        fill,
        anchor="mm",
        shadow=(0, 2),
        shadow_fill=(10, 8, 7, 160),
        stroke_width=1,
        stroke_fill=stroke,
    )
    return tab


def make_back_button() -> Image.Image:
    size = (132, 44)
    back = BACK_BUTTON_SHELL.resize(size, Image.Resampling.LANCZOS)
    draw = ImageDraw.Draw(back)
    back_font = font(FONT_REAVER, 19)
    shadowed_text(draw, (size[0] // 2, size[1] // 2 - 1), "BACK", back_font, CREAM, anchor="mm")
    return back


def progress_panel(title: str, percent: str, *, hidden: bool = False) -> Image.Image:
    size = (1570, 95)
    panel = make_plate(size, fill_mode="dark", border_outer=(63, 50, 37, 255), border_inner=(138, 118, 90, 255))
    draw = ImageDraw.Draw(panel)
    left_font = font(FONT_REAVER, 24)
    right_font = font(FONT_REAVER, 24)
    if hidden:
        title = "???"
        percent = "???"
    shadowed_text(draw, (30, 20), title, left_font, CREAM, anchor="la")
    shadowed_text(draw, (size[0] - 30, 20), percent, right_font, CREAM, anchor="ra")

    track = make_plate((size[0] - 40, 18), fill_mode="dark", border_outer=(18, 18, 20, 255), border_inner=(52, 54, 58, 255))
    track_inner = Image.new("RGBA", (track.width - 6, track.height - 6), TRACK_FILL)
    track.alpha_composite(track_inner, (3, 3))
    panel.alpha_composite(track, (20, 58))
    return panel


def reward_badge(secret: bool = False) -> Image.Image:
    if secret:
        badge = make_plate((116, 54), fill_mode="green", border_outer=(88, 93, 70, 255), border_inner=(188, 201, 167, 255))
        draw = ImageDraw.Draw(badge)
        badge_font = font(FONT_REAVER, 20)
        shadowed_text(draw, (badge.width // 2, badge.height // 2 - 1), "???", badge_font, (210, 220, 199, 255), anchor="mm")
        return badge
    return STAR_BADGE.resize((54, 50), Image.Resampling.LANCZOS)


def achievement_card(title: str, description: str, *, hidden: bool = False) -> Image.Image:
    size = (1570, 95)
    card = make_plate(size, fill_mode="dark", border_outer=(50, 37, 24, 255), border_inner=(92, 97, 107, 255))
    draw = ImageDraw.Draw(card)

    title_font = font(FONT_REAVER, 28)
    body_font = font(FONT_RADIANCE, 18)
    stat_font = font(FONT_REAVER, 26)

    left_title = "???" if hidden else title
    left_body = "???" if hidden else description
    mid_text = "???" if hidden else "0/1"
    right_text = "???" if hidden else "5 CC"

    shadowed_text(draw, (26, 18), left_title, title_font, CREAM, anchor="la")
    shadowed_text(draw, (26, 51), left_body, body_font, MUTED, anchor="la", shadow_fill=(8, 8, 8, 140))
    shadowed_text(draw, (1005, size[1] // 2 + 3), mid_text, stat_font, (207, 213, 222, 255), anchor="mm")
    shadowed_text(draw, (1442, 34), right_text, stat_font, CREAM, anchor="mm")

    badge = reward_badge(secret=hidden)
    card.alpha_composite(badge, (size[0] - badge.width - 20, (size[1] - badge.height) // 2))
    return card


def draw_scrollbar(base: Image.Image, top: int, bottom: int, x: int, thumb_top: int, thumb_height: int) -> None:
    draw = ImageDraw.Draw(base)
    draw.rounded_rectangle((x, top, x + 7, bottom), radius=3, fill=(36, 37, 41, 255))
    draw.rounded_rectangle((x, thumb_top, x + 7, thumb_top + thumb_height), radius=3, fill=(112, 116, 122, 255))


def compose_screen(secret_tab: bool) -> Image.Image:
    screen = create_background()

    back = make_back_button()
    screen.alpha_composite(back, (22, 168))

    left_tab = make_subtab("ACHIEVEMENTS", active=not secret_tab)
    right_tab = make_subtab("SECRET", active=secret_tab)
    tabs_total_width = left_tab.width + right_tab.width + 18
    tabs_x = (CANVAS_SIZE[0] - tabs_total_width) // 2
    tabs_y = 160
    paste_with_shadow(screen, left_tab, (tabs_x, tabs_y), blur=10, alpha=90)
    paste_with_shadow(screen, right_tab, (tabs_x + left_tab.width + 18, tabs_y), blur=10, alpha=90)

    summary = progress_panel("0/100 ACHIEVEMENTS", "0%", hidden=secret_tab)
    paste_with_shadow(screen, summary, (51, 258), blur=14, alpha=110)

    draw = ImageDraw.Draw(screen)
    section_font = font(FONT_REAVER, 38)
    section_text = "???" if secret_tab else "ACHIEVEMENTS  0/100"
    shadowed_text(draw, (52, 364), section_text, section_font, CREAM, anchor="la", stroke_width=1, stroke_fill=GOLD_DARK)

    cards = [
        ("Collector 1", "Discover 1 items."),
        ("Field Notes 1", "Discover 1 enemies."),
        ("Token Rank 1", "Unlock Gambler's Token level 1."),
        ("First Blood", "Kill 1 enemy."),
        ("Boss Hunter", "Defeat 1 stage boss."),
        ("First Steps", "Clear your opening floor."),
    ]
    card_y = 416
    for idx, (title, desc) in enumerate(cards):
        card = achievement_card(title, desc, hidden=secret_tab)
        paste_with_shadow(screen, card, (51, card_y + idx * 104), blur=12, alpha=100)

    draw_scrollbar(screen, 360, 902, 1620, 438 if not secret_tab else 362, 92 if not secret_tab else 128)
    return screen


def save(image: Image.Image, name: str) -> Path:
    out_path = OUT_DIR / name
    flattened = Image.new("RGBA", image.size, (5, 6, 8, 255))
    flattened.alpha_composite(image)
    flattened.convert("RGB").save(out_path)
    return out_path


def main() -> None:
    achievements = compose_screen(secret_tab=False)
    secret = compose_screen(secret_tab=True)
    out_a = save(achievements, "achievements_screen_mainmenu_style.png")
    out_s = save(secret, "secret_screen_mainmenu_style.png")
    print(out_a)
    print(out_s)


if __name__ == "__main__":
    main()
