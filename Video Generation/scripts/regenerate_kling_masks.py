from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(r"C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM")
IMAGE_PATH = ROOT / "main_menu_newmm_base_1920.png"
OUT_DIR = ROOT / "VideoGen"
STATIC_MASK_PATH = OUT_DIR / "main_menu_static_freeze_mask.png"
CORONA_MASK_PATH = OUT_DIR / "main_menu_corona_dynamic_mask.png"


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    with Image.open(IMAGE_PATH) as source:
        width, height = source.size

    static = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(static)

    altar_polygon = [
        (245, height),
        (245, 1035),
        (360, 958),
        (455, 870),
        (565, 790),
        (675, 710),
        (730, 620),
        (760, 500),
        (820, 385),
        (940, 325),
        (1095, 355),
        (1175, 470),
        (1215, 615),
        (1310, 705),
        (1430, 805),
        (1550, 910),
        (1665, 1015),
        (1680, height),
    ]

    # Freeze idol, altar, inner eclipse disk, and bottom strip. Leave corona and stars free.
    draw.polygon(altar_polygon, fill=255)
    draw.ellipse((715, 315, 1225, 780), fill=255)
    draw.polygon([(710, 690), (1225, 690), (1325, 930), (610, 930)], fill=255)
    draw.ellipse((685, 215, 1280, 810), fill=255)
    draw.ellipse((630, 160, 1335, 870), fill=0)
    draw.ellipse((690, 220, 1275, 805), fill=255)
    draw.ellipse((715, 315, 1225, 780), fill=255)
    draw.polygon(altar_polygon, fill=255)
    draw.rectangle((0, height - 70, width, height), fill=255)

    static = static.filter(ImageFilter.GaussianBlur(radius=5))
    static = static.point(lambda pixel: 255 if pixel > 40 else 0)
    static.save(STATIC_MASK_PATH)

    dynamic = Image.new("L", (width, height), 0)
    dynamic_draw = ImageDraw.Draw(dynamic)
    dynamic_draw.ellipse((585, 130, 1385, 900), fill=255)
    dynamic_draw.ellipse((675, 210, 1295, 810), fill=0)
    dynamic = dynamic.filter(ImageFilter.GaussianBlur(radius=3))
    dynamic = dynamic.point(lambda pixel: 255 if pixel > 40 else 0)
    dynamic.save(CORONA_MASK_PATH)

    print(f"image={IMAGE_PATH} size={width}x{height}")
    print(f"static_mask={STATIC_MASK_PATH}")
    print(f"corona_mask={CORONA_MASK_PATH}")


if __name__ == "__main__":
    main()
