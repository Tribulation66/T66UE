from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageOps


ROOT = Path(__file__).resolve().parents[1]
CHEST_SOURCE_DIR = ROOT / "SourceAssets" / "ItemSprites"
CURRENCY_SOURCE = ROOT / "SourceAssets" / "UI" / "Dota" / "Generated" / "frontend_topbar_achievement_coins_icon.png"
CHEST_OUTPUT_DIR = ROOT / "SourceAssets" / "UI" / "ChestRewards"
CURRENCY_OUTPUT_DIR = ROOT / "SourceAssets" / "UI" / "Currency"
CANVAS_SIZE = (256, 256)

try:
    RESAMPLING = Image.Resampling
except AttributeError:
    class _Resampling:
        LANCZOS = Image.LANCZOS
        BICUBIC = Image.BICUBIC

    RESAMPLING = _Resampling()


RARITIES = {
    "black": {"glow": (255, 195, 92, 210)},
    "red": {"glow": (255, 116, 96, 220)},
    "yellow": {"glow": (255, 214, 102, 220)},
    "white": {"glow": (255, 239, 160, 230)},
}


def ensure_dirs() -> None:
    CHEST_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    CURRENCY_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)


def remove_background(image: Image.Image) -> Image.Image:
    source = image.convert("RGBA")
    small = 18
    corners = [
        source.crop((0, 0, small, small)),
        source.crop((source.width - small, 0, source.width, small)),
        source.crop((0, source.height - small, small, source.height)),
        source.crop((source.width - small, source.height - small, source.width, source.height)),
    ]

    avg_r = avg_g = avg_b = 0.0
    count = 0
    for corner in corners:
        for r, g, b, _ in corner.getdata():
            avg_r += r
            avg_g += g
            avg_b += b
            count += 1

    bg = (avg_r / count, avg_g / count, avg_b / count)

    def distance(pixel: tuple[int, int, int, int]) -> float:
        r, g, b, _ = pixel
        dr = r - bg[0]
        dg = g - bg[1]
        db = b - bg[2]
        return (dr * dr + dg * dg + db * db) ** 0.5

    mask = Image.new("L", source.size, 0)
    mask_pixels = []
    for pixel in source.getdata():
        dist = distance(pixel)
        alpha = 0
        if dist >= 52:
            alpha = 255
        elif dist > 20:
            alpha = int(((dist - 20) / 32.0) * 255)
        mask_pixels.append(alpha)
    mask.putdata(mask_pixels)
    mask = mask.filter(ImageFilter.GaussianBlur(1.5))

    isolated = source.copy()
    isolated.putalpha(mask)
    return isolated


def center_sprite(image: Image.Image, canvas_size: tuple[int, int], scale: float = 1.0) -> Image.Image:
    sprite = image.convert("RGBA")
    bbox = sprite.getchannel("A").getbbox()
    if not bbox:
        return Image.new("RGBA", canvas_size, (0, 0, 0, 0))

    sprite = sprite.crop(bbox)
    width = max(1, int(sprite.width * scale))
    height = max(1, int(sprite.height * scale))
    sprite = sprite.resize((width, height), RESAMPLING.LANCZOS)

    canvas = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
    offset = ((canvas_size[0] - width) // 2, canvas_size[1] - height - 34)
    canvas.alpha_composite(sprite, offset)
    return canvas


def center_full_frame(image: Image.Image, canvas_size: tuple[int, int], scale: float = 1.0) -> Image.Image:
    frame = image.convert("RGBA")
    width = max(1, int(frame.width * scale))
    height = max(1, int(frame.height * scale))
    frame = frame.resize((width, height), RESAMPLING.LANCZOS)

    canvas = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
    offset = ((canvas_size[0] - width) // 2, (canvas_size[1] - height) // 2 + 8)
    canvas.alpha_composite(frame, offset)
    return canvas


def add_shadow(image: Image.Image, offset: tuple[int, int], blur_radius: int, opacity: int) -> Image.Image:
    alpha = image.getchannel("A")
    shadow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    shadow_layer = Image.new("RGBA", image.size, (0, 0, 0, opacity))
    shadow.paste(shadow_layer, offset, alpha)
    shadow = shadow.filter(ImageFilter.GaussianBlur(blur_radius))
    return Image.alpha_composite(shadow, image)


def make_closed_chest(source_path: Path) -> Image.Image:
    source = Image.open(source_path).convert("RGBA")
    closed = center_full_frame(source, CANVAS_SIZE, scale=0.42)
    closed = ImageEnhance.Brightness(closed).enhance(0.56)
    closed = ImageEnhance.Color(closed).enhance(0.78)

    shifted = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    shifted.alpha_composite(closed, (0, 14))
    return add_shadow(shifted, (8, 16), blur_radius=18, opacity=140)


def make_open_chest(source_path: Path, glow_color: tuple[int, int, int, int]) -> Image.Image:
    source = Image.open(source_path).convert("RGBA")
    opened = center_full_frame(source, CANVAS_SIZE, scale=0.46)

    glow = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_center = (CANVAS_SIZE[0] // 2, CANVAS_SIZE[1] // 2 + 2)
    glow_draw.ellipse(
        (glow_center[0] - 60, glow_center[1] - 86, glow_center[0] + 60, glow_center[1] + 30),
        fill=glow_color,
    )
    glow = glow.filter(ImageFilter.GaussianBlur(24))

    open_canvas = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    open_canvas = Image.alpha_composite(open_canvas, glow)
    open_canvas = Image.alpha_composite(open_canvas, opened)
    return add_shadow(open_canvas, (8, 16), blur_radius=18, opacity=120)


def make_gold_icon() -> Image.Image:
    source = Image.open(CURRENCY_SOURCE).convert("RGBA")
    bbox = source.getchannel("A").getbbox()
    if bbox:
        source = source.crop(bbox)

    source = source.resize((96, 96), RESAMPLING.LANCZOS)
    canvas = Image.new("RGBA", (128, 128), (0, 0, 0, 0))

    shadow = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    shadow.alpha_composite(Image.new("RGBA", source.size, (0, 0, 0, 120)), (18, 22))
    shadow.putalpha(ImageChops.multiply(shadow.getchannel("A"), Image.new("L", shadow.size, 255)))
    shadow = shadow.filter(ImageFilter.GaussianBlur(10))
    canvas = Image.alpha_composite(canvas, shadow)
    canvas.alpha_composite(source, (16, 14))
    return canvas


def make_debt_icon(gold_icon: Image.Image) -> Image.Image:
    alpha = gold_icon.getchannel("A")
    gray = ImageOps.grayscale(gold_icon)
    tinted = ImageOps.colorize(gray, black="#2B0E0E", white="#C53B3B").convert("RGBA")
    tinted.putalpha(alpha)

    draw = ImageDraw.Draw(tinted)
    draw.line((24, 102, 102, 24), fill=(255, 236, 236, 240), width=14)
    draw.line((24, 102, 102, 24), fill=(92, 10, 10, 235), width=8)
    draw.line((36, 110, 110, 36), fill=(255, 188, 188, 190), width=4)
    return tinted


def generate() -> None:
    ensure_dirs()

    gold_icon = make_gold_icon()
    gold_icon.save(CURRENCY_OUTPUT_DIR / "gold_icon.png")

    debt_icon = make_debt_icon(gold_icon)
    debt_icon.save(CURRENCY_OUTPUT_DIR / "debt_icon.png")

    for rarity, data in RARITIES.items():
        source_path = CHEST_SOURCE_DIR / f"Item_TreasureChest_{rarity}.png"
        if not source_path.exists():
            raise FileNotFoundError(f"Missing chest sprite: {source_path}")

        closed = make_closed_chest(source_path)
        closed.save(CHEST_OUTPUT_DIR / f"chest_reward_{rarity}_closed.png")

        open_variant = make_open_chest(source_path, data["glow"])
        open_variant.save(CHEST_OUTPUT_DIR / f"chest_reward_{rarity}_open.png")


if __name__ == "__main__":
    generate()
